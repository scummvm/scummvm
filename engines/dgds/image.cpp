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

#include "common/debug.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/platform.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "dgds/dgds.h"
#include "dgds/font.h"
#include "dgds/includes.h"
#include "dgds/image.h"
#include "dgds/resource.h"
#include "dgds/parser.h"

namespace Dgds {

Image::Image(ResourceManager *resourceMan, Decompressor *decompressor) : _resourceMan(resourceMan), _decompressor(decompressor), _tileWidth(0), _tileHeight(0), _tileOffset(0) {
	memset(_palette, 0, 256 * 3);
	memset(_blacks, 0, 256 * 3);
}

Image::~Image() {
	_bmpData.free();
}

void Image::drawScreen(Common::String filename, Graphics::Surface &surface) {
	const char *dot;
	DGDS_EX ex;
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	DgdsParser ctx(*fileStream, filename.c_str());

	DgdsChunk chunk;

	if ((dot = strrchr(filename.c_str(), '.'))) {
		ex = MKTAG24(dot[1], dot[2], dot[3]);
	} else {
		ex = 0;
	}

	if (ex != EX_SCR) {
		warning("Unknown screen tag: %d", ex);
		delete fileStream;
		return;
	}

	surface.fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);

	// Currently does not handle the VQT: and OFF: chunks
	// for the compressed pics in the DOS port.
	while (chunk.readHeader(ctx)) {
		Common::SeekableReadStream *stream = chunk.isPacked(ex) ? chunk.decodeStream(ctx, _decompressor) : chunk.readStream(ctx);
		if (chunk.isSection(ID_BIN)) {
			loadBitmap4(surface, SCREEN_WIDTH, SCREEN_HEIGHT, 0, stream, false);
		} else if (chunk.isSection(ID_VGA)) {
			loadBitmap4(surface, SCREEN_WIDTH, SCREEN_HEIGHT, 0, stream, true);
		} else if (chunk.isSection(ID_MA8)) {
			loadBitmap8(surface, SCREEN_WIDTH, SCREEN_HEIGHT, 0, stream);
		} else if (chunk.isSection(ID_VQT)) {
			loadVQT(surface, SCREEN_WIDTH, SCREEN_HEIGHT, 0, stream);
		}
	}

	delete fileStream;
}

void Image::loadBitmap(Common::String filename, int number) {
	const char *dot;
	DGDS_EX ex;
	uint16 *mtx;
	uint16 mw, mh;
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	DgdsParser ctx(*fileStream, filename.c_str());
	DgdsChunk chunk;

	_tileOffset = 0;
	_bmpData.free();

	if ((dot = strrchr(filename.c_str(), '.'))) {
		ex = MKTAG24(dot[1], dot[2], dot[3]);
	} else {
		ex = 0;
	}

	if (ex != EX_BMP) {
		warning("Unknown bitmap tag: %d", ex);
		delete fileStream;
		return;
	}

	// Currently does not handle the VQT: and OFF: chunks
	// for the compressed pics in the DOS port.
	while (chunk.readHeader(ctx)) {
		Common::SeekableReadStream *stream = chunk.isPacked(ex) ? chunk.decodeStream(ctx, _decompressor) : chunk.readStream(ctx);
		if (chunk.isSection(ID_INF)) {
			uint16 tileCount = stream->readUint16LE();
			uint16 *tileWidths = new uint16[tileCount];
			uint16 *tileHeights = new uint16[tileCount];

			for (uint16 k = 0; k < tileCount; k++) {
				tileWidths[k] = stream->readUint16LE();
				if (k == number)
					_tileWidth = tileWidths[k];
			}

			for (uint16 k = 0; k < tileCount; k++) {
				tileHeights[k] = stream->readUint16LE();
				if (k == number)
					_tileHeight = tileHeights[k];

				if (k < number)
					_tileOffset += tileWidths[k] * tileHeights[k];
			}

			delete[] tileWidths;
			delete[] tileHeights;
		} else if (chunk.isSection(ID_MTX)) {
			// Scroll offset
			mw = stream->readUint16LE();
			mh = stream->readUint16LE();
			uint32 mcount = uint32(mw) * mh;
			debug("        %ux%u: %u bytes", mw, mh, mcount * 2);

			mtx = new uint16[mcount];
			for (uint32 k = 0; k < mcount; k++) {
				uint16 tile;
				tile = stream->readUint16LE();
				mtx[k] = tile;
			}
			// TODO: Use these
			delete mtx;
		} else if (chunk.isSection(ID_BIN)) {
			loadBitmap4(_bmpData, _tileWidth, _tileHeight, _tileOffset, stream, false);
		} else if (chunk.isSection(ID_VGA)) {
			loadBitmap4(_bmpData, _tileWidth, _tileHeight, _tileOffset, stream, true);
		} else if (chunk.isSection(ID_VQT)) {
			loadVQT(_bmpData, _tileWidth, _tileHeight, _tileOffset, stream);
		}
	}

	delete fileStream;
}

void Image::drawBitmap(int x, int y, Common::Rect &drawWin, Graphics::Surface &surface) {
	const Common::Rect destRect(x, y, x + _tileWidth, y + _tileHeight);
	Common::Rect clippedDestRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	clippedDestRect.clip(destRect);
	clippedDestRect.clip(drawWin);

	const Common::Point croppedBy(clippedDestRect.left - destRect.left, clippedDestRect.top - destRect.top);
	const int rows = clippedDestRect.height();
	const int columns = clippedDestRect.width();

	byte *src = (byte *)_bmpData.getPixels() + croppedBy.y * _tileWidth + croppedBy.x;
	byte *ptr = (byte *)surface.getBasePtr(clippedDestRect.left, clippedDestRect.top);
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			if (src[j])
				ptr[j] = src[j];
		}
		ptr += surface.pitch;
		src += _tileWidth;
	}
}

void Image::loadBitmap4(Graphics::Surface &surf, uint16 tw, uint16 th, uint32 toffset, Common::SeekableReadStream *stream, bool highByte) {
	uint16 outPitch = tw;
	if (surf.h == 0)
		surf.create(outPitch, th, Graphics::PixelFormat::createFormatCLUT8());
	byte *data = (byte *)surf.getPixels();
	byte buf;

	stream->skip(toffset >> 1);

	if (highByte) {
		for (int i = 0; i < tw * th; i += 2) {
			buf = stream->readByte();
			data[i + 0] |= buf & 0xF0;
			data[i + 1] |= (buf & 0x0F) << 4;
		}
	} else {
		for (int i = 0; i < tw * th; i += 2) {
			buf = stream->readByte();
			data[i + 0] |= (buf & 0xF0) >> 4;
			data[i + 1] |= buf & 0x0F;
		}
	}
}

void Image::loadBitmap8(Graphics::Surface &surf, uint16 tw, uint16 th, uint32 toffset, Common::SeekableReadStream *stream) {
	uint16 outPitch = tw;
	surf.create(outPitch, th, Graphics::PixelFormat::createFormatCLUT8());
	byte *data = (byte *)surf.getPixels();

	stream->skip(toffset);
	stream->read(data, uint32(outPitch) * th);
}

void Image::loadVQT(Graphics::Surface &surf, uint16 tw, uint16 th, uint32 toffset, Common::SeekableReadStream *stream) {
	uint16 outPitch = tw;
	surf.create(outPitch, th, Graphics::PixelFormat::createFormatCLUT8());
	byte *data = (byte *)surf.getPixels();

	// HACK
	stream->skip(toffset);
	stream->read(data, uint32(outPitch) * th);
}

void Image::loadPalette(Common::String filename) {
	const char *dot;
	DGDS_EX ex;
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	DgdsParser ctx(*fileStream, filename.c_str());

	DgdsChunk chunk;

	while (chunk.readHeader(ctx)) {
		Common::SeekableReadStream *stream = chunk.readStream(ctx);
		if (chunk.isSection(ID_VGA)) {
			stream->read(_palette, 256 * 3);

			for (uint k = 0; k < 256 * 3; k += 3) {
				_palette[k + 0] <<= 2;
				_palette[k + 1] <<= 2;
				_palette[k + 2] <<= 2;
			}
		}
	}

	delete fileStream;
}

void Image::setPalette() {
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void Image::clearPalette() {
	g_system->getPaletteManager()->setPalette(_blacks, 0, 256);
}

	// grayscale palette.
/*
	for (uint i=0; i<256; i++) {
	    palette[i*3+0] = 255-i;
	    palette[i*3+1] = 255-i;
	    palette[i*3+2] = 255-i;
	}
	*/
/*
	// Amiga grayscale palette.
	for (uint i=0; i<32; i++) {
	    palette[i*3+0] = 255-i*8;
	    palette[i*3+1] = 255-i*8;
	    palette[i*3+2] = 255-i*8;
	}
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
	*/

} // End of namespace Dgds
