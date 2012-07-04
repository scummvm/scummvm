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
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/utils/PathUtil.h"
#include "engines/wintermute/Base/file/BDiskFile.h"
#include "engines/wintermute/Base/file/BSaveThumbFile.h"
#include "engines/wintermute/Base/BFileEntry.h"
#include "engines/wintermute/Base/file/BPkgFile.h"
#include "engines/wintermute/Base/BResources.h"
#include "engines/wintermute/Base/BPackage.h"
#include "engines/wintermute/Base/BRegistry.h"
#include "engines/wintermute/Base/BGame.h"
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
HRESULT CBFileManager::cleanup() {
	int i;

	// delete registered paths
	for (i = 0; i < _singlePaths.GetSize(); i++)
		delete [] _singlePaths[i];
	_singlePaths.RemoveAll();

	for (i = 0; i < _packagePaths.GetSize(); i++)
		delete [] _packagePaths[i];
	_packagePaths.RemoveAll();


	// delete file entries
	_filesIter = _files.begin();
	while (_filesIter != _files.end()) {
		delete _filesIter->_value;
		_filesIter++;
	}
	_files.clear();

	// close open files
	for (i = 0; i < _openFiles.GetSize(); i++) {
		delete _openFiles[i];
	}
	_openFiles.RemoveAll();


	// delete packages
	for (i = 0; i < _packages.GetSize(); i++)
		delete _packages[i];
	_packages.RemoveAll();

	delete[] _basePath;
	_basePath = NULL;

	return S_OK;
}



#define MAX_FILE_SIZE 10000000
//////////////////////////////////////////////////////////////////////
byte *CBFileManager::readWholeFile(const Common::String &filename, uint32 *Size, bool MustExist) {

	byte *buffer = NULL;

	Common::SeekableReadStream *File = openFile(filename);
	if (!File) {
		if (MustExist) Game->LOG(0, "Error opening file '%s'", filename.c_str());
		return NULL;
	}

	/*
	if (File->GetSize() > MAX_FILE_SIZE) {
	    Game->LOG(0, "File '%s' exceeds the maximum size limit (%d bytes)", Filename, MAX_FILE_SIZE);
	    CloseFile(File);
	    return NULL;
	}
	*/


	buffer = new byte[File->size() + 1];
	if (buffer == NULL) {
		Game->LOG(0, "Error allocating buffer for file '%s' (%d bytes)", filename.c_str(), File->size() + 1);
		closeFile(File);
		return NULL;
	}

	if (File->read(buffer, File->size()) != File->size()) {
		Game->LOG(0, "Error reading file '%s'", filename.c_str());
		closeFile(File);
		delete [] buffer;
		return NULL;
	};

	buffer[File->size()] = '\0';
	if (Size != NULL) *Size = File->size();
	closeFile(File);

	return buffer;
}

Common::SeekableReadStream *CBFileManager::loadSaveGame(const Common::String &filename) {
	Common::SaveFileManager *saveMan = g_wintermute->getSaveFileMan();
	Common::InSaveFile *file = saveMan->openForLoading(filename);
	return file;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::saveFile(const Common::String &filename, byte *Buffer, uint32 BufferSize, bool Compressed, byte *PrefixBuffer, uint32 PrefixSize) {
	// TODO
	warning("Implement SaveFile");

	Common::SaveFileManager *saveMan = g_wintermute->getSaveFileMan();
	Common::OutSaveFile *file = saveMan->openForSaving(filename);
	file->write(PrefixBuffer, PrefixSize);
	file->write(Buffer, BufferSize);
	file->finalize();
	delete file;
#if 0
	RestoreCurrentDir();

	CBUtils::CreatePath(filename, false);

	FILE *f = fopen(filename, "wb");
	if (!f) {
		Game->LOG(0, "Error opening file '%s' for writing.", filename);
		return E_FAIL;
	}

	if (PrefixBuffer && PrefixSize) {
		fwrite(PrefixBuffer, PrefixSize, 1, f);
	}

	if (Compressed) {
		uint32 CompSize = BufferSize + (BufferSize / 100) + 12; // 1% extra space
		byte *CompBuffer = new byte[CompSize];
		if (!CompBuffer) {
			Game->LOG(0, "Error allocating compression buffer while saving '%s'", filename);
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
				Game->LOG(0, "Error compressing data while saving '%s'", filename);
				Compressed = false;
			}

			delete [] CompBuffer;
		}
	}

	if (!Compressed) fwrite(Buffer, BufferSize, 1, f);

	fclose(f);
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::requestCD(int CD, char *PackageFile, char *Filename) {
	// unmount all non-local packages
	for (int i = 0; i < _packages.GetSize(); i++) {
		if (_packages[i]->_cD > 0) _packages[i]->close();
	}


	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::addPath(TPathType Type, const Common::String &Path) {
	if (Path.c_str() == NULL || strlen(Path.c_str()) < 1) return E_FAIL;

	bool slashed = (Path[Path.size() - 1] == '\\' || Path[Path.size() - 1] == '/');

	char *buffer = new char [strlen(Path.c_str()) + 1 + (slashed ? 0 : 1)];
	if (buffer == NULL) return E_FAIL;

	strcpy(buffer, Path.c_str());
	if (!slashed) strcat(buffer, "\\");
	//CBPlatform::strlwr(buffer);

	switch (Type) {
	case PATH_SINGLE:
		_singlePaths.Add(buffer);
		break;
	case PATH_PACKAGE:
		_packagePaths.Add(buffer);
		break;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::reloadPaths() {
	// delete registered paths
	for (int i = 0; i < _singlePaths.GetSize(); i++)
		delete [] _singlePaths[i];
	_singlePaths.RemoveAll();

	for (int i = 0; i < _packagePaths.GetSize(); i++)
		delete [] _packagePaths[i];
	_packagePaths.RemoveAll();

	return initPaths();
}


#define TEMP_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::initPaths() {
	restoreCurrentDir();

	AnsiString pathList;
	int numPaths;

	// single files paths
	pathList = Game->_registry->ReadString("Resource", "CustomPaths", "");
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

	/*#ifdef __APPLE__
	    // search .app path and Resources dir in the bundle
	    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
	    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());

	#ifdef __IPHONE__
	    AddPath(PATH_PACKAGE, pathPtr);
	    AddPath(PATH_SINGLE, pathPtr);
	#else
	    char bundlePath[MAX_PATH];

	    sprintf(bundlePath, "%s/../", pathPtr);
	    AddPath(PATH_PACKAGE, bundlePath);
	    AddPath(PATH_SINGLE, bundlePath);

	    sprintf(bundlePath, "%s/Contents/Resources/", pathPtr);
	    AddPath(PATH_PACKAGE, bundlePath);
	    AddPath(PATH_SINGLE, bundlePath);


	    CFRelease(appUrlRef);
	    CFRelease(macPath);
	#endif
	#endif*/


	pathList = Game->_registry->ReadString("Resource", "PackagePaths", "");
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

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::registerPackages() {
	restoreCurrentDir();

	Game->LOG(0, "Scanning packages...");
	warning("Scanning packages");

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.dcp");

	for (Common::ArchiveMemberList::iterator it = files.begin(); it != files.end(); it++) {
		registerPackage((*it)->getName().c_str());
	}
#if 0
	AnsiString extension = AnsiString(PACKAGE_EXTENSION);

	for (int i = 0; i < _packagePaths.GetSize(); i++) {
		boost::filesystem::path absPath = boost::filesystem::syste_complete(_packagePaths[i]);

		//Game->LOG(0, "Scanning: %s", absPath.string().c_str());
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
	warning("  Registered %d files in %d package(s)", _files.size(), _packages.GetSize());
	Game->LOG(0, "  Registered %d files in %d package(s)", _files.size(), _packages.GetSize());

	warning("  Registered %d files in %d package(s)", _files.size(), _packages.GetSize());
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::registerPackage(const Common::String &filename , bool searchSignature) {
//	FILE *f = fopen(filename, "rb");
	Common::File *package = new Common::File();
	package->open(filename);
	if (!package->isOpen()) {
		Game->LOG(0, "  Error opening package file '%s'. Ignoring.", filename.c_str());
		return S_OK;
	}

	uint32 AbsoluteOffset = 0;
	bool BoundToExe = false;

	if (searchSignature) {
		uint32 Offset;
		if (!findPackageSignature(package, &Offset)) {
			delete package;
			return S_OK;
		} else {
			package->seek(Offset, SEEK_SET);
			AbsoluteOffset = Offset;
			BoundToExe = true;
		}
	}

	TPackageHeader hdr;
	hdr.readFromStream(package);
//	package->read(&hdr, sizeof(TPackageHeader), 1, f);
	if (hdr.Magic1 != PACKAGE_MAGIC_1 || hdr.Magic2 != PACKAGE_MAGIC_2 || hdr.PackageVersion > PACKAGE_VERSION) {
		Game->LOG(0, "  Invalid header in package file '%s'. Ignoring.", filename.c_str());
		delete package;
		return S_OK;
	}

	if (hdr.PackageVersion != PACKAGE_VERSION) {
		Game->LOG(0, "  Warning: package file '%s' is outdated.", filename.c_str());
	}

	// new in v2
	if (hdr.PackageVersion == PACKAGE_VERSION) {
		uint32 DirOffset;
		DirOffset = package->readUint32LE();
		DirOffset += AbsoluteOffset;
		package->seek(DirOffset, SEEK_SET);
	}

	for (uint32 i = 0; i < hdr.NumDirs; i++) {
		CBPackage *pkg = new CBPackage(Game);
		if (!pkg) return E_FAIL;

		pkg->_boundToExe = BoundToExe;

		// read package info
		byte nameLength = package->readByte();
		pkg->_name = new char[nameLength];
		package->read(pkg->_name, nameLength);
		pkg->_cD = package->readByte();
		pkg->_priority = hdr.Priority;

		if (!hdr.MasterIndex) pkg->_cD = 0; // override CD to fixed disk
		_packages.Add(pkg);


		// read file entries
		uint32 NumFiles = package->readUint32LE();

		for (uint32 j = 0; j < NumFiles; j++) {
			char *name;
			uint32 Offset, Length, CompLength, Flags, TimeDate1, TimeDate2;

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

			Offset = package->readUint32LE();
			Offset += AbsoluteOffset;
			Length = package->readUint32LE();
			CompLength = package->readUint32LE();
			Flags = package->readUint32LE();

			if (hdr.PackageVersion == PACKAGE_VERSION) {
				TimeDate1 = package->readUint32LE();
				TimeDate2 = package->readUint32LE();
			}
			_filesIter = _files.find(name);
			if (_filesIter == _files.end()) {
				CBFileEntry *file = new CBFileEntry(Game);
				file->_package = pkg;
				file->_offset = Offset;
				file->_length = Length;
				file->_compressedLength = CompLength;
				file->_flags = Flags;

				_files[name] = file;
			} else {
				// current package has lower CD number or higher priority, than the registered
				if (pkg->_cD < _filesIter->_value->_package->_cD || pkg->_priority > _filesIter->_value->_package->_priority) {
					_filesIter->_value->_package = pkg;
					_filesIter->_value->_offset = Offset;
					_filesIter->_value->_length = Length;
					_filesIter->_value->_compressedLength = CompLength;
					_filesIter->_value->_flags = Flags;
				}
			}
			delete [] name;
		}
	}


	delete package;
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::registerPackage(const char *Path, const char *name, bool SearchSignature) {
// TODO
	error("Implement RegisterPackage, this is the old one");
#if 0
	char Filename[MAX_PATH];
	sprintf(filename, "%s%s", Path, name);

	FILE *f = fopen(filename, "rb");
	if (!f) {
		Game->LOG(0, "  Error opening package file '%s'. Ignoring.", filename);
		return S_OK;
	}

	uint32 AbsoluteOffset = 0;
	bool BoundToExe = false;

	if (SearchSignature) {
		uint32 Offset;
		if (!FindPackageSignature(f, &Offset)) {
			fclose(f);
			return S_OK;
		} else {
			fseek(f, Offset, SEEK_SET);
			AbsoluteOffset = Offset;
			BoundToExe = true;
		}
	}

	TPackageHeader hdr;
	fread(&hdr, sizeof(TPackageHeader), 1, f);
	if (hdr.Magic1 != PACKAGE_MAGIC_1 || hdr.Magic2 != PACKAGE_MAGIC_2 || hdr.PackageVersion > PACKAGE_VERSION) {
		Game->LOG(0, "  Invalid header in package file '%s'. Ignoring.", filename);
		fclose(f);
		return S_OK;
	}

	if (hdr.PackageVersion != PACKAGE_VERSION) {
		Game->LOG(0, "  Warning: package file '%s' is outdated.", filename);
	}

	// new in v2
	if (hdr.PackageVersion == PACKAGE_VERSION) {
		uint32 DirOffset;
		fread(&DirOffset, sizeof(uint32), 1, f);
		DirOffset += AbsoluteOffset;
		fseek(f, DirOffset, SEEK_SET);
	}

	for (int i = 0; i < hdr.NumDirs; i++) {
		CBPackage *pkg = new CBPackage(Game);
		if (!pkg) return E_FAIL;

		pkg->_boundToExe = BoundToExe;

		// read package info
		byte nameLength;
		fread(&nameLength, sizeof(byte), 1, f);
		pkg->_name = new char[nameLength];
		fread(pkg->_name, nameLength, 1, f);
		fread(&pkg->_cD, sizeof(byte), 1, f);
		pkg->_priority = hdr.Priority;

		if (!hdr.MasterIndex) pkg->_cD = 0; // override CD to fixed disk
		_packages.Add(pkg);


		// read file entries
		uint32 NumFiles;
		fread(&NumFiles, sizeof(uint32), 1, f);

		for (int j = 0; j < NumFiles; j++) {
			char *name;
			uint32 Offset, Length, CompLength, Flags, TimeDate1, TimeDate2;

			fread(&nameLength, sizeof(byte), 1, f);
			name = new char[nameLength];
			fread(name, nameLength, 1, f);

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

			fread(&Offset, sizeof(uint32), 1, f);
			Offset += AbsoluteOffset;
			fread(&Length, sizeof(uint32), 1, f);
			fread(&CompLength, sizeof(uint32), 1, f);
			fread(&Flags, sizeof(uint32), 1, f);

			if (hdr.PackageVersion == PACKAGE_VERSION) {
				fread(&TimeDate1, sizeof(uint32), 1, f);
				fread(&TimeDate2, sizeof(uint32), 1, f);
			}
			_filesIter = _files.find(name);
			if (_filesIter == _files.end()) {
				CBFileEntry *file = new CBFileEntry(Game);
				file->_package = pkg;
				file->_offset = Offset;
				file->_length = Length;
				file->_compressedLength = CompLength;
				file->_flags = Flags;

				_files[name] = file;
			} else {
				// current package has lower CD number or higher priority, than the registered
				if (pkg->_cD < _filesIter->_value->_package->_cD || pkg->_priority > _filesIter->_value->_package->_priority) {
					_filesIter->_value->_package = pkg;
					_filesIter->_value->_offset = Offset;
					_filesIter->_value->_length = Length;
					_filesIter->_value->_compressedLength = CompLength;
					_filesIter->_value->_flags = Flags;
				}
			}
			delete [] name;
		}
	}


	fclose(f);
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBFileManager::isValidPackage(const AnsiString &fileName) const {
	AnsiString plainName = PathUtil::getFileNameWithoutExtension(fileName);

	// check for device-type specific packages
	if (StringUtil::startsWith(plainName, "xdevice_", true)) {
		return StringUtil::compareNoCase(plainName, "xdevice_" + Game->GetDeviceType());
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
Common::File *CBFileManager::openPackage(const Common::String &name) {
	//TODO: Is it really necessary to do this when we have the ScummVM-system?

	//RestoreCurrentDir();

	Common::File *ret = new Common::File();
	char filename[MAX_PATH];

	for (int i = 0; i < _packagePaths.GetSize(); i++) {
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
	char filename[MAX_PATH];

	for (int i = 0; i < _singlePaths.GetSize(); i++) {
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
bool CBFileManager::getFullPath(const Common::String &filename, char *Fullname) {
	restoreCurrentDir();

	Common::File f;
	bool found = false;

	for (int i = 0; i < _singlePaths.GetSize(); i++) {
		sprintf(Fullname, "%s%s", _singlePaths[i], filename.c_str());
		f.open(Fullname);
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
			strcpy(Fullname, filename.c_str());
		}
	}

	return found;
}


//////////////////////////////////////////////////////////////////////////
CBFileEntry *CBFileManager::getPackageEntry(const Common::String &Filename) {
	char *upc_name = new char[strlen(Filename.c_str()) + 1];
	strcpy(upc_name, Filename.c_str());
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
Common::SeekableReadStream *CBFileManager::openFile(const Common::String &filename, bool AbsPathWarning, bool keepTrackOf) {
	if (strcmp(filename.c_str(), "") == 0) return NULL;
	//Game->LOG(0, "open file: %s", filename);
	/*#ifdef __WIN32__
	    if (Game->_dEBUG_DebugMode && Game->_dEBUG_AbsolutePathWarning && AbsPathWarning) {
	        char Drive[_MAX_DRIVE];
	        _splitpath(filename, Drive, NULL, NULL, NULL);
	        if (Drive[0] != '\0') {
	            Game->LOG(0, "WARNING: Referencing absolute path '%s'. The game will NOT work on another computer.", filename);
	        }
	    }
	#endif*/

	Common::SeekableReadStream *File = openFileRaw(filename);
	if (File && keepTrackOf) _openFiles.Add(File);
	return File;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::closeFile(Common::SeekableReadStream *File) {
	for (int i = 0; i < _openFiles.GetSize(); i++) {
		if (_openFiles[i] == File) {
			delete _openFiles[i];
			_openFiles.RemoveAt(i);
			return S_OK;
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *CBFileManager::openFileRaw(const Common::String &filename) {
	restoreCurrentDir();

	Common::SeekableReadStream *ret = NULL;

	if (scumm_strnicmp(filename.c_str(), "savegame:", 9) == 0) {
		CBSaveThumbFile *SaveThumbFile = new CBSaveThumbFile(Game);
		if (SUCCEEDED(SaveThumbFile->open(filename))) {
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
HRESULT CBFileManager::restoreCurrentDir() {
	if (!_basePath) return S_OK;
	else {
		/*if (!chdir(_basePath)) return S_OK;
		else return E_FAIL;*/
		warning("CBFileManager::RestoreCurrentDir - ignored");
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::setBasePath(const Common::String &Path) {
	cleanup();

	if (Path.c_str()) {
		_basePath = new char[Path.size() + 1];
		strcpy(_basePath, Path.c_str());
	}

	initPaths();
	registerPackages();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::findPackageSignature(Common::File *f, uint32 *Offset) {
	byte buf[32768];

	byte Signature[8];
	((uint32 *)Signature)[0] = PACKAGE_MAGIC_1;
	((uint32 *)Signature)[1] = PACKAGE_MAGIC_2;

	uint32 FileSize = f->size();

	int StartPos = 1024 * 1024;

	uint32 BytesRead = StartPos;

	while (BytesRead < FileSize - 16) {
		int ToRead = MIN((unsigned int)32768, FileSize - BytesRead);
		f->seek(StartPos, SEEK_SET);
		int ActuallyRead = f->read(buf, ToRead);
		if (ActuallyRead != ToRead) return false;

		for (int i = 0; i < ToRead - 8; i++)
			if (!memcmp(buf + i, Signature, 8)) {
				*Offset =  StartPos + i;
				return true;
			}

		BytesRead = BytesRead + ToRead - 16;
		StartPos = StartPos + ToRead - 16;

	}
	return false;

}

} // end of namespace WinterMute
