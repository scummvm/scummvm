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

#ifndef AGS_LIB_ALLEGRO_FILE_H
#define AGS_LIB_ALLEGRO_FILE_H

namespace AGS3 {

struct PACKFILE {
	// TODO: PACKFILE handling
	int dummy;
};

extern char *fix_filename_case(char *path);
extern char *fix_filename_slashes(char *path);
extern char *append_filename(char *dest, const char *path, const char *filename, int size);
extern char *canonicalize_filename(char *dest, const char *filename, int size);
extern char *make_relative_filename(char *dest, const char *path, const char *filename, int size);
extern int is_relative_filename(const char *filename);

} // namespace AGS3

#endif
