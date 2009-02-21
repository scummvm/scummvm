/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_TOOLS_H
#define SCI_TOOLS_H

#include "common/scummsys.h"
#include "common/endian.h"

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

/*#define _SCI_RESOURCE_DEBUG */
/*#define _SCI_DECOMPRESS_DEBUG*/

//TODO: Remove these defines by replacing their functionality by their ScummVM counterparts
#define HAVE_MEMCHR
#define HAVE_FCNTL_H
#ifndef _MSC_VER
#define HAVE_UNISTD_H
#define HAVE_SYS_TIME_H
#define HAVE_GETTIMEOFDAY
#endif
#define VERSION "0.6.4"

// FIXME: Mostly for close() in lots of places. Get rid of this!
#ifndef _MSC_VER
#include <unistd.h>
#endif

// FIXME: Get rid of the following (needed for O_RDONLY etc.)
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif


// FIXME: rework sci_dir_t to use common/fs.h and remove these includes
#include <sys/types.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include <io.h>
#endif


#ifdef WIN32
#  define FO_BINARY "b"
#else
#  define FO_BINARY ""
#endif

namespace Sci {


typedef struct {
	long tv_sec;
	long tv_usec;
} GTimeVal;

typedef struct {
#ifdef WIN32
	long search;
	struct _finddata_t fileinfo;
#else
	DIR *dir;
	char *mask_copy;
#endif
} sci_dir_t; /* used by sci_find_first and friends */



/**** FUNCTION DECLARATIONS ****/

#define getInt16	(int16)READ_LE_UINT16
#define getUInt16	READ_LE_UINT16
#define putInt16	WRITE_LE_UINT16


/* --- */

void sci_gettime(long *seconds, long *useconds);
/* Calculates the current time in seconds and microseconds
** Parameters: (long *) seconds: Pointer to the variable the seconds part of the
**                               current time will be stored in
**             (long *) useconds: Pointer to the variable the microseconds part
**                                of the current time will be stored in
** Returns   : (void)
** The resulting values must be relative to an arbitrary fixed point in time
** (typically 01/01/1970 on *NIX systems).
*/

void sci_get_current_time(GTimeVal *val);
/* GTimeVal version of sci_gettime()
** Parameters: (GTimeVal *) val: Pointer to the structure the values will be stored in
** Returns   : (void)
*/

void sci_init_dir(sci_dir_t *dirent);
/* Initializes an sci directory search structure
** Parameters: (sci_dir_t *) dirent: The entity to initialize
** Returns   : (void)
** The entity is initialized to "empty" values, meaning that it can be
** used in subsequent sci_find_first/sci_find_next constructs. In no
** event should this function be used upon a structure which has been
** subjected to any of the other dirent calls.
*/

char *sci_find_first(sci_dir_t *dirent, const char *mask);
/* Finds the first file matching the specified file mask
** Parameters: (sci_dir_t *) dirent: Pointer to an unused dirent structure
**             (const char *) mask: File mask to apply
** Returns   : (char *) Name of the first matching file found, or NULL
*/

char *sci_find_next(sci_dir_t *dirent);
/* Finds the next file specified by an sci_dir initialized by sci_find_first()
** Parameters: (sci_dir_t *) dirent: Pointer to SCI dir entity
** Returns   : (char *) Name of the next matching file, or NULL
*/

void sci_finish_find(sci_dir_t *dirent);
/* Completes an 'sci_find_first/next' procedure
** Parameters: (sci_dir_t *) dirent: Pointer to the dirent used
** Returns   : (void)
** In the operation sequences
**   sci_init_dir(x); sci_finish_find(x);
** and
**   sci_finish_find(x); sci_finish_find(x);
** the second operation is guaranteed to be a no-op.
*/

FILE *sci_fopen(const char *fname, const char *mode);
/* Opens a FILE* case-insensitively
** Parameters: (const char *) fname: Name of the file to open
**             (const char *) mode: Mode to open it with
** Returns   : (FILE *) A valid file handle, or NULL on failure
** Always refers to the cwd, cannot address subdirectories
*/

int sci_open(const char *fname, int flags);
/* Opens a file descriptor case-insensitively
** Parameters: (const char *) fname: Name of the file to open
**             (int) flags: open(2) flags for the file
** Returns   : (int) a file descriptor of the open file,
**             or SCI_INVALID_FD on failure
** Always refers to the cwd, cannot address subdirectories
*/


int sciprintf(const char *fmt, ...) GCC_PRINTF(1, 2);
#define gfxprintf sciprintf
/* Prints a string to the console stack
** Parameters: fmt: a printf-style format string
**             ...: Additional parameters as defined in fmt
** Returns   : (int) 1
** Implementation is in src/scicore/console.c
*/

char *sci_getcwd();
/* Returns the current working directory, malloc'd.
** Parameters: (void)
** Returns   : (char *) a malloc'd cwd, or NULL if it couldn't be determined.
*/

int sci_fd_size(int fd);
/* Returns the filesize of an open file
** Parameters: (int) fd: File descriptor of open file
** Returns   : (int) filesize of file pointed to by fd, -1 on error
*/

int sci_file_size(const char *fname);
/* Returns the filesize of a file
** Parameters: (const char *) fname: Name of file to get filesize of
** Returns   : (int) filesize of the file, -1 on error
*/

/* Simple heuristic to work around array handling peculiarity in SQ4:
It uses StrAt() to read the individual elements, so we must determine
whether a string is really a string or an array. */
int is_print_str(char *str);

/** Find first set bit in bits and return its index. Returns 0 if bits is 0. */
int sci_ffs(int bits);


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

} // End of namespace Sci

#endif
