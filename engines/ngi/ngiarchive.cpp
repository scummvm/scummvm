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

#include "ngi/ngi.h"
#include "common/archive.h"

#include "common/file.h"
#include "common/hash-str.h"
#include "common/memstream.h"
#include "common/bufferedstream.h"
#include "common/textconsole.h"

#include "ngi/ngiarchive.h"

namespace NGI {

NGIArchive::NGIArchive(const Common::Path &filename) : _ngiFile(new Common::File()) {
	if (!_ngiFile->open(filename)) {
		warning("NGIArchive::NGIArchive(): Could not find the archive file");
		delete _ngiFile;
		_ngiFile = nullptr;
		return;
	}

	_ngiFile->seek(4, SEEK_SET);

	unsigned int count = _ngiFile->readUint16LE(); // How many entries?

	_ngiFile->seek(20, SEEK_SET);

	unsigned int key = _ngiFile->readUint16LE();

	byte key1, key2;

	key1 = key & 0xff;
	key2 = (key >> 8) & 0xff;

	int fatSize = count * 32;

	_ngiFile->seek(32, SEEK_SET);

	byte *fat = (byte *)calloc(fatSize, 1);

	_ngiFile->read(fat, fatSize);

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

	debug(4, "NGIArchive::NGIArchive(%s): Located %d files", filename.toString().c_str(), _headers.size());
}

NGIArchive::~NGIArchive() {
	debugC(0, kDebugLoading, "NGIArchive Destructor Called");
	g_nmi->_currArchive = nullptr;

	delete _ngiFile;
}

bool NGIArchive::hasFile(const Common::Path &path) const {
	return _headers.contains(path);
}

int NGIArchive::listMembers(Common::ArchiveMemberList &list) const {
	int matches = 0;

	NgiHeadersMap::const_iterator it = _headers.begin();
	for ( ; it != _headers.end(); ++it) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(Common::Path(it->_value->filename), *this)));
		matches++;
	}

	return matches;
}

const Common::ArchiveMemberPtr NGIArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *NGIArchive::createReadStreamForMember(const Common::Path &path) const {
	if (!_ngiFile || !_headers.contains(path)) {
		return nullptr;
	}

	NgiHeader *hdr = _headers[path].get();

	_ngiFile->seek(hdr->pos, SEEK_SET);

	byte *data = (byte *)malloc(hdr->size);
	assert(data);

	int32 len = _ngiFile->read(data, hdr->size);
	assert(len == hdr->size);

	return new Common::MemoryReadStream(data, hdr->size, DisposeAfterUse::YES);
}

NGIArchive *makeNGIArchive(const Common::Path &name) {
	return new NGIArchive(name);
}

} // End of namespace NGI
