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

Image::Image(ResourceManager *resourceMan, Decompressor *decompressor) : _resourceMan(resourceMan), _decompressor(decompressor) {
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
	if (!fileStream)
		error("Couldn't get image resource %s", filename.c_str());

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

	DgdsChunk chunk;
	while (chunk.readHeader(fileStream, filename)) {
		Common::SeekableReadStream *stream = chunk.getStream(ex, fileStream, _decompressor);
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
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	if (!fileStream)
		error("Couldn't get bitmap resource %s", filename.c_str());
	DgdsChunk chunk;

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

	int64 vqtpos = -1;
	int32 vqtsize = -1;
	uint16 tileWidths[64];
	uint16 tileHeights[64];
	int32 tileOffset = 0;

	while (chunk.readHeader(fileStream, filename)) {
		Common::SeekableReadStream *stream = chunk.getStream(ex, fileStream, _decompressor);
		if (chunk.isSection(ID_INF)) {
			uint16 tileCount = stream->readUint16LE();
			for (uint16 k = 0; k < tileCount; k++) {
				tileWidths[k] = stream->readUint16LE();
			}

			for (uint16 k = 0; k < tileCount; k++) {
				tileHeights[k] = stream->readUint16LE();
				if (k < number)
					tileOffset += tileWidths[k] * tileHeights[k];
			}
		} else if (chunk.isSection(ID_MTX)) {
			// Scroll offset
			uint16 mw, mh;
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
			loadBitmap4(_bmpData, tileWidths[number], tileHeights[number], tileOffset, stream, false);
		} else if (chunk.isSection(ID_VGA)) {
			loadBitmap4(_bmpData, tileWidths[number], tileHeights[number], tileOffset, stream, true);
		} else if (chunk.isSection(ID_VQT)) {
			// Postpone parsing this until we have the offsets, which come after.
			vqtpos = fileStream->pos();
			vqtsize = chunk._size;
			stream->skip(vqtsize);
		} else if (chunk.isSection(ID_OFF)) {
			if (vqtpos == -1)
				error("Expect VQT chunk before OFF chunk in BMP resource %s", filename.c_str());

			// 2 possibilities: A set of offsets (find the one which we need and use it)
			// or a single value of 0xffff.  If it's only one tile the offset is always
			// zero anyway.  For subsequent images, round up to the next byte to start
			// reading.
			if (chunk._size == 2) {
				if (number != 0) {
					uint16 val = stream->readUint16LE();
					if (val != 0xffff)
						warning("Expected 0xffff in 2-byte offset list, got %04x", val);
				}

				uint32 nextOffset = 0;
				for (int i = 0; i < number + 1; i++) {
					fileStream->seek(vqtpos);
					_bmpData.free();
					nextOffset = loadVQT(_bmpData, tileWidths[i], tileHeights[i], nextOffset, fileStream);
					nextOffset = ((nextOffset + 7) / 8) * 8;
				}
			} else {
				if (number)
					stream->skip(4 * number);
				uint32 vqtOffset = stream->readUint32LE();
				// TODO: seek stream to end for tidiness?
				fileStream->seek(vqtpos + vqtOffset);
				loadVQT(_bmpData, tileWidths[number], tileHeights[number], 0, fileStream);
			}
		}
		delete stream;
	}

	delete fileStream;
}

void Image::drawBitmap(int x, int y, Common::Rect &drawWin, Graphics::Surface &surface) {
	const Common::Rect destRect(x, y, x + _bmpData.w, y + _bmpData.h);
	Common::Rect clippedDestRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	clippedDestRect.clip(destRect);
	clippedDestRect.clip(drawWin);

	const Common::Point croppedBy(clippedDestRect.left - destRect.left, clippedDestRect.top - destRect.top);
	const int rows = clippedDestRect.height();
	const int columns = clippedDestRect.width();

	byte *src = (byte *)_bmpData.getPixels() + croppedBy.y * _bmpData.pitch + croppedBy.x;
	byte *ptr = (byte *)surface.getBasePtr(clippedDestRect.left, clippedDestRect.top);
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			if (src[j])
				ptr[j] = src[j];
		}
		ptr += surface.pitch;
		src += _bmpData.pitch;
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

struct VQTDecodeState {
	uint32 offset;
	const byte *srcPtr;
	byte *dstPtr;
	uint16 rowStarts[200];
};

static inline uint16 _getVqtBits(struct VQTDecodeState *state, int nbits) {
    const uint32 offset = state->offset;
   const uint32 index = offset >> 3;
   const uint32 shift = offset & 7;
   state->offset += nbits;
   return (*(const uint16 *)(state->srcPtr + index) >> (shift)) & (byte)(0xff00 >> (16 - nbits));
}

static void _doVqtDecode2(struct VQTDecodeState *state, const uint16 x, const uint16 y, const uint16 w, const uint16 h) {
    // Empty region -> nothing to do
    if (h == 0 || w == 0)
        return;

    // 1x1 region -> put the byte directly
    if (w == 1 && h == 1) {
        state->dstPtr[state->rowStarts[y] + x] = _getVqtBits(state, 8);
        return;
    }

    const uint losize = (w & 0xff) * (h & 0xff);
    uint bitcount1 = 8;
    if (losize < 256) {
        bitcount1 = 0;
        byte b = (byte)(losize - 1);
        do {
            bitcount1++;
            b >>= 1;
        } while (b != 0);
    }

    uint16 firstval = _getVqtBits(state, bitcount1);

    uint16 bitcount2 = 0;
    byte bval = (byte)firstval;
    while (firstval != 0) {
        bitcount2++;
        firstval >>= 1;
    }

    bval++;

    if (losize * 8 <= losize * bitcount2 + bval * 8) {
        for (int xx = x; xx < x + w; xx++) {
            for (int yy = y; yy < y + h; yy++) {
                state->dstPtr[state->rowStarts[yy] + xx] = _getVqtBits(state, 8);
            }
        }
        return;
    }

    if (bval == 1) {
        const uint16 val = _getVqtBits(state, 8);
        for (int yy = y; yy < y + h; yy++) {
            for (int xx = x; xx < x + w; xx++) {
                state->dstPtr[state->rowStarts[yy] + xx] = val;
            }
        }
        return;
    }

    byte tmpbuf [262];
    byte *ptmpbuf = tmpbuf;
    for (; bval != 0; bval--) {
        *ptmpbuf = _getVqtBits(state, 8);
        ptmpbuf++;
    }

    for (int xx = x; xx < x + w; xx++) {
        for (int yy = y; yy < y + h; yy++) {
            state->dstPtr[state->rowStarts[yy] + xx] = tmpbuf[_getVqtBits(state, bitcount2)];
        }
    }
}

static void _doVqtDecode(struct VQTDecodeState *state, uint16 x, uint16 y, uint16 w, uint16 h) {
    if (!w && !h)
        return;

    const uint16 mask = _getVqtBits(state, 4);

    // Top left quadrant
    if (mask & 8)
        _doVqtDecode(state, x, y, w / 2, h / 2);
    else
        _doVqtDecode2(state, x, y, w / 2, h / 2);

    // Top right quadrant
    if (mask & 4)
        _doVqtDecode(state, x + (w / 2), y, (w + 1) >> 1, h >> 1);
    else
        _doVqtDecode2(state, x + (w / 2), y, (w + 1) >> 1, h >> 1);

    // Bottom left quadrant
    if (mask & 2)
        _doVqtDecode(state, x, y + (h / 2), w / 2, (h + 1) / 2);
    else
        _doVqtDecode2(state, x, y + (h / 2), w / 2, (h + 1) / 2);

    // Bottom right quadrant
    if (mask & 1)
        _doVqtDecode(state, x + (w / 2), y + (h / 2), (w + 1) / 2, (h + 1) / 2);
    else
        _doVqtDecode2(state, x + (w / 2), y + (h / 2), (w + 1) / 2, (h + 1) / 2);
}


uint32 Image::loadVQT(Graphics::Surface &surf, uint16 tw, uint16 th, uint32 toffset, Common::SeekableReadStream *stream) {
	if (th > 200)
		error("Max VQT height supported is 200px");
	uint16 outPitch = tw;
	surf.create(outPitch, th, Graphics::PixelFormat::createFormatCLUT8());
	VQTDecodeState state;
	state.dstPtr = (byte *)surf.getPixels();
	state.offset = toffset;
	// FIXME: This sometimes reads more than it needs to..
	uint64 nbytes = stream->size() - stream->pos();
	byte *buf = (byte *)malloc(nbytes + 8);
	memset(buf, 0, nbytes + 8);
	stream->read(buf, nbytes);
	state.srcPtr = buf;
	for (uint i = 0; i < th; i++)
		state.rowStarts[i] = tw * i;
	
	_doVqtDecode(&state, 0, 0, tw, th);
	free(buf);
	return state.offset;
}

void Image::loadPalette(Common::String filename) {
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	if (!fileStream) {
		// Happens in the Amiga version of Dragon
		warning("Couldn't load palette resource %s", filename.c_str());
		return;
	}

	DgdsChunk chunk;

	while (chunk.readHeader(fileStream, filename)) {
		Common::SeekableReadStream *stream = chunk.readStream(fileStream);
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
