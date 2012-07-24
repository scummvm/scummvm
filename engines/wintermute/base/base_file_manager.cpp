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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/file/base_disk_file.h"
#include "engines/wintermute/base/file/base_save_thumb_file.h"
#include "engines/wintermute/base/file/base_file_entry.h"
#include "engines/wintermute/base/file/base_package.h"
#include "engines/wintermute/base/file/base_resources.h"
#include "engines/wintermute/base/file/BPkgFile.h"
#include "engines/wintermute/base/base_registry.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/file/dcpackage.h"
#include "engines/wintermute/wintermute.h"
#include "common/str.h"
#include "common/tokenizer.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/fs.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
BaseFileManager::BaseFileManager(BaseGame *inGame) : _gameRef(inGame) {
	initPaths();
	registerPackages();
}

//////////////////////////////////////////////////////////////////////
BaseFileManager::~BaseFileManager() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::cleanup() {
	// delete registered paths
	_packagePaths.clear();

	// delete file entries
	_filesIter = _files.begin();
	while (_filesIter != _files.end()) {
		delete _filesIter->_value;
		_filesIter++;
	}
	_files.clear();

	// close open files
	for (uint32 i = 0; i < _openFiles.size(); i++) {
		delete _openFiles[i];
	}
	_openFiles.clear();


	// delete packages
	for (uint32 i = 0; i < _packages.size(); i++)
		delete _packages[i];
	_packages.clear();

	return STATUS_OK;
}



//////////////////////////////////////////////////////////////////////
byte *BaseFileManager::readWholeFile(const Common::String &filename, uint32 *size, bool mustExist) {
	byte *buffer = NULL;

	Common::SeekableReadStream *file = openFile(filename);
	if (!file) {
		if (mustExist)
			debugC(kWinterMuteDebugFileAccess | kWinterMuteDebugLog, "Error opening file '%s'", filename.c_str());
		return NULL;
	}

	buffer = new byte[file->size() + 1];
	if (buffer == NULL) {
		debugC(kWinterMuteDebugFileAccess | kWinterMuteDebugLog, "Error allocating buffer for file '%s' (%d bytes)", filename.c_str(), file->size() + 1);
		closeFile(file);
		return NULL;
	}

	if (file->read(buffer, (uint32)file->size()) != (uint32)file->size()) {
		debugC(kWinterMuteDebugFileAccess | kWinterMuteDebugLog, "Error reading file '%s'", filename.c_str());
		closeFile(file);
		delete [] buffer;
		return NULL;
	};

	buffer[file->size()] = '\0';
	if (size != NULL) *size = file->size();
	closeFile(file);

	return buffer;
}

Common::SeekableReadStream *BaseFileManager::loadSaveGame(const Common::String &filename) {
	Common::SaveFileManager *saveMan = g_wintermute->getSaveFileMan();
	Common::InSaveFile *file = saveMan->openForLoading(filename);
	return file;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::saveFile(const Common::String &filename, byte *buffer, uint32 bufferSize, bool compressed, byte *prefixBuffer, uint32 prefixSize) {
	// TODO
	warning("Implement SaveFile");

	Common::SaveFileManager *saveMan = g_wintermute->getSaveFileMan();
	Common::OutSaveFile *file = saveMan->openForSaving(filename);
	file->write(prefixBuffer, prefixSize);
	file->write(buffer, bufferSize);
	file->finalize();
	delete file;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::requestCD(int cd, char *packageFile, const char *filename) {
	// unmount all non-local packages
	for (uint32 i = 0; i < _packages.size(); i++) {
		if (_packages[i]->_cd > 0) _packages[i]->close();
	}


	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::addPath(TPathType type, const Common::FSNode &path) {
	if (!path.exists())
		return STATUS_FAILED;

	switch (type) {
	case PATH_SINGLE:
	//	_singlePaths.push_back(path);
		error("TODO: Allow adding single-paths");
		break;
	case PATH_PACKAGE:
		_packagePaths.push_back(path);
		break;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::reloadPaths() {
	// delete registered paths
	//_singlePaths.clear();
	_packagePaths.clear();

	return initPaths();
}


#define TEMP_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::initPaths() {
	if (!_gameRef) // This function only works when the game-registry is loaded
		return STATUS_FAILED;

	AnsiString pathList;

	// single files paths
	pathList = _gameRef->_registry->readString("Resource", "CustomPaths", "");
	Common::StringTokenizer *entries = new Common::StringTokenizer(pathList, ";");
//	numPaths = BaseUtils::strNumEntries(pathList.c_str(), ';');
	while (!entries->empty()) {
		Common::String path = entries->nextToken();
		if (path.size() > 0) {
			error("BaseFileManager::initPaths - Game wants to add customPath: %s", path.c_str()); // TODO
			//			addPath(PATH_SINGLE, path);
		}
	}
	delete entries;
	entries = NULL;

	// package files paths
	const Common::FSNode gameData(ConfMan.get("path"));
	addPath(PATH_PACKAGE, gameData);

	pathList = _gameRef->_registry->readString("Resource", "PackagePaths", "");
	entries = new Common::StringTokenizer(pathList, ";");
	while (!entries->empty()) {
		Common::String path = entries->nextToken();
		if (path.size() > 0) {
			error("BaseFileManager::initPaths - Game wants to add packagePath: %s", path.c_str()); // TODO
			//			addPath(PATH_SINGLE, path);
		}
	}
	delete entries;
	entries = NULL;

	Common::FSNode dataSubFolder = gameData.getChild("data");
	if (dataSubFolder.exists()) {
		addPath(PATH_PACKAGE, dataSubFolder);
	}
	Common::FSNode languageSubFolder = gameData.getChild("language");
	if (languageSubFolder.exists()) {
		addPath(PATH_PACKAGE, languageSubFolder);
	}
	return STATUS_OK;
}

bool BaseFileManager::registerPackages(const Common::FSList &fslist) {
	for (Common::FSList::const_iterator it = fslist.begin(); it != fslist.end(); it++) {
		debugC(kWinterMuteDebugFileAccess, "Adding %s", (*it).getName().c_str());
		if ((*it).getName().contains(".dcp")) {
			if (registerPackage((*it))) {
				addPath(PATH_PACKAGE, (*it));
			}
		}
	}
	debugC(kWinterMuteDebugFileAccess, "  Registered %d files in %d package(s)", _files.size(), _packages.size());
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::registerPackages() {
	debugC(kWinterMuteDebugFileAccess | kWinterMuteDebugLog, "Scanning packages");

	// Register without using SearchMan, as otherwise the FSNode-based lookup in openPackage will fail
	// and that has to be like that to support the detection-scheme.
	Common::FSList files;
	for (Common::FSList::iterator it = _packagePaths.begin(); it != _packagePaths.end(); it++) {
		warning("Should register %s %s", (*it).getPath().c_str(), (*it).getName().c_str());
		(*it).getChildren(files, Common::FSNode::kListFilesOnly);
		for (Common::FSList::iterator fileIt = files.begin(); fileIt != files.end(); fileIt++) {
			if (!fileIt->getName().hasSuffix(".dcp"))
				continue;
			// Avoid registering all the language files
			// TODO: Select based on the gameDesc.
			if (fileIt->getParent().getName() == "language") {
				Common::String parentName = fileIt->getParent().getName();
				Common::String dcpName = fileIt->getName();
				if (fileIt->getName() != "english.dcp")
					continue;
			}
			warning("Registering %s %s", (*fileIt).getPath().c_str(), (*fileIt).getName().c_str());
			registerPackage((*fileIt));
		}
	}

	debugC(kWinterMuteDebugFileAccess | kWinterMuteDebugLog, "  Registered %d files in %d package(s)", _files.size(), _packages.size());

	return STATUS_OK;
}

bool BaseFileManager::registerPackage(Common::FSNode file, const Common::String &filename, bool searchSignature) {
	uint32 absoluteOffset = 0;
	bool boundToExe = false;
	Common::SeekableReadStream * package = file.createReadStream();
	if (!package)
		return STATUS_FAILED;
	if (searchSignature) {
		uint32 offset;
		if (!findPackageSignature(package, &offset)) {
			delete package;
			return STATUS_OK;
		} else {
			package->seek(offset, SEEK_SET);
			absoluteOffset = offset;
			boundToExe = true;
		}
	}

	TPackageHeader hdr;
	hdr.readFromStream(package);
	if (hdr._magic1 != PACKAGE_MAGIC_1 || hdr._magic2 != PACKAGE_MAGIC_2 || hdr._packageVersion > PACKAGE_VERSION) {
		debugC(kWinterMuteDebugFileAccess | kWinterMuteDebugLog, "  Invalid header in package file '%s'. Ignoring.", filename.c_str());
		delete package;
		return STATUS_OK;
	}

	if (hdr._packageVersion != PACKAGE_VERSION) {
		debugC(kWinterMuteDebugFileAccess | kWinterMuteDebugLog, "  Warning: package file '%s' is outdated.", filename.c_str());
	}

	// new in v2
	if (hdr._packageVersion == PACKAGE_VERSION) {
		uint32 dirOffset;
		dirOffset = package->readUint32LE();
		dirOffset += absoluteOffset;
		package->seek(dirOffset, SEEK_SET);
	}

	for (uint32 i = 0; i < hdr._numDirs; i++) {
		BasePackage *pkg = new BasePackage(this);
		pkg->_fsnode = file;
		if (!pkg) return STATUS_FAILED;

		pkg->_boundToExe = boundToExe;

		// read package info
		byte nameLength = package->readByte();
		pkg->_name = new char[nameLength];
		package->read(pkg->_name, nameLength);
		pkg->_cd = package->readByte();
		pkg->_priority = hdr._priority;

		if (!hdr._masterIndex) pkg->_cd = 0; // override CD to fixed disk
		_packages.push_back(pkg);


		// read file entries
		uint32 numFiles = package->readUint32LE();

		for (uint32 j = 0; j < numFiles; j++) {
			char *name;
			uint32 offset, length, compLength, flags, timeDate1, timeDate2;

			nameLength = package->readByte();
			name = new char[nameLength];
			package->read(name, nameLength);

			// v2 - xor name
			if (hdr._packageVersion == PACKAGE_VERSION) {
				for (int k = 0; k < nameLength; k++) {
					((byte *)name)[k] ^= 'D';
				}
			}
			debugC(kWinterMuteDebugFileAccess, "Package contains %s", name);
			// some old version of ProjectMan writes invalid directory entries
			// so at least prevent strupr from corrupting memory
			name[nameLength - 1] = '\0';
			
			Common::String upcName = name;
			upcName.toUppercase();
			delete[] name;
			name = NULL;

			offset = package->readUint32LE();
			offset += absoluteOffset;
			length = package->readUint32LE();
			compLength = package->readUint32LE();
			flags = package->readUint32LE();

			if (hdr._packageVersion == PACKAGE_VERSION) {
				timeDate1 = package->readUint32LE();
				timeDate2 = package->readUint32LE();
			}
			_filesIter = _files.find(upcName.c_str());
			if (_filesIter == _files.end()) {
				BaseFileEntry *file = new BaseFileEntry();
				file->_package = pkg;
				file->_offset = offset;
				file->_length = length;
				file->_compressedLength = compLength;
				file->_flags = flags;

				_files[upcName.c_str()] = file;
			} else {
				// current package has lower CD number or higher priority, than the registered
				if (pkg->_cd < _filesIter->_value->_package->_cd || pkg->_priority > _filesIter->_value->_package->_priority) {
					_filesIter->_value->_package = pkg;
					_filesIter->_value->_offset = offset;
					_filesIter->_value->_length = length;
					_filesIter->_value->_compressedLength = compLength;
					_filesIter->_value->_flags = flags;
				}
			}
		}
	}


	delete package;
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
Common::File *BaseFileManager::openPackage(const Common::String &name) {
	//TODO: Is it really necessary to do this when we have the ScummVM-system?
	Common::File *ret = new Common::File();
	for (Common::FSList::iterator it = _packagePaths.begin(); it != _packagePaths.end(); it++) {
		Common::String packageName = (*it).getName();
		if (packageName == (name + ".dcp"))
			ret->open((*it));
		if (ret->isOpen()) {
			return ret;
		}
	}
	Common::String filename = Common::String::format("%s.%s", name.c_str(), PACKAGE_EXTENSION);
	ret->open(filename);
	if (ret->isOpen()) {
		return ret;
	}
	warning("BaseFileManager::OpenPackage - Couldn't load file %s", name.c_str());
	delete ret;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
BaseFileEntry *BaseFileManager::getPackageEntry(const Common::String &filename) {
	Common::String upc_name = filename;
	upc_name.toUppercase();

	BaseFileEntry *ret = NULL;
	_filesIter = _files.find(upc_name.c_str());
	if (_filesIter != _files.end()) ret = _filesIter->_value;

	return ret;
}

bool BaseFileManager::hasFile(const Common::String &filename) {
	if (diskFileExists(filename))
		return true;
	if (getPackageEntry(filename))
		return true; // We don't bother checking if the file can actually be opened, something bigger is wrong if that is the case.
	if (BaseResources::hasFile(filename))
		return true;
	return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BaseFileManager::openFile(const Common::String &filename, bool absPathWarning, bool keepTrackOf) {
	if (strcmp(filename.c_str(), "") == 0)
		return NULL;
	debugC(kWinterMuteDebugFileAccess, "Open file %s", filename.c_str());

	Common::SeekableReadStream *file = openFileRaw(filename);
	if (file && keepTrackOf) _openFiles.push_back(file);
	return file;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::closeFile(Common::SeekableReadStream *File) {
	for (uint32 i = 0; i < _openFiles.size(); i++) {
		if (_openFiles[i] == File) {
			delete _openFiles[i];
			_openFiles.remove_at(i);
			return STATUS_OK;
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BaseFileManager::openFileRaw(const Common::String &filename) {
	Common::SeekableReadStream *ret = NULL;

	if (scumm_strnicmp(filename.c_str(), "savegame:", 9) == 0) {
		if (!_gameRef)
			error("Attempt to load filename: %s without BaseGame-object, this is unsupported", filename.c_str());
		BaseSaveThumbFile *SaveThumbFile = new BaseSaveThumbFile(_gameRef);
		if (DID_SUCCEED(SaveThumbFile->open(filename))) {
			ret = SaveThumbFile->getMemStream();
		} 
		delete SaveThumbFile;
		return ret;
	}

	ret = openDiskFile(filename);
	if (ret)
		return ret;

	ret = openPkgFile(filename, this);
	if (ret)
		return ret;

	ret = BaseResources::getFile(filename);
	if (ret)
		return ret;

	warning("BFileManager::OpenFileRaw - Failed to open %s", filename.c_str());
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::findPackageSignature(Common::SeekableReadStream *f, uint32 *offset) {
	byte buf[32768];

	byte signature[8];
	((uint32 *)signature)[0] = PACKAGE_MAGIC_1;
	((uint32 *)signature)[1] = PACKAGE_MAGIC_2;

	uint32 fileSize = (uint32)f->size();
	uint32 startPos = 1024 * 1024;
	uint32 bytesRead = startPos;

	while (bytesRead < fileSize - 16) {
		uint32 toRead = MIN((unsigned int)32768, fileSize - bytesRead);
		f->seek((int32)startPos, SEEK_SET);
		uint32 actuallyRead = f->read(buf, toRead);
		if (actuallyRead != toRead) return false;

		for (uint32 i = 0; i < toRead - 8; i++)
			if (!memcmp(buf + i, signature, 8)) {
				*offset =  startPos + i;
				return true;
			}

		bytesRead = bytesRead + toRead - 16;
		startPos = startPos + toRead - 16;

	}
	return false;

}

} // end of namespace WinterMute
