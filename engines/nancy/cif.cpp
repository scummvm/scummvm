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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/cif.h"
#include "engines/nancy/decompress.h"
#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"

#include "common/memstream.h"
#include "common/substream.h"
#include "common/serializer.h"
#include "common/config-manager.h"

namespace Nancy {

// Reads the data common to standalone .cif files and the ones embedded in a ciftree
static void syncCifInfo(Common::Serializer &ser, CifInfo &info, bool tree) {
	// gross switch of what "version means"
	uint ver = ser.getVersion();
	ser.setVersion(g_nancy->getGameType());
	readRect(ser, info.src, kGameTypeNancy2);
	readRect(ser, info.dest, kGameTypeNancy2);
	ser.setVersion(ver);

	ser.syncAsUint16LE(info.width);
	ser.syncAsUint16LE(info.pitch);
	ser.syncAsUint16LE(info.height);
	ser.syncAsByte(info.depth);

	ser.syncAsByte(info.comp);

	if (tree) {
		ser.syncAsUint32LE(info.dataOffset, 0, 1);
	}

	ser.syncAsUint32LE(info.size);
	ser.skip(4); // A 2nd size for obsolete Cif type 1
	ser.syncAsUint32LE(info.compressedSize);

	ser.syncAsByte(info.type);

	if (!tree) {
		info.dataOffset = ser.bytesSynced();
	}
}

// Reads the data for ciftree cif files
static void syncCiftreeInfo(Common::Serializer &ser, CifInfo &info) {
	uint nameSize = g_nancy->getGameType() <= kGameTypeNancy2 ? 9 : 33;
	byte name[34];
	if (ser.isSaving()) {
		memcpy(name, info.name.toString('/').c_str(), nameSize);
		name[nameSize] = 0;
	}

	ser.syncBytes(name, nameSize);
	name[nameSize] = 0;
	info.name = (char *)name;

	ser.skip(2); // Index of this block

	ser.syncAsUint32LE(info.dataOffset, 2);
	ser.skip(2, 2); // Next id in chain

	syncCifInfo(ser, info, true);

	ser.skip(2, 0, 1); // Next id in chain
}

enum {
	kHashMapSize = 1024
};

CifFile::CifFile(Common::SeekableReadStream *stream, const Common::Path &name) {
	assert(stream);
	_stream = stream;

	_info.name = name;
	Common::Serializer ser(stream, nullptr);
	if (!sync(ser)) {
		return;
	}
}

CifFile::~CifFile() {
	delete _stream;
}

Common::SeekableReadStream *CifFile::createReadStream() const {
	byte *buf = new byte[_info.size];

	bool success = true;

	if (_info.comp == CifInfo::kResCompression) {
		// Decompress the data into the buffer
		if (_stream->seek(_info.dataOffset)) {
			Common::MemoryWriteStream write(buf, _info.size);
			Common::SeekableSubReadStream read(_stream, _info.dataOffset, _info.dataOffset + _info.compressedSize);
			Decompressor dec;
			success = dec.decompress(read, write);
		} else {
			success = false;
		}
	} else {
		if (!_stream->seek(_info.dataOffset) || _stream->read(buf, _info.size) < _info.size) {
			success = false;
		}
	}

	if (!success) {
		warning("Failed to read data for CifFile '%s'", _info.name.toString().c_str());
		delete[] buf;
		_stream->clearErr();
		return nullptr;
	}

	return new Common::MemoryReadStream(buf, _info.size, DisposeAfterUse::YES);
}

Common::SeekableReadStream *CifFile::createReadStreamRaw() const {
	uint size = (_info.comp == CifInfo::kResCompression ? _info.compressedSize : _info.size);
	byte *buf = new byte[size];

	if (!_stream->seek(_info.dataOffset) || _stream->read(buf, size) < size) {
		warning("Failed to read data for CifFile '%s'", _info.name.toString().c_str());
	}

	return new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
}

bool CifFile::sync(Common::Serializer &ser) {
	if (!ser.matchBytes("CIF FILE WayneSikes", 20)) {
		warning("Invalid id string found in CifFile '%s'", _info.name.toString().c_str());
		return false;
	}

	// 4 bytes unused
	ser.skip(4);

	// Version high bytes. These do not change
	uint16 hi = 2;
	ser.syncAsUint16LE(hi);

	uint32 ver = (g_nancy->getGameType() <= kGameTypeNancy1) ? 0 : 1;
	ser.syncAsUint16LE(ver);

	if (ver != 0 && ver != 1) {
		warning("Unsupported version %d found in CifFile '%s'", ver, _info.name.toString().c_str());
		return false;
	}

	if (g_nancy->getGameType() >= kGameTypeNancy6) {
		++ver; // nancy6 made changes to the CifTree structure, but didn't bump the file version
	}

	ser.setVersion(ver);

	syncCifInfo(ser, _info, false);
	return true;
}

CifTree::CifTree(Common::SeekableReadStream *stream, const Common::Path &name) :
		_stream(stream),
		_name(name) {}

CifTree::~CifTree() {
	delete _stream;
}

const CifInfo &CifTree::getCifInfo(const Common::Path &name) const {
	return _fileMap[name];
}

bool CifTree::hasFile(const Common::Path &path) const {
	return _fileMap.contains(path);
}

int CifTree::listMembers(Common::ArchiveMemberList &list) const {
	for (auto &f : _fileMap) {
		list.push_back(Common::ArchiveMemberPtr(new Common::GenericArchiveMember(f._key, *this)));
	}

	return list.size();
}

const Common::ArchiveMemberPtr CifTree::getMember(const Common::Path &path) const {
	if (!hasFile(path)) {
		return Common::ArchiveMemberPtr();
	}

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *CifTree::createReadStreamForMember(const Common::Path &path) const {
	if (!hasFile(path)) {
		return nullptr;
	}

	const CifInfo &info = _fileMap[path];
	byte *buf = new byte[info.size];

	bool success = true;

	if (info.comp == CifInfo::kResCompression) {
		// Decompress the data into the buffer
		if (_stream->seek(info.dataOffset)) {
			Common::MemoryWriteStream write(buf, info.size);
			Common::SeekableSubReadStream read(_stream, info.dataOffset, info.dataOffset + info.compressedSize);
			Decompressor dec;
			success = dec.decompress(read, write);
		} else {
			success = false;
		}
	} else {
		if (!_stream->seek(info.dataOffset) || _stream->read(buf, info.size) < info.size) {
			success = false;
		}
	}

	if (!success) {
		warning("Failed to read data for '%s' from CifTree '%s'", info.name.toString().c_str(), _name.toString().c_str());
		delete[] buf;
		_stream->clearErr();
		return nullptr;
	}

	return new Common::MemoryReadStream(buf, info.size, DisposeAfterUse::YES);
}

Common::SeekableReadStream *CifTree::createReadStreamRaw(const Common::Path &path) const {
	if (!hasFile(path)) {
		return nullptr;
	}

	const CifInfo &info = _fileMap[path];
	uint32 size = (info.comp == CifInfo::kResCompression ? info.compressedSize : info.size);
	byte *buf = new byte[size];

	if (!_stream->seek(info.dataOffset) || _stream->read(buf, size) < size) {
		warning("Failed to read data for '%s' from CifTree '%s'", info.name.toString().c_str(), _name.toString().c_str());
	}

	return new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
}

CifTree *CifTree::makeCifTreeArchive(const Common::String &name, const Common::String &ext) {
	Common::Path path(name);
	path.appendInPlace('.' + ext);

	auto *stream = SearchMan.createReadStreamForMember(path);

	if (!stream) {
		return nullptr;
	}

	CifTree *ret = new CifTree(stream, path);
	Common::Serializer ser(stream, nullptr);

	if (!ret->sync(ser)) {
		delete ret;
		return nullptr;
	}

	return ret;
}

bool CifTree::sync(Common::Serializer &ser) {
	if (!ser.matchBytes("CIF TREE WayneSikes", 20)) {
		warning("Invalid id string found in CifTree '%s'", _name.toString().c_str());
		return false;
	}

	// 4 bytes unused
	ser.skip(4);

	// Version high bytes. These do not change
	uint16 hi = 2;
	ser.syncAsUint16LE(hi);

	uint32 ver = (g_nancy->getGameType() <= kGameTypeNancy1) ? 0 : 1;
	ser.syncAsUint16LE(ver);

	if (ver != 0 && ver != 1) {
		warning("Unsupported version %d found in CifTree '%s'", ver, _name.toString().c_str());
		return false;
	}

	if (g_nancy->getGameType() >= kGameTypeNancy6) {
		++ver; // nancy6 made changes to the CifTree structure, but didn't bump the file version
	}

	ser.setVersion(ver);

	uint16 infoBlockCount = _writeFileMap.size();
	ser.syncAsUint16LE(infoBlockCount);
	ser.skip(2, 1);

	// We will be doing our own hashing, so skip the table built into the tree
	ser.skip(kHashMapSize * 2);

	CifInfo info;
	for (int i = 0; i < infoBlockCount; i++) {
		if (ser.isLoading()) {
			syncCiftreeInfo(ser, info);
			if (info.size && info.type != CifInfo::kResTypeEmpty) {
				_fileMap.setVal(info.name, info);
			}
		} else {
			syncCiftreeInfo(ser, _writeFileMap[i]);
		}
	}

	return true;
}

bool PatchTree::hasFile(const Common::Path &path) const {
	if (CifTree::hasFile(path)) {
		// An association is just a Pair of two StringArrays
		// The first member is an array of Pairs of Strings: a ConfMan property name, and the required value for that ConfMan property
		// The second member is an array with the names of the files to be enabled if all the ConfMan property requirements are satisfied
		for (auto &assoc : _associations) {
			auto &confManProps = assoc.first;
			auto &filenames = assoc.second;
			for (const Common::Path &s : filenames) {
				if (s == path) {
					bool satisfied = true;

					for (uint i = 0; i < confManProps.size(); ++i) {
						// Check all
						if (ConfMan.get(confManProps[i].first, ConfMan.getActiveDomainName()) != confManProps[i].second) {
							satisfied = false;
							break;
						}
					}

					return satisfied;
				}
			}
		}

		// Files without an associated ConfMan ID are always marked as present
		return true;
	}

	return false;
}

} // End of namespace Nancy
