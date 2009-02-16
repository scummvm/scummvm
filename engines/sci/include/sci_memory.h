/***************************************************************************
 sci_memory.h Copyright (C) 2001 Alexander R Angas


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
              about, namely resource.h, tools.c (for repeated allocation
              attempts), menubar.h (for malloc_cpy, malloc_ncpy).
                -- Alex Angas

***************************************************************************/


/** This header file defines a portable library for allocating memory safely
 ** throughout FreeSCI.
 ** Implementations of basic functions found here are in this file and
 ** $(SRCDIR)/src/scicore/sci_memory.c
 *
 **************
 *
 * Sets behaviour if memory allocation call fails.
 * UNCHECKED_MALLOCS:  use C library routine without checks
 * (nothing defined):  check mallocs and exit immediately on fail (recommended)
 *
 ** -- Alex Angas
 **
 **/


#ifndef _SCI_MEMORY_H
#define _SCI_MEMORY_H

#include "common/scummsys.h"
#include "sci/include/resource.h"

#ifdef WIN32
#  undef scim_inline /* just to be sure it is not defined */
#  define scim_inline __inline
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199900L
#  define scim_inline
#else
#  define scim_inline inline
#endif

#ifdef _MSC_VER
#  include <direct.h> // for chdir, rmdir, _gecwd, getcwd, mkdir
#endif

/********** macros for error messages **********/

/*
 * Prints an error message.
 */
#define PANIC(prn)\
	fprintf prn;

/*
 * Called if memory allocation fails.
 */
#define PANIC_MEMORY(size, filename, linenum, funcname, more_info)\
	PANIC((stderr, "Memory allocation of %lu bytes failed\n"\
		" [%s (%s) : %u]\n " #more_info "\n",\
		(unsigned long)size, filename, funcname, linenum))


/********** the memory allocation macros **********/

#define INFO_MEMORY(alloc_statement, size, filename, linenum, funcname)\
	fprintf(stderr, "ALLOC_MEM(%lu bytes) [%s (%s) : %u] "\
		#alloc_statement "\n",\
		(unsigned long)size, filename, funcname, linenum);


#ifdef UNCHECKED_MALLOCS

#define ALLOC_MEM(alloc_statement, size, filename, linenum, funcname)\
do {\
	alloc_statement;\
} while (0);

#else /* !UNCHECKED_MALLOCS */

#define ALLOC_MEM(alloc_statement, size, filename, linenum, funcname)\
do {\
	if (size == 0)\
	{\
		PANIC_MEMORY(size, filename, linenum, funcname, "WARNING: allocating zero bytes of memory.")\
	}\
	else if (!(size > 0))\
	{\
		PANIC_MEMORY(size, filename, linenum, funcname, "Cannot allocate negative bytes of memory!")\
		BREAKPOINT()\
	}\
\
	alloc_statement; /* attempt to allocate the memory */\
\
	if (res == NULL)\
	{\
		/* exit immediately */\
		PANIC_MEMORY(size, filename, linenum, funcname, "Failed! Exiting...")\
		BREAKPOINT()\
\
		/* attempt to allocate memory indefinitely\
		do\
		{\
			PANIC_MEMORY(size, filename, linenum, funcname, "Failed! Trying again...");\
			sleep(1000);\
			alloc_statement;\
		} while (res == NULL);\
		*/\
	}\
} while (0);

#endif /* !UNCHECKED_MALLOCS */


/********** memory allocation routines **********/

extern void *
	_SCI_MALLOC(size_t size, const char *file, int line, const char *funct);
/* Allocates the specified amount of memory.
** Parameters: (size_t) size: Number of bytes to allocate
**             (char *) file: Filename this routine is called from
**                            (use __FILE__)
**             (int) line: Line number this routine is called from
**                            (use __LINE__)
**             (char *) funct: Function this routine is called from
**                            (use __PRETTY_FUNCTION__ if available)
** Returns   : (void *) A pointer to the allocated memory chunk
** To free this string, use the sci_free() command.
** If the call fails, behaviour is dependent on the definition of SCI_ALLOC.
*/

extern void *
	_SCI_CALLOC(size_t num, size_t size, const char *file, int line, const char *funct);
/* Allocates num * size bytes of zeroed-out memory.
** Parameters: (size_t) num: Number of elements to allocate
**             (size_t) size: Amount of memory per element to allocate
**             Please see _SCI_MALLOC() for details on other parameters.
** Returns   : (void *) A pointer to the allocated memory chunk
** To free this string, use the sci_free() command.
** See _SCI_MALLOC() for more information if call fails.
*/

extern void *
	_SCI_REALLOC(void *ptr, size_t size, const char *file, int line, const char *funct);
/* Increases the size of an allocated memory chunk.
** Parameters: (void *) ptr: The original pointer
**             (size_t) size: New size of the memory chunk
**             Please see _SCI_MALLOC() for details on other parameters.
** Returns   : (void *) A possibly new pointer, containing 'size'
**             bytes of memory and everything contained in the original 'ptr'
**             (possibly minus some trailing data if the new memory area is
**             smaller than the old one).
** To free this string, use the sci_free() command.
** See _SCI_MALLOC() for more information if call fails.
*/

extern void
	_SCI_FREE(void *ptr, const char *file, int line, const char *funct);
/* Frees previously allocated memory chunks
** Parameters: (void *) ptr: The pointer to free
**             Please see _SCI_MALLOC() for details on other parameters.
** Returns   : (void)
*/

extern void *
	_SCI_MEMDUP(const void *src, size_t size, const char *file, int line, const char *funct);
/* Duplicates a chunk of memory
** Parameters: (void *) src: Pointer to the data to duplicate
**             (size_t) size: Number of bytes to duplicate
**             Please see _SCI_MALLOC() for details on other parameters.
** Returns   : (void *) An appropriately allocated duplicate, or NULL on error
** Please try to avoid data duplication unless absolutely neccessary!
** To free this string, use the sci_free() command.
** See _SCI_MALLOC() for more information if call fails.
*/

extern char *
	_SCI_STRDUP(const char *src, const char *file, int line, const char *funct);
/* Duplicates a string.
** Parameters: (const char *) src: The original pointer
**             Please see _SCI_MALLOC() for details on other parameters.
** Returns   : (char *) a pointer to the storage location for the copied
**             string.
** To free this string, use the sci_free() command.
** See _SCI_MALLOC() for more information if call fails.
*/


extern char *
	_SCI_STRNDUP(const char *src, size_t length, const char *file, int line, const char *funct);
/* Copies a string into a newly allocated memory part, up to a certain length.
** Parameters: (char *) src: The source string
**             (int) length: The maximum length of the string (not counting
**                           a trailing \0).
**             Please see _SCI_MALLOC() for details on other parameters.
** Returns   : (char *) The resulting copy, allocated with sci_malloc().
** To free this string, use the sci_free() command.
** See _SCI_MALLOC() for more information if call fails.
*/

/****************************************/
/* Refcounting garbage collected memory */
/****************************************/

/* Refcounting memory calls are a little slower than the others,
** and using it improperly may cuase memory leaks. It conserves
** memory, though.  */

#define SCI_REFCOUNT_TEST(f) sci_refcount_incref(f); sci_refcount_decref(f);

extern void *
	sci_refcount_alloc(size_t length);
/* Allocates "garbage" memory
** Parameters: (size_t) length: Number of bytes to allocate
** Returns   : (void *) The allocated memory
** Memory allocated in this fashion will be marked as holding one reference.
** It cannot be freed with 'free()', only by using sci_refcount_decref().
*/

extern void *
	sci_refcount_incref(void *data);
/* Adds another reference to refcounted memory
** Parameters: (void *) data: The data to add a reference to
** Returns   : (void *) data
*/

extern void
	sci_refcount_decref(void *data);
/* Decrements the reference count for refcounted memory
** Parameters: (void *) data: The data to add a reference to
** Returns   : (void *) data
** If the refcount reaches zero, the memory will be deallocated
*/

extern void *
	sci_refcount_memdup(void *data, size_t len);
/* Duplicates non-refcounted memory into a refcounted block
** Parameters: (void *) data: The memory to copy from
**             (size_t) len: The number of bytes to copy/allocate
** Returns   : (void *) Newly allocated refcounted memory
** The number of references accounted for will be one.
*/

/********** macro definitions for routines **********/

#	ifdef __GNUC__
#		define sci_malloc(size)\
			_SCI_MALLOC(size, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#	else
#		define sci_malloc(size)\
			_SCI_MALLOC(size, __FILE__, __LINE__, "")
#	endif

#	ifdef __GNUC__
#		define sci_calloc(num, count)\
			_SCI_CALLOC(num, count, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#	else
#		define sci_calloc(num, count)\
			_SCI_CALLOC(num, count, __FILE__, __LINE__, "")
#	endif


#	ifdef __GNUC__
#		define sci_realloc(ptr, size)\
			_SCI_REALLOC(ptr, size, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#	else
#		define sci_realloc(ptr, size)\
			_SCI_REALLOC(ptr, size, __FILE__, __LINE__, "")
#	endif


#	ifdef __GNUC__
#		define sci_free(ptr)\
			_SCI_FREE(ptr, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#	else
#		define sci_free(ptr)\
			_SCI_FREE(ptr, __FILE__, __LINE__, "")
#	endif


#	ifdef __GNUC__
#		define sci_memdup(src, size)\
			_SCI_MEMDUP(src, size, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#	else
#		define sci_memdup(src, size)\
			_SCI_MEMDUP(src, size, __FILE__, __LINE__, "")
#	endif


#	ifdef __GNUC__
#		define sci_strdup(src)\
			_SCI_STRDUP(src, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#	else
#		define sci_strdup(src)\
			_SCI_STRDUP(src, __FILE__, __LINE__, "")
#	endif


#	ifdef __GNUC__
#		define sci_strndup(src, length)\
			_SCI_STRNDUP(src, length, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#	else
#		define sci_strndup(src, length)\
			_SCI_STRNDUP(src, length, __FILE__, __LINE__, "")
#	endif


/********** other memory/debug related routines **********/

#ifdef WIN32
extern void
	debug_win32_memory(int dbg_setting);
/* Sets debugging for Win32 memory.
** Parameters: (bool) dbg_setting:
**                      0: no debugging
**                      1: call _CrtCheckMemory at every memory request
**                      2: perform automatic leak checking at program exit
**                      3: enable debug heap allocations
*/
#endif


#endif	/* _SCI_MEMORY_H */
