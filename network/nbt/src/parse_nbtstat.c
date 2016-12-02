#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "nbtdefs.h"

/*
 * getshort()
 *
 *    Given a handle to a pointer to two bytes, fetch it as an unsigned short
 *    in network order and convert to host order. We advance the pointer.
 */
static unsigned short getshort(const char **p)
{
    unsigned short    s;

    assert( p != 0);
    assert(*p != 0);

    memcpy(&s, *p, 2);

    *p += 2;

    return ntohs(s);
}


int parse_nbtstat(const struct NMBpacket *pak, int paklen,
                  struct NMB_query_response *rsp,
                  char *errbuf)
{
    const char  *p,
                *pmax,
                *nmax,
                *pstats;
    int         rdlength,
                remaining,
                nnames;
    int         qtype,        /* query type (always "NBSTAT")        */
                qclass;        /* query class (always "IN")        */
    char        tmpbuf[256];    /* random buffer            */

    assert(pak    != 0);
    assert(rsp    != 0);
    assert(paklen >  0);
    assert(errbuf != 0);

    memset(rsp, 0, sizeof *rsp);

    /*----------------------------------------------------------------
     * Set up our initial pointers into the received record. We are
     * trying to be very careful about not running away with our
     * memory, so we set a pointer to the very end of the valid part
     * of the data from the other end, and we try to never look past
     * this.
     *
     *   +-----------------------------------------------------------+
     *   | headers |        response data                            |
     *   +-----------------------------------------------------------+
     *    ^--pak    ^--p                                         pmax-^
     *
     * Note that we do >nothing< with the headers, but probably should
     * (to verify that there is actually an answer?).
     */
    pmax = paklen + (char *)pak;
    p    = pak->data;

    /*----------------------------------------------------------------
     * The first thing we should see is the "question" section, which
     * should simply echo what we gave them. Parse this out to skip
     * past it. We decode it only for the benefit of the debugging
     * code.
     */
    NETBIOS_unpack(&p, tmpbuf, sizeof tmpbuf);

    qtype  = getshort(&p);    /* question type    */
    qclass = getshort(&p);    /* question class    */

    // if(verbose > 1)
    // {
        // printf(" QUESTION SECTION:\n");
        // printf("   name  = \"%s\"\n",    tmpbuf);
        // printf("   type  = %s\n",
            // printable_NETBIOS_question_type (tmpbuf, qtype));

        // printf("   class = %s\n",
            // printable_NETBIOS_question_class(tmpbuf, qclass));
    // }

    p += 4;                    /* skip past TTL (always zero)    */

    /*----------------------------------------------------------------
     * Fetch the length of the rest of this packet and make sure that
     * we actually have this much room left. If we don't, we must have
     * gotten a short UDP packet and won't be able to finish off this
     * processing. The max size is ~~500 bytes or so.
     */
    rdlength = getshort(&p);

    remaining = (int)(pmax - p);

    if(rdlength > remaining)
    {
        // printf(" ERROR: rdlength = %d, remaining bytes = %d\n", rdlength, remaining);
        return -1;
    }

    /*----------------------------------------------------------------
     * Fetch the number of names to be found in the rest of this node
     * object. Sometimes we get >zero< and it's not clear why this is.
     * Perhaps it means that there is no NETBIOS nameserver running
     * but it will answer status requests. Hmmm.
     */
    nnames  = *(unsigned char *)p; p++;

    // if ( verbose > 1 )
        // printf(" NODE COUNT = %d\n", nnames);

    if(nnames < 0)
    {
        // sprintf(errbuf, "bad NETBIOS response (count=%d)", nnames);
        return FALSE;
    }

    pstats = p + (nnames * NODE_RECORD_SIZE);

    if(nnames > TBLSIZE(rsp->nodes))
    {
        nnames = TBLSIZE(rsp->nodes);
        rsp->nametrunc = TRUE;
    }

    nmax = p + (nnames * NODE_RECORD_SIZE);

    for(; p < nmax; p += NODE_RECORD_SIZE)
    {
        struct node_name_record    nr;
        struct nodeinfo        *ni = &rsp->nodes[ rsp->nnodes++ ];

        /* Solaris has alignment problems, gotta copy */
        memcpy(&nr, p, NODE_RECORD_SIZE);

        ni->flags = ntohs(nr.flags);
        ni->type  = nr.type;

        strncpy(ni->name, nr.name, 15)[15] = '\0';

        strip(ni->name);
    }

    /*----------------------------------------------------------------
     * Now we've finished processing the node information and gathered
     * up everything we can find, so now look for the statistics. We
     * ONLY try to gather these stats if there is actually any room
     * left in our buffer.
     */
    if((int)(pmax - pstats) >= NODE_STATS_SIZE)
    {
        memcpy( &rsp->nodestats, pstats, NODE_STATS_SIZE );

        byteswap_nodestats( &rsp->nodestats );

        sprintf(rsp->ether, "%02x:%02x:%02x:%02x:%02x:%02x",
                rsp->nodestats.uniqueid[0],
                rsp->nodestats.uniqueid[1],
                rsp->nodestats.uniqueid[2],
                rsp->nodestats.uniqueid[3],
                rsp->nodestats.uniqueid[4],
                rsp->nodestats.uniqueid[5]);
    }

    /* postprocessing for good measure */
    process_response(rsp);

    return TRUE;
}
