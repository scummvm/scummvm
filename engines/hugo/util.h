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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_UTIL_H
#define HUGO_UTIL_H

namespace Hugo {

enum seqTextUtil {
	kGameOver = 0
};

namespace Utils {

int   firstBit(byte data);
int   lastBit(byte data);

void  reverseByte(byte *data);

/**
 * Show a dialog notifying the user about something, with
 * only a simple "OK" button to dismiss it.
 */
void notifyBox(const Common::String &msg);	// Redirect to call notifyBox with u32strings
void notifyBox(const Common::U32String &msg);

/**
 * Show a dialog prompting the player to input some text.
 */
Common::String promptBox(const Common::String &msg);

/**
 * Show a dialog prompting the player for a "yes"/"no" choice.
 */
bool yesNoBox(const Common::String &msg);	// Redirect to call yesNoBox with u32strings
bool yesNoBox(const Common::U32String &msg);

/**
 * Convert a string to lower case, in place.
 * @param buffer	string to convert to lower case
 * @return the string which was passed in
 */
char *hugo_strlwr(char *buffer);

} // End of namespace Utils

} // End of namespace Hugo

#endif
