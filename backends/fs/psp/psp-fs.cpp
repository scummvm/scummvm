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
 * $URL$
 * $Id$
 */

#ifdef __PSP__

#include "engines/engine.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/stdiostream.h"

#include <sys/stat.h>
#include <unistd.h>

#include <pspkernel.h>

#define	ROOT_PATH	"ms0:/"

/**
 * Implementation of the ScummVM file system API based on PSPSDK API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class PSPFilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;

public:
	/**
	 * Creates a PSPFilesystemNode with the root node as path.
	 */
	PSPFilesystemNode();

	/**
	 * Creates a PSPFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 * @param verify true if the isValid and isDirectory flags should be verified during the construction.
	 */
	PSPFilesystemNode(const Common::String &p, bool verify = true);

	virtual bool exists() const { return access(_path.c_str(), F_OK) == 0; }
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return access(_path.c_str(), R_OK) == 0; }
	virtual bool isWritable() const { return access(_path.c_str(), W_OK) == 0; }

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();
};

PSPFilesystemNode::PSPFilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = ROOT_PATH;
}

PSPFilesystemNode::PSPFilesystemNode(const Common::String &p, bool verify) {
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path, '/');
	_isValid = true;
	_isDirectory = true;

	if (verify) {
		struct stat st;
		_isValid = (0 == stat(_path.c_str(), &st));
		_isDirectory = S_ISDIR(st.st_mode);
	}
}

AbstractFSNode *PSPFilesystemNode::getChild(const Common::String &n) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return new PSPFilesystemNode(newPath, true);
}

bool PSPFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	//TODO: honor the hidden flag

	int dfd  = sceIoDopen(_path.c_str());
	if (dfd > 0) {
		SceIoDirent dir;
		memset(&dir, 0, sizeof(dir));

		while (sceIoDread(dfd, &dir) > 0) {
			// Skip 'invisible files
			if (dir.d_name[0] == '.')
				continue;

			PSPFilesystemNode entry;

			entry._isValid = true;
			entry._displayName = dir.d_name;

			Common::String newPath(_path);
			if (newPath.lastChar() != '/')
				newPath += '/';
			newPath += dir.d_name;

			entry._path = newPath;
			entry._isDirectory = dir.d_stat.st_attr & FIO_SO_IFDIR;

			// Honor the chosen mode
			if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) ||
			   (mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
				continue;

			myList.push_back(new PSPFilesystemNode(entry));
		}

		sceIoDclose(dfd);
		return true;
	} else {
		return false;
	}
}

AbstractFSNode *PSPFilesystemNode::getParent() const {
	if (_path == ROOT_PATH)
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '/');

	return new PSPFilesystemNode(Common::String(start, end - start), false);
}

Common::SeekableReadStream *PSPFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *PSPFilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

#endif //#ifdef __PSP__
