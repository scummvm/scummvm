#include <PalmOS.h>

#define memcmp			MemCmp
#define memcpy			MemMove
#define memmove			MemMove
#define memset(a,b,c)	MemSet(a,c,b)
#define strcat			StrCat
#define strchr			StrChr
#define strcmp			StrCompare
#define strcpy			StrCopy
#define strncpy			StrNCopy
#define stricmp			StrCaselessCompare
#define strlen			StrLen
#define strncmp			StrNCompare
#define strstr			StrStr

Char *strtok(Char *str, const Char *sep);
Char *strrchr(const Char *s, int c);
Char *strdup(const Char *strSource);
Char *strpbrk(const Char *s1, const Char *s2);

#define StrTok			strtok