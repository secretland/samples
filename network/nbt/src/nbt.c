#include <assert.h>
#include "nbtdefs.h"

static unsigned short bind_portno = 0;
static unsigned short dest_portno = 137;
static int write_sleep_msecs = 10;
static int broadcast = 0;

static int sendpacket(int sfd, const void *pak, int len, const struct sockaddr_in *dst)
{
    return sendpacket_direct(sfd, pak, len, dst);
}

static int recvpacket(int sfd, void *pak, int len, struct sockaddr_in *dst)
{
    return recvpacket_direct(sfd, pak, len, dst);
}

static void fill_namerequest(struct NMBpacket *pak, int *len, short seq)
{
    char    *pbuf;

    assert(pak != 0);
    assert(len != 0);

    *len = 50;

    memset(pak, 0, *len);

    /* POPULATE THE HEADER */

    pak->tranid  = htons(seq);  /* transaction ID */
    pak->flags   = 0;
    pak->qdcount = htons(1);    /* query count */
    pak->ancount = 0;
    pak->nscount = 0;
    pak->arcount = 0;

    if(broadcast)
        pak->flags |= htons(0x0010);	/* broadcast */

    /*----------------------------------------------------------------
     * Encode the NETBIOS name, which is really just a "*" that's
     * fully padded out. Then add the status and name class at the
     * end.
     */
    pbuf = pak->data;

    pbuf += NETBIOS_pack_name("*", 0, pbuf);
    *pbuf++ = 0x00;	/* length of next segment */

    *pbuf++ = 0x00;	/* NODE STATUS */
    *pbuf++ = 0x21;

    *pbuf++ = 0x00;	/* IN */
    *pbuf++ = 0x01;
}

int query_name(SOCKET sockfd, char const* ip_address, char* netbios_name, unsigned int len)
{
    char  errbuf[256];
    int seq = 1000;
    {
        struct sockaddr_in  dst;
        struct NMBpacket    pak;
        int sendlen;

        memset(&dst, 0, sizeof dst);

        dst.sin_family      = AF_INET;
        dst.sin_addr.s_addr = inet_addr(ip_address);
        dst.sin_port        = htons(dest_portno);

        // have_next_addr = FALSE;

        fill_namerequest(&pak, &sendlen, seq++);

        /* yes, ignore response! */
        sendpacket(sockfd, &pak, sendlen, &dst);

        if(write_sleep_msecs > 0)
            sleep_msecs(write_sleep_msecs);

        // npending++;
    }
    {
        int    paklen;
        struct sockaddr_in  src;
        struct NMBpacket    pak;
        struct NMB_query_response rsp;

        memset(&src, 0, sizeof src);
        memset(&rsp, 0, sizeof rsp);

        if((paklen = (int)recvpacket(sockfd, &pak, sizeof pak, &src)) < 0)
            return -2;

        if(parse_nbtstat(&pak, paklen, &rsp, errbuf) != FALSE)
        {
            if(netbios_name != NULL && len > strlen(rsp.computer))
                strcpy(netbios_name, rsp.computer);
            else
                return -1;
        }
        else
        {
            return -2;
        }
    }
    return 0;
}

int netbios_by_ip(char const* ip, char* name, int len)
{
    struct sockaddr_in  myaddr;
    SOCKET sockfd;

    if(NULL == ip || NULL == name || 0 == len)
        return -1;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if(!SOCKET_IS_VALID(sockfd))
    {
        // printf("ERROR: cannot create socket [%s]", NATIVE_ERROR);
        return -2;
    }

    memset(&myaddr, 0, sizeof myaddr);
    myaddr.sin_family      = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port        = htons(bind_portno);

    if(bind(sockfd, (struct sockaddr const*)&myaddr, sizeof(myaddr)) != 0)
    {
#ifdef _WIN32
        // printf("ERROR: cannot bind to local socket [%ld]", WSAGetLastError());
#else
        // printf("ERROR: cannot bind to local socket [%s]", strerror(errno));
#endif
        return -2;
    }

    return query_name(sockfd, ip, name, len);
}