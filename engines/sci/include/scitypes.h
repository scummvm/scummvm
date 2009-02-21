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

#ifndef SCI_TYPES
#define SCI_TYPES

#include "common/scummsys.h"

// TODO: rework sci_dir_t to use common/fs.h and remove these includes
#include <sys/types.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include <io.h>
#endif

namespace Sci {

typedef int8 gint8;
typedef uint8 guint8;

typedef int16 gint16;
typedef uint16 guint16;

typedef int32 gint32;
typedef uint32 guint32;

#undef byte

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

} // End of namespace Sci

#endif /* !SCI_TYPES */
