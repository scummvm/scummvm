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

#include "common/config-manager.h"
#include "common/fs.h"
#include "ags/shared/core/platform.h"
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
		if ((cur - sect == 1) && (*cur == '.' || *cur == '/' || *cur == PATH_ALT_SEPARATOR)) {
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

} // namespace Directory

} // namespace Shared
} // namespace AGS
} // namespace AGS3
