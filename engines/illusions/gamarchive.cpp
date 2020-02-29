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

#include "illusions/gamarchive.h"

namespace Illusions {

GamArchive::GamArchive(const char *filename)
	: _fd(0), _groupCount(0), _groups(0) {
	_fd = new Common::File();
	if (!_fd->open(filename))
		error("GamArchive::GamArchive() Could not open %s", filename);
	loadDictionary();
}

GamArchive::~GamArchive() {
	delete[] _groups;
	delete _fd;
}

byte *GamArchive::readResource(uint32 sceneId, uint32 resId, uint32 &dataSize) {
	const GamFileEntry *fileEntry = getGroupFileEntry(sceneId, resId);
	_fd->seek(fileEntry->_fileOffset);
	dataSize = fileEntry->_fileSize;
	byte *data = (byte*)malloc(dataSize);
	_fd->read(data, dataSize);
	return data;
}

void GamArchive::loadDictionary() {
	_groupCount = _fd->readUint32LE();
	_groups = new GamGroupEntry[_groupCount];
	uint32 *groupOffsets = new uint32[_groupCount];

	for (uint i = 0; i < _groupCount; ++i) {
		_groups[i]._id = _fd->readUint32LE();
		groupOffsets[i] = _fd->readUint32LE();
	}

	for (uint i = 0; i < _groupCount; ++i) {
		_fd->seek(groupOffsets[i]);
		uint32 fileCount = _fd->readUint32LE();
		_groups[i]._fileCount = fileCount;
		_groups[i]._files = new GamFileEntry[fileCount];

		debug(8, "Group %08X, fileCount: %d", _groups[i]._id, _groups[i]._fileCount);

		for (uint j = 0; j < fileCount; ++j) {
			_groups[i]._files[j]._id = _fd->readUint32LE();
			_groups[i]._files[j]._fileOffset = _fd->readUint32LE();
			_groups[i]._files[j]._fileSize = _fd->readUint32LE();
			debug(8, "  %08X, %08X, %d", _groups[i]._files[j]._id, _groups[i]._files[j]._fileOffset, _groups[i]._files[j]._fileSize);
		}
	}

	delete[] groupOffsets;
}

const GamGroupEntry *GamArchive::getGroupEntry(uint32 sceneId) {
	for (uint i = 0; i < _groupCount; ++i) {
		if (_groups[i]._id == sceneId)
			return &_groups[i];
	}
	return 0;
}

const GamFileEntry *GamArchive::getFileEntry(const GamGroupEntry *groupEntry, uint32 resId) {
	for (uint i = 0; i < groupEntry->_fileCount; ++i) {
		if (groupEntry->_files[i]._id == resId)
			return &groupEntry->_files[i];
	}
	return 0;
}

const GamFileEntry *GamArchive::getGroupFileEntry(uint32 sceneId, uint32 resId) {
	const GamGroupEntry *groupEntry = getGroupEntry(sceneId);
	if (!groupEntry)
		error("GamArchive::getFileEntry() Group %08X not found", sceneId);
	const GamFileEntry *fileEntry = getFileEntry(groupEntry, resId);
	if (!fileEntry)
		error("GamArchive::getFileEntry() File %08X in group %08X not found", resId, sceneId);
	return fileEntry;
}

} // End of namespace Illusions
