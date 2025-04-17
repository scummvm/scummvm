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

#ifndef AWE_RESOURCE_WIN31_H
#define AWE_RESOURCE_WIN31_H

#include "awe/intern.h"
#include "awe/file.h"

namespace Awe {

struct Win31BankEntry {
	char name[16];
	uint8_t type;
	uint32_t offset;
	uint32_t size;
	uint32_t packedSize;
};

struct ResourceWin31 {
	static const char *FILENAME;

	File _f;
	const char *_dataPath;
	Win31BankEntry *_entries;
	int _entriesCount;
	uint8_t *_textBuf;
	const char *_stringsTable[614];

	ResourceWin31(const char *dataPath);
	~ResourceWin31();

	bool readEntries();
	uint8_t *loadFile(int num, uint8_t *dst, uint32_t *size);
	void readStrings();
	const char *getString(int num) const;
	const char *getMusicName(int num) const;
};

} // namespace Awe

#endif
