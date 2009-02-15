/***************************************************************************
 sci_memory.c Copyright (C) 2001 Alexander R Angas
			Refcounted memory by Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Alexander R Angas (Alex Angas) <wgd@internode.on.net>

 History:

   20010815 - assembled from the various memory allocation functions lying
              about, namely console.c (extra dmalloc stuff), menubar.c
              (for malloc_cpy -> strdup, malloc_ncpy -> strndup).
                -- Alex Angas

***************************************************************************/

#include <sci_memory.h>

/*#define POISON_MEMORY*/

/* set optimisations for Win32: */
/* g on: enable global optimizations */
/* t on: use fast code */
/* y on: suppress creation of frame pointers on stack */
/* s off: disable minimize size code */

#ifdef _MSC_VER
#	include <crtdbg.h>
#	ifndef SATISFY_PURIFY
#		pragma optimize( "s", off )
#		pragma optimize( "gty", on )
#		pragma intrinsic( memcpy, strlen )
#	endif
#endif


void *
_SCI_MALLOC(size_t size, const char *file, int line, const char *funct)
{
	void *res;
#ifdef MALLOC_DEBUG
	INFO_MEMORY("_SCI_MALLOC()", size, file, line, funct);
#endif
	ALLOC_MEM((res = malloc(size)), size, file, line, funct)
#ifdef POISON_MEMORY
	{
		memset(res, 0xa5, size);
	}
#endif
	return res;
}


void *
_SCI_CALLOC(size_t num, size_t size, const char *file, int line, const char *funct)
{
	void *res;
#ifdef MALLOC_DEBUG
	INFO_MEMORY("_SCI_CALLOC()", size, file, line, funct);
#endif
	ALLOC_MEM((res = calloc(num, size)), num * size, file, line, funct)
	return res;
}


void *
_SCI_REALLOC(void *ptr, size_t size, const char *file, int line, const char *funct)
{
	void *res;
#ifdef MALLOC_DEBUG
	INFO_MEMORY("_SCI_REALLOC()", size, file, line, funct);
#endif
	ALLOC_MEM((res = realloc(ptr, size)), size, file, line, funct)
	return res;
}


void
_SCI_FREE(void *ptr, const char *file, int line, const char *funct)
{
#ifdef MALLOC_DEBUG
	INFO_MEMORY("_SCI_FREE()", 0, file, line, funct);
#endif
	if (!ptr)
	{
		fprintf(stderr, "_SCI_FREE() [%s (%s) : %u]\n",
			file, funct, line);
		fprintf(stderr, " attempt to free NULL pointer\n");
		BREAKPOINT();
	}
	free(ptr);
}


void *
_SCI_MEMDUP(const void *ptr, size_t size, const char *file, int line, const char *funct)
{
	void *res;
#ifdef MALLOC_DEBUG
	INFO_MEMORY("_SCI_MEMDUP()", size, file, line, funct);
#endif
	if (!ptr)
	{
		fprintf(stderr, "_SCI_MEMDUP() [%s (%s) : %u]\n",
			file, funct, line);
		fprintf(stderr, " attempt to memdup NULL pointer\n");
		BREAKPOINT();
	}
	ALLOC_MEM((res = malloc(size)), size, file, line, funct)
	memcpy(res, ptr, size);
	return res;
}


char *
_SCI_STRDUP(const char *src, const char *file, int line, const char *funct)
{
	void *res;
#ifdef MALLOC_DEBUG
	INFO_MEMORY("_SCI_STRDUP()", 0, file, line, funct);
#endif
	if (!src)
	{
		fprintf(stderr, "_SCI_STRDUP() [%s (%s) : %u]\n",
			file, funct, line);
		fprintf(stderr, " attempt to strdup NULL pointer\n");
		BREAKPOINT();
	}
	ALLOC_MEM((res = strdup(src)), strlen(src), file, line, funct)
	return (char*)res;
}


char *
_SCI_STRNDUP(const char *src, size_t length, const char *file, int line, const char *funct)
{
	void *res;
	char *strres;
	size_t rlen = (int)MIN(strlen(src), length) + 1;
#ifdef MALLOC_DEBUG
	INFO_MEMORY("_SCI_STRNDUP()", 0, file, line, funct);
#endif
	if (!src)
	{
		fprintf(stderr, "_SCI_STRNDUP() [%s (%s) : %u]\n",
			file, funct, line);
		fprintf(stderr, " attempt to strndup NULL pointer\n");
		BREAKPOINT();
	}
	ALLOC_MEM((res = malloc(rlen)), rlen, file, line, funct)

	strres = (char*)res;
	strncpy(strres, src, rlen);
	strres[rlen - 1] = 0;

	return strres;
}


/********** Win32 functions **********/

#ifdef _MSC_VER
void
debug_win32_memory(int dbg_setting)
{
#if defined(NDEBUG)
	fprintf(stderr,
		"WARNING: Cannot debug Win32 memory in release mode.\n");
#elif defined(SATISFY_PURIFY)
	fprintf(stderr,
		"WARNING: Cannot debug Win32 memory in this mode.\n");
#else

	int tmpFlag;

	tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	if (dbg_setting > 0)
		tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
		/* call _CrtCheckMemory at every request */

	if (dbg_setting > 1)
        tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
		/* perform automatic leak checking at program exit */

	if (dbg_setting > 2)
        tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
		/* enable debug heap allocations */

	if (dbg_setting > 3)
	{
		PANIC((stderr, "Invalid value for debug_win32_memory!\n"));
		BREAKPOINT();
	}

	if (dbg_setting <= 0)
	{
		/* turn off above */
		tmpFlag &= ~_CRTDBG_CHECK_ALWAYS_DF;
        tmpFlag &= ~_CRTDBG_DELAY_FREE_MEM_DF;
        tmpFlag &= ~_CRTDBG_LEAK_CHECK_DF;
	}

	/* set new state for flag */
	_CrtSetDbgFlag( tmpFlag );
#endif
}
#endif



/*-------- Refcounting ----------*/

#define REFCOUNT_OVERHEAD (sizeof(guint32)*3)
#define REFCOUNT_MAGIC_LIVE_1 0xebdc1741
#define REFCOUNT_MAGIC_LIVE_2 0x17015ac9
#define REFCOUNT_MAGIC_DEAD_1 0x11dead11
#define REFCOUNT_MAGIC_DEAD_2 0x22dead22

#define REFCOUNT_CHECK(p) ((((guint32 *)(p))[-3] == REFCOUNT_MAGIC_LIVE_2)    \
			   && (((guint32 *)(p))[-1] == REFCOUNT_MAGIC_LIVE_1))

#define REFCOUNT(p) (((guint32 *)p)[-2])

#undef TRACE_REFCOUNT


extern void *
sci_refcount_alloc(size_t length)
{
	guint32 *data = (guint32*)sci_malloc(REFCOUNT_OVERHEAD + length);
#ifdef TRACE_REFCOUNT
fprintf(stderr, "[] REF: Real-alloc at %p\n", data);
#endif
	data += 3;

	data[-1] = REFCOUNT_MAGIC_LIVE_1;
	data[-3] = REFCOUNT_MAGIC_LIVE_2;
	REFCOUNT(data) = 1;
#ifdef TRACE_REFCOUNT
fprintf(stderr, "[] REF: Alloc'd %p (ref=%d) OK=%d\n", data, REFCOUNT(data),
	REFCOUNT_CHECK(data));
#endif
	return data;
}

extern void *
sci_refcount_incref(void *data)
{
	if (!REFCOUNT_CHECK(data)) {
		BREAKPOINT();
	} else
		REFCOUNT(data)++;

#ifdef TRACE_REFCOUNT
fprintf(stderr, "[] REF: Inc'ing %p (now ref=%d)\n", data, REFCOUNT(data));
#endif
	return data;
}

extern void
sci_refcount_decref(void *data)
{
#ifdef TRACE_REFCOUNT
fprintf(stderr, "[] REF: Dec'ing %p (prev ref=%d) OK=%d\n", data, REFCOUNT(data),
	REFCOUNT_CHECK(data));
#endif
	if (!REFCOUNT_CHECK(data)) {
		BREAKPOINT();
	} else if (--REFCOUNT(data) == 0) {
		guint32 *fdata = (guint32*)data;

		fdata[-1] = REFCOUNT_MAGIC_DEAD_1;
		fdata[-3] = REFCOUNT_MAGIC_DEAD_2;

#ifdef TRACE_REFCOUNT
fprintf(stderr, "[] REF: Freeing (%p)...\n", fdata - 3);
#endif
		sci_free(fdata - 3);
#ifdef TRACE_REFCOUNT
fprintf(stderr, "[] REF: Done.\n");
#endif
	}
}

extern void *
sci_refcount_memdup(void *data, size_t len)
{
	void *dest = sci_refcount_alloc(len);
	memcpy(dest, data, len);
	return dest;
}
