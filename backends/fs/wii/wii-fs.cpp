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

#if defined(__WII__)

#include "backends/fs/abstract-fs.h"
#include "backends/fs/stdiostream.h"

#include <sys/dir.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Implementation of the ScummVM file system API based on Wii.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class WiiFilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory, _isReadable, _isWritable;

public:
	/**
	 * Creates a WiiFilesystemNode with the root node as path.
	 */
	WiiFilesystemNode();

	/**
	 * Creates a WiiFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 * @param verify true if the isValid and isDirectory flags should be verified during the construction.
	 */
	WiiFilesystemNode(const Common::String &path, bool verify);

	virtual bool exists() const;
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return _isReadable; }
	virtual bool isWritable() const { return _isWritable; }

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *openForReading();
	virtual Common::WriteStream *openForWriting();

private:
	virtual void setFlags();
};

void WiiFilesystemNode::setFlags() {
	struct stat st;

	_isDirectory = false;
	_isReadable = false;
	_isWritable = false;

	if (!stat(_path.c_str(), &st)) {
		_isDirectory = S_ISDIR(st.st_mode);
		_isReadable = (st.st_mode & S_IRUSR) > 0;
		_isWritable = (st.st_mode & S_IWUSR) > 0;
	}
}


WiiFilesystemNode::WiiFilesystemNode() {
	// The root dir.
	_path = "fat:/";
	_displayName = _path;

	setFlags();
}

WiiFilesystemNode::WiiFilesystemNode(const Common::String &p, bool verify) {
	assert(p.size() > 0);

	_path = p;

	_displayName = lastPathComponent(_path, '/');

	if (verify)
		setFlags();
}

bool WiiFilesystemNode::exists() const {
	struct stat st;
	return stat(_path.c_str (), &st) == 0;
}

AbstractFSNode *WiiFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);

	Common::String newPath(_path);
	if (newPath.lastChar() != '/')
			newPath += '/';
	newPath += n;

	return new WiiFilesystemNode(newPath, true);
}

bool WiiFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	DIR_ITER* dp = diropen (_path.c_str());

	if (dp == NULL)
		return false;

	char filename[MAXPATHLEN];
	struct stat st;

	while (dirnext(dp, filename, &st) == 0) {
		if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
			continue;

		Common::String newPath(_path);
		if (newPath.lastChar() != '/')
				newPath += '/';
		newPath += filename;

		bool isDir = S_ISDIR(st.st_mode);

		if ((mode == Common::FSNode::kListFilesOnly && isDir) ||
			(mode == Common::FSNode::kListDirectoriesOnly && !isDir))
			continue;

		if (isDir)
			newPath += '/';

		myList.push_back(new WiiFilesystemNode(newPath, true));
	}

	dirclose(dp);

	return true;
}

AbstractFSNode *WiiFilesystemNode::getParent() const {
	if (_path == "/")
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '/');

	return new WiiFilesystemNode(Common::String(start, end - start), true);
}

Common::SeekableReadStream *WiiFilesystemNode::openForReading() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *WiiFilesystemNode::openForWriting() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

#endif //#if defined(__WII__)

