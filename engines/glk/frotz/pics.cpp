/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/frotz/pics.h"
#include "glk/glk.h"

namespace Glk {
namespace Frotz {

enum {
	PIC_FILE_HEADER_FLAGS      = 1,
	PIC_FILE_HEADER_NUM_IMAGES = 4,
	PIC_FILE_HEADER_ENTRY_SIZE = 8,
	PIC_FILE_HEADER_VERSION    = 14
};

Pics::Pics() : Common::Archive(), _filename(getFilename()) {
	Common::File f;
	if (!f.open(_filename))
		error("Error reading Pics file");

	byte buffer[16];
	f.read(buffer, 16);
	_index.resize(READ_LE_UINT16(&buffer[PIC_FILE_HEADER_NUM_IMAGES]));
	_entrySize = buffer[PIC_FILE_HEADER_ENTRY_SIZE];
	_version = buffer[PIC_FILE_HEADER_FLAGS];

	// Iterate through loading the index
	for (uint idx = 0; idx < _index.size(); ++idx) {
		Entry &e = _index[idx];
		e._number = f.readUint16LE();
		e._offset = f.pos();
		e._size = _entrySize - 2;
		f.skip(_entrySize - 2);

		e._filename = Common::String::format("PIC%u", e._number);
	}

	f.close();
}

Common::String Pics::getFilename() {
	Common::String filename = g_vm->getFilename();
	while (filename.contains('.'))
		filename.deleteLastChar();

	return filename + ".mg1";
}

bool Pics::exists() {
	return Common::File::exists(getFilename());
}

bool Pics::hasFile(const Common::String &name) const {
	for (uint idx = 0; idx < _index.size(); ++idx) {
		if (_index[idx]._filename.equalsIgnoreCase(name))
			return true;
	}

	return false;
}

int Pics::listMembers(Common::ArchiveMemberList &list) const {
	for (uint idx = 0; idx < _index.size(); ++idx) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(_index[idx]._filename, this)));
	}

	return (int)_index.size();
}

const Common::ArchiveMemberPtr Pics::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *Pics::createReadStreamForMember(const Common::String &name) const {
	for (uint idx = 0; idx < _index.size(); ++idx) {
		if (_index[idx]._filename.equalsIgnoreCase(name)) {
			Common::File f;
			if (!f.open(_filename))
				error("Reading failed");

			f.seek(_index[idx]._offset);
			Common::SeekableReadStream *result = f.readStream(_index[idx]._size);
			f.close();

			return result;
		}
	}

	return nullptr;
}

} // End of namespace Frotz
} // End of namespace Glk
