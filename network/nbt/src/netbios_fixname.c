#include <assert.h>
#include <ctype.h>
#include "nbtdefs.h"

char *NETBIOS_fixname(char *buf)
{
    char *buf_save = buf;

    assert(buf != 0);

    for (; *buf; buf++)
    {
        if(!isprint(*buf))
            *buf = '.';
    }
    return strip(buf_save);
}
