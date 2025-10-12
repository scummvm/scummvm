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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"

#include "image/png.h"

#include "director/director.h"
#include "director/cast.h"
#include "graphics/pixelformat.h"
#include "director/rte.h"

namespace Director {

RTE0::RTE0(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast) {
	if (debugChannelSet(9, kDebugLoading)) {
		debugC(9, kDebugLoading, "RTE0:");
		stream.hexdump(stream.size());
	}
	data.resize(stream.size(), 0);
	if (stream.size())
		stream.read(&data[0], stream.size());
}

RTE1::RTE1(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast) {
	if (debugChannelSet(9, kDebugLoading)) {
		debugC(9, kDebugLoading, "RTE1:");
		stream.hexdump(stream.size());
	}
	data.resize(stream.size(), 0);
	if (stream.size())
		stream.read(&data[0], stream.size());
}

RTE2::RTE2(Cast *cast, Common::SeekableReadStreamEndian &stream, uint16 id) : _cast(cast), _id(id) {
	if (debugChannelSet(9, kDebugLoading)) {
		debugC(9, kDebugLoading, "RTE2:");
		stream.hexdump(stream.size());
	}
	_width = 0;
	_height = 0;
	_bpp = 0;
	if (!stream.size())
		return;

	_width = stream.readUint16BE();
	_height = stream.readUint16BE();
	_bpp = stream.readUint16BE();
	int rleCount = stream.size() - stream.pos();
	_rle.resize(rleCount, 0x00);
	stream.read(_rle.data(), rleCount);
	debugC(5, kDebugLoading, "RTE2: _width: %d, _height: %d, _bpp: %d, _rle: %d bytes", _width, _height, _bpp, rleCount);
}

Graphics::ManagedSurface *RTE2::createSurface(uint32 foreColor, uint32 bgColor, const Graphics::PixelFormat &pf, bool renderBg) const {
	if (_rle.empty())
		return nullptr;
	Common::MemoryReadStream stream(_rle.data(), _rle.size());
	// Create a 32-bit alpha surface for the decoded image
	Graphics::Surface surface;
	surface.create((int16)_width, (int16)_height, pf);
	int checkMax = (1 << _bpp) - 1;
	uint8 r = 0, g = 0, b = 0, a = 0;
	pf.colorToRGB(foreColor, r, g, b);
	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width;) {
			uint32 pos = stream.pos();
			byte check = stream.readByte();
			if (check == 0x1f) {
				r = stream.readByte();
				g = stream.readByte();
				b = stream.readByte();
				debugC(8, kDebugLoading, "[%04x] (%d, %d): color %d %d %d", pos, x, y, r, g, b);
				continue;
			}
			a = ((uint32)check*0xff/((1 << _bpp) - 1));
			if (check == 0 || check == checkMax) {
				byte count = stream.readByte();
				debugC(8, kDebugLoading, "[%04x] (%d, %d): %02x, count %d", pos, x, y, check, count);
				if (count == 0x00 && check == 0x00) {
					// end of line, fill the remaining colour
					a = 0;
					while (x < _width) {
						*(uint32 *)surface.getBasePtr(x, y) = pf.ARGBToColor(a, r, g, b);
						x += 1;
					}
					break;
				}
				for (byte j = 0; j < count; j++) {
					*(uint32 *)surface.getBasePtr(x, y) = pf.ARGBToColor(a, r, g, b);
					x += 1;
					if (x >= _width)
						break;
				}
			} else {
				debugC(8, kDebugLoading, "[%04x] (%d, %d): %02x", pos, x, y, check);
				*(uint32 *)surface.getBasePtr(x, y) = pf.ARGBToColor(a, r, g, b);
				x += 1;
			}
		}
	}
	Graphics::ManagedSurface *result = new Graphics::ManagedSurface();
	result->create((int16)_width, (int16)_height, pf);

	if (renderBg) {
		// Fill it with the background colour
		result->fillRect(Common::Rect(_width, _height), bgColor);
	}

	// Blit the alpha text map
	result->blitFrom(surface, nullptr);

	surface.free();

	if (ConfMan.getBool("dump_scripts")) {

		Common::String prepend = _cast->getMacName();
		Common::String filename = Common::String::format("./dumps/%s-%s%s-%d.png",
				encodePathForDump(prepend).c_str(), "RTE2", (renderBg ? "-bg" : ""), _id);
		Common::DumpFile bitmapFile;

		warning("RTE2::createSurface(): Dumping RTE2 to '%s'", filename.c_str());

		bitmapFile.open(Common::Path(filename), true);
		Image::writePNG(bitmapFile, *result->surfacePtr(), nullptr);

		bitmapFile.close();
	}

	return result;
}

RTE2::~RTE2() {
}

} // End of namespace Director
