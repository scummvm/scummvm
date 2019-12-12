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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/filesys/zip_file.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/file_system.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(NamedArchiveFile, ArchiveFile)

DEFINE_RUNTIME_CLASSTYPE_CODE(ZipFile, NamedArchiveFile)

ZipFile::ZipFile(IDataSource *ds_) {
#ifdef TODO
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
#endif
}


ZipFile::~ZipFile() {
#ifdef TODO
	if (valid) {
		PentZip::unzFile unzfile = static_cast<PentZip::unzFile>(unzipfile);
		PentZip::unzClose(unzfile);
	}
	delete ds;
#endif
}

//static
bool ZipFile::isZipFile(IDataSource *ids) {
#ifdef TODO
	PentZip::zlib_filefunc_def filefuncs = IDS_filefunc_templ;
	filefuncs.opaque = static_cast<void *>(ids);

	// filefuncs contains the datasource, so no need to actually use a path
	PentZip::unzFile unzfile = PentZip::unzOpen2("", &filefuncs);

	if (unzfile != 0) {
		PentZip::unzClose(unzfile);
		return true;
	}
#endif
	return false;
}

bool ZipFile::readMetadata() {
#ifdef TODO
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
#endif
	return true;
}

bool ZipFile::exists(const std::string &name) {
#ifdef TODO
	std::map<Common::String, uint32>::iterator iter;
	iter = sizes.find(name);
	return (iter != sizes.end());
#endif
	return false;
}

uint32 ZipFile::getSize(const std::string &name) {
#ifdef TODO
	std::map<Common::String, uint32>::iterator iter;
	iter = sizes.find(name);
	if (iter == sizes.end()) return 0;
	return (iter->_value);
#else
	return 0;
#endif
}

uint8 *ZipFile::getObject(const std::string &name, uint32 *sizep) {
#ifdef TODO
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
#else
	return nullptr;
#endif
}

} // End of namespace Ultima8
