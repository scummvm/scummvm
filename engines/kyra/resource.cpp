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

#include "gui/message.h"

#include "kyra/resource.h"

namespace Kyra {

Resource::Resource(KyraEngine *vm) : _loaders(), _map(), _vm(vm) {
	initializeLoaders();
}

Resource::~Resource() {
	unloadAllPakFiles();
	_loaders.clear();
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
		if (_vm->gameFlags().isDemo && !_vm->gameFlags().isTalkie) {
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

		for (int i = 0; i < ARRAYSIZE(list); ++i) {
			ResFileMap::iterator iterator = _map.find(list[i]);
			if (iterator != _map.end())
				iterator->_value.prot = true;
		}
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

	if (iter->_value.preload) {
		iter->_value.mounted = true;
		return true;
	}

	const ResArchiveLoader *loader = getLoader(iter->_value.type);
	if (!loader) {
		error("no archive loader for file '%s' found which is of type %d", filename.c_str(), iter->_value.type);
		return false;
	}

	if (!isAccessable(filename))
		return false;

	Common::SeekableReadStream *stream = getFileStream(filename);
	if (!stream) {
		error("archive file '%s' not found", filename.c_str());
		return false;
	}

	iter->_value.mounted = true;
	iter->_value.preload = true;
	ResArchiveLoader::FileList files;
	loader->loadFile(filename, *stream, files);
	delete stream;
	stream = 0;
	
	for (ResArchiveLoader::FileList::iterator i = files.begin(); i != files.end(); ++i) {
		iter = _map.find(i->filename);
		if (iter == _map.end()) {
			// A new file entry, so we just insert it into the file map.
			_map[i->filename] = i->entry;
		} else if (!iter->_value.parent.empty()) {
			if (!iter->_value.parent.equalsIgnoreCase(filename)) {
				ResFileMap::iterator oldParent = _map.find(iter->_value.parent);
				if (oldParent != _map.end()) {
					// Protected files and their embedded file entries do not get overwritten.
					if (!oldParent->_value.prot) {
						// If the old parent is not protected we mark it as not preload anymore,
						// since now no longer all of its embedded files are in the filemap.
						oldParent->_value.preload = false;
						_map[i->filename] = i->entry;
					}
				} else {
					// Old parent not found? That's strange... But we just overwrite the old
					// entry.
					_map[i->filename] = i->entry;
				}
			} else {
				// The old parent has the same filenames as the new archive, we are sure and overwrite the
				// old file entry, could be afterall that the preload flag of the new archive was
				// just unflagged.
				_map[i->filename] = i->entry;
			}
		}
		// 'else' case would mean here overwriting an existing file entry in the map without parent.
		// We don't support that though, so one can overwrite files from archives by putting
		// them in the gamepath.
	}

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
			if (!isAccessable(filename) && _vm->gameFlags().isDemo) {
				// the demo version supplied with Kyra3 does not 
				// contain all pak files listed in filedata.fdt
				// so we don't do anything here if they are non
				// existant.
			} else if (!loadPakFile(filename)) {
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
			iter->_value.mounted = false;
	}
}

bool Resource::isInPakList(const Common::String &filename) const {
	return isAccessable(filename);
}

void Resource::unloadAllPakFiles() {
	// remove all entries
	_map.clear();

	addSearchPath(ConfMan.get("path"));
	addSearchPath(ConfMan.get("extrapath"));
	
	Common::File temp;
	
	ResFileMap::iterator iter = _map.find(StaticResource::staticDataFilename());
	if (iter == _map.end()) {
		if (temp.open(StaticResource::staticDataFilename())) {
			ResFileEntry entry;
			entry.parent = "";
			entry.size = temp.size();
			entry.mounted = true;
			entry.preload = false;
			entry.prot = false;
			entry.type = ResFileEntry::kAutoDetect;
			entry.offset = 0;
			_map[StaticResource::staticDataFilename()] = entry;
			temp.close();
		}
	}

	detectFileTypes();
}

bool Resource::addSearchPath(const Common::String &path) {
	if (path.empty())
		return false;

	FilesystemNode dir(path);

	if (!dir.exists() || !dir.isDirectory()) {
		warning("invalid data path '%s'", dir.getPath().c_str());
		return false;
	}

	FSList fslist;
	if (!dir.getChildren(fslist, FilesystemNode::kListFilesOnly)) {
		warning("can't list files inside path '%s'", dir.getPath().c_str());
		return false;
	}

	Common::File temp;

	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		ResFileEntry entry;
		entry.parent = "";
		if (!temp.open(file->getPath()))
			error("couldn't open file '%s'", file->getName().c_str());
		entry.size = temp.size();
		entry.offset = 0;
		entry.mounted = false;
		entry.preload = false;
		entry.prot = false;
		entry.type = ResFileEntry::kAutoDetect;
		_map[file->getName()] = entry;
		temp.close();
	}

	detectFileTypes();
	return true;
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

		return loader->loadFileFromArchive(file, parent, iter->_value);
	} else {
		Common::File *stream = new Common::File();
		if (!stream->open(file)) {
			warning("Couldn't open file '%s'", file.c_str());
			return 0;
		}
		return stream;
	}

	return 0;
}

bool Resource::isAccessable(const Common::String &file) const {
	ResFileMap::const_iterator iter = _map.find(file);
	while (iter != _map.end()) {
		if (!iter->_value.parent.empty()) {
			iter = _map.find(iter->_value.parent);
			if (iter != _map.end()) {
				// parent can never be a non archive file
				if (iter->_value.type == ResFileEntry::kRaw)
					return false;
				// not mounted parent means not accessable
				else if (!iter->_value.mounted)
					return false;
			}
		} else {
			return true;
		}
	}
	return false;
}

void Resource::detectFileTypes() {
	for (ResFileMap::iterator i = _map.begin(); i != _map.end(); ++i) {
		if (!isAccessable(i->_key))
			continue;

		if (i->_value.type == ResFileEntry::kAutoDetect) {
			Common::SeekableReadStream *stream = 0;
			for (LoaderIterator l = _loaders.begin(); l != _loaders.end(); ++l) {
				if (!(*l)->checkFilename(i->_key))
					continue;
				
				if (!stream)
					stream = getFileStream(i->_key);

				if ((*l)->isLoadable(i->_key, *stream)) {
					i->_value.type = (*l)->getType();
					i->_value.mounted = false;
					i->_value.preload = false;
					break;
				}
			}
			delete stream;
			stream = 0;

			if (i->_value.type == ResFileEntry::kAutoDetect)
				i->_value.type = ResFileEntry::kRaw;
		}
	}
}

#pragma mark -
#pragma mark - ResFileLodaer
#pragma mark -

class ResLoaderPak : public ResArchiveLoader {
public:
	bool checkFilename(Common::String filename) const;
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const;
	bool loadFile(const Common::String &filename, Common::SeekableReadStream &stream, FileList &files) const;
	Common::SeekableReadStream *loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileEntry entry) const;

	ResFileEntry::kType getType() const {
		return ResFileEntry::kPak;
	}
};

bool ResLoaderPak::checkFilename(Common::String filename) const {
	filename.toUppercase();
	return (filename.hasSuffix(".PAK") || filename.hasSuffix(".APK") || filename.hasSuffix(".VRM") || filename.hasSuffix(".TLK") || filename.equalsIgnoreCase(StaticResource::staticDataFilename()));
}

bool ResLoaderPak::isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const {
	uint32 filesize = stream.size();
	uint32 offset = 0;
	bool switchEndian = false;
	bool firstFile = true;

	offset = stream.readUint32LE();
	if (offset > filesize) {
		switchEndian = true;
		offset = SWAP_BYTES_32(offset);
	}

	while (!stream.eos()) {
		// The start offset of a file should never be in the filelist
		if (offset < stream.pos() || offset > filesize)
			return false;

		Common::String file = "";
		byte c = 0;

		while (!stream.eos() && (c = stream.readByte()) != 0)
			file += c;

		if (stream.eos())
			return false;

		// Quit now if we encounter an empty string
		if (file.empty()) {
			if (firstFile)
				return false;
			else
				break;
		}

		firstFile = false;
		offset = switchEndian ? stream.readUint32BE() : stream.readUint32LE();

		if (!offset || offset == filesize)
			break;
	}

	return true;
}

bool ResLoaderPak::loadFile(const Common::String &filename, Common::SeekableReadStream &stream, FileList &files) const {
	uint32 filesize = stream.size();
	
	uint32 startoffset = 0, endoffset = 0;
	bool switchEndian = false;
	bool firstFile = true;

	startoffset = stream.readUint32LE();
	if (startoffset > filesize) {
		switchEndian = true;
		startoffset = SWAP_BYTES_32(startoffset);
	}

	while (!stream.eos()) {
		// The start offset of a file should never be in the filelist
		if (startoffset < stream.pos() || startoffset > filesize) {
			warning("PAK file '%s' is corrupted", filename.c_str());
			return false;
		}

		Common::String file = "";
		byte c = 0;

		while (!stream.eos() && (c = stream.readByte()) != 0)
			file += c;

		if (stream.eos()) {
			warning("PAK file '%s' is corrupted", filename.c_str());
			return false;
		}

		// Quit now if we encounter an empty string
		if (file.empty()) {
			if (firstFile) {
				warning("PAK file '%s' is corrupted", filename.c_str());
				return false;
			} else {
				break;
			}
		}

		firstFile = false;
		endoffset = switchEndian ? stream.readUint32BE() : stream.readUint32LE();

		if (!endoffset)
			endoffset = filesize;

		if (startoffset != endoffset) {
			ResFileEntry entry;
			entry.size = endoffset - startoffset;
			entry.offset = startoffset;
			entry.parent = filename;
			entry.type = ResFileEntry::kAutoDetect;
			entry.mounted = false;
			entry.prot = false;
			entry.preload = false;

			files.push_back(File(file, entry));
		}

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
	}

	return true;
}

Common::SeekableReadStream *ResLoaderPak::loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileEntry entry) const {
	assert(archive);

	archive->seek(entry.offset, SEEK_SET);
	Common::SeekableSubReadStream *stream = new Common::SeekableSubReadStream(archive, entry.offset, entry.offset + entry.size, true);
	assert(stream);
	return stream;
}

class ResLoaderIns : public ResArchiveLoader {
public:
	bool checkFilename(Common::String filename) const;
	bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const;
	bool loadFile(const Common::String &filename, Common::SeekableReadStream &stream, FileList &files) const;
	Common::SeekableReadStream *loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileEntry entry) const;

	ResFileEntry::kType getType() const {
		return ResFileEntry::kIns;
	}
};

bool ResLoaderIns::checkFilename(Common::String filename) const {
	filename.toUppercase();
	return (filename.hasSuffix(".001"));
}

bool ResLoaderIns::isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const {
	stream.seek(3);
	uint32 size = stream.readUint32LE();

	if (size+7 > stream.size())
		return false;

	stream.seek(size+5, SEEK_SET);
	uint8 buffer[2];
	stream.read(&buffer, 2);

	return (buffer[0] == 0x0D && buffer[1] == 0x0A);
}

bool ResLoaderIns::loadFile(const Common::String &filename, Common::SeekableReadStream &stream, FileList &files) const {
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
		ResFileEntry entry;
		entry.parent = filename;
		entry.type = ResFileEntry::kAutoDetect;
		entry.mounted = false;
		entry.preload = false;
		entry.prot = false;
		entry.size = stream.readUint32LE();
		entry.offset = stream.pos();
		stream.seek(entry.size, SEEK_CUR);
		files.push_back(File(*file, entry));
	}

	return true;
}

Common::SeekableReadStream *ResLoaderIns::loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileEntry entry) const {
	assert(archive);

	archive->seek(entry.offset, SEEK_SET);
	Common::SeekableSubReadStream *stream = new Common::SeekableSubReadStream(archive, entry.offset, entry.offset + entry.size, true);
	assert(stream);
	return stream;
}

void Resource::initializeLoaders() {
	_loaders.push_back(LoaderList::value_type(new ResLoaderPak()));
	_loaders.push_back(LoaderList::value_type(new ResLoaderIns()));
}

const ResArchiveLoader *Resource::getLoader(ResFileEntry::kType type) const {
	for (CLoaderIterator i = _loaders.begin(); i != _loaders.end(); ++i) {
		if ((*i)->getType() == type)
			return (*i).get();
	}
	return 0;
}

} // end of namespace Kyra


