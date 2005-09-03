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

#include "common/file.h"
#include "common/util.h"

namespace Common {

StringList File::_defaultDirectories;


static FILE *fopenNoCase(const char *filename, const char *directory, const char *mode) {
	FILE *file;
	char buf[512];
	char *ptr;

	assert(directory);
	strcpy(buf, directory);

#if !defined(__GP32__) && !defined(PALMOS_MODE)
	// Add a trailing slash, if necessary.
	if (buf[0] != 0) {
		const int dirLen = strlen(buf);
		if (buf[dirLen-1] != ':' && buf[dirLen-1] != '/' && buf[dirLen-1] != '\\')
			strcat(buf, "/");
	}
#endif

	// Append the filename to the path string
	const int offsetToFileName = strlen(buf);
	strcat(buf, filename);

	//
	// Try to open the file normally
	//
	file = fopen(buf, mode);

	//
	// Try again, with file name converted to upper case
	//
	if (!file) {
		ptr = buf + offsetToFileName;
		while (*ptr) {
			*ptr = toupper(*ptr);
			ptr++;
		}
		file = fopen(buf, mode);
	}

	//
	// Try again, with file name converted to lower case
	//
	if (!file) {
		ptr = buf + offsetToFileName;
		while (*ptr) {
			*ptr = tolower(*ptr);
			ptr++;
		}
		file = fopen(buf, mode);
	}

	//
	// Try again, with file name capitalized
	//
	if (!file) {
		ptr = buf + offsetToFileName;
		*ptr = toupper(*ptr);
		file = fopen(buf, mode);
	}

#ifdef __amigaos4__
	//
	// Work around for possibility that someone uses AmigaOS "newlib" build with SmartFileSystem (blocksize 512 bytes), leading
	// to buffer size being only 512 bytes. "Clib2" sets the buffer size to 8KB, resulting smooth movie playback. This forces the buffer
	// to be enough also when using "newlib" compile on SFS.
	//
	if (file) {
		setvbuf(file, NULL, _IOFBF, 8192);
	}
#endif

	return file;
}

void File::addDefaultDirectory(const String &directory) {
	_defaultDirectories.push_back(directory);
}

void File::resetDefaultDirectories() {
	_defaultDirectories.clear();
}

File::File()
 : _handle(0), _ioFailed(false), _refcount(1) {
}

//#define DEBUG_FILE_REFCOUNT

File::~File() {
#ifdef DEBUG_FILE_REFCOUNT
	warning("File::~File on file '%s'", _name.c_str());
#endif
	close();
}
void File::incRef() {
#ifdef DEBUG_FILE_REFCOUNT
	warning("File::incRef on file '%s'", _name.c_str());
#endif
	_refcount++;
}

void File::decRef() {
#ifdef DEBUG_FILE_REFCOUNT
	warning("File::decRef on file '%s'", _name.c_str());
#endif
	if (--_refcount == 0) {
		delete this;
	}
}


bool File::open(const char *filename, AccessMode mode, const char *directory) {
	assert(mode == kFileReadMode || mode == kFileWriteMode);

	if (_handle) {
		error("File::open: This file object already is opened (%s), won't open '%s'", _name.c_str(), filename);
	}

	if (filename == NULL || *filename == 0) {
		error("File::open: No filename was specified!");
	}

	clearIOFailed();

	const char *modeStr = (mode == kFileReadMode) ? "rb" : "wb";
	if (mode == kFileWriteMode || directory) {
		_handle = fopenNoCase(filename, directory ? directory : "", modeStr);
	} else {
		StringList::const_iterator x;
		// Try all default directories
		for (x = _defaultDirectories.begin(); _handle == NULL && x != _defaultDirectories.end(); ++x) {
			_handle = fopenNoCase(filename, x->c_str(), modeStr);
		}
		// Last resort: try the current directory
		if (_handle == NULL)
			_handle = fopenNoCase(filename, "", modeStr);
	}

	if (_handle == NULL) {
		if (mode == kFileReadMode)
			debug(2, "File %s not found", filename);
		else
			debug(2, "File %s not opened", filename);
		return false;
	}


	_name = filename;

#ifdef DEBUG_FILE_REFCOUNT
	warning("File::open on file '%s'", _name.c_str());
#endif

	return true;
}

bool File::exists(const char *filename, const char *directory) {
	// FIXME: Ugly ugly hack!
	File tmp;
	return tmp.open(filename, kFileReadMode, directory);
}

void File::close() {
	if (_handle)
		fclose(_handle);
	_handle = NULL;
}

bool File::isOpen() const {
	return _handle != NULL;
}

bool File::ioFailed() const {
	return _ioFailed != 0;
}

void File::clearIOFailed() {
	_ioFailed = false;
}

bool File::eof() const {
	if (_handle == NULL) {
		error("File::eof: File is not open!");
		return false;
	}

	return feof(_handle) != 0;
}

uint32 File::pos() const {
	if (_handle == NULL) {
		error("File::pos: File is not open!");
		return 0;
	}

	return ftell(_handle);
}

uint32 File::size() const {
	if (_handle == NULL) {
		error("File::size: File is not open!");
		return 0;
	}

	uint32 oldPos = ftell(_handle);
	fseek(_handle, 0, SEEK_END);
	uint32 length = ftell(_handle);
	fseek(_handle, oldPos, SEEK_SET);

	return length;
}

void File::seek(int32 offs, int whence) {
	if (_handle == NULL) {
		error("File::seek: File is not open!");
		return;
	}

	if (fseek(_handle, offs, whence) != 0)
		clearerr(_handle);
}

uint32 File::read(void *ptr, uint32 len) {
	byte *ptr2 = (byte *)ptr;
	uint32 real_len;

	if (_handle == NULL) {
		error("File::read: File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	real_len = fread(ptr2, 1, len, _handle);
	if (real_len < len) {
		_ioFailed = true;
	}

	return real_len;
}

uint32 File::write(const void *ptr, uint32 len) {
	if (_handle == NULL) {
		error("File::write: File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	if ((uint32)fwrite(ptr, 1, len, _handle) != len) {
		_ioFailed = true;
	}

	return len;
}

}	// End of namespace Common
