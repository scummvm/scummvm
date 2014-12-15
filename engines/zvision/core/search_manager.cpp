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
*/

#include "common/debug.h"
#include "common/fs.h"
#include "common/stream.h"

#include "zvision/core/search_manager.h"
#include "zvision/utility/zfs_archive.h"

namespace ZVision {

SearchManager::SearchManager(const Common::String &rootPath, int depth) {
	_root = rootPath;
	if (_root[_root.size() - 1] == '\\' || _root[_root.size() - 1] == '/')
		_root.deleteLastChar();

	Common::FSNode fsNode(_root);

	listDirRecursive(_dirList, fsNode, depth);

	for (Common::List<Common::String>::iterator it = _dirList.begin(); it != _dirList.end();)
		if (it->size() == _root.size())
			it = _dirList.erase(it);
		else if (it->size() > _root.size()) {
			*it = Common::String(it->c_str() + _root.size() + 1);
			it++;
		} else
			it++;
}

SearchManager::~SearchManager() {
	Common::List<Common::Archive *>::iterator it = _archList.begin();
	while (it != _archList.end()) {
		delete *it;
		it++;
	}

	_archList.clear();
}

void SearchManager::addPatch(const Common::String &src, const Common::String &dst) {
	Common::String lowerCaseName = dst;
	lowerCaseName.toLowercase();

	SearchManager::MatchList::iterator it = _files.find(lowerCaseName);

	if (it != _files.end()) {
		lowerCaseName = src;
		lowerCaseName.toLowercase();
		_files[lowerCaseName] = it->_value;
	}
}

void SearchManager::addFile(const Common::String &name, Common::Archive *arch) {
	bool addArch = true;
	Common::List<Common::Archive *>::iterator it = _archList.begin();
	while (it != _archList.end()) {
		if (*it == arch) {
			addArch = false;
			break;
		}
		it++;
	}
	if (addArch)
		_archList.push_back(arch);

	Common::String lowerCaseName = name;
	lowerCaseName.toLowercase();

	SearchManager::Node nod;
	nod.name = lowerCaseName;
	nod.arch = arch;

	SearchManager::MatchList::iterator fit = _files.find(lowerCaseName);

	if (fit == _files.end()) {
		_files[lowerCaseName] = nod;
	} else {
		Common::SeekableReadStream *stream = fit->_value.arch->createReadStreamForMember(fit->_value.name);
		if (stream) {
			if (stream->size() < 10)
				fit->_value.arch = arch;
			delete stream;
		} else {
			_files[lowerCaseName] = nod;
		}
	}
}

Common::File *SearchManager::openFile(const Common::String &name) {
	Common::String lowerCaseName = name;
	lowerCaseName.toLowercase();

	SearchManager::MatchList::iterator fit = _files.find(lowerCaseName);

	if (fit != _files.end()) {
		Common::File *tmp = new Common::File();
		tmp->open(fit->_value.name, *fit->_value.arch);
		return tmp;
	}
	return NULL;
}

bool SearchManager::openFile(Common::File &file, const Common::String &name) {
	Common::String lowerCaseName = name;
	lowerCaseName.toLowercase();

	SearchManager::MatchList::iterator fit = _files.find(lowerCaseName);

	if (fit != _files.end())
		return file.open(fit->_value.name, *fit->_value.arch);
	return false;
}

bool SearchManager::hasFile(const Common::String &name) {
	Common::String lowerCaseName = name;
	lowerCaseName.toLowercase();

	SearchManager::MatchList::iterator fit = _files.find(lowerCaseName);

	if (fit != _files.end())
		return true;
	return false;
}

void SearchManager::loadZix(const Common::String &name) {
	Common::File file;
	file.open(name);

	Common::String line;

	while (!file.eos()) {
		line = file.readLine();
		if (line.matchString("----------*", true))
			break;
	}

	if (file.eos())
		return;

	Common::Array<Common::Archive *> archives;

	while (!file.eos()) {
		line = file.readLine();
		line.trim();
		if (line.matchString("----------*", true))
			break;
		else if (line.matchString("DIR:*", true)) {
			Common::String path(line.c_str() + 5);
			Common::Archive *arc;
			char tempPath[128];
			strcpy(tempPath, path.c_str());
			for (uint i = 0; i < path.size(); i++)
				if (tempPath[i] == '\\')
					tempPath[i] = '/';

			path = Common::String(tempPath);
			if (path.size() && path[0] == '.')
				path.deleteChar(0);
			if (path.size() && path[0] == '/')
				path.deleteChar(0);

			if (path.matchString("*.zfs", true))
				arc = new ZfsArchive(path);
			else {
				if (path.size()) {
					if (path[path.size() - 1] == '\\' || path[path.size() - 1] == '/')
						path.deleteLastChar();
					if (path.size())
						for (Common::List<Common::String>::iterator it = _dirList.begin(); it != _dirList.end(); ++it)
							if (path.equalsIgnoreCase(*it)) {
								path = *it;
								break;
							}
				}

				path = Common::String::format("%s/%s", _root.c_str(), path.c_str());

				arc = new Common::FSDirectory(path);
			}
			archives.push_back(arc);
		}
	}

	if (file.eos())
		return;

	while (!file.eos()) {
		line = file.readLine();
		line.trim();
		uint dr = 0;
		char buf[32];
		if (sscanf(line.c_str(), "%u %s", &dr, buf) == 2) {
			if (dr <= archives.size() && dr > 0) {
				addFile(Common::String(buf), archives[dr - 1]);
			}
		}
	}
}

void SearchManager::addDir(const Common::String &name) {
	Common::String path;
	for (Common::List<Common::String>::iterator it = _dirList.begin(); it != _dirList.end(); ++it)
		if (name.equalsIgnoreCase(*it)) {
			path = *it;
			break;
		}

	if (path.size() == 0)
		return;

	path = Common::String::format("%s/%s", _root.c_str(), path.c_str());

	Common::FSDirectory *dir = new Common::FSDirectory(path);

	Common::ArchiveMemberList list;
	dir->listMatchingMembers(list, "*.zfs");

	for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
		Common::String flname = (*iter)->getName();

		ZfsArchive *zfs = new ZfsArchive(Common::String::format("%s/%s", name.c_str(), flname.c_str()));

		Common::ArchiveMemberList zfslist;
		zfs->listMembers(zfslist);

		for (Common::ArchiveMemberList::iterator ziter = zfslist.begin(); ziter != zfslist.end(); ++ziter) {
			Common::String zfsFileName = (*ziter)->getName();
			addFile(zfsFileName, zfs);
		}
	}

	list.clear();
	dir->listMembers(list);

	for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
		Common::String flname = (*iter)->getName();
		addFile(flname, dir);
	}
}

void SearchManager::listDirRecursive(Common::List<Common::String> &_list, const Common::FSNode &fsNode, int depth) {
	Common::FSList fsList;
	fsNode.getChildren(fsList);

	_list.push_back(fsNode.getPath());

	if (depth > 1)
		for (Common::FSList::const_iterator it = fsList.begin(); it != fsList.end(); ++it)
			listDirRecursive(_list, *it, depth - 1);
}

} // End of namespace ZVision
