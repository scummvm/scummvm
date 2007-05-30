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

#include "common/stdafx.h"
#include "common/savefile.h"
#include "common/util.h"
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

Common::OutSaveFile *DefaultSaveFileManager::openForSaving(const char *filename) {
	char buf[256];

	// Ensure that the savepath exists and is writeable. If not, generate
	// an appropriate error
	const char *savePath = getSavePath();
#if defined(UNIX) || defined(__SYMBIAN32__)
	struct stat sb;
	
	// Check whether the dir exists
	if (stat(savePath, &sb) == -1) {
		// The dir does not exist, or stat failed for some other reason.
		// If the problem was that the path pointed to nothing, try
		// to create the dir.
		if (errno == ENOENT) {
			if (mkdir(savePath, 0755) != 0) {
				// mkdir could fail for various reasons: The parent dir doesn't exist,
				// or is not writeable, the path could be completly bogus, etc.
				warning("mkdir for '%s' failed!", savePath);
				perror("mkdir");
				// TODO: Specify an error code here so that callers can 
				// determine what exactly went wrong.
				return 0;
			}
		} else {
			// Unknown error, abort.
			// TODO: Specify an error code here so that callers can 
			// determine what exactly went wrong.
			return 0;
		}
	} else {
		// So stat() succeeded. But is the path actually pointing to a
		// directory?
		if (!S_ISDIR(sb.st_mode)) {
			// TODO: Specify an error code here so that callers can 
			// determine what exactly went wrong.
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

void DefaultSaveFileManager::listSavefiles(const char * /* prefix */, bool *marks, int num) {
	// TODO: Implement this properly, at least on systems that support
	// opendir/readdir.
	// Even better, replace this with a better design...
	memset(marks, true, num * sizeof(bool));
}

#endif // !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
