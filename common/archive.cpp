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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/archive.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/util.h"

namespace Common {


FSDirectory::FSDirectory(const FilesystemNode &node, int depth)
  : _node(node), _cached(false), _depth(depth) {
}

FSDirectory::FSDirectory(const String &name, int depth)
  : _node(name), _cached(false), _depth(depth) {
}

FSDirectory::~FSDirectory() {
}

FilesystemNode FSDirectory::getFSNode() const {
	return _node;
}

FilesystemNode FSDirectory::lookupCache(NodeCache &cache, const String &name) {
	// make caching as lazy as possible
	if (!name.empty()) {
		if (!_cached) {
			cacheDirectoryRecursive(_node, _depth, "");
			_cached = true;
		}

		if (cache.contains(name))
			return cache[name];
	}

	return FilesystemNode();
}

bool FSDirectory::hasFile(const String &name) {
	if (name.empty() || !_node.isDirectory()) {
		return false;
	}

	FilesystemNode node = lookupCache(_fileCache, name);
	return node.exists();
}

SeekableReadStream *FSDirectory::openFile(const String &name) {
	if (name.empty() || !_node.isDirectory()) {
		return 0;
	}

	FilesystemNode node = lookupCache(_fileCache, name);

	if (!node.exists()) {
		warning("FSDirectory::openFile: Trying to open a FilesystemNode which does not exist");
		return 0;
	} else if (node.isDirectory()) {
		warning("FSDirectory::openFile: Trying to open a FilesystemNode which is a directory");
		return 0;
	}

	SeekableReadStream *stream = node.openForReading();
	if (!stream) {
		warning("FSDirectory::openFile: Can't create stream for file '%s'", name.c_str());
	}

	return stream;
}

FSDirectory *FSDirectory::getSubDirectory(const String &name) {
	if (name.empty() || !_node.isDirectory()) {
		return 0;
	}

	FilesystemNode node = lookupCache(_subDirCache, name);
	return new FSDirectory(node);
}

void FSDirectory::cacheDirectoryRecursive(FilesystemNode node, int depth, const String& prefix) {
	if (depth <= 0) {
		return;
	}

	FSList list;
	node.getChildren(list, FilesystemNode::kListAll, false);

	FSList::iterator it = list.begin();
	for ( ; it != list.end(); it++) {
		String name = prefix + (*it).getName();

		// don't touch name as it might be used for warning messages
		String lowercaseName = name;
		lowercaseName.toLowercase();

		// since the hashmap is case insensitive, we need to check for clashes when caching
		if ((*it).isDirectory()) {
			if (_subDirCache.contains(lowercaseName)) {
				warning("FSDirectory::cacheDirectory: name clash when building cache, ignoring sub-directory '%s'", name.c_str());
			} else {
				cacheDirectoryRecursive(*it, depth - 1, lowercaseName + "/");
				_subDirCache[lowercaseName] = *it;
			}
		} else {
			if (_fileCache.contains(lowercaseName)) {
				warning("FSDirectory::cacheDirectory: name clash when building cache, ignoring file '%s'", name.c_str());
			} else {
				_fileCache[lowercaseName] = *it;
			}
		}
	}

}

int FSDirectory::matchPattern(StringList &list, const String &pattern) {
	if (pattern.empty() || !_node.isDirectory()) {
		return 0;
	}

	if (!_cached) {
		cacheDirectoryRecursive(_node, _depth, "");
		_cached = true;
	}

	int matches = 0;

	// need to match lowercase key
	String lowercasePattern = pattern;
	lowercasePattern.toLowercase();

	// Full *key* match, with path separators (backslashes) considered
	// as normal characters.
	NodeCache::iterator it = _fileCache.begin();
	for ( ; it != _fileCache.end(); it++) {
		if (matchString((*it)._key.c_str(), lowercasePattern.c_str())) {
			list.push_back((*it)._key.c_str());
		}
	}

	return matches;
}





SearchSet::SearchSet() {

}

SearchSet::~SearchSet() {

}

SearchSet::ArchiveList::iterator SearchSet::find(const String &name) const {
	ArchiveList::iterator it = _list.begin();
	for ( ; it != _list.end(); it++) {
		if ((*it)._name == name) {
			break;
		}
	}
	return it;
}

/*
	Keep the nodes sorted according to descending priorities.
	In case two or node nodes have the same priority, insertion
	order prevails.
*/
void SearchSet::insert(const Node &node) {
	ArchiveList::iterator it = _list.begin();
	for ( ; it != _list.end(); it++) {
		if ((*it)._priority < node._priority) {
			break;
		}
	}
	_list.insert(it, node);
}

void SearchSet::add(const String& name, ArchivePtr archive, uint priority) {
	if (find(name) == _list.end()) {
		Node node = { priority, name, archive };
		insert(node);
	} else {
		warning("SearchSet::add: archive '%s' already present", name.c_str());
	}

}

void SearchSet::remove(const String& name) {
	ArchiveList::iterator it = find(name);
	if (it != _list.end()) {
		_list.erase(it);
	}
}

void SearchSet::clear() {
	_list.clear();
}

void SearchSet::setPriority(const String& name, uint priority) {
	ArchiveList::iterator it = find(name);
	if (it == _list.end()) {
		warning("SearchSet::setPriority: archive '%s' is not present", name.c_str());
		return;
	}

	if (priority == (*it)._priority) {
		return;
	}

	Node node(*it);
	_list.erase(it);
	node._priority = priority;
	insert(node);
}

bool SearchSet::hasFile(const String &name) {
	if (name.empty()) {
		return false;
	}

	ArchiveList::iterator it = _list.begin();
	for ( ; it != _list.end(); it++) {
		if ((*it)._arc->hasFile(name)) {
			return true;
		}
	}

	return false;
}

int SearchSet::matchPattern(StringList &list, const String &pattern) {
	// Shall we short circuit out if pattern is empty?

	int matches = 0;

	ArchiveList::iterator it = _list.begin();
	for ( ; it != _list.end(); it++) {
		matches += (*it)._arc->matchPattern(list, pattern);
	}

	return matches;
}

SeekableReadStream *SearchSet::openFile(const String &name) {
	if (name.empty()) {
		return 0;
	}

	ArchiveList::iterator it = _list.begin();
	for ( ; it != _list.end(); it++) {
		if ((*it)._arc->hasFile(name)) {
			return (*it)._arc->openFile(name);
		}
	}

	return 0;
}


} // namespace Common
