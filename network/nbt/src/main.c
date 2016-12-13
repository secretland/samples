#include <stdio.h>

#include <assert.h>
#if defined(_WIN32)
    #include <winsock2.h>
#endif /* defined(_WIN32) */
#include "nbtdefs.h"

#include "nbt.h"

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

#ifdef _WIN32
    static int winsock_level = 2;
#endif

int main(int argc, char** argv)
{
    char netbios[0x20] = { 0 };

    if(argc < 2)
        return -1;
#ifdef _WIN32
    WORD wVersion = MAKEWORD(winsock_level, winsock_level);
    WSADATA wsaData;
    DWORD   err = (DWORD)WSAStartup(wVersion, &wsaData);
    if(err != 0)
    {
        exit(0);
    }
#endif

    netbios_by_ip(argv[1], netbios, sizeof(netbios));
    printf("netbios name: %s\n", netbios);
    return 0;
}
