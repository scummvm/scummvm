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

#include "common/file.h"
#include "common/memstream.h"
#include "got/utils/compression.h"
#include "got/utils/res_archive.h"

namespace Got {

static const char *RES_FILENAME = "gotres.dat";

void resInit() {
	ResArchive *a = new ResArchive();
	SearchMan.add("Res", a);
}

void ResHeader::load(Common::SeekableReadStream *src) {
	char buf[10];
	src->read(buf, 9);
	buf[9] = '\0';
	_name = buf;

	_offset = src->readUint32LE();
	_size = src->readUint32LE();
	_originalSize = src->readUint32LE();
	_key = src->readUint16LE();
}

ResArchive::ResArchive() {
	Common::File f;
	if (!f.open(RES_FILENAME))
		error("Could not open %s", RES_FILENAME);

	// Read in header data and decrypt it
	byte buf[RES_MAX_ENTRIES * RES_HEADER_ENTRY_SIZE];
	if (f.read(buf, RES_MAX_ENTRIES * RES_HEADER_ENTRY_SIZE) !=
		(RES_MAX_ENTRIES * RES_HEADER_ENTRY_SIZE))
		error("Could not read in resource headers");

	decrypt(buf, RES_MAX_ENTRIES * RES_HEADER_ENTRY_SIZE, 128);

	// Load headers
	Common::MemoryReadStream hdrData(buf, RES_MAX_ENTRIES * RES_HEADER_ENTRY_SIZE);
	for (int i = 0; i < RES_MAX_ENTRIES; ++i) {
		ResHeader hdr;
		hdr.load(&hdrData);
		if (!(hdr._offset == 0 && hdr._size == 0))
			_headers.push_back(hdr);
	}
}

void ResArchive::decrypt(byte *buf, size_t len, byte key) const {
	for (size_t i = 0; i < len; ++i)
		*buf++ ^= key++;
}

int ResArchive::indexOf(const Common::String &name) const {
	for (uint i = 0; i < _headers.size(); ++i) {
		if (_headers[i]._name.equalsIgnoreCase(name))
			return i;
	}

	return -1;
}

bool ResArchive::hasFile(const Common::Path &path) const {
	return indexOf(path.baseName()) != -1;
}

int ResArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (uint i = 0; i < _headers.size(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(
			new Common::GenericArchiveMember(_headers[i]._name, *this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr ResArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *ResArchive::createReadStreamForMember(const Common::Path &path) const {
	// Get the index of the header entry for this file
	int hdrIndex = indexOf(path.baseName());
	if (hdrIndex == -1)
		return nullptr;

	Common::File f;
	if (!f.open(RES_FILENAME))
		error("Error reading resource");

	// Set up buffers
	const ResHeader &hdr = _headers[hdrIndex];
	byte *buf = (byte *)malloc(hdr._size);

	f.seek(hdr._offset);
	if (f.read(buf, hdr._size) != hdr._size)
		error("Error reading resource");

	// Decrypt if necessary
	if (hdr._key != 0) {
		byte *temp = buf;
		buf = (byte *)malloc(hdr._originalSize);

		lzssDecompress(temp, buf);
		free(temp);
	}

	return new Common::MemoryReadStream(buf, hdr._originalSize,
										DisposeAfterUse::YES);
}

void ResArchive::lzssDecompress(const byte *src, byte *dest) const {
	uint16 size = READ_LE_UINT16(src);
	assert(READ_LE_UINT16(src + 2) == 1);
	src += 4;

	lzss_decompress(src, dest, size);
}

} // namespace Got
