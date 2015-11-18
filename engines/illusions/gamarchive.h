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
 */

#ifndef ILLUSIONS_GAMARCHIVE_H
#define ILLUSIONS_GAMARCHIVE_H

#include "illusions/illusions.h"
#include "common/file.h"

namespace Illusions {

struct GamFileEntry {
	uint32 _id;
	uint32 _fileOffset;
	uint32 _fileSize;
};

struct GamGroupEntry {
	uint32 _id;
	uint _fileCount;
	GamFileEntry *_files;
	GamGroupEntry() : _fileCount(0), _files(0) {
	}
	~GamGroupEntry() {
		delete[] _files;
	}
};

class GamArchive {
public:
	GamArchive(const char *filename);
	~GamArchive();
	byte *readResource(uint32 sceneId, uint32 resId, uint32 &dataSize);
protected:
	Common::File *_fd;
	uint _groupCount;
	GamGroupEntry *_groups;
	void loadDictionary();
	const GamGroupEntry *getGroupEntry(uint32 sceneId);
	const GamFileEntry *getFileEntry(const GamGroupEntry *groupEntry, uint32 resId);
	const GamFileEntry *getGroupFileEntry(uint32 sceneId, uint32 resId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_GAMARCHIVE_H
