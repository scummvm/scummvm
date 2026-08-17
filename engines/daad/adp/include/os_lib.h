#pragma once

#include <os_types.h>

// Debug functions

#ifndef TRACE_ON
#define TRACE_ON 0
#endif

#if TRACE_DEBUG
#define TRACE DebugPrintf
#elif TRACE_ON
#  ifdef __cplusplus
extern "C" {
#  endif
extern void TracePrintf(const char* format, ...);
#  ifdef __cplusplus
}
#  endif
#define TRACE TracePrintf
#else
#define TRACE(...)
#endif

#ifdef _DEBUGPRINT
#  if defined(_ATARIST)
	extern void DebugPrintfImpl(const char* format, ...);
#    define DebugPrintf(...) DebugPrintfImpl(__VA_ARGS__)
#  elif defined(_DOS)
	extern void DebugPrintfImpl(const char* format, ...);
#    define DebugPrintf(...) DebugPrintfImpl(__VA_ARGS__)
#  elif defined(_STDCLIB)
#    include <stdio.h>
#    define DebugPrintf(...) printf(__VA_ARGS__)
#  elif defined(_AMIGA)
#    include "../src-amiga/gcc8_c_support.h"
#    define DebugPrintf(...) KPrintF(__VA_ARGS__)
#  else
#    define DebugPrintf(...)
#  endif
#else
#  define DebugPrintf(...)
#endif

// Misc functions

extern uint32_t RandInt    (uint32_t min, uint32_t max);
extern void     RandSeed   (uint32_t seed);
extern void     RandSetDefaultSeed (uint32_t seed);
extern void     RandSeedFromClock  (uint32_t entropy);
extern void     RandReset  ();
extern void     Abort      ();

// OS Specific

enum OSMemoryPool
{
	OSMemoryPool_Any = 0,
	OSMemoryPool_Chip,
};

extern void     OSInit     ();
extern void     OSError    (const char* message);
extern void     OSSyncFS   ();
extern void*    OSAlloc    (size_t size, OSMemoryPool pool);
extern void     OSFree     (void* mem);

static inline void* OSAlloc(size_t size)
{
	return OSAlloc(size, OSMemoryPool_Any);
}

extern size_t   OSReserveArena	(size_t size);
extern void     OSReleaseArena	();
extern size_t   OSGetArenaSize	();
extern size_t   OSGetFree		();

// Memory/string functions

extern void*       MemSet     (void* mem, uint8_t val,     size_t size);
extern void*       MemClear   (void* mem, size_t size);
extern void*       MemMove    (void* dst, const void* src, size_t size);
extern void*       MemCopy    (void* dst, const void* src, size_t size);
extern int         MemComp    (void* dst, const void* src, size_t size);

extern char*       LongToChar (long value, char *buffer, int radix);
extern void        IntToHex2  (int value, char *buffer);

extern size_t      StrCopy    (char* dst, uint32_t dstSize, const char* src);
extern const char* StrRChr    (const char* ptr, char c);
extern size_t      StrCat     (char* dst, uint32_t dstSize, const char* src);
extern int         StrComp    (const char* dst, const char* src);
extern int         StrIComp   (const char* dst, const char* src);
extern int         StrComp    (const char* dst, const char* src, size_t maxSize);

// Widely used, uses static buffer

extern const char* ChangeExtension (const char* filename, const char* newExtension);

// Convenience inline versions for uint8_t string buffers

static inline size_t StrCopy (uint8_t* dst, uint32_t dstSize, const uint8_t* src)
{
	return StrCopy ((char *)dst, dstSize, (const char*)src);
}
static inline size_t StrCat  (uint8_t* dst, uint32_t dstSize, const uint8_t* src)
{
	return StrCat ((char *)dst, dstSize, (const char*)src);
}
static inline int    StrComp (const uint8_t* dst, const uint8_t* src)
{
	return StrComp ((const char*)dst, (const char*)src);
}
static inline int    StrComp (const uint8_t* dst, const uint8_t* src, size_t maxSize)
{
	return StrComp ((const char*)dst, (const char*)src, maxSize);
}
static inline uint8_t* LongToChar (long value, uint8_t *buffer, int radix)
{
	return (uint8_t*)LongToChar (value, (char*)buffer, radix);
}

#ifdef _STDCLIB

#include <string.h>
static inline size_t StrLen(const char *str)
{
	return strlen(str);
}
static inline size_t StrLen  (const uint8_t* str)
{
	return strlen ((const char*)str);
}

#else

static inline size_t StrLen(const char *str)
{
	size_t len = 0;
	while (*str)
	{
		str++;
		len++;
	}
	return len;
}
static inline size_t StrLen  (const uint8_t* str)
{
	return StrLen ((const char*)str);
}

#endif
