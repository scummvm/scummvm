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

#if !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#include "backends/saves/default/default-saves.h"
#include "backends/saves/compressed/compressed-saves.h"

#include "common/savefile.h"
#include "common/util.h"
#include "common/fs.h"
#include "common/config-manager.h"

#include <errno.h>	// for removeSavefile()


DefaultSaveFileManager::DefaultSaveFileManager() {
}

DefaultSaveFileManager::DefaultSaveFileManager(const Common::String &defaultSavepath) {
	ConfMan.registerDefault("savepath", defaultSavepath);
}


Common::StringList DefaultSaveFileManager::listSavefiles(const char *pattern) {
	Common::FSNode savePath(getSavePath());
	Common::FSList savefiles;
	Common::StringList results;
	Common::String search(pattern);

	if (savePath.lookupFile(savefiles, search, false, true, 0)) {
		for (Common::FSList::const_iterator file = savefiles.begin(); file != savefiles.end(); ++file) {
			results.push_back(file->getName());
		}
	}

	return results;
}

void DefaultSaveFileManager::checkPath(const Common::FSNode &dir) {
	clearError();
	if (!dir.exists()) {
		setError(SFM_DIR_NOENT, "A component of the path does not exist, or the path is an empty string: "+dir.getPath());
	} else if (!dir.isDirectory()) {
		setError(SFM_DIR_NOTDIR, "The given savepath is not a directory: "+dir.getPath());
	}
}

Common::InSaveFile *DefaultSaveFileManager::openForLoading(const char *filename) {
	// Ensure that the savepath is valid. If not, generate an appropriate error.
	Common::FSNode savePath(getSavePath());
	checkPath(savePath);

	if (getError() == SFM_NO_ERROR) {
		Common::FSNode file = savePath.getChild(filename);

		// Open the file for reading
		Common::SeekableReadStream *sf = file.openForReading();

		return wrapInSaveFile(sf);
	} else {
		return 0;
	}
}

Common::OutSaveFile *DefaultSaveFileManager::openForSaving(const char *filename) {
	// Ensure that the savepath is valid. If not, generate an appropriate error.
	Common::FSNode savePath(getSavePath());
	checkPath(savePath);

	if (getError() == SFM_NO_ERROR) {
		Common::FSNode file = savePath.getChild(filename);

		// Open the file for saving
		Common::WriteStream *sf = file.openForWriting();

		return wrapOutSaveFile(sf);
	} else {
		return 0;
	}
}

bool DefaultSaveFileManager::removeSavefile(const char *filename) {
	clearError();

	Common::FSNode savePath(getSavePath());
	Common::FSNode file = savePath.getChild(filename);

	// TODO: Add new method FSNode::remove()
	if (remove(file.getPath().c_str()) != 0) {
#ifndef _WIN32_WCE
		if (errno == EACCES)
			setError(SFM_DIR_ACCESS, "Search or write permission denied: "+file.getName());

		if (errno == ENOENT)
			setError(SFM_DIR_NOENT, "A component of the path does not exist, or the path is an empty string: "+file.getName());
#endif
		return false;
	} else {
		return true;
	}
}

Common::String DefaultSaveFileManager::getSavePath() const {

	Common::String dir;

	// Try to use game specific savepath from config
	dir = ConfMan.get("savepath");

	// Work around a bug (#999122) in the original 0.6.1 release of
	// ScummVM, which would insert a bad savepath value into config files.
	if (dir == "None") {
		ConfMan.removeKey("savepath", ConfMan.getActiveDomainName());
		ConfMan.flushToDisk();
		dir = ConfMan.get("savepath");
	}

#ifdef _WIN32_WCE
	if (dir.empty())
		dir = ConfMan.get("path");
#endif

	return dir;
}

#endif // !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
