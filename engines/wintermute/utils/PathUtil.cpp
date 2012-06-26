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
#include "common/file.h"
#include "PathUtil.h"
#include "StringUtil.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::UnifySeparators(const AnsiString &path) {
	AnsiString newPath = path;

	AnsiString::iterator it;
	for (it = newPath.begin(); it != newPath.end(); it++) {
		if (*it == '\\') *it = '/';
	}
	return newPath;
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::NormalizeFileName(const AnsiString &path) {
	AnsiString newPath = UnifySeparators(path);
	StringUtil::ToLowerCase(newPath);
	return newPath;
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::Combine(const AnsiString &path1, const AnsiString &path2) {
	AnsiString newPath1 = UnifySeparators(path1);
	AnsiString newPath2 = UnifySeparators(path2);

	if (!StringUtil::EndsWith(newPath1, "/", true) && !StringUtil::StartsWith(newPath2, "/", true))
		newPath1 += "/";

	return newPath1 + newPath2;
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::GetDirectoryName(const AnsiString &path) {
	AnsiString newPath = UnifySeparators(path);

	//size_t pos = newPath.find_last_of(L'/');
	Common::String filename = GetFileName(path);
	return Common::String(path.c_str(), path.size() - filename.size());
	//if (pos == AnsiString::npos) return "";
	//else return newPath.substr(0, pos + 1);
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::GetFileName(const AnsiString &path) {
	AnsiString newPath = UnifySeparators(path);

	//size_t pos = newPath.find_last_of(L'/'); TODO REMOVE.
	Common::String lastPart = Common::lastPathComponent(path, '/');
	if (lastPart[lastPart.size() - 1 ] != '/')
		return lastPart;
	else
		return path;
	//if (pos == AnsiString::npos) return path;
	//else return newPath.substr(pos + 1);
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::GetFileNameWithoutExtension(const AnsiString &path) {
	AnsiString fileName = GetFileName(path);

	//size_t pos = fileName.find_last_of('.'); //TODO REMOVE!
	// TODO: Prettify this.
	Common::String extension = Common::lastPathComponent(path, '.');
	Common::String filename = Common::String(path.c_str(), path.size() - extension.size());
	return filename;
	//if (pos == AnsiString::npos) return fileName;
	//else return fileName.substr(0, pos);
}

//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::GetExtension(const AnsiString &path) {
	AnsiString fileName = GetFileName(path);

	//size_t pos = fileName.find_last_of('.');
	return Common::lastPathComponent(path, '.');
	//if (pos == AnsiString::npos) return "";
	//else return fileName.substr(pos);
}


//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::GetSafeLogFileName() {
	AnsiString logFileName = GetUserDirectory();

#ifdef __WIN32__
	char moduleName[MAX_PATH];
	::GetModuleFileName(NULL, moduleName, MAX_PATH);

	AnsiString fileName = GetFileNameWithoutExtension(moduleName) + ".log";
	fileName = Combine("/Wintermute Engine/Logs/", fileName);
	logFileName = Combine(logFileName, fileName);

#else
	// !PORTME
	logFileName = Combine(logFileName, "/Wintermute Engine/wme.log");
#endif

	CreateDirectory(GetDirectoryName(logFileName));
	return logFileName;
}

//////////////////////////////////////////////////////////////////////////
bool PathUtil::CreateDirectory(const AnsiString &path) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool PathUtil::MatchesMask(const AnsiString &fileName, const AnsiString &mask) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool PathUtil::FileExists(const AnsiString &fileName) {
	warning("PathUtil::FileExists(%s)", fileName.c_str());

	Common::File stream;

	stream.open(fileName.c_str());
	bool ret = stream.isOpen();
	stream.close();

	return ret;
}


//////////////////////////////////////////////////////////////////////////
AnsiString PathUtil::GetUserDirectory() {
	warning("PathUtil::GetUserDirectory - stubbed");
	AnsiString userDir = "./";
#if 0
#ifdef __WIN32__
	char buffer[MAX_PATH];
	buffer[0] = '\0';
	LPITEMIDLIST pidl = NULL;
	LPMALLOC pMalloc;
	if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
		SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
		if (pidl) {
			SHGetPathFromIDList(pidl, buffer);
		}
		pMalloc->Free(pidl);
		userDir = AnsiString(buffer);
	}
#elif __MACOSX__
	FSRef fileRef;
	OSStatus error = FSFindFolder(kUserDomain, kApplicationSupportFolderType, true, &fileRef);
	if (error == noErr) {
		char buffer[MAX_PATH];
		error = FSRefMakePath(&fileRef, (UInt8 *)buffer, sizeof(buffer));
		if (error == noErr)
			userDir = buffer;

	}
#elif __IPHONEOS__
	char path[MAX_PATH];
	IOS_GetDataDir(path);
	userDir = AnsiString(path);
#endif
#endif // 0
	return userDir;
}

} // end of namespace WinterMute
