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
#include "common/compression/installshield_cab.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/ptr.h"
#include "common/compression/zlib.h"

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
	enum Flags { kSplit = 1, kObfuscated = 2, kCompressed = 4, kInvalid = 8 };

	struct FileEntry {
		uint32 uncompressedSize;
		uint32 compressedSize;
		uint32 offset;
		uint16 flags;
		uint16 volume;
	};

	struct VolumeHeader {
		int version;
		uint32 cabDescriptorOffset;

		uint32 dataOffset;
		uint32 firstFileIndex;
		uint32 lastFileIndex;
		uint32 firstFileOffset;
		uint32 firstFileSizeUncompressed;
		uint32 firstFileSizeCompressed;
		uint32 lastFileOffset;
		uint32 lastFileSizeUncompressed;
		uint32 lastFileSizeCompressed;
	};

	int _version;
	typedef HashMap<String, FileEntry, IgnoreCase_Hash, IgnoreCase_EqualTo> FileMap;
	FileMap _map;
	String _baseName;
	Common::Array<VolumeHeader> _volumeHeaders;

	static bool readVolumeHeader(SeekableReadStream *volumeStream, VolumeHeader &inVolumeHeader);

	String getHeaderName() const;
	String getVolumeName(uint volume) const;
};

InstallShieldCabinet::InstallShieldCabinet() : _version(0) {
}

bool InstallShieldCabinet::open(const String &baseName) {
	// Store the base name so we can generate file names
	_baseName = baseName;

	uint fileIndex = 0;
	ScopedPtr<SeekableReadStream> file;

	// First, open all the .cab files and read their headers
	uint volume = 1;
	for (;;) {
		file.reset(SearchMan.createReadStreamForMember(getVolumeName(volume++)));
		if (!file.get()) {
			break;
		}

		_volumeHeaders.push_back(VolumeHeader());
		readVolumeHeader(file.get(), _volumeHeaders.back());
	}

	// Try to open a header (.hdr) file to get the file list
	file.reset(SearchMan.createReadStreamForMember(getHeaderName()));
	if (!file) {
		// No header file is present, file list is in first .cab file
		file.reset(SearchMan.createReadStreamForMember(getVolumeName(1)));
	}

	if (!file) {
		close();
		return false;
	}

	VolumeHeader headerHeader;
	if (!readVolumeHeader(file.get(), headerHeader)) {
		close();
		return false;
	}

	_version = headerHeader.version;

	file->seek(headerHeader.cabDescriptorOffset);

	file->skip(12);
	uint32 fileTableOffset = file->readUint32LE();
	file->skip(4);
	uint32 fileTableSize = file->readUint32LE();
	uint32 fileTableSize2 = file->readUint32LE();
	uint32 directoryCount = file->readUint32LE();
	file->skip(8);
	uint32 fileCount = file->readUint32LE();

	if (fileTableSize != fileTableSize2)
		warning("file table sizes do not match");

	// We're ignoring file groups and components since we
	// should not need them. Moving on to the files...

	if (_version >= 6) {
		uint32 fileTableOffset2 = file->readUint32LE();

		for (uint32 j = 0; j < fileCount; j++) {
			file->seek(headerHeader.cabDescriptorOffset + fileTableOffset + fileTableOffset2 + j * 0x57);
			FileEntry entry;
			entry.flags = file->readUint16LE();
			entry.uncompressedSize = file->readUint32LE();
			file->skip(4);
			entry.compressedSize = file->readUint32LE();
			file->skip(4);
			entry.offset = file->readUint32LE();
			file->skip(36);
			uint32 nameOffset = file->readUint32LE();
			/* uint32 directoryIndex = */ file->readUint16LE();
			file->skip(12);
			/* entry.linkPrev = */ file->readUint32LE();
			/* entry.linkNext = */ file->readUint32LE();
			/* entry.linkFlags = */ file->readByte();
			entry.volume = file->readUint16LE();

			// Make sure the entry has a name and data inside the cab
			if (nameOffset == 0 || entry.offset == 0 || (entry.flags & kInvalid))
				continue;

			// Then let's get the string
			file->seek(headerHeader.cabDescriptorOffset + fileTableOffset + nameOffset);
			String fileName = file->readString();
			_map[fileName] = entry;
		}
	} else {
		file->seek(headerHeader.cabDescriptorOffset + fileTableOffset);
		uint32 fileTableCount = directoryCount + fileCount;
		Array<uint32> fileTableOffsets;
		fileTableOffsets.resize(fileTableCount);
		for (uint32 j = 0; j < fileTableCount; j++)
			fileTableOffsets[j] = file->readUint32LE();

		for (uint32 j = directoryCount; j < fileCount + directoryCount; j++) {
			file->seek(headerHeader.cabDescriptorOffset + fileTableOffset + fileTableOffsets[j]);
			uint32 nameOffset = file->readUint32LE();
			/* uint32 directoryIndex = */ file->readUint32LE();

			// First read in data needed by us to get at the file data
			FileEntry entry;
			entry.flags = file->readUint16LE();
			entry.uncompressedSize = file->readUint32LE();
			entry.compressedSize = file->readUint32LE();
			file->skip(20);
			entry.offset = file->readUint32LE();
			entry.volume = 0;

			// Make sure the entry has a name and data inside the cab
			if (nameOffset == 0 || entry.offset == 0 || (entry.flags & kInvalid))
				continue;

			for (uint i = 1; i < _volumeHeaders.size() + 1; ++i) {
				// Check which volume the file is in
				VolumeHeader &volumeHeader = _volumeHeaders[i - 1];
				if (fileIndex >= volumeHeader.firstFileIndex && fileIndex <= volumeHeader.lastFileIndex) {
					entry.volume = i;

					// Check if the file is split across volumes
					if (fileIndex == volumeHeader.lastFileIndex &&
						entry.compressedSize != headerHeader.lastFileSizeCompressed &&
						headerHeader.lastFileSizeCompressed != 0) {
						
						entry.flags |= kSplit;
					}

					break;
				}
			}

			// Then let's get the string
			file->seek(headerHeader.cabDescriptorOffset + fileTableOffset + nameOffset);
			String fileName = file->readString();

			if (entry.volume == 0) {
				warning("Couldn't find the volume for file %s", fileName.c_str());
				close();
				return false;
			}

			++fileIndex;

			_map[fileName] = entry;
		}
	}

	return true;
}

void InstallShieldCabinet::close() {
	_baseName.clear();
	_map.clear();
    _volumeHeaders.clear();
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

    if (entry.flags & kObfuscated) {
        warning("Cannot extract obfuscated file %s", name.c_str());
        return nullptr;
    }

	ScopedPtr<SeekableReadStream> stream(SearchMan.createReadStreamForMember(getVolumeName((entry.volume))));
	if (!stream) {
		warning("Failed to open volume for file '%s'", name.c_str());
		return nullptr;
	}

	byte *src = nullptr;
	if (entry.flags & kSplit) {
		// File is split across volumes
		src = (byte *)malloc(entry.compressedSize);
		uint bytesRead = 0;
		uint volume = entry.volume;

		// Read the first part of the split file
		stream->seek(entry.offset);
		stream->read(src, _volumeHeaders[volume - 1].lastFileSizeCompressed);
		bytesRead += _volumeHeaders[volume - 1].lastFileSizeCompressed;

		// Then, iterate through the next volumes until we've read all the data for the file
		while (bytesRead < entry.compressedSize) {
			stream.reset(SearchMan.createReadStreamForMember(getVolumeName((++volume))));
			if (!stream.get()) {
				warning("Failed to read split file %s", name.c_str());
				free(src);
				return nullptr;
			}
			stream->seek(_volumeHeaders[volume - 1].firstFileOffset);
			stream->read(src + bytesRead, _volumeHeaders[volume - 1].firstFileSizeCompressed);
			bytesRead += _volumeHeaders[volume - 1].firstFileSizeCompressed;
		}
	}

	// Uncompressed file
	if (!(entry.flags & kCompressed)) {
		if (src == nullptr) {
			// File not split, return a substream
			return new SeekableSubReadStream(stream.release(), entry.offset, entry.offset + entry.uncompressedSize, DisposeAfterUse::YES);
		} else {
			// File split, return the assembled data
			return new MemoryReadStream(src, entry.uncompressedSize, DisposeAfterUse::YES);
		}		
	}

#ifdef USE_ZLIB
	byte *dst = (byte *)malloc(entry.uncompressedSize);

	if (!src) {
		src = (byte *)malloc(entry.compressedSize);
		stream->seek(entry.offset);
		stream->read(src, entry.compressedSize);
	}

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
	free(src);
	return 0;
#endif
}

bool InstallShieldCabinet::readVolumeHeader(SeekableReadStream *volumeStream, InstallShieldCabinet::VolumeHeader &inVolumeHeader) {
	// Check for the cab signature
	volumeStream->seek(0);
	uint32 signature = volumeStream->readUint32LE();
	if (signature != 0x28635349) {
		warning("InstallShieldCabinet signature doesn't match: expecting %x but got %x", 0x28635349, signature);
		return false;
	}

	// We support cabinet versions 5 - 13, but do not deobfuscate obfuscated files

	uint32 magicBytes = volumeStream->readUint32LE();
	int shift = magicBytes >> 24;
	inVolumeHeader.version = shift == 1 ? (magicBytes >> 12) & 0xf : (magicBytes & 0xffff) / 100;
	inVolumeHeader.version = (inVolumeHeader.version == 0) ? 5 : inVolumeHeader.version;

	if (inVolumeHeader.version < 5 || inVolumeHeader.version > 13) {
		warning("Unsupported CAB version %d, magic bytes %08x", inVolumeHeader.version, magicBytes);
		return false;
	}

	/* uint32 volumeInfo = */ volumeStream->readUint32LE();
	inVolumeHeader.cabDescriptorOffset = volumeStream->readUint32LE();
	/* uint32 cabDescriptorSize = */ volumeStream->readUint32LE();

	// Read the version-specific part of the header
	if (inVolumeHeader.version == 5) {
		inVolumeHeader.dataOffset = volumeStream->readUint32LE();
		volumeStream->skip(4);
		inVolumeHeader.firstFileIndex = volumeStream->readUint32LE();
		inVolumeHeader.lastFileIndex = volumeStream->readUint32LE();
		inVolumeHeader.firstFileOffset = volumeStream->readUint32LE();
		inVolumeHeader.firstFileSizeUncompressed = volumeStream->readUint32LE();
		inVolumeHeader.firstFileSizeCompressed = volumeStream->readUint32LE();
		inVolumeHeader.lastFileOffset = volumeStream->readUint32LE();
		inVolumeHeader.lastFileSizeUncompressed = volumeStream->readUint32LE();
		inVolumeHeader.lastFileSizeCompressed = volumeStream->readUint32LE();
	} else {
		inVolumeHeader.dataOffset = volumeStream->readUint32LE();
		volumeStream->skip(4);
		inVolumeHeader.firstFileIndex = volumeStream->readUint32LE();
		inVolumeHeader.lastFileIndex = volumeStream->readUint32LE();
		inVolumeHeader.firstFileOffset = volumeStream->readUint32LE();
		volumeStream->skip(4);
		inVolumeHeader.firstFileSizeUncompressed = volumeStream->readUint32LE();
		volumeStream->skip(4);
		inVolumeHeader.firstFileSizeCompressed = volumeStream->readUint32LE();
		volumeStream->skip(4);
		inVolumeHeader.lastFileOffset = volumeStream->readUint32LE();
		volumeStream->skip(4);
		inVolumeHeader.lastFileSizeUncompressed = volumeStream->readUint32LE();
		volumeStream->skip(4);
		inVolumeHeader.lastFileSizeCompressed = volumeStream->readUint32LE();
		volumeStream->skip(4);
	}

	return true;
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
