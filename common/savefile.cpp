/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/util.h"
#include "common/config-manager.h"
#include "common/savefile.h"

#include <stdio.h>
#include <string.h>

#ifdef USE_ZLIB
#include <zlib.h>
#endif

namespace Common {

const char *SaveFileManager::getSavePath() const {

#if defined(PALMOS_MODE) || defined(__PSP__)
	return SCUMMVM_SAVEPATH;
#else

	const char *dir = NULL;

	// Try to use game specific savepath from config
	dir = ConfMan.get("savepath").c_str();

	// Work around a bug (#999122) in the original 0.6.1 release of
	// ScummVM, which would insert a bad savepath value into config files.
	if (0 == strcmp(dir, "None")) {
		ConfMan.removeKey("savepath", ConfMan.getActiveDomain());
		ConfMan.flushToDisk();
		dir = ConfMan.get("savepath").c_str();
	}

#ifdef _WIN32_WCE
	if (dir[0] == 0)
		dir = ConfMan.get("path").c_str();
#endif

	assert(dir);

	return dir;
#endif
}

class StdioSaveFile : public SaveFile {
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
		return ::fread(dataPtr, 1, dataSize, fh);
	}
	uint32 write(const void *dataPtr, uint32 dataSize) {
		return ::fwrite(dataPtr, 1, dataSize, fh);
	}

	void skip(uint32 offset) {
		::fseek(fh, offset, SEEK_CUR);
	}
};


#ifdef USE_ZLIB
class GzipSaveFile : public SaveFile {
private:
	gzFile fh;
	bool _ioError;
public:
	GzipSaveFile(const char *filename, bool saveOrLoad) {
		_ioError = false;
		fh = ::gzopen(filename, (saveOrLoad? "wb" : "rb"));
	}
	~GzipSaveFile() {
		if (fh)
			::gzclose(fh);
	}

	bool eos() const { return gzeof(fh) != 0; }
	bool ioFailed() const { return _ioError; }
	void clearIOFailed() { _ioError = false; }

	bool isOpen() const { return fh != 0; }

	uint32 read(void *dataPtr, uint32 dataSize) {
		int ret = ::gzread(fh, dataPtr, dataSize);
		if (ret <= -1)
			_ioError = true;
		return ret;
	}
	uint32 write(const void *dataPtr, uint32 dataSize) {
		// Due to a "bug" in the zlib headers (or maybe I should say,
		// a bug in the C++ spec? Whatever <g>) we have to be a bit
		// hackish here and remove the const qualifier.
		// Note that gzwrite's buf param is declared as "const voidp"
		// which you might think is the same as "const void *" but it
		// is not - rather it is equal to "void const *" which is the
		// same as "void *". Hrmpf
		int ret = ::gzwrite(fh, const_cast<void *>(dataPtr), dataSize);
		if (ret <= 0)
			_ioError = true;
		return ret;
	}

	void skip(uint32 offset) {
		::gzseek(fh, offset, SEEK_CUR);
	}
};
#endif


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

#if !defined(__GP32__) && !defined(PALMOS_MODE)
		strncat(buf, "/", bufsize-1);	// prevent double /
#endif
	}
	strncat(buf, filename, bufsize-1);
}

OutSaveFile *DefaultSaveFileManager::openForSaving(const char *filename) {
	char buf[256];
	join_paths(filename, getSavePath(), buf, sizeof(buf));
	return makeSaveFile(buf, true);
}

InSaveFile *DefaultSaveFileManager::openForLoading(const char *filename) {
	char buf[256];
	join_paths(filename, getSavePath(), buf, sizeof(buf));
	return makeSaveFile(buf, false);
}

void DefaultSaveFileManager::listSavefiles(const char * /* prefix */, bool *marks, int num) {
	memset(marks, true, num * sizeof(bool));
}

SaveFile *DefaultSaveFileManager::makeSaveFile(const char *filename, bool saveOrLoad) {
#ifdef USE_ZLIB
	GzipSaveFile *sf = new GzipSaveFile(filename, saveOrLoad);
#else
	StdioSaveFile *sf = new StdioSaveFile(filename, saveOrLoad);
#endif
	if (!sf->isOpen()) {
		delete sf;
		sf = 0;
	}
	return sf;
}


} // End of namespace Common
