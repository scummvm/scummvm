/***************************************************************************
 resource.h Copyright (C) 1999,2000,01,02 Christoph Reichenbach


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

    Christoph Reichenbach (CR) [creichen@rbg.informatik.tu-darmstadt.de]

 History:

   990327 - created (CR)

***************************************************************************/

#ifndef FREESCI_PRIMARY_RESOURCE_H_
#define FREESCI_PRIMARY_RESOURCE_H_

/** This header file defines (mostly) generic tools and utility functions.
 ** It also handles portability stuff, in cooperation with scitypes.h
 ** (which specializes in primitive data types).
 ** Most implementations of the functions found here are in
 ** $(SRCDIR)/src/scicore/tools.c
 **
 ** -- Christoph Reichenbach
 **/

#define SCI_INVALID_FD -1
#define IS_VALID_FD(a) ((a) != SCI_INVALID_FD) /* Tests validity of a file descriptor */

#ifdef WIN32
#  ifndef _Win32
#    define _Win32
/* Work around problem with some versions of flex */
#  endif
#endif

/*#define _SCI_RESOURCE_DEBUG */
/*#define _SCI_DECOMPRESS_DEBUG*/
#ifndef WITH_DMALLOC
#  define SCI_SAFE_ALLOC /* Undefine for debugging */
#endif

#ifdef SCUMMVM
//TODO: Remove these defines by replacing their functionality by their ScummVM counterparts
#define HAVE_MEMCHR
#define HAVE_UNLINK
#define HAVE_RMDIR
#define HAVE_FCNTL_H
#ifndef _MSC_VER
#define HAVE_UNISTD_H
#define HAVE_SYS_TIME_H
#define HAVE_GETTIMEOFDAY
#endif
#define VERSION "0.6.4"
#endif // SCUMMVM

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef __unix__
#  if defined(__OpenBSD__) || defined(__NetBSD__)
#    define __unix__
#  endif
#endif

#if defined(HAVE_FORK) && !defined(__BEOS__)
#  define HAVE_SYSV_IPC
#else
#  undef HAVE_SYSV_IPC
#endif

#ifdef __DECC
#  include <c_asm.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef WITH_DMALLOC
#  include <dmalloc.h>
#endif

#include "sci/include/scitypes.h"

// FIXME: Mostly for close() in lots of places. Get rid of this!
#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef _MSC_VER
#	undef strcasecmp
#	undef strncasecmp
#	define strcasecmp _stricmp
#	define strncasecmp _strnicmp
#endif


#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif
#include <assert.h>
#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif

// FIXME: Get rid of G_DIR_SEPARATOR  / G_DIR_SEPARATOR_S
#if _MSC_VER
#  define G_DIR_SEPARATOR_S "\\"
#  define G_DIR_SEPARATOR '\\'
#else
#  define G_DIR_SEPARATOR_S "/"
#  define G_DIR_SEPARATOR '/'
#endif

// FIXME: Get rid of MIN / MAX, use common/util.h instead
#ifndef MIN
#  define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#  define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif /* HP-UX defines both */


#define GUINT16_SWAP_LE_BE_CONSTANT(val) ((((val) & 0x00ff) << 8) | (((val) & 0xff00) >> 8))

#define GUINT32_SWAP_LE_BE_CONSTANT(val)  ( \
                                             (((val) & 0xff000000) >> 24) \
                                           | (((val) & 0x00ff0000) >> 8) \
                                           | (((val) & 0x0000ff00) << 8) \
                                           | (((val) & 0x000000ff) << 24))

#define SCI_MAX_RESOURCE_SIZE 0x0400000
/* The maximum allowed size for a compressed or decompressed resource */



#define MAX_HOMEDIR_SIZE 255

#ifdef WIN32
#  define FO_BINARY "b"
#else
#  define FO_BINARY ""
#endif

#ifdef WIN32
#  define FO_TEXT "t"
#else
#  define FO_TEXT ""
#endif

#ifndef O_BINARY
#  define O_BINARY 0
#endif

/**** FUNCTION DECLARATIONS ****/

#ifdef WIN32
#    define scimkdir(arg1,arg2) mkdir(arg1)
#else
#    define scimkdir(arg1,arg2) mkdir(arg1,arg2)
#endif

#ifndef _GET_INT_16
#define _GET_INT_16

static inline gint16
getInt16(byte *d)
{
	return (gint16)(*d | (d[1] << 8));
}

#define getUInt16(d) (guint16)(getInt16(d))
#endif
/* Turns a little endian 16 bit value into a machine-dependant 16 bit value
** Parameters: d: Pointer to the memory position from which to read
** Returns   : (gint16) The (possibly converted) 16 bit value
** getUInt16 returns the int unsigned.
*/

static inline void
putInt16(byte* dest, int src)
{
	dest[0]=(byte)src&0xff;
	dest[1]=(byte)(src>>8)&0xff;
}
/* Converse of getInt16()
** Parameters: (byte *) dest: The position to write to
**             (int) src: value to write
*/

#define SCI_MEMTEST memtest(__FILE__, __LINE__)

/*-- queues --*/

typedef struct _sci_queue_node {
	void *data;
	int type;
	struct _sci_queue_node *next;
} sci_queue_node_t;

typedef struct {
	sci_queue_node_t *start; /* Where things go in */
	sci_queue_node_t *end;   /* Where they come out */
} sci_queue_t;

sci_queue_t *
sci_init_queue(sci_queue_t *queue);
/* Initializes an SCI queue
** Parameters: (sci_queue_t *) queue: Pointer to the queue to initialize
** Returns   : (sci_queue_t *) queue
*/

sci_queue_t *
sci_add_to_queue(sci_queue_t *queue, void *data, int type);
/* Adds an entry to an initialized SCI queue
** Parameters: (sci_queue_t *) queue: The queue to add to
**             (void *) data: The data to contain
**             (int) type: Some metadata, e.g. size of the data block
** Returns   : (sci_queue_t *) queue
** The meaning of the 'type' field may be determined individually.
*/

void *
sci_get_from_queue(sci_queue_t *queue, int *type);
/* Reads an entry from an SCI queue
** Parameters: (sci_queue_t *) queue: The queue to remove from
**             (int *) type: A pointer to a variable to write the metadata
**                     into, or NULL to omit this step
** Returns   : (void *) The data block contained, or NULL if none was inside
*/

/* --- */

int
memtest(const char *file, int line);
/* Allocates, manipulates, and frees some memory
** Parameters: (const char *) file: The file name to print when starting the
**				tests
**             (int) line: The line number to claim it was executed on
** Returns   : (int) 0
** This function calls malloc(), free(), and memfrob() or memset()
** to provocate segmentation faults caused by dynamic allocation bugs
** in previous parts of the code.
*/

void
sci_gettime(long *seconds, long *useconds);
/* Calculates the current time in seconds and microseconds
** Parameters: (long *) seconds: Pointer to the variable the seconds part of the
**                               current time will be stored in
**             (long *) useconds: Pointer to the variable the microseconds part
**                                of the current time will be stored in
** Returns   : (void)
** The resulting values must be relative to an arbitrary fixed point in time
** (typically 01/01/1970 on *NIX systems).
*/

void
sci_get_current_time(GTimeVal *val);
/* GTimeVal version of sci_gettime()
** Parameters: (GTimeVal *) val: Pointer to the structure the values will be stored in
** Returns   : (void)
*/

void
sci_init_dir(sci_dir_t *dirent);
/* Initializes an sci directory search structure
** Parameters: (sci_dir_t *) dirent: The entity to initialize
** Returns   : (void)
** The entity is initialized to "empty" values, meaning that it can be
** used in subsequent sci_find_first/sci_find_next constructs. In no
** event should this function be used upon a structure which has been
** subjected to any of the other dirent calls.
*/

char *
sci_find_first(sci_dir_t *dirent, const char *mask);
/* Finds the first file matching the specified file mask
** Parameters: (sci_dir_t *) dirent: Pointer to an unused dirent structure
**             (const char *) mask: File mask to apply
** Returns   : (char *) Name of the first matching file found, or NULL
*/

char *
sci_find_next(sci_dir_t *dirent);
/* Finds the next file specified by an sci_dir initialized by sci_find_first()
** Parameters: (sci_dir_t *) dirent: Pointer to SCI dir entity
** Returns   : (char *) Name of the next matching file, or NULL
*/

void
sci_finish_find(sci_dir_t *dirent);
/* Completes an 'sci_find_first/next' procedure
** Parameters: (sci_dir_t *) dirent: Pointer to the dirent used
** Returns   : (void)
** In the operation sequences
**   sci_init_dir(x); sci_finish_find(x);
** and
**   sci_finish_find(x); sci_finish_find(x);
** the second operation is guaranteed to be a no-op.
*/

FILE *
sci_fopen(const char *fname, const char *mode);
/* Opens a FILE* case-insensitively
** Parameters: (const char *) fname: Name of the file to open
**             (const char *) mode: Mode to open it with
** Returns   : (FILE *) A valid file handle, or NULL on failure
** Always refers to the cwd, cannot address subdirectories
*/

int
sci_open(const char *fname, int flags);
/* Opens a file descriptor case-insensitively
** Parameters: (const char *) fname: Name of the file to open
**             (int) flags: open(2) flags for the file
** Returns   : (int) a file descriptor of the open file,
**             or SCI_INVALID_FD on failure
** Always refers to the cwd, cannot address subdirectories
*/


int
sciprintf(const char *fmt, ...);
#define gfxprintf sciprintf
/* Prints a string to the console stack
** Parameters: fmt: a printf-style format string
**             ...: Additional parameters as defined in fmt
** Returns   : (int) 1
** Implementation is in src/scicore/console.c
*/

char *
sci_getcwd(void);
/* Returns the current working directory, malloc'd.
** Parameters: (void)
** Returns   : (char *) a malloc'd cwd, or NULL if it couldn't be determined.
*/


char *
sci_get_homedir(void);
/* Returns the user's home directory
** Parameters: (void)
** Returns   : (char *) Pointer to a static buffer containing the user's home,
**                      or NULL if there is no such thing.
*/

int
sci_mkpath(const char *path);
/* Asserts that the specified path is available
** Parameters: (const char *) path: Path to verify/create
** Returns   : (int) 0 on success, <0 on error
** This function will create any directories that couldn't be found
*/

int
sci_fd_size(int fd);
/* Returns the filesize of an open file
** Parameters: (int) fd: File descriptor of open file
** Returns   : (int) filesize of file pointed to by fd, -1 on error
*/ 

int
sci_file_size(const char *fname);
/* Returns the filesize of a file
** Parameters: (const char *) fname: Name of file to get filesize of
** Returns   : (int) filesize of the file, -1 on error
*/

char *
_fcaseseek(const char *fname, sci_dir_t *dir);
/* Returns the case-sensitive filename of a file.
** Expects *dir to be uninitialized and the caller to free it afterwards.
** Parameters: (const char *) fname: Name of file to get case-sensitive.
**             (sci_dir_t *) dir: Directory to find file within.
** Returns   : (char *) Case-sensitive filename of the file.
*/

/* Simple heuristic to work around array handling peculiarity in SQ4:
It uses StrAt() to read the individual elements, so we must determine
whether a string is really a string or an array. */
int is_print_str(char *str);

#ifdef HAVE_UNLINK
#  define sci_unlink unlink
#else  /* !HAVE_UNLINK */
#  error "Please provide an int sci_unlink(const char *) for removing filesystem entries"
#endif /* !HAVE_UNLINK */

#ifdef HAVE_RMDIR
#  define sci_rmdir rmdir
#else  /* !HAVE_RMDIR */
#  ifdef WIN32
#    define sci_rmdir _rmdir
#  else
#    error "Please provide an int sci_rmdir(const char *) for removing directories"
#  endif
#endif /* !HAVE_RMDIR */

#ifndef HAVE_FFS
int sci_ffs(int _mask);
#else
#define sci_ffs ffs
#endif


void
sci_sched_yield(void);
/* Yields the running process/thread to the scheduler
** Parameters: (void)
** Returns   : after a while.
*/

/* The following was originally based on glib.h code, which was
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 */
#if defined (__GNUC__) && __GNUC__ >= 2
#  if defined (__i386__)
#    define BREAKPOINT()          {__asm__ __volatile__ ("int $03"); }
#  elif defined(__alpha__)
#    define BREAKPOINT()          {__asm__ __volatile__ ("call_pal 0x80"); }
#  endif /* !__i386__ && !__alpha__ */
#elif defined (_MSC_VER)
#  if defined (_M_IX86)
#    define BREAKPOINT()          { __asm { int 03 } }
#  elif defined(_M_ALPHA)
#    define BREAKPOINT()          { __asm { bpt } }
#  endif /* !_M_IX86 && !_M_ALPHA */
#elif defined (__DECC)
#  if defined(__alpha__)
#    define BREAKPOINT()          {asm ("call_pal 0x80"); }
#  endif /* !__i386__ && !__alpha__ */
#endif
#ifndef BREAKPOINT
#  define BREAKPOINT() { fprintf(stderr, "Missed breakpoint in %s, line %d\n", __FILE__, __LINE__); *((int *) NULL) = 42; }
#endif  /* !BREAKPOINT() */

#define WARNING(foo) {char i; i = 500;}

#endif
