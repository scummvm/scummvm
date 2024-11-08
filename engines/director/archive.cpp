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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/macresman.h"
#include "common/memstream.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/movie.h"
#include "director/window.h"

namespace Director {

// Base Archive code

Archive::Archive() {
	_stream = nullptr;
	_isBigEndian = true;
}

Archive::~Archive() {
	close();
}

Common::String Archive::getFileName() const { return Director::getFileName(_pathName.toString(g_director->_dirSeparator)); }

bool Archive::openFile(const Common::Path &path) {
	Common::File *file = new Common::File();

	if (path.empty() || !file->open(path)) {
		warning("Archive::openFile(): Error opening file %s", path.toString(Common::Path::kNativeSeparator).c_str());
		delete file;
		return false;
	}

	_pathName = path.toString(g_director->_dirSeparator);

	if (!openStream(file)) {
		warning("Archive::openFile(): Error loading stream from file %s", path.toString(Common::Path::kNativeSeparator).c_str());
		close();
		return false;
	}

	return true;
}

void Archive::close() {
	listUnaccessedChunks();

	_types.clear();

	if (_stream)
		delete _stream;

	_stream = nullptr;
}

void Archive::listUnaccessedChunks() {
	Common::String s;

	for (const auto &type : _types) {
		bool accessed = false;

		for (const auto &res : type._value) {
			if (res._value.accessed) {
				accessed = true;
				break;
			}
		}

		if (!accessed) {
			switch (type._key) {
			case MKTAG('f','r','e','e'):	// Freed chunk
			case MKTAG('j','u','n','k'):	// Some unreferenced junk
			case MKTAG('T','H','U','M'):	// Cast thumbnail - authoring only
			case MKTAG('T','h','u','m'):	// Cast thumbnail - authoring only (D7+)
			case MKTAG('F','C','O','L'):	// Favorite colors - authoring only
			case MKTAG('S','C','R','F'):	// Shared Cast refs - authoring only
			case MKTAG('V','W','t','c'):	// Score time code
			case MKTAG('V','W','t','k'):	// Tape Key resource. Used as a lookup for labels in early Directors
				break;
			default:
				s += Common::String::format("%s: %d items\n", tag2str(type._key), type._value.size());
			}
		}
	}

	if (!s.empty())
		debugC(5, kDebugLoading, "Unaccessed Chunks in '%s':\n%s", _pathName.toString(g_director->_dirSeparator).c_str(), s.c_str());
}

uint32 Archive::getFileSize() {
	if (!_stream)
		return 0;

	return _stream->size();
}

bool Archive::hasResource(uint32 tag, int id) const {
	if (!_types.contains(tag))
		return false;

	if (id == -1)
		return true;

	return _types[tag].contains(id);
}

bool Archive::hasResource(uint32 tag, const Common::String &resName) const {
	if (!_types.contains(tag) || resName.empty())
		return false;

	const ResourceMap &resMap = _types[tag];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		if (it->_value.name.matchString(resName))
			return true;

	return false;
}

Common::SeekableReadStreamEndian *Archive::getFirstResource(uint32 tag) {
	return getResource(tag, getResourceIDList(tag)[0]);
}

Common::SeekableReadStreamEndian *Archive::getFirstResource(uint32 tag, uint16 parentId) {
	return getResource(tag, getResourceIDList(tag)[0]);
}

Common::SeekableReadStreamEndian *Archive::getResource(uint32 tag, uint16 id) {
	if (!_types.contains(tag))
		error("Archive::getResource(): Archive does not contain '%s' %d", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive::getResource(): Archive does not contain '%s' %d", tag2str(tag), id);

	const Resource &res = resMap[id];
	auto stream = new Common::SeekableSubReadStream(_stream, res.offset, res.offset + res.size, DisposeAfterUse::NO);

	_types[tag][id].accessed = true; // Mark it as accessed

	return new Common::SeekableReadStreamEndianWrapper(stream, _isBigEndian, DisposeAfterUse::YES);
}

Resource Archive::getResourceDetail(uint32 tag, uint16 id) {
	if (!_types.contains(tag))
		error("Archive::getResourceDetail(): Archive does not contain '%s' %d", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive::getResourceDetail(): Archive does not contain '%s' %d", tag2str(tag), id);

	return resMap[id];
}

uint32 Archive::getOffset(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		error("Archive::getOffset(): Archive does not contain '%s' %d", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive::getOffset(): Archive does not contain '%s' %d", tag2str(tag), id);

	return resMap[id].offset;
}

uint16 Archive::findResourceID(uint32 tag, const Common::String &resName, bool ignoreCase) const {
	if (!_types.contains(tag) || resName.empty())
		return 0xFFFF;

	const ResourceMap &resMap = _types[tag];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		if (it->_value.name.matchString(resName, ignoreCase))
			return it->_key;

	return 0xFFFF;
}

Common::String Archive::getName(uint32 tag, uint16 id) const {
	if (!_types.contains(tag))
		error("Archive::getName(): Archive does not contain '%s' %d", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("Archive::getName(): Archive does not contain '%s' %d", tag2str(tag), id);

	return resMap[id].name;
}

Common::SeekableReadStreamEndian *Archive::getMovieResourceIfPresent(uint32 tag) {
	if (g_director->getVersion() >= 400) {
		if (_movieChunks.contains(tag) && hasResource(tag, _movieChunks[tag]))
			return getResource(tag, _movieChunks[tag]);
	} else if (hasResource(tag, -1)) {
		return getFirstResource(tag);
	}

	return nullptr;
}

Common::Array<uint32> Archive::getResourceTypeList() const {
	Common::Array<uint32> typeList;

	for (TypeMap::const_iterator it = _types.begin(); it != _types.end(); it++)
		typeList.push_back(it->_key);

	return typeList;
}

Common::Array<uint16> Archive::getResourceIDList(uint32 type) const {
	Common::Array<uint16> idList;

	if (!_types.contains(type))
		return idList;

	const ResourceMap &resMap = _types[type];

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		idList.push_back(it->_key);

	return idList;
}

uint32 Archive::convertTagToUppercase(uint32 tag) {
	uint32 newTag = toupper(tag >> 24) << 24;
	newTag |= toupper((tag >> 16) & 0xFF) << 16;
	newTag |= toupper((tag >> 8) & 0xFF) << 8;

	return newTag | toupper(tag & 0xFF);
}

void Archive::dumpChunk(Resource &res, Common::DumpFile &out) {
	byte *data = nullptr;
	uint dataSize = 0;

	Common::SeekableReadStreamEndian *resStream = getResource(res.tag, res.index);
	if (!resStream) {
		return;
	}
	uint32 len = resStream->size();

	if (dataSize < len) {
		data = (byte *)malloc(resStream->size());
		dataSize = resStream->size();
	}

	Common::Path prepend = _pathName.empty() ? _pathName : "stream";
	Common::Path filename(Common::String::format("./dumps/%s-%s-%d", encodePathForDump(prepend.toString(g_director->_dirSeparator)).c_str(), tag2str(res.tag), res.index), '/');
	resStream->read(data, len);

	if (!out.open(filename, true)) {
		warning("Archive::dumpChunk(): Can not open dump file %s", filename.toString(Common::Path::kNativeSeparator).c_str());
	} else {
		out.write(data, len);
		out.flush();
		out.close();
	}

	delete resStream;
	free(data);
}

Common::String Archive::formatArchiveInfo() {
	Common::String result = "unknown, ";
	if (_isBigEndian)
		result += "big endian,";
	else
		result += "little endian, ";
	return result;
}

// Mac Archive code

MacArchive::MacArchive() : Archive(), _resFork(nullptr) {
}

MacArchive::~MacArchive() {
	delete _resFork;
}

uint32 MacArchive::getFileSize() {
	if (!_resFork)
		return 0;

	return _resFork->getResForkDataSize();
}

void MacArchive::close() {
	Archive::close();
	delete _resFork;
	_resFork = nullptr;
}

bool MacArchive::openFile(const Common::Path &path) {
	close();

	_resFork = new Common::MacResManager();

	if (path.empty() || !_resFork->open(path) || !_resFork->hasResFork()) {
		close();
		return false;
	}

	_pathName = _resFork->getBaseFileName();
	Common::String basename(_pathName.baseName());
	if (basename.hasSuffix(".bin")) {
		for (int i = 0; i < 4; i++)
			basename.deleteLastChar();
		_pathName = _pathName.getParent().appendComponent(basename);
	}

	readTags();

	return true;
}

bool MacArchive::openStream(Common::SeekableReadStream *stream, uint32 startOffset) {
	close();

	if (startOffset)
		error("MacArchive::openStream(): startOffset > 0 is not yet implemented");

	_resFork = new Common::MacResManager();
	stream->seek(startOffset);

	if (!_resFork->loadFromMacBinary(stream)) {
		warning("MacArchive::openStream(): Error loading Mac Binary");
		close();
		return false;
	}

	_pathName = "<stream>";
	_resFork->setBaseFileName(Common::Path(_pathName));

	readTags();

	return true;
}

void MacArchive::readTags() {
	Common::MacResTagArray tagArray = _resFork->getResTagArray();
	Common::DumpFile out;

	for (uint32 i = 0; i < tagArray.size(); i++) {
		ResourceMap resMap;
		Common::MacResIDArray idArray = _resFork->getResIDArray(tagArray[i]);

		for (uint32 j = 0; j < idArray.size(); j++) {
			// Avoid assigning invalid entries to _types, because other
			// functions will assume they exist and are valid if listed.
			Common::SeekableReadStream *readStream = _resFork->getResource(tagArray[i], idArray[j]);
			if (readStream == nullptr) {
				continue;
			}
			delete readStream;

			Resource &res = resMap[idArray[j]];

			res.offset = res.size = 0; // unused
			res.name = _resFork->getResName(tagArray[i], idArray[j]);
			res.tag = tagArray[i];
			res.index = idArray[j];
			res.accessed = false;
			debugC(3, kDebugLoading, "MacArchive::readTags(): Found MacArchive resource '%s' %d: %s", tag2str(tagArray[i]), idArray[j], res.name.c_str());
			if (ConfMan.getBool("dump_scripts"))
				dumpChunk(res, out);

			res.accessed = false; // Clear flag so stats are not spoiled
		}

		// Don't assign a 0-entry resMap to _types.
		if (resMap.size() > 0) {
			 _types[tagArray[i]] = resMap;
		}
	}
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "MacArchive::readTags(): Resources found:");
		for (const auto &it : _types) {
			debugC(5, kDebugLoading, "%s: %d", tag2str(it._key), it._value.size());
		}
	}
}

Common::SeekableReadStreamEndian *MacArchive::getResource(uint32 tag, uint16 id) {
	assert(_resFork);
	Common::SeekableReadStream *stream = _resFork->getResource(tag, id);

	if (stream == nullptr) {
		error("MacArchive::getResource(): Archive does not contain '%s' %d", tag2str(tag), id);
	}

	_types[tag][id].accessed = true; // Mark it as accessed

	return new Common::SeekableReadStreamEndianWrapper(stream, true, DisposeAfterUse::YES);
}

Common::String MacArchive::formatArchiveInfo() {
	Common::String result = "Mac resource fork, ";
	if (_isBigEndian)
		result += "big endian";
	else
		result += "little endian";
	return result;
}

// RIFF Archive code

bool RIFFArchive::openStream(Common::SeekableReadStream *stream, uint32 startOffset) {
	close();

	_startOffset = startOffset;

	stream->seek(startOffset);

	_stream = stream;

	uint32 tag = stream->readUint32BE();

	if (convertTagToUppercase(tag) != MKTAG('R', 'I', 'F', 'F')) {
		debugC(5, kDebugLoading, "RIFFArchive::openStream(): RIFF expected but got '%s'", tag2str(tag));
		return false;
	}

	stream->readUint32LE(); // size

	tag = stream->readUint32BE();

	if (convertTagToUppercase(tag) != MKTAG('R', 'M', 'M', 'P')) {
		debugC(5, kDebugLoading, "RIFFArchive::openStream(): RMMP expected but  got '%s'", tag2str(tag));
		return false;
	}

	tag = stream->readUint32BE();
	if (convertTagToUppercase(tag) != MKTAG('C', 'F', 'T', 'C')) {
		debugC(5, kDebugLoading, "RIFFArchive::openStream(): CFTC expected but  got '%s'", tag2str(tag));
		return false;
	}

	uint32 cftcSize = stream->readUint32LE();
	uint32 startPos = stream->pos();
	stream->readUint32LE(); // Chunk number, alsways 0

	Common::DumpFile out;

	while ((uint32)stream->pos() < startPos + cftcSize) {
		tag = convertTagToUppercase(stream->readUint32BE());

		uint32 size = stream->readUint32LE();
		uint32 id = stream->readUint32LE();
		uint32 offset = stream->readUint32LE();

		if (tag == 0)
			break;

		uint32 startResPos = stream->pos();
		stream->seek(startOffset + offset + 12);

		Common::String name = "";
		byte nameSize = stream->readByte();

		if (nameSize) {
			bool skip = false;
			for (uint8 i = 0; i < nameSize; i++) {
				byte b = stream->readByte();

				if (!b)
					skip = true;

				if (!skip)
					name += b;
			}
		}

		debugC(3, kDebugLoading, "Found RIFF resource '%s' %d: %d @ 0x%08x (0x%08x)", tag2str(tag), id, size, offset, startOffset + offset);

		Resource &res = _types[tag][id];
		res.index = id;
		res.offset = offset;
		res.size = size;
		res.name = name;
		res.tag = tag;
		res.accessed = false;

		if (ConfMan.getBool("dump_scripts"))
			dumpChunk(res, out);

		res.accessed = false; // Clear flag so stats are not spoiled

		stream->seek(startResPos);
	}

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "RIFFArchive::openStream(): Resources found:");
		for (const auto &it : _types) {
			debugC(5, kDebugLoading, "%s: %d", tag2str(it._key), it._value.size());
		}
	}
	return true;
}

Common::SeekableReadStreamEndian *RIFFArchive::getResource(uint32 tag, uint16 id) {
	if (!_types.contains(tag))
		error("RIFFArchive::getResource(): Archive does not contain '%s' %d", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("RIFFArchive::getResource(): Archive does not contain '%s' %d", tag2str(tag), id);

	const Resource &res = resMap[id];

	// Adjust to skip the resource header
	// FourCC, size and id (all 4 bytes each)
	// Resource size excludes FourCC and size.
	uint32 offset = res.offset + 12;
	uint32 size = res.size - 4;

	// Skip the Pascal string
	_stream->seek(_startOffset + offset);
	byte stringSize = _stream->readByte(); // 1 for this byte

	offset += stringSize + 1;
	size -= stringSize + 1;

	// 'DIB ' was observed to always have two 0 bytes
	// other resources such as 'SND ' need alignment to nearest word boundary.
	bool needsAlignment = offset & 1;
	if (needsAlignment || tag == MKTAG('D', 'I', 'B', ' ')) {
		offset++;
		size--;
	}

	debugC(4, kDebugLoading, "RIFFArchive::getResource() tag: %s id: %i offset: %i size: %i", tag2str(tag), id, res.offset, res.size);

	_types[tag][id].accessed = true; // Mark it as accessed

	auto stream = new Common::SeekableSubReadStream(_stream, _startOffset + offset, _startOffset + offset + size, DisposeAfterUse::NO);
	return new Common::SeekableReadStreamEndianWrapper(stream, true, DisposeAfterUse::YES);
}

Common::String RIFFArchive::formatArchiveInfo() {
	Common::String result = "RIFF, ";
	if (_isBigEndian)
		result += "big endian";
	else
		result += "little endian";
	return result;
}

// RIFX Archive code

RIFXArchive::RIFXArchive() : Archive() {
	_isBigEndian = true;
	_rifxType = 0;
	_ilsBodyOffset = 0;
}

RIFXArchive::~RIFXArchive() {
	for (auto &it : _ilsData)
		free(it._value);
}

bool RIFXArchive::openStream(Common::SeekableReadStream *stream, uint32 startOffset) {
	if (stream != _stream) {
		close();
		_stream = stream;
	}

	stream->seek(startOffset);

	uint32 moreOffset = 0;

	uint32 headerTag = stream->readUint32BE();

	if (headerTag != MKTAG('R', 'I', 'F', 'X') &&
		headerTag != MKTAG('X', 'F', 'I', 'R')) {
		// Check if it is MacBinary

		stream->seek(startOffset);

		if (Common::MacResManager::isMacBinary(*stream)) {
			warning("RIFXArchive::openStream(): MacBinary detected, overriding");

			// We need to look at the resource fork to detect XCOD resources
			Common::SeekableSubReadStream *macStream = new Common::SeekableSubReadStream(stream, 0, stream->size());
			MacArchive *macArchive = new MacArchive();
			if (macArchive->openStream(macStream)) {
				g_director->getCurrentWindow()->probeResources(macArchive);
			} else {
				delete macStream;
			}
			delete macArchive;

			// Then read the data fork
			moreOffset = Common::MacResManager::getDataForkOffset();
			stream->seek(startOffset + moreOffset);

			headerTag = stream->readUint32BE();
		}
	}

	if (headerTag == MKTAG('R', 'I', 'F', 'X')) {
		_isBigEndian = true;
	} else if (SWAP_BYTES_32(headerTag) == MKTAG('R', 'I', 'F', 'X')) {
		_isBigEndian = false;
	} else {
		warning("RIFXArchive::openStream(): RIFX or XFIR expected but %s found", tag2str(headerTag));
		return false;
	}

	Common::SeekableReadStreamEndianWrapper endianStream(stream, _isBigEndian, DisposeAfterUse::NO);
	endianStream.seek(startOffset + moreOffset + 4);

	uint32 sz = endianStream.readUint32() + 8; // size

	// If it is an embedded file, dump it if requested.
	// Start by copying the movie data to a new buffer.
	byte *dumpData = nullptr;
	Common::SeekableMemoryWriteStream *dumpStream = nullptr;
	if (ConfMan.getBool("dump_scripts") && startOffset) {
		dumpData = (byte *)malloc(sz);
		dumpStream = new Common::SeekableMemoryWriteStream(dumpData, sz);
		stream->seek(startOffset);
		stream->read(dumpData, sz);
		stream->seek(startOffset + 8);

		// Add the padding data to match the file size
		endianStream.seek(sz - 4);
		uint32 _junk = endianStream.readUint32();
		if (_junk != 0) {
			dumpStream->seek(sz - 4);
			dumpStream->writeUint32BE(0);
		}

		endianStream.seek(startOffset + moreOffset + 8);
	}

	_rifxType = endianStream.readUint32();
	warning("RIFX: type: %s", tag2str(_rifxType));

	// Now read the memory map.
	// At the same time, we will patch the offsets in the dump data.
	bool readMapSuccess = false;
	switch (_rifxType) {
	case MKTAG('M', 'V', '9', '3'):
	case MKTAG('M', 'C', '9', '5'):
		readMapSuccess = readMemoryMap(endianStream, moreOffset, dumpStream, startOffset);
		break;
	case MKTAG('A', 'P', 'P', 'L'):
		readMapSuccess = readMemoryMap(endianStream, moreOffset, dumpStream, startOffset);
		break;
	case MKTAG('F', 'G', 'D', 'M'):
	case MKTAG('F', 'G', 'D', 'C'):
		readMapSuccess = readAfterburnerMap(endianStream, moreOffset);
		break;
	default:
		break;
	}

	_types[MKTAG('R', 'I', 'F', 'X')][0].accessed = true; // Mark it as accessed

	// Now that the dump data has been patched, actually dump it.
	if (dumpData) {
		Common::DumpFile out;

		char buf[256];
		Common::sprintf_s(buf, "./dumps/%s-%08x", encodePathForDump(g_director->getEXEName()).c_str(), startOffset);

		if (out.open(buf, true)) {
			out.write(dumpData, sz);
			out.flush();
			out.close();
		} else {
			warning("RIFXArchive::openStream(): Can not open dump file %s", buf);
		}
	}
	free(dumpData);
	delete dumpStream;

	// If we couldn't read the map, we can't do anything past this point.
	if (!readMapSuccess)
		return false;

	if (_rifxType == MKTAG('A', 'P', 'P', 'L')) {
		if (hasResource(MKTAG('F', 'i', 'l', 'e'), -1)) {
			// Replace this archive with the embedded archive.
			Common::Array<uint16> subFiles = getResourceIDList(MKTAG('F', 'i', 'l', 'e'));
			uint numFile;
			int32 fileOffset = 0;
			for (numFile = 0; numFile < subFiles.size(); numFile++) {
				uint32 fileId = subFiles[numFile];
				fileOffset = _resources[fileId]->offset;
				endianStream.seek(fileOffset + 8);
				uint32 tag = endianStream.readUint32();
				if (tag != MKTAG('X', 't', 'r', 'a') && tag != MKTAG('a', 'r', 't', 'X'))
					break;
			}
			if (numFile < subFiles.size()) {
				_types.clear();
				_resources.clear();
				return openStream(_stream, fileOffset);
			}
		}

		warning("No 'File' resource present in APPL archive");
		return false;
	}

	if (ConfMan.getBool("dump_scripts")) {
		debug("RIFXArchive::openStream(): Dumping %d resources", _resources.size());

		Common::DumpFile out;

		for (uint i = 0; i < _resources.size(); i++) {
			if ((_rifxType == MKTAG('F', 'G', 'D', 'M') || _rifxType == MKTAG('F', 'G', 'D', 'C')) && _resources[i]->index < 3) {
				// This is in the initial load segment and can't be read like a normal chunk.
				continue;
			}
			dumpChunk(*_resources[i], out);
		}
	}

	// A KEY* must be present
	if (!hasResource(MKTAG('K', 'E', 'Y', '*'), -1)) {
		warning("No 'KEY*' resource present");
	} else {
		// Parse the KEY*
		Common::SeekableReadStreamEndian *keyStream = getFirstResource(MKTAG('K', 'E', 'Y', '*'), true);
		readKeyTable(*keyStream);
		delete keyStream;
	}

	// Parse the CAS* for each library, if present
	uint32 casTag = MKTAG('C', 'A', 'S', '*');
	if (_keyData.contains(casTag)) {
		for (auto &it : _keyData[casTag]) {
			for (auto &jt : it._value) {
				if (Common::SeekableReadStreamEndian *casStream = getResource(casTag, jt)) {
					Resource res = getResourceDetail(casTag, jt);
					readCast(*casStream, it._key);
					delete casStream;
				}
			}
		}
	}

	return true;
}

bool RIFXArchive::readMemoryMap(Common::SeekableReadStreamEndian &stream, uint32 moreOffset, Common::SeekableMemoryWriteStream *dumpStream, uint32 movieStartOffset) {
	if (stream.readUint32() != MKTAG('i', 'm', 'a', 'p'))
		return false;

	_types[MKTAG('i', 'm', 'a', 'p')][0].accessed = true; // Mark it as accessed

	stream.readUint32(); // imap length
	uint32 mapversion = stream.readUint32(); // version, seen 0 or 1
	uint32 mmapOffsetPos = stream.pos();
	uint32 mmapOffset = stream.readUint32() + moreOffset;
	if (dumpStream) {
		// If we're dumping the movie, patch this offset in the dump data.
		dumpStream->seek(mmapOffsetPos - movieStartOffset);
		if (stream.isBE())
			dumpStream->writeUint32BE(mmapOffset - movieStartOffset);
		else
			dumpStream->writeUint32LE(mmapOffset - movieStartOffset);
	}
	uint32 version = stream.readUint32(); // 0 for 4.0, 0x4c1 for 5.0, 0x4c7 for 6.0, 0x708 for 8.5, 0x742 for 10.0
	warning("mmap: mapversion: %d version: %x offset: 0x%x (%d)", mapversion, version, mmapOffset, mmapOffset);

	stream.seek(mmapOffset);

	if (stream.readUint32() != MKTAG('m', 'm', 'a', 'p')) {
		warning("RIFXArchive::readMemoryMap(): mmap expected but not found");
		return false;
	}

	_types[MKTAG('m', 'm', 'a', 'p')][0].accessed = true; // Mark it as accessed

	stream.readUint32(); // mmap length
	stream.readUint16(); // header size
	stream.readUint16(); // size of map entry
	stream.readUint32(); // resCount + empty entries
	uint32 resCount = stream.readUint32();
	stream.skip(8); // all 0xFF
	stream.readUint32(); // id of the first free resource, -1 if none.

	_resources.reserve(resCount);

	for (uint32 i = 0; i < resCount; i++) {
		uint32 tag = stream.readUint32();
		uint32 size = stream.readUint32();
		uint32 offsetPos = stream.pos();
		int32 offset = stream.readUint32() + moreOffset;
		if (dumpStream) {
			dumpStream->seek(offsetPos - movieStartOffset);
			if (stream.isBE())
				dumpStream->writeUint32BE(offset - movieStartOffset);
			else
				dumpStream->writeUint32LE(offset - movieStartOffset);
		}
		uint16 flags = stream.readUint16();
		uint16 unk1 = stream.readUint16();
		uint32 nextFreeResourceId = stream.readUint32(); // for free resources, the next id, flag like for imap and mmap resources

		debugC(3, kDebugLoading, "Found RIFX resource index %d: '%s', %d bytes @ 0x%08x (%d), flags: %x unk1: %x nextFreeResourceId: %d",
			i, tag2str(tag), size, offset, offset, flags, unk1, nextFreeResourceId);

		Resource &res = _types[tag][i];
		res.index = i;
		res.offset = offset;
		res.size = size;
		res.tag = tag;
		res.accessed = false;
		_resources.push_back(&res);
	}

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "RIFXArchive::readMemoryMap(): Resources found:");
		for (const auto &it : _types) {
			debugC(5, kDebugLoading, "%s: %d", tag2str(it._key), it._value.size());
		}
	}

	return true;
}

bool RIFXArchive::readAfterburnerMap(Common::SeekableReadStreamEndian &stream, uint32 moreOffset) {
	uint32 start, end;

	// File version
	if (stream.readUint32() != MKTAG('F', 'v', 'e', 'r')) {
		warning("RIFXArchive::readAfterburnerMap(): Fver expected but not found");
		return false;
	}

	uint32 fverLength = readVarInt(stream);
	start = stream.pos();
	uint32 version = readVarInt(stream);
	debugC(3, kDebugLoading, "Fver: version: %x", version);
	end = stream.pos();

	if (end - start != fverLength) {
		warning("RIFXArchive::readAfterburnerMap(): Expected Fver of length %d but read %d bytes", fverLength, end - start);
		stream.seek(start + fverLength);
	}

	// Compression types
	if (stream.readUint32() != MKTAG('F', 'c', 'd', 'r')) {
		warning("RIFXArchive::readAfterburnerMap(): Fcdr expected but not found");
		return false;
	}

	uint32 fcdrLength = readVarInt(stream);
	stream.skip(fcdrLength);

	// Afterburner map
	if (stream.readUint32() != MKTAG('A', 'B', 'M', 'P')) {
		warning("RIFXArchive::readAfterburnerMap(): ABMP expected but not found");
		return false;
	}
	uint32 abmpLength = readVarInt(stream);
	uint32 abmpEnd = stream.pos() + abmpLength;
	uint32 abmpCompressionType = readVarInt(stream);
	unsigned long abmpUncompLength = readVarInt(stream);
	unsigned long abmpActualUncompLength = abmpUncompLength;
	debugC(3, kDebugLoading, "ABMP: length: %d compressionType: %d uncompressedLength: %lu",
		abmpLength, abmpCompressionType, abmpUncompLength);

	Common::SeekableReadStreamEndian *abmpStream = readZlibData(stream, abmpEnd - stream.pos(), &abmpActualUncompLength, _isBigEndian);
	if (!abmpStream) {
		warning("RIFXArchive::readAfterburnerMap(): Could not uncompress ABMP");
		return false;
	}
	if (abmpUncompLength != abmpActualUncompLength) {
		warning("ABMP: Expected uncompressed length %lu but got length %lu", abmpUncompLength, abmpActualUncompLength);
	}

	if (ConfMan.getBool("dump_scripts")) {
		Common::DumpFile out;

		char buf[256];
		Common::sprintf_s(buf, "./dumps/%s-%s", encodePathForDump(g_director->getEXEName()).c_str(), "ABMP");

		if (out.open(buf, true)) {
			byte *data = (byte *)malloc(abmpStream->size());

			abmpStream->read(data, abmpStream->size());
			out.write(data, abmpStream->size());
			out.flush();
			out.close();

			free(data);

			abmpStream->seek(0);
		} else {
			warning("RIFXArchive::readAfterburnerMap(): Can not open dump file %s", buf);
		}
	}

	uint32 abmpUnk1 = readVarInt(*abmpStream);
	uint32 abmpUnk2 = readVarInt(*abmpStream);
	uint32 resCount = readVarInt(*abmpStream);
	debugC(3, kDebugLoading, "ABMP: unk1: %d unk2: %d resCount: %d",
		abmpUnk1, abmpUnk2, resCount);

	Common::HashMap<uint32, Resource *> resourceMap;
	for (uint32 i = 0; i < resCount; i++) {
		uint32 resId = readVarInt(*abmpStream);
		int32 offset = readVarInt(*abmpStream);
		if (offset >= 0)
			offset += moreOffset;
		uint32 compSize = readVarInt(*abmpStream);
		uint32 uncompSize = readVarInt(*abmpStream);
		uint32 compressionType = readVarInt(*abmpStream);
		uint32 tag = abmpStream->readUint32();

		debugC(3, kDebugLoading, "Found RIFX resource index %d: '%s', %d bytes (%d uncompressed) @ pos 0x%08x (%d), compressionType: %d",
			resId, tag2str(tag), compSize, uncompSize, offset, offset, compressionType);

		Resource &res = _types[tag][resId];
		res.index = resId;
		res.offset = offset;
		res.size = compSize;
		res.uncompSize = uncompSize;
		res.compressionType = compressionType;
		res.tag = tag;
		res.accessed = false;
		_resources.push_back(&res);
		resourceMap[resId] = &res;
	}

	delete abmpStream;

	// Handle Initial Load Segment (ILS)
	if (!resourceMap.contains(2)) {
		warning("RIFXArchive::readAfterburnerMap(): Map has no entry for ILS");
		return false;
	}
	if (stream.readUint32() != MKTAG('F', 'G', 'E', 'I')) {
		warning("RIFXArchive::readAfterburnerMap(): FGEI expected but not found");
		return false;
	}

	Resource *ilsRes = resourceMap[2];
	uint32 ilsUnk1 = readVarInt(stream);
	debugC(3, kDebugLoading, "ILS: length: %d unk1: %d", ilsRes->size, ilsUnk1);
	_ilsBodyOffset = stream.pos();
	uint32 ilsLength = ilsRes->size;
	unsigned long ilsActualUncompLength = ilsRes->uncompSize;
	Common::SeekableReadStreamEndian *ilsStream = readZlibData(stream, ilsLength, &ilsActualUncompLength, _isBigEndian);
	if (!ilsStream) {
		warning("RIFXArchive::readAfterburnerMap(): Could not uncompress FGEI");
		return false;
	}
	if (ilsRes->uncompSize != ilsActualUncompLength) {
		warning("ILS: Expected uncompressed length %d but got length %lu", ilsRes->uncompSize, ilsActualUncompLength);
	}

	while (ilsStream->pos() < ilsStream->size()) {
		uint32 resId = readVarInt(*ilsStream);
		Resource *res = resourceMap[resId];

		debugC(3, kDebugLoading, "Loading ILS resource %d: '%s', %d bytes", resId, tag2str(res->tag), res->size);

		byte *data = (byte *)malloc(res->size);
		ilsStream->read(data, res->size);
		_ilsData[resId] = data;
	}

	delete ilsStream;

	return true;
}

void RIFXArchive::readCast(Common::SeekableReadStreamEndian &casStream, uint16 libResourceId) {
	uint castTag = MKTAG('C', 'A', 'S', 't');

	uint casSize = casStream.size() / 4;

	debugCN(2, kDebugLoading, "CAS*: libResourceId %d, %d members [", libResourceId, casSize);

	for (uint i = 0; i < casSize; i++) {
		uint32 castIndex = casStream.readUint32BE();
		debugCN(2, kDebugLoading, "%d ", castIndex);

		if (castIndex == 0) {
			continue;
		}
		Resource &res = _types[castTag][castIndex];
		res.castId = i;
		res.libResourceId = libResourceId;
	}
	debugC(2, kDebugLoading, "]");
}

void RIFXArchive::readKeyTable(Common::SeekableReadStreamEndian &keyStream) {
	uint16 entrySize = keyStream.readUint16(); // Should always be 12 (3 uint32's)
	uint16 entrySize2 = keyStream.readUint16();
	uint32 entryCount = keyStream.readUint32(); // There are more entries than actually used
	uint32 usedCount = keyStream.readUint32();

	debugC(2, kDebugLoading, "KEY*: entrySize: %d entrySize2: %d entryCount: %d usedCount: %d", entrySize, entrySize2, entryCount, usedCount);

	ResourceMap &castResMap = _types[MKTAG('C', 'A', 'S', 't')];

	for (uint16 i = 0; i < usedCount; i++) {
		uint32 childIndex = keyStream.readUint32();
		uint32 parentIndex = keyStream.readUint32();
		uint32 childTag = keyStream.readUint32();

		debugC(2, kDebugLoading, "KEY*: childIndex: %d parentIndex: %d childTag: %s", childIndex, parentIndex, tag2str(childTag));

		if (!_keyData.contains(childTag)) {
			_keyData[childTag] = KeyMap();
		}
		if (!_keyData[childTag].contains(parentIndex)) {
			_keyData[childTag][parentIndex] = KeyArray();
		}
		_keyData[childTag][parentIndex].push_back(childIndex);

		// Link cast members to their resources.
		if (castResMap.contains(parentIndex)) {
			castResMap[parentIndex].children.push_back(_types[childTag][childIndex]);
		} else if (castResMap.contains(childIndex)) { // sometimes parent and child index are reversed...
			castResMap[childIndex].children.push_back(_types[childTag][parentIndex]);
		}

		// Link the movie to its resources.
		// The movie has the hardcoded ID 1024, which may collide with a cast member's ID
		// when there are many chunks. This is not a problem since cast members and
		// movies use different resource types, so we can tell them apart.
		if (parentIndex == 1024) {
			_movieChunks.setVal(childTag, childIndex);
		}
	}
}

Common::SeekableReadStreamEndian *RIFXArchive::getFirstResource(uint32 tag) {
	return getResource(tag, getResourceIDList(tag)[0], false);
}

Common::SeekableReadStreamEndian *RIFXArchive::getFirstResource(uint32 tag, bool fileEndianness) {
	return getResource(tag, getResourceIDList(tag)[0], fileEndianness);
}

Common::SeekableReadStreamEndian *RIFXArchive::getFirstResource(uint32 tag, uint16 parentId) {
	if (!_keyData.contains(tag))
		return nullptr;
	if (!_keyData[tag].contains(parentId))
		return nullptr;
	if (_keyData[tag][parentId].empty())
		return nullptr;
	return getResource(tag, _keyData[tag][parentId][0], false);
}


Common::SeekableReadStreamEndian *RIFXArchive::getResource(uint32 tag, uint16 id) {
	return getResource(tag, id, false);
}

Common::SeekableReadStreamEndian *RIFXArchive::getResource(uint32 tag, uint16 id, bool fileEndianness) {
	if (!_types.contains(tag))
		error("RIFXArchive::getResource(): Archive does not contain '%s' %d", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("RIFXArchive::getResource(): Archive does not contain '%s' %d", tag2str(tag), id);

	_types[tag][id].accessed = true; // Mark it as accessed

	const Resource &res = resMap[id];
	bool bigEndian = fileEndianness ? _isBigEndian : true;

	if (_rifxType == MKTAG('F', 'G', 'D', 'M') || _rifxType == MKTAG('F', 'G', 'D', 'C')) {
		if (res.offset == -1) {
			return new Common::MemoryReadStreamEndian(_ilsData[id], res.uncompSize, bigEndian, DisposeAfterUse::NO);
		} else {
			_stream->seek(_ilsBodyOffset + res.offset);
			unsigned long actualUncompLength = res.uncompSize;
			Common::SeekableReadStreamEndian *stream = readZlibData(*_stream, res.size, &actualUncompLength, _isBigEndian);
			if (!stream) {
				error("RIFXArchive::getResource(): Could not uncompress '%s' %d", tag2str(tag), id);
			}
			if (res.uncompSize != actualUncompLength) {
				warning("RIFXArchive::getResource(): For '%s' %d expected uncompressed length %d but got length %lu",
					tag2str(tag), id, res.uncompSize, actualUncompLength);
			}
			return stream;
		}
	}

	uint32 offset = res.offset + 8;
	uint32 size = res.size;
	auto stream = new Common::SeekableSubReadStream(_stream, offset, offset + size, DisposeAfterUse::NO);

	return new Common::SeekableReadStreamEndianWrapper(stream, bigEndian, DisposeAfterUse::YES);
}

Resource RIFXArchive::getResourceDetail(uint32 tag, uint16 id) {
	if (!_types.contains(tag))
		error("RIFXArchive::getResourceDetail(): Archive does not contain '%s' %d", tag2str(tag), id);

	const ResourceMap &resMap = _types[tag];

	if (!resMap.contains(id))
		error("RIFXArchive::getResourceDetail(): Archive does not contain '%s' %d", tag2str(tag), id);

	return resMap[id];
}

Common::String RIFXArchive::formatArchiveInfo() {
	Common::String result = "RIFX, ";
	if (_isBigEndian)
		result += "big endian, ";
	else
		result += "little endian, ";
	result += "type ";
	result += tag2str(_rifxType);
	return result;
}

} // End of namespace Director
