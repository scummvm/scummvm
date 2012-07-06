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

#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/Base/BFileEntry.h"
#include "common/archive.h"
#include "common/str.h"

namespace Common {
class File;
}

namespace WinterMute {
class CBFile;
class CBFileManager: CBBase {
public:
	bool findPackageSignature(Common::File *f, uint32 *offset);
	HRESULT cleanup();
	HRESULT setBasePath(const Common::String &path);
	HRESULT restoreCurrentDir();
	char *_basePath;
	bool getFullPath(const Common::String &filename, char *fullname);
	Common::SeekableReadStream *openFileRaw(const Common::String &filename);
	HRESULT closeFile(Common::SeekableReadStream *File);
	bool hasFile(const Common::String &filename);
	Common::SeekableReadStream *openFile(const Common::String &filename, bool absPathWarning = true, bool keepTrackOf = true);
	CBFileEntry *getPackageEntry(const Common::String &filename);
	Common::File *openSingleFile(const Common::String &name);
	Common::File *openPackage(const Common::String &name);
	HRESULT registerPackages();
	HRESULT initPaths();
	HRESULT reloadPaths();
	typedef enum {
	    PATH_PACKAGE, PATH_SINGLE
	} TPathType;
	HRESULT addPath(TPathType type, const Common::String &path);
	HRESULT requestCD(int cd, char *packageFile, char *filename);
	Common::SeekableReadStream *loadSaveGame(const Common::String &filename);
	HRESULT saveFile(const Common::String &filename, byte *buffer, uint32 bufferSize, bool compressed = false, byte *prefixBuffer = NULL, uint32 prefixSize = 0);
	byte *readWholeFile(const Common::String &filename, uint32 *size = NULL, bool mustExist = true);
	CBFileManager(CBGame *inGame = NULL);
	virtual ~CBFileManager();
	CBArray<char *, char *> _singlePaths;
	CBArray<char *, char *> _packagePaths;
	CBArray<CBPackage *, CBPackage *> _packages;
	CBArray<Common::SeekableReadStream *, Common::SeekableReadStream *> _openFiles;

	Common::HashMap<Common::String, CBFileEntry *> _files;
private:
	HRESULT registerPackage(const char *path, const char *name, bool searchSignature = false);
	HRESULT registerPackage(const Common::String &filename, bool searchSignature = false);
	Common::HashMap<Common::String, CBFileEntry *>::iterator _filesIter;
	bool isValidPackage(const AnsiString &fileName) const;

};

} // end of namespace WinterMute

#endif
