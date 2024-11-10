/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/shared/util/string_compat.h"
#include "ags/shared/core/platform.h"
#include "ags/lib/allegro/error.h"
#include "common/str.h"

namespace AGS3 {

char *ags_strlwr(char *s) {
	char *p = s;
	for (; *p; p++)
		*p = (char)tolower(*p);
	return s;
}

char *ags_strupr(char *s) {
	char *p = s;
	for (; *p; p++)
		*p = (char)toupper(*p);
	return s;
}

int ags_stricmp(const char *s1, const char *s2) {
	return scumm_stricmp(s1, s2);
}

int ags_strnicmp(const char *s1, const char *s2, size_t n) {
	return scumm_strnicmp(s1, s2, n);
}

char *ags_strdup(const char *s) {
	size_t len = strlen(s);
	char *result = (char *)malloc(len + 1);
	memcpy(result, s, len + 1);
	return result;
}

int ags_strncpy_s(char *dest, size_t dest_sz, const char *src, size_t count) {
	// NOTE: implementation approximately mimics explanation for "strncpy_s":
	// https://en.cppreference.com/w/c/string/byte/strncpy
	assert(dest && dest_sz > 0 && ((dest + dest_sz - 1 < src) || (dest > src + count)));
	if (!dest || dest_sz == 0 || ((dest <= src) && (dest + dest_sz - 1 >= src)) || ((src <= dest) && (src + count - 1 >= dest)))
		return AL_EINVAL; // null buffer, or dest and src overlap
	if (!src) {
		dest[0] = 0; // ensure null terminator
		return AL_EINVAL;
	}

	const size_t copy_len = (count < dest_sz - 1) ? count : dest_sz - 1; // reserve null-terminator
	const char *psrc = src;
	const char *src_end = src + copy_len;
	char *pdst = dest;
	for (; *psrc && (psrc != src_end); ++psrc, ++pdst)
		*pdst = *psrc;
	*pdst = 0; // ensure null terminator
	assert((*psrc == 0) || ((psrc - src) == (int)count)); // assert that no *unintended* truncation occured
	if ((*psrc != 0) && ((psrc - src) < (int)count))
		return AL_ERANGE; // not enough dest buffer - error
	return 0; // success
}

} // namespace AGS3
