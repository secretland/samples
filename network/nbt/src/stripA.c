#include <ctype.h>
#include <assert.h>
#include "penlib.h"

char * __stdcall stripA(char *str)
{
    char    *old = str;    /* save ptr to original string          */
    char    *lnsp = 0;     /* ptr to last non-space in string      */

    assert(str != 0);

    for(; *str; str++)
    {
        if(!isspace(*str))
            lnsp = str;
    }
    if(lnsp)
        lnsp[1] = '\0';
    else
        *old = '\0';
    return old;
}