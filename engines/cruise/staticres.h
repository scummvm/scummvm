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

#ifndef CRUISE_STATICRES_H
#define CRUISE_STATICRES_H

#include "common/scummsys.h"
#include "cruise/cruise.h"

namespace Cruise {

extern int actor_move[][13];

extern int actor_end[][13];

extern int actor_stat[][13];

extern int actor_invstat[][13];

extern short int english_fontCharacterTable[256];
extern short int german_fontCharacterTable[256];
extern short int spanish_fontCharacterTable[256];

#define fontCharacterTable (_vm->getLanguage() == Common::DE_DEU ? \
	german_fontCharacterTable : (_vm->getLanguage() == Common::ES_ESP ? \
	spanish_fontCharacterTable : english_fontCharacterTable))

// Mouse cursor data
extern const byte mouseCursorNormal[];
extern const byte mouseCursorDisk[];
extern const byte mouseCursorCross[];
extern const byte mouseCursorNoMouse[];
extern const byte mouseCursorWalk[];
extern const byte mouseCursorExit[];
extern const byte mouseCursorMagnifyingGlass[];

// Language strings
extern const char *englishLanguageStrings[13];
extern const char *frenchLanguageStrings[13];

} // End of namespace Cruise

#endif
