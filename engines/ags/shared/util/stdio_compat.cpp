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

#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/core/platform.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/textconsole.h"

namespace AGS3 {

int ags_fseek(Common::Stream *stream, file_off_t offset, int whence) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(stream);

	if (rs)
		return rs->seek(offset, whence);
	else if (ws)
		return ws->seek(offset, whence);
	else
		error("Seek on null stream");
}

file_off_t ags_ftell(Common::Stream *stream) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);
	return rs->pos();
}

Common::FSNode getFSNOde(const char *path) {
	Common::FSNode node(ConfMan.get("path"));
	Common::String filePath(path);

	// If it's the root game folder, return the node for it
	if (filePath.empty() || filePath == "." || filePath == "./")
		return node;

	if (filePath.hasPrefix("./"))
		filePath = Common::String(filePath.c_str() + 2);

	// Iterate through any further subfolders or filename
	size_t separator;
	while ((separator = filePath.find('/')) != Common::String::npos) {
		node = node.getChild(filePath.substr(0, separator));
		filePath = Common::String(filePath.c_str() + separator + 1);
	}

	if (!filePath.empty())
		node = node.getChild(filePath);

	return node;
}

int  ags_file_exists(const char *path) {
	Common::FSNode node = getFSNOde(path);
	return node.exists() && !node.isDirectory()  ? 1 : 0;
}

int ags_directory_exists(const char *path) {
	Common::FSNode node = getFSNOde(path);
	return node.exists() && node.isDirectory() ? 1 : 0;
}

int ags_path_exists(const char *path) {
	Common::FSNode node = getFSNOde(path);
	return node.exists() ? 1 : 0;
}

file_off_t ags_file_size(const char *path) {
	Common::FSNode node = getFSNOde(path);
	Common::File f;

	return f.open(node) ? f.size() : (file_off_t )-1;
}

} // namespace AGS3
