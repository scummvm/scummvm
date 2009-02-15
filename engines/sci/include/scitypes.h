/***************************************************************************
 scitypes.h Copyright (C) 2001 Christoph Reichenbach

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

***************************************************************************/

#ifndef SCI_TYPES
#define SCI_TYPES

#ifdef SCUMMVM

#include "common/scummsys.h"

// TODO: rework sci_dir_t to use common/fs.h and remove these includes
#include <sys/types.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include <io.h>
#	undef inline /* just to be sure it is not defined */
#	define inline __inline
#endif

typedef int8 gint8;
typedef uint8 guint8;

typedef int16 gint16;
typedef uint16 guint16;

typedef int32 gint32;
typedef uint32 guint32;

#undef byte

#else // SCUMMVM

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_DIRENT_H
#  include <sys/types.h>
#  include <dirent.h>
#endif
#ifdef WIN32
#  include <io.h>
#  include <sci_win32.h>
#endif
#ifdef __DC__
#  include <stdio.h>
#endif

#if defined(WIN32) && defined(_MSC_VER)
#  define TYPE_16 short
#  define TYPE_32 int
#endif

#ifndef TYPE_8
#  define TYPE_8 char /* Guaranteed by ISO */
#endif

#ifndef TYPE_16
#  if (SIZEOF_SHORT == 2)
#    define TYPE_16 short
#  elif (SIZEOF_INT == 2)
#    define TYPE_16 int
#  else
#    error "Could not find a 16 bit data type!"
#  endif
#endif /* !TYPE_16 */

#ifndef TYPE_32
#  if (SIZEOF_INT == 4)
#    define TYPE_32 int
#  elif (SIZEOF_LONG == 4)
#    define TYPE_32 long
#  else
#    error "Could not find a 32 bit data type!"
#  endif
#endif /* !TYPE_32 */

typedef signed TYPE_8 gint8;
typedef unsigned TYPE_8 guint8;

typedef signed TYPE_16 gint16;
typedef unsigned TYPE_16 guint16;

typedef signed TYPE_32 gint32;
typedef unsigned TYPE_32 guint32;

#undef TYPE_8
#undef TYPE_16
#undef TYPE_32

#endif // SCUMMVM

typedef gint8 sbyte;
typedef guint8 byte;
typedef guint16 word;

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

/*
 * Fixed point type, borrowed from ScummVM.
 * The precision of the fractional (fixed point) type we define below.
 * Normally you should never have to modify this value.
 */
enum {
	FRAC_BITS = 16,
	FRAC_LO_MASK = ((1L << FRAC_BITS) - 1),
	FRAC_HI_MASK = (((1L << (32 - FRAC_BITS)) - 1) << FRAC_BITS),

	FRAC_ONE = (1L << FRAC_BITS),		// 1.0
	FRAC_HALF = (1L << (FRAC_BITS - 1))	// 0.5
};

/*
 * Fixed-point fractions, used by the sound rate converter and other code.
 */
typedef gint32 frac_t;

static inline frac_t double_to_frac(double value) { return (frac_t)(value * FRAC_ONE); }
static inline double frac_to_double(frac_t value) { return ((double)value) / FRAC_ONE; }

static inline frac_t int_to_frac(gint32 value) { return value << FRAC_BITS; }
static inline gint32 frac_to_int(frac_t value) { return value >> FRAC_BITS; }

#endif /* !SCI_TYPES */
