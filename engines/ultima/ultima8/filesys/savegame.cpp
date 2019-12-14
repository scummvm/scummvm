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
#include "ultima/ultima8/filesys/savegame.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima8 {

SavegameReader::SavegameReader(IDataSource *ds) : _file(ds) {
}

SavegameReader::~SavegameReader() {
}

uint32 SavegameReader::getVersion() {
	IDataSource *ids = getDataSource("VERSION");
	if (!ids || ids->getSize() != 4) return 0;

	uint32 version = ids->read4();
	delete ids;

	return version;
}

std::string SavegameReader::getDescription() const {
	return _comments;
}

IDataSource *SavegameReader::getDataSource(const std::string &name) {
	assert(_index.contains(name));

	const FileEntry &fe = _index[name];
	uint8 *data = (uint8 *)malloc(fe._size);
	return new IBufferDataSource(data, fe._size, false, true);
}


SavegameWriter::SavegameWriter(ODataSource *ds) : _file(ds) {
/*
	PentZip::zlib_filefunc_def filefuncs = ODS_filefunc_templ;
	filefuncs.opaque = static_cast<void *>(ds);

	PentZip::zipFile zfile = PentZip::zipOpen2("", 0, 0, &filefuncs);
	zipfile = static_cast<void *>(zfile);
	*/
}

SavegameWriter::~SavegameWriter() {
	if (_file)
		delete _file;
}

bool SavegameWriter::finish() {
/*
	PentZip::zipFile zfile = static_cast<PentZip::zipFile>(zipfile);
	zipfile = 0;
	if (PentZip::zipClose(zfile, comment.c_str()) != ZIP_OK) return false;
*/
	return true;
}


bool SavegameWriter::writeFile(const std::string &name, const uint8 *data, uint32 size) {
	_index.push_back(FileEntry());

	FileEntry &fe = _index.back();
	fe._name = name;
	fe.write(data, size);

	return true;
}

bool SavegameWriter::writeFile(const std::string &name, OAutoBufferDataSource *ods) {
	return writeFile(name, ods->getBuf(), ods->getSize());
}

bool SavegameWriter::writeVersion(uint32 version) {
	uint8 buf[4];
	buf[0] = version & 0xFF;
	buf[1] = (version >> 8) & 0xFF;
	buf[2] = (version >> 16) & 0xFF;
	buf[3] = (version >> 24) & 0xFF;
	return writeFile("VERSION", buf, 4);
}

bool SavegameWriter::writeDescription(const std::string &desc) {
	_comments = desc;
	return true;
}

} // End of namespace Ultima8
