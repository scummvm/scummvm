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

#include "hdb/file-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/error.h"

namespace HDB {

bool FileMan::openMSD(const Common::String &filename) {
	uint32 offset;

	if (!_msdFile->open(filename)) {
		error("FileMan::openMSD(): Error reading the MSD file");
		return false;
	}

	_msdFile->read(&dataHeader.id, 4);
	
	if (dataHeader.id == MKTAG('M', 'P', 'C', 'C')) {
		_compressed = true;
		debug("COMPRESSED FILE");
		return false;
	}
	else if (dataHeader.id == MKTAG('M', 'P', 'C', 'U')) {
		_compressed = false;

		offset = _msdFile->readUint32LE();
		_msdFile->seek((int32)offset, SEEK_SET);

		// Note: The MPC archive format assumes the offset to be uint32,
		// but Common::File::seek() takes the offset as int32. 

		dataHeader.dirSize = _msdFile->readUint32LE();

		for (uint32 fileIndex = 0; fileIndex < dataHeader.dirSize; fileIndex++) {
			MSDEntry *dirEntry = new MSDEntry();

			for (int fileNameIndex = 0; fileNameIndex < 64; fileNameIndex++) {
				dirEntry->filename[fileNameIndex] = _msdFile->readByte();
			}

			dirEntry->offset = _msdFile->readUint32LE();
			dirEntry->length = _msdFile->readUint32LE();
			dirEntry->ulength = _msdFile->readUint32LE();
			dirEntry->type = (DataType)_msdFile->readUint32LE();

			_dir.push_back(dirEntry);
		}

		return true;

	}
	
	error("Invalid MPC File.");
	return false;

}

void FileMan::closeMSD() {
	_dir.clear();
	_msdFile->close();
}

} // End of Namespace HDB
