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

#include "common/config-manager.h"
#include "common/fs.h"
#include "ags/lib/std/regex.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/util/string_utils.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/stdio_compat.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

const char *SAVE_FOLDER_PREFIX = "/saves/";

namespace Directory {

bool CreateDirectory(const String &path) {
	return Common::FSNode(path.GetCStr()).createDirectory();
}

bool CreateAllDirectories(const String &parent, const String &sub_dirs) {
	if (sub_dirs == SAVE_FOLDER_PREFIX)
		// ScummVM save folder doesn't need creating
		return true;

	if (parent.IsEmpty() || !ags_directory_exists(parent.GetCStr()))
		return false; // no sense, or base dir not exist
	if (sub_dirs.IsEmpty())
		return true; // nothing to create, so fine

	String make_path = String::FromFormat("%s/", parent.GetCStr());
	for (const char *sect = sub_dirs.GetCStr();
		sect < sub_dirs.GetCStr() + sub_dirs.GetLength();) {
		const char *cur = sect + 1;
		for (; *cur && *cur != '/' && *cur != PATH_ALT_SEPARATOR; ++cur);
		// Skip empty dirs (duplicated separators etc)
		if ((cur - sect == 1) && (*sect == '.' || *sect == '/' || *sect == PATH_ALT_SEPARATOR)) {
			sect = cur;
			continue;
		}
		// In case of ".." just fail
		if (strncmp(sect, "..", cur - sect) == 0)
			return false;
		make_path.Append(sect, cur - sect);
		if (!CreateDirectory(make_path))
			return false;
		sect = cur;
	}
	return true;
}

String SetCurrentDirectory(const String &path) {
	warning("TODO: SetCurrentDirectory: %s", path.GetCStr());
	//  chdir(path);
	//  return GetCurrentDirectory();
	return path;
}

String GetCurrentDirectory() {
#ifdef TODO
	char buf[512];
	getcwd(buf, 512);
	String str(buf);
	Path::FixupPath(str);
	return str;
#else
	return ConfMan.get("path");
#endif
}

static bool GetFilesImpl(const String &dir_path, std::vector<String> &files, bool isDirectories) {
	Common::FSNode fsNode(dir_path.GetCStr());
	Common::FSList fsList;

	fsNode.getChildren(fsList,
		isDirectories ? Common::FSNode::kListDirectoriesOnly :
		Common::FSNode::kListFilesOnly);

	for (uint i = 0; i < fsList.size(); ++i)
		files.emplace_back(fsList[i].getName());
	return true;
}

bool GetDirs(const String &dir_path, std::vector<String> &dirs) {
	return GetFilesImpl(dir_path, dirs, true);
}

bool GetFiles(const String &dir_path, std::vector<String> &files) {
	return GetFilesImpl(dir_path, files, false);
}

} // namespace Directory

FindFile::~FindFile() {
	Close();
}

FindFile FindFile::Open(const String &path, const String &wildcard, bool do_file, bool do_dir) {
	FindFile ff;
	ff._folder = Common::FSNode(path.GetCStr());

	Common::FSNode::ListMode mode = Common::FSNode::kListAll;
	if (do_file && !do_dir)
		mode = Common::FSNode::kListFilesOnly;
	else if (!do_file && do_dir)
		mode = Common::FSNode::kListDirectoriesOnly;

	warning("TODO: Wildcard not yet supported - %s", wildcard.GetCStr());

	ff._folder.getChildren(ff._files, mode);
	return ff;
}

void FindFile::Close() {
	_index = 0;
}

bool FindFile::Next() {
	++_index;
	return _index < (int)_files.size();
}

FindFileRecursive::~FindFileRecursive() {
	Close();
}

FindFileRecursive FindFileRecursive::Open(const String &path, const String &wildcard, size_t max_level) {
	FindFile fdir = FindFile::OpenDirs(path);
	FindFile ffile = FindFile::OpenFiles(path, wildcard);
	if (ffile.AtEnd() && fdir.AtEnd())
		return {}; // return invalid object
	FindFileRecursive ff;
	ff._fdir = std::move(fdir);
	ff._ffile = std::move(ffile);
	// Try get the first matching entry
	if (ff._ffile.AtEnd() && !ff.Next())
		return {}; // return invalid object
	ff._maxLevel = max_level;
	ff._fullDir = path;
	ff._curFile = ff._ffile.Current();
	return ff; // success
}

void FindFileRecursive::Close() {
	while (!_fdirs.empty())
		_fdirs.pop();
	_fdir.Close();
	_ffile.Close();
}

bool FindFileRecursive::Next() {
	// Look up for the next file in the current dir
	if (_ffile.Next()) {
		Path::ConcatPaths(_curFile, _curDir, _ffile.Current());
		return true;
	}
	// No more files? Find a directory that still has
	while (_ffile.AtEnd()) {
		// first make sure there are unchecked subdirs left in current dir
		while (_fdir.AtEnd()) { // if not, go up, until found any, or hit the top
			if (!PopDir())
				return false; // no more directories
		}

		// Found an unchecked subdirectory/ies, try opening one
		while (!PushDir(_fdir.Current()) && !_fdir.AtEnd())
			_fdir.Next();
	}
	Path::ConcatPaths(_curFile, _curDir, _ffile.Current());
	return true; // success
}

bool FindFileRecursive::PushDir(const String &sub) {
	if (_maxLevel != SIZE_MAX && (uint32_t)_fdirs.size() == _maxLevel)
		return false; // no more nesting allowed

	String path = Path::ConcatPaths(_fullDir, sub);
	FindFile fdir = FindFile::OpenDirs(path);
	FindFile ffile = FindFile::OpenFiles(path);
	if (ffile.AtEnd() && fdir.AtEnd())
		return false; // dir is empty, or error
	_fdirs.push(std::move(_fdir)); // save previous dir iterator
	_fdir = std::move(fdir);
	_ffile = std::move(ffile);
	_fullDir = path;
	_curDir = Path::ConcatPaths(_curDir, sub);
	return true;
}

bool FindFileRecursive::PopDir() {
	if (_fdirs.empty())
		return false; // no more parent levels
	// restore parent level
	_fdir = std::move(_fdirs.top());
	_fdirs.pop();
	_fullDir = Path::GetParent(_fullDir);
	_curDir = Path::GetParent(_curDir);
	if (_curDir.Compare(".") == 0)
		_curDir = ""; // hotfix for GetParent returning "."
	// advance dir iterator that we just recovered
	_fdir.Next();
	return true;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
