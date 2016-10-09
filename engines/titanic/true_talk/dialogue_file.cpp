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

#include "titanic/true_talk/dialogue_file.h"

namespace Titanic {

void DialogueIndexEntry::load(Common::SeekableReadStream &s) {
	_v1 = s.readUint32LE();
	_offset = s.readUint32LE();
}

/*------------------------------------------------------------------------*/

CDialogueFile::CDialogueFile(const CString &filename, uint count) {
	if (!_file.open(filename))
		error("Could not locate dialogue file - %s", filename.c_str());

	_cache.resize(count);

	_file.readUint32LE();		// Skip over file Id
	_index.resize(_file.readUint32LE());

	// Read in the entries
	for (uint idx = 0; idx < _index.size(); ++idx)
		_index[idx].load(_file);
}

CDialogueFile::~CDialogueFile() {
	clear();
}

void CDialogueFile::clear() {
	_file.close();
}

DialogueResource *CDialogueFile::addToCache(int index) {
	if (_index.size() == 0 || index < 0 || index >= (int)_index.size()
			|| _cache.empty())
		return nullptr;

	// Scan cache for a free slot
	uint cacheIndex = 0;
	while (cacheIndex < _cache.size() && _cache[cacheIndex]._active)
		++cacheIndex;
	if (cacheIndex == _cache.size())
		return nullptr;

	DialogueIndexEntry &indexEntry = _index[index];
	DialogueResource &res = _cache[cacheIndex];

	res._active = true;
	res._offset = indexEntry._offset;
	res._bytesRead = 0;
	res._entryPtr = &indexEntry;

	// Figure out the size of the entry
	if (index == ((int)_index.size() - 1)) {
		res._size = _file.size() - indexEntry._offset;
	} else {
		res._size = _index[index + 1]._offset - indexEntry._offset;
	}

	// Return a pointer to the loaded entry
	return &res;
}

bool CDialogueFile::closeEntry(DialogueResource *cacheEntry) {
	if (!cacheEntry || !cacheEntry->_active)
		return false;

	cacheEntry->_active = false;
	return true;
}

bool CDialogueFile::read(DialogueResource *cacheEntry, byte *buffer, size_t bytesToRead) {
	// Sanity checks that a valid record is passed, and the size can be read
	if (!cacheEntry || !cacheEntry->_active || !bytesToRead
		|| (cacheEntry->_bytesRead + bytesToRead) > cacheEntry->_size)
		return false;

	// Move to the correct position in the file
	_file.seek(cacheEntry->_offset + cacheEntry->_bytesRead);
	bool result = _file.read(buffer, bytesToRead) == bytesToRead;
	cacheEntry->_bytesRead += bytesToRead;

	return result;
}

} // End of namespace Titanic
