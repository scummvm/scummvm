/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if defined(__WII__)

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir

#include "backends/fs/wii/wii-fs.h"
#include "backends/fs/wii/wii-fs-factory.h"
#include "backends/fs/stdiostream.h"

#include <sys/iosupport.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <gctypes.h>

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
	_isDirectory = ( (st->st_mode & S_IFDIR) != 0 );
	_isReadable = ( (st->st_mode & S_IRUSR) != 0 );
	_isWritable = ( (st->st_mode & S_IWUSR) != 0 );
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
	if(stat(_path.c_str(), &st) != -1)
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

	DIR *dp = opendir (_path.c_str());

	if (dp == NULL)
		return false;

	struct dirent *pent;

	while ((pent = readdir(dp)) != NULL) {
		if (strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;

		Common::String newPath(_path);
		if (newPath.lastChar() != '/')
			newPath += '/';
		newPath += pent->d_name;

		bool isDir = ( pent->d_type == DT_DIR );

		if ((mode == Common::FSNode::kListFilesOnly && isDir) ||
			(mode == Common::FSNode::kListDirectoriesOnly && !isDir))
			continue;

		struct stat st;
		st.st_mode = 0;
		st.st_mode |= ( isDir ? S_IFDIR : 0 );
		st.st_mode |= S_IRUSR;
		st.st_mode |= S_IWUSR;

		list.push_back(new WiiFilesystemNode(newPath, &st));
	}

	closedir(dp);

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
	StdioStream *readStream = StdioStream::makeFromPath(getPath(), StdioStream::WriteMode_Read);

	// disable newlib's buffering, the device libraries handle caching
	if (readStream) {
		readStream->setBufferSize(0);
	}

	return readStream;
}

Common::SeekableWriteStream *WiiFilesystemNode::createWriteStream(bool atomic) {
	StdioStream *writeStream = StdioStream::makeFromPath(getPath(), atomic ?
			StdioStream::WriteMode_WriteAtomic : StdioStream::WriteMode_Write);

	// disable newlib's buffering, the device libraries handle caching
	if (writeStream) {
		writeStream->setBufferSize(0);
	}

	return writeStream;
}

bool WiiFilesystemNode::createDirectory() {
	if(!_exists) {
 		if (mkdir(_path.c_str(), 0755) == 0) {
			_exists = true;
			_isDirectory = true;
			_isReadable = true;
			_isWritable = true;
		}
	}

	return _exists && _isDirectory;
}

#endif //#if defined(__WII__)
