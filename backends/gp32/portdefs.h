	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
	#include <ctype.h>

extern "C" {
	#include "h/gpfont.h"
	#include "h/gpfont_port.h"
	#include "h/gpgraphic.h"
	#include "h/gpmm.h"	
	#include "h/gpmem.h"	
	#include "h/gpos_def.h"
	#include "h/gpstdio.h"
	#include "h/gpstdlib.h"	
	#include "h/gpdef.h"
	#include "h/defines.h"
}

	extern int gpprintf(const char *fmt, ...);
	#define printf gpprintf

	extern void *gpmalloc(size_t size);
	extern void *gpcalloc(size_t nitems, size_t size);
	extern void gpfree(void *block);
	#define malloc gpmalloc 
	#define calloc gpcalloc //gm_calloc
	#define free gpfree	
	/*#define memset gm_memset
	#define memcopy gm_memcopy

	#define strcpy gm_strcpy	// uncomment?
	#define strncpy gm_strncpy
	#define strcat gm_strcat
	#define sprintf gm_sprintf*/

	#define assert(e) ((e) ? 0 : (printf("!AS: " #e " (%s, %d)\n", __FILE__, __LINE__)))
	#define ASSERT assert

	#define ENDLESSLOOP while (1)

	#define FILE F_HANDLE
	#define stderr NULL	// hack...
	#define stdout stderr
	#define stdin stderr

	extern FILE *gpfopen(const char *filename, const char *mode);
	extern int gpfclose(FILE *stream);
	extern int gpfseek(FILE *stream, long offset, int whence);
	extern  size_t gpfread(void *ptr, size_t size, size_t n, FILE *stream);
	extern size_t gpfwrite(const void *ptr, size_t size, size_t n, FILE*stream);
	extern long gpftell(FILE *stream);
	extern void gpclearerr(FILE *stream);
	extern int gpfeof(FILE *stream);
	extern char *gpfgets(char *s, int n, FILE *stream);
	extern int gpfflush(FILE *stream);

	#define fopen gpfopen
	#define fclose gpfclose
	#define fseek gpfseek
	#define fread gpfread
	#define fwrite gpfwrite
	#define ftell gpftell
	#define clearerr gpclearerr
	#define feof gpfeof
	#define fgets gpfgets	
	
	extern int gpfprintf(FILE *stream, const char *fmt, ...);
	#define fprintf gpfprintf
	#define fflush gpfflush

	extern void gphalt(int code=0);
	#define exit gphalt
	//#define error printf

	#define time(x) (0) // fixme! (SIMON)

	// EOF
