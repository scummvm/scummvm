#ifndef STDLIB_H
#define STDLIB_H

#include <PalmOS.h>
#include "mathlib.h"

//#define memNewChunkFlagNonMovable	0x0200 
#define memNewChunkFlagAllowLarge	0x1000  // this is not in the sdk *g* 
#define memHeapFlagReadOnly			0x0001

#define	NO_HEAP_FOUND				-1

SysAppInfoPtr SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP) 
							SYS_TRAP(sysTrapSysGetAppInfo); 


#define atoi				StrAToI
#define atol				StrAToI
#define abs(a)				((a) < 0 ? -(a) : (a))
//#define abs					fabs
#define	malloc(a)			calloc(a,1)
//#define free				MemPtrFree
#define strtol(a,b,c)		StrAToI(a)
#define qsort(a,b,c,d)
#define rand()				SysRandom(0)

void MemExtInit();
void MemExtCleanup();

MemPtr realloc(MemPtr oldP, UInt32 size);
MemPtr calloc(UInt32 nelem, UInt32 elsize);
Err free(MemPtr memP);
void exit(Int16 status);
void *bsearch(const void *key, const void *base, UInt32 nmemb, 
				UInt32 size, int (*compar)(const void *, const void *));


#endif