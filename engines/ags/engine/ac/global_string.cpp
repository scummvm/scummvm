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

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/global_string.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/util/string_compat.h"
#include "ags/globals.h"

namespace AGS3 {

int StrGetCharAt(const char *strin, int posn) {
	if ((posn < 0) || (static_cast<size_t>(posn) >= strlen(strin)))
		return 0;
	return strin[posn];
}

void StrSetCharAt(char *strin, int posn, int nchar) {
	size_t len = strlen(strin);
	if ((posn < 0) || (static_cast<size_t>(posn) > len) || (posn >= MAX_MAXSTRLEN))
		quit("!StrSetCharAt: tried to write past end of string");

	strin[posn] = static_cast<char>(nchar);
	if (static_cast<size_t>(posn) == len)
		strin[posn + 1] = 0;
	commit_scstr_update(strin);
}

void _sc_strcat(char *s1, const char *s2) {
	VALIDATE_STRING(s2);
	size_t buflen = check_scstrcapacity(s1);
	size_t s1_len = strlen(s1);
	size_t buf_avail = (buflen - s1_len);
	snprintf(s1 + s1_len, buf_avail, "%s", s2);
	commit_scstr_update(s1);
}

void _sc_strlower(char *desbuf) {
	VALIDATE_STRING(desbuf);
	ags_strlwr(desbuf);
	commit_scstr_update(desbuf);
}

void _sc_strupper(char *desbuf) {
	VALIDATE_STRING(desbuf);
	ags_strupr(desbuf);
	commit_scstr_update(desbuf);
}

void _sc_strcpy(char *destt, const char *text) {
	VALIDATE_STRING(destt);
	size_t buflen = check_scstrcapacity(destt);
	snprintf(destt, buflen, "%s", text);
	commit_scstr_update(destt);
}

} // namespace AGS3
