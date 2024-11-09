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

#include "common/debug.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/platform.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"
#include "dgds/dgds.h"
#include "dgds/font.h"
#include "dgds/includes.h"
#include "dgds/image.h"
#include "dgds/resource.h"
#include "dgds/parser.h"

namespace Dgds {

Image::Image(ResourceManager *resourceMan, Decompressor *decompressor) : _resourceMan(resourceMan), _decompressor(decompressor), _matrixX(0), _matrixY(0) {
}

Image::~Image() {
}

void Image::drawScreen(const Common::String &filename, Graphics::ManagedSurface &surface) {
	const char *dot;
	DGDS_EX ex;

	if (filename.empty()) {
		warning("Image::drawScreen Tried to draw empty image");
		return;
	}

	// surface should be allocated already.
	assert(!surface.empty());

	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	if (!fileStream)
		error("Couldn't get image resource %s", filename.c_str());

	if ((dot = strrchr(filename.c_str(), '.'))) {
		ex = MKTAG24(toupper(dot[1]), toupper(dot[2]), toupper(dot[3]));
	} else {
		ex = 0;
	}

	if (ex != EX_SCR) {
		warning("Unknown screen tag: %d", ex);
		delete fileStream;
		return;
	}

	_filename = filename;

	surface.fillRect(Common::Rect(surface.w, surface.h), 0);

	uint16 xsize = surface.w;
	uint16 ysize = surface.h;

	DgdsChunkReader chunk(fileStream);
	while (chunk.readNextHeader(ex, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(_decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();
		if (chunk.isSection(ID_BIN)) {
			loadBitmap4(&surface, 0, stream, false, xsize, ysize);
		} else if (chunk.isSection(ID_DIM)) {
			xsize = stream->readUint16LE();
			ysize = stream->readUint16LE();
			if (xsize > surface.w || ysize > surface.h) {
				error("Trying to load SCR size %d x %d which is larger than screen %d x %d",
					xsize, ysize, surface.w, surface.h);
			}
			debug("screen file %s dims %d x %d into surface %d x %d",
				filename.c_str(), xsize, ysize, surface.w, surface.h);
		} else if (chunk.isSection(ID_VGA)) {
			loadBitmap4(&surface, 0, stream, true, xsize, ysize);
		} else if (chunk.isSection(ID_MA8)) {
			loadBitmap8(&surface, 0, stream, xsize, ysize);
		} else if (chunk.isSection(ID_VQT)) {
			loadVQT(&surface, 0, stream);
		}
	}

	delete fileStream;
}

int Image::frameCount(const Common::String &filename) {
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	if (!fileStream)
		error("frameCount: Couldn't get bitmap resource '%s'", filename.c_str());

	int tileCount = -1;
	DgdsChunkReader chunk(fileStream);
	while (chunk.readNextHeader(EX_BMP, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(_decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();
		if (chunk.isSection(ID_INF)) {
			tileCount = stream->readUint16LE();
		}
	}
	return tileCount;
}

void Image::loadBitmap(const Common::String &filename) {
	DGDS_EX ex;
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	if (!fileStream)
		error("loadBitmap: Couldn't get bitmap resource '%s'", filename.c_str());

	_frames.clear();

	const char *dot;
	if ((dot = strrchr(filename.c_str(), '.'))) {
		ex = MKTAG24(toupper(dot[1]), toupper(dot[2]), toupper(dot[3]));
	} else {
		ex = 0;
	}

	if (ex != EX_BMP) {
		warning("Unknown bitmap tag: %d", ex);
		delete fileStream;
		return;
	}

	_filename = filename;

	int64 vqtpos = -1;
	int64 scnpos = -1;

	DgdsChunkReader chunk(fileStream);
	while (chunk.readNextHeader(ex, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(_decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();
		if (chunk.isSection(ID_INF)) {
			Common::Array<Common::Point> tileSizes;
			uint16 tileCount = stream->readUint16LE();
			if (tileCount > 1024)
				error("Image::loadBitmap: Unexpectedly large number of tiles in image (%d)", tileCount);
			_frames.resize(tileCount);
			tileSizes.resize(tileCount);
			for (uint16 k = 0; k < tileCount; k++) {
				tileSizes[k].x = stream->readUint16LE();
			}
			for (uint16 k = 0; k < tileCount; k++) {
				tileSizes[k].y = stream->readUint16LE();
			}

			for (uint16 k = 0; k < tileCount; k++) {
				_frames[k].reset(new Graphics::ManagedSurface(tileSizes[k].x, tileSizes[k].y, Graphics::PixelFormat::createFormatCLUT8()));
			}
		} else if (chunk.isSection(ID_MTX)) {
			// Scroll offset
			_matrixX = stream->readUint16LE();
			_matrixY = stream->readUint16LE();
			uint32 mcount = (uint32)_matrixX * _matrixY;
			_tileMatrix.resize(mcount);
			debug("		%u x %u: mtx vals", _matrixX, _matrixY);

			for (uint32 k = 0; k < mcount; k++) {
				uint16 tile;
				tile = stream->readUint16LE();
				_tileMatrix[k] = tile;
			}
		} else if (chunk.isSection(ID_BIN)) {
			for (auto & frame : _frames) {
				loadBitmap4(frame.get(), 0, stream, false, frame->w, frame->h);
			}
		} else if (chunk.isSection(ID_VGA)) {
			for (auto & frame : _frames) {
				loadBitmap4(frame.get(), 0, stream, true, frame->w, frame->h);
			}
		} else if (chunk.isSection(ID_VQT)) {
			// Postpone parsing this until we have the offsets, which come after.
			vqtpos = fileStream->pos();
		} else if (chunk.isSection(ID_SCN)) {
			// Postpone parsing this until we have the offsets, which come after.
			scnpos = fileStream->pos();
		} else if (chunk.isSection(ID_OFF)) {
			if (vqtpos == -1 && scnpos == -1)
				error("Expect VQT or SCN chunk before OFF chunk in BMP resource %s", filename.c_str());

			// 2 possibilities: A set of offsets (find the one which we need and use it)
			// or a single value of 0xffff.  If it's only one tile, the offset is always
			// zero anyway.  For subsequent images, round up to the next byte to start
			// reading.
			if (chunk.getSize() == 2) {
				assert(scnpos == -1);  // don't support this mode for SCN?
				if (_frames.size() > 1) {
					uint16 val = stream->readUint16LE();
					if (val != 0xffff)
						warning("Expected 0xffff in 2-byte offset list, got %04x", val);
				}

				uint32 nextOffset = 0;
				for (auto & frame : _frames) {
					fileStream->seek(vqtpos);
					nextOffset = loadVQT(frame.get(), nextOffset, fileStream);
					nextOffset = ((nextOffset + 7) / 8) * 8;
				}
			} else {
				Common::Array<uint32> frameOffsets;
				for (uint i = 0; i < _frames.size(); i++)
					frameOffsets.push_back(stream->readUint32LE());

				for (uint i = 0; i < _frames.size(); i++) {
					if (vqtpos != -1) {
						fileStream->seek(vqtpos + frameOffsets[i]);
						loadVQT(_frames[i].get(), 0, fileStream);
					} else {
						fileStream->seek(scnpos + frameOffsets[i]);
						loadSCN(_frames[i].get(), fileStream);
					}
				}
			}
			// NOTE: This was proably the last chunk, but we don't check - should have
			// the image data now. If we need to read another chunk we should fix up the
			// offset of fileStream because we just broke it.
			break;
		}
	}

	delete fileStream;
}

void Image::drawBitmap(uint frameno, int x, int y, const Common::Rect &drawWin, Graphics::ManagedSurface &destSurf, ImageFlipMode flipMode, int dstWidth, int dstHeight) const {
	if (frameno >= _frames.size()) {
		warning("drawBitmap: Trying to draw frame %d from a %d frame image %s!", frameno, _frames.size(), _filename.c_str());
		return;
	}

	const Common::SharedPtr<Graphics::ManagedSurface> srcFrame = _frames[frameno];

	int srcWidth = srcFrame->w;
	int srcHeight = srcFrame->h;
	if (dstWidth == 0)
		dstWidth = srcWidth;
	if (dstHeight == 0)
		dstHeight = srcHeight;

	const Common::Rect destRect(Common::Point(x, y), dstWidth, dstHeight);
	Common::Rect clippedDestRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	clippedDestRect.clip(destRect);
	clippedDestRect.clip(drawWin);
	if (clippedDestRect.isEmpty())
		return;

	const byte *src = (const byte *)srcFrame->getBasePtr(0, 0);

	// Note: this is not particularly optimized, written to be easier to understand

	byte *dst = (byte *)destSurf.getBasePtr(x, y);

	for (int i = 0; i < dstHeight; ++i) {
		const byte *srcRow = src;
		if (flipMode & kImageFlipV)
			srcRow += srcFrame->pitch * ((dstHeight - i - 1) * srcHeight / dstHeight);
		else
			srcRow += srcFrame->pitch * (i * srcHeight / dstHeight);

		for (int j = 0; j < dstWidth; ++j) {
			int srcX;
			if (flipMode & kImageFlipH)
				srcX = (dstWidth - j - 1) * srcWidth / dstWidth;
			else
				srcX = j * srcWidth / dstWidth;

			if (srcRow[srcX] && clippedDestRect.contains(j + x, i + y))
				dst[j] = srcRow[srcX];
		}
		dst += destSurf.pitch;
	}
}

void Image::drawScrollBitmap(int16 x, int16 y, int16 width, int16 height, int16 scrollX, int16 scrollY, const Common::Rect &drawWin, Graphics::ManagedSurface &dstSurf) const {
	if (_frames.empty())
		error("Trying to draw scroll for empty image.");
	if (_tileMatrix.empty())
		error("Trying to draw scroll with non-tiled image.");
	int tileW = _frames[0]->w;
	int tileH = _frames[0]->h;
	byte *dst = (byte *)dstSurf.getBasePtr(0, 0);

	for (int yTile = 0; yTile < height / tileH; yTile++) {
		int tileDstY = y + yTile * tileH;
		int tileRowIndex = (yTile + scrollY) % _matrixY;
		if (tileRowIndex < 0)
			tileRowIndex += _matrixY;
		for (int xTile = 0; xTile < width / tileW; xTile++) {
			int tileDstX = x + xTile * tileW;
			Common::Rect tileDest(Common::Point(tileDstX, tileDstY), tileW, tileH);
			tileDest.clip(drawWin);
			if (tileDest.isEmpty())
				continue;

			int tileColIndex = (xTile + scrollX) % _matrixX;
			if (tileColIndex < 0)
				tileColIndex += _matrixX;

			uint16 tileNo = _tileMatrix[tileRowIndex + tileColIndex * _matrixY];
			Common::SharedPtr<Graphics::ManagedSurface> tile = _frames[tileNo];
			const byte *src = (const byte *)tile->getBasePtr(0, 0);

			for (int dstY = tileDstY; dstY < tileDstY + tileH; dstY++) {
				for (int dstX = tileDstX; dstX < tileDstX + tileW; dstX++) {
					if (!tileDest.contains(dstX, dstY))
						continue;
					dst[dstY * dstSurf.pitch + dstX] = src[(dstY - tileDstY) * tile->pitch + (dstX - tileDstX)];
				}
			}
		}
	}
}


void Image::loadBitmap4(Graphics::ManagedSurface *surf, uint32 toffset, Common::SeekableReadStream *stream, bool highByte, uint16 tw, uint16 th) {
	assert(th != 0);
	byte *data = (byte *)surf->getPixels();

	stream->skip(toffset >> 1);

	if (highByte) {
		for (uint i = 0; i < tw * th; i += 2) {
			byte val = stream->readByte();
			data[i + 0] |= val & 0xF0;
			data[i + 1] |= (val & 0x0F) << 4;
		}
	} else {
		for (uint i = 0; i < tw * th; i += 2) {
			byte val = stream->readByte();
			data[i + 0] |= (val & 0xF0) >> 4;
			data[i + 1] |= val & 0x0F;
		}
	}
}

void Image::loadBitmap8(Graphics::ManagedSurface *surf, uint32 toffset, Common::SeekableReadStream *stream, uint16 tw, uint16 th) {
	assert(th != 0);
	byte *data = (byte *)surf->getPixels();

	stream->skip(toffset);
	stream->read(data, (uint32)tw * th);
}

struct VQTDecodeState {
	uint32 offset;
	const byte *srcPtr;
	byte *dstPtr;
	uint16 rowStarts[SCREEN_HEIGHT_FIXED];
};

static inline byte _getVqtBits(struct VQTDecodeState *state, uint16 nbits) {
	assert(nbits <= 8);
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

	// this will always fit in uint16 but we multiply it later so need 32 bits.
	const uint32 losize = (w & 0xff) * (h & 0xff);
	byte bitcount1 = 8;
	if (losize < 256) {
		bitcount1 = 0;
		byte b = (byte)(losize - 1);
		do {
			bitcount1++;
			b >>= 1;
		} while (b != 0);
	}

	byte firstval = _getVqtBits(state, bitcount1);

	uint16 bitcount2 = 0;
	byte bval = (byte)firstval;
	while (firstval != 0) {
		bitcount2++;
		firstval >>= 1;
	}

	bval++;

	if (losize * 8 <= losize * bitcount2 + (uint32)bval * 8) {
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

	byte tmpbuf[255];
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
		_doVqtDecode(state, x + (w / 2), y, (w + 1) / 2, h / 2);
	else
		_doVqtDecode2(state, x + (w / 2), y, (w + 1) / 2, h / 2);

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


uint32 Image::loadVQT(Graphics::ManagedSurface *surf, uint32 toffset, Common::SeekableReadStream *stream) {
	uint32 tw = surf->w;
	uint32 th = surf->h;
	assert(th != 0);
	assert(DgdsEngine::getInstance()->getGameId() != GID_CASTAWAY);
	if (th > SCREEN_HEIGHT)
		error("Max VQT height supported is 200px");
	VQTDecodeState state;
	state.dstPtr = (byte *)surf->getPixels();
	state.offset = toffset;
	// FIXME: This sometimes reads more than it needs to..
	uint64 nbytes = stream->size() - stream->pos();
	byte *buf = (byte *)malloc(nbytes + 8);
	if (!buf)
		error("Image::loadVQT: Alloc failed");
	memset(buf, 0, nbytes + 8);
	stream->read(buf, nbytes);
	state.srcPtr = buf;
	for (uint i = 0; i < th; i++)
		state.rowStarts[i] = tw * i;

	_doVqtDecode(&state, 0, 0, tw, th);
	free(buf);
	return state.offset;
}

//
// SCN file parsing - eg, "WILLCRED.BMP" from Willy Beamish
// Ref: https://moddingwiki.shikadi.net/wiki/The_Incredible_Machine_Image_Format
//
bool Image::loadSCN(Graphics::ManagedSurface *surf, Common::SeekableReadStream *stream) {
	int32 tw = surf->w;
	int32 th = surf->h;
	assert(th != 0);
	byte *dst = (byte *)surf->getPixels();

	int32 y = 0;
	int32 x = 0;

	const byte addVal = stream->readByte();

	byte lastcmd = 0xff;
	while (y < th && !stream->eos()) {
		byte val = stream->readByte();
		byte cmd = val & 0xc0; // top 2 bits are the command
		val &= 0x3f;		   // bottom 6 bits are the value
		switch (cmd) {
		case 0x00: // CMD 00 - move cursor down and back
			if (lastcmd == 0x00 && val) {
				x -= (val << 6);
			} else {
				y++;
				x -= val;
			}
			if (x < 0)
				error("Image::loadSCN: Moved x too far back on line %d", y);
			break;
		case 0x40: // CMD 01 - skip
			if (!val)
				y = th; // end of image.
			else
				x += val;
			break;
		case 0x80: { // CMD 10 - repeat val
			byte color = stream->readByte();
			for (uint16 i = 0; i < val; i++)
				dst[y * tw + x + i] = color + addVal;
			x += val;
			break;
		}
		case 0xc0: { // CMD 11 - direct read of `val` * 4-bit values
			for (uint16 i = 0; i < val; i += 2) {
				byte color = stream->readByte();
				dst[y * tw + x + i] = (color >> 4) + addVal;
				if (x + i + 1 < tw && i < val - 1)
					dst[y * tw + x + i + 1] = (color & 0xf) + addVal;
			}
			x += val;
			break;
		}
		}
		lastcmd = cmd;
		if (x > tw)
			error("Image::loadSCN: Invalid data, x went off the end of the row");
	}
	return !stream->err();
}

int16 Image::width(uint frameno) const {
	if (frameno >= _frames.size())
		error("Invalid frameno %d", frameno);
	return _frames[frameno]->w;
}

int16 Image::height(uint frameno) const {
	if (frameno >= _frames.size())
		error("Invalid frameno %d", frameno);
	return _frames[frameno]->h;
}

Common::SharedPtr<Graphics::ManagedSurface> Image::getSurface(uint frameno) const {
	if (frameno >= _frames.size())
		error("Invalid frameno %d", frameno);
	return _frames[frameno];
}

} // End of namespace Dgds
