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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/BFileManager.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/utils/PathUtil.h"
#include "engines/wintermute/base/file/BDiskFile.h"
#include "engines/wintermute/base/file/BSaveThumbFile.h"
#include "engines/wintermute/base/BFileEntry.h"
#include "engines/wintermute/base/file/BPkgFile.h"
#include "engines/wintermute/base/BResources.h"
#include "engines/wintermute/base/BPackage.h"
#include "engines/wintermute/base/BRegistry.h"
#include "engines/wintermute/base/BGame.h"
#include "engines/wintermute/dcpackage.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/wintermute.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/savefile.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CBFileManager::CBFileManager(CBGame *inGame): CBBase(inGame) {
	_basePath = NULL;

	initPaths();
	registerPackages();
}


//////////////////////////////////////////////////////////////////////
CBFileManager::~CBFileManager() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::cleanup() {
	// delete registered paths
	for (int i = 0; i < _singlePaths.size(); i++)
		delete [] _singlePaths[i];
	_singlePaths.clear();

	for (int i = 0; i < _packagePaths.size(); i++)
		delete [] _packagePaths[i];
	_packagePaths.clear();


	// delete file entries
	_filesIter = _files.begin();
	while (_filesIter != _files.end()) {
		delete _filesIter->_value;
		_filesIter++;
	}
	_files.clear();

	// close open files
	for (int i = 0; i < _openFiles.size(); i++) {
		delete _openFiles[i];
	}
	_openFiles.clear();


	// delete packages
	for (int i = 0; i < _packages.size(); i++)
		delete _packages[i];
	_packages.clear();

	delete[] _basePath;
	_basePath = NULL;

	return STATUS_OK;
}



#define MAX_FILE_SIZE 10000000
//////////////////////////////////////////////////////////////////////
byte *CBFileManager::readWholeFile(const Common::String &filename, uint32 *size, bool mustExist) {
	byte *buffer = NULL;

	Common::SeekableReadStream *file = openFile(filename);
	if (!file) {
		if (mustExist) _gameRef->LOG(0, "Error opening file '%s'", filename.c_str());
		return NULL;
	}

	/*
	if (File->GetSize() > MAX_FILE_SIZE) {
	    _gameRef->LOG(0, "File '%s' exceeds the maximum size limit (%d bytes)", Filename, MAX_FILE_SIZE);
	    CloseFile(File);
	    return NULL;
	}
	*/


	buffer = new byte[file->size() + 1];
	if (buffer == NULL) {
		_gameRef->LOG(0, "Error allocating buffer for file '%s' (%d bytes)", filename.c_str(), file->size() + 1);
		closeFile(file);
		return NULL;
	}

	if (file->read(buffer, (uint32)file->size()) != (uint32)file->size()) {
		_gameRef->LOG(0, "Error reading file '%s'", filename.c_str());
		closeFile(file);
		delete [] buffer;
		return NULL;
	};

	buffer[file->size()] = '\0';
	if (size != NULL) *size = file->size();
	closeFile(file);

	return buffer;
}

Common::SeekableReadStream *CBFileManager::loadSaveGame(const Common::String &filename) {
	Common::SaveFileManager *saveMan = g_wintermute->getSaveFileMan();
	Common::InSaveFile *file = saveMan->openForLoading(filename);
	return file;
}

//////////////////////////////////////////////////////////////////////////
bool CBFileManager::saveFile(const Common::String &filename, byte *buffer, uint32 bufferSize, bool compressed, byte *prefixBuffer, uint32 prefixSize) {
	// TODO
	warning("Implement SaveFile");

	Common::SaveFileManager *saveMan = g_wintermute->getSaveFileMan();
	Common::OutSaveFile *file = saveMan->openForSaving(filename);
	file->write(prefixBuffer, prefixSize);
	file->write(buffer, bufferSize);
	file->finalize();
	delete file;
#if 0
	RestoreCurrentDir();

	CBUtils::CreatePath(filename, false);

	FILE *f = fopen(filename, "wb");
	if (!f) {
		_gameRef->LOG(0, "Error opening file '%s' for writing.", filename);
		return STATUS_FAILED;
	}

	if (PrefixBuffer && PrefixSize) {
		fwrite(PrefixBuffer, PrefixSize, 1, f);
	}

	if (Compressed) {
		uint32 CompSize = BufferSize + (BufferSize / 100) + 12; // 1% extra space
		byte *CompBuffer = new byte[CompSize];
		if (!CompBuffer) {
			_gameRef->LOG(0, "Error allocating compression buffer while saving '%s'", filename);
			Compressed = false;
		} else {
			if (compress(CompBuffer, (uLongf *)&CompSize, Buffer, BufferSize) == Z_OK) {
				uint32 magic = DCGF_MAGIC;
				fwrite(&magic, sizeof(uint32), 1, f);
				magic = COMPRESSED_FILE_MAGIC;
				fwrite(&magic, sizeof(uint32), 1, f);

				uint32 DataOffset = 5 * sizeof(uint32);
				fwrite(&DataOffset, sizeof(uint32), 1, f);

				fwrite(&CompSize, sizeof(uint32), 1, f);
				fwrite(&BufferSize, sizeof(uint32), 1, f);

				fwrite(CompBuffer, CompSize, 1, f);
			} else {
				_gameRef->LOG(0, "Error compressing data while saving '%s'", filename);
				Compressed = false;
			}

			delete [] CompBuffer;
		}
	}

	if (!Compressed) fwrite(Buffer, BufferSize, 1, f);

	fclose(f);
#endif
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::requestCD(int cd, char *packageFile, const char *filename) {
	// unmount all non-local packages
	for (int i = 0; i < _packages.size(); i++) {
		if (_packages[i]->_cD > 0) _packages[i]->close();
	}


	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::addPath(TPathType type, const Common::String &path) {
	if (path.c_str() == NULL || strlen(path.c_str()) < 1) return STATUS_FAILED;

	bool slashed = (path[path.size() - 1] == '\\' || path[path.size() - 1] == '/');

	char *buffer = new char [strlen(path.c_str()) + 1 + (slashed ? 0 : 1)];
	if (buffer == NULL) return STATUS_FAILED;

	strcpy(buffer, path.c_str());
	if (!slashed) strcat(buffer, "\\");
	//CBPlatform::strlwr(buffer);

	switch (type) {
	case PATH_SINGLE:
		_singlePaths.push_back(buffer);
		break;
	case PATH_PACKAGE:
		_packagePaths.push_back(buffer);
		break;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBFileManager::reloadPaths() {
	// delete registered paths
	for (int i = 0; i < _singlePaths.size(); i++)
		delete [] _singlePaths[i];
	_singlePaths.clear();

	for (int i = 0; i < _packagePaths.size(); i++)
		delete [] _packagePaths[i];
	_packagePaths.clear();

	return initPaths();
}


#define TEMP_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
bool CBFileManager::initPaths() {
	restoreCurrentDir();

	AnsiString pathList;
	int numPaths;

	// single files paths
	pathList = _gameRef->_registry->readString("Resource", "CustomPaths", "");
	numPaths = CBUtils::strNumEntries(pathList.c_str(), ';');

	for (int i = 0; i < numPaths; i++) {
		char *path = CBUtils::strEntry(i, pathList.c_str(), ';');
		if (path && strlen(path) > 0) {
			addPath(PATH_SINGLE, path);
		}
		delete[] path;
		path = NULL;
	}
	addPath(PATH_SINGLE, ".\\");


	// package files paths
	addPath(PATH_PACKAGE, "./");

	pathList = _gameRef->_registry->readString("Resource", "PackagePaths", "");
	numPaths = CBUtils::strNumEntries(pathList.c_str(), ';');

	for (int i = 0; i < numPaths; i++) {
		char *path = CBUtils::strEntry(i, pathList.c_str(), ';');
		if (path && strlen(path) > 0) {
			addPath(PATH_PACKAGE, path);
		}
		delete[] path;
		path = NULL;
	}
	addPath(PATH_PACKAGE, "data");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::registerPackages() {
	restoreCurrentDir();

	_gameRef->LOG(0, "Scanning packages...");
	debugC(kWinterMuteDebugFileAccess, "Scanning packages");

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.dcp");

	for (Common::ArchiveMemberList::iterator it = files.begin(); it != files.end(); it++) {
		registerPackage((*it)->getName().c_str());
	}
#if 0
	AnsiString extension = AnsiString(PACKAGE_EXTENSION);

	for (int i = 0; i < _packagePaths.getSize(); i++) {
		boost::filesystem::path absPath = boost::filesystem::syste_complete(_packagePaths[i]);

		//_gameRef->LOG(0, "Scanning: %s", absPath.string().c_str());
		//printf("Scanning: %s\n", absPath.string().c_str());

		if (!exists(absPath)) continue;

		// scan files
		boost::filesystem::directory_iterator endIter;
		for (boost::filesystem::directory_iterator dit(absPath); dit != endIter; ++dit) {
			if (!is_directory((*dit).status())) {
				AnsiString fileName = (*dit).path().string();

				if (!IsValidPackage(fileName)) continue;

				warning("%s", fileName.c_str());
				//printf("%s\n", fileName.c_str());
				if (!StringUtil::CompareNoCase(extension, PathUtil::GetExtension(fileName))) continue;
				warning("Registered");
				RegisterPackage(absPath.string().c_str(), dit->path().filename().string().c_str());
			}
		}
	}
#endif
	debugC(kWinterMuteDebugFileAccess, "  Registered %d files in %d package(s)", _files.size(), _packages.size());
	_gameRef->LOG(0, "  Registered %d files in %d package(s)", _files.size(), _packages.size());

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBFileManager::registerPackage(const Common::String &filename , bool searchSignature) {
//	FILE *f = fopen(filename, "rb");
	Common::File *package = new Common::File();
	package->open(filename);
	if (!package->isOpen()) {
		_gameRef->LOG(0, "  Error opening package file '%s'. Ignoring.", filename.c_str());
		return STATUS_OK;
	}

	uint32 absoluteOffset = 0;
	bool boundToExe = false;

	if (searchSignature) {
		uint32 Offset;
		if (!findPackageSignature(package, &Offset)) {
			delete package;
			return STATUS_OK;
		} else {
			package->seek(Offset, SEEK_SET);
			absoluteOffset = Offset;
			boundToExe = true;
		}
	}

	TPackageHeader hdr;
	hdr.readFromStream(package);
//	package->read(&hdr, sizeof(TPackageHeader), 1, f);
	if (hdr.Magic1 != PACKAGE_MAGIC_1 || hdr.Magic2 != PACKAGE_MAGIC_2 || hdr.PackageVersion > PACKAGE_VERSION) {
		_gameRef->LOG(0, "  Invalid header in package file '%s'. Ignoring.", filename.c_str());
		delete package;
		return STATUS_OK;
	}

	if (hdr.PackageVersion != PACKAGE_VERSION) {
		_gameRef->LOG(0, "  Warning: package file '%s' is outdated.", filename.c_str());
	}

	// new in v2
	if (hdr.PackageVersion == PACKAGE_VERSION) {
		uint32 dirOffset;
		dirOffset = package->readUint32LE();
		dirOffset += absoluteOffset;
		package->seek(dirOffset, SEEK_SET);
	}

	for (uint32 i = 0; i < hdr.NumDirs; i++) {
		CBPackage *pkg = new CBPackage(_gameRef);
		if (!pkg) return STATUS_FAILED;

		pkg->_boundToExe = boundToExe;

		// read package info
		byte nameLength = package->readByte();
		pkg->_name = new char[nameLength];
		package->read(pkg->_name, nameLength);
		pkg->_cD = package->readByte();
		pkg->_priority = hdr.Priority;

		if (!hdr.MasterIndex) pkg->_cD = 0; // override CD to fixed disk
		_packages.push_back(pkg);


		// read file entries
		uint32 NumFiles = package->readUint32LE();

		for (uint32 j = 0; j < NumFiles; j++) {
			char *name;
			uint32 offset, length, compLength, flags, timeDate1, timeDate2;

			nameLength = package->readByte();
			name = new char[nameLength];
			package->read(name, nameLength);

			// v2 - xor name
			if (hdr.PackageVersion == PACKAGE_VERSION) {
				for (int k = 0; k < nameLength; k++) {
					((byte *)name)[k] ^= 'D';
				}
			}

			// some old version of ProjectMan writes invalid directory entries
			// so at least prevent strupr from corrupting memory
			name[nameLength - 1] = '\0';


			CBPlatform::strupr(name);

			offset = package->readUint32LE();
			offset += absoluteOffset;
			length = package->readUint32LE();
			compLength = package->readUint32LE();
			flags = package->readUint32LE();

			if (hdr.PackageVersion == PACKAGE_VERSION) {
				timeDate1 = package->readUint32LE();
				timeDate2 = package->readUint32LE();
			}
			_filesIter = _files.find(name);
			if (_filesIter == _files.end()) {
				CBFileEntry *file = new CBFileEntry(_gameRef);
				file->_package = pkg;
				file->_offset = offset;
				file->_length = length;
				file->_compressedLength = compLength;
				file->_flags = flags;

				_files[name] = file;
			} else {
				// current package has lower CD number or higher priority, than the registered
				if (pkg->_cD < _filesIter->_value->_package->_cD || pkg->_priority > _filesIter->_value->_package->_priority) {
					_filesIter->_value->_package = pkg;
					_filesIter->_value->_offset = offset;
					_filesIter->_value->_length = length;
					_filesIter->_value->_compressedLength = compLength;
					_filesIter->_value->_flags = flags;
				}
			}
			delete [] name;
		}
	}


	delete package;
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBFileManager::isValidPackage(const AnsiString &fileName) const {
	AnsiString plainName = PathUtil::getFileNameWithoutExtension(fileName);

	// check for device-type specific packages
	if (StringUtil::startsWith(plainName, "xdevice_", true)) {
		return StringUtil::compareNoCase(plainName, "xdevice_" + _gameRef->getDeviceType());
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
Common::File *CBFileManager::openPackage(const Common::String &name) {
	//TODO: Is it really necessary to do this when we have the ScummVM-system?

	//RestoreCurrentDir();

	Common::File *ret = new Common::File();
	char filename[MAX_PATH_LENGTH];

	for (int i = 0; i < _packagePaths.size(); i++) {
		sprintf(filename, "%s%s.%s", _packagePaths[i], name.c_str(), PACKAGE_EXTENSION);
		ret->open(filename);
		if (ret->isOpen()) {
			return ret;
		}
	}

	sprintf(filename, "%s.%s", name.c_str(), PACKAGE_EXTENSION);
	ret->open(filename);
	if (ret->isOpen()) {
		return ret;
	}
	warning("CBFileManager::OpenPackage - Couldn't load file %s", name.c_str());
	delete ret;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
Common::File *CBFileManager::openSingleFile(const Common::String &name) {
	restoreCurrentDir();

	Common::File *ret = NULL;
	char filename[MAX_PATH_LENGTH];

	for (int i = 0; i < _singlePaths.size(); i++) {
		sprintf(filename, "%s%s", _singlePaths[i], name.c_str());
		ret->open(filename);
		if (ret->isOpen())
			return ret;
	}

	// didn't find in search paths, try to open directly
	ret->open(name);
	if (ret->isOpen()) {
		return ret;
	} else {
		delete ret;
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::getFullPath(const Common::String &filename, char *fullname) {
	restoreCurrentDir();

	Common::File f;
	bool found = false;

	for (int i = 0; i < _singlePaths.size(); i++) {
		sprintf(fullname, "%s%s", _singlePaths[i], filename.c_str());
		f.open(fullname);
		if (f.isOpen()) {
			f.close();
			found = true;
			break;
		}
	}

	if (!found) {
		f.open(filename.c_str());
		if (f.isOpen()) {
			f.close();
			found = true;
			strcpy(fullname, filename.c_str());
		}
	}

	return found;
}


//////////////////////////////////////////////////////////////////////////
CBFileEntry *CBFileManager::getPackageEntry(const Common::String &filename) {
	char *upc_name = new char[strlen(filename.c_str()) + 1];
	strcpy(upc_name, filename.c_str());
	CBPlatform::strupr(upc_name);

	CBFileEntry *ret = NULL;
	_filesIter = _files.find(upc_name);
	if (_filesIter != _files.end()) ret = _filesIter->_value;

	delete [] upc_name;

	return ret;
}

bool CBFileManager::hasFile(const Common::String &filename) {
	//TODO: Do this in a much simpler fashion
	Common::SeekableReadStream *stream = openFile(filename, true, false);
	if (!stream) {
		return false;
	}
	delete stream;
	return true;
}

//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *CBFileManager::openFile(const Common::String &filename, bool absPathWarning, bool keepTrackOf) {
	if (strcmp(filename.c_str(), "") == 0) return NULL;
	//_gameRef->LOG(0, "open file: %s", filename);
	/*#ifdef __WIN32__
	    if (_gameRef->_debugDebugMode && _gameRef->_debugAbsolutePathWarning && AbsPathWarning) {
	        char Drive[_MAX_DRIVE];
	        _splitpath(filename, Drive, NULL, NULL, NULL);
	        if (Drive[0] != '\0') {
	            _gameRef->LOG(0, "WARNING: Referencing absolute path '%s'. The game will NOT work on another computer.", filename);
	        }
	    }
	#endif*/

	Common::SeekableReadStream *file = openFileRaw(filename);
	if (file && keepTrackOf) _openFiles.push_back(file);
	return file;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::closeFile(Common::SeekableReadStream *File) {
	for (int i = 0; i < _openFiles.size(); i++) {
		if (_openFiles[i] == File) {
			delete _openFiles[i];
			_openFiles.remove_at(i);
			return STATUS_OK;
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *CBFileManager::openFileRaw(const Common::String &filename) {
	restoreCurrentDir();

	Common::SeekableReadStream *ret = NULL;

	if (scumm_strnicmp(filename.c_str(), "savegame:", 9) == 0) {
		CBSaveThumbFile *SaveThumbFile = new CBSaveThumbFile(_gameRef);
		if (DID_SUCCEED(SaveThumbFile->open(filename))) {
			ret = SaveThumbFile->getMemStream();
		} 
		delete SaveThumbFile;
		return ret;
	}



	ret = openDiskFile(filename, this);
	if (ret) return ret;

	ret = openPkgFile(filename, this);
	if (ret) return ret;

	ret = CBResources::getFile(filename);
	if (ret) return ret;

	warning("BFileManager::OpenFileRaw - Failed to open %s", filename.c_str());
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::restoreCurrentDir() {
	if (!_basePath) return STATUS_OK;
	else {
		/*if (!chdir(_basePath)) return STATUS_OK;
		else return STATUS_FAILED;*/
		warning("CBFileManager::RestoreCurrentDir - ignored");
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::setBasePath(const Common::String &path) {
	cleanup();

	if (path.c_str()) {
		_basePath = new char[path.size() + 1];
		strcpy(_basePath, path.c_str());
	}

	initPaths();
	registerPackages();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::findPackageSignature(Common::File *f, uint32 *offset) {
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
