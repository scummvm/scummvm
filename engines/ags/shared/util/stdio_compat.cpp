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

int  ags_file_exists(const char *path) {
	Common::FSNode fs(path);
	return fs.exists() && !fs.isDirectory()  ? 0 : -1;
}

int ags_directory_exists(const char *path) {
	Common::FSNode fs(path);
	return fs.exists() && fs.isDirectory() ? 0 : -1;
}

int ags_path_exists(const char *path) {
	return Common::FSNode(path).exists() ? 0 : -1;
}

file_off_t ags_file_size(const char *path) {
	Common::File f;

	return f.open(path) ? f.size() : (file_off_t )-1;
}

} // namespace AGS3
