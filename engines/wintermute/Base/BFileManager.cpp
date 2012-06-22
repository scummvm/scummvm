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


#define FORBIDDEN_SYMBOL_ALLOW_ALL
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

	InitPaths();
	RegisterPackages();
}


//////////////////////////////////////////////////////////////////////
CBFileManager::~CBFileManager() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::Cleanup() {
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
byte *CBFileManager::ReadWholeFile(const Common::String &Filename, uint32 *Size, bool MustExist) {

	byte *buffer = NULL;

	Common::SeekableReadStream *File = OpenFile(Filename);
	if (!File) {
		if (MustExist) Game->LOG(0, "Error opening file '%s'", Filename.c_str());
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
		Game->LOG(0, "Error allocating buffer for file '%s' (%d bytes)", Filename.c_str(), File->size() + 1);
		CloseFile(File);
		return NULL;
	}

	if (File->read(buffer, File->size()) != File->size()) {
		Game->LOG(0, "Error reading file '%s'", Filename.c_str());
		CloseFile(File);
		delete [] buffer;
		return NULL;
	};

	buffer[File->size()] = '\0';
	if (Size != NULL) *Size = File->size();
	CloseFile(File);

	return buffer;
}

Common::SeekableReadStream *CBFileManager::loadSaveGame(const Common::String &filename) {
	Common::SaveFileManager *saveMan = g_wintermute->getSaveFileMan();
	Common::InSaveFile *file = saveMan->openForLoading(filename);
	return file;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::SaveFile(const Common::String &Filename, byte *Buffer, uint32 BufferSize, bool Compressed, byte *PrefixBuffer, uint32 PrefixSize) {
	// TODO
	warning("Implement SaveFile");

	Common::SaveFileManager *saveMan = g_wintermute->getSaveFileMan();
	Common::OutSaveFile *file = saveMan->openForSaving(Filename);
	file->write(PrefixBuffer, PrefixSize);
	file->write(Buffer, BufferSize);
	file->finalize();
	delete file;
#if 0
	RestoreCurrentDir();

	CBUtils::CreatePath(Filename, false);

	FILE *f = fopen(Filename, "wb");
	if (!f) {
		Game->LOG(0, "Error opening file '%s' for writing.", Filename);
		return E_FAIL;
	}

	if (PrefixBuffer && PrefixSize) {
		fwrite(PrefixBuffer, PrefixSize, 1, f);
	}

	if (Compressed) {
		uint32 CompSize = BufferSize + (BufferSize / 100) + 12; // 1% extra space
		byte *CompBuffer = new byte[CompSize];
		if (!CompBuffer) {
			Game->LOG(0, "Error allocating compression buffer while saving '%s'", Filename);
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
				Game->LOG(0, "Error compressing data while saving '%s'", Filename);
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
HRESULT CBFileManager::RequestCD(int CD, char *PackageFile, char *Filename) {
	// unmount all non-local packages
	for (int i = 0; i < _packages.GetSize(); i++) {
		if (_packages[i]->_cD > 0) _packages[i]->Close();
	}


	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::AddPath(TPathType Type, const Common::String &Path) {
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
HRESULT CBFileManager::ReloadPaths() {
	// delete registered paths
	for (int i = 0; i < _singlePaths.GetSize(); i++)
		delete [] _singlePaths[i];
	_singlePaths.RemoveAll();

	for (int i = 0; i < _packagePaths.GetSize(); i++)
		delete [] _packagePaths[i];
	_packagePaths.RemoveAll();

	return InitPaths();
}


#define TEMP_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::InitPaths() {
	RestoreCurrentDir();

	AnsiString pathList;
	int numPaths;

	// single files paths
	pathList = Game->_registry->ReadString("Resource", "CustomPaths", "");
	numPaths = CBUtils::StrNumEntries(pathList.c_str(), ';');

	for (int i = 0; i < numPaths; i++) {
		char *path = CBUtils::StrEntry(i, pathList.c_str(), ';');
		if (path && strlen(path) > 0) {
			AddPath(PATH_SINGLE, path);
		}
		delete[] path;
		path = NULL;
	}
	AddPath(PATH_SINGLE, ".\\");


	// package files paths
	AddPath(PATH_PACKAGE, "./");

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
	numPaths = CBUtils::StrNumEntries(pathList.c_str(), ';');

	for (int i = 0; i < numPaths; i++) {
		char *path = CBUtils::StrEntry(i, pathList.c_str(), ';');
		if (path && strlen(path) > 0) {
			AddPath(PATH_PACKAGE, path);
		}
		delete[] path;
		path = NULL;
	}
	AddPath(PATH_PACKAGE, "data");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::RegisterPackages() {
	RestoreCurrentDir();

	Game->LOG(0, "Scanning packages...");
	warning("Scanning packages");

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.dcp");

	for (Common::ArchiveMemberList::iterator it = files.begin(); it != files.end(); it++) {
		RegisterPackage((*it)->getName().c_str());
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
HRESULT CBFileManager::RegisterPackage(const Common::String &Filename , bool SearchSignature) {
//	FILE *f = fopen(Filename, "rb");
	Common::File *package = new Common::File();
	package->open(Filename);
	if (!package->isOpen()) {
		Game->LOG(0, "  Error opening package file '%s'. Ignoring.", Filename.c_str());
		return S_OK;
	}

	uint32 AbsoluteOffset = 0;
	bool BoundToExe = false;

	if (SearchSignature) {
		uint32 Offset;
		if (!FindPackageSignature(package, &Offset)) {
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
		Game->LOG(0, "  Invalid header in package file '%s'. Ignoring.", Filename.c_str());
		delete package;
		return S_OK;
	}

	if (hdr.PackageVersion != PACKAGE_VERSION) {
		Game->LOG(0, "  Warning: package file '%s' is outdated.", Filename.c_str());
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
		byte NameLength = package->readByte();
		pkg->_name = new char[NameLength];
		package->read(pkg->_name, NameLength);
		pkg->_cD = package->readByte();
		pkg->_priority = hdr.Priority;

		if (!hdr.MasterIndex) pkg->_cD = 0; // override CD to fixed disk
		_packages.Add(pkg);


		// read file entries
		uint32 NumFiles = package->readUint32LE();

		for (uint32 j = 0; j < NumFiles; j++) {
			char *Name;
			uint32 Offset, Length, CompLength, Flags, TimeDate1, TimeDate2;

			NameLength = package->readByte();
			Name = new char[NameLength];
			package->read(Name, NameLength);

			// v2 - xor name
			if (hdr.PackageVersion == PACKAGE_VERSION) {
				for (int k = 0; k < NameLength; k++) {
					((byte *)Name)[k] ^= 'D';
				}
			}

			// some old version of ProjectMan writes invalid directory entries
			// so at least prevent strupr from corrupting memory
			Name[NameLength - 1] = '\0';


			CBPlatform::strupr(Name);

			Offset = package->readUint32LE();
			Offset += AbsoluteOffset;
			Length = package->readUint32LE();
			CompLength = package->readUint32LE();
			Flags = package->readUint32LE();

			if (hdr.PackageVersion == PACKAGE_VERSION) {
				TimeDate1 = package->readUint32LE();
				TimeDate2 = package->readUint32LE();
			}
			_filesIter = _files.find(Name);
			if (_filesIter == _files.end()) {
				CBFileEntry *file = new CBFileEntry(Game);
				file->_package = pkg;
				file->_offset = Offset;
				file->_length = Length;
				file->_compressedLength = CompLength;
				file->_flags = Flags;

				_files[Name] = file;
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
			delete [] Name;
		}
	}


	delete package;
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::RegisterPackage(const char *Path, const char *Name, bool SearchSignature) {
// TODO
	error("Implement RegisterPackage, this is the old one");
#if 0
	char Filename[MAX_PATH];
	sprintf(Filename, "%s%s", Path, Name);

	FILE *f = fopen(Filename, "rb");
	if (!f) {
		Game->LOG(0, "  Error opening package file '%s'. Ignoring.", Filename);
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
		Game->LOG(0, "  Invalid header in package file '%s'. Ignoring.", Filename);
		fclose(f);
		return S_OK;
	}

	if (hdr.PackageVersion != PACKAGE_VERSION) {
		Game->LOG(0, "  Warning: package file '%s' is outdated.", Filename);
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
		byte NameLength;
		fread(&NameLength, sizeof(byte), 1, f);
		pkg->_name = new char[NameLength];
		fread(pkg->_name, NameLength, 1, f);
		fread(&pkg->_cD, sizeof(byte), 1, f);
		pkg->_priority = hdr.Priority;

		if (!hdr.MasterIndex) pkg->_cD = 0; // override CD to fixed disk
		_packages.Add(pkg);


		// read file entries
		uint32 NumFiles;
		fread(&NumFiles, sizeof(uint32), 1, f);

		for (int j = 0; j < NumFiles; j++) {
			char *Name;
			uint32 Offset, Length, CompLength, Flags, TimeDate1, TimeDate2;

			fread(&NameLength, sizeof(byte), 1, f);
			Name = new char[NameLength];
			fread(Name, NameLength, 1, f);

			// v2 - xor name
			if (hdr.PackageVersion == PACKAGE_VERSION) {
				for (int k = 0; k < NameLength; k++) {
					((byte *)Name)[k] ^= 'D';
				}
			}

			// some old version of ProjectMan writes invalid directory entries
			// so at least prevent strupr from corrupting memory
			Name[NameLength - 1] = '\0';


			CBPlatform::strupr(Name);

			fread(&Offset, sizeof(uint32), 1, f);
			Offset += AbsoluteOffset;
			fread(&Length, sizeof(uint32), 1, f);
			fread(&CompLength, sizeof(uint32), 1, f);
			fread(&Flags, sizeof(uint32), 1, f);

			if (hdr.PackageVersion == PACKAGE_VERSION) {
				fread(&TimeDate1, sizeof(uint32), 1, f);
				fread(&TimeDate2, sizeof(uint32), 1, f);
			}
			_filesIter = _files.find(Name);
			if (_filesIter == _files.end()) {
				CBFileEntry *file = new CBFileEntry(Game);
				file->_package = pkg;
				file->_offset = Offset;
				file->_length = Length;
				file->_compressedLength = CompLength;
				file->_flags = Flags;

				_files[Name] = file;
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
			delete [] Name;
		}
	}


	fclose(f);
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBFileManager::IsValidPackage(const AnsiString &fileName) const {
	AnsiString plainName = PathUtil::GetFileNameWithoutExtension(fileName);

	// check for device-type specific packages
	if (StringUtil::StartsWith(plainName, "xdevice_", true)) {
		return StringUtil::CompareNoCase(plainName, "xdevice_" + Game->GetDeviceType());
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
Common::File *CBFileManager::OpenPackage(const Common::String &Name) {
	//TODO: Is it really necessary to do this when we have the ScummVM-system?

	//RestoreCurrentDir();

	Common::File *ret = new Common::File();
	char Filename[MAX_PATH];

	for (int i = 0; i < _packagePaths.GetSize(); i++) {
		sprintf(Filename, "%s%s.%s", _packagePaths[i], Name.c_str(), PACKAGE_EXTENSION);
		ret->open(Filename);
		if (ret->isOpen()) {
			return ret;
		}
	}

	sprintf(Filename, "%s.%s", Name.c_str(), PACKAGE_EXTENSION);
	ret->open(Filename);
	if (ret->isOpen()) {
		return ret;
	}
	warning("CBFileManager::OpenPackage - Couldn't load file %s", Name.c_str());
	delete ret;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
Common::File *CBFileManager::OpenSingleFile(const Common::String &Name) {
	RestoreCurrentDir();

	Common::File *ret = NULL;
	char Filename[MAX_PATH];

	for (int i = 0; i < _singlePaths.GetSize(); i++) {
		sprintf(Filename, "%s%s", _singlePaths[i], Name.c_str());
		ret->open(Filename);
		if (ret->isOpen())
			return ret;
	}

	// didn't find in search paths, try to open directly
	ret->open(Name);
	if (ret->isOpen()) {
		return ret;
	} else {
		delete ret;
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::GetFullPath(const Common::String &filename, char *Fullname) {
	RestoreCurrentDir();

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
CBFileEntry *CBFileManager::GetPackageEntry(const Common::String &Filename) {
	char *upc_name = new char[strlen(Filename.c_str()) + 1];
	strcpy(upc_name, Filename.c_str());
	CBPlatform::strupr(upc_name);

	CBFileEntry *ret = NULL;
	_filesIter = _files.find(upc_name);
	if (_filesIter != _files.end()) ret = _filesIter->_value;

	delete [] upc_name;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *CBFileManager::OpenFile(const Common::String &filename, bool AbsPathWarning, bool keepTrackOf) {
	if (strcmp(filename.c_str(), "") == 0) return NULL;
	//Game->LOG(0, "open file: %s", Filename);
	/*#ifdef __WIN32__
	    if (Game->_dEBUG_DebugMode && Game->_dEBUG_AbsolutePathWarning && AbsPathWarning) {
	        char Drive[_MAX_DRIVE];
	        _splitpath(Filename, Drive, NULL, NULL, NULL);
	        if (Drive[0] != '\0') {
	            Game->LOG(0, "WARNING: Referencing absolute path '%s'. The game will NOT work on another computer.", Filename);
	        }
	    }
	#endif*/

	Common::SeekableReadStream *File = OpenFileRaw(filename);
	if (File && keepTrackOf) _openFiles.Add(File);
	return File;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::CloseFile(Common::SeekableReadStream *File) {
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
Common::SeekableReadStream *CBFileManager::OpenFileRaw(const Common::String &Filename) {
	RestoreCurrentDir();

	if (scumm_strnicmp(Filename.c_str(), "savegame:", 9) == 0) {
		CBSaveThumbFile *SaveThumbFile = new CBSaveThumbFile(Game);
		if (SUCCEEDED(SaveThumbFile->Open(Filename))) return SaveThumbFile->getMemStream();
		else {
			delete SaveThumbFile;
			return NULL;
		}
	}

	Common::SeekableReadStream *ret = NULL;

	ret = openDiskFile(Filename, this);
	if (ret) return ret;

	ret = openPkgFile(Filename, this);
	if (ret) return ret;

	ret = CBResources::getFile(Filename);
	if (ret) return ret;

	warning("BFileManager::OpenFileRaw - Failed to open %s", Filename.c_str());
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::RestoreCurrentDir() {
	if (!_basePath) return S_OK;
	else {
		/*if (!chdir(_basePath)) return S_OK;
		else return E_FAIL;*/
		warning("CBFileManager::RestoreCurrentDir - ignored");
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::SetBasePath(const Common::String &Path) {
	Cleanup();

	if (Path.c_str()) {
		_basePath = new char[Path.size() + 1];
		strcpy(_basePath, Path.c_str());
	}

	InitPaths();
	RegisterPackages();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::FindPackageSignature(Common::File *f, uint32 *Offset) {
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
