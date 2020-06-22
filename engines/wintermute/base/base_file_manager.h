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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#ifndef WINTERMUTE_BASE_FILE_MANAGER_H
#define WINTERMUTE_BASE_FILE_MANAGER_H

#include "common/archive.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/language.h"

namespace Wintermute {
class BaseFileManager {
public:
	bool cleanup();

	bool closeFile(Common::SeekableReadStream *File);
	bool hasFile(const Common::String &filename);
	int listMatchingPackageMembers(Common::ArchiveMemberList &list, const Common::String &pattern);
	int listMatchingFiles(Common::StringArray &list, const Common::String &pattern);
	Common::SeekableReadStream *openFile(const Common::String &filename, bool absPathWarning = true, bool keepTrackOf = true);
	Common::WriteStream *openFileForWrite(const Common::String &filename);
	byte *readWholeFile(const Common::String &filename, uint32 *size = nullptr, bool mustExist = true);
	uint32 getPackageVersion(const Common::String &filename);

	BaseFileManager(Common::Language lang, bool detectionMode = false);
	virtual ~BaseFileManager();
	// Used only for detection
	bool registerPackages(const Common::FSList &fslist);
	static BaseFileManager *getEngineInstance();
private:
	typedef enum {
		PATH_PACKAGE,
		PATH_SINGLE
	} TPathType;
	bool reloadPaths();
	bool initPaths();
	bool addPath(TPathType type, const Common::FSNode &path);
	bool registerPackages();
	void initResources();
	Common::SeekableReadStream *openFileRaw(const Common::String &filename);
	Common::WriteStream *openFileForWriteRaw(const Common::String &filename);
	Common::SeekableReadStream *openPkgFile(const Common::String &filename);
	Common::FSList _packagePaths;
	bool registerPackage(Common::FSNode package, const Common::String &filename = "", bool searchSignature = false);
	bool _detectionMode;
	Common::SearchSet _packages;
	Common::Array<Common::SeekableReadStream *> _openFiles;
	Common::Language _language;
	Common::Archive *_resources;
	Common::HashMap<Common::String, uint32> _versions;

	// This class is intentionally not a subclass of Base, as it needs to be used by
	// the detector too, without launching the entire engine:
};

} // End of namespace Wintermute

#endif
