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

#include "common/archive.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/util.h"
#include "common/system.h"

namespace Common {

void File::addDefaultDirectory(const String &directory) {
	FilesystemNode dir(directory);
	addDefaultDirectoryRecursive(dir, 1);
}

void File::addDefaultDirectoryRecursive(const String &directory, int level) {
	FilesystemNode dir(directory);
	addDefaultDirectoryRecursive(dir, level);
}

void File::addDefaultDirectory(const FilesystemNode &directory) {
	addDefaultDirectoryRecursive(directory, 1);
}

void File::addDefaultDirectoryRecursive(const FilesystemNode &dir, int level) {
	if (level <= 0 || !dir.exists() || !dir.isDirectory())
		return;

	Common::ArchivePtr dataArchive(new Common::FSDirectory(dir, level));
	SearchMan.add(dir.getPath(), dataArchive);
}

void File::resetDefaultDirectories() {
	SearchMan.clear();
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

	if (SearchMan.hasFile(filename)) {
		debug(3, "Opening hashed: %s", filename.c_str());
		_handle = SearchMan.openFile(filename);
	} else if (SearchMan.hasFile(filename + ".")) {
		// WORKAROUND: Bug #1458388: "SIMON1: Game Detection fails"
		// sometimes instead of "GAMEPC" we get "GAMEPC." (note trailing dot)
		debug(3, "Opening hashed: %s.", filename.c_str());
		_handle = SearchMan.openFile(filename);
	} else {
		// Last resort: try the current directory
		FilesystemNode file(filename);
		if (file.exists() && !file.isDirectory())
			_handle = file.openForReading();
	}
	
	if (_handle == NULL)
		debug(2, "File %s not opened", filename.c_str());
	else
		_name = filename;

	return _handle != NULL;
}

bool File::open(const FilesystemNode &node) {

	if (!node.exists()) {
		warning("File::open: FilesystemNode does not exist");
		return false;
	} else if (node.isDirectory()) {
		warning("File::open: FilesystemNode is a directory");
		return false;
	}

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
	if (SearchMan.hasFile(filename)) {
		return true;
	} else if (SearchMan.hasFile(filename + ".")) {
		// WORKAROUND: Bug #1458388: "SIMON1: Game Detection fails"
		// sometimes instead of "GAMEPC" we get "GAMEPC." (note trailing dot)
		return true;
	} else {
		// Last resort: try the current directory
		FilesystemNode file(filename);
		if (file.exists() && !file.isDirectory())
			return true;
	}
	
	return false;
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

bool File::err() const {
	assert(_handle);
	return _handle->err();
}

void File::clearErr() {
	assert(_handle);
	_handle->clearErr();
}

bool File::eos() const {
	assert(_handle);
	return _handle->eos();
}

int32 File::pos() const {
	assert(_handle);
	return _handle->pos();
}

int32 File::size() const {
	assert(_handle);
	return _handle->size();
}

bool File::seek(int32 offs, int whence) {
	assert(_handle);
	return _handle->seek(offs, whence);
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

	FilesystemNode node(filename);
	return open(node);
}

bool DumpFile::open(const FilesystemNode &node) {
	assert(!_handle);

	if (node.isDirectory()) {
		warning("DumpFile::open: FilesystemNode is a directory");
		return false;
	}

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

bool DumpFile::err() const {
	assert(_handle);
	return _handle->ioFailed();
}

void DumpFile::clearErr() {
	assert(_handle);
	_handle->clearIOFailed();
}

uint32 DumpFile::write(const void *ptr, uint32 len) {
	assert(_handle);
	return _handle->write(ptr, len);
}

bool DumpFile::flush() {
	assert(_handle);
	return _handle->flush();
}

}	// End of namespace Common
