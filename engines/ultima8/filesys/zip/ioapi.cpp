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

#include "ultima8/misc/pent_include.h"
#include "ultima8/filesys/zip/ioapi.h"
#include <zlib.h>

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

namespace PentZip {

voidpf ZCALLBACK fopen_file_func(
    voidpf opaque,
    const char *filename,
    int mode);

uLong ZCALLBACK fread_file_func(
    voidpf opaque,
    voidpf stream,
    void *buf,
    uLong size);

uLong ZCALLBACK fwrite_file_func(
    voidpf opaque,
    voidpf stream,
    const void *buf,
    uLong size);

long ZCALLBACK ftell_file_func(
    voidpf opaque,
    voidpf stream);

long ZCALLBACK fseek_file_func(
    voidpf opaque,
    voidpf stream,
    uLong offset,
    int origin);

int ZCALLBACK fclose_file_func(
    voidpf opaque,
    voidpf stream);

int ZCALLBACK ferror_file_func(
    voidpf opaque,
    voidpf stream);


voidpf ZCALLBACK fopen_file_func(voidpf opaque, const char *filename, int mode) {
	FILE *file = NULL;
	const char *mode_fopen = NULL;
	if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ)
		mode_fopen = "rb";
	else if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
		mode_fopen = "r+b";
	else if (mode & ZLIB_FILEFUNC_MODE_CREATE)
		mode_fopen = "wb";

	if ((filename != NULL) && (mode_fopen != NULL))
		file = fopen(filename, mode_fopen);
	return file;
}


uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream,
                                void *buf, uLong size) {
	uLong ret;
	ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
	return ret;
}


uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream,
                                 const void *buf, uLong size) {
	uLong ret;
	ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
	return ret;
}

long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
	long ret;
	ret = ftell((FILE *)stream);
	return ret;
}

long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream,
                               uLong offset, int origin) {
	int fseek_origin = 0;
	long ret;
	switch (origin) {
	case ZLIB_FILEFUNC_SEEK_CUR :
		fseek_origin = SEEK_CUR;
		break;
	case ZLIB_FILEFUNC_SEEK_END :
		fseek_origin = SEEK_END;
		break;
	case ZLIB_FILEFUNC_SEEK_SET :
		fseek_origin = SEEK_SET;
		break;
	default:
		return -1;
	}
	ret = 0;
	fseek((FILE *)stream, offset, fseek_origin);
	return ret;
}

int ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
	int ret;
	ret = fclose((FILE *)stream);
	return ret;
}

int ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
	int ret;
	ret = ferror((FILE *)stream);
	return ret;
}

void fill_fopen_filefunc(zlib_filefunc_def *pzlib_filefunc_def) {
	pzlib_filefunc_def->zopen_file = fopen_file_func;
	pzlib_filefunc_def->zread_file = fread_file_func;
	pzlib_filefunc_def->zwrite_file = fwrite_file_func;
	pzlib_filefunc_def->ztell_file = ftell_file_func;
	pzlib_filefunc_def->zseek_file = fseek_file_func;
	pzlib_filefunc_def->zclose_file = fclose_file_func;
	pzlib_filefunc_def->zerror_file = ferror_file_func;
	pzlib_filefunc_def->opaque = NULL;
}

}
