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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/filesystem.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

const char Path::delim = '/';

#if !defined(S_IFDIR)
#define S_IFDIR _S_IFDIR
#endif

/**
 * Creates a path to a directory or file
 */
Path::Path(const Common::String &p) : path(p) {
	unsigned int pos;
	bool _exists = false, isDir = false;

	/* determine if the path really exists */
	Common::FSNode node(path);
	_exists = node.exists();

	/* if so, let's glean more information */
	if (_exists)
		isDir = node.isDirectory();

	/* If it's for sure a file, get file information! */
	if (_exists && !isDir) {
		file = node.getName();

		if ((pos = file.findLastOf(".")) < file.size()) {
			ext = file.substr(pos + 1);
			file = file.substr(0, pos);
		}
	}
}

/**
 * Returns true if the path exists in the filesystem
 */
bool Path::exists() const {
	return Common::FSNode(path).exists();
}

/**
 * Returns true if the path points to a file
 */
bool Path::isFile() const {
	Common::FSNode node(path);
	return node.exists() && !node.isDirectory();
}

/**
 * Returns true if the path points to a directory
 */
bool Path::isDir() const {
	Common::FSNode node(path);
	return node.exists() && node.isDirectory();
}

/**
 * Returns the directory indicated in the path.
 */
Common::String Path::getDir() const {
	Common::FSNode node(path);
	return !node.exists() || node.isDirectory() ? node.getPath() : node.getParent().getPath();
}

/** Returns the full filename of the file designated in the path */
Common::String Path::getFilename() const        {
	return (ext.empty()) ? file : file + Common::String(".") + ext;
}
Common::String Path::getBaseFilename() const    {
	return file;    /**< Returns the base filename of the file */
}
Common::String Path::getExt() const             {
	return ext;    /**< Returns the extension of the file (if it exists) */
}

/**
 * Returns true if the given path exists in the filesystem
 */
bool Path::exists(const Common::String &path) {
	return Common::FSNode(path).exists();
}

/**
 * Opens a file and attempts to create the directory structure beneath it before opening the file.
 */
Common::SeekableReadStream *FileSystem::openForReading(const Common::String &filepath) {
	Common::File *f = new Common::File();
	if (f->open(filepath)) {
		return f;
	} else {
		delete f;
		return nullptr;
	}
}

Common::WriteStream *FileSystem::openForWriting(const Common::String &filepath) {
	return g_system->getSavefileManager()->openForSaving(filepath);
}


/**
 * Create the directory that composes the path.
 * If any directories that make up the path do not exist,
 * they are created.
 */
void FileSystem::createDirectory(Path &path) {
	Common::FSNode node(path.getPath());
	node.createDirectory();
}

/**
 * Create a directory that composes the path
 */
void FileSystem::createDirectory(const Common::String &filepath) {
	Path path(filepath);
	createDirectory(path);
}

} // End of namespace Ultima4
} // End of namespace Ultima
