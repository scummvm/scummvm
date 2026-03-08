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

#include "common/archive.h"

#include "common/file.h"
#include "common/hash-str.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "waynesworld/gxlarchive.h"
#include "waynesworld/graphics.h"

namespace WaynesWorld {

GxlArchive::GxlArchive(const Common::String name) : _gxlFile(new Common::File()) {
	const Common::Path filename = Common::Path(name + ".GXL");
	if (!_gxlFile->open(filename)) {
		warning("GlxArchive::GlxArchive(): Could not find the archive file %s", filename.baseName().c_str());
		delete _gxlFile;
		_gxlFile = nullptr;
		return;
	}

	const uint16 sign = _gxlFile->readUint16BE();
	if (sign != 0x01CA) {
		error("Invalid GXL signature");
	}

	// signature (2) + copyright (50) + version (2) + garbage(40)
	_gxlFile->seek(94);
	const uint16 count = _gxlFile->readUint16LE();
	
	GxlHeader header;
	_gxlFile->seek(128);
	
	for (uint i = 0; i < count; i++) {
		_gxlFile->readByte(); // Packing Type
		char baseName[9] = {0};
		char extension[6] = {0};
		_gxlFile->read(baseName, 8);
		_gxlFile->read(extension, 5);
		Common::String fullName = Common::String(baseName);
		fullName.trim();
		fullName += Common::String(extension);
		fullName.trim();
		
		strncpy(header.filename, fullName.c_str(), 12);
		header.filename[12] = 0;
		
		header.offset = _gxlFile->readUint32LE();
		header.size = _gxlFile->readUint32LE();

		_gxlFile->readUint32LE(); // date and time of the file

		// warning("file: %s offset: %d  size: %d", header.filename, header.offset, header.size);

		_headers[header.filename].reset(new GxlHeader(header));
	}
}

GxlArchive::~GxlArchive() {
	delete _gxlFile;
}

bool GxlArchive::hasFile(const Common::Path &path) const {
	return _headers.contains(path);
}

int GxlArchive::listMembers(Common::ArchiveMemberList &list) const {
	int matches = 0;

	GxlHeadersMap::const_iterator it = _headers.begin();
	for ( ; it != _headers.end(); ++it) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(Common::Path(it->_value->filename), *this)));
		matches++;
	}

	return matches;
}

const Common::ArchiveMemberPtr GxlArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *GxlArchive::createReadStreamForMember(const Common::Path &path) const {
	if (!_gxlFile || !_headers.contains(path)) {
		return nullptr;
	}

	GxlHeader *hdr = _headers[path].get();

	_gxlFile->seek(hdr->offset, SEEK_SET);

	byte *data = (byte *)malloc(hdr->size);
	assert(data);

	int32 len = _gxlFile->read(data, hdr->size);
	assert(len == hdr->size);

	return new Common::MemoryReadStream(data, hdr->size, DisposeAfterUse::YES);
}

Image::PCXDecoder *GxlArchive::loadImage(const char *filename) {
	const char *posDot = strrchr(filename, '.');
	Common::String searchName;
	if (!posDot)
		searchName = Common::String::format("%s.PCX", filename);
	else
		searchName = Common::String(filename);
	
	Common::Path pathName = Common::Path(searchName);
	if (!hasFile(pathName)) {
		error("loadImage() Could not find '%s'", filename);
	}

	Image::PCXDecoder *pcx = new Image::PCXDecoder();
	Common::SeekableReadStream *pcxStr = createReadStreamForMember(Common::Path(filename));
	if (!pcx->loadStream(*pcxStr))
		error("loadImage() Could not process '%s'", filename);

	delete pcxStr;
	
	return pcx;
}

WWSurface *GxlArchive::loadSurfaceIntern(const char *filename) {
	Image::PCXDecoder *imageDecoder = loadImage(filename);
	WWSurface *surface = new WWSurface(imageDecoder->getSurface());
	delete imageDecoder;
	return surface;
}

WWSurface *GxlArchive::loadRoomSurface(const char *filename) {
	return loadSurfaceIntern(filename);
}

WWSurface *GxlArchive::loadSurface(const char *filename) {
	return loadSurfaceIntern(filename);
}
} // End of namespace WaynesWorld
