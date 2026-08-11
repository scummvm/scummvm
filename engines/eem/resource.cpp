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
#include "common/array.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/memstream.h"
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

static uint32 readUint24BE(Common::SeekableReadStream &stream) {
	const uint32 hi = stream.readByte();
	const uint32 mid = stream.readByte();
	const uint32 lo = stream.readByte();
	return (hi << 16) | (mid << 8) | lo;
}

static Common::SeekableReadStream *openRawMacResource(const Common::Path &path,
													  uint32 typeId,
													  uint16 resourceId) {
	Common::File f;
	if (!f.open(path))
		return nullptr;

	const uint32 fileSize = (uint32)f.size();
	if (fileSize < 16)
		return nullptr;

	const uint32 dataOffset = f.readUint32BE();
	const uint32 mapOffset = f.readUint32BE();
	const uint32 dataLength = f.readUint32BE();
	const uint32 mapLength = f.readUint32BE();
	if (dataOffset > fileSize || mapOffset > fileSize ||
		dataLength > fileSize - dataOffset ||
		mapLength > fileSize - mapOffset || mapLength < 28)
		return nullptr;

	f.seek(mapOffset + 24);
	const uint16 typeListOffset = f.readUint16BE();
	const uint32 typeListBase = mapOffset + typeListOffset;
	const uint32 mapEnd = mapOffset + mapLength;
	if (typeListBase + 2 > mapEnd)
		return nullptr;

	f.seek(typeListBase);
	const uint16 typeCount = f.readUint16BE() + 1;
	for (uint typeIndex = 0; typeIndex < typeCount; typeIndex++) {
		const uint32 typeEntry = typeListBase + 2 + typeIndex * 8;
		if (typeEntry + 8 > mapEnd)
			return nullptr;

		f.seek(typeEntry);
		const uint32 curType = f.readUint32BE();
		const uint16 resourceCount = f.readUint16BE() + 1;
		const uint16 refListOffset = f.readUint16BE();
		if (curType != typeId)
			continue;

		const uint32 refListBase = typeListBase + refListOffset;
		if (refListBase > mapEnd)
			return nullptr;

		for (uint resIndex = 0; resIndex < resourceCount; resIndex++) {
			const uint32 refEntry = refListBase + resIndex * 12;
			if (refEntry + 12 > mapEnd)
				return nullptr;

			f.seek(refEntry);
			const uint16 curId = f.readUint16BE();
			f.skip(2); // resource name offset
			f.skip(1); // attributes
			const uint32 resourceDataOffset = readUint24BE(f);
			f.skip(4); // handle

			if (curId != resourceId)
				continue;

			const uint32 resourceData = dataOffset + resourceDataOffset;
			if (resourceData + 4 > dataOffset + dataLength)
				return nullptr;

			f.seek(resourceData);
			const uint32 resourceLength = f.readUint32BE();
			if (resourceLength > dataOffset + dataLength - resourceData - 4)
				return nullptr;

			byte *data = (byte *)malloc(resourceLength);
			if (!data)
				return nullptr;
			if (f.read(data, resourceLength) != resourceLength) {
				free(data);
				return nullptr;
			}

			return new Common::MemoryReadStream(data, resourceLength,
											   DisposeAfterUse::YES);
		}
	}

	return nullptr;
}

Common::SeekableReadStream *openMacResource(const Common::Path &path,
											uint32 typeId,
											uint16 resourceId) {
	Common::MacResManager res;
	Common::SeekableReadStream *stream = nullptr;
	if (res.open(path) && res.hasResFork())
		stream = res.getResource(typeId, resourceId);
	if (stream)
		return stream;

	return openRawMacResource(path, typeId, resourceId);
}

bool DBDArchive::open(const Common::Path &dbdName, const Common::Path &dbxName, bool bigEndian) {
	close();
	_bigEndian = bigEndian;

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
		if (_bigEndian) {
			entry.offset     = dbx.readUint32BE();
			entry.compressed = dbx.readUint16BE();
			entry.size       = dbx.readUint32BE();
		} else {
			entry.offset     = dbx.readUint32LE();
			entry.compressed = dbx.readUint16LE();
			entry.size       = dbx.readUint32LE();
		}
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
	_bigEndian = false;
}

bool decompressMacLZSS(Common::SeekableReadStream &stream, byte *dest,
					   uint32 packedSize, uint32 unpackedSize) {
	Common::Array<byte> packed;
	packed.resize(packedSize);
	if (stream.read(packed.data(), packedSize) != packedSize) {
		warning("decompressMacLZSS: short packed read (%u bytes)", packedSize);
		return false;
	}

	byte dict[4096];
	memset(dict, 0x20, sizeof(dict));

	uint32 src = 0;
	uint32 dst = 0;
	uint16 dictPos = 0x0fee;
	byte flags = 0;
	byte mask = 0;

	while (dst < unpackedSize && src < packedSize) {
		if (mask == 0) {
			flags = packed[src++];
			mask = 1;
			if (src > packedSize)
				break;
		}

		if (flags & mask) {
			if (src >= packedSize)
				break;
			const byte b = packed[src++];
			dest[dst++] = b;
			dict[dictPos] = b;
			dictPos = (dictPos + 1) & 0x0fff;
		} else {
			if (src + 1 >= packedSize)
				break;
			const byte lo = packed[src++];
			const byte hi = packed[src++];
			uint16 offset = (uint16)lo | (uint16)((hi & 0xf0) << 4);
			uint16 count = (hi & 0x0f) + 3;
			while (count-- && dst < unpackedSize) {
				const byte b = dict[offset];
				offset = (offset + 1) & 0x0fff;
				dest[dst++] = b;
				dict[dictPos] = b;
				dictPos = (dictPos + 1) & 0x0fff;
			}
		}

		mask <<= 1;
	}

	if (dst != unpackedSize) {
		warning("decompressMacLZSS: decoded %u of %u bytes", dst, unpackedSize);
		return false;
	}
	return true;
}

/// Read one frame header + payload at the current stream position.
/// Shared between picture and animation loaders since the layout is identical.
bool readFrame(Common::SeekableReadStream &stream, bool compressed, bool bigEndian, Picture &out) {
	uint16 height;
	uint16 width;

	if (bigEndian) {
		out.flags     = stream.readUint16BE();
		height        = stream.readUint16BE();
		width         = stream.readUint16BE();
		out.rowoff    = stream.readUint16BE();
		out.miscflags = stream.readUint16BE();
		out.compsize  = stream.readUint32BE();
	} else {
		out.flags     = stream.readUint16LE();
		height        = stream.readUint16LE();
		width         = stream.readUint16LE();
		out.rowoff    = stream.readUint16LE();
		out.miscflags = stream.readUint16LE();
		out.compsize  = stream.readUint16LE();
	}

	if (width == 0 || height == 0) {
		warning("readFrame: zero dimensions (%ux%u)", width, height);
		return false;
	}

	out.surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	const uint32 unpacked = (uint32)width * (uint32)height;

	if (!compressed) {
		if (!bigEndian && stream.read(out.surface.getPixels(), unpacked) != unpacked) {
			warning("readFrame: short raw read (%u bytes)", unpacked);
			return false;
		}
		if (bigEndian) {
			const uint32 rowBytes = (width + 1) & ~1U;
			Common::Array<byte> row;
			row.resize(rowBytes);
			for (uint16 y = 0; y < height; y++) {
				if (stream.read(row.data(), rowBytes) != rowBytes) {
					warning("readFrame: short Mac raw row read (%u bytes)", rowBytes);
					return false;
				}
				memcpy(out.surface.getBasePtr(0, y), row.data(), width);
			}
		}
		return true;
	}

	if (bigEndian) {
		if (out.compsize < 4) {
			warning("readFrame: invalid Mac compressed size %u", out.compsize);
			return false;
		}
		const uint32 decodedSize = stream.readUint32BE();
		const uint32 packedSize = out.compsize - 4;

		Common::Array<byte> decoded;
		decoded.resize(decodedSize);
		if (!decompressMacLZSS(stream, decoded.data(), packedSize, decodedSize)) {
			warning("readFrame: Mac LZSS decompression failed (%u packed -> %u bytes)",
					packedSize, decodedSize);
			return false;
		}

		const uint32 rowBytes = (width + 1) & ~1U;
		if (decodedSize < rowBytes * height) {
			warning("readFrame: Mac decoded frame too short (%u < %u)",
					decodedSize, rowBytes * height);
			return false;
		}
		for (uint16 y = 0; y < height; y++)
			memcpy(out.surface.getBasePtr(0, y), decoded.data() + y * rowBytes, width);
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
	_bigEndian ? _dbd.readUint16BE() : _dbd.readUint16LE();
	return readFrame(_dbd, entry.compressed != 0, _bigEndian, out);
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

	const uint16 frameCount = _bigEndian ? _dbd.readUint16BE() : _dbd.readUint16LE();
	if (frameCount == 0 || frameCount > 256) {
		warning("DBDArchive::loadAnimation: %u has implausible frame count %u",
				num, frameCount);
		return false;
	}

	out.resize(frameCount);
	for (uint16 i = 0; i < frameCount; i++) {
		if (!readFrame(_dbd, entry.compressed != 0, _bigEndian, out[i])) {
			warning("DBDArchive::loadAnimation: frame %u/%u failed in entry %u",
					i, frameCount, num);
			return false;
		}
	}
	return true;
}

} // End of namespace EEM
