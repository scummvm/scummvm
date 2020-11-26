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

#include "ags/lib/allegro/alconfig.h"

namespace AGS3 {

#define F_READ          "r"
#define F_WRITE         "w"
#define F_READ_PACKED   "rp"
#define F_WRITE_PACKED  "wp"
#define F_WRITE_NOPACK  "w!"

#define F_BUF_SIZE      4096           /* 4K buffer for caching data */
#define F_PACK_MAGIC    0x736C6821L    /* magic number for packed files */
#define F_NOPACK_MAGIC  0x736C682EL    /* magic number for autodetect */
#define F_EXE_MAGIC     0x736C682BL    /* magic number for appended data */

struct _al_normal_packfile_details {
	int hndl;                           /* DOS file handle */
	int flags;                          /* PACKFILE_FLAG_* constants */
	unsigned char *buf_pos;             /* position in buffer */
	int buf_size;                       /* number of bytes in the buffer */
	long todo;                          /* number of bytes still on the disk */
	struct PACKFILE *parent;            /* nested, parent file */
	struct LZSS_PACK_DATA *pack_data;   /* for LZSS compression */
	struct LZSS_UNPACK_DATA *unpack_data; /* for LZSS decompression */
	char *filename;                     /* name of the file */
	char *passdata;                     /* encryption key data */
	char *passpos;                      /* current key position */
	unsigned char buf[F_BUF_SIZE];      /* the actual data buffer */
};

struct PACKFILE_VTABLE {
	AL_METHOD(int, pf_fclose, (void *userdata));
	AL_METHOD(int, pf_getc, (void *userdata));
	AL_METHOD(int, pf_ungetc, (int c, void *userdata));
	AL_METHOD(long, pf_fread, (void *p, long n, void *userdata));
	AL_METHOD(int, pf_putc, (int c, void *userdata));
	AL_METHOD(long, pf_fwrite, (AL_CONST void *p, long n, void *userdata));
	AL_METHOD(int, pf_fseek, (void *userdata, int offset));
	AL_METHOD(int, pf_feof, (void *userdata));
	AL_METHOD(int, pf_ferror, (void *userdata));
};

/**
 * Allegro file class
 */
struct PACKFILE {
	AL_CONST PACKFILE_VTABLE *vtable;
	void *userdata;
	int is_normal_packfile;

	/* The following is only to be used for the "normal" PACKFILE vtable,
	 * i.e. what is implemented by Allegro itself. If is_normal_packfile is
	 * false then the following is not even allocated. This must be the last
	 * member in the structure.
	 */
	struct _al_normal_packfile_details normal;
};

extern char *fix_filename_case(char *path);
extern char *fix_filename_slashes(char *path);
extern char *append_filename(char *dest, const char *path, const char *filename, int size);
extern char *canonicalize_filename(char *dest, const char *filename, int size);
extern char *make_relative_filename(char *dest, const char *path, const char *filename, int size);
extern int is_relative_filename(const char *filename);

AL_FUNC(void, packfile_password, (AL_CONST char *password));
AL_FUNC(PACKFILE *, pack_fopen, (AL_CONST char *filename, AL_CONST char *mode));
AL_FUNC(PACKFILE *, pack_fopen_vtable, (AL_CONST PACKFILE_VTABLE *vtable, void *userdata));
AL_FUNC(int, pack_fclose, (PACKFILE *f));
AL_FUNC(int, pack_fseek, (PACKFILE *f, int offset));
AL_FUNC(PACKFILE *, pack_fopen_chunk, (PACKFILE *f, int pack));
AL_FUNC(PACKFILE *, pack_fclose_chunk, (PACKFILE *f));
AL_FUNC(int, pack_getc, (PACKFILE *f));
AL_FUNC(int, pack_putc, (int c, PACKFILE *f));
AL_FUNC(int, pack_feof, (PACKFILE *f));
AL_FUNC(int, pack_ferror, (PACKFILE *f));
AL_FUNC(int, pack_igetw, (PACKFILE *f));
AL_FUNC(long, pack_igetl, (PACKFILE *f));
AL_FUNC(int, pack_iputw, (int w, PACKFILE *f));
AL_FUNC(long, pack_iputl, (long l, PACKFILE *f));
AL_FUNC(int, pack_mgetw, (PACKFILE *f));
AL_FUNC(long, pack_mgetl, (PACKFILE *f));
AL_FUNC(int, pack_mputw, (int w, PACKFILE *f));
AL_FUNC(long, pack_mputl, (long l, PACKFILE *f));
AL_FUNC(long, pack_fread, (void *p, long n, PACKFILE *f));
AL_FUNC(long, pack_fwrite, (AL_CONST void *p, long n, PACKFILE *f));
AL_FUNC(int, pack_ungetc, (int c, PACKFILE *f));
AL_FUNC(char *, pack_fgets, (char *p, int max, PACKFILE *f));
AL_FUNC(int, pack_fputs, (AL_CONST char *p, PACKFILE *f));
AL_FUNC(void *, pack_get_userdata, (PACKFILE *f));

} // namespace AGS3

#endif
