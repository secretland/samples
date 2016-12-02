#include <assert.h>
#include "nbtdefs.h"

int sendpacket_direct(SOCKET ofd, const void *pak, int len, const struct sockaddr_in *dst)
{
    assert(pak != 0);
    assert(dst != 0);

    return sendto_in(ofd, pak, len, 0, dst);
}

int recvpacket_direct(SOCKET ifd, void *pak, int maxlen, struct sockaddr_in *src)
{
#ifdef _SCO_DS
    int    srclen = sizeof *src;
#else
    size_t    srclen = sizeof *src;
#endif

    assert(pak != 0);
    assert(src != 0);

    return recvfrom_in(ifd, pak, maxlen, 0, src, &srclen);
}
