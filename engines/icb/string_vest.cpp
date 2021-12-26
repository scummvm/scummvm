/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/mission.h"
#include "engines/icb/string_vest.h"

namespace ICB {

// stop initing these in multiple routines and use this instead
char temp_buf[ENGINE_STRING_LEN];

void Set_string_and_len(const char *from, char *to, uint32 *length) {
	// copy a string and set length flag

	if (strlen(from) >= ENGINE_STRING_LEN)
		Fatal_error("Set_string_and_len length violation [%s]", from);

	strcpy(to, from);

	*length = strlen(to);
}

void Set_string(const char *from, char *to) {
	// copy a string

	if (strlen(from) >= ENGINE_STRING_LEN)
		Fatal_error("Set_string length violation [%s]", from);

	strcpy(to, from);
}

void Set_string(const char *from, char *to, uint32 length) {
	// copy a string

	if (strlen(from) >= length)
		Fatal_error("Set_string length violation [%s] - max length = %d", from, length);

	strcpy(to, from);
}

} // End of namespace ICB
