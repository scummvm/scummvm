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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "ultima8/misc/pent_include.h"

#include "ultima8/filesys/zip_file.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/file_system.h"

// unzip API
#include "ultima8/filesys/zip/unzip.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(NamedArchiveFile, ArchiveFile)

DEFINE_RUNTIME_CLASSTYPE_CODE(ZipFile, NamedArchiveFile)



// ioapi IDataSource wrapper functions

static voidpf ids_open(voidpf opaque, const char *filename, int mode);
static uLong ids_read(voidpf opaque, voidpf stream, void *buf, uLong size);
static uLong ids_write(voidpf opaque, voidpf stream,
                       const void *buf, uLong size);
static long ids_tell(voidpf opaque, voidpf stream);
static long ids_seek(voidpf opaque, voidpf stream, uLong offset, int origin);
static int ids_close(voidpf opaque, voidpf stream);
static int ids_error(voidpf opaque, voidpf stream);

PentZip::zlib_filefunc_def IDS_filefunc_templ = {
	ids_open, ids_read, ids_write, ids_tell, ids_seek, ids_close, ids_error, 0
};


ZipFile::ZipFile(IDataSource *ds_) {
	ds = ds_;
	PentZip::zlib_filefunc_def filefuncs = IDS_filefunc_templ;
	filefuncs.opaque = static_cast<void *>(ds);

	// filefuncs contains the datasource, so no need to actually use a path
	PentZip::unzFile unzfile = PentZip::unzOpen2("", &filefuncs);

	valid = (unzfile != 0);

	unzipfile = static_cast<void *>(unzfile);

	if (valid) {
		valid = readMetadata();
		if (!valid) PentZip::unzClose(unzfile);
	}
}


ZipFile::~ZipFile() {
	if (valid) {
		PentZip::unzFile unzfile = static_cast<PentZip::unzFile>(unzipfile);
		PentZip::unzClose(unzfile);
	}
	delete ds;
}

//static
bool ZipFile::isZipFile(IDataSource *ids) {
	PentZip::zlib_filefunc_def filefuncs = IDS_filefunc_templ;
	filefuncs.opaque = static_cast<void *>(ids);

	// filefuncs contains the datasource, so no need to actually use a path
	PentZip::unzFile unzfile = PentZip::unzOpen2("", &filefuncs);

	if (unzfile != 0) {
		PentZip::unzClose(unzfile);
		return true;
	}

	return false;
}

bool ZipFile::readMetadata() {
	PentZip::unzFile unzfile = static_cast<PentZip::unzFile>(unzipfile);

	PentZip::unz_global_info ginfo;
	if (PentZip::unzGetGlobalInfo(unzfile, &ginfo) != UNZ_OK) return false;

	count = ginfo.number_entry;

	globalComment = "";
	if (ginfo.size_comment > 0) {
		char *commentbuf = new char[ginfo.size_comment + 1];
		int c = PentZip::unzGetGlobalComment(unzfile,
		                                     commentbuf, ginfo.size_comment + 1);
		if (c > 0) globalComment = commentbuf;
		delete[] commentbuf;
	}

	PentZip::unz_file_info info;
	char name[256];

	bool done = (PentZip::unzGoToFirstFile(unzfile) != UNZ_OK);

	while (!done) {
		int ret = PentZip::unzGetCurrentFileInfo(unzfile, &info, name, 256,
		          0, 0, 0, 0);
		if (ret != UNZ_OK) continue;

		std::string filename = name;
		storeIndexedName(filename);
		sizes[filename] = info.uncompressed_size;

		done = (PentZip::unzGoToNextFile(unzfile) != UNZ_OK);
	}

	return true;
}

bool ZipFile::exists(const std::string &name) {
	std::map<Common::String, uint32>::iterator iter;
	iter = sizes.find(name);
	return (iter != sizes.end());
}

uint32 ZipFile::getSize(const std::string &name) {
	std::map<Common::String, uint32>::iterator iter;
	iter = sizes.find(name);
	if (iter == sizes.end()) return 0;
	return (iter->_value);
}

uint8 *ZipFile::getObject(const std::string &name, uint32 *sizep) {
	PentZip::unzFile unzfile = static_cast<PentZip::unzFile>(unzipfile);
	if (sizep) *sizep = 0;

	if (PentZip::unzLocateFile(unzfile, name.c_str(), 1) != UNZ_OK) return 0;

	PentZip::unz_file_info info;
	uint8 *buf = 0;

	if (PentZip::unzGetCurrentFileInfo(unzfile, &info, 0, 0, 0, 0, 0, 0)
	        != UNZ_OK)
		return 0;

	if (PentZip::unzOpenCurrentFile(unzfile) != UNZ_OK) return 0;
	uint32 size = info.uncompressed_size;

	buf = new uint8[size];

	if (PentZip::unzReadCurrentFile(unzfile, buf, size) < static_cast<int>(size)) {
		delete[] buf;
		return 0;
	}

	if (PentZip::unzCloseCurrentFile(unzfile) != UNZ_OK) {
		delete[] buf;
		return 0;
	}

	if (sizep) *sizep = size;

	return buf;
}



// ------------

static voidpf ids_open(voidpf opaque, const char *filename, int mode) {
	// read-only, for now
	if (mode != (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_EXISTING))
		return 0;

	// opaque is actually the IDataSource*
	return opaque;
}

static uLong ids_read(voidpf opaque, voidpf stream, void *buf, uLong size) {
	IDataSource *ids = static_cast<IDataSource *>(stream);
	return ids->read(buf, size);
}

static uLong ids_write(voidpf opaque, voidpf stream,
                       const void *buf, uLong size) {
	return 0;
}

static long ids_tell(voidpf opaque, voidpf stream) {
	IDataSource *ids = static_cast<IDataSource *>(stream);
	return ids->getPos();
}

static long ids_seek(voidpf opaque, voidpf stream, uLong offset, int origin) {
	IDataSource *ids = static_cast<IDataSource *>(stream);
	switch (origin) {
	case ZLIB_FILEFUNC_SEEK_CUR:
		ids->skip(offset);
		break;
	case ZLIB_FILEFUNC_SEEK_END:
		ids->seek(ids->getSize() + offset);
		break;
	case ZLIB_FILEFUNC_SEEK_SET:
		ids->seek(offset);
		break;
	default:
		return -1;
	}
	return 0;
}

static int ids_close(voidpf opaque, voidpf stream) {
	return 0;
}

static int ids_error(voidpf opaque, voidpf stream) {
	return 0;
}

} // End of namespace Ultima8
