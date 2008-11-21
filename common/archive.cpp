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
#include "common/util.h"
#include "common/system.h"

namespace Common {

GenericArchiveMember::GenericArchiveMember(String name, Archive *parent)
	: _parent(parent), _name(name) {
}

String GenericArchiveMember::getName() const {
	return _name;
}

SeekableReadStream *GenericArchiveMember::open() {
	return _parent->openFile(_name);
}


int Archive::listMatchingMembers(ArchiveMemberList &list, const String &pattern) {
	// Get all "names" (TODO: "files" ?)
	ArchiveMemberList allNames;
	listMembers(allNames);

	int matches = 0;

	// need to match lowercase key
	String lowercasePattern = pattern;
	lowercasePattern.toLowercase();

	ArchiveMemberList::iterator it = allNames.begin();
	for ( ; it != allNames.end(); it++) {
		if ((*it)->getName().matchString(lowercasePattern)) {
			list.push_back(*it);
			matches++;
		}
	}

	return matches;
}

/**
 *  FSDirectoryMemeber is the implementation of ArchiveMember used by
 *  by FSDirectory. It is right now a light wrapper or FSNode.
 */
class FSDirectoryMember : public ArchiveMember {
	FSNode 	_node;

public:
	FSDirectoryMember(FSNode &node) : _node(node) {
	}

	String getName() const {
		return _node.getName();
	}

	String getDisplayName() const {
		return _node.getPath();
	}

	SeekableReadStream *open() {
		return _node.openForReading();
	}
};

FSDirectory::FSDirectory(const FSNode &node, int depth)
  : _node(node), _cached(false), _depth(depth) {
}

FSDirectory::FSDirectory(const String &prefix, const FSNode &node, int depth)
  : _node(node), _cached(false), _depth(depth) {

	setPrefix(prefix);
}

FSDirectory::FSDirectory(const String &name, int depth)
  : _node(name), _cached(false), _depth(depth) {
}

FSDirectory::FSDirectory(const String &prefix, const String &name, int depth)
  : _node(name), _cached(false), _depth(depth) {

	setPrefix(prefix);
}

FSDirectory::~FSDirectory() {
}

void FSDirectory::setPrefix(const String &prefix) {
	_prefix = prefix;

	if (!_prefix.empty() && !_prefix.hasSuffix("/")) {
		_prefix += "/";
	}
}

FSNode FSDirectory::getFSNode() const {
	return _node;
}

FSNode FSDirectory::lookupCache(NodeCache &cache, const String &name) {
	// make caching as lazy as possible
	if (!name.empty()) {
		if (!_cached) {
			cacheDirectoryRecursive(_node, _depth, _prefix);
			_cached = true;
		}

		if (cache.contains(name))
			return cache[name];
	}

	return FSNode();
}

bool FSDirectory::hasFile(const String &name) {
	if (name.empty() || !_node.isDirectory()) {
		return false;
	}

	FSNode node = lookupCache(_fileCache, name);
	return node.exists();
}

ArchiveMemberPtr FSDirectory::getMember(const String &name) {
	if (name.empty() || !_node.isDirectory()) {
		return ArchiveMemberPtr();
	}

	FSNode node = lookupCache(_fileCache, name);

	if (!node.exists()) {
		warning("FSDirectory::getMember: FSNode does not exist");
		return ArchiveMemberPtr();
	} else if (node.isDirectory()) {
		warning("FSDirectory::getMember: FSNode is a directory");
		return ArchiveMemberPtr();
	}

	return ArchiveMemberPtr(new FSDirectoryMember(node));
}

SeekableReadStream *FSDirectory::openFile(const String &name) {
	if (name.empty() || !_node.isDirectory()) {
		return 0;
	}

	FSNode node = lookupCache(_fileCache, name);

	if (!node.exists()) {
		warning("FSDirectory::openFile: FSNode does not exist");
		return 0;
	} else if (node.isDirectory()) {
		warning("FSDirectory::openFile: FSNode is a directory");
		return 0;
	}

	SeekableReadStream *stream = node.openForReading();
	if (!stream) {
		warning("FSDirectory::openFile: Can't create stream for file '%s'", name.c_str());
	}

	return stream;
}

FSDirectory *FSDirectory::getSubDirectory(const String &name, int depth) {
	return getSubDirectory(String::emptyString, name, depth);
}

FSDirectory *FSDirectory::getSubDirectory(const String &prefix, const String &name, int depth) {
	if (name.empty() || !_node.isDirectory()) {
		return 0;
	}

	FSNode node = lookupCache(_subDirCache, name);
	return new FSDirectory(prefix, node, depth);
}

void FSDirectory::cacheDirectoryRecursive(FSNode node, int depth, const String& prefix) {
	if (depth <= 0) {
		return;
	}

	FSList list;
	node.getChildren(list, FSNode::kListAll, false);

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

int FSDirectory::listMatchingMembers(ArchiveMemberList &list, const String &pattern) {
	if (!_node.isDirectory())
		return 0;

	// Cache dir data
	if (!_cached) {
		cacheDirectoryRecursive(_node, _depth, "");
		_cached = true;
	}

	String lowercasePattern(pattern);
	lowercasePattern.toLowercase();

	int matches = 0;
	NodeCache::iterator it = _fileCache.begin();
	for ( ; it != _fileCache.end(); it++) {
		if ((*it)._key.matchString(lowercasePattern)) {
			list.push_back(ArchiveMemberPtr(new FSDirectoryMember((*it)._value)));
			matches++;
		}
	}
	return matches;
}

int FSDirectory::listMembers(ArchiveMemberList &list) {
	return listMatchingMembers(list, "*");
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

void SearchSet::add(const String &name, Archive *archive, int priority, bool autoFree) {
	if (find(name) == _list.end()) {
		Node node(priority, name, archive, autoFree);
		insert(node);
	} else {
		if (autoFree)
			delete archive;
		warning("SearchSet::add: archive '%s' already present", name.c_str());
	}

}

void SearchSet::remove(const String &name) {
	ArchiveList::iterator it = find(name);
	if (it != _list.end()) {
		if (it->_autoFree)
			delete it->_arc;
		_list.erase(it);
	}
}

bool SearchSet::hasArchive(const String &name) const {
	return (find(name) != _list.end());
}

void SearchSet::clear() {
	for (ArchiveList::iterator i = _list.begin(); i != _list.end(); ++i) {
		if (i->_autoFree)
			delete i->_arc;
	}

	_list.clear();
}

void SearchSet::setPriority(const String &name, int priority) {
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

int SearchSet::listMatchingMembers(ArchiveMemberList &list, const String &pattern) {
	int matches = 0;

	ArchiveList::iterator it = _list.begin();
	for ( ; it != _list.end(); it++) {
		matches += (*it)._arc->listMatchingMembers(list, pattern);
	}

	return matches;
}

int SearchSet::listMembers(ArchiveMemberList &list) {
	int matches = 0;

	ArchiveList::iterator it = _list.begin();
	for ( ; it != _list.end(); it++) {
		matches += (*it)._arc->listMembers(list);
	}

	return matches;
}

ArchiveMemberPtr SearchSet::getMember(const String &name) {
	if (name.empty()) {
		return ArchiveMemberPtr();
	}

	ArchiveList::iterator it = _list.begin();
	for ( ; it != _list.end(); it++) {
		if ((*it)._arc->hasFile(name)) {
			return (*it)._arc->getMember(name);
		}
	}

	return ArchiveMemberPtr();
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


DECLARE_SINGLETON(SearchManager);

SearchManager::SearchManager() {
	clear();	// Force a reset
}

void SearchManager::addDirectory(const String &name, const String &directory, int priority, int depth) {
	FSNode dir(directory);
	addDirectory(name, dir, priority, depth);
}

void SearchManager::addDirectory(const String &name, const FSNode &dir, int priority, int depth) {
	if (!dir.exists() || !dir.isDirectory())
		return;

	add(name, new FSDirectory(dir, depth), priority);
}

void SearchManager::clear() {
	SearchSet::clear();

	// Always keep system specific archives in the SearchManager.
	// But we give them a lower priority than the default priority (which is 0),
	// so that archives added by client code are searched first.
	g_system->addSysArchivesToSearchSet(*this, -1);

	// Add the current dir as a very last resort.
	// See also bug #2137680.
	addDirectory(".", ".", -2);
}

} // namespace Common
