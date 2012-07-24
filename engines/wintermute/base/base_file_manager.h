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

#include "common/archive.h"
#include "common/str.h"
#include "common/fs.h"
#include "common/file.h"

namespace WinterMute {
class BaseFile;
class BaseFileEntry;
class BaseGame;
class BasePackage;
class BaseFileManager {
public:
	bool cleanup();

	bool closeFile(Common::SeekableReadStream *File);
	bool hasFile(const Common::String &filename);
	Common::SeekableReadStream *openFile(const Common::String &filename, bool absPathWarning = true, bool keepTrackOf = true);
	Common::SeekableReadStream *loadSaveGame(const Common::String &filename);
	byte *readWholeFile(const Common::String &filename, uint32 *size = NULL, bool mustExist = true);

	BaseFileManager(BaseGame *inGame = NULL);
	virtual ~BaseFileManager();
	// Used only by BasePersistenceManager
	bool saveFile(const Common::String &filename, byte *buffer, uint32 bufferSize, bool compressed = false, byte *prefixBuffer = NULL, uint32 prefixSize = 0);
	// Used only for detection
	bool registerPackages(const Common::FSList &fslist);
	// Used by BasePackage only
	BaseFileEntry *getPackageEntry(const Common::String &filename);
	Common::File *openPackage(const Common::String &name);
	bool requestCD(int cd, char *packageFile, const char *filename);
private:
	typedef enum {
	    PATH_PACKAGE,
		PATH_SINGLE
	} TPathType;
	bool reloadPaths();
	bool initPaths();
	bool addPath(TPathType type, const Common::FSNode &path);
	bool registerPackages();
	Common::SeekableReadStream *openFileRaw(const Common::String &filename);
	Common::FSList _packagePaths;
//	Common::FSList _singlePaths;
	bool findPackageSignature(Common::SeekableReadStream *f, uint32 *offset);
	bool registerPackage(Common::FSNode package, const Common::String &filename = "", bool searchSignature = false);
//	bool registerPackage(const Common::String &filename, bool searchSignature = false);
	BaseGame *_gameRef;
	Common::Array<BasePackage *> _packages;
	Common::Array<Common::SeekableReadStream *> _openFiles;
	Common::HashMap<Common::String, BaseFileEntry *> _files;
	Common::HashMap<Common::String, BaseFileEntry *>::iterator _filesIter;
};

} // end of namespace WinterMute

#endif
