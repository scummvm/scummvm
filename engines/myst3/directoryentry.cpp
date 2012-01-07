/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/directoryentry.h"
#include "common/debug.h"

namespace Myst3 {

DirectoryEntry::DirectoryEntry(Archive *archive) :
	_archive(archive) {
	memset(_roomName, 0, sizeof(_roomName));
}

void DirectoryEntry::readFromStream(Common::SeekableReadStream &inStream, const char *room) {
	if (room == 0)
		inStream.read(_roomName, 4);
	else
		Common::strlcpy(_roomName, room, sizeof(_roomName));

	_index = inStream.readUint16LE();
	_unk = inStream.readByte();
	byte subItemCount = inStream.readByte();
	
	// End of directory marker ?
	if (_unk > 2) {
		subItemCount = 0;
	}

	_subentries.clear();
	for (uint i = 0; i < subItemCount ; i++) {
		DirectorySubEntry subEntry(_archive);
		subEntry.readFromStream(inStream);
		_subentries.push_back(subEntry);
	}
}

void DirectoryEntry::dump() {
	debug("index : %d unk: %d subitems : %d", _index, _unk, _subentries.size());

	for (uint i = 0; i < _subentries.size(); i++) {
		_subentries[i].dump();
	}
}

void DirectoryEntry::dumpToFiles(Common::SeekableReadStream &inStream) {
	for (uint i = 0; i < _subentries.size(); i++) {
		_subentries[i].dumpToFile(inStream, _index);
	}
}

DirectorySubEntry *DirectoryEntry::getItemDescription(uint16 face, DirectorySubEntry::ResourceType type) {
	for (uint i = 0; i < _subentries.size(); i++) {
		if (_subentries[i].getFace() == face
				&& _subentries[i].getType() == type) {
			return &_subentries[i];
		}
	}
	
	return 0;
}

} // end of namespace Myst3
