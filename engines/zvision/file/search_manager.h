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

#ifndef ZVISION_SEARCH_MANAGER_H
#define ZVISION_SEARCH_MANAGER_H

#include "common/str.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/archive.h"
#include "common/file.h"
#include "common/list.h"

namespace ZVision {

class SearchManager {
public:
	SearchManager(const Common::Path &rootPath, int depth);
	~SearchManager();

	void addFile(const Common::Path &name, Common::Archive *arch);
	void addDir(const Common::Path &name);

	Common::File *openFile(const Common::Path &name);
	bool openFile(Common::File &file, const Common::Path &name);
	bool hasFile(const Common::Path &name);

	bool loadZix(const Common::Path &name);

	struct Node {
		Common::Path name;
		Common::Archive *arch;
	};

	typedef Common::HashMap<Common::Path, Node, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> MatchList;

	void listMembersWithExtension(MatchList &fileList, Common::String extension);

private:

	void listDirRecursive(Common::List<Common::Path> &dirList, const Common::FSNode &fsNode, int depth);

	Common::List<Common::Path> _dirList;
	Common::List<Common::Archive *> _archList;
	Common::Path _root;
	MatchList _files;
};

}

#endif // ZVISION_SEARCH_MANAGER_H
