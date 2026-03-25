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

#include "backends/fs/iso/iso-abstract-fs.h"
#include "common/tokenizer.h"
#include "common/ustr.h"

ISOAbstractFSNode::ISOAbstractFSNode(
		Common::SharedPtr<Common::ISO9660FileSystem> fs,
		const Common::String &isoImagePath)
	: _fs(fs) {
	// Root node: synthesize a DirEntry that represents the root directory
	_entry.isDir = true;
	_entry.name  = "";
	_entry.lba   = fs ? fs->getRootLba()  : 0;
	_entry.size  = fs ? fs->getRootSize() : 0;
	_virtualPath = isoImagePath.empty() ? "/" : (isoImagePath + "/");
}

ISOAbstractFSNode::ISOAbstractFSNode(
		Common::SharedPtr<Common::ISO9660FileSystem> fs,
		const Common::ISO9660FileSystem::DirEntry &entry,
		const Common::String &virtualPath)
	: _fs(fs), _entry(entry), _virtualPath(virtualPath) {
}

Common::String ISOAbstractFSNode::lastComponent() const {
	Common::String::size_type slash = _virtualPath.rfind('/');
	if (slash == Common::String::npos) { return _virtualPath; }
	return _virtualPath.substr(slash + 1);
}

Common::String ISOAbstractFSNode::getName() const {
	Common::String n = lastComponent();
	return n.empty() ? _virtualPath : n;
}

Common::U32String ISOAbstractFSNode::getDisplayName() const {
	return Common::U32String(getName());
}

Common::String ISOAbstractFSNode::childPath(const Common::String &childName) const {
	Common::String p = _virtualPath;
	if (!p.empty() && p.lastChar() != '/')
		p += '/';
	p += childName;
	return p;
}

bool ISOAbstractFSNode::getChildren(AbstractFSList &list,
                                     ListMode mode, bool /*hidden*/) const {
	if (!_entry.isDir || !_fs || !_fs->isOpen()) return false;

	Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
		_fs->readDirectory(_entry);

	for (const auto &e : entries) {
		if (mode == Common::FSNode::kListFilesOnly && e.isDir) continue;
		if (mode == Common::FSNode::kListDirectoriesOnly && !e.isDir) continue;

		list.push_back(new ISOAbstractFSNode(_fs, e, childPath(e.name)));
	}
	return true;
}

AbstractFSNode *ISOAbstractFSNode::getChild(const Common::String &name) const {
	if (!_entry.isDir || !_fs) return nullptr;

	Common::Array<Common::ISO9660FileSystem::DirEntry> entries =
		_fs->readDirectory(_entry);

	for (const auto &e : entries) {
		if (e.name.equalsIgnoreCase(name)) {
			return new ISOAbstractFSNode(_fs, e, childPath(e.name));
		}
	}
	return nullptr;
}

AbstractFSNode *ISOAbstractFSNode::getParent() const {
	Common::String::size_type slash = _virtualPath.rfind('/');
	if (slash == Common::String::npos || slash == 0) {
		return new ISOAbstractFSNode(*this);
	}
	Common::String parentPath = _virtualPath.substr(0, slash);

	Common::ISO9660FileSystem::DirEntry cur;
	cur.isDir = true;
	cur.lba   = _fs->getRootLba();
	cur.size  = _fs->getRootSize();
	cur.name  = "";

	Common::String walkPath = parentPath;
	if (!walkPath.empty() && walkPath[0] == '/') { walkPath = walkPath.substr(1); }

	Common::StringTokenizer tok(walkPath, "/");
	while (!tok.empty()) {
		Common::String component = tok.nextToken();
		if (component.empty()) { continue; }

		Common::Array<Common::ISO9660FileSystem::DirEntry> children =
			_fs->readDirectory(cur);

		bool found = false;
		for (const auto &e : children) {
			if (e.name.equalsIgnoreCase(component)) {
				cur = e;
				found = true;
				break;
			}
		}
		if (!found) {
			return new ISOAbstractFSNode(_fs, "");
		}
	}

	return new ISOAbstractFSNode(_fs, cur, parentPath);
}

Common::SeekableReadStream *ISOAbstractFSNode::createReadStream() {
	if (_entry.isDir || !_fs) return nullptr;
	return _fs->openFile(_entry);
}

Common::FSNode makeISOFSNodeRoot(Common::SharedPtr<Common::ISO9660FileSystem> fs,
                                  const Common::String &isoImagePath) {
	return AbstractFSNode::makeFSNode(
		new ISOAbstractFSNode(fs, isoImagePath));
}
