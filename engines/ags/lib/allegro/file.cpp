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

#include "ags/lib/allegro/file.h"
#include "common/file.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace AGS3 {

PACKFILE *PACKFILE::pack_fopen_chunk(int pack) {
	error("TODO: pack_fopen_chunk is not yet supported");
}

PACKFILE *PACKFILE::pack_fclose_chunk() {
	error("TODO: pack_fclose_chunk is not yet supported");
}

int PACKFILE::pack_igetw() {
	byte buf[2];
	return pack_fread(buf, 2) == 2 ? READ_LE_UINT16(buf) : 0;
}

int32_t PACKFILE::pack_igetl() {
	byte buf[4];
	return pack_fread(buf, 4) == 4 ? READ_LE_UINT32(buf) : 0;
}

int PACKFILE::pack_iputw(int w) {
	byte buf[2];
	WRITE_LE_UINT16(buf, w);
	pack_fwrite(buf, 2);
	return 0;
}

int32_t PACKFILE::pack_iputl(int32_t l) {
	byte buf[4];
	WRITE_LE_UINT32(buf, l);
	pack_fwrite(buf, 4);
	return 0;
}

int PACKFILE::pack_mgetw() {
	byte buf[2];
	return pack_fread(buf, 2) == 2 ? READ_BE_UINT16(buf) : 0;
}

int32_t PACKFILE::pack_mgetl() {
	byte buf[4];
	return pack_fread(buf, 4) == 4 ? READ_BE_UINT32(buf) : 0;
}

int PACKFILE::pack_mputw(int w) {
	byte buf[2];
	WRITE_BE_UINT16(buf, 2);
	pack_fwrite(buf, 2);
	return 0;
}

int32_t PACKFILE::pack_mputl(int32_t l) {
	byte buf[4];
	WRITE_BE_UINT16(buf, 4);
	pack_fwrite(buf, 4);
	return 0;
}

char *PACKFILE::pack_fgets(char *p, int max) {
	int c;
	char *dest = p;

	while ((c = pack_getc()) != 0 && !pack_feof() && max-- > 0) {
		*dest++ = c;
	}

	return p;
}

int PACKFILE::pack_fputs(AL_CONST char *p) {
	pack_fwrite(p, strlen(p));
	pack_putc(0);
	return 0;
}

/*------------------------------------------------------------------*/

/* Use strictly UTF-8 encoding for the file paths
 */
#define U_CURRENT U_UTF8
#define ugetc     utf8_getc
#define ugetx     utf8_getx
#define ugetxc    utf8_getx
#define usetc     utf8_setc
#define uwidth    utf8_width
#define ucwidth   utf8_cwidth
#define uisok     utf8_isok

void set_filename_encoding(int) {
	// No implementation
}

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
	// ScummVM doesn't have absolute paths
	return true;
}

/*------------------------------------------------------------------*/

void packfile_password(AL_CONST char *password) {
	error("TODO: packfile_password");
}

PACKFILE *pack_fopen(AL_CONST char *filename, AL_CONST char *mode) {
	assert(!strcmp(mode, "r") || !strcmp(mode, "rb"));

	Common::File *f = new Common::File();
	if (f->open(filename)) {
		return new ScummVMPackFile(f);

	} else {
		delete f;
		return nullptr;
	}
}

PACKFILE *pack_fopen_vtable(AL_CONST PACKFILE_VTABLE *vtable, void *userdata) {
	return new VTablePackFile(vtable, userdata);
}

int pack_fclose(PACKFILE *f) {
	f->close();
	delete f;
	return 0;
}

int pack_fseek(PACKFILE *f, int offset) {
	return f->pack_fseek(offset);
}

PACKFILE *pack_fopen_chunk(PACKFILE *f, int pack) {
	return f->pack_fopen_chunk(pack);
}

PACKFILE *pack_fclose_chunk(PACKFILE *f) {
	return f->pack_fclose_chunk();
}

int pack_getc(PACKFILE *f) {
	return f->pack_getc();
}

int pack_putc(int c, PACKFILE *f) {
	return f->pack_putc(c);
}

int pack_feof(PACKFILE *f) {
	return f->pack_feof();
}

int pack_ferror(PACKFILE *f) {
	return f->pack_ferror();
}

int pack_igetw(PACKFILE *f) {
	error("TODO: xxx");
}

int32_t pack_igetl(PACKFILE *f) {
	return f->pack_igetl();
}

int pack_iputw(int w, PACKFILE *f) {
	return f->pack_iputw(w);
}

int32_t pack_iputl(int32_t l, PACKFILE *f) {
	return f->pack_iputl(l);
}

int pack_mgetw(PACKFILE *f) {
	return f->pack_mgetw();
}

int32_t pack_mgetl(PACKFILE *f) {
	return f->pack_mgetl();
}

int pack_mputw(int w, PACKFILE *f) {
	return f->pack_mputw(w);
}

int32_t pack_mputl(int32_t l, PACKFILE *f) {
	return f->pack_mputl(l);
}

long pack_fread(void *p, long n, PACKFILE *f) {
	return f->pack_fread(p, n);
}

long pack_fwrite(AL_CONST void *p, long n, PACKFILE *f) {
	return f->pack_fwrite(p, n);
}

int pack_ungetc(int c, PACKFILE *f) {
	return f->pack_ungetc(c);
}

char *pack_fgets(char *p, int max, PACKFILE *f) {
	return f->pack_fgets(p, max);
}

int pack_fputs(AL_CONST char *p, PACKFILE *f) {
	return f->pack_fputs(p);
}

void *pack_get_userdata(PACKFILE *f) {
	return f->pack_get_userdata();
}

} // namespace AGS3
