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


// This header file defines a portable library for allocating memory safely.


#ifndef SCI_SCI_MEMORY_H
#define SCI_SCI_MEMORY_H

#include "common/scummsys.h"

namespace Sci {

/********** memory allocation routines **********/

extern void *sci_malloc(size_t size);
/* Allocates the specified amount of memory.
** Parameters: (size_t) size: Number of bytes to allocate
** Returns   : (void *) A pointer to the allocated memory chunk
** To free this string, use the free() command.
** If the call fails, behaviour is dependent on the definition of SCI_ALLOC.
*/

extern void *sci_calloc(size_t num, size_t size);
/* Allocates num * size bytes of zeroed-out memory.
** Parameters: (size_t) num: Number of elements to allocate
**             (size_t) size: Amount of memory per element to allocate
** Returns   : (void *) A pointer to the allocated memory chunk
** To free this string, use the free() command.
** See _SCI_MALLOC() for more information if call fails.
*/

extern void *sci_realloc(void *ptr, size_t size);
/* Increases the size of an allocated memory chunk.
** Parameters: (void *) ptr: The original pointer
**             (size_t) size: New size of the memory chunk
** Returns   : (void *) A possibly new pointer, containing 'size'
**             bytes of memory and everything contained in the original 'ptr'
**             (possibly minus some trailing data if the new memory area is
**             smaller than the old one).
** To free this string, use the free() command.
** See _SCI_MALLOC() for more information if call fails.
*/


extern char *sci_strdup(const char *src);
/* Duplicates a string.
** Parameters: (const char *) src: The original pointer
** Returns   : (char *) a pointer to the storage location for the copied
**             string.
** To free this string, use the free() command.
** See _SCI_MALLOC() for more information if call fails.
*/


extern char *sci_strndup(const char *src, size_t length);
/* Copies a string into a newly allocated memory part, up to a certain length.
** Parameters: (char *) src: The source string
**             (int) length: The maximum length of the string (not counting
**                           a trailing \0).
** Returns   : (char *) The resulting copy, allocated with sci_malloc().
** To free this string, use the free() command.
** See _SCI_MALLOC() for more information if call fails.
*/

} // End of namespace Sci

#endif	// SCI_SCI_MEMORY_H
