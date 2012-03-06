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

#ifndef WINTERMUTE_BFILEMANAGER_H
#define WINTERMUTE_BFILEMANAGER_H


#include <map>
#include "coll_templ.h"
#include "BFileEntry.h"
#include "common/archive.h"

namespace WinterMute {
class CBFile;
class CBFileManager: CBBase {
public:
	bool FindPackageSignature(FILE *f, uint32 *Offset);
	HRESULT Cleanup();
	HRESULT SetBasePath(char *Path);
	HRESULT RestoreCurrentDir();
	char *m_BasePath;
	bool GetFullPath(char *Filename, char *Fullname);
	CBFile *OpenFileRaw(const char *Filename);
	HRESULT CloseFile(CBFile *File);
	CBFile *OpenFile(const char *Filename, bool AbsPathWarning = true);
	CBFileEntry *GetPackageEntry(const char *Filename);
	FILE *OpenSingleFile(char *Name);
	FILE *OpenPackage(char *Name);
	HRESULT RegisterPackages();
	HRESULT InitPaths();
	HRESULT ReloadPaths();
	typedef enum {
	    PATH_PACKAGE, PATH_SINGLE
	} TPathType;
	HRESULT AddPath(TPathType Type, const char *Path);
	HRESULT RequestCD(int CD, char *PackageFile, char *Filename);
	HRESULT SaveFile(char *Filename, byte *Buffer, uint32 BufferSize, bool Compressed = false, byte *PrefixBuffer = NULL, uint32 PrefixSize = 0);
	byte *ReadWholeFile(const char *Filename, uint32 *Size = NULL, bool MustExist = true);
	CBFileManager(CBGame *inGame = NULL);
	virtual ~CBFileManager();
	CBArray<char *, char *> m_SinglePaths;
	CBArray<char *, char *> m_PackagePaths;
	CBArray<CBPackage *, CBPackage *> m_Packages;
	CBArray<CBFile *, CBFile *> m_OpenFiles;

	std::map<std::string, CBFileEntry *> m_Files;
private:
	HRESULT RegisterPackage(const char *Path, const char *Name, bool SearchSignature = false);
	std::map<std::string, CBFileEntry *>::iterator m_FilesIter;
	bool IsValidPackage(const AnsiString &fileName) const;

};

} // end of namespace WinterMute

#endif
