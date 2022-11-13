/* ScummVM Tools
 *
 * ScummVM Tools is the legal property of its developers, whose names
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

#include "common/endian.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/file.h"
#include "common/util.h"
#include "common/memstream.h"
#include "common/dcl.h"
#include "mps_installer.h"

namespace MADS {

MpsInstaller* MpsInstaller::open(const Common::Path& baseName) {
	Common::File indexFile;
	FileMap _files;

	if (!indexFile.open(baseName.append(".IDX")))
		return nullptr;

	uint32 filecnt = indexFile.readUint16LE();

	indexFile.skip(10);

	uint32 indexSize = indexFile.size();

	if (filecnt > (indexSize - 12) / sizeof(FileDescriptorBin))
		filecnt = (indexSize - 12) / sizeof(FileDescriptorBin);

	for (uint i = 0; i < filecnt; i++) {
		FileDescriptorBin descBin;
		indexFile.read(&descBin, sizeof(descBin));
		FileDescriptor desc(descBin);
		_files[desc._fileName] = desc;
	}

	return new MpsInstaller(_files, baseName);
}

// Use of \\ as path separator is a guess. Never seen an archive with subfolders
static Common::String translateName(const Common::Path &path) {
	return Common::normalizePath(path.toString('\\'), '\\');
}

bool MpsInstaller::hasFile(const Common::Path &path) const {
	return _files.contains(translateName(path));
}

int MpsInstaller::listMembers(Common::ArchiveMemberList &list) const {
	for (FileMap::const_iterator i = _files.begin(), end = _files.end(); i != end; ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, this)));
	}

	return _files.size();
}

const Common::ArchiveMemberPtr MpsInstaller::getMember(const Common::Path &path) const {
	Common::String translated = translateName(path);
	if (!_files.contains(translated))
		return nullptr;

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(_files.getVal(translated)._fileName, this));
}

// TODO: Make streams stay valid after destruction of archive
Common::SeekableReadStream *MpsInstaller::createReadStreamForMember(const Common::Path &path) const {
	Common::String translated = translateName(path);
	if (!_files.contains(translated))
		return nullptr;
	FileDescriptor desc = _files.getVal(translated);
	if (_cache.contains(desc._fileName)) {
		return new Common::MemoryReadStream(_cache[desc._fileName].get(), desc._uncompressedSize, DisposeAfterUse::NO);
	}

	if (desc._compressionAlgo != 0 && desc._compressionAlgo != 1) {
		debug ("Unsupported compression algorithm %d for %s", desc._compressionAlgo, desc._fileName.c_str());
		return nullptr;
	}

       
	uint32 off = desc._offsetInVolume;
	uint vol = desc._volumeNumber;
	uint32 rem = desc._compressedSize;
	byte *compressedBuf = new byte[rem];
	byte *outptr = compressedBuf;
	while (rem > 0) {
		Common::File fvol;
		Common::Path volumePath = _baseName.append(Common::String::format(".%03d", vol));
		if (!fvol.open(volumePath)) {
			error("Failed to open volume %s.%03d", volumePath.toString().c_str(), vol);
			delete[] compressedBuf;
			return nullptr;
		}
		fvol.seek(off);
		int32 actual = fvol.read(outptr, rem);
		if (actual <= 0) {
			warning("Read failure in volume %s.%03d", volumePath.toString().c_str(), vol);
			delete[] compressedBuf;
			return nullptr;
		}
	       
		rem -= actual;
		outptr += actual;
		if (actual == 0)
			break;
		vol++;
		off = 0;
	}

	byte *uncompressedBuf;
	uint32 uncompressedSize = desc._uncompressedSize;
	switch (desc._compressionAlgo) {
	case 0:
		uncompressedBuf = compressedBuf;
		uncompressedSize = desc._compressedSize;
		compressedBuf = nullptr;
		break;
	case 1:
		Common::MemoryReadStream compressedReadStream(compressedBuf, desc._compressedSize);
		uncompressedBuf = new byte[uncompressedSize];
		if (!Common::decompressDCL(&compressedReadStream, uncompressedBuf, desc._compressedSize, uncompressedSize)) {
			delete[] compressedBuf;
			delete[] uncompressedBuf;
			error("Unable to decompress %s", desc._fileName.c_str());
			return nullptr;
		}
		delete[] compressedBuf;
		compressedBuf = nullptr;
				
		break;
	}

	_cache[desc._fileName].reset(uncompressedBuf);
	// TODO: Make it configurable to read directly from disk, at least in the uncompressed case
	return new Common::MemoryReadStream(uncompressedBuf, desc._uncompressedSize, DisposeAfterUse::NO);
}
}
