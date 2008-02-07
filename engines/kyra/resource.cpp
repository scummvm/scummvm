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


#include "common/config-manager.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/func.h"
#include "common/algorithm.h"

#include "gui/message.h"

#include "kyra/resource.h"

namespace Kyra {

Resource::Resource(KyraEngine *vm) : _loaders(), _map(), _vm(vm) {
	initializeLoaders();
}

Resource::~Resource() {
	unloadAllPakFiles();
}

bool Resource::reset() {
	unloadAllPakFiles();

	FilesystemNode dir(ConfMan.get("path"));

	if (!dir.exists() || !dir.isDirectory())
		error("invalid game path '%s'", dir.getPath().c_str());

	if (_vm->game() != GI_KYRA3) {
		if (!loadPakFile(StaticResource::staticDataFilename()) || !StaticResource::checkKyraDat()) {
			Common::String errorMessage = "You're missing the '" + StaticResource::staticDataFilename() + "' file or it got corrupted, (re)get it from the ScummVM website";
			GUI::MessageDialog errorMsg(errorMessage);
			errorMsg.runModal();
			error(errorMessage.c_str());
		}
	}

	if (_vm->game() == GI_KYRA1) {
		// We only need kyra.dat for the demo.
		if (_vm->gameFlags().isDemo)
			return true;

		// only VRM file we need in the *whole* game for kyra1
		if (_vm->gameFlags().isTalkie)
			loadPakFile("CHAPTER1.VRM");
	} else if (_vm->game() == GI_KYRA2) {
		// mouse pointer, fonts, etc. required for initializing
		if (_vm->gameFlags().isDemo) {
			loadPakFile("GENERAL.PAK");
		} else {
			loadPakFile("INTROGEN.PAK");
			loadPakFile("OTHER.PAK");
		}

		return true;
	} else if (_vm->game() == GI_KYRA3) {
		loadPakFile("WESTWOOD.001");
	}

	FSList fslist;
	if (!dir.getChildren(fslist, FilesystemNode::kListFilesOnly))
		error("can't list files inside game path '%s'", dir.getPath().c_str());

	if (_vm->game() == GI_KYRA1 && _vm->gameFlags().isTalkie) {
		static const char *list[] = {
			"ADL.PAK", "CHAPTER1.VRM", "COL.PAK", "FINALE.PAK", "INTRO1.PAK", "INTRO2.PAK",
			"INTRO3.PAK", "INTRO4.PAK", "MISC.PAK",	"SND.PAK", "STARTUP.PAK", "XMI.PAK",
			"CAVE.APK", "DRAGON1.APK", "DRAGON2.APK", "LAGOON.APK"
		};

		Common::for_each(list, list + ARRAYSIZE(list), Common::bind1st(Common::mem_fun(&Resource::loadPakFile), this));
	} else {
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			Common::String filename = file->getName();
			filename.toUppercase();

			// No real PAK file!
			if (filename == "TWMUSIC.PAK")
				continue;

			if (filename == ((_vm->gameFlags().lang == Common::EN_ANY) ? "JMC.PAK" : "EMC.PAK"))
				continue;

			if (filename.hasSuffix(".PAK") || filename.hasSuffix(".APK")) {
				if (!loadPakFile(file->getName()))
					error("couldn't open pakfile '%s'", file->getName().c_str());
			}
		}
	}

	return true;
}

bool Resource::loadPakFile(const Common::String &filename) {
	ResFileMap::iterator iter = _map.find(filename);
	if (iter == _map.end())
		return false;

	iter->_value.loadable = true;

	if (!isAccessable(filename))
		return false;

	if (iter->_value.preload)
		return true;

	Common::SeekableReadStream *stream = getFileStream(filename);
	assert(stream);

	const ResArchiveLoader *loader = getLoader(iter->_value.type);
	assert(loader);

	loader->loadFile(filename, *stream, _map);
	delete stream;
	stream = 0;

	iter = _map.find(filename);
	if (iter == _map.end())
		return false;
	iter->_value.preload = true;
	detectFileTypes();

	return true;
}

bool Resource::loadFileList(const Common::String &filedata) {
	Common::File f;

	if (!f.open(filedata))
		return false;

	uint32 filenameOffset = 0;
	while ((filenameOffset = f.readUint32LE()) != 0) {
		uint32 offset = f.pos();
		f.seek(filenameOffset, SEEK_SET);

		uint8 buffer[64];
		f.read(buffer, sizeof(buffer));
		f.seek(offset + 16, SEEK_SET);

		Common::String filename = (char*)buffer;
		filename.toUppercase();

		if (filename.hasSuffix(".PAK")) {
			if (!loadPakFile(filename)) {
				error("couldn't load file '%s'", filename.c_str());
				return false;
			}
		}
	}

	return true;
}

bool Resource::loadFileList(const char * const *filelist, uint32 numFiles) {
	if (!filelist)
		return false;

	while (numFiles--) {
		if (!loadPakFile(filelist[numFiles])) {
			error("couldn't load file '%s'", filelist[numFiles]);
			return false;
		}
	}

	return true;
}

void Resource::unloadPakFile(const Common::String &filename) {
	ResFileMap::iterator iter = _map.find(filename);
	if (iter != _map.end()) {
		if (!iter->_value.prot)
			iter->_value.loadable = false;
	}
}

bool Resource::isInPakList(const Common::String &filename) const {
	return isAccessable(filename);
}

void Resource::unloadAllPakFiles() {
	FilesystemNode dir(ConfMan.get("path"));

	if (!dir.exists() || !dir.isDirectory())
		error("invalid game path '%s'", dir.getPath().c_str());

	FSList fslist;
	if (!dir.getChildren(fslist, FilesystemNode::kListFilesOnly))
		error("can't list files inside game path '%s'", dir.getPath().c_str());

	// remove all entries
	_map.clear();
	
	Common::File temp;
	if (temp.open("kyra.dat")) {
		ResFileEntry entry;
		entry.parent = "";
		entry.size = temp.size();
		entry.loadable = true;
		entry.preload = false;
		entry.prot = false;
		entry.type = ResFileEntry::kPak;
		entry.offset = 0;
		_map["kyra.dat"] = entry;
		temp.close();
	}
	
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		ResFileEntry entry;
		entry.parent = "";
		if (!temp.open(file->getPath()))
			error("couldn't open file '%s'", file->getName().c_str());
		entry.size = temp.size();
		entry.offset = 0;
		entry.loadable = true;
		entry.preload = false;
		entry.prot = false;
		entry.type = ResFileEntry::kAutoDetect;
		_map[file->getName()] = entry;
		temp.close();
	}

	detectFileTypes();
}

uint8 *Resource::fileData(const char *file, uint32 *size) const {
	Common::SeekableReadStream *stream = getFileStream(file);
	if (!stream)
		return 0;

	uint32 bufferSize = stream->size();
	uint8 *buffer = new uint8[bufferSize];
	assert(buffer);
	if (size)
		*size = bufferSize;
	stream->read(buffer, bufferSize);
	delete stream;
	return buffer;
}

uint32 Resource::getFileSize(const char *file) const {
	if (!isAccessable(file))
		return 0;

	ResFileMap::const_iterator iter = _map.find(file);
	if (iter != _map.end())
		return iter->_value.size;
	return 0;
}

bool Resource::loadFileToBuf(const char *file, void *buf, uint32 maxSize) {
	Common::SeekableReadStream *stream = getFileStream(file);
	if (!stream)
		return false;

	if (maxSize < stream->size()) {
		delete stream;
		return false;
	}
	memset(buf, 0, maxSize);
	stream->read(buf, stream->size());
	delete stream;
	return true;
}

Common::SeekableReadStream *Resource::getFileStream(const Common::String &file) const {
	if (!isAccessable(file))
		return 0;

	ResFileMap::const_iterator iter = _map.find(file);
	if (iter == _map.end())
		return 0;

	if (!iter->_value.parent.empty()) {
		Common::SeekableReadStream *parent = getFileStream(iter->_value.parent);
		assert(parent);

		ResFileMap::const_iterator parentIter = _map.find(iter->_value.parent);
		const ResArchiveLoader *loader = getLoader(parentIter->_value.type);
		assert(loader);

		return loader->loadFileFromArchive(file, parent, _map);
	} else {
		Common::File *stream = new Common::File();
		if (!stream->open(file.c_str())) {
			warning("Couldn't open file '%s'", file.c_str());
			return 0;
		}
		return stream;
	}

	return 0;
}

bool Resource::isAccessable(const Common::String &file) const {
	ResFileMap::const_iterator iter = _map.find(file);
	while (true) {
		if (iter == _map.end())
			break;

		if (!iter->_value.loadable)
			return false;

		if (!iter->_value.parent.empty())
			iter = _map.find(iter->_value.parent);
		else
			return iter->_value.loadable;
	}
	return false;
}

void Resource::detectFileTypes() {
	for (ResFileMap::iterator i = _map.begin(); i != _map.end(); ++i) {
		if (!isAccessable(i->_key))
			continue;

		if (i->_value.type == ResFileEntry::kAutoDetect) {
			for (LoaderIterator l = _loaders.begin(); l != _loaders.end(); ++l) {
				Common::SeekableReadStream *stream = getFileStream(i->_key);
				if ((*l)->isLoadable(i->_key, *stream)) {
					i->_value.type = (*l)->getType();
					i->_value.loadable = false;
					i->_value.preload = false;
					break;
				}
				delete stream;
				stream = 0;
			}

			if (i->_value.type == ResFileEntry::kAutoDetect) {
				i->_value.type = ResFileEntry::kRaw;
				i->_value.loadable = true;
			}
		}
	}
}

#pragma mark -
#pragma mark - ResFileLodaer
#pragma mark -

class ResLoaderPak : public ResArchiveLoader {
public:
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const;
	bool loadFile(const Common::String &filename, Common::SeekableReadStream &stream, ResFileMap &map) const;
	Common::SeekableReadStream *loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileMap &map) const;

	ResFileEntry::kType getType() const {
		return ResFileEntry::kPak;
	}
};

bool ResLoaderPak::isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const {
	// TODO improve check:
	Common::String file = filename;
	file.toUppercase();
	return ((file.hasSuffix(".PAK") && file != "TWMUSIC.PAK") || file.hasSuffix(".APK") || file.hasSuffix(".VRM") || file.hasSuffix(".TLK"));
}

bool ResLoaderPak::loadFile(const Common::String &filename, Common::SeekableReadStream &stream, ResFileMap &map) const {
	uint32 filesize = stream.size();
	
	Common::List<Common::String> filenames;
	Common::List<ResFileEntry> entries;

	uint32 pos = 0, startoffset = 0, endoffset = 0;
	bool switchEndian = false;

	startoffset = stream.readUint32LE(); pos += 4;
	if (startoffset > filesize) {
		switchEndian = true;
		startoffset = SWAP_BYTES_32(startoffset);
	}

	while (pos < filesize) {
		uint8 buffer[64];
		uint32 nameLength;

		// Move to the position of the next file entry
		stream.seek(pos);

		// Read in the header
		if (stream.read(&buffer, 64) < 5) {
			warning("PAK file '%s' is corrupted", filename.c_str());
			return false;
		}

		// Quit now if we encounter an empty string
		if (!(*((const char*)buffer)))
			break;

		nameLength = strlen((const char*)buffer) + 1;

		if (nameLength > 60) {
			warning("PAK file '%s' is corrupted", filename.c_str());
			return false;
		}

		endoffset = (switchEndian ? READ_BE_UINT32 : READ_LE_UINT32)(buffer + nameLength);

		if (!endoffset)
			endoffset = filesize;

		if (startoffset != endoffset) {
			ResFileEntry entry;
			entry.size = endoffset - startoffset;
			entry.offset = startoffset;
			entry.parent = filename;
			entry.type = ResFileEntry::kAutoDetect;
			entry.loadable = true;
			entry.prot = false;
			entry.preload = false;

			filenames.push_back(Common::String((const char*)buffer));
			entries.push_back(entry);
		}

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
		pos += nameLength + 4;
	}

	assert(filenames.size() == entries.size());

	Common::List<ResFileEntry>::iterator entry = entries.begin();
	Common::List<Common::String>::iterator file = filenames.begin();

	for (; entry != entries.end(); ++entry, ++file) {
		map.erase(*file);
		map[*file] = *entry;
	}

	return true;
}

Common::SeekableReadStream *ResLoaderPak::loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileMap &map) const {
	assert(archive);

	ResFileMap::const_iterator entry = map.find(file);
	if (entry == map.end())
		return 0;

	archive->seek(entry->_value.offset, SEEK_SET);
	Common::SeekableSubReadStream *stream = new Common::SeekableSubReadStream(archive, entry->_value.offset, entry->_value.offset + entry->_value.size, true);
	assert(stream);
	return stream;
}

class ResLoaderIns : public ResArchiveLoader {
public:
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const;
	bool loadFile(const Common::String &filename, Common::SeekableReadStream &stream, ResFileMap &map) const;
	Common::SeekableReadStream *loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileMap &map) const;

	ResFileEntry::kType getType() const {
		return ResFileEntry::kIns;
	}
};

bool ResLoaderIns::isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const {
	stream.seek(3);
	uint32 size = stream.readUint32LE();

	if (size > stream.size())
		return false;

	stream.seek(size+1, SEEK_SET);
	uint8 buffer[2];
	stream.read(&buffer, 2);

	return (buffer[0] == 0x0D && buffer[1] == 0x0A);
}

bool ResLoaderIns::loadFile(const Common::String &filename, Common::SeekableReadStream &stream, ResFileMap &map) const {
	Common::List<Common::String> filenames;

	// thanks to eriktorbjorn for this code (a bit modified though)
	stream.seek(3, SEEK_SET);

	// first file is the index table
	uint32 size = stream.readUint32LE();
	Common::String temp = "";

	for (uint32 i = 0; i < size; ++i) {
		byte c = stream.readByte();

		if (c == '\\') {
			temp = "";
		} else if (c == 0x0D) {
			// line endings are CRLF
			c = stream.readByte();
			assert(c == 0x0A);
			++i;

			filenames.push_back(temp);
		} else {
			temp += (char)c;
		}
	}

	stream.seek(3, SEEK_SET);

	for (Common::List<Common::String>::iterator file = filenames.begin(); file != filenames.end(); ++file) {
		map.erase(*file);

		ResFileEntry entry;
		entry.parent = filename;
		entry.type = ResFileEntry::kAutoDetect;
		entry.loadable = true;
		entry.preload = false;
		entry.prot = false;
		entry.size = stream.readUint32LE();
		entry.offset = stream.pos();
		stream.seek(entry.size, SEEK_CUR);

		map[*file] = entry;
	}

	return true;
}

Common::SeekableReadStream *ResLoaderIns::loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileMap &map) const {
	assert(archive);

	ResFileMap::const_iterator entry = map.find(file);
	if (entry == map.end())
		return 0;

	archive->seek(entry->_value.offset, SEEK_SET);
	Common::SeekableSubReadStream *stream = new Common::SeekableSubReadStream(archive, entry->_value.offset, entry->_value.offset + entry->_value.size, true);
	assert(stream);
	return stream;
}

void Resource::initializeLoaders() {
	_loaders.push_back(new ResLoaderPak());
	_loaders.push_back(new ResLoaderIns());
}

const ResArchiveLoader *Resource::getLoader(ResFileEntry::kType type) const {
	for (CLoaderIterator i = _loaders.begin(); i != _loaders.end(); ++i) {
		if ((*i)->getType() == type)
			return *i;
	}
	return 0;
}

} // end of namespace Kyra

