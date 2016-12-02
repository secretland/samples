#include <stdio.h>

#include <assert.h>
#if defined(_WIN32)
    #include <winsock2.h>
#endif /* defined(_WIN32) */
#include "nbtdefs.h"

#if !defined(FALSE)
    #define FALSE   0
    #define TRUE    1
#endif /* !defined(FALSE) */

int verbose = FALSE;
int no_inverse_lookup = FALSE;
int show_mac_address = FALSE;
static unsigned short bind_portno = 0;
static int full_nbtstat = FALSE;

char ip_address[] = "192.168.15.33";

#ifdef _WIN32
    static int winsock_level = 2;
#endif

void query_name(SOCKET sockfd);

const char *unix_errorstr(int error_num)
{
    return strerror(error_num);
}

#ifdef _WIN32
const char *win32_errorstr(DWORD error_num)
{
static char errorbuf[20];

    sprintf(errorbuf, "#%ld", error_num);
    return errorbuf;
}
#endif

int main(int argc, char** argv)
{
    struct sockaddr_in  myaddr;

#ifdef _WIN32
    WORD wVersion = MAKEWORD(winsock_level,winsock_level);
    WSADATA wsaData;
    DWORD   err = (DWORD)WSAStartup(wVersion, &wsaData);
    if(err != 0)
    {
        exit(0);
    }
#endif

    SOCKET sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if(!SOCKET_IS_VALID(sockfd))
    {
        printf("ERROR: cannot create socket [%s]", NATIVE_ERROR);
        exit(0);
    }
    memset(&myaddr, 0, sizeof myaddr);

    myaddr.sin_family      = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port        = htons(bind_portno);

    if(bind(sockfd, (struct sockaddr const*)&myaddr, sizeof(myaddr)) != 0)
    {
#ifdef _WIN32
        printf("ERROR: cannot bind to local socket [%ld]", WSAGetLastError());
#else
        printf("ERROR: cannot bind to local socket [%s]", strerror(errno));
#endif
        exit(0);
    }

    query_name(sockfd);

    return 0;
}

static short dest_portno = 137;
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

    pak->tranid  = htons(seq);	/* transaction ID */
    pak->flags   = 0;
    pak->qdcount = htons(1);	/* query count */
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

void query_name(SOCKET sockfd)
{
    FILE* ofp = stdout;
    char  errbuf[256];
    int seq = 1000;
    {
        struct sockaddr_in  dst;
        struct NMBpacket    pak;
        int sendlen;

        memset(&dst, 0, sizeof dst);

        dst.sin_family      = AF_INET;
        // dst.sin_addr.s_addr = next_addr.s_addr;
        dst.sin_addr.s_addr = inet_addr(ip_address);
        dst.sin_port        = htons(dest_portno);

        // have_next_addr = FALSE;

        fill_namerequest(&pak, &sendlen, seq++);

        if(verbose)
        {
            fprintf(ofp, "sending to %s\n", inet_ntoa(dst.sin_addr));
        }

        /* yes, ignore response! */
        (void)sendpacket(sockfd, &pak, sendlen, &dst);

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

        paklen = (int)recvpacket(sockfd, &pak, sizeof pak, &src);

        // if ( verbose )
        // {
            // if ( paklen < 0 )
            // {
                // fprintf(ofp, "Error on read: %s\n", strerror(errno));
            // }
            // else
            // {
                // fprintf(ofp, "Got %d bytes from %s\n", paklen, inet_ntoa(src.sin_addr));

                // if ( verbose > 1 )
                    // dump_nbtpacket(&pak, paklen, stdout);
            // }
        // }

        /*------------------------------------------------
         * If we actually got something from the other end,
         * parse the response, plug in the remote's IP addr,
         * and display it.
         */
        // if ( paklen <= 0 ) continue;

        // npending--;

        if(parse_nbtstat(&pak, paklen, &rsp, errbuf))
        {
            rsp.remote = src;

            // if ( target_responded(&rsp.remote.sin_addr) )
            // {
// #ifdef ENABLE_PERL
                // if ( gen_Perl )
                    // generate_perl(ofp, &rsp);
                // else
// #endif
                    // display_nbtstat(ofp, &rsp, full_nbtstat);
            // }
        }
        else
        {
            fprintf(ofp, "ERROR: no parse for %s -- %s\n", inet_ntoa(src.sin_addr), errbuf);
        }
    }
}
