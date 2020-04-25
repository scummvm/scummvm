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

#ifndef ULTIMA4_FILESYS_FILESYSTEM_H
#define ULTIMA4_FILESYS_FILESYSTEM_H

#include "common/list.h"
#include "common/str.h"
#include "common/stream.h"

namespace Ultima {
namespace Ultima4 {

#define FS_POSIX

/**
 * Provides cross-platform functionality for representing and validating paths.
 */
class Path {
public:
	/**
	 * Creates a path to a directory or file
	 */
	Path(const Common::String &p);

	/**
	 * Returns true if the path exists in the filesystem
	 */
	bool exists() const;

	/**
	 * Returns true if the path points to a file
	 */
	bool isFile() const;

	/**
	 * Returns true if the path points to a directory
	 */
	bool isDir() const;

	/**
	 * Returns the full translated path
	 */
	Common::String getPath() const {
		return path;
	}

	/**
	 * Returns the list of directories for the path
	 */
	Common::List<Common::String> *getDirTree() {
		return &dirs;
	}

	/**
	 * Returns the directory indicated in the path.
	 */
	Common::String getDir() const;

	/**
	 * Returns the full filename of the file designated in the path
	 */
	Common::String getFilename() const;

	/**
	 * Returns the base filename of the file
	 */
	Common::String getBaseFilename() const;

	/**
	 * Returns the extension of the file (if it exists)
	 */
	Common::String getExt() const;

	/**
	 * Returns true if the given path exists in the filesystem
	 */
	static bool exists(const Common::String &path);

	// Properties
	static const char delim;

private:
	Common::String path;
	Common::List<Common::String> dirs;
	Common::String file, ext;
};

/**
 * Provides cross-platform functionality for file and directory operations.
 * It currently only supports directory creation, but other operations
 * will be added as support is needed.
 */
class FileSystem {
public:
	/**
	 * Opens a file for reading
	 */
	static Common::SeekableReadStream *openForReading(const Common::String &filepath);

	/**
	 * Opens a file for writing in the save folder
	 */
	static Common::WriteStream *openForWriting(const Common::String &filepath);

	/**
	 * Create the directory that composes the path.
	 * If any directories that make up the path do not exist,
	 * they are created.
	 */
	static void createDirectory(Path &path);

	/**
	 * Create a directory that composes the path
	 */
	static void createDirectory(const Common::String &filepath);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
