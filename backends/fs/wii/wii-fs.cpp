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

#include <sys/iosupport.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <gctypes.h>

/**
 * Implementation of the ScummVM file system API based on Wii.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class WiiFilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _exists, _isDirectory, _isReadable, _isWritable;

	virtual void initRootNode();
	virtual bool getDevopChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual void setFlags(const struct stat *st);
	virtual void clearFlags();

public:
	/**
	 * Creates a WiiFilesystemNode with the root node as path.
	 */
	WiiFilesystemNode();

	/**
	 * Creates a WiiFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 */
	WiiFilesystemNode(const Common::String &path);
	WiiFilesystemNode(const Common::String &p, const struct stat *st);

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

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();
};

// gets all registered devoptab devices
bool WiiFilesystemNode::getDevopChildren(AbstractFSList &list, ListMode mode, bool hidden) const {
	u8 i;
	const devoptab_t* dt;

	if (mode == Common::FSNode::kListFilesOnly)
		return true;

	// skip in, out and err
	for (i = 3; i < STD_MAX; ++i) {
		dt = devoptab_list[i];

		if (!dt || !dt->name || !dt->open_r || !dt->diropen_r)
			continue;

		list.push_back(new WiiFilesystemNode(Common::String(dt->name) + ":/"));
	}

	return true;
}

void WiiFilesystemNode::initRootNode() {
	_path.clear();
	_displayName = "<devices>";

	_exists = true;
	_isDirectory = true;
	_isReadable = false;
	_isWritable = false;
}

void WiiFilesystemNode::clearFlags() {
	_exists = false;
	_isDirectory = false;
	_isReadable = false;
	_isWritable = false;
}

void WiiFilesystemNode::setFlags(const struct stat *st) {
	_exists = true;
	_isDirectory = S_ISDIR(st->st_mode);
	_isReadable = (st->st_mode & S_IRUSR) > 0;
	_isWritable = (st->st_mode & S_IWUSR) > 0;
}

WiiFilesystemNode::WiiFilesystemNode() {
	initRootNode();
}

WiiFilesystemNode::WiiFilesystemNode(const Common::String &p) {
	if (p.empty()) {
		initRootNode();
		return;
	}

	_path = Common::normalizePath(p, '/');

	WiiFilesystemFactory::instance().mountByPath(_path);

	// "sd:" is not a valid directory, but "sd:/" is
	if (_path.lastChar() == ':')
		_path += '/';

	_displayName = lastPathComponent(_path, '/');

	struct stat st;
	if (!stat(_path.c_str(), &st))
		setFlags(&st);
	else
		clearFlags();
}

WiiFilesystemNode::WiiFilesystemNode(const Common::String &p, const struct stat *st) {
	if (p.empty()) {
		initRootNode();
		return;
	}

	_path = Common::normalizePath(p, '/');

	// "sd:" is not a valid directory, but "sd:/" is
	if (_path.lastChar() == ':')
		_path += '/';

	_displayName = lastPathComponent(_path, '/');

	setFlags(st);
}

bool WiiFilesystemNode::exists() const {
	return _exists;
}

AbstractFSNode *WiiFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);

	assert(!n.contains('/'));

	Common::String newPath(_path);
	if (newPath.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return new WiiFilesystemNode(newPath);
}

bool WiiFilesystemNode::getChildren(AbstractFSList &list, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	if (_path.empty())
		return getDevopChildren(list, mode, hidden);

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

		list.push_back(new WiiFilesystemNode(newPath, &st));
	}

	dirclose(dp);

	return true;
}

AbstractFSNode *WiiFilesystemNode::getParent() const {
	if (_path.empty())
		return NULL;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '/');

	return new WiiFilesystemNode(Common::String(start, end - start));
}

Common::SeekableReadStream *WiiFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *WiiFilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}

#endif //#if defined(__WII__)

