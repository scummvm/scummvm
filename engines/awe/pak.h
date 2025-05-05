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

#ifndef AWE_PAK_H
#define AWE_PAK_H

#include "common/file.h"

#include "awe/intern.h"

namespace Awe {

struct PakEntry {
	char name[32] = { '\0' };
	uint32 offset = 0;
	uint32 size = 0;
};

struct Pak {
	static const char *FILENAME;

	Common::File _f;
	PakEntry *_entries = nullptr;
	int _entriesCount = 0;

	Pak();
	~Pak();

	void open(const char *dataPath);
	void close();

	void readEntries();
	const PakEntry *find(const char *name);
	void loadData(const PakEntry *e, uint8 *buf, uint32 *size);
};

} // namespace Awe

#endif
