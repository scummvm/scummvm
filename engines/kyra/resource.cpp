/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/hash-str.h"

#include "gui/message.h"

#include "kyra/resource.h"
#include "kyra/script.h"
#include "kyra/wsamovie.h"
#include "kyra/screen.h"

namespace Kyra {
Resource::Resource(KyraEngine *engine) {
	_engine = engine;

	if (_engine->game() == GI_KYRA1) {
		// we're loading KYRA.DAT here too (but just for Kyrandia 1)
		if (!loadPakFile("KYRA.DAT", true) || !StaticResource::checkKyraDat()) {
			GUI::MessageDialog errorMsg("You're missing the 'KYRA.DAT' file or it got corrupted, (re)get it from the ScummVM website");
			errorMsg.runModal();
			error("You're missing the 'KYRA.DAT' file or it got corrupted, (re)get it from the ScummVM website");
		}

		// We only need kyra.dat for the demo.
		if (_engine->gameFlags().isDemo)
			return;

		// only VRM file we need in the *whole* game for kyra1
		if (_engine->gameFlags().isTalkie)
			loadPakFile("CHAPTER1.VRM");
	} else if (_engine->game() == GI_KYRA3) {
		// load the installation package file for kyra3
		INSFile *insFile = new INSFile("WESTWOOD.001");
		assert(insFile);
		if (!insFile->isValid())
			return;
		_pakfiles.push_back(insFile);
	}

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));

	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly))
		error("invalid game path '%s'", dir.path().c_str());

	if (_engine->game() == GI_KYRA1 && _engine->gameFlags().isTalkie) {
		static const char *list[] = {
			"ADL.PAK", "CHAPTER1.VRM", "COL.PAK", "FINALE.PAK", "INTRO1.PAK", "INTRO2.PAK",
			"INTRO3.PAK", "INTRO4.PAK", "MISC.PAK",	"SND.PAK", "STARTUP.PAK", "XMI.PAK",
			"CAVE.APK", "DRAGON1.APK", "DRAGON2.APK", "LAGOON.APK", 0
		};

		for (int i = 0; list[i]; ++i) {
			if (!loadPakFile(list[i]))
				error("couldn't open pakfile '%s'", list[i]);
		}

		Common::List<ResourceFile*>::iterator start = _pakfiles.begin();
		for (;start != _pakfiles.end(); ++start) {
			(*start)->protect();
		}
	} else {
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			Common::String filename = file->name();
			filename.toUppercase();

			// No real PAK file!
			if (filename == "TWMUSIC.PAK")
				continue;

			if (filename.hasSuffix("PAK") || filename.hasSuffix("APK")) {
				if (!loadPakFile(file->name())) {
					error("couldn't open pakfile '%s'", file->name().c_str());
				}
			}
		}

		if (_engine->gameFlags().platform == Common::kPlatformFMTowns) {
			Common::List<ResourceFile*>::iterator start = _pakfiles.begin();
			uint unloadHash = (_engine->gameFlags().lang == Common::EN_ANY) ? Common::hashit_lower("JMC.PAK") : Common::hashit_lower("EMC.PAK");

			for (;start != _pakfiles.end(); ++start) {
				if ((*start)->filename() == unloadHash) {
					delete *start;
					*start = 0;
					_pakfiles.erase(start);
					break;
				}
			}
		}
	}
}

Resource::~Resource() {
	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();

	for (;start != _pakfiles.end(); ++start) {
		delete *start;
		*start = 0;
	}
}

bool Resource::loadPakFile(const Common::String &filename, const bool forcePC) {
	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();
	uint hash = Common::hashit_lower(filename.c_str());

	for (;start != _pakfiles.end(); ++start) {
		if ((*start)->filename() == hash) {
			(*start)->open();
			return true;
		}
	}

	uint32 size = 0;
	
	Common::File handle;
	if (!getFileHandle(filename.c_str(), &size, handle)) {
		warning("couldn't load file: '%s'", filename.c_str());
		return false;
	}

	PAKFile *file = new PAKFile(filename.c_str(), handle.name(), handle, (_engine->gameFlags().platform == Common::kPlatformAmiga) && !forcePC);
	handle.close();

	if (!file)
		return false;

	if (!file->isValid()) {
		warning("'%s' is no valid pak file", filename.c_str());
		delete file;
		return false;
	}

	_pakfiles.push_back(file);
	return true;
}

void Resource::unloadPakFile(const Common::String &filename) {
	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();
	uint hash = Common::hashit_lower(filename.c_str());
	for (;start != _pakfiles.end(); ++start) {
		if ((*start)->filename() == hash) {
			(*start)->close();
			break;
		}
	}
	return;
}

bool Resource::isInPakList(const Common::String &filename) {
	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();
	uint hash = Common::hashit_lower(filename.c_str());
	for (;start != _pakfiles.end(); ++start) {
		if ((*start)->filename() == hash)
			return true;
	}
	return false;
}

uint8 *Resource::fileData(const char *file, uint32 *size) {
	Common::File fileHandle;

	if (size)
		*size = 0;

	// test to open it in the main dir
	if (fileHandle.open(file)) {
		uint32 fileSize = fileHandle.size();
		uint8 *buffer = new uint8[fileSize];
		assert(buffer);

		fileHandle.read(buffer, fileSize);

		if (size)
			*size = fileSize;

		return buffer;
	} else {
		// opens the file in a PAK File
		Common::List<ResourceFile*>::iterator start = _pakfiles.begin();

		uint fileHash = Common::hashit_lower(file);
		for (;start != _pakfiles.end(); ++start) {
			if (!(*start)->isOpen())
				continue;

			uint32 fileSize = (*start)->getFileSize(fileHash);

			if (!fileSize)
				continue;

			if (size)
				*size = fileSize;

			return (*start)->getFile(fileHash);
		}
	}

	return 0;
}

bool Resource::getFileHandle(const char *file, uint32 *size, Common::File &filehandle) {
	filehandle.close();

	if (filehandle.open(file))
		return true;

	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();

	uint fileHash = Common::hashit_lower(file);
	for (;start != _pakfiles.end(); ++start) {
		if (!(*start)->isOpen())
			continue;

		*size = (*start)->getFileSize(fileHash);
		
		if (!(*size))
			continue;

		if ((*start)->getFileHandle(fileHash, filehandle))
			return true;
	}
	
	return false;
}

uint32 Resource::getFileSize(const char *file) {
	Common::List<ResourceFile*>::iterator start = _pakfiles.begin();

	Common::File temp;
	if (temp.open(file))
		return temp.size();

	uint fileHash = Common::hashit_lower(file);
	for (;start != _pakfiles.end(); ++start) {
		if (!(*start)->isOpen())
			continue;

		uint32 size = (*start)->getFileSize(fileHash);
		
		if (size)
			return size;
	}

	return 0;
}

bool Resource::loadFileToBuf(const char *file, void *buf, uint32 maxSize) {
	Common::File tempHandle;
	uint32 size = 0;

	if (!getFileHandle(file, &size, tempHandle))
		return false;

	if (size > maxSize)
		return false;

	memset(buf, 0, maxSize);
	tempHandle.read(buf, size);

	return true;
}

///////////////////////////////////////////
// Pak file manager
#define PAKFile_Iterate Common::List<PakChunk>::iterator start=_files.begin();start != _files.end(); ++start
PAKFile::PAKFile(const char *file, const char *physfile, Common::File &pakfile, bool isAmiga) : ResourceFile() {
	_open = false;

	if (!pakfile.isOpen()) {
		debug(3, "couldn't open pakfile '%s'\n", file);
		return;
	}

	uint32 off = pakfile.pos();
	uint32 filesize = pakfile.size();

	uint32 pos = 0, startoffset = 0, endoffset = 0;

	if (!isAmiga)
		startoffset = pakfile.readUint32LE();
	else
		startoffset = pakfile.readUint32BE();

	if (startoffset > filesize) {
		warning("PAK file '%s' is corrupted", file);
		return;
	}

	pos += 4;

	while (pos < filesize) {
		PakChunk chunk;
		uint8 buffer[64];
		uint32 nameLength;
		
		// Move to the position of the next file entry
		pakfile.seek(pos);
		
		// Read in the header
		if (pakfile.read(&buffer, 64) < 5) {
			warning("PAK file '%s' is corrupted", file);
			return;
		}
		
		// Quit now if we encounter an empty string
		if (!(*((const char*)buffer)))
			break;

		chunk._name = Common::hashit_lower((const char*)buffer);
		nameLength = strlen((const char*)buffer) + 1; 

		if (nameLength > 60) {
			warning("PAK file '%s' is corrupted", file);
			return;
		}

		if (!isAmiga)
			endoffset = READ_LE_UINT32(buffer + nameLength);
		else
			endoffset = READ_BE_UINT32(buffer + nameLength);

		if (!endoffset) {
			endoffset = filesize;
		} else if (endoffset > filesize || startoffset > endoffset) {
			warning("PAK file '%s' is corrupted", file);
			return;
		}

		if (startoffset != endoffset) {
			chunk._start = startoffset;
			chunk._size = endoffset - startoffset;

			_files.push_back(chunk);
		}

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
		pos += nameLength + 4;
	}

	_open = true;	
	_filename = Common::hashit_lower(file);
	_physfile = physfile;
	_physOffset = off;
}


PAKFile::~PAKFile() {
	_physfile.clear();
	_open = false;

	_files.clear();
}

uint8 *PAKFile::getFile(uint hash) {
	for (PAKFile_Iterate) {
		if (start->_name == hash) {
			Common::File pakfile;
			if (!openFile(pakfile))
				return false;

			pakfile.seek(start->_start, SEEK_CUR);
			uint8 *buffer = new uint8[start->_size];
			assert(buffer);
			pakfile.read(buffer, start->_size);
			return buffer;
		}
	}
	return 0;
}

bool PAKFile::getFileHandle(uint hash, Common::File &filehandle) {
	filehandle.close();

	for (PAKFile_Iterate) {
		if (start->_name == hash) {
			if (!openFile(filehandle))
				return false;

			filehandle.seek(start->_start, SEEK_CUR);
			return true;
		}
	}
	return false;
}

uint32 PAKFile::getFileSize(uint hash) {
	for (PAKFile_Iterate) {
		if (start->_name == hash)
			return start->_size;
	}
	return 0;
}

bool PAKFile::openFile(Common::File &filehandle) {
	filehandle.close();

	if (!filehandle.open(_physfile))
		return false;

	filehandle.seek(_physOffset, SEEK_CUR);
	return true;
}

///////////////////////////////////////////
// Ins file manager
#define INSFile_Iterate Common::List<FileEntry>::iterator start=_files.begin();start != _files.end(); ++start
INSFile::INSFile(const char *file) : ResourceFile(), _files() {
	Common::File pakfile;
	_open = false;

	if (!pakfile.open(file)) {
		debug(3, "couldn't open insfile '%s'\n", file);
		return;
	}

	// thanks to eriktorbjorn for this code (a bit modified though)

	// skip first three bytes
	pakfile.seek(3);

	// first file is the index table
	uint32 filesize = pakfile.readUint32LE();

	Common::String temp = "";

	for (uint i = 0; i < filesize; ++i) {
		byte c = pakfile.readByte();

		if (c == '\\') {
			temp = "";
		} else if (c == 0x0D) {
			// line endings are CRLF
			c = pakfile.readByte();
			assert(c == 0x0A);
			++i;

			FileEntry newEntry;
			newEntry._name = Common::hashit_lower(temp.c_str());
			newEntry._start = 0;
			newEntry._size = 0;
			_files.push_back(newEntry);

			temp = "";
		} else {
			temp += (char)c;
		}
	}

	pakfile.seek(3);

	for (INSFile_Iterate) {
		filesize = pakfile.readUint32LE();
		start->_size = filesize;
		start->_start = pakfile.pos();
		pakfile.seek(filesize, SEEK_CUR);
	}

	_filename = Common::hashit_lower(file);
	_physfile = file;
	_open = true;
}

INSFile::~INSFile() {
	_open = false;

	_files.clear();
}

uint8 *INSFile::getFile(uint hash) {
	for (INSFile_Iterate) {
		if (start->_name == hash) {
			Common::File pakfile;
			if (!pakfile.open(_physfile))
				return false;

			pakfile.seek(start->_start);
			uint8 *buffer = new uint8[start->_size];
			assert(buffer);
			pakfile.read(buffer, start->_size);
			return buffer;
		}
	}
	return 0;
}

bool INSFile::getFileHandle(uint hash, Common::File &filehandle) {
	for (INSFile_Iterate) {
		if (start->_name == hash) {
			if (!filehandle.open(_physfile)) 
				return false;

			filehandle.seek(start->_start, SEEK_CUR);
			return true;
		}
	}
	return false;
}

uint32 INSFile::getFileSize(uint hash) {
	for (INSFile_Iterate) {
		if (start->_name == hash)
			return start->_size;
	}
	return 0;
}

} // end of namespace Kyra
