/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/formats/xmg.h"
#include "engines/stark/debug.h"
#include "engines/stark/gfx/driver.h"

#include "graphics/conversion.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Stark {
namespace Formats {

XMGDecoder::XMGDecoder(Common::ReadStream *stream) :
		_width(0),
		_height(0),
		_currX(0),
		_currY(0),
		_stream(stream),
		_transColor(0) {
}

Graphics::Surface *XMGDecoder::decode(Common::ReadStream *stream) {
	XMGDecoder dec(stream);
	dec.readHeader();
	return dec.decodeImage();
}

void XMGDecoder::readSize(Common::ReadStream *stream, uint &width, uint &height) {
	XMGDecoder dec(stream);
	dec.readHeader();

	width = dec._width;
	height = dec._height;
}

void XMGDecoder::readHeader() {
	// Read the file version
	uint32 version = _stream->readUint32LE();
	if (version != 3) {
		error("Stark::XMG: File version unknown: %d", version);
	}

	// Read the transparency color (RGBA)
	_transColor = _stream->readUint32LE();

	// Read the image size
	_width = _stream->readUint32LE();
	_height = _stream->readUint32LE();
	debugC(10, kDebugXMG, "Stark::XMG: Version=%d, TransparencyColor=0x%08x, size=%dx%d", version, _transColor, _width, _height);

	// Read the scan length
	uint32 scanLen = _stream->readUint32LE();
	if (scanLen != 3 * _width) {
		error("Stark::XMG: The scan length (%d) doesn't match the width bytes (%d)", scanLen, 3 * _width);
	}

	// Unknown
	uint32 unknown2 = _stream->readUint32LE();
	debugC(kDebugUnknown, "Stark::XMG: unknown2 = %08x = %d", unknown2, unknown2);
	uint32 unknown3 = _stream->readUint32LE();
	debugC(kDebugUnknown, "Stark::XMG: unknown3 = %08x = %d", unknown3, unknown3);
}

Graphics::Surface *XMGDecoder::decodeImage() {
	// Create the destination surface
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_width, _height, Gfx::Driver::getRGBAPixelFormat());

	_currX = 0, _currY = 0;
	while (!_stream->eos()) {
		if (_currX >= _width) {
			assert(_currX == _width);
			_currX = 0;
			_currY += 2;
			if (_currY >= _height)
				break;
		}

		// Read the number and mode of the tiles
		byte op = _stream->readByte();
		uint16 count;
		if ((op & 0xC0) != 0xC0) {
			count = op & 0x3F;
		} else {
			count = ((op & 0xF) << 8) + _stream->readByte();
			op <<= 2;
		}
		op &= 0xC0;

		// Process the current serie
		for (int i = 0; i < count; i++) {
			Block block = decodeBlock(op);
			drawBlock(block, surface);
		}
	}

	return surface;
}

XMGDecoder::Block XMGDecoder::decodeBlock(byte op) {
	Block block;

	switch (op) {
		case 0x00:
			// YCrCb
			block = processYCrCb();
			break;
		case 0x40:
			// Trans
			block = processTrans();
			break;
		case 0x80:
			// RGB
			block = processRGB();
			break;
		default:
			error("Unsupported color mode '%d'", op);
	}

	return block;
}

void XMGDecoder::drawBlock(const Block &block, Graphics::Surface *surface) {
	uint32 *pixels = (uint32 *)surface->getBasePtr(_currX, _currY);

	bool drawTwoColumns = _currX + 1 < _width;
	bool drawTwoLines = _currY + 1 < _height;

	pixels[0] = TO_LE_32(block.a1);

	if (drawTwoColumns) {
		pixels[1] = TO_LE_32(block.a2);
	}

	if (drawTwoLines) {
		pixels[_width + 0] = TO_LE_32(block.b1);
	}

	if (drawTwoColumns && drawTwoLines) {
		pixels[_width + 1] = TO_LE_32(block.b2);
	}

	_currX += drawTwoColumns ? 2 : 1;
}

XMGDecoder::Block XMGDecoder::processYCrCb() {
	byte y0, y1, y2, y3;
	byte cr, cb;

	y0 = _stream->readByte();
	y1 = _stream->readByte();
	y2 = _stream->readByte();
	y3 = _stream->readByte();
	cr = _stream->readByte();
	cb = _stream->readByte();

	byte r, g, b;
	Block block;

	Graphics::YUV2RGB(y0, cb, cr, r, g, b);
	block.a1 = (255u << 24) + (b << 16) + (g << 8) + r;

	Graphics::YUV2RGB(y1, cb, cr, r, g, b);
	block.a2 = (255u << 24) + (b << 16) + (g << 8) + r;

	Graphics::YUV2RGB(y2, cb, cr, r, g, b);
	block.b1 = (255u << 24) + (b << 16) + (g << 8) + r;

	Graphics::YUV2RGB(y3, cb, cr, r, g, b);
	block.b2 = (255u << 24) + (b << 16) + (g << 8) + r;

	return block;
}

XMGDecoder::Block XMGDecoder::processTrans() {
	Block block;

	block.a1 = 0;
	block.a2 = 0;
	block.b1 = 0;
	block.b2 = 0;

	return block;
}

XMGDecoder::Block XMGDecoder::processRGB() {
	Block block;
	uint32 color;

	color = _stream->readUint16LE();
	color += _stream->readByte() << 16;
	if (color != _transColor)
		color += 255 << 24;
	else
		color = 0;
	block.a1 = color;

	color = _stream->readUint16LE();
	color += _stream->readByte() << 16;
	if (color != _transColor)
		color += 255 << 24;
	else
		color = 0;
	block.a2 = color;

	color = _stream->readUint16LE();
	color += _stream->readByte() << 16;
	if (color != _transColor)
		color += 255 << 24;
	else
		color = 0;
	block.b1 = color;

	color = _stream->readUint16LE();
	color += _stream->readByte() << 16;
	if (color != _transColor)
		color += 255 << 24;
	else
		color = 0;
	block.b2 = color;

	return block;
}

} // End of namespace Formats
} // End of namespace Stark
