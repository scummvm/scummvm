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

} // namespace AGS3
