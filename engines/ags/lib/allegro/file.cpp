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

#include "ags/lib/allegro/file.h"
#include "common/str.h"

namespace AGS3 {

char *fix_filename_case(char *path) {
	return path;
}

char *fix_filename_slashes(char *path) {
	return path;
}

char *append_filename(char *dest, const char *path, const char *filename, int size) {
	strncpy(dest, path, size);
	strncat(dest, filename, size);
	return dest;
}

char *canonicalize_filename(char *dest, const char *filename, int size) {
	strncpy(dest, filename, size);
	return dest;
}

char *make_relative_filename(char *dest, const char *path, const char *filename, int size) {
	strncpy(dest, filename, size);
	return dest;
}

int is_relative_filename(const char *filename) {
	Common::String fname(filename);
	return !fname.contains('/') && !fname.contains('\\') ? 0 : -1;
}

} // namespace AGS3
