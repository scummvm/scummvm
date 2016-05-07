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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_DIALOGUE_FILE_H
#define TITANIC_DIALOGUE_FILE_H

#include "common/file.h"
#include "titanic/support/string.h"

namespace Titanic {

struct DialogueFileIndexEntry {
	uint _v1, _offset;

	DialogueFileIndexEntry() : _v1(0), _offset(0) {}
	void load(Common::SeekableReadStream &s);
};

struct DialogueFileCacheEntry {
	bool _active;
	uint _offset, v3, _size;
	DialogueFileIndexEntry *_entryPtr;

	DialogueFileCacheEntry() : _active(false), _offset(0), _size(0),
		v3(0), _entryPtr(nullptr) {}
};

class CDialogueFile {
private:
	Common::File _file;
	Common::Array<DialogueFileIndexEntry> _entries;
	Common::Array<DialogueFileCacheEntry> _cache;
public:
	CDialogueFile(const CString &filename, uint count);
	~CDialogueFile();

	/**
	 * Clear the loaded data
	 */
	void clear();

	/**
	 * Add a dialogue file entry to the active cache
	 */
	DialogueFileCacheEntry *addToCache(int index);
};

} // End of namespace Titanic

#endif /* TITANIC_TITLE_ENGINE_H */
