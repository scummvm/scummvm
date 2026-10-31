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

#ifndef IGOR_RESOURCE_TYPES_H
#define IGOR_RESOURCE_TYPES_H

#include "common/language.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Igor {

struct DetectedGameVersion {
	int version;
	int flags;
	Common::Language language;
	const char *ovlFileName;
	const char *sfxFileName;
};

enum ResourceSource {
	kResourceSourceContainer, // the game's resource container (IGOR.DAT)
	kResourceSourceTable = 2 // data embedded in IGOR.TBL, offset is absolute in that file
};

struct ResourceEntry {
	int id;
	uint32 offs;
	uint32 size;
	ResourceSource source;
};

struct StringEntry {
	int id;
	Common::String str;

	StringEntry() : id(0) {}
	StringEntry(int i, const char *s) : id(i), str(s) {}
};

} // End of namespace Igor

#endif // IGOR_RESOURCE_TYPES_H
