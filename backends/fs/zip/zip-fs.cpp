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

#include "common/system.h"
#include "common/archive.h"
#include "common/unzip.h"
#include "backends/fs/fs-factory.h"
#include "backends/fs/zip/zip-fs.h"
#include "backends/fs/stdiostream.h"

ZipFilesystemNode::ZipFilesystemNode(const Common::String &zipFile, const Common::String &pathInZip, const char sep) {
	_sep = sep;
	_zipFile = zipFile;
	_pathInZip = pathInZip;
	_path = _zipFile;
	if (!_pathInZip.empty())
		_path += '/' + _pathInZip;
	_displayName = lastPathComponent(_path, _pathInZip.empty() ? _sep : '/');
	_zipArchive = Common::makeZipArchive(StdioStream::makeFromPath(_zipFile, false));

	// Determine if the path inside the zip is a directory
	_isValid = _zipArchive != nullptr;
	_isDirectory = true;
	if (_zipArchive && !_pathInZip.empty()) {
		Common::ArchiveMemberList arcList;
		_zipArchive->listMatchingMembers(arcList, _pathInZip + '/');
		_isDirectory = !arcList.empty() && !_zipArchive->hasFile(_pathInZip);
		_isValid = _isDirectory || _zipArchive->hasFile(_pathInZip);
	}
}

Common::String ZipFilesystemNode::getParentPath(const Common::String &path, const char sep) {
	Common::String newPath = normalizePath(path, sep);
	Common::String last = lastPathComponent(newPath, sep);

	newPath.erase(newPath.size() - last.size(), last.size());

	return normalizePath(newPath, sep);
}

bool ZipFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	Common::ArchiveMemberList arcList;

	if (!_zipArchive)
		return false;

	if (_pathInZip.empty())
		_zipArchive->listMembers(arcList);
	else
		_zipArchive->listMatchingMembers(arcList, _pathInZip + '*');

	for (Common::ArchiveMemberList::const_iterator i = arcList.begin(); i != arcList.end(); ++i) {
		Common::String n((*i)->getName());

		if (getParentPath(n, '/') != _pathInZip)
			continue;

		myList.push_back(new ZipFilesystemNode(_zipFile, normalizePath(n, '/'), _sep));
	}

	return true;
}

AbstractFSNode *ZipFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return new ZipFilesystemNode(_zipFile, newPath, _sep);
}

AbstractFSNode *ZipFilesystemNode::getParent() const {
	Common::String newPath;

	if (_pathInZip.empty()) {
		newPath = getParentPath(_zipFile, _sep);
		return g_system->getFilesystemFactory()->makeFileNodePath(newPath);
	}

	newPath = getParentPath(_pathInZip, '/');
	return new ZipFilesystemNode(_zipFile, newPath, _sep);
}

bool ZipFilesystemNode::exists() const {
	return _isDirectory || (_zipArchive && _zipArchive->hasFile(_pathInZip));
}

Common::SeekableReadStream *ZipFilesystemNode::createReadStream() {
	if (_pathInZip.empty())
		return StdioStream::makeFromPath(_zipFile, false);

	assert(_zipArchive);
	return _zipArchive->createReadStreamForMember(_pathInZip);
}

Common::WriteStream *ZipFilesystemNode::createWriteStream() {
	if (_pathInZip.empty())
		return StdioStream::makeFromPath(_zipFile, true);

	return nullptr;
}
