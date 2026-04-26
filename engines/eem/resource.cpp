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

#include "common/compression/dcl.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "graphics/pixelformat.h"

#include "eem/detection.h"
#include "eem/resource.h"

namespace EEM {

DBDArchive::DBDArchive() {
}

DBDArchive::~DBDArchive() {
	close();
}

bool DBDArchive::open(const Common::Path &dbdName, const Common::Path &dbxName) {
	close();

	if (!_dbd.open(dbdName)) {
		warning("DBDArchive: cannot open %s", dbdName.toString().c_str());
		return false;
	}

	Common::File dbx;
	if (!dbx.open(dbxName)) {
		warning("DBDArchive: cannot open %s", dbxName.toString().c_str());
		_dbd.close();
		return false;
	}

	// _InitGraphicsSystem @ 172b:0145 reads 10 bytes per entry until EOF.
	const int32 dbxSize = dbx.size();
	_index.reserve(dbxSize / 10);
	while (dbx.pos() + 10 <= dbxSize) {
		DBEntry entry;
		entry.offset     = dbx.readUint32LE();
		entry.compressed = dbx.readUint16LE();
		entry.size       = dbx.readUint32LE();
		_index.push_back(entry);
	}
	dbx.close();

	debugC(1, kDebugGfx, "DBDArchive: opened %s (%u entries)",
		   dbdName.toString().c_str(), _index.size());
	return true;
}

void DBDArchive::close() {
	if (_dbd.isOpen())
		_dbd.close();
	_index.clear();
}

bool DBDArchive::loadEntry(uint num, Picture &out) {
	if (num >= _index.size()) {
		warning("DBDArchive::loadEntry: %u out of range (max %u)", num, _index.size());
		return false;
	}

	const DBEntry &entry = _index[num];
	if (!_dbd.seek(entry.offset)) {
		warning("DBDArchive::loadEntry: seek to 0x%08x failed", entry.offset);
		return false;
	}

	// Mirrors _GetFromDB @ 172b:105d:
	//   _fread(i)            // 2-byte skip word (purpose unclear; always 0x0001)
	//   _fread(pic, 1, 12)   // 12-byte header
	(void)_dbd.readUint16LE();
	out.flags             = _dbd.readUint16LE();
	const uint16 height   = _dbd.readUint16LE();
	const uint16 width    = _dbd.readUint16LE();
	out.rowoff            = _dbd.readUint16LE();
	out.miscflags         = _dbd.readUint16LE();
	out.compsize          = _dbd.readUint16LE();

	if (width == 0 || height == 0) {
		warning("DBDArchive::loadEntry: %u has zero dimensions (%ux%u)",
				num, width, height);
		return false;
	}

	out.surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	if (entry.compressed == 0) {
		// Raw pixel data — read width*height bytes verbatim.
		const uint32 pixelCount = (uint32)width * (uint32)height;
		if (_dbd.read(out.surface.getPixels(), pixelCount) != pixelCount) {
			warning("DBDArchive::loadEntry: short raw read on %u", num);
			return false;
		}
		return true;
	}

	// Compressed payload: feed the .DBD stream straight into the DCL
	// decoder, matching the pattern used by Neverhood's BLB archive.
	const uint32 unpacked = (uint32)width * (uint32)height;
	if (!Common::decompressDCL(&_dbd, (byte *)out.surface.getPixels(),
							   out.compsize, unpacked)) {
		warning("DBDArchive::loadEntry: DCL decompression failed on %u "
				"(%u packed -> %u pixels)", num, out.compsize, unpacked);
		return false;
	}
	return true;
}

} // End of namespace EEM
