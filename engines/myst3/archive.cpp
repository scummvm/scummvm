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

#include "engines/myst3/archive.h"
#include "common/debug.h"
#include "common/memstream.h"

namespace Myst3 {

void Archive::_decryptHeader(Common::SeekableReadStream &inStream, Common::WriteStream &outStream) {
	static const uint32 addKey = 0x3C6EF35F;
	static const uint32 multKey = 0x0019660D;

	inStream.seek(0);
	uint32 encryptedSize = inStream.readUint32LE();
	uint32 decryptedSize = encryptedSize ^ addKey;
	
	inStream.seek(0);
	uint32 currentKey = 0;

	for (uint i = 0; i < decryptedSize; i++) {
		currentKey += addKey;
		outStream.writeUint32LE(inStream.readUint32LE() ^ currentKey);
		currentKey *= multKey;
	}
}

void Archive::_readDirectory() {
	Common::MemoryWriteStreamDynamic buf(DisposeAfterUse::YES);
	_decryptHeader(_file, buf);
	
	Common::MemoryReadStream directory(buf.getData(), buf.size());
	directory.skip(sizeof(uint32));
	
	while (directory.pos() < directory.size()) {
		DirectoryEntry entry;
		entry.readFromStream(directory);
		if (entry.hasSubEntries()) {
			_directory.push_back(entry);
		}
	}
}

void Archive::dumpDirectory() {
	for (uint i = 0; i < _directory.size(); i++) {
		_directory[i].dump();
	}
}

void Archive::dumpToFiles() {
	for (uint i = 0; i < _directory.size(); i++) {
		_directory[i].dumpToFiles(_file);
	}
}

Common::MemoryReadStream *Archive::dumpToMemory(uint16 index, uint16 face, DirectorySubEntry::ResourceType type) {
	for (uint i = 0; i < _directory.size(); i++) {
		if (_directory[i].getIndex() == index) {
			return _directory[i].dumpToMemory(_file, face, type);
		}
	}
	
	return 0;
}

bool Archive::open(const char *fileName) {
	if (_file.open(fileName)) {
		_readDirectory();
		return true;
	}
	
	return false;
}

void Archive::close() {
	_directory.clear();
	_file.close();
}

} // end of namespace Myst3
