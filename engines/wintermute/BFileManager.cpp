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

#include "dcgf.h"
#include "BFileManager.h"
#include "StringUtil.h"
#include "PathUtil.h"
#include "BDiskFile.h"
#include "BResourceFile.h"
#include "BSaveThumbFile.h"
#include "BFileEntry.h"
#include "BPkgFile.h"
#include "BPackage.h"
#include "BRegistry.h"
#include "BGame.h"
#include "dcpackage.h"
#include "utils.h"
#include "PlatformSDL.h"
#include "common/str.h"
#include "common/textconsole.h"
#include <boost/filesystem.hpp>

#ifdef __WIN32__
#   include <direct.h>
#else
#   include <unistd.h>
#endif

#ifdef __APPLE__
#   include <CoreFoundation/CoreFoundation.h>
#endif


#if _DEBUG
#pragma comment(lib, "zlib_d.lib")
#else
#pragma comment(lib, "zlib.lib")
#endif


extern "C"
{
#include "zlib.h"
}

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CBFileManager::CBFileManager(CBGame *inGame): CBBase(inGame) {
	m_BasePath = NULL;

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
	for (i = 0; i < m_SinglePaths.GetSize(); i++)
		delete [] m_SinglePaths[i];
	m_SinglePaths.RemoveAll();

	for (i = 0; i < m_PackagePaths.GetSize(); i++)
		delete [] m_PackagePaths[i];
	m_PackagePaths.RemoveAll();


	// delete file entries
	m_FilesIter = m_Files.begin();
	while (m_FilesIter != m_Files.end()) {
		delete m_FilesIter->second;
		m_FilesIter++;
	}
	m_Files.clear();

	// close open files
	for (i = 0; i < m_OpenFiles.GetSize(); i++) {
		m_OpenFiles[i]->Close();
		delete m_OpenFiles[i];
	}
	m_OpenFiles.RemoveAll();


	// delete packages
	for (i = 0; i < m_Packages.GetSize(); i++)
		delete m_Packages[i];
	m_Packages.RemoveAll();

	delete[] m_BasePath;
	m_BasePath = NULL;

	return S_OK;
}



#define MAX_FILE_SIZE 10000000
//////////////////////////////////////////////////////////////////////
byte *CBFileManager::ReadWholeFile(const char *Filename, uint32 *Size, bool MustExist) {

	byte *buffer = NULL;

	CBFile *File = OpenFile(Filename);
	if (!File) {
		if (MustExist) Game->LOG(0, "Error opening file '%s'", Filename);
		return NULL;
	}

	/*
	if(File->GetSize()>MAX_FILE_SIZE){
	    Game->LOG(0, "File '%s' exceeds the maximum size limit (%d bytes)", Filename, MAX_FILE_SIZE);
	    CloseFile(File);
	    return NULL;
	}
	*/


	buffer = new byte[File->GetSize() + 1];
	if (buffer == NULL) {
		Game->LOG(0, "Error allocating buffer for file '%s' (%d bytes)", Filename, File->GetSize() + 1);
		CloseFile(File);
		return NULL;
	}

	if (FAILED(File->Read(buffer, File->GetSize()))) {
		Game->LOG(0, "Error reading file '%s'", Filename);
		CloseFile(File);
		delete [] buffer;
		return NULL;
	};

	buffer[File->GetSize()] = '\0';
	if (Size != NULL) *Size = File->GetSize();
	CloseFile(File);

	return buffer;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::SaveFile(char *Filename, byte *Buffer, uint32 BufferSize, bool Compressed, byte *PrefixBuffer, uint32 PrefixSize) {
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

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::RequestCD(int CD, char *PackageFile, char *Filename) {
	// unmount all non-local packages
	for (int i = 0; i < m_Packages.GetSize(); i++) {
		if (m_Packages[i]->m_CD > 0) m_Packages[i]->Close();
	}


	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::AddPath(TPathType Type, const char *Path) {
	if (Path == NULL || strlen(Path) < 1) return E_FAIL;

	bool slashed = (Path[strlen(Path) - 1] == '\\' || Path[strlen(Path) - 1] == '/');

	char *buffer = new char [strlen(Path) + 1 + (slashed ? 0 : 1)];
	if (buffer == NULL) return E_FAIL;

	strcpy(buffer, Path);
	if (!slashed) strcat(buffer, "\\");
	//CBPlatform::strlwr(buffer);

	switch (Type) {
	case PATH_SINGLE:
		m_SinglePaths.Add(buffer);
		break;
	case PATH_PACKAGE:
		m_PackagePaths.Add(buffer);
		break;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::ReloadPaths() {
	// delete registered paths
	for (int i = 0; i < m_SinglePaths.GetSize(); i++)
		delete [] m_SinglePaths[i];
	m_SinglePaths.RemoveAll();

	for (int i = 0; i < m_PackagePaths.GetSize(); i++)
		delete [] m_PackagePaths[i];
	m_PackagePaths.RemoveAll();

	return InitPaths();
}


#define TEMP_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::InitPaths() {
	RestoreCurrentDir();

	AnsiString pathList;
	int numPaths;

	// single files paths
	pathList = Game->m_Registry->ReadString("Resource", "CustomPaths", "");
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

#ifdef __APPLE__
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
#endif


	pathList = Game->m_Registry->ReadString("Resource", "PackagePaths", "");
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

	AnsiString extension = AnsiString(".") + AnsiString(PACKAGE_EXTENSION);

	for (int i = 0; i < m_PackagePaths.GetSize(); i++) {
		boost::filesystem::path absPath = boost::filesystem::system_complete(m_PackagePaths[i]);

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


	Game->LOG(0, "  Registered %d files in %d package(s)", m_Files.size(), m_Packages.GetSize());

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::RegisterPackage(const char *Path, const char *Name, bool SearchSignature) {
	char Filename[MAX_PATH];
	sprintf(Filename, "%s%s", Path, Name);

	FILE *f = fopen(Filename, "rb");
	if (!f) {
		Game->LOG(0, "  Error opening package file '%s'. Ignoring.", Filename);
		return S_OK;
	}

	uint32 AbosulteOffset = 0;
	bool BoundToExe = false;

	if (SearchSignature) {
		uint32 Offset;
		if (!FindPackageSignature(f, &Offset)) {
			fclose(f);
			return S_OK;
		} else {
			fseek(f, Offset, SEEK_SET);
			AbosulteOffset = Offset;
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
		DirOffset += AbosulteOffset;
		fseek(f, DirOffset, SEEK_SET);
	}

	for (int i = 0; i < hdr.NumDirs; i++) {
		CBPackage *pkg = new CBPackage(Game);
		if (!pkg) return E_FAIL;

		pkg->m_BoundToExe = BoundToExe;

		// read package info
		byte NameLength;
		fread(&NameLength, sizeof(byte ), 1, f);
		pkg->m_Name = new char[NameLength];
		fread(pkg->m_Name, NameLength, 1, f);
		fread(&pkg->m_CD, sizeof(byte ), 1, f);
		pkg->m_Priority = hdr.Priority;

		if (!hdr.MasterIndex) pkg->m_CD = 0; // override CD to fixed disk
		m_Packages.Add(pkg);


		// read file entries
		uint32 NumFiles;
		fread(&NumFiles, sizeof(uint32), 1, f);

		for (int j = 0; j < NumFiles; j++) {
			char *Name;
			uint32 Offset, Length, CompLength, Flags, TimeDate1, TimeDate2;

			fread(&NameLength, sizeof(byte ), 1, f);
			Name = new char[NameLength];
			fread(Name, NameLength, 1, f);

			// v2 - xor name
			if (hdr.PackageVersion == PACKAGE_VERSION) {
				for (int k = 0; k < NameLength; k++) {
					((byte  *)Name)[k] ^= 'D';
				}
			}

			// some old version of ProjectMan writes invalid directory entries
			// so at least prevent strupr from corrupting memory
			Name[NameLength - 1] = '\0';


			CBPlatform::strupr(Name);

			fread(&Offset, sizeof(uint32), 1, f);
			Offset += AbosulteOffset;
			fread(&Length, sizeof(uint32), 1, f);
			fread(&CompLength, sizeof(uint32), 1, f);
			fread(&Flags, sizeof(uint32), 1, f);

			if (hdr.PackageVersion == PACKAGE_VERSION) {
				fread(&TimeDate1, sizeof(uint32), 1, f);
				fread(&TimeDate2, sizeof(uint32), 1, f);
			}

			m_FilesIter = m_Files.find(Name);
			if (m_FilesIter == m_Files.end()) {
				CBFileEntry *file = new CBFileEntry(Game);
				file->m_Package = pkg;
				file->m_Offset = Offset;
				file->m_Length = Length;
				file->m_CompressedLength = CompLength;
				file->m_Flags = Flags;

				m_Files[Name] = file;
			} else {
				// current package has lower CD number or higher priority, than the registered
				if (pkg->m_CD < m_FilesIter->second->m_Package->m_CD || pkg->m_Priority > m_FilesIter->second->m_Package->m_Priority) {
					m_FilesIter->second->m_Package = pkg;
					m_FilesIter->second->m_Offset = Offset;
					m_FilesIter->second->m_Length = Length;
					m_FilesIter->second->m_CompressedLength = CompLength;
					m_FilesIter->second->m_Flags = Flags;
				}
			}
			delete [] Name;
		}
	}


	fclose(f);

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
FILE *CBFileManager::OpenPackage(char *Name) {
	RestoreCurrentDir();

	FILE *ret = NULL;
	char Filename[MAX_PATH];

	for (int i = 0; i < m_PackagePaths.GetSize(); i++) {
		sprintf(Filename, "%s%s.%s", m_PackagePaths[i], Name, PACKAGE_EXTENSION);
		ret = fopen(Filename, "rb");
		if (ret != NULL) return ret;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
FILE *CBFileManager::OpenSingleFile(char *Name) {
	RestoreCurrentDir();

	FILE *ret = NULL;
	char Filename[MAX_PATH];

	for (int i = 0; i < m_SinglePaths.GetSize(); i++) {
		sprintf(Filename, "%s%s", m_SinglePaths[i], Name);
		ret = fopen(Filename, "rb");
		if (ret != NULL) return ret;
	}

	// didn't find in search paths, try to open directly
	return fopen(Name, "rb");
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::GetFullPath(char *Filename, char *Fullname) {
	RestoreCurrentDir();

	FILE *f = NULL;
	bool found = false;

	for (int i = 0; i < m_SinglePaths.GetSize(); i++) {
		sprintf(Fullname, "%s%s", m_SinglePaths[i], Filename);
		f = fopen(Fullname, "rb");
		if (f) {
			fclose(f);
			found = true;
			break;
		}
	}

	if (!found) {
		f = fopen(Filename, "rb");
		if (f) {
			fclose(f);
			found = true;
			strcpy(Fullname, Filename);
		}
	}

	return found;
}


//////////////////////////////////////////////////////////////////////////
CBFileEntry *CBFileManager::GetPackageEntry(const char *Filename) {
	char *upc_name = new char[strlen(Filename) + 1];
	strcpy(upc_name, Filename);
	CBPlatform::strupr(upc_name);

	CBFileEntry *ret = NULL;
	m_FilesIter = m_Files.find(upc_name);
	if (m_FilesIter != m_Files.end()) ret = m_FilesIter->second;

	delete [] upc_name;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
CBFile *CBFileManager::OpenFile(const char *Filename, bool AbsPathWarning) {
	if (strcmp(Filename, "") == 0) return NULL;
	//Game->LOG(0, "open file: %s", Filename);
#ifdef __WIN32__
	if (Game->m_DEBUG_DebugMode && Game->m_DEBUG_AbsolutePathWarning && AbsPathWarning) {
		char Drive[_MAX_DRIVE];
		_splitpath(Filename, Drive, NULL, NULL, NULL);
		if (Drive[0] != '\0') {
			Game->LOG(0, "WARNING: Referencing absolute path '%s'. The game will NOT work on another computer.", Filename);
		}
	}
#endif

	CBFile *File = OpenFileRaw(Filename);
	if (File) m_OpenFiles.Add(File);
	return File;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::CloseFile(CBFile *File) {
	for (int i = 0; i < m_OpenFiles.GetSize(); i++) {
		if (m_OpenFiles[i] == File) {
			m_OpenFiles[i]->Close();
			delete m_OpenFiles[i];
			m_OpenFiles.RemoveAt(i);
			return S_OK;
		}
	}
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CBFile *CBFileManager::OpenFileRaw(const char *Filename) {
	RestoreCurrentDir();

	if (scumm_strnicmp(Filename, "savegame:", 9) == 0) {
		CBSaveThumbFile *SaveThumbFile = new CBSaveThumbFile(Game);
		if (SUCCEEDED(SaveThumbFile->Open(Filename))) return SaveThumbFile;
		else {
			delete SaveThumbFile;
			return NULL;
		}
	}

	CBDiskFile *DiskFile = new CBDiskFile(Game);
	if (SUCCEEDED(DiskFile->Open(Filename))) return DiskFile;

	delete DiskFile;

	CBPkgFile *PkgFile = new CBPkgFile(Game);
	if (SUCCEEDED(PkgFile->Open(Filename))) return PkgFile;

	delete PkgFile;

	CBResourceFile *ResFile = new CBResourceFile(Game);
	if (SUCCEEDED(ResFile->Open(Filename))) return ResFile;

	delete ResFile;

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::RestoreCurrentDir() {
	if (!m_BasePath) return S_OK;
	else {
		if (!chdir(m_BasePath)) return S_OK;
		else return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFileManager::SetBasePath(char *Path) {
	Cleanup();

	if (Path) {
		m_BasePath = new char[strlen(Path) + 1];
		strcpy(m_BasePath, Path);
	}

	InitPaths();
	RegisterPackages();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFileManager::FindPackageSignature(FILE *f, uint32 *Offset) {
	byte buf[32768];

	byte Signature[8];
	((uint32 *)Signature)[0] = PACKAGE_MAGIC_1;
	((uint32 *)Signature)[1] = PACKAGE_MAGIC_2;

	fseek(f, 0, SEEK_END);
	uint32 FileSize = ftell(f);

	int StartPos = 1024 * 1024;

	int BytesRead = StartPos;

	while (BytesRead < FileSize - 16) {
		int ToRead = MIN(32768, FileSize - BytesRead);
		fseek(f, StartPos, SEEK_SET);
		int ActuallyRead = fread(buf, 1, ToRead, f);
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
