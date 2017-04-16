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

#ifndef ZIP_FILESYSTEM_H
#define ZIP_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

/**
 * Implementation of the ScummVM file system API for zip files.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class ZipFilesystemNode : public AbstractFSNode {
private:
	Common::String _zipFile;
	Common::String _pathInZip;
	Common::Archive *_zipArchive;
	char _sep;
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;
public:
	/**
	 * Creates a ZipFilesystemNode with a path to a zip file and a path within the zip,
	 *
	 * @param zipFile Common::String with the path to the zip file.
	 * @param pathInZip Common::String with the path inside the zip file.
	 * @param sep character used to seperate the path components in the zipFile path
	 */
	ZipFilesystemNode(const Common::String &zipFile, const Common::String &pathInZip, const char sep);

	virtual bool exists() const;
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const {return true; }
	virtual bool isWritable() const { return false; }

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();

	/**
	 * Removes the last component and returns the parent path of a given path.
	 *
	 * Example: /path/to/file => /path/to
	 *
	 * @param path the path of which we want to know the parent of
	 * @param sep  the separator token (usually '/' on Unix-style systems, or '\\' on Windows based stuff)
	 * @return     the parent of path
	 */
	static Common::String getParentPath(const Common::String &path, const char sep);

};
#endif /*ZIP_FILESYSTEM_H*/
