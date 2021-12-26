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
	if ((posn < 0) || (posn >= (int)strlen(strin)))
		return 0;
	return strin[posn];
}

void StrSetCharAt(char *strin, int posn, int nchar) {
	if ((posn < 0) || (posn > (int)strlen(strin)) || (posn >= MAX_MAXSTRLEN))
		quit("!StrSetCharAt: tried to write past end of string");

	if (posn == (int)strlen(strin))
		strin[posn + 1] = 0;
	strin[posn] = nchar;
}

void _sc_strcat(char *s1, const char *s2) {
	// make sure they don't try to append a char to the string
	VALIDATE_STRING(s2);
	check_strlen(s1);
	int mosttocopy = (_G(MAXSTRLEN) - strlen(s1)) - 1;
	//  int numbf=_GP(game).iface[4].numbuttons;
	my_strncpy(&s1[strlen(s1)], s2, mosttocopy);
}

void _sc_strlower(char *desbuf) {
	VALIDATE_STRING(desbuf);
	check_strlen(desbuf);
	ags_strlwr(desbuf);
}

void _sc_strupper(char *desbuf) {
	VALIDATE_STRING(desbuf);
	check_strlen(desbuf);
	ags_strupr(desbuf);
}

/*int _sc_strcmp (char *s1, char *s2) {
return strcmp (get_translation (s1), get_translation(s2));
}

int _sc_stricmp (char *s1, char *s2) {
return ags_stricmp (get_translation (s1), get_translation(s2));
}*/

void _sc_strcpy(char *destt, const char *text) {
	VALIDATE_STRING(destt);
	check_strlen(destt);
	my_strncpy(destt, text, _G(MAXSTRLEN) - 1);
}

} // namespace AGS3
