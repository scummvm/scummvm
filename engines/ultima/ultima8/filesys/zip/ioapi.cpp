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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/filesys/zip/ioapi.h"
#include "common/file.h"

namespace Ultima8 {
namespace PentZip {

voidpf ZCALLBACK fopen_file_func(voidpf opaque, const char* filename, int mode);
uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream,
	void *buf, uLong size);
uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream,
	const void *buf, uLong size);
long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream);
long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream,
		uLong offset, int origin);
int ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream);
int ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream);


voidpf ZCALLBACK fopen_file_func(voidpf opaque,const char* filename,int mode) {
	if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ) {
		Common::File *f = new Common::File();
		if (f->open(filename))
			return f;
		delete f;
	} else if (mode & ZLIB_FILEFUNC_MODE_EXISTING) {
		error("Appending not supported");
	} else if (mode & ZLIB_FILEFUNC_MODE_CREATE) {
		error("Writing not supported");
	}

	return 0;
}


uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, void *buf, uLong size) {
    return static_cast<Common::ReadStream *>(stream)->read(buf, size);
}


uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, const void * buf, uLong size) {
	return static_cast<Common::WriteStream *>(stream)->write(buf, size);
}

long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
	return static_cast<Common::SeekableReadStream *>(stream)->pos();
}

long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream,
		uLong offset, int origin) {
    int fseek_origin=0;
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
    default: return -1;
    }

	ret = 0;
    static_cast<Common::SeekableReadStream *>(stream)->seek(offset, fseek_origin);
    return ret;
}

int ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
	Common::Stream *file = static_cast<Common::Stream *>(stream);
	delete file;
	return 0;
}

int ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
//    return ferror((FILE *)stream);
	return 0;
}

void fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = NULL;
}

} // End of namespace PentZip
} // End of namespace Ultima8
