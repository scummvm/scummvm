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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/common/file.cpp $
 * $Id: file.cpp 28150 2007-07-20 19:42:38Z david_corrales $
 *
 */

#include "backends/file/base-file.h"
#include "common/fs.h"
#include "common/hashmap.h"
#include "common/util.h"
#include "common/hash-str.h"

#if defined(UNIX) || defined(__SYMBIAN32__)
#include <errno.h>
#endif

#ifdef MACOSX
#include "CoreFoundation/CoreFoundation.h"
#endif

BaseFile::BaseFile() {
	_handle = 0;
	_ioFailed = false;
}

//#define DEBUG_FILE_REFCOUNT

BaseFile::~BaseFile() {
#ifdef DEBUG_FILE_REFCOUNT
	warning("File::~File on file '%s'", _name.c_str());
#endif
	close();
}

bool BaseFile::open(const String &filename, AccessMode mode) {
	assert(mode == kFileReadMode || mode == kFileWriteMode);

	if (filename.empty()) {
		error("File::open: No filename was specified");
	}

	if (_handle) {
		error("File::open: This file object already is opened (%s), won't open '%s'", _name.c_str(), filename.c_str());
	}

	_name.clear();
	clearIOFailed();

	const char *modeStr = (mode == kFileReadMode) ? "rb" : "wb";
	_handle = _fopen(filename.c_str(), modeStr);
	
	if (_handle == NULL) {
		if (mode == kFileReadMode)
			debug(2, "File %s not found", filename.c_str());
		else
			debug(2, "File %s not opened", filename.c_str());
		return false;
	}
	
	_name = filename;

#ifdef DEBUG_FILE_REFCOUNT
	warning("File::open on file '%s'", _name.c_str());
#endif

	return true;
}

bool BaseFile::remove(const String &filename){
	if (remove(filename.c_str()) != 0) {
		if(errno == EACCES)
			;//TODO: read-only file
		if(errno == ENOENT)
			;//TODO: non-existent file
		
		return false;
	} else {
		return true;
	}
}

void BaseFile::close() {
	if (_handle)
		_fclose((FILE *)_handle);
	_handle = NULL;
}

bool BaseFile::isOpen() const {
	return _handle != NULL;
}

bool BaseFile::ioFailed() const {
	return _ioFailed != 0;
}

void BaseFile::clearIOFailed() {
	_ioFailed = false;
}

bool BaseFile::eof() const {
	if (_handle == NULL) {
		error("File::eof: File is not open!");
		return false;
	}

	return _feof((FILE *)_handle) != 0;
}

uint32 BaseFile::pos() const {
	if (_handle == NULL) {
		error("File::pos: File is not open!");
		return 0;
	}

	return _ftell((FILE *)_handle);
}

uint32 BaseFile::size() const {
	if (_handle == NULL) {
		error("File::size: File is not open!");
		return 0;
	}

	uint32 oldPos = _ftell((FILE *)_handle);
	_fseek((FILE *)_handle, 0, SEEK_END);
	uint32 length = _ftell((FILE *)_handle);
	_fseek((FILE *)_handle, oldPos, SEEK_SET);

	return length;
}

void BaseFile::seek(int32 offs, int whence) {
	if (_handle == NULL) {
		error("File::seek: File is not open!");
		return;
	}

	if (_fseek((FILE *)_handle, offs, whence) != 0)
		_clearerr((FILE *)_handle);
}

uint32 BaseFile::read(void *ptr, uint32 len) {
	byte *ptr2 = (byte *)ptr;
	uint32 real_len;

	if (_handle == NULL) {
		error("File::read: File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	real_len = _fread(ptr2, 1, len, (FILE *)_handle);
	if (real_len < len) {
		_ioFailed = true;
	}

	return real_len;
}

/*uint32 File::write(const void *ptr, uint32 len) {
	if (_handle == NULL) {
		error("File::write: File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	if ((uint32)_fwrite(ptr, 1, len, (FILE *)_handle) != len) {
		_ioFailed = true;
	}

	return len;
}*/

void BaseFile::_clearerr(FILE *stream) { 
	clearerr(stream);
}

int BaseFile::_fclose(FILE *stream) {
	return fclose(stream);
}

int BaseFile::_feof(FILE *stream) const {
	return feof(stream);
}
FILE *BaseFile::_fopen(const char * filename, const char * mode) {
	return fopen(filename, mode);
}

int BaseFile::_fread(void *buffer, size_t obj_size, size_t num, FILE *stream) {
	return fread(buffer, obj_size, num, stream);
}

int BaseFile::_fseek(FILE * stream, long int offset, int origin) const {
	return fseek(stream, offset, origin);
}

long BaseFile::_ftell(FILE *stream) const {
	return ftell(stream);
}

int BaseFile::_fwrite(const void * ptr, size_t obj_size, size_t count, FILE * stream) {
	return fwrite(ptr, obj_size, count, stream);
}
