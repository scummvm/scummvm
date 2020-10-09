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

#include "ngi/ngi.h"
#include "common/archive.h"

#include "common/file.h"
#include "common/hash-str.h"
#include "common/memstream.h"
#include "common/bufferedstream.h"
#include "common/textconsole.h"

#include "ngi/ngiarchive.h"

namespace NGI {

NGIArchive::NGIArchive(const Common::String &filename) : _ngiFilename(filename) {
	Common::File ngiFile;

	if (!ngiFile.open(_ngiFilename)) {
		warning("NGIArchive::NGIArchive(): Could not find the archive file");
		return;
	}

	ngiFile.seek(4, SEEK_SET);

	unsigned int count = ngiFile.readUint16LE(); // How many entries?

	ngiFile.seek(20, SEEK_SET);

	unsigned int key = ngiFile.readUint16LE();

	byte key1, key2;

	key1 = key & 0xff;
	key2 = (key >> 8) & 0xff;

	int fatSize = count * 32;

	ngiFile.seek(32, SEEK_SET);

	byte *fat = (byte *)calloc(fatSize, 1);

	ngiFile.read(fat, fatSize);

	for (int i = 0; i < fatSize; i++) {
		key1 = (key1 << 1) ^ key2;
		key2 = (key2 >> 1) ^ key1;

		fat[i] ^= key1;
	}

	NgiHeader header;

	for (uint i = 0; i < count; i++) {
		memcpy(header.filename, &fat[i * 32], 12);
		header.filename[12] = 0;
		header.flags = READ_LE_UINT32(&fat[i * 32 + 16]);
		header.extVal = READ_LE_UINT32(&fat[i * 32 + 20]);
		header.pos = READ_LE_UINT32(&fat[i * 32 + 24]);
		header.size = READ_LE_UINT32(&fat[i * 32 + 28]);

		debug(5, "file: %s  flags: %x  extVal: %d  pos: %d  size: %d", header.filename, header.flags, header.extVal, header.pos, header.size);

		if (header.flags & 0x1e0) {
			warning("File has flags: %.8x\n", header.flags & 0x1e0);
		}

		_headers[header.filename].reset(new NgiHeader(header));
	}

	free(fat);

	g_nmi->_currArchive = this;

	debug(4, "NGIArchive::NGIArchive(%s): Located %d files", filename.c_str(), _headers.size());
}

NGIArchive::~NGIArchive() {
	debugC(0, kDebugLoading, "NGIArchive Destructor Called");
	g_nmi->_currArchive = nullptr;
}

bool NGIArchive::hasFile(const Common::String &name) const {
	return _headers.contains(name);
}

int NGIArchive::listMembers(Common::ArchiveMemberList &list) const {
	int matches = 0;

	NgiHeadersMap::const_iterator it = _headers.begin();
	for ( ; it != _headers.end(); ++it) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(it->_value->filename, this)));
		matches++;
	}

	return matches;
}

const Common::ArchiveMemberPtr NGIArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *NGIArchive::createReadStreamForMember(const Common::String &name) const {
	if (!_headers.contains(name)) {
		return 0;
	}

	NgiHeader *hdr = _headers[name].get();

	Common::File archiveFile;
	archiveFile.open(_ngiFilename);
	archiveFile.seek(hdr->pos, SEEK_SET);

	byte *data = (byte *)malloc(hdr->size);
	assert(data);

	int32 len = archiveFile.read(data, hdr->size);
	assert(len == hdr->size);

	return new Common::MemoryReadStream(data, hdr->size, DisposeAfterUse::YES);
}

NGIArchive *makeNGIArchive(const Common::String &name) {
	return new NGIArchive(name);
}

} // End of namespace NGI
