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

#include "glk/zcode/pics.h"
#include "glk/zcode/pics_decoder.h"
#include "glk/glk.h"
#include "common/algorithm.h"
#include "common/memstream.h"

namespace Glk {
namespace ZCode {

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

	_palette = new Common::Array<byte>();

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

		if (e._dataOffset)
			e._filename = Common::String::format("pic%u.raw", e._number);
		else
			e._filename = Common::String::format("pic%u.rect", e._number);
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

Pics::~Pics() {
	delete _palette;
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
	PictureDecoder decoder;

	for (uint idx = 0; idx < _index.size(); ++idx) {
		const Entry &e = _index[idx];
		if (e._filename.equalsIgnoreCase(name)) {
			Common::File f;
			Common::SeekableReadStream *dest;
			if (!f.open(_filename))
				error("Reading failed");

			if (e._dataSize) {
				loadPalette(f, e, *_palette);

				f.seek(e._dataOffset);
				Common::SeekableReadStream *src = f.readStream(e._dataSize);
				dest = decoder.decode(*src, e._flags, *_palette, kMCGA, e._width, e._height);
				delete src;
			} else {
				byte *rect = (byte *)malloc(2 * sizeof(uint32));
				WRITE_BE_UINT32(rect, e._width);
				WRITE_BE_UINT32(rect + 4, e._height);
				dest = new Common::MemoryReadStream(rect, 2 * sizeof(uint32), DisposeAfterUse::YES);
			}

			f.close();
			return dest;
		}
	}

	return nullptr;
}

void Pics::loadPalette(Common::File &f, const Entry &e, Common::Array<byte> &palette) const {
	if (e._paletteOffset) {
		// Read in the image's palette
		assert(e._paletteOffset);
		f.seek(e._paletteOffset);
		_palette->resize(f.readByte() * 3);
		f.read(&(*_palette)[0], _palette->size());
	}

	if (e._flags & 1) {
		byte *entry = &palette[(e._flags >> 12) * 3];
		Common::fill(entry, entry + 3, 0);
	}
}

} // End of namespace ZCode
} // End of namespace Glk
