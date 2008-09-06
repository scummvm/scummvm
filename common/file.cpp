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

#include "backends/fs/stdiostream.h"

#if defined(MACOSX) || defined(IPHONE)
#include "CoreFoundation/CoreFoundation.h"
#endif

namespace Common {

typedef HashMap<String, int> StringIntMap;

// The following two objects could be turned into static members of class
// File. However, then we would be forced to #include hashmap in file.h
// which seems to be a high price just for a simple beautification...
static StringIntMap *_defaultDirectories;
static StringMap *_filesMap;

static SeekableReadStream *fopenNoCase(const String &filename, const String &directory) {
	SeekableReadStream *handle;
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
	handle = StdioStream::makeFromPath(pathBuf, false);

	//
	// Try again, with file name converted to upper case
	//
	if (!handle) {
		fileBuf.toUppercase();
		pathBuf = dirBuf + fileBuf;
		handle = StdioStream::makeFromPath(pathBuf, false);
	}

	//
	// Try again, with file name converted to lower case
	//
	if (!handle) {
		fileBuf.toLowercase();
		pathBuf = dirBuf + fileBuf;
		handle = StdioStream::makeFromPath(pathBuf, false);
	}

	//
	// Try again, with file name capitalized
	//
	if (!handle) {
		fileBuf.toLowercase();
		fileBuf.setChar(toupper(fileBuf[0]),0);
		pathBuf = dirBuf + fileBuf;
		handle = StdioStream::makeFromPath(pathBuf, false);
	}

	return handle;
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
	: _handle(0) {
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
		_handle = StdioStream::makeFromPath(fname, false);
	} else if (_filesMap && _filesMap->contains(fname + ".")) {
		// WORKAROUND: Bug #1458388: "SIMON1: Game Detection fails"
		// sometimes instead of "GAMEPC" we get "GAMEPC." (note trailing dot)
		fname = (*_filesMap)[fname + "."];
		debug(3, "Opening hashed: %s", fname.c_str());
		_handle = StdioStream::makeFromPath(fname, false);
	} else {

		if (_defaultDirectories) {
			// Try all default directories
			StringIntMap::const_iterator x(_defaultDirectories->begin());
			for (; _handle == NULL && x != _defaultDirectories->end(); ++x) {
				_handle = fopenNoCase(filename, x->_key);
			}
		}

		// Last resort: try the current directory
		if (_handle == NULL)
			_handle = fopenNoCase(filename, "");

		// Last last (really) resort: try looking inside the application bundle on Mac OS X for the lowercase file.
#if defined(MACOSX) || defined(IPHONE)
		if (!_handle) {
			CFStringRef cfFileName = CFStringCreateWithBytes(NULL, (const UInt8 *)filename.c_str(), filename.size(), kCFStringEncodingASCII, false);
			CFURLRef fileUrl = CFBundleCopyResourceURL(CFBundleGetMainBundle(), cfFileName, NULL, NULL);
			if (fileUrl) {
				UInt8 buf[256];
				if (CFURLGetFileSystemRepresentation(fileUrl, false, (UInt8 *)buf, 256)) {
					_handle = StdioStream::makeFromPath((char *)buf, false);
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

	_handle = node.openForReading();

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
	delete _handle;
	_handle = NULL;
}

bool File::isOpen() const {
	return _handle != NULL;
}

bool File::ioFailed() const {
	// TODO/FIXME: Just use ferror() here?
	return !_handle || _handle->ioFailed();
}

void File::clearIOFailed() {
	if (_handle)
		_handle->clearIOFailed();
}

bool File::eos() const {
	assert(_handle);
	return _handle->eos();
}

uint32 File::pos() const {
	assert(_handle);
	return _handle->pos();
}

uint32 File::size() const {
	assert(_handle);
	return _handle->size();
}

void File::seek(int32 offs, int whence) {
	assert(_handle);
	_handle->seek(offs, whence);
}

uint32 File::read(void *ptr, uint32 len) {
	assert(_handle);
	return _handle->read(ptr, len);
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
	
	_handle = StdioStream::makeFromPath(filename, true);

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

	_handle = node.openForWriting();

	if (_handle == NULL)
		debug(2, "File %s not found", node.getName().c_str());

	return _handle != NULL;
}

void DumpFile::close() {
	delete _handle;
	_handle = NULL;
}

bool DumpFile::isOpen() const {
	return _handle != NULL;
}

bool DumpFile::ioFailed() const {
	assert(_handle);
	return _handle->ioFailed();
}

void DumpFile::clearIOFailed() {
	assert(_handle);
	_handle->clearIOFailed();
}

uint32 DumpFile::write(const void *ptr, uint32 len) {
	assert(_handle);
	return _handle->write(ptr, len);
}

void DumpFile::flush() {
	assert(_handle);
	_handle->flush();
}

}	// End of namespace Common
