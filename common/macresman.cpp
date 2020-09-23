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

#include "common/scummsys.h"
#include "common/debug.h"
#include "common/util.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/macresman.h"
#include "common/md5.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "common/archive.h"

#ifdef MACOSX
#include "common/config-manager.h"
#endif

namespace Common {

#define MBI_ZERO1 0
#define MBI_NAMELEN 1
#define MBI_ZERO2 74
#define MBI_ZERO3 82
#define MBI_DFLEN 83
#define MBI_RFLEN 87
#define MAXNAMELEN 63

MacResManager::MacResManager() {
	_stream = nullptr;
	// _baseFileName cleared by String constructor

	_mode = kResForkNone;

	_resForkOffset = -1;
	_resForkSize = 0;

	_dataOffset = 0;
	_dataLength = 0;
	_mapOffset = 0;
	_mapLength = 0;
	_resMap.reset();
	_resTypes = nullptr;
	_resLists = nullptr;
}

MacResManager::~MacResManager() {
	close();
}

void MacResManager::close() {
	_resForkOffset = -1;
	_mode = kResForkNone;

	for (int i = 0; i < _resMap.numTypes; i++) {
		for (int j = 0; j < _resTypes[i].items; j++)
			if (_resLists[i][j].nameOffset != -1)
				delete[] _resLists[i][j].name;

		delete[] _resLists[i];
	}

	delete[] _resLists; _resLists = nullptr;
	delete[] _resTypes; _resTypes = nullptr;
	delete _stream; _stream = nullptr;
	_resMap.numTypes = 0;
}

bool MacResManager::hasDataFork() const {
	return !_baseFileName.empty();
}

bool MacResManager::hasResFork() const {
	return !_baseFileName.empty() && _mode != kResForkNone;
}

uint32 MacResManager::getResForkDataSize() const {
	if (!hasResFork())
		return 0;

	_stream->seek(_resForkOffset + 4);
	return _stream->readUint32BE();
}

String MacResManager::computeResForkMD5AsString(uint32 length) const {
	if (!hasResFork())
		return String();

	_stream->seek(_resForkOffset);
	uint32 dataOffset = _stream->readUint32BE() + _resForkOffset;
	/* uint32 mapOffset = */ _stream->readUint32BE();
	uint32 dataLength = _stream->readUint32BE();


	SeekableSubReadStream resForkStream(_stream, dataOffset, dataOffset + dataLength);
	return computeStreamMD5AsString(resForkStream, MIN<uint32>(length, _resForkSize));
}

bool MacResManager::open(const String &fileName) {
	return open(fileName, SearchMan);
}

bool MacResManager::open(const String &fileName, Archive &archive) {
	close();

#ifdef MACOSX
	// Check the actual fork on a Mac computer
	const ArchiveMemberPtr archiveMember = archive.getMember(fileName);
	const Common::FSNode *plainFsNode = dynamic_cast<const Common::FSNode *>(archiveMember.get());
	if (plainFsNode) {
		String fullPath = plainFsNode->getPath() + "/..namedfork/rsrc";
		FSNode resFsNode = FSNode(fullPath);
		SeekableReadStream *macResForkRawStream = resFsNode.createReadStream();
		if (macResForkRawStream && loadFromRawFork(*macResForkRawStream)) {
			_baseFileName = fileName;
			return true;
		}

		delete macResForkRawStream;
	}
#endif

	// Prefer standalone files first, starting with raw forks
	SeekableReadStream *stream = archive.createReadStreamForMember(fileName + ".rsrc");
	if (stream && loadFromRawFork(*stream)) {
		_baseFileName = fileName;
		return true;
	}
	delete stream;

	// Then try for AppleDouble using Apple's naming
	stream = archive.createReadStreamForMember(constructAppleDoubleName(fileName));
	if (stream && loadFromAppleDouble(*stream)) {
		_baseFileName = fileName;
		return true;
	}
	delete stream;

	// Check .bin for MacBinary next
	stream = archive.createReadStreamForMember(fileName + ".bin");
	if (stream && loadFromMacBinary(*stream)) {
		_baseFileName = fileName;
		return true;
	}
	delete stream;

	// As a last resort, see if just the data fork exists
	stream = archive.createReadStreamForMember(fileName);
	if (stream) {
		_baseFileName = fileName;

		// Maybe file is in MacBinary but without .bin extension?
		// Check it here
		if (isMacBinary(*stream)) {
			stream->seek(0);
			if (loadFromMacBinary(*stream))
				return true;
		}

		stream->seek(0);
		_stream = stream;
		return true;
	}

	// The file doesn't exist
	return false;
}

bool MacResManager::exists(const String &fileName) {
	// Try the file name by itself
	if (File::exists(fileName))
		return true;

	// Try the .rsrc extension
	if (File::exists(fileName + ".rsrc"))
		return true;

	// Check if we have a MacBinary file
	File tempFile;
	if (tempFile.open(fileName + ".bin") && isMacBinary(tempFile))
		return true;

	// Check if we have an AppleDouble file
	if (tempFile.open(constructAppleDoubleName(fileName)) && tempFile.readUint32BE() == 0x00051607)
		return true;

	return false;
}

void MacResManager::listFiles(StringArray &files, const String &pattern) {
	// Base names discovered so far.
	typedef HashMap<String, bool, IgnoreCase_Hash, IgnoreCase_EqualTo> BaseNameSet;
	BaseNameSet baseNames;

	// List files itself.
	ArchiveMemberList memberList;
	SearchMan.listMatchingMembers(memberList, pattern);
	SearchMan.listMatchingMembers(memberList, pattern + ".rsrc");
	SearchMan.listMatchingMembers(memberList, pattern + ".bin");
	SearchMan.listMatchingMembers(memberList, constructAppleDoubleName(pattern));

	for (ArchiveMemberList::const_iterator i = memberList.begin(), end = memberList.end(); i != end; ++i) {
		String filename = (*i)->getName();

		// For raw resource forks and MacBinary files we strip the extension
		// here to obtain a valid base name.
		int lastDotPos = filename.size() - 1;
		for (; lastDotPos >= 0; --lastDotPos) {
			if (filename[lastDotPos] == '.') {
				break;
			}
		}

		if (lastDotPos != -1) {
			const char *extension = filename.c_str() + lastDotPos + 1;
			bool removeExtension = false;

			// TODO: Should we really keep filenames suggesting raw resource
			// forks or MacBinary files but not being such around? This might
			// depend on the pattern the client requests...
			if (!scumm_stricmp(extension, "rsrc")) {
				SeekableReadStream *stream = (*i)->createReadStream();
				removeExtension = stream && isRawFork(*stream);
				delete stream;
			} else if (!scumm_stricmp(extension, "bin")) {
				SeekableReadStream *stream = (*i)->createReadStream();
				removeExtension = stream && isMacBinary(*stream);
				delete stream;
			}

			if (removeExtension) {
				filename.erase(lastDotPos);
			}
		}

		// Strip AppleDouble '._' prefix if applicable.
		bool isAppleDoubleName = false;
		const String filenameAppleDoubleStripped = disassembleAppleDoubleName(filename, &isAppleDoubleName);

		if (isAppleDoubleName) {
			SeekableReadStream *stream = (*i)->createReadStream();
			if (stream->readUint32BE() == 0x00051607) {
				filename = filenameAppleDoubleStripped;
			}
			// TODO: Should we really keep filenames suggesting AppleDouble
			// but not being AppleDouble around? This might depend on the
			// pattern the client requests...
			delete stream;
		}

		baseNames[filename] = true;
	}

	// Append resulting base names to list to indicate found files.
	for (BaseNameSet::const_iterator i = baseNames.begin(), end = baseNames.end(); i != end; ++i) {
		files.push_back(i->_key);
	}
}

bool MacResManager::loadFromAppleDouble(SeekableReadStream &stream) {
	if (stream.readUint32BE() != 0x00051607) // tag
		return false;

	stream.skip(20); // version + home file system

	uint16 entryCount = stream.readUint16BE();

	for (uint16 i = 0; i < entryCount; i++) {
		uint32 id = stream.readUint32BE();
		uint32 offset = stream.readUint32BE();
		uint32 length = stream.readUint32BE(); // length

		if (id == 2) {
			// Found the resource fork!
			_resForkOffset = offset;
			_mode = kResForkAppleDouble;
			_resForkSize = length;
			return load(stream);
		}
	}

	return false;
}

bool MacResManager::isMacBinary(SeekableReadStream &stream) {
	byte infoHeader[MBI_INFOHDR];
	int resForkOffset = -1;

	if (stream.read(infoHeader, MBI_INFOHDR) != MBI_INFOHDR)
		return false;

	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		uint32 dataSize = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		uint32 rsrcSize = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		uint32 dataSizePad = (((dataSize + 127) >> 7) << 7);
		// Files produced by ISOBuster are not padded, thus, compare with the actual size
		//uint32 rsrcSizePad = (((rsrcSize + 127) >> 7) << 7);

		// Length check
		if (MBI_INFOHDR + dataSizePad + rsrcSize <= (uint32)stream.size()) {
			resForkOffset = MBI_INFOHDR + dataSizePad;
		}
	}

	if (resForkOffset < 0)
		return false;

	return true;
}

bool MacResManager::isRawFork(SeekableReadStream &stream) {
	// TODO: Is there a better way to detect whether this is a raw fork?
	const uint32 dataOffset = stream.readUint32BE();
	const uint32 mapOffset = stream.readUint32BE();
	const uint32 dataLength = stream.readUint32BE();
	const uint32 mapLength = stream.readUint32BE();

	return    !stream.eos() && !stream.err()
	       && dataOffset < (uint32)stream.size() && dataOffset + dataLength <= (uint32)stream.size()
	       && mapOffset < (uint32)stream.size() && mapOffset + mapLength <= (uint32)stream.size();
}

bool MacResManager::loadFromMacBinary(SeekableReadStream &stream) {
	byte infoHeader[MBI_INFOHDR];
	stream.read(infoHeader, MBI_INFOHDR);

	// Maybe we have MacBinary?
	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		uint32 dataSize = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		uint32 rsrcSize = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		uint32 dataSizePad = (((dataSize + 127) >> 7) << 7);
		// Files produced by ISOBuster are not padded, thus, compare with the actual size
		//uint32 rsrcSizePad = (((rsrcSize + 127) >> 7) << 7);

		// Length check
		if (MBI_INFOHDR + dataSizePad + rsrcSize <= (uint32)stream.size()) {
			_resForkOffset = MBI_INFOHDR + dataSizePad;
			_resForkSize = rsrcSize;
		}
	}

	if (_resForkOffset < 0)
		return false;

	_mode = kResForkMacBinary;
	return load(stream);
}

bool MacResManager::loadFromRawFork(SeekableReadStream &stream) {
	_mode = kResForkRaw;
	_resForkOffset = 0;
	_resForkSize = stream.size();
	return load(stream);
}

bool MacResManager::load(SeekableReadStream &stream) {
	if (_mode == kResForkNone)
		return false;

	stream.seek(_resForkOffset);

	_dataOffset = stream.readUint32BE() + _resForkOffset;
	_mapOffset = stream.readUint32BE() + _resForkOffset;
	_dataLength = stream.readUint32BE();
	_mapLength = stream.readUint32BE();

	// do sanity check
	if (stream.eos() || _dataOffset >= (uint32)stream.size() || _mapOffset >= (uint32)stream.size() ||
			_dataLength + _mapLength  > (uint32)stream.size()) {
		_resForkOffset = -1;
		_mode = kResForkNone;
		return false;
	}

	debug(7, "got header: data %d [%d] map %d [%d]",
		_dataOffset, _dataLength, _mapOffset, _mapLength);

	_stream = &stream;

	readMap();
	return true;
}

SeekableReadStream *MacResManager::getDataFork() {
	if (!_stream)
		return nullptr;

	if (_mode == kResForkMacBinary) {
		_stream->seek(MBI_DFLEN);
		uint32 dataSize = _stream->readUint32BE();
		return new SeekableSubReadStream(_stream, MBI_INFOHDR, MBI_INFOHDR + dataSize);
	}

	File *file = new File();
	if (file->open(_baseFileName))
		return file;
	delete file;

	return nullptr;
}

MacResIDArray MacResManager::getResIDArray(uint32 typeID) {
	int typeNum = -1;
	MacResIDArray res;

	for (int i = 0; i < _resMap.numTypes; i++)
		if (_resTypes[i].id == typeID) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return res;

	res.resize(_resTypes[typeNum].items);

	for (int i = 0; i < _resTypes[typeNum].items; i++)
		res[i] = _resLists[typeNum][i].id;

	return res;
}

MacResTagArray MacResManager::getResTagArray() {
	MacResTagArray tagArray;

	if (!hasResFork())
		return tagArray;

	tagArray.resize(_resMap.numTypes);

	for (uint32 i = 0; i < _resMap.numTypes; i++)
		tagArray[i] = _resTypes[i].id;

	return tagArray;
}

String MacResManager::getResName(uint32 typeID, uint16 resID) const {
	int typeNum = -1;

	for (int i = 0; i < _resMap.numTypes; i++)
		if (_resTypes[i].id == typeID) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return "";

	for (int i = 0; i < _resTypes[typeNum].items; i++)
		if (_resLists[typeNum][i].id == resID)
			return _resLists[typeNum][i].name;

	return "";
}

SeekableReadStream *MacResManager::getResource(uint32 typeID, uint16 resID) {
	int typeNum = -1;
	int resNum = -1;

	for (int i = 0; i < _resMap.numTypes; i++)
		if (_resTypes[i].id == typeID) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return nullptr;

	for (int i = 0; i < _resTypes[typeNum].items; i++)
		if (_resLists[typeNum][i].id == resID) {
			resNum = i;
			break;
		}

	if (resNum == -1)
		return nullptr;

	_stream->seek(_dataOffset + _resLists[typeNum][resNum].dataOffset);
	uint32 len = _stream->readUint32BE();

	// Ignore resources with 0 length
	if (!len)
		return nullptr;

	return _stream->readStream(len);
}

SeekableReadStream *MacResManager::getResource(const String &fileName) {
	for (uint32 i = 0; i < _resMap.numTypes; i++) {
		for (uint32 j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1 && fileName.equalsIgnoreCase(_resLists[i][j].name)) {
				_stream->seek(_dataOffset + _resLists[i][j].dataOffset);
				uint32 len = _stream->readUint32BE();

				// Ignore resources with 0 length
				if (!len)
					return nullptr;

				return _stream->readStream(len);
			}
		}
	}

	return nullptr;
}

SeekableReadStream *MacResManager::getResource(uint32 typeID, const String &fileName) {
	for (uint32 i = 0; i < _resMap.numTypes; i++) {
		if (_resTypes[i].id != typeID)
			continue;

		for (uint32 j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1 && fileName.equalsIgnoreCase(_resLists[i][j].name)) {
				_stream->seek(_dataOffset + _resLists[i][j].dataOffset);
				uint32 len = _stream->readUint32BE();

				// Ignore resources with 0 length
				if (!len)
					return nullptr;

				return _stream->readStream(len);
			}
		}
	}

	return nullptr;
}

void MacResManager::readMap() {
	_stream->seek(_mapOffset + 22);

	_resMap.resAttr = _stream->readUint16BE();
	_resMap.typeOffset = _stream->readUint16BE();
	_resMap.nameOffset = _stream->readUint16BE();
	_resMap.numTypes = _stream->readUint16BE();
	_resMap.numTypes++;

	_stream->seek(_mapOffset + _resMap.typeOffset + 2);
	_resTypes = new ResType[_resMap.numTypes];

	for (int i = 0; i < _resMap.numTypes; i++) {
		_resTypes[i].id = _stream->readUint32BE();
		_resTypes[i].items = _stream->readUint16BE();
		_resTypes[i].offset = _stream->readUint16BE();
		_resTypes[i].items++;

		debug(8, "resType: <%s> items: %d offset: %d (0x%x)", tag2str(_resTypes[i].id), _resTypes[i].items,  _resTypes[i].offset, _resTypes[i].offset);
	}

	_resLists = new ResPtr[_resMap.numTypes];

	for (int i = 0; i < _resMap.numTypes; i++) {
		_resLists[i] = new Resource[_resTypes[i].items];
		_stream->seek(_resTypes[i].offset + _mapOffset + _resMap.typeOffset);

		for (int j = 0; j < _resTypes[i].items; j++) {
			ResPtr resPtr = _resLists[i] + j;

			resPtr->id = _stream->readUint16BE();
			resPtr->nameOffset = _stream->readUint16BE();
			resPtr->dataOffset = _stream->readUint32BE();
			_stream->readUint32BE();
			resPtr->name = nullptr;

			resPtr->attr = resPtr->dataOffset >> 24;
			resPtr->dataOffset &= 0xFFFFFF;
		}

		for (int j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1) {
				_stream->seek(_resLists[i][j].nameOffset + _mapOffset + _resMap.nameOffset);

				byte len = _stream->readByte();
				_resLists[i][j].name = new char[len + 1];
				_resLists[i][j].name[len] = 0;
				_stream->read(_resLists[i][j].name, len);
			}
		}
	}
}

String MacResManager::constructAppleDoubleName(String name) {
	// Insert "._" before the last portion of a path name
	for (int i = name.size() - 1; i >= 0; i--) {
		if (i == 0) {
			name.insertChar('_', 0);
			name.insertChar('.', 0);
		} else if (name[i] == '/') {
			name.insertChar('_', i + 1);
			name.insertChar('.', i + 1);
			break;
		}
	}

	return name;
}

String MacResManager::disassembleAppleDoubleName(String name, bool *isAppleDouble) {
	if (isAppleDouble) {
		*isAppleDouble = false;
	}

	// Remove "._" before the last portion of a path name.
	for (int i = name.size() - 1; i >= 0; --i) {
		if (i == 0) {
			if (name.size() > 2 && name[0] == '.' && name[1] == '_') {
				name.erase(0, 2);
				if (isAppleDouble) {
					*isAppleDouble = true;
				}
			}
		} else if (name[i] == '/') {
			if ((uint)(i + 2) < name.size() && name[i + 1] == '.' && name[i + 2] == '_') {
				name.erase(i + 1, 2);
				if (isAppleDouble) {
					*isAppleDouble = true;
				}
			}
			break;
		}
	}

	return name;
}

void MacResManager::dumpRaw() {
	byte *data = nullptr;
	uint dataSize = 0;
	Common::DumpFile out;

	for (int i = 0; i < _resMap.numTypes; i++) {
		for (int j = 0; j < _resTypes[i].items; j++) {
			_stream->seek(_dataOffset + _resLists[i][j].dataOffset);
			uint32 len = _stream->readUint32BE();

			if (dataSize < len) {
				free(data);
				data = (byte *)malloc(len);
				dataSize = len;
			}

			Common::String filename = Common::String::format("./dumps/%s-%s-%d", _baseFileName.c_str(), tag2str(_resTypes[i].id), j);
			_stream->read(data, len);

			if (!out.open(filename)) {
				warning("MacResManager::dumpRaw(): Can not open dump file %s", filename.c_str());
				return;
			}

			out.write(data, len);

			out.flush();
			out.close();

		}
	}
}

MacResManager::MacVers *MacResManager::parseVers(SeekableReadStream *vvers) {
	MacVers *v = new MacVers;

	v->majorVer = vvers->readByte();
	v->minorVer = vvers->readByte();
	byte devStage = vvers->readByte();
	const char *s;
	switch (devStage) {
	case 0x20: s = "Prealpha"; break;
	case 0x40: s = "Alpha";    break;
	case 0x60: s = "Beta";     break;
	case 0x80: s = "Final";    break;
	default:   s = "";
	}
	v->devStr = s;

	v->preReleaseVer = vvers->readByte();
	v->region = vvers->readUint16BE();
	v->str = vvers->readPascalString();
	v->msg = vvers->readPascalString();

	return v;
}

} // End of namespace Common
