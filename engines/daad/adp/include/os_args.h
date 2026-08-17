#pragma once

#ifdef _AMIGA

#include <stdarg.h>
#define VA_LIST             va_list
#define VA_START(va, start) va_start((va), (start))
#define VA_ARG(va, type)    (type)(va)
#define VA_COPY(d, s)       (d) = (s)
#define VA_END(va)          va_end((va))

#else

#include <stdarg.h>

#endif