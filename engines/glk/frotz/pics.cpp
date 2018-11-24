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
#include "glk/frotz/pics_decoder.h"
#include "glk/glk.h"
#include "common/algorithm.h"

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

	Common::Array<uint> offsets;
	byte buffer[16];
	f.read(buffer, 16);
	_index.resize(READ_LE_UINT16(&buffer[PIC_FILE_HEADER_NUM_IMAGES]));
	_entrySize = buffer[PIC_FILE_HEADER_ENTRY_SIZE];
	_version = buffer[PIC_FILE_HEADER_FLAGS];
	assert(_entrySize >= 8 && _entrySize <= 14);

	// Iterate through loading the index
	for (uint idx = 0; idx < _index.size(); ++idx) {
		Entry &e = _index[idx];
		f.read(buffer, _entrySize);

		e._number = READ_LE_UINT16(buffer);
		e._width = READ_LE_UINT16(buffer + 2);
		e._height = READ_LE_UINT16(buffer + 4);
		e._flags = READ_LE_UINT16(buffer + 6);

		if (_entrySize >= 11) {
			e._dataOffset = READ_BE_UINT32(buffer + 7) & 0xffffff;
			if (e._dataOffset)
				offsets.push_back(e._dataOffset);

			if (_entrySize == 14) {
				e._paletteOffset = READ_BE_UINT32(buffer + 10) & 0xffffff;
			}
		}

		e._filename = Common::String::format("pic%u.raw", e._number);
	}

	// Further processing of index to calculate data sizes
	Common::sort(offsets.begin(), offsets.end());

	for (uint idx = 0; idx < _index.size(); ++idx) {
		Entry &e = _index[idx];
		if (!e._dataOffset)
			continue;

		// Find the entry in the offsets array
		uint oidx = 0;
		while (oidx < offsets.size() && offsets[oidx] != e._dataOffset)
			++oidx;

		// Set the size
		e._dataSize = (oidx == (offsets.size() - 1) ? f.size() : offsets[oidx + 1]) - e._dataOffset;
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
		const Entry &e = _index[idx];
		if (e._filename.equalsIgnoreCase(name)) {
			Common::Array<byte> palette;
			Common::File f;
			Common::SeekableReadStream *dest;
			if (!f.open(_filename))
				error("Reading failed");

			// Read in the image's palette
			assert(e._paletteOffset);
			f.seek(e._paletteOffset);
			palette.resize(f.readByte() * 3);
			f.read(&palette[0], palette.size());

			PictureDecoder decoder;
			if (e._dataSize) {
				Common::SeekableReadStream *src = f.readStream(e._dataSize);
				dest = decoder.decode(*src, e._flags, palette, MCGA, e._width, e._height);
				delete src;
			} else {
				error("TODO: Empty rect renderings");
			}

			f.close();
			return dest;
		}
	}

	return nullptr;
}

} // End of namespace Frotz
} // End of namespace Glk
