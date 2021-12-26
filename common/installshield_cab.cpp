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

#include "common/archive.h"
#include "common/debug.h"
#include "common/hash-str.h"
#include "common/installshield_cab.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/ptr.h"
#include "common/zlib.h"

namespace Common {

namespace {

class InstallShieldCabinet : public Archive {
public:
	InstallShieldCabinet();

	bool open(const String &baseName);
	void close();

	// Archive API implementation
	bool hasFile(const Path &path) const override;
	int listMembers(ArchiveMemberList &list) const override;
	const ArchiveMemberPtr getMember(const Path &path) const override;
	SeekableReadStream *createReadStreamForMember(const Path &path) const override;

private:
	struct FileEntry {
		uint32 uncompressedSize;
		uint32 compressedSize;
		uint32 offset;
		uint16 flags;
		uint16 volume;
	};

	int _version;
	typedef HashMap<String, FileEntry, IgnoreCase_Hash, IgnoreCase_EqualTo> FileMap;
	FileMap _map;
	String _baseName;

	String getHeaderName() const;
	String getVolumeName(uint volume) const;
};

InstallShieldCabinet::InstallShieldCabinet() : _version(0) {
}

bool InstallShieldCabinet::open(const String &baseName) {
	// Store the base name so we can generate file names
	_baseName = baseName;

	// Try to find the file
	SeekableReadStream *header = SearchMan.createReadStreamForMember(getHeaderName());
	if (!header)
		header = SearchMan.createReadStreamForMember(getVolumeName(1));

	if (!header) {
		close();
		return false;
	}

	// Check for the cab signature
	uint32 signature = header->readUint32LE();
	if (signature != 0x28635349) {
		warning("InstallShieldCabinet::InstallShieldCabinet(): Signature doesn't match: expecting %x but got %x", 0x28635349, signature);
		close();
		return false;
	}

	// We support cabinet versions 5 - 13, with some exceptions:
	// - obfuscated files are not deobfuscated
	// - no support for files split across volumes
	// - single-volume v5 cabinets only

	uint32 magicBytes = header->readUint32LE();
	int shift = magicBytes >> 24;
	_version = shift == 1 ? (magicBytes >> 12) & 0xf : (magicBytes & 0xffff) / 100;
	_version = (_version == 0) ? 5 : _version;

	if (_version < 5 || _version > 13) {
		warning("Unsupported CAB version %d, magic bytes %08x", _version, magicBytes);
		close();
		return false;
	}

	/* uint32 volumeInfo = */ header->readUint32LE();
	uint32 cabDescriptorOffset = header->readUint32LE();
	/* uint32 cabDescriptorSize = */ header->readUint32LE();

	header->seek(cabDescriptorOffset);

	header->skip(12);
	uint32 fileTableOffset = header->readUint32LE();
	header->skip(4);
	uint32 fileTableSize = header->readUint32LE();
	uint32 fileTableSize2 = header->readUint32LE();
	uint32 directoryCount = header->readUint32LE();
	header->skip(8);
	uint32 fileCount = header->readUint32LE();

	if (fileTableSize != fileTableSize2)
		warning("file table sizes do not match");

	// We're ignoring file groups and components since we
	// should not need them. Moving on to the files...

	if (_version >= 6) {
		uint32 fileTableOffset2 = header->readUint32LE();

		for (uint32 i = 0; i < fileCount; i++) {
			header->seek(cabDescriptorOffset + fileTableOffset + fileTableOffset2 + i * 0x57);
			FileEntry entry;
			entry.flags = header->readUint16LE();
			entry.uncompressedSize = header->readUint32LE();
			header->skip(4);
			entry.compressedSize = header->readUint32LE();
			header->skip(4);
			entry.offset = header->readUint32LE();
			header->skip(36);
			uint32 nameOffset = header->readUint32LE();
			/* uint32 directoryIndex = */ header->readUint16LE();
			header->skip(12);
			/* entry.linkPrev = */ header->readUint32LE();
			/* entry.linkNext = */ header->readUint32LE();
			/* entry.linkFlags = */ header->readByte();
			entry.volume = header->readUint16LE();

			// Make sure the entry has a name and data inside the cab
			if (nameOffset == 0 || entry.offset == 0 || (entry.flags & 8))
				continue;

			// Then let's get the string
			header->seek(cabDescriptorOffset + fileTableOffset + nameOffset);
			String fileName = header->readString();
			_map[fileName] = entry;
		}
	} else {
		header->seek(cabDescriptorOffset + fileTableOffset);
		uint32 fileTableCount = directoryCount + fileCount;
		Array<uint32> fileTableOffsets;
		fileTableOffsets.resize(fileTableCount);
		for (uint32 i = 0; i < fileTableCount; i++)
			fileTableOffsets[i] = header->readUint32LE();

		for (uint32 i = directoryCount; i < fileCount + directoryCount; i++) {
			header->seek(cabDescriptorOffset + fileTableOffset + fileTableOffsets[i]);
			uint32 nameOffset = header->readUint32LE();
			/* uint32 directoryIndex = */ header->readUint32LE();

			// First read in data needed by us to get at the file data
			FileEntry entry;
			entry.flags = header->readUint16LE();
			entry.uncompressedSize = header->readUint32LE();
			entry.compressedSize = header->readUint32LE();
			header->skip(20);
			entry.offset = header->readUint32LE();
			entry.volume = 0;

			// Then let's get the string
			header->seek(cabDescriptorOffset + fileTableOffset + nameOffset);
			String fileName = header->readString();
			_map[fileName] = entry;
		}
	}

	return true;
}

void InstallShieldCabinet::close() {
	_baseName.clear();
	_map.clear();
	_version = 0;
}

bool InstallShieldCabinet::hasFile(const Path &path) const {
	String name = path.toString();
	return _map.contains(name);
}

int InstallShieldCabinet::listMembers(ArchiveMemberList &list) const {
	for (FileMap::const_iterator it = _map.begin(); it != _map.end(); it++)
		list.push_back(getMember(it->_key));

	return _map.size();
}

const ArchiveMemberPtr InstallShieldCabinet::getMember(const Path &path) const {
	String name = path.toString();
	return ArchiveMemberPtr(new GenericArchiveMember(name, this));
}

SeekableReadStream *InstallShieldCabinet::createReadStreamForMember(const Path &path) const {
	String name = path.toString();
	if (!_map.contains(name))
		return nullptr;

	const FileEntry &entry = _map[name];

	ScopedPtr<SeekableReadStream> stream(SearchMan.createReadStreamForMember(getVolumeName((entry.volume == 0) ? 1 : entry.volume)));
	if (!stream) {
		warning("Failed to open volume for file '%s'", name.c_str());
		return nullptr;
	}

	if (!(entry.flags & 0x04)) {
		// Uncompressed
		// Return a substream
		return new SeekableSubReadStream(stream.release(), entry.offset, entry.offset + entry.uncompressedSize, DisposeAfterUse::YES);
	}

	stream->seek(entry.offset);

#ifdef USE_ZLIB
	byte *src = (byte *)malloc(entry.compressedSize);
	byte *dst = (byte *)malloc(entry.uncompressedSize);

	stream->read(src, entry.compressedSize);

	bool result = inflateZlibInstallShield(dst, entry.uncompressedSize, src, entry.compressedSize);
	free(src);

	if (!result) {
		warning("failed to inflate CAB file '%s'", name.c_str());
		free(dst);
		return nullptr;
	}

	return new MemoryReadStream(dst, entry.uncompressedSize, DisposeAfterUse::YES);
#else
	warning("zlib required to extract compressed CAB file '%s'", name.c_str());
	return 0;
#endif
}

String InstallShieldCabinet::getHeaderName() const {
	return _baseName + "1.hdr";
}

String InstallShieldCabinet::getVolumeName(uint volume) const {
	return String::format("%s%d.cab", _baseName.c_str(), volume);
}

} // End of anonymous namespace

Archive *makeInstallShieldArchive(const String &baseName) {
	InstallShieldCabinet *cab = new InstallShieldCabinet();
	if (!cab->open(baseName)) {
		delete cab;
		return nullptr;
	}

	return cab;
}

} // End of namespace Common
