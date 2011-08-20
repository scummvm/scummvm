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
 */

// The following code is based on unshield
// Original copyright:

// Copyright (c) 2003 David Eriksson <twogood@users.sourceforge.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "agos/installshield_cab.h"

#include "common/debug.h"
#include "common/memstream.h"
#include "common/zlib.h"

namespace AGOS {

InstallShieldCabinet::InstallShieldCabinet() : Common::Archive() {
	_stream = 0;
}

InstallShieldCabinet::~InstallShieldCabinet() {
	close();
}

bool InstallShieldCabinet::open(const Common::String &filename) {
	close();

	_stream = SearchMan.createReadStreamForMember(filename);

	if (!_stream)
		return false;

	// Note that we only support a limited subset of cabinet files
	// Only single cabinet files and ones without data shared between
	// cabinets.

	// Check for the magic uint32
	if (_stream->readUint32LE() != 0x28635349) {
		close();
		return false;
	}

	uint32 version = _stream->readUint32LE();

	if (version != 0x01000004) {
		warning("Unsupported CAB version %08x", version);
		close();
		return false;
	}

	/* uint32 volumeInfo = */ _stream->readUint32LE();
	uint32 cabDescriptorOffset = _stream->readUint32LE();
	/* uint32 cabDescriptorSize = */ _stream->readUint32LE();

	_stream->seek(cabDescriptorOffset);

	_stream->skip(12);
	uint32 fileTableOffset = _stream->readUint32LE();
	_stream->skip(4);
	uint32 fileTableSize = _stream->readUint32LE();
	uint32 fileTableSize2 = _stream->readUint32LE();
	uint32 directoryCount = _stream->readUint32LE();
	_stream->skip(8);
	uint32 fileCount = _stream->readUint32LE();

	if (fileTableSize != fileTableSize2)
		warning("file table sizes do not match");

	// We're ignoring file groups and components since we
	// should not need them. Moving on to the files...

	_stream->seek(cabDescriptorOffset + fileTableOffset);
	uint32 fileTableCount = directoryCount + fileCount;
	uint32 *fileTableOffsets = new uint32[fileTableCount];
	for (uint32 i = 0; i < fileTableCount; i++)
		fileTableOffsets[i] = _stream->readUint32LE();

	for (uint32 i = directoryCount; i < fileCount + directoryCount; i++) {
		_stream->seek(cabDescriptorOffset + fileTableOffset + fileTableOffsets[i]);
		uint32 nameOffset = _stream->readUint32LE();
		/* uint32 directoryIndex = */ _stream->readUint32LE();

		// First read in data needed by us to get at the file data
		FileEntry entry;
		entry.flags = _stream->readUint16LE();
		entry.uncompressedSize = _stream->readUint32LE();
		entry.compressedSize = _stream->readUint32LE();
		_stream->skip(20);
		entry.offset = _stream->readUint32LE();

		// Then let's get the string
		_stream->seek(cabDescriptorOffset + fileTableOffset + nameOffset);
		Common::String fileName;

		char c = _stream->readByte();
		while (c) {
			fileName += c;
			c = _stream->readByte();
		}

		_map[fileName] = entry;
	}

	delete[] fileTableOffsets;

	return true;
}

void InstallShieldCabinet::close() {
	delete _stream; _stream = 0;
	_map.clear();
}

bool InstallShieldCabinet::hasFile(const Common::String &name) {
	return _map.contains(name);
}

int InstallShieldCabinet::listMembers(Common::ArchiveMemberList &list) {
	for (FileMap::const_iterator it = _map.begin(); it != _map.end(); it++)
		list.push_back(getMember(it->_key));

	return _map.size();
}

Common::ArchiveMemberPtr InstallShieldCabinet::getMember(const Common::String &name) {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *InstallShieldCabinet::createReadStreamForMember(const Common::String &name) const {
	if (!_stream || !_map.contains(name))
		return 0;

	const FileEntry &entry = _map[name];

	_stream->seek(entry.offset);

	if (!(entry.flags & 0x04)) {
		// Not compressed
		return _stream->readStream(entry.uncompressedSize);
	}

#ifdef USE_ZLIB
	byte *src = (byte *)malloc(entry.compressedSize);
	byte *dst = (byte *)malloc(entry.uncompressedSize);

	_stream->read(src, entry.compressedSize);

	bool result = Common::inflateZlibHeaderless(dst, entry.uncompressedSize, src, entry.compressedSize);
	free(src);

	if (!result) {
		warning("failed to inflate CAB file '%s'", name.c_str());
		free(dst);
		return 0;
	}

	return new Common::MemoryReadStream(dst, entry.uncompressedSize, DisposeAfterUse::YES);
#else
	warning("zlib required to extract compressed CAB file '%s'", name.c_str());
	return 0;
#endif
}

} // End of namespace AGOS
