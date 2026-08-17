#pragma once

#ifndef VERSION
#define VERSION 0
#endif
#define __xstr(a) __str(a)
#define __str(a) #a
#define VERSION_STR __xstr(VERSION)

#ifdef _STDCLIB

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#else

typedef long long           int64_t ;
typedef unsigned long long uint64_t ;
typedef long                int32_t ;
typedef unsigned int       uint32_t ;
typedef short               int16_t ;
typedef unsigned short     uint16_t ;
typedef signed   char   	int8_t  ;
typedef unsigned char      uint8_t  ;

typedef unsigned long       size_t  ;

#define bool	char
#define true    1
#define false   0

#define NULL    ((void *)0)

typedef uint32_t time_t;

#endif