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

#include "common/file.h"
#include "common/fs.h"
#include "common/hashmap.h"
#include "common/util.h"
#include "common/hash-str.h"
#include <errno.h>

#if defined(MACOSX) || defined(IPHONE)
#include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	typedef unsigned long	uint64;
	typedef signed long	int64;
	#include "backends/platform/ps2/fileio.h"

	#define fopen(a, b)			ps2_fopen(a, b)
	#define fclose(a)			ps2_fclose(a)
	#define fseek(a, b, c)			ps2_fseek(a, b, c)
	#define ftell(a)			ps2_ftell(a)
	#define feof(a)				ps2_feof(a)
	#define fread(a, b, c, d)		ps2_fread(a, b, c, d)
	#define fwrite(a, b, c, d)		ps2_fwrite(a, b, c, d)

	//#define fprintf				ps2_fprintf	// used in common/util.cpp
	//#define fflush(a)			ps2_fflush(a)	// used in common/util.cpp

	//#define fgetc(a)			ps2_fgetc(a)	// not used
	//#define fgets(a, b, c)			ps2_fgets(a, b, c)	// not used
	//#define fputc(a, b)			ps2_fputc(a, b)	// not used
	//#define fputs(a, b)			ps2_fputs(a, b)	// not used

	//#define fsize(a)			ps2_fsize(a)	// not used -- and it is not a standard function either
#endif

#ifdef __DS__

	// These functions replease the standard library functions of the same name.
	// As this header is included after the standard one, I have the chance to #define
	// all of these to my own code.
	//
	// A #define is the only way, as redefinig the functions would cause linker errors.

	// These functions need to be #undef'ed, as their original definition
	// in devkitarm is done with #includes (ugh!)
	#undef feof
	#undef clearerr
	//#undef getc
	//#undef ferror

	#include "backends/fs/ds/ds-fs.h"


	//void	std_fprintf(FILE* handle, const char* fmt, ...);	// used in common/util.cpp
	//void	std_fflush(FILE* handle);	// used in common/util.cpp

	//char*	std_fgets(char* str, int size, FILE* file);	// not used
	//int	std_getc(FILE* handle);	// not used
	//char*	std_getcwd(char* dir, int dunno);	// not used
	//void	std_cwd(char* dir);	// not used
	//int	std_ferror(FILE* handle);	// not used

	// Only functions used in the ScummVM source have been defined here!
	#define fopen(name, mode)					DS::std_fopen(name, mode)
	#define fclose(handle)						DS::std_fclose(handle)
	#define fread(ptr, size, items, file)		DS::std_fread(ptr, size, items, file)
	#define fwrite(ptr, size, items, file)		DS::std_fwrite(ptr, size, items, file)
	#define feof(handle)						DS::std_feof(handle)
	#define ftell(handle)						DS::std_ftell(handle)
	#define fseek(handle, offset, whence)		DS::std_fseek(handle, offset, whence)
	#define clearerr(handle)					DS::std_clearerr(handle)

	//#define printf(fmt, ...)					consolePrintf(fmt, ##__VA_ARGS__)

	//#define fprintf(file, fmt, ...)				{ char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); }
	//#define fflush(file)						DS::std_fflush(file)	// used in common/util.cpp

	//#define fgets(str, size, file)				DS::std_fgets(str, size, file)	// not used
	//#define getc(handle)						DS::std_getc(handle)	// not used
	//#define getcwd(dir, dunno)					DS::std_getcwd(dir, dunno)	// not used
	#define ferror(handle)						DS::std_ferror(handle)

#endif

#ifdef __SYMBIAN32__
	#undef feof
	#undef clearerr

	#define FILE void

	FILE*	symbian_fopen(const char* name, const char* mode);
	void	symbian_fclose(FILE* handle);
	size_t	symbian_fread(const void* ptr, size_t size, size_t numItems, FILE* handle);
	size_t	symbian_fwrite(const void* ptr, size_t size, size_t numItems, FILE* handle);
	bool	symbian_feof(FILE* handle);
	long int symbian_ftell(FILE* handle);
	int		symbian_fseek(FILE* handle, long int offset, int whence);
	void	symbian_clearerr(FILE* handle);

	// Only functions used in the ScummVM source have been defined here!
	#define fopen(name, mode)					symbian_fopen(name, mode)
	#define fclose(handle)						symbian_fclose(handle)
	#define fread(ptr, size, items, file)		symbian_fread(ptr, size, items, file)
	#define fwrite(ptr, size, items, file)		symbian_fwrite(ptr, size, items, file)
	#define feof(handle)						symbian_feof(handle)
	#define ftell(handle)						symbian_ftell(handle)
	#define fseek(handle, offset, whence)		symbian_fseek(handle, offset, whence)
	#define clearerr(handle)					symbian_clearerr(handle)
#endif

namespace Common {

typedef HashMap<String, int> StringIntMap;

// The following two objects could be turned into static members of class
// File. However, then we would be forced to #include hashmap in file.h
// which seems to be a high price just for a simple beautification...
static StringIntMap *_defaultDirectories;
static StringMap *_filesMap;

static FILE *fopenNoCase(const String &filename, const String &directory, const char *mode) {
	FILE *file;
	String dirBuf(directory);
	String fileBuf(filename);

#if !defined(__GP32__) && !defined(PALMOS_MODE)
	// Add a trailing slash, if necessary.
	if (!dirBuf.empty()) {
		const char c = dirBuf.lastChar();
		if (c != ':' && c != '/' && c != '\\')
			dirBuf += '/';
	}
#endif

	// Append the filename to the path string
	String pathBuf(dirBuf);
	pathBuf += fileBuf;

	//
	// Try to open the file normally
	//
	file = fopen(pathBuf.c_str(), mode);

	//
	// Try again, with file name converted to upper case
	//
	if (!file) {
		fileBuf.toUppercase();
		pathBuf = dirBuf + fileBuf;
		file = fopen(pathBuf.c_str(), mode);
	}

	//
	// Try again, with file name converted to lower case
	//
	if (!file) {
		fileBuf.toLowercase();
		pathBuf = dirBuf + fileBuf;
		file = fopen(pathBuf.c_str(), mode);
	}

	//
	// Try again, with file name capitalized
	//
	if (!file) {
		fileBuf.toLowercase();
		fileBuf.setChar(toupper(fileBuf[0]),0);
		pathBuf = dirBuf + fileBuf;
		file = fopen(pathBuf.c_str(), mode);
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
	FilesystemNode dir(directory);
	addDefaultDirectoryRecursive(dir, 1);
}

void File::addDefaultDirectoryRecursive(const String &directory, int level, const String &prefix) {
	FilesystemNode dir(directory);
	addDefaultDirectoryRecursive(dir, level, prefix);
}

void File::addDefaultDirectory(const FilesystemNode &directory) {
	addDefaultDirectoryRecursive(directory, 1);
}

void File::addDefaultDirectoryRecursive(const FilesystemNode &dir, int level, const String &prefix) {
	if (level <= 0)
		return;

	FSList fslist;
	if (!dir.getChildren(fslist, FilesystemNode::kListAll)) {
		// Failed listing the contents of this node, so it is either not a
		// directory, or just doesn't exist at all.
		return;
	}

	if (!_defaultDirectories)
		_defaultDirectories = new StringIntMap;

	// Do not add directories multiple times, unless this time they are added
	// with a bigger depth.
	const String &directory(dir.getPath());
	if (_defaultDirectories->contains(directory) && (*_defaultDirectories)[directory] >= level)
		return;
	(*_defaultDirectories)[directory] = level;

	if (!_filesMap)
		_filesMap = new StringMap;

	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			addDefaultDirectoryRecursive(file->getPath(), level - 1, prefix + file->getName() + "/");
		} else {
			String lfn(prefix);
			lfn += file->getName();
			lfn.toLowercase();
			if (!_filesMap->contains(lfn)) {
				(*_filesMap)[lfn] = file->getPath();
			}
		}
	}
}

void File::resetDefaultDirectories() {
	delete _defaultDirectories;
	delete _filesMap;

	_defaultDirectories = 0;
	_filesMap = 0;
}

File::File()
	: _handle(0), _ioFailed(false) {
}

File::~File() {
	close();
}


bool File::open(const String &filename) {
	assert(!filename.empty());
	assert(!_handle);

	_name.clear();
	clearIOFailed();

	String fname(filename);
	fname.toLowercase();

	if (_filesMap && _filesMap->contains(fname)) {
		fname = (*_filesMap)[fname];
		debug(3, "Opening hashed: %s", fname.c_str());
		_handle = fopen(fname.c_str(), "rb");
	} else if (_filesMap && _filesMap->contains(fname + ".")) {
		// WORKAROUND: Bug #1458388: "SIMON1: Game Detection fails"
		// sometimes instead of "GAMEPC" we get "GAMEPC." (note trailing dot)
		fname = (*_filesMap)[fname + "."];
		debug(3, "Opening hashed: %s", fname.c_str());
		_handle = fopen(fname.c_str(), "rb");
	} else {

		if (_defaultDirectories) {
			// Try all default directories
			StringIntMap::const_iterator x(_defaultDirectories->begin());
			for (; _handle == NULL && x != _defaultDirectories->end(); ++x) {
				_handle = fopenNoCase(filename, x->_key, "rb");
			}
		}

		// Last resort: try the current directory
		if (_handle == NULL)
			_handle = fopenNoCase(filename, "", "rb");

		// Last last (really) resort: try looking inside the application bundle on Mac OS X for the lowercase file.
#if defined(MACOSX) || defined(IPHONE)
		if (!_handle) {
			CFStringRef cfFileName = CFStringCreateWithBytes(NULL, (const UInt8 *)filename.c_str(), filename.size(), kCFStringEncodingASCII, false);
			CFURLRef fileUrl = CFBundleCopyResourceURL(CFBundleGetMainBundle(), cfFileName, NULL, NULL);
			if (fileUrl) {
				UInt8 buf[256];
				if (CFURLGetFileSystemRepresentation(fileUrl, false, (UInt8 *)buf, 256)) {
					_handle = fopen((char *)buf, "rb");
				}
				CFRelease(fileUrl);
			}
			CFRelease(cfFileName);
		}
#endif

	}

	if (_handle == NULL)
		debug(2, "File %s not opened", filename.c_str());
	else
		_name = filename;

	return _handle != NULL;
}

bool File::open(const FilesystemNode &node) {

	if (!node.exists()) {
		warning("File::open: Trying to open a FilesystemNode which does not exist");
		return false;
	} else if (node.isDirectory()) {
		warning("File::open: Trying to open a FilesystemNode which is a directory");
		return false;
	} /*else if (!node.isReadable() && mode == kFileReadMode) {
		warning("File::open: Trying to open an unreadable FilesystemNode object for reading");
		return false;
	} else if (!node.isWritable() && mode == kFileWriteMode) {
		warning("File::open: Trying to open an unwritable FilesystemNode object for writing");
		return false;
	}*/

	String filename(node.getName());

	if (_handle) {
		error("File::open: This file object already is opened (%s), won't open '%s'", _name.c_str(), filename.c_str());
	}

	clearIOFailed();
	_name.clear();

	_handle = fopen(node.getPath().c_str(), "rb");

	if (_handle == NULL)
		debug(2, "File %s not found", filename.c_str());
	else
		_name = filename;

	return _handle != NULL;
}

bool File::exists(const String &filename) {
	// First try to find the file via a FilesystemNode (in case an absolute
	// path was passed). This is only used to filter out directories.
	FilesystemNode file(filename);
	if (file.exists())
		return !file.isDirectory();

	// See if the file is already mapped
	if (_filesMap && _filesMap->contains(filename)) {
		FilesystemNode file2((*_filesMap)[filename]);

		if (file2.exists())
			return !file2.isDirectory();
	}

	// Try all default directories
	if (_defaultDirectories) {
		StringIntMap::const_iterator i(_defaultDirectories->begin());
		for (; i != _defaultDirectories->end(); ++i) {
			FilesystemNode file2(i->_key + filename);

			if(file2.exists())
				return !file2.isDirectory();
		}
	}

	//Try opening the file inside the local directory as a last resort
	File tmp;
	return tmp.open(filename);
}

void File::close() {
	if (_handle)
		fclose((FILE *)_handle);
	_handle = NULL;
}

bool File::isOpen() const {
	return _handle != NULL;
}

bool File::ioFailed() const {
	// TODO/FIXME: Just use ferror() here?
	return _ioFailed != 0;
}

void File::clearIOFailed() {
	// TODO/FIXME: Just use clearerr() here?
	_ioFailed = false;
}

bool File::eof() const {
	assert(_handle);

	return feof((FILE *)_handle) != 0;
}

uint32 File::pos() const {
	assert(_handle);

	return ftell((FILE *)_handle);
}

uint32 File::size() const {
	assert(_handle);

	uint32 oldPos = ftell((FILE *)_handle);
	fseek((FILE *)_handle, 0, SEEK_END);
	uint32 length = ftell((FILE *)_handle);
	fseek((FILE *)_handle, oldPos, SEEK_SET);

	return length;
}

void File::seek(int32 offs, int whence) {
	assert(_handle);

	if (fseek((FILE *)_handle, offs, whence) != 0)
		clearerr((FILE *)_handle);
}

uint32 File::read(void *ptr, uint32 len) {
	byte *ptr2 = (byte *)ptr;
	uint32 real_len;

	assert(_handle);

	if (len == 0)
		return 0;

	real_len = fread(ptr2, 1, len, (FILE *)_handle);
	if (real_len < len) {
		_ioFailed = true;
	}

	return real_len;
}


DumpFile::DumpFile() : _handle(0) {
}

DumpFile::~DumpFile() {
	close();
}

bool DumpFile::open(const String &filename) {
	assert(!filename.empty());
	assert(!_handle);

	String fname(filename);
	fname.toLowercase();
	
	_handle = fopenNoCase(filename, "", "wb");

	if (_handle == NULL)
		debug(2, "Failed to open '%s' for writing", filename.c_str());

	return _handle != NULL;
}

bool DumpFile::open(const FilesystemNode &node) {
	assert(!_handle);

	if (node.isDirectory()) {
		warning("File::open: Trying to open a FilesystemNode which is a directory");
		return false;
	} /*else if (!node.isReadable() && mode == kFileReadMode) {
		warning("File::open: Trying to open an unreadable FilesystemNode object for reading");
		return false;
	} else if (!node.isWritable() && mode == kFileWriteMode) {
		warning("File::open: Trying to open an unwritable FilesystemNode object for writing");
		return false;
	}*/

	_handle = fopen(node.getPath().c_str(), "wb");

	if (_handle == NULL)
		debug(2, "File %s not found", node.getName().c_str());

	return _handle != NULL;
}

void DumpFile::close() {
	if (_handle)
		fclose((FILE *)_handle);
	_handle = NULL;
}

bool DumpFile::isOpen() const {
	return _handle != NULL;
}

bool DumpFile::ioFailed() const {
	assert(_handle);
	return ferror((FILE *)_handle) != 0;
}

void DumpFile::clearIOFailed() {
	assert(_handle);
	clearerr((FILE *)_handle);
}

bool DumpFile::eof() const {
	assert(_handle);
	return feof((FILE *)_handle) != 0;
}

uint32 DumpFile::write(const void *ptr, uint32 len) {
	assert(_handle);

	if (len == 0)
		return 0;

	return (uint32)fwrite(ptr, 1, len, (FILE *)_handle);
}

void DumpFile::flush() {
	assert(_handle);
	// TODO: Should check the return value of fflush, and if it is non-zero,
	// check errno and set an error flag.
	fflush((FILE *)_handle);
}


StdioStream::StdioStream(void *handle) : _handle(handle) {
	assert(handle);
}

StdioStream::~StdioStream() {
	fclose((FILE *)_handle);
}

bool StdioStream::ioFailed() const {
	return ferror((FILE *)_handle) != 0;
}

void StdioStream::clearIOFailed() {
	clearerr((FILE *)_handle);
}

bool StdioStream::eos() const {
	return feof((FILE *)_handle) != 0;
}

uint32 StdioStream::pos() const {
	// FIXME: ftell can return -1 to indicate an error (in which case errno gets set)
	//        Maybe we should support that, too?
	return ftell((FILE *)_handle);
}

uint32 StdioStream::size() const {
	uint32 oldPos = ftell((FILE *)_handle);
	fseek((FILE *)_handle, 0, SEEK_END);
	uint32 length = ftell((FILE *)_handle);
	fseek((FILE *)_handle, oldPos, SEEK_SET);

	return length;
}

void StdioStream::seek(int32 offs, int whence) {
	assert(_handle);

	if (fseek((FILE *)_handle, offs, whence) != 0)
		clearerr((FILE *)_handle);	// FIXME: why do we call clearerr here?
		
	// FIXME: fseek has a return value to indicate errors; 
	//        Maybe we should support that, too?
}

uint32 StdioStream::read(void *ptr, uint32 len) {
	return (uint32)fread((byte *)ptr, 1, len, (FILE *)_handle);
}

uint32 StdioStream::write(const void *ptr, uint32 len) {
	return (uint32)fwrite(ptr, 1, len, (FILE *)_handle);
}

void StdioStream::flush() {
	// TODO: Should check the return value of fflush, and if it is non-zero,
	// check errno and set an error flag.
	fflush((FILE *)_handle);
}




}	// End of namespace Common
