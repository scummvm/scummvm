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

#ifndef AGS_LIB_ALLEGRO_FILE_H
#define AGS_LIB_ALLEGRO_FILE_H

#include "ags/lib/allegro/alconfig.h"
#include "common/file.h"

namespace AGS3 {

#define F_READ          "r"
#define F_WRITE         "w"

#define F_BUF_SIZE      4096           /* 4K buffer for caching data */

struct _al_normal_packfile_details {
	int hndl;                           /* DOS file handle */
	int flags;                          /* PACKFILE_FLAG_* constants */
	unsigned char *buf_pos;             /* position in buffer */
	int buf_size;                       /* number of bytes in the buffer */
	long todo;                          /* number of bytes still on the disk */
	struct PACKFILE *parent;            /* nested, parent file */
	char *filename;                     /* name of the file */
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
	virtual ~PACKFILE() {
		close();
	}

	virtual void close() {}
	virtual int pack_fseek(int offset) = 0;
	virtual int pack_getc() = 0;
	virtual int pack_putc(int c) = 0;
	virtual int pack_ungetc(int c) = 0;
	virtual long pack_fread(void *p, long n) = 0;
	virtual long pack_fwrite(AL_CONST void *p, long n) = 0;
	virtual int pack_feof() = 0;
	virtual int pack_ferror() = 0;
	virtual void *pack_get_userdata() const {
		return nullptr;
	}

	PACKFILE *pack_fopen_chunk(int pack);
	PACKFILE *pack_fclose_chunk();
	int pack_igetw();
	long pack_igetl();
	int pack_iputw(int w);
	long pack_iputl(long l);
	int pack_mgetw();
	long pack_mgetl();
	int pack_mputw(int w);
	long pack_mputl(long l);
	char *pack_fgets(char *p, int max);
	int pack_fputs(AL_CONST char *p);
	};

struct ScummVMPackFile : public PACKFILE {
public:
	Common::SeekableReadStream *_stream;

	ScummVMPackFile(Common::SeekableReadStream *rs) : PACKFILE(), _stream(rs) {
	}

	virtual ~ScummVMPackFile() {}

	void close() override {
		delete _stream;
		_stream = nullptr;
	}

	int pack_fseek(int offset) override {
		return _stream->seek(offset, SEEK_CUR);
	}

	int pack_getc() override {
		return _stream->readByte();
	}

	int pack_putc(int c) override {
		error("pack_putc is not yet supported");
	}

	int pack_ungetc(int c) override {
		_stream->seek(-1, SEEK_CUR);
		return 0;
	}

	long pack_fread(void *p, long n) override {
		return _stream->read(p, n);
	}

	long pack_fwrite(AL_CONST void *p, long n) override {
		error("pack_fwrite is not yet supported");
	}

	int pack_feof() override {
		return _stream->eos();
	}

	int pack_ferror() override {
		return _stream->err();
	}
	};

	struct VTablePackFile : public PACKFILE {
	AL_CONST PACKFILE_VTABLE *_vTable;
	void *_userData;

	VTablePackFile(AL_CONST PACKFILE_VTABLE *vTable, void *userData) :
		_vTable(vTable), _userData(userData) {
	}

	void close() override {
		_vTable->pf_fclose(_userData);
	}

	int pack_fseek(int offset) override {
		return _vTable->pf_fseek(_userData, offset);
	}

	int pack_getc() override {
		return _vTable->pf_getc(_userData);
	}

	int pack_putc(int c) override {
		return _vTable->pf_putc(c, _userData);
	}

	int pack_ungetc(int c) override {
		return _vTable->pf_ungetc(c, _userData);
	}

	long pack_fread(void *p, long n) override {
		return _vTable->pf_fread(p, n, _userData);
	}

	long pack_fwrite(AL_CONST void *p, long n) override {
		return _vTable->pf_fwrite(p, n, _userData);
	}

	int pack_feof() override {
		return _vTable->pf_feof(_userData);
	}

	int pack_ferror() override {
		return _vTable->pf_ferror(_userData);
	}

	void *pack_get_userdata() const override {
		return _userData;
	}
};

extern void set_filename_encoding(int);
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
