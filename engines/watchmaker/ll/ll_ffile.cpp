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

#include "watchmaker/ll/ll_ffile.h"
#include "common/substream.h"
#include "watchmaker/ll/ll_system.h"

namespace Watchmaker {

const int MAX_NAME_LEN = 52;

struct FileEntry {
	Common::String name;
	int32 offset;
	int32 time;
	int32 date;
};

FastFile::FastFile(const char *path) : _path(path) {
	auto stream = openFile(path);
	assert(stream);

	_numFiles = stream->readUint32LE();
	_files = new FileEntry[_numFiles] {};
	_totalSize = stream->size();

	for (int i = 0; i < _numFiles; i++) {
		char name[MAX_NAME_LEN] = {};
		stream->read(name, MAX_NAME_LEN);
		_files[i].name = name;
		_files[i].offset = stream->readUint32LE();
		_files[i].time = stream->readUint32LE();
		_files[i].date = stream->readUint32LE();
		for (auto it = _files[i].name.begin(); it != _files[i].name.end(); ++it) {
			if (*it == '\\') {
				*it = '/';
			}
		}
	}
}

Common::SharedPtr<Common::SeekableReadStream> FastFile::resolve(const char *filename) {
	int index = -1;
	//HACK
	if (strlen(filename) >= 2 && filename[0] == '.' && filename[1] == '/') {
		filename = filename + 2;
	}
	for (int i = 0; i < _numFiles; i++) {
		if (_files[i].name.equalsIgnoreCase(filename)) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		return nullptr;
	}
	auto entry = _files[index];
	int size = 0;
	if (index == _numFiles - 1) {
		size = _totalSize - entry.offset;
	} else {
		size = _files[index + 1].offset - entry.offset;
	}
	auto stream = openFile(_path, entry.offset, size);
	assert(stream);
	return Common::SharedPtr<Common::SeekableReadStream>(stream);
}

} // End of namespace Watchmaker
