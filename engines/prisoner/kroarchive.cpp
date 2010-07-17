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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/debug.h"
#include "common/zlib.h"

#include "prisoner/kroarchive.h"

namespace Prisoner {

/* KroArchive */

KroArchive::KroArchive() : _fd(NULL) {
}

KroArchive::~KroArchive() {
	close();
}

void KroArchive::open(const char *filename) {

	close();
	_entries.clear();

	_fd = new Common::File();
	if (!_fd->open(filename))
		error("KroArchive::open() Could not open %s", filename);

	_fd->readUint32LE(); // skip Burp
	uint32 count = _fd->readUint32LE();
	_entries.reserve(count);

	for (uint32 i = 0; i < count; i++) {
		KroArchiveEntry entry;
		entry.size = _fd->readUint32LE();
		entry.compressedSize = _fd->readUint32LE();
		_fd->readUint32LE(); // skip unknown/unused value
		entry.offset = _fd->readUint32LE();
		entry.compressionType = _fd->readUint32LE();
		_entries.push_back(entry);
	}

}

void KroArchive::close() {
	delete _fd;
}

byte *KroArchive::load(uint index) {
	byte *data = NULL;
	KroArchiveEntry *entry = &_entries[index];

	debug("index = %08X", index);

	_fd->seek(entry->offset);

	data = new byte[entry->size];

	switch (entry->compressionType) {
	case 0: // uncompressed
	{
		_fd->read(data, entry->size);
		break;
	}
	case 4: // zlib compressed
	{
		uint32 dstLen = entry->size;
		byte *compressedData = new byte[entry->compressedSize];
		_fd->read(compressedData, entry->compressedSize);
		if (uncompress(data, &dstLen, compressedData, entry->compressedSize) != Z_OK)
			error("KroArchive::load(%d) Error decompressing type %d", index, entry->compressionType);
		delete[] compressedData;
		break;
	}
	default:
		error("KroArchive::load(%d) Unknown compression type %d", index, entry->compressionType);
	}

	return data;
}

uint32 KroArchive::getSize(uint index) {
	return _entries[index].size;
}

/* PakDirectory */

PakDirectory::PakDirectory() {
}

PakDirectory::~PakDirectory() {
}

void PakDirectory::load(const char *filename, uint32 offset, bool isEncrypted) {
	Common::ReadStream *stream = NULL;
	if (isEncrypted) {
		// TODO: Load file and decrypt
	} else {
		Common::File *fd = new Common::File();
		if (!fd->open(filename))
			error("PakDirectory::load() Could not open %s", filename);
		stream = fd;
	}
	uint32 count = stream->readUint32LE();
	_directory.reserve(count);
	for (uint32 i = 0; i < count; i++) {
		PakDirectoryEntry entry;
		char pakName[8];
		stream->read(pakName, 8);
		entry.pakName = pakName;
		entry.baseIndex = stream->readUint32LE();
		_directory.push_back(entry);

		debug("pakName = %s; baseIndex = %d", entry.pakName.c_str(), entry.baseIndex);

	}
	delete stream;
}

uint32 PakDirectory::getBaseIndex(Common::String &pakName) {
	for (uint i = 0; i < _directory.size(); i++) {
		if (_directory[i].pakName == pakName)
			return _directory[i].baseIndex;
	}
	error("PakDirectory::getBaseIndex() PakName %s not found", pakName.c_str());
}

int uncompress (Bytef *dest, uint32 *destLen, Bytef *source, uint32 sourceLen) {
	z_stream stream;
	int err;
	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	err = inflateInit2_(&stream, -MAX_WBITS, ZLIB_VERSION, sizeof(z_stream));
	if (err != Z_OK) return err;
	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
			return Z_DATA_ERROR;
		return err;
	}
	*destLen = stream.total_out;
	err = inflateEnd(&stream);
	return err;
}

} // End of namespace Prisoner
