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

#ifndef ULTIMA4_FILESYSTEM_H
#define ULTIMA4_FILESYSTEM_H

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
    Path(const Common::String &p);
    
    bool exists() const;
    bool isFile() const;
    bool isDir() const;
    Common::String getPath() const          { return path; } /**< Returns the full translated path */
    Common::List<Common::String>* getDirTree() { return &dirs; } /**< Returns the list of directories for the path */
    Common::String getDir() const;
    Common::String getFilename() const;
    Common::String getBaseFilename() const;
    Common::String getExt() const;

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
	static Common::SeekableReadStream *openForReading(const Common::String &filepath);
	static Common::WriteStream *openForWriting(const Common::String &filepath);

	static void createDirectory(Path &path);
	static void createDirectory(const Common::String &filepath);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
