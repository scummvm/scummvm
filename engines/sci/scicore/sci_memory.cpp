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

#include "sci/include/sci_memory.h"
#include "common/util.h"

namespace Sci {

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

//-------- Refcounting ----------

#define REFCOUNT_OVERHEAD (sizeof(guint32) * 3)
#define REFCOUNT_MAGIC_LIVE_1 0xebdc1741
#define REFCOUNT_MAGIC_LIVE_2 0x17015ac9
#define REFCOUNT_MAGIC_DEAD_1 0x11dead11
#define REFCOUNT_MAGIC_DEAD_2 0x22dead22

#define REFCOUNT_CHECK(p) ((((guint32 *)(p))[-3] == REFCOUNT_MAGIC_LIVE_2) && (((guint32 *)(p))[-1] == REFCOUNT_MAGIC_LIVE_1))

#define REFCOUNT(p) (((guint32 *)p)[-2])

#undef TRACE_REFCOUNT

extern void *sci_refcount_alloc(size_t length) {
	guint32 *data = (guint32 *)sci_malloc(REFCOUNT_OVERHEAD + length);
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

extern void *sci_refcount_incref(void *data) {
	if (!REFCOUNT_CHECK(data)) {
		BREAKPOINT();
	} else
		REFCOUNT(data)++;

#ifdef TRACE_REFCOUNT
	fprintf(stderr, "[] REF: Inc'ing %p (now ref=%d)\n", data, REFCOUNT(data));
#endif
	return data;
}

extern void sci_refcount_decref(void *data) {
#ifdef TRACE_REFCOUNT
	fprintf(stderr, "[] REF: Dec'ing %p (prev ref=%d) OK=%d\n", data, REFCOUNT(data),
	        REFCOUNT_CHECK(data));
#endif
	if (!REFCOUNT_CHECK(data)) {
		BREAKPOINT();
	} else if (--REFCOUNT(data) == 0) {
		guint32 *fdata = (guint32 *)data;

		fdata[-1] = REFCOUNT_MAGIC_DEAD_1;
		fdata[-3] = REFCOUNT_MAGIC_DEAD_2;

#ifdef TRACE_REFCOUNT
		fprintf(stderr, "[] REF: Freeing (%p)...\n", fdata - 3);
#endif
		free(fdata - 3);
#ifdef TRACE_REFCOUNT
		fprintf(stderr, "[] REF: Done.\n");
#endif
	}
}

extern void *sci_refcount_memdup(void *data, size_t len) {
	void *dest = sci_refcount_alloc(len);
	memcpy(dest, data, len);
	return dest;
}

} // End of namespace Sci
