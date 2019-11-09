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

#ifndef RISCOS_FILESYSTEM_H
#define RISCOS_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

/**
 * Implementation of the ScummVM file system API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class RISCOSFilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _nativePath;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;

	virtual AbstractFSNode *makeNode(const Common::String &path) const {
		return new RISCOSFilesystemNode(path);
	}

	/**
	 * Plain constructor, for internal use only (hence protected).
	 */
	RISCOSFilesystemNode() : _isDirectory(false), _isValid(false) {}
public:
	/**
	 * Creates a RISCOSFilesystemNode for a given path.
	 *
	 * @param path the path the new node should point to.
	 */
	RISCOSFilesystemNode(const Common::String &path);

	virtual bool exists() const;
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const;
	virtual bool isWritable() const;

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();
	virtual bool createDirectory();
private:
	/**
	 * Tests and sets the _isValid and _isDirectory flags, using OS_File 20.
	 */
	virtual void setFlags();
};

namespace Riscos {

/**
 * Assure that a directory path exists.
 *
 * @param dir The path which is required to exist.
 * @param prefix An (optional) prefix which should not be created if non existent.
 *               prefix is prepended to dir if supplied.
 * @return true in case the directoy exists (or was created), false otherwise.
 */
bool assureDirectoryExists(const Common::String &dir, const char *prefix = nullptr);

} // End of namespace RISCOS

#endif
