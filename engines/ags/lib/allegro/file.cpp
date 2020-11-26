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
#include "common/textconsole.h"

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

void packfile_password(AL_CONST char *password) {
	error("TODO: packfile_password");
}

PACKFILE *pack_fopen(AL_CONST char *filename, AL_CONST char *mode) {
	error("TODO: pack_fopen");
}

PACKFILE *pack_fopen_vtable(AL_CONST PACKFILE_VTABLE *vtable, void *userdata) {
	error("TODO: pack_fopen_vtable");
}

int pack_fclose(PACKFILE *f) {
	error("TODO: xxx");
}

int pack_fseek(PACKFILE *f, int offset) {
	error("TODO: xxx");
}

PACKFILE *pack_fopen_chunk(PACKFILE *f, int pack) {
	error("TODO: xxx");
}

PACKFILE *pack_fclose_chunk(PACKFILE *f) {
	error("TODO: xxx");
}

int pack_getc(PACKFILE *f) {
	error("TODO: xxx");
}

int pack_putc(int c, PACKFILE *f) {
	error("TODO: xxx");
}

int pack_feof(PACKFILE *f) {
	error("TODO: xxx");
}

int pack_ferror(PACKFILE *f) {
	error("TODO: xxx");
}

int pack_igetw(PACKFILE *f) {
	error("TODO: xxx");
}

long pack_igetl(PACKFILE *f) {
	error("TODO: xxx");
}

int pack_iputw(int w, PACKFILE *f) {
	error("TODO: xxx");
}

long pack_iputl(long l, PACKFILE *f) {
	error("TODO: xxx");
}

int pack_mgetw(PACKFILE *f) {
	error("TODO: xxx");
}

long pack_mgetl(PACKFILE *f) {
	error("TODO: xxx");
}

int pack_mputw(int w, PACKFILE *f) {
	error("TODO: xxx");
}

long pack_mputl(long l, PACKFILE *f) {
	error("TODO: xxx");
}

long pack_fread(void *p, long n, PACKFILE *f) {
	error("TODO: xxx");
}

long pack_fwrite(AL_CONST void *p, long n, PACKFILE *f) {
	error("TODO: xxx");
}

int pack_ungetc(int c, PACKFILE *f) {
	error("TODO: xxx");
}

char *pack_fgets(char *p, int max, PACKFILE *f) {
	error("TODO: xxx");
}

int pack_fputs(AL_CONST char *p, PACKFILE *f) {
	error("TODO: xxx");
}

void *pack_get_userdata(PACKFILE *f) {
	error("TODO: xxx");
}

} // namespace AGS3
