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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"
#include "sword25/kernel/filesystemutil.h"
#include "sword25/kernel/persistenceservice.h"

namespace Sword25 {

#define BS_LOG_PREFIX "FILESYSTEMUTIL"

Common::String getAbsolutePath(const Common::String &path) {
	Common::FSNode node(path);

	if (!node.exists()) {
		// An error has occurred finding the node
		// We can do nothing at this pointer than return an empty string
		BS_LOG_ERRORLN("A call to GetAbsolutePath failed.");
		return "";
	}

	// Return the result
	return node.getPath();
}

class BS_FileSystemUtilScummVM : public FileSystemUtil {
public:
	virtual Common::String getUserdataDirectory() {
		Common::String path = ConfMan.get("savepath");

		if (path.empty()) {
			error("No save path has been defined");
			return "";
		}

		// Return the path
		return path;
	}

	virtual Common::String getPathSeparator() {
		return Common::String("/");
	}

	virtual int32 getFileSize(const Common::String &filename) {
		Common::FSNode node(filename);

		// If the file does not exist, return -1 as a result
		if (!node.exists())
			return -1;

		// Get the size of the file and return it
		Common::File f;
		f.open(node);
		uint32 size = f.size();
		f.close();

		return size;
	}

	virtual TimeDate getFileTime(const Common::String &filename) {
		// TODO: There isn't any way in ScummVM to get a file's modified date/time. We will need to check
		// what code makes use of it. If it's only the save game code, for example, we may be able to
		// encode the date/time inside the savegame files themselves.
		TimeDate result;
		g_system->getTimeAndDate(result);
		return result;
	}

	virtual bool fileExists(const Common::String &filename) {
		Common::File f;
		if (f.exists(filename))
			return true;

		// Check if the file exists in the save folder
		Common::FSNode folder(PersistenceService::getSavegameDirectory());
		Common::FSNode fileNode = folder.getChild(FileSystemUtil::getInstance().getPathFilename(filename));
		return fileNode.exists();
	}

	virtual bool createDirectory(const Common::String &sirectoryName) {
		// ScummVM doesn't support creating folders, so this is only a stub
		BS_LOG_ERRORLN("CreateDirectory method called");
		return false;
	}

	virtual Common::String getPathFilename(const Common::String &path) {
		for (int i = path.size() - 1; i >= 0; --i) {
			if ((path[i] == '/') || (path[i] == '\\')) {
				return Common::String(&path.c_str()[i + 1]);
			}
		}

		return path;
	}
};

FileSystemUtil &FileSystemUtil::getInstance() {
	static BS_FileSystemUtilScummVM instance;
	return instance;
}

} // End of namespace Sword25
