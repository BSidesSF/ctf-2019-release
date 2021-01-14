#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

#ifdef DEBUG
# define DBG(fmt, args...)  do { \
    fprintf(stderr, (fmt), ## args); \
    fprintf(stderr, "\n"); \
    fflush(stderr); \
  } while(0)
#else
# define DBG(fmt, ...) do {} while(0)
#endif

#endif // _COMMON_H_
