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

/// Read one 12-byte frame header + payload at the current stream position.
/// Shared between picture and animation loaders since the layout is identical.
bool readFrame(Common::SeekableReadStream &stream, bool compressed, Picture &out) {
	out.flags             = stream.readUint16LE();
	const uint16 height   = stream.readUint16LE();
	const uint16 width    = stream.readUint16LE();
	out.rowoff            = stream.readUint16LE();
	out.miscflags         = stream.readUint16LE();
	out.compsize          = stream.readUint16LE();

	if (width == 0 || height == 0) {
		warning("readFrame: zero dimensions (%ux%u)", width, height);
		return false;
	}

	out.surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	const uint32 unpacked = (uint32)width * (uint32)height;

	if (!compressed) {
		if (stream.read(out.surface.getPixels(), unpacked) != unpacked) {
			warning("readFrame: short raw read (%u bytes)", unpacked);
			return false;
		}
		return true;
	}

	if (!Common::decompressDCL(&stream, (byte *)out.surface.getPixels(),
							   out.compsize, unpacked)) {
		warning("readFrame: DCL decompression failed (%u packed -> %u pixels)",
				out.compsize, unpacked);
		return false;
	}
	return true;
}

bool DBDArchive::loadEntry(uint num, Picture &out) {
	if (num >= _index.size()) {
		// Out-of-range picture IDs are non-fatal
		debugC(2, kDebugGfx,
			   "DBDArchive::loadEntry: %u out of range (max %u)",
			   num, (uint)_index.size());
		return false;
	}

	const DBEntry &entry = _index[num];
	if (!_dbd.seek(entry.offset)) {
		warning("DBDArchive::loadEntry: seek to 0x%08x failed", entry.offset);
		return false;
	}

	// Leading u16 = frame count (always 1 for pictures).
	_dbd.skip(2);
	return readFrame(_dbd, entry.compressed != 0, out);
}

bool DBDArchive::loadAnimation(uint num, Animation &out) {
	if (num >= _index.size()) {
		warning("DBDArchive::loadAnimation: %u out of range (max %u)", num, _index.size());
		return false;
	}

	const DBEntry &entry = _index[num];
	if (!_dbd.seek(entry.offset)) {
		warning("DBDArchive::loadAnimation: seek to 0x%08x failed", entry.offset);
		return false;
	}

	const uint16 frameCount = _dbd.readUint16LE();
	if (frameCount == 0 || frameCount > 256) {
		warning("DBDArchive::loadAnimation: %u has implausible frame count %u",
				num, frameCount);
		return false;
	}

	out.resize(frameCount);
	for (uint16 i = 0; i < frameCount; i++) {
		if (!readFrame(_dbd, entry.compressed != 0, out[i])) {
			warning("DBDArchive::loadAnimation: frame %u/%u failed in entry %u",
					i, frameCount, num);
			return false;
		}
	}
	return true;
}

} // End of namespace EEM
