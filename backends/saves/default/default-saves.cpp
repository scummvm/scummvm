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
#include "backends/saves/default/default-saves.h"
#include "backends/saves/compressed/compressed-saves.h"

#include <stdio.h>
#include <string.h>

#if defined(UNIX) || defined(__SYMBIAN32__)
#include <errno.h>
#include <sys/stat.h>
#endif


class StdioSaveFile : public Common::InSaveFile, public Common::OutSaveFile {
private:
	FILE *fh;
public:
	StdioSaveFile(const char *filename, bool saveOrLoad) {
		fh = ::fopen(filename, (saveOrLoad? "wb" : "rb"));
	}
	~StdioSaveFile() {
		if (fh)
			::fclose(fh);
	}

	bool eos() const { return feof(fh) != 0; }
	bool ioFailed() const { return ferror(fh) != 0; }
	void clearIOFailed() { clearerr(fh); }

	bool isOpen() const { return fh != 0; }

	uint32 read(void *dataPtr, uint32 dataSize) {
		assert(fh);
		return fread(dataPtr, 1, dataSize, fh);
	}
	uint32 write(const void *dataPtr, uint32 dataSize) {
		assert(fh);
		return fwrite(dataPtr, 1, dataSize, fh);
	}

	uint32 pos() const {
		assert(fh);
		return ftell(fh);
	}
	uint32 size() const {
		assert(fh);
		uint32 oldPos = ftell(fh);
		fseek(fh, 0, SEEK_END);
		uint32 length = ftell(fh);
		fseek(fh, oldPos, SEEK_SET);
		return length;
	}

	void seek(int32 offs, int whence = SEEK_SET) {
		assert(fh);
		fseek(fh, offs, whence);
	}
};

static void join_paths(const char *filename, const char *directory,
								 char *buf, int bufsize) {
	buf[bufsize-1] = '\0';
	strncpy(buf, directory, bufsize-1);

#ifdef WIN32
	// Fix for Win98 issue related with game directory pointing to root drive ex. "c:\"
	if ((buf[0] != 0) && (buf[1] == ':') && (buf[2] == '\\') && (buf[3] == 0)) {
		buf[2] = 0;
	}
#endif

	const int dirLen = strlen(buf);

	if (dirLen > 0) {
#if defined(__MORPHOS__) || defined(__amigaos4__)
		if (buf[dirLen-1] != ':' && buf[dirLen-1] != '/')
#endif

#if !defined(__GP32__)
		strncat(buf, "/", bufsize-1);	// prevent double /
#endif
	}
	strncat(buf, filename, bufsize-1);
}

Common::StringList DefaultSaveFileManager::listSavefiles(const char *regex) {
	FilesystemNode savePath(getSavePath());
	FSList savefiles;
	Common::StringList results;
	Common::String search(regex);

	if (savePath.lookupFile(savefiles, savePath, search, false, true)) {
		for (FSList::const_iterator file = savefiles.begin(); file != savefiles.end(); file++) {
			results.push_back(file->getPath());
		}
	}

	return results;
}

Common::InSaveFile *DefaultSaveFileManager::openForLoading(const char *filename) {
	char buf[256];
	join_paths(filename, getSavePath(), buf, sizeof(buf));

	StdioSaveFile *sf = new StdioSaveFile(buf, false);

	if (!sf->isOpen()) {
		delete sf;
		sf = 0;
	}
	return wrapInSaveFile(sf);
}

Common::OutSaveFile *DefaultSaveFileManager::openForSaving(const char *filename) {
	char buf[256];

	// Ensure that the savepath exists and is writeable. If not, generate
	// an appropriate error
	const char *savePath = getSavePath();

#if defined(UNIX) || defined(__SYMBIAN32__)
	struct stat sb;
	clearError();

	// Check whether the dir exists
	if (stat(savePath, &sb) == -1) {
		// The dir does not exist, or stat failed for some other reason.
		// If the problem was that the path pointed to nothing, try
		// to create the dir (ENOENT case).
		switch (errno) {
		case EACCES:
			setError(SFM_DIR_ACCESS, Common::String("Search or write permission denied"));
			break;
		case ELOOP:
			setError(SFM_DIR_LOOP, Common::String("Too many symbolic links encountered while traversing the path"));
			break;
		case ENAMETOOLONG:
			setError(SFM_DIR_NAMETOOLONG, Common::String("The path name is too long"));
			break;
		case ENOENT:
			if (mkdir(savePath, 0755) != 0) {
				// mkdir could fail for various reasons: The parent dir doesn't exist,
				// or is not writeable, the path could be completly bogus, etc.
				warning("mkdir for '%s' failed!", savePath);
				perror("mkdir");

				switch (errno) {
				case EACCES:
					setError(SFM_DIR_ACCESS, Common::String("Search or write permission denied"));
					break;
				case EMLINK:
					setError(SFM_DIR_LINKMAX, Common::String("The link count of the parent directory would exceed {LINK_MAX}"));
					break;
				case ELOOP:
					setError(SFM_DIR_LOOP, Common::String("Too many symbolic links encountered while traversing the path"));
					break;
				case ENAMETOOLONG:
					setError(SFM_DIR_NAMETOOLONG, Common::String("The path name is too long"));
					break;
				case ENOENT:
					setError(SFM_DIR_NOENT, Common::String("A component of the path path does not exist, or the path is an empty string"));
					break;
				case ENOTDIR:
					setError(SFM_DIR_NOTDIR, Common::String("A component of the path prefix is not a directory"));
					break;
				case EROFS:
					setError(SFM_DIR_ROFS, Common::String("The parent directory resides on a read-only file system"));
					break;
				}

				return 0;
			}
			break;
		case ENOTDIR:
			setError(SFM_DIR_NOTDIR, Common::String("A component of the path prefix is not a directory"));
			break;
		}
	} else {
		// So stat() succeeded. But is the path actually pointing to a directory?
		if (!S_ISDIR(sb.st_mode)) {
			setError(SFM_DIR_NOTDIR, Common::String("The given savepath is not a directory"));

			return 0;
		}
	}
#endif


	join_paths(filename, savePath, buf, sizeof(buf));

	StdioSaveFile *sf = new StdioSaveFile(buf, true);

	if (!sf->isOpen()) {
		delete sf;
		sf = 0;
	}

	return wrapOutSaveFile(sf);
}

bool DefaultSaveFileManager::removeSavefile(const char *filename) {
	Common::File file;
	FilesystemNode savePath(filename);
	return file.remove(savePath);
}

#endif // !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
