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

#include "ags/shared/core/platform.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/stdio_compat.h"
#include "common/config-manager.h"
#include "common/fs.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

const char *SAVE_FOLDER_PREFIX = "/saves/";

namespace Directory {

bool CreateDirectory(const String &path) {
	return Common::FSNode(path.GetNullableCStr()).createDirectory();
}

bool CreateAllDirectories(const String &parent, const String &path) {
	if (path == SAVE_FOLDER_PREFIX)
		// ScummVM save folder doesn't need creating
		return true;

	if (!ags_directory_exists(parent.GetCStr()))
		return false;
	if (path.IsEmpty())
		return true;
	if (!Path::IsSameOrSubDir(parent, path))
		return false;

	String sub_path = Path::MakeRelativePath(parent, path);
	String make_path = parent;
	std::vector<String> dirs = sub_path.Split('/');
	for (const String &dir : dirs) {
		if (dir.IsEmpty() || dir.Compare(".") == 0) continue;
		make_path.AppendChar('/');
		make_path.Append(dir);
		if (!CreateDirectory(make_path))
			return false;
	}
	return true;
}

String SetCurrentDirectory(const String &path) {
	warning("TODO: SetCurrentDirectory: %s", path.GetNullableCStr());
//	chdir(path);
//	return GetCurrentDirectory();
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
