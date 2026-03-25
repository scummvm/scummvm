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

#include "common/formats/iso9660archive.h"
#include "common/debug.h"
#include "common/str.h"

namespace Common {

ISO9660Archive::ISO9660Archive(SharedPtr<ISO9660FileSystem> fs, const String &subdir)
	: _fs(fs) {
	// Normalize the subdir to lowercase for case-insensitive matching
	_subdir = subdir;
	_subdir.toLowercase();
	buildIndex();
}

void ISO9660Archive::buildIndex() {
	if (!_fs || !_fs->isOpen()) {
		return;
	}

	if (_subdir.empty()) {
		// Index everything from root
		scanDirectory(_fs->getRootEntry(), "");
	} else {
		// Find the named subdirectory in the root
		auto root = _fs->readRootDirectory();
		for (const auto &entry : root) {
			if (entry.isDir && entry.name.equalsIgnoreCase(_subdir)) {
				scanDirectory(entry, "");
				return;
			}
		}
		warning("ISO9660Archive: subdir '%s' not found in ISO", _subdir.c_str());
	}
}

void ISO9660Archive::scanDirectory(const ISO9660FileSystem::DirEntry &dir, const String &prefix) {
	auto entries = _fs->readDirectory(dir);

	for (const auto &entry : entries) {
		String relPath = prefix.empty() ? entry.name : prefix + "/" + entry.name;

		if (entry.isDir) {
			scanDirectory(entry, relPath);
		}

		FileRecord rec;
		rec.entry       = entry;
		rec.archivePath = relPath;
		_index[Path(relPath, '/')] = rec;
	}
}

bool ISO9660Archive::hasFile(const Path &path) const {
	return _index.contains(path);
}

bool ISO9660Archive::isPathDirectory(const Path &path) const {
	auto it = _index.find(path);
	if (it == _index.end()) {
		return false;
	}
	return it->_value.entry.isDir;
}

int ISO9660Archive::listMembers(ArchiveMemberList &list) const {
	int count = 0;
	for (const auto &kv : _index) {
		if (!kv._value.entry.isDir) {
			list.push_back(ArchiveMemberPtr(
				new GenericArchiveMember(kv._key, *this)));
			count++;
		}
	}
	return count;
}

const ArchiveMemberPtr ISO9660Archive::getMember(const Path &path) const {
	if (!hasFile(path)) {
		return ArchiveMemberPtr();
	}
	return ArchiveMemberPtr(new GenericArchiveMember(path, *this));
}

SeekableReadStream *ISO9660Archive::createReadStreamForMember(const Path &path) const {
	auto it = _index.find(path);
	if (it == _index.end()) {
		return nullptr;
	}
	if (it->_value.entry.isDir) {
		return nullptr;
	}
	return _fs->openFile(it->_value.entry);
}

bool ISO9660Archive::getChildren(const Path &path, Array<String> &list,
                                  ListMode mode, bool hidden) const {
	String prefix = path.toString('/');
	while (!prefix.empty() && prefix.lastChar() == '/') {
		prefix.deleteLastChar();
	}

	bool found = false;
	for (const auto &kv : _index) {
		const auto &ap = kv._value.archivePath;
		auto slash = ap.rfind('/');
		String parent = slash == String::npos ? String() : ap.substr(0, slash);

		if (!parent.equalsIgnoreCase(prefix)) { continue; }
		if (mode == kListFilesOnly && kv._value.entry.isDir) { continue; }
		if (mode == kListDirectoriesOnly && !kv._value.entry.isDir) { continue; }

		list.push_back(slash == String::npos ? ap : ap.substr(slash + 1));
		found = true;
	}
	return found;
}

} // namespace Common
