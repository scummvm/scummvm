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

#include "common/scummsys.h"
#include "common/crc.h"
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

MacFinderInfo::MacFinderInfo() : type{0, 0, 0, 0}, creator{0, 0, 0, 0}, flags(0), position(0, 0), windowID(0) {
}

MacFinderInfo::MacFinderInfo(const MacFinderInfoData &data) {
	memcpy(type, data.data + 0, 4);
	memcpy(creator, data.data + 4, 4);
	flags = READ_BE_UINT16(data.data + 8);
	position.y = READ_BE_INT16(data.data + 10);
	position.x = READ_BE_INT16(data.data + 12);
	windowID = READ_BE_INT16(data.data + 14);
}

MacFinderInfoData MacFinderInfo::toData() const {
	MacFinderInfoData data;
	memcpy(data.data + 0, type, 4);
	memcpy(data.data + 4, creator, 4);
	WRITE_BE_UINT16(data.data + 8, flags);
	WRITE_BE_INT16(data.data + 10, position.y);
	WRITE_BE_INT16(data.data + 12, position.x);
	WRITE_BE_INT16(data.data + 14, windowID);

	return data;
}

MacFinderExtendedInfo::MacFinderExtendedInfo() : iconID(0), commentID(0), homeDirectoryID(0) {
}

MacFinderExtendedInfo::MacFinderExtendedInfo(const MacFinderExtendedInfoData &data) {
	iconID = READ_BE_INT16(data.data + 0);
	commentID = READ_BE_INT16(data.data + 10);
	homeDirectoryID = READ_BE_INT32(data.data + 12);
}

MacFinderExtendedInfoData MacFinderExtendedInfo::toData() const {
	MacFinderExtendedInfoData data;
	WRITE_BE_INT16(data.data + 0, iconID);
	memset(data.data + 2, 0, 8);
	WRITE_BE_INT16(data.data + 10, commentID);
	WRITE_BE_INT32(data.data + 12, homeDirectoryID);

	return data;
}

#define MBI_ZERO1 0
#define MBI_NAMELEN 1
#define MBI_TYPE 65
#define MBI_CREATOR 69
#define MBI_FLAGSHIGH 73
#define MBI_ZERO2 74
#define MBI_POSY 75
#define MBI_POSX 77
#define MBI_FOLDERID 79
#define MBI_ZERO3 82
#define MBI_DFLEN 83
#define MBI_RFLEN 87
#define MBI_FLAGSLOW 101
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

bool MacResManager::hasResFork() const {
	return !_baseFileName.empty() && _mode != kResForkNone && _resForkSize != 0;
}

uint32 MacResManager::getResForkDataSize() const {
	if (!hasResFork())
		return 0;

	_stream->seek(_resForkOffset + 8);
	return _stream->readUint32BE();
}

String MacResManager::computeResForkMD5AsString(uint32 length, bool tail) const {
	if (!hasResFork())
		return String();

	_stream->seek(_resForkOffset);
	uint32 dataOffset = _stream->readUint32BE() + _resForkOffset;
	/* uint32 mapOffset = */ _stream->readUint32BE();
	uint32 dataLength = _stream->readUint32BE();


	SeekableSubReadStream resForkStream(_stream, dataOffset, dataOffset + dataLength);
	if (tail && dataLength > length)
		resForkStream.seek(-(int64)length, SEEK_END);

	return computeStreamMD5AsString(resForkStream, MIN<uint32>(length, _resForkSize));
}

bool MacResManager::open(const Path &fileName) {
	return open(fileName, SearchMan);
}

SeekableReadStream *MacResManager::openAppleDoubleWithAppleOrOSXNaming(Archive& archive, const Path &fileName) {
	SeekableReadStream *stream = archive.createReadStreamForMember(constructAppleDoubleName(fileName));
	if (stream)
		return stream;

	const ArchiveMemberPtr archiveMember = archive.getMember(fileName);
        const Common::FSNode *plainFsNode = dynamic_cast<const Common::FSNode *>(archiveMember.get());

	// Try finding __MACOSX
	Common::StringArray components = (plainFsNode ? plainFsNode->getPath() : fileName).splitComponents();
	if (components.empty() || components[components.size() - 1].empty())
		return nullptr;
	for (int i = components.size() - 1; i >= 0; i--) {
		Common::StringArray newComponents;
		int j;
		for (j = 0; j < i; j++)
			newComponents.push_back(components[j]);
		newComponents.push_back("__MACOSX");
		for (; j < (int) components.size() - 1; j++)
			newComponents.push_back(components[j]);
		newComponents.push_back("._" + components[(int) components.size() - 1]);

		Common::Path newPath = Common::Path::joinComponents(newComponents);
		stream = archive.createReadStreamForMember(newPath);

		if (!stream) {
			Common::FSNode fsn(newPath);
			if (fsn.exists()) {
				stream = fsn.createReadStream();
			}
		}

		if (stream) {
			bool appleDouble = (stream->readUint32BE() == 0x00051607);
			stream->seek(0);

			if (appleDouble) {
				return stream;
			}
		}
		delete stream;
	}

	return nullptr;
}

bool MacResManager::open(const Path &fileName, Archive &archive) {
	close();

	SeekableReadStream *stream = nullptr;

	// Our preference is as following:
	// AppleDouble in .rsrc -> Raw .rsrc -> MacBinary with .bin -> MacBinary without .bin -> AppleDouble in ._
	// -> AppleDouble in __MACOSX -> Actual resource fork -> No resource fork

	Common::ArchiveMemberPtr archiveMember = archive.getMember(fileName);

	// If this is in a Mac archive, then the resource fork will always be in the alt stream
	if (archiveMember && archiveMember->isInMacArchive()) {
		_baseFileName = fileName;

		stream = archive.createReadStreamForMemberAltStream(fileName, AltStreamType::MacResourceFork);
		if (stream && !loadFromRawFork(stream)) {
			delete stream;
			_stream = nullptr;
		}

		// If the archive member exists, then the file exists, but has no res fork, so we should return true
		return true;
	}

	// Prefer standalone files first, starting with raw forks
	stream = archive.createReadStreamForMember(fileName.append(".rsrc"));

	if (stream) {
		// Some programs actually store AppleDouble there. Check it
		bool appleDouble = (stream->readUint32BE() == 0x00051607);
		stream->seek(0);

		if (appleDouble && loadFromAppleDouble(stream)) {
			_baseFileName = fileName;
			return true;
		}

		if (loadFromRawFork(stream)) {
			_baseFileName = fileName;
			return true;
		}
	}
	delete stream;

	// Check .bin for MacBinary next
	stream = archive.createReadStreamForMember(fileName.append(".bin"));
	if (stream && loadFromMacBinary(stream)) {
		_baseFileName = fileName;
		return true;
	}
	delete stream;

	// Maybe file is in MacBinary but without .bin extension?
	// Check it here
	if (archiveMember) {
		stream = archiveMember->createReadStream();
		if (stream && isMacBinary(*stream)) {
			stream->seek(0);
			if (loadFromMacBinary(stream)) {
				_baseFileName = fileName;
				return true;
			}
		}
	} else
		stream = nullptr;

	bool fileExists = (stream != nullptr);

	delete stream;

	// Then try for AppleDouble using Apple's naming
	// As they are created silently from plain files (e.g. from a macbinary) they are pretty low quality often.
	stream = openAppleDoubleWithAppleOrOSXNaming(archive, fileName);
	if (stream && loadFromAppleDouble(stream)) {
		_baseFileName = fileName;
		return true;
	}
	delete stream;

	// Try alternate stream
	stream = archive.createReadStreamForMemberAltStream(fileName, AltStreamType::MacResourceFork);
	if (stream && loadFromRawFork(stream)) {
		_baseFileName = fileName;
		return true;
	}
	delete stream;


#ifdef MACOSX
	// Check the actual fork on a Mac computer. It's even worse than __MACOSX as
	// it's present on any HFS(+) and appears even after copying macbin on HFS(+).
	if (archiveMember.get()) {
		// This could be a MacBinary file that still has a
		// resource fork; if it is, it needs to get opened as MacBinary
		// and not treated as raw.
		stream = archive.createReadStreamForMember(fileName);
		bool isMacBinaryFile = false;
		if (stream) {
			isMacBinaryFile = isMacBinary(*stream);
		}
		delete stream;

		Path fullPath = archiveMember.get()->getPathInArchive().join("/..namedfork/rsrc");
		SeekableReadStream *macResForkRawStream = archive.createReadStreamForMember(fullPath);
		if (!isMacBinaryFile && macResForkRawStream && loadFromRawFork(macResForkRawStream)) {
			_baseFileName = fileName;
			return true;
		}

		delete macResForkRawStream;
	}
#endif

	if (fileExists) { // No non-empty resource fork found, but the file still exists
		_baseFileName = fileName;
		_stream = nullptr;
		return true;
	}

	// The file doesn't exist
	return false;
}

SeekableReadStream * MacResManager::openFileOrDataFork(const Path &fileName) {
	return openFileOrDataFork(fileName, SearchMan);
}

SeekableReadStream * MacResManager::openDataForkFromMacBinary(SeekableReadStream *inStream, DisposeAfterUse::Flag disposeAfterUse) {
	if (!inStream || !isMacBinary(*inStream)) {
		return nullptr;
	}
	inStream->seek(MBI_DFLEN);
	uint32 dataSize = inStream->readUint32BE();
	return new SeekableSubReadStream(inStream, MBI_INFOHDR, MBI_INFOHDR + dataSize, disposeAfterUse);
}

SeekableReadStream * MacResManager::openFileOrDataFork(const Path &fileName, Archive &archive) {
	SeekableReadStream *stream = nullptr;

	Common::ArchiveMemberPtr archiveMember = archive.getMember(fileName);

	bool mayBeMacBinary = true;

	// Our preference is as following:
	// File itself as macbinary -> File itself as raw -> .bin as macbinary
	// Compared to open:
	// * It moves macbinary without .bin ahead of macbinary with .bin
	//    Shouldn't matter unless we have both x and x.bin and both are macbinary. If we ever need it,
	//    fixing it is easy at the cost of some readability.
	// * Even in presence of .rsrc macbinary is still probed for and its header stripped
	//    Shouldn't be a problem unless we need to handle double macbinary like if
	//    some publisher decides to put a macbinary on HFS(+) in *Retail* which would be
	//    a "funny" problem to solve regardless as we will then need to ensure to match
	//    right levels of onion. Fortunately no game so far does it. But someday...
	//    Hopefully not.

	if (archiveMember && archiveMember->isInMacArchive())
		mayBeMacBinary = false;

	if (archiveMember) {
		stream = archiveMember->createReadStream();

		// Check the basename for Macbinary
		if (mayBeMacBinary && stream && isMacBinary(*stream)) {
			stream->seek(MBI_DFLEN);
			uint32 dataSize = stream->readUint32BE();
			return new SeekableSubReadStream(stream, MBI_INFOHDR, MBI_INFOHDR + dataSize, DisposeAfterUse::YES);
		}

		// All formats other than Macbinary and AppleSingle (not supported) use
		// basename-named file as data fork holder.
		if (stream) {
			stream->seek(0);
			return stream;
		}
	}

	if (mayBeMacBinary) {
		// Check .bin for MacBinary next
		stream = archive.createReadStreamForMember(fileName.append(".bin"));
		if (stream && isMacBinary(*stream)) {
			stream->seek(MBI_DFLEN);
			uint32 dataSize = stream->readUint32BE();
			return new SeekableSubReadStream(stream, MBI_INFOHDR, MBI_INFOHDR + dataSize, DisposeAfterUse::YES);
		}
		delete stream;
	}

	// The file doesn't exist
	return nullptr;
}


bool MacResManager::exists(const Path &fileName) {
	// Try the file name by itself
	if (File::exists(fileName))
		return true;

	// Try the .rsrc extension
	if (File::exists(fileName.append(".rsrc")))
		return true;

	// Check if we have a MacBinary file
	File tempFile;
	if (tempFile.open(fileName.append(".bin")) && isMacBinary(tempFile))
		return true;

	// Check if we have an AppleDouble file
	if (tempFile.open(constructAppleDoubleName(fileName)) && tempFile.readUint32BE() == 0x00051607)
		return true;

	return false;
}

bool MacResManager::getFileFinderInfo(const Path &fileName, Archive &archive, MacFinderInfo &outFinderInfo) {
	MacFinderExtendedInfo fxinfo;
	return getFileFinderInfo(fileName, archive, outFinderInfo, fxinfo);
}

bool MacResManager::getFileFinderInfo(const Path &fileName, Archive &archive, MacFinderInfo &outFinderInfo, MacFinderExtendedInfo &outFinderExtendedInfo) {
	// Our preference is as following:
	// Alt stream -> AppleDouble in .rsrc -> MacBinary with .bin -> MacBinary without .bin -> AppleDouble in ._
	// -> AppleDouble in __MACOSX -> No finder info
	// If you compare with open there are following differences:
	// * We add .finf. It has only finder info
	// * We skip raw .rsrc as it lack finder info
	// * Actual finder info on OSX isn't implemented yet

	// Prefer alt stream first (especially since this can avoid decompressing entire files from slow archive formats like StuffIt Installer)
	Common::ScopedPtr<SeekableReadStream> stream(archive.createReadStreamForMemberAltStream(fileName, AltStreamType::MacFinderInfo));
	if (stream) {
		MacFinderInfoData finfoData;
		MacFinderExtendedInfoData fxinfoData;

		uint32 finfoSize = stream->read(&finfoData, sizeof(finfoData));
		uint32 fxinfoSize = stream->read(&fxinfoData, sizeof(fxinfoData));

		if (finfoSize == sizeof(MacFinderInfoData)) {
			outFinderInfo = MacFinderInfo(finfoData);

			if (fxinfoSize == sizeof(MacFinderExtendedInfoData)) {
				outFinderExtendedInfo = MacFinderExtendedInfo(fxinfoData);
			} else {
				outFinderExtendedInfo = MacFinderExtendedInfo();
				if (fxinfoSize != 0)
					warning("Finder extended info metadata file was too small");
			}

			return true;
		} else if (finfoSize != 0) {
			warning("Finder info metadata file was too small");
		}
	}

	// Might have AppleDouble in the resource file
	stream.reset();
	stream.reset(archive.createReadStreamForMember(fileName.append(".rsrc")));

	if (stream) {
		bool appleDouble = (stream->readUint32BE() == 0x00051607);
		stream->seek(0);

		if (appleDouble && getFinderInfoFromAppleDouble(stream.get(), outFinderInfo, outFinderExtendedInfo))
			return true;
	}

	// Check .bin for MacBinary next
	stream.reset();
	stream.reset(archive.createReadStreamForMember(fileName.append(".bin")));
	if (stream && getFinderInfoFromMacBinary(stream.get(), outFinderInfo, outFinderExtendedInfo))
		return true;

	// Check if the file is in MacBinary format
	stream.reset();
	stream.reset(archive.createReadStreamForMember(fileName));
	if (stream && getFinderInfoFromMacBinary(stream.get(), outFinderInfo, outFinderExtendedInfo))
		return true;

	// Try for AppleDouble using Apple's naming
	stream.reset();
	stream.reset(openAppleDoubleWithAppleOrOSXNaming(archive, fileName));
	if (stream && getFinderInfoFromAppleDouble(stream.get(), outFinderInfo, outFinderExtendedInfo))
		return true;

	// No metadata
	return false;
}

bool MacResManager::getFileFinderInfo(const Path &fileName, MacFinderInfo &outFinderInfo) {
	MacFinderExtendedInfo fxinfo;
	return getFileFinderInfo(fileName, outFinderInfo, fxinfo);
}

bool MacResManager::getFileFinderInfo(const Path &fileName, MacFinderInfo &outFinderInfo, MacFinderExtendedInfo &outFinderExtendedInfo) {
	return getFileFinderInfo(fileName, SearchMan, outFinderInfo, outFinderExtendedInfo);
}

void MacResManager::listFiles(Array<Path> &files, const Path &pattern) {
	// Base names discovered so far.
	typedef HashMap<Path, bool, Path::IgnoreCase_Hash, Path::IgnoreCase_EqualTo> BaseNameSet;
	BaseNameSet baseNames;

	// List files itself.
	ArchiveMemberList memberList;
	SearchMan.listMatchingMembers(memberList, pattern);
	SearchMan.listMatchingMembers(memberList, pattern.append(".rsrc"));
	SearchMan.listMatchingMembers(memberList, pattern.append(".bin"));
	SearchMan.listMatchingMembers(memberList, constructAppleDoubleName(pattern));

	for (ArchiveMemberList::const_iterator i = memberList.begin(), end = memberList.end(); i != end; ++i) {
		String filename = (*i)->getFileName();

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
		const Path filenameAppleDoubleStripped = disassembleAppleDoubleName(
				Common::Path(filename, Common::Path::kNoSeparator), &isAppleDoubleName);

		if (isAppleDoubleName) {
			SeekableReadStream *stream = (*i)->createReadStream();
			if (stream->readUint32BE() == 0x00051607) {
				filename = filenameAppleDoubleStripped.baseName();
			}
			// TODO: Should we really keep filenames suggesting AppleDouble
			// but not being AppleDouble around? This might depend on the
			// pattern the client requests...
			delete stream;
		}

		Common::Path basePath((*i)->getPathInArchive().getParent().appendComponent(filename));
		baseNames[basePath] = true;
	}

	// Append resulting base names to list to indicate found files.
	for (BaseNameSet::const_iterator i = baseNames.begin(), end = baseNames.end(); i != end; ++i) {
		files.push_back(i->_key);
	}
}

bool MacResManager::loadFromAppleDouble(SeekableReadStream *stream) {
	if (!stream)
		return false;

	if (stream->readUint32BE() != 0x00051607) // tag
		return false;

	stream->skip(20); // version + home file system

	uint16 entryCount = stream->readUint16BE();

	for (uint16 i = 0; i < entryCount; i++) {
		uint32 id = stream->readUint32BE();
		uint32 offset = stream->readUint32BE();
		uint32 length = stream->readUint32BE(); // length

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

bool MacResManager::getFinderInfoFromMacBinary(SeekableReadStream *stream, MacFinderInfo &outFinderInfo, MacFinderExtendedInfo &outFinderExtendedInfo) {
	byte infoHeader[MBI_INFOHDR];
	if (!readAndValidateMacBinaryHeader(*stream, infoHeader))
		return false;

	MacFinderInfo finfo;

	// Parse fields
	memcpy(finfo.type, infoHeader + MBI_TYPE, 4);
	memcpy(finfo.creator, infoHeader + MBI_CREATOR, 4);
	finfo.flags = (infoHeader[MBI_FLAGSHIGH] << 8) + infoHeader[MBI_FLAGSLOW];
	finfo.position.x = READ_BE_INT16(infoHeader + MBI_POSX);
	finfo.position.y = READ_BE_INT16(infoHeader + MBI_POSY);
	finfo.windowID = READ_BE_INT16(infoHeader + MBI_FOLDERID);

	outFinderInfo = finfo;
	outFinderExtendedInfo = MacFinderExtendedInfo();

	return true;
}

bool MacResManager::getFinderInfoFromAppleDouble(SeekableReadStream *stream, MacFinderInfo &outFinderInfo, MacFinderExtendedInfo &outFinderExtendedInfo) {
	if (!stream)
		return false;

	if (stream->readUint32BE() != 0x00051607) // tag
		return false;

	stream->skip(20); // version + home file system

	uint16 entryCount = stream->readUint16BE();

	uint32 finderInfoPos = 0;
	uint32 finderInfoLength = 0;

	for (uint16 i = 0; i < entryCount; i++) {
		uint32 id = stream->readUint32BE();
		uint32 offset = stream->readUint32BE();
		uint32 length = stream->readUint32BE(); // length

		if (id == 9) {
			finderInfoPos = offset;
			finderInfoLength = length;
			break;
		}
	}

	if (finderInfoLength < sizeof(MacFinderInfoData) + sizeof(MacFinderExtendedInfoData))
		return false;

	if (!stream->seek(finderInfoPos))
		return false;

	MacFinderInfoData finfo;
	MacFinderExtendedInfoData fxinfo;
	if (stream->read(&finfo, sizeof(finfo)) != sizeof(finfo) || stream->read(&fxinfo, sizeof(fxinfo)) != sizeof(fxinfo))
		return false;

	outFinderInfo = MacFinderInfo(finfo);
	outFinderExtendedInfo = MacFinderExtendedInfo(fxinfo);

	return true;
}

bool MacResManager::readAndValidateMacBinaryHeader(SeekableReadStream &stream, byte (&infoHeader)[MBI_INFOHDR]) {
	int resForkOffset = -1;

	if (stream.read(infoHeader, MBI_INFOHDR) != MBI_INFOHDR)
		return false;

	/* CRC_BINHEX of block of zeros is zero so checksum below will lead a false positive.
	   Header of all zeros is not a valid MacBinary header
	   as it lacks name, resource fork and data fork.
	   Exclude headers that have zero name len, zero data fork, zero name fork and zero type_creator.
	   Keep it at the top as a quick and cheap check
	*/
	if (infoHeader[MBI_NAMELEN] == 0 && READ_BE_UINT32(infoHeader + MBI_DFLEN) == 0
	    && READ_BE_UINT32(infoHeader + MBI_RFLEN) == 0 &&
	    READ_BE_UINT32(infoHeader + MBI_TYPE) == 0 && READ_BE_UINT32(infoHeader + MBI_CREATOR) == 0)
		return false;

	CRC_BINHEX crc;
	uint16 checkSum = crc.crcFast(infoHeader, 124);

	// Sanity check on the CRC. Some movies could look like MacBinary
	if (checkSum != READ_BE_UINT16(&infoHeader[124]))
		return false;

	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		uint32 dataSize = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		uint32 rsrcSize = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		uint32 dataSizePad = (((dataSize + 127) >> 7) << 7);
		// Files produced by ISOBuster are not padded, thus, compare with the actual size
		// uint32 rsrcSizePad = (((rsrcSize + 127) >> 7) << 7);

		// Length check
		if (MBI_INFOHDR + dataSizePad + rsrcSize <= (uint32)stream.size()) {
			resForkOffset = MBI_INFOHDR + dataSizePad;
		}
	}

	if (resForkOffset < 0)
		return false;

	return true;
}

bool MacResManager::isMacBinary(SeekableReadStream &stream) {
	byte infoHeader[MBI_INFOHDR];
	return readAndValidateMacBinaryHeader(stream, infoHeader);
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

bool MacResManager::loadFromMacBinary(SeekableReadStream *stream) {
	if (!stream)
		return false;

	byte infoHeader[MBI_INFOHDR];
	stream->read(infoHeader, MBI_INFOHDR);

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
		if (MBI_INFOHDR + dataSizePad + rsrcSize <= (uint32)stream->size()) {
			_resForkOffset = MBI_INFOHDR + dataSizePad;
			_resForkSize = rsrcSize;
		}

		if (_resForkOffset < 0)
			return false;

		_mode = kResForkMacBinary;
		return load(stream);
	}

	return false;
}

bool MacResManager::loadFromRawFork(SeekableReadStream *stream) {
	if (!stream)
		return false;

	_mode = kResForkRaw;
	_resForkOffset = 0;
	_resForkSize = stream->size();
	return load(stream);
}

bool MacResManager::load(SeekableReadStream *stream) {
	if (!stream)
		return false;

	if (_mode == kResForkNone)
		return false;

	if (_resForkSize == 0) {
		_stream = stream;
		return true;
	}

	stream->seek(_resForkOffset);

	_dataOffset = stream->readUint32BE() + _resForkOffset;
	_mapOffset = stream->readUint32BE() + _resForkOffset;
	_dataLength = stream->readUint32BE();
	_mapLength = stream->readUint32BE();

	// do sanity check
	if (stream->eos() || _dataOffset >= (uint32)stream->size() || _mapOffset >= (uint32)stream->size() ||
			_dataLength + _mapLength  > (uint32)stream->size()) {
		_resForkOffset = -1;
		_mode = kResForkNone;
		return false;
	}

	debug(7, "got header: data %d [%d] map %d [%d]",
		_dataOffset, _dataLength, _mapOffset, _mapLength);

	_stream = stream;

	readMap();
	return true;
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

uint32 MacResManager::getResLength(uint32 typeID, uint16 resID) {
	int typeNum = -1;
	int resNum = -1;

	for (int i = 0; i < _resMap.numTypes; i++)
		if (_resTypes[i].id == typeID) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return 0;

	for (int i = 0; i < _resTypes[typeNum].items; i++)
		if (_resLists[typeNum][i].id == resID) {
			resNum = i;
			break;
		}

	if (resNum == -1)
		return 0;

	_stream->seek(_dataOffset + _resLists[typeNum][resNum].dataOffset);
	uint32 len = _stream->readUint32BE();

	return len;
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

	debug(8, "numResTypes: %d total size: %u", _resMap.numTypes, unsigned(_stream->size()));

	if (_stream->pos() + _resMap.numTypes * 8 > _stream->size())
		error("MacResManager::readMap(): incorrect resource map, too big, %d types", _resMap.numTypes);

	int totalItems = 0;

	for (int i = 0; i < _resMap.numTypes; i++) {
		_resTypes[i].id = _stream->readUint32BE();
		_resTypes[i].items = _stream->readUint16BE();
		_resTypes[i].offset = _stream->readUint16BE();
		_resTypes[i].items++;

		totalItems += _resTypes[i].items;

		debug(8, "resType: <%s> items: %d offset: %d (0x%x)", tag2str(_resTypes[i].id), _resTypes[i].items,  _resTypes[i].offset, _resTypes[i].offset);
	}

	if (totalItems * 4 > _stream->size())
		error("MacResManager::readMap(): incorrect resource map, too big, %d total items", totalItems);

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

Path MacResManager::constructAppleDoubleName(const Path &name) {
	// Insert "._" before the last portion of a path name
	Path ret = name.getParent();
	Path lastComponent = name.getLastComponent();
	return ret.appendInPlace("._").appendInPlace(lastComponent);
}

Path MacResManager::disassembleAppleDoubleName(const Path &name, bool *isAppleDouble) {
	if (isAppleDouble) {
		*isAppleDouble = false;
	}

	// Remove "._" before the last portion of a path name.
	Path ret = name.getParent();
	String lastComponent = name.baseName();
	if (!lastComponent.hasPrefix("._"))
		return name;
	ret = ret.appendComponent(lastComponent.substr(2));
	if (name.isSeparatorTerminated()) {
		ret.appendInPlace("/");
	}
	return ret;
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

			Common::String filename = Common::String::format("./dumps/%s-%s-%d", _baseFileName.baseName().c_str(), tag2str(_resTypes[i].id), j);
			_stream->read(data, len);

			if (!out.open(Common::Path(filename, '/'))) {
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
