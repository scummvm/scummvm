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

#include "common/savefile.h"
#include "common/util.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "backends/saves/default/default-saves.h"
#include "backends/saves/compressed/compressed-saves.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(UNIX) || defined(__SYMBIAN32__)
#include <sys/stat.h>
#endif

#ifdef IPHONE
#include "backends/platform/iphone/osys_iphone.h"
#endif

#ifdef UNIX
#ifdef MACOSX
#define DEFAULT_SAVE_PATH "Documents/ScummVM Savegames"
#else
#define DEFAULT_SAVE_PATH ".scummvm"
#endif
#elif defined(__SYMBIAN32__)
#define DEFAULT_SAVE_PATH "Savegames"
#elif defined(PALMOS_MODE)
#define DEFAULT_SAVE_PATH "/PALM/Programs/ScummVM/Saved"
#endif

DefaultSaveFileManager::DefaultSaveFileManager() {
	// Register default savepath
	// TODO: Remove this code here, and instead leave setting the
	// default savepath to the ports using this class.
#ifdef DEFAULT_SAVE_PATH
	char savePath[MAXPATHLEN];
#if defined(UNIX) && !defined(IPHONE)
	const char *home = getenv("HOME");
	if (home && *home && strlen(home) < MAXPATHLEN) {
		snprintf(savePath, MAXPATHLEN, "%s/%s", home, DEFAULT_SAVE_PATH);
		ConfMan.registerDefault("savepath", savePath);
	}
#elif defined(__SYMBIAN32__)
	strcpy(savePath, Symbian::GetExecutablePath());
	strcat(savePath, DEFAULT_SAVE_PATH);
	strcat(savePath, "\\");
	ConfMan.registerDefault("savepath", savePath);
#elif defined (IPHONE)
	ConfMan.registerDefault("savepath", OSystem_IPHONE::getSavePath());

#elif defined(PALMOS_MODE)
	ConfMan.registerDefault("savepath", DEFAULT_SAVE_PATH);
#endif
#endif // #ifdef DEFAULT_SAVE_PATH
}



Common::StringList DefaultSaveFileManager::listSavefiles(const char *pattern) {
	FilesystemNode savePath(getSavePath());
	FSList savefiles;
	Common::StringList results;
	Common::String search(pattern);

	if (savePath.lookupFile(savefiles, search, false, true, 0)) {
		for (FSList::const_iterator file = savefiles.begin(); file != savefiles.end(); ++file) {
			results.push_back(file->getName());
		}
	}

	return results;
}

void DefaultSaveFileManager::checkPath(const FilesystemNode &dir) {
	const Common::String path = dir.getPath();
	clearError();

#if defined(UNIX) || defined(__SYMBIAN32__)
	struct stat sb;

	// Check whether the dir exists
	if (stat(path.c_str(), &sb) == -1) {
		// The dir does not exist, or stat failed for some other reason.
		// If the problem was that the path pointed to nothing, try
		// to create the dir (ENOENT case).
		switch (errno) {
		case EACCES:
			setError(SFM_DIR_ACCESS, "Search or write permission denied: "+path);
			break;
#if !defined(__SYMBIAN32__)
		case ELOOP:
			setError(SFM_DIR_LOOP, "Too many symbolic links encountered while traversing the path: "+path);
			break;
#endif
		case ENAMETOOLONG:
			setError(SFM_DIR_NAMETOOLONG, "The path name is too long: "+path);
			break;
		case ENOENT:
			if (mkdir(path.c_str(), 0755) != 0) {
				// mkdir could fail for various reasons: The parent dir doesn't exist,
				// or is not writeable, the path could be completly bogus, etc.
				warning("mkdir for '%s' failed!", path.c_str());
				perror("mkdir");

				switch (errno) {
				case EACCES:
					setError(SFM_DIR_ACCESS, "Search or write permission denied: "+path);
					break;
				case EMLINK:
					setError(SFM_DIR_LINKMAX, "The link count of the parent directory would exceed {LINK_MAX}: "+path);
					break;
#if !defined(__SYMBIAN32__)
				case ELOOP:
					setError(SFM_DIR_LOOP, "Too many symbolic links encountered while traversing the path: "+path);
					break;
#endif
				case ENAMETOOLONG:
					setError(SFM_DIR_NAMETOOLONG, "The path name is too long: "+path);
					break;
				case ENOENT:
					setError(SFM_DIR_NOENT, "A component of the path does not exist, or the path is an empty string: "+path);
					break;
				case ENOTDIR:
					setError(SFM_DIR_NOTDIR, "A component of the path prefix is not a directory: "+path);
					break;
				case EROFS:
					setError(SFM_DIR_ROFS, "The parent directory resides on a read-only file system:"+path);
					break;
				}
			}
			break;
		case ENOTDIR:
			setError(SFM_DIR_NOTDIR, "A component of the path prefix is not a directory: "+path);
			break;
		}
	} else {
		// So stat() succeeded. But is the path actually pointing to a directory?
		if (!S_ISDIR(sb.st_mode)) {
			setError(SFM_DIR_NOTDIR, "The given savepath is not a directory: "+path);
		}
	}
#else
	if (!dir.exists()) {
		// TODO: We could try to mkdir the directory here; or rather, we could
		// add a mkdir method to FilesystemNode and invoke that here.
		setError(SFM_DIR_NOENT, "A component of the path does not exist, or the path is an empty string: "+path);
	} else if (!dir.isDirectory()) {
		setError(SFM_DIR_NOTDIR, "The given savepath is not a directory: "+path);
	}
#endif
}

Common::InSaveFile *DefaultSaveFileManager::openForLoading(const char *filename) {
	// Ensure that the savepath is valid. If not, generate an appropriate error.
	FilesystemNode savePath(getSavePath());
	checkPath(savePath);

	if (getError() == SFM_NO_ERROR) {
		FilesystemNode file = savePath.getChild(filename);

		// Open the file for reading
		Common::SeekableReadStream *sf = file.openForReading();

		return wrapInSaveFile(sf);
	} else {
		return 0;
	}
}

Common::OutSaveFile *DefaultSaveFileManager::openForSaving(const char *filename) {
	// Ensure that the savepath is valid. If not, generate an appropriate error.
	FilesystemNode savePath(getSavePath());
	checkPath(savePath);

	if (getError() == SFM_NO_ERROR) {
		FilesystemNode file = savePath.getChild(filename);

		// Open the file for saving
		Common::WriteStream *sf = file.openForWriting();

		return wrapOutSaveFile(sf);
	} else {
		return 0;
	}
}

bool DefaultSaveFileManager::removeSavefile(const char *filename) {
	clearError();

	FilesystemNode savePath(getSavePath());
	FilesystemNode file = savePath.getChild(filename);

	// TODO: Add new method FilesystemNode::remove()
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
