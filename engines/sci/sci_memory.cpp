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

#include "common/util.h"
#include "sci/tools.h"
#include "sci/sci_memory.h"

namespace Sci {

/********** the memory allocation macros **********/

#ifdef UNCHECKED_MALLOCS

#define ALLOC_MEM(alloc_statement, size, filename, linenum, funcname)\
do {\
	alloc_statement;\
} while (0);

#else /* !UNCHECKED_MALLOCS */

#define ALLOC_MEM(alloc_statement, size, filename, linenum, funcname)\
do {\
	if (size == 0) {\
		warning("Allocating zero bytes of memory [%s (%s) : %u]", filename, funcname, linenum);\
	} else if (!(size > 0)) {\
		error("Cannot allocate negative bytes of memory [%s (%s) : %u]", filename, funcname, linenum);\
	}\
\
	alloc_statement; /* attempt to allocate the memory */\
\
	if (res == NULL) {\
		/* exit immediately */\
		error("Memory allocation of %u bytes failed [%s (%s) : %u]", (uint32)size, filename, funcname, linenum);\
	}\
} while (0);

#endif /* !UNCHECKED_MALLOCS */


void * sci_malloc(size_t size) {
	void *res;

	ALLOC_MEM((res = malloc(size)), size, __FILE__, __LINE__, "")
	return res;
}


void * sci_calloc(size_t num, size_t size) {
	void *res;

	ALLOC_MEM((res = calloc(num, size)), num * size, __FILE__, __LINE__, "")
	return res;
}

void *sci_realloc(void *ptr, size_t size) {
	void *res;

	ALLOC_MEM((res = realloc(ptr, size)), size, __FILE__, __LINE__, "")
	return res;
}

char *sci_strdup(const char *src) {
	void *res;

	if (!src) {
		fprintf(stderr, "_SCI_STRDUP() [%s (%s) : %u]\n",
		        __FILE__, "", __LINE__);
		fprintf(stderr, " attempt to strdup NULL pointer\n");
		BREAKPOINT();
	}
	ALLOC_MEM((res = strdup(src)), strlen(src), __FILE__, __LINE__, "")

	return (char*)res;
}

char *sci_strndup(const char *src, size_t length) {
	void *res;
	char *strres;
	size_t rlen = (int)MIN(strlen(src), length) + 1;

	if (!src) {
		fprintf(stderr, "_SCI_STRNDUP() [%s (%s) : %u]\n",
		        __FILE__, "", __LINE__);
		fprintf(stderr, " attempt to strndup NULL pointer\n");
		BREAKPOINT();
	}
	ALLOC_MEM((res = malloc(rlen)), rlen, __FILE__, __LINE__, "")

	strres = (char *)res;
	strncpy(strres, src, rlen);
	strres[rlen - 1] = 0;

	return strres;
}

} // End of namespace Sci
