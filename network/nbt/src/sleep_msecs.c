#include "penlib.h"

void __stdcall sleep_msecs(long msecs)
{
    if(msecs <= 0)
        return;
#ifdef _WIN32
    Sleep((unsigned long)msecs);
#elif defined(M_XENIX)
    napms(msecs);
#else
    usleep(msecs * 1000);    /* microseconds! */
#endif
}