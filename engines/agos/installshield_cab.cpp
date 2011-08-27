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
#include "common/file.h"
#include "common/memstream.h"
#include "common/zlib.h"

namespace AGOS {

class InstallShieldCabinet : public Common::Archive {
	Common::String _installShieldFilename;

public:
	InstallShieldCabinet(const Common::String &filename);
	~InstallShieldCabinet();

	// Common::Archive API implementation
	bool hasFile(const Common::String &name);
	int listMembers(Common::ArchiveMemberList &list);
	Common::ArchiveMemberPtr getMember(const Common::String &name);
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;

private:
	struct FileEntry {
		uint32 uncompressedSize;
		uint32 compressedSize;
		uint32 offset;
		uint16 flags;
	};

	typedef Common::HashMap<Common::String, FileEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
	FileMap _map;
};

InstallShieldCabinet::~InstallShieldCabinet() {
	_map.clear();
}

InstallShieldCabinet::InstallShieldCabinet(const Common::String &filename) : _installShieldFilename(filename) {
	Common::File installShieldFile;

	if (!installShieldFile.open(_installShieldFilename)) {
		warning("InstallShieldCabinet::InstallShieldCabinet(): Could not find the archive file %s", _installShieldFilename.c_str());
		return;
	}

	// Note that we only support a limited subset of cabinet files
	// Only single cabinet files and ones without data shared between
	// cabinets.

	// Check for the magic uint32
	if (installShieldFile.readUint32LE() != 0x28635349) {
		warning("InstallShieldCabinet::InstallShieldCabinet(): Magic ID doesn't match");
		return;
	}

	uint32 version = installShieldFile.readUint32LE();

	if (version != 0x01000004) {
		warning("Unsupported CAB version %08x", version);
		return;
	}

	/* uint32 volumeInfo = */ installShieldFile.readUint32LE();
	uint32 cabDescriptorOffset = installShieldFile.readUint32LE();
	/* uint32 cabDescriptorSize = */ installShieldFile.readUint32LE();

	installShieldFile.seek(cabDescriptorOffset);

	installShieldFile.skip(12);
	uint32 fileTableOffset = installShieldFile.readUint32LE();
	installShieldFile.skip(4);
	uint32 fileTableSize = installShieldFile.readUint32LE();
	uint32 fileTableSize2 = installShieldFile.readUint32LE();
	uint32 directoryCount = installShieldFile.readUint32LE();
	installShieldFile.skip(8);
	uint32 fileCount = installShieldFile.readUint32LE();

	if (fileTableSize != fileTableSize2)
		warning("file table sizes do not match");

	// We're ignoring file groups and components since we
	// should not need them. Moving on to the files...

	installShieldFile.seek(cabDescriptorOffset + fileTableOffset);
	uint32 fileTableCount = directoryCount + fileCount;
	uint32 *fileTableOffsets = new uint32[fileTableCount];
	for (uint32 i = 0; i < fileTableCount; i++)
		fileTableOffsets[i] = installShieldFile.readUint32LE();

	for (uint32 i = directoryCount; i < fileCount + directoryCount; i++) {
		installShieldFile.seek(cabDescriptorOffset + fileTableOffset + fileTableOffsets[i]);
		uint32 nameOffset = installShieldFile.readUint32LE();
		/* uint32 directoryIndex = */ installShieldFile.readUint32LE();

		// First read in data needed by us to get at the file data
		FileEntry entry;
		entry.flags = installShieldFile.readUint16LE();
		entry.uncompressedSize = installShieldFile.readUint32LE();
		entry.compressedSize = installShieldFile.readUint32LE();
		installShieldFile.skip(20);
		entry.offset = installShieldFile.readUint32LE();

		// Then let's get the string
		installShieldFile.seek(cabDescriptorOffset + fileTableOffset + nameOffset);
		Common::String fileName;

		char c = installShieldFile.readByte();
		while (c) {
			fileName += c;
			c = installShieldFile.readByte();
		}
		_map[fileName] = entry;
	}

	delete[] fileTableOffsets;
}

bool InstallShieldCabinet::hasFile(const Common::String &name) {
	warning("hasFile: Filename %s", name.c_str());
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
	if (!_map.contains(name))
		return 0;

	const FileEntry &entry = _map[name];

	Common::File archiveFile;
	archiveFile.open(_installShieldFilename);
	archiveFile.seek(entry.offset);

	if (!(entry.flags & 0x04)) {
		// Not compressed
		return archiveFile.readStream(entry.uncompressedSize);
	}

#ifdef USE_ZLIB
	byte *src = (byte *)malloc(entry.compressedSize);
	byte *dst = (byte *)malloc(entry.uncompressedSize);

	archiveFile.read(src, entry.compressedSize);

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

Common::Archive *makeInstallShieldArchive(const Common::String &name) {
	return new InstallShieldCabinet(name);
}

} // End of namespace AGOS
