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

#include "backends/cloud/storagefile.h"

namespace Cloud {

StorageFile::StorageFile() {
	_id = "";
	_path = "";
	_name = "";
	_size = 0;
	_timestamp = 0;
	_isDirectory = false;
}

StorageFile::StorageFile(const Common::String &pth, uint32 sz, uint32 ts, bool dir) {
	_id = pth;
	_path = pth;

	_name = pth;
	if (_name.size() != 0) {
		uint32 i = _name.size() - 1;
		while (true) {
			if (_name[i] == '/' || _name[i] == '\\') {
				_name.erase(0, i + 1);
				break;
			}
			if (i == 0)
				break;
			--i;
		}
	}

	_size = sz;
	_timestamp = ts;
	_isDirectory = dir;
}

StorageFile::StorageFile(const Common::String &fileId, const Common::String &filePath, const Common::String &fileName, uint32 sz, uint32 ts, bool dir) {
	_id = fileId;
	_path = filePath;
	_name = fileName;
	_size = sz;
	_timestamp = ts;
	_isDirectory = dir;
}

} // End of namespace Cloud
