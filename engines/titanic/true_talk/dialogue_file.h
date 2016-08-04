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

#include "titanic/support/simple_file.h"
#include "titanic/support/string.h"

namespace Titanic {

struct DialogueIndexEntry {
	uint _v1, _offset;

	DialogueIndexEntry() : _v1(0), _offset(0) {}
	void load(Common::SeekableReadStream &s);
};

struct DialogueResource {
	bool _active;
	uint _offset, _bytesRead, _size;
	DialogueIndexEntry *_entryPtr;

	DialogueResource() : _active(false), _offset(0),
		_bytesRead(0), _size(0), _entryPtr(nullptr) {}

	/**
	 * Return the size of a cache entry
	 */
	size_t size() const { return _active ? _size : 0; }
};

class CDialogueFile {
private:
	File _file;
	Common::Array<DialogueIndexEntry> _index;
	Common::Array<DialogueResource> _cache;
private:
	/**
	 * Add a dialogue file entry to the active cache
	 */
	DialogueResource *addToCache(int index);
public:
	CDialogueFile(const CString &filename, uint count);
	~CDialogueFile();

	/**
	 * Clear the loaded data
	 */
	void clear();

	File *getFile() { return &_file; }

	/**
	 * Sets up a text entry within the dialogue file for access
	 */
	DialogueResource *openTextEntry(int index) {
		return addToCache(index * 2);
	}

	/**
	 * Sets up a wave (sound) entry within the dialogue file for access
	 */
	DialogueResource *openWaveEntry(int index) {
		return addToCache(index * 2 + 1);
	}

	/**
	 * Removes an entry from the cache
	 */
	bool closeEntry(DialogueResource *cacheEntry);

	/**
	 * Read data for a resource
	 */
	bool read(DialogueResource *cacheEntry, byte *buffer, size_t bytesToRead);
};

} // End of namespace Titanic

#endif /* TITANIC_TITLE_ENGINE_H */
