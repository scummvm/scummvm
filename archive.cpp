/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/stark/archive.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/util.h"

namespace Stark {

bool XARCArchive::open(Common::String filename) {
	if (!_file.open(filename)) {
		warning("Could not open file \'%s\'", filename.c_str());
		return false;
	}
	
	_file.readUint32LE();
	uint32 files = _file.readUint32LE();
	uint32 base = _file.readUint32LE();

	_fileEntries = new FileEntry[files];

	for (uint32 i = 0; i < files; i++){
		char ch;
		while ((ch = (char)_file.readByte()) != 0)
			_fileEntries[i].filename += ch;
		_fileEntries[i].length = _file.readUint32LE();
		_fileEntries[i].offset = base;
		base += _fileEntries[i].length;
		_file.readUint32LE();
	}
	
	return true;
}

Common::SeekableReadStream *XARCArchive::getRawData(uint32 fileNum){
	_file.seek(_fileEntries[fileNum].offset);
	
	return _file.readStream(_fileEntries[fileNum].length);
}

void XARCArchive::close() {
	_file.close();
}

//inline Common::String XARCArchive::readString(Common::SeekableReadStream *stream){
	

} // End of namespace Stark
