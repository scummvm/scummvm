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
 * Copyright 2020 Google
 *
 */
#include "common/debug.h"
#include "common/stream.h"

#include "hadesch/pod_image.h"
#include "hadesch/tag_file.h"
#include "hadesch/baptr.h"
#include "hadesch/gfx_context.h"

namespace Hadesch {
PodImage::PodImage() {
	_w = 0;
	_h = 0;
	_pos = Common::Point(0, 0);
	_ncolors = 0;
}

PodImage::~PodImage() {
}

bool PodImage::loadImage(const PodFile &col, int index) {
	char bufname[256];
	snprintf (bufname, sizeof(bufname) - 1, "%d", index);
	Common::SharedPtr<Common::SeekableReadStream> dataStream(col.getFileStream(bufname));
	if (!dataStream) {
		return false;
	}

	Common::SharedPtr<Common::SeekableReadStream> palStream(col.getFileStream("0"));
	TagFile palTags;
	if (!palStream || !palTags.openStoreCel(palStream)) {
		debug("Couldn't open palette");
		return false;
	}

	Common::SharedPtr<Common::SeekableReadStream> palTagStream(palTags.getFileStream(MKTAG('P', 'A', 'L', ' ')));

	if (!palTagStream) {
		debug("Couldn't open PAL palette in image %s", col.getDebugName().c_str());
		return false;
	}

	uint palSize = palTagStream->size();
	if (palSize > 256 * 4) {
		debug("Palette unexpectedly large");
		palSize = 256 * 4;
	}

	_palette = sharedPtrByteAlloc(256 * 4);
	memset(_palette.get(), 0, 256 * 4);
	_paletteCursor = sharedPtrByteAlloc(256 * 3);
	memset(_paletteCursor.get(), 0, 256 * 3);

	palTagStream->read(_palette.get(), palSize);
	_ncolors = palSize / 4;

	for (int i = 0; i < _ncolors; i++) {
		int color = _palette.get()[4 * i] & 0xff;
		
		_paletteCursor.get()[3 * color    ] = _palette.get()[4 * i + 1];
		_paletteCursor.get()[3 * color + 1] = _palette.get()[4 * i + 2];
		_paletteCursor.get()[3 * color + 2] = _palette.get()[4 * i + 3];
	}

	TagFile dataTags;
	if (!dataTags.openStoreCel(dataStream)) {
		debug("Couldn't open data for image %d", index);
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> infoTagStream(dataTags.getFileStream(MKTAG('I', 'N', 'F', 'O')));

	if (!infoTagStream) {
		debug("Couldn't open INFO");
		return false;
	}

	if (infoTagStream->size() < 0x1c) {
		debug("INFO section too small");
		return false;
	}

	infoTagStream->skip(0xc);
	int x = -(int32)infoTagStream->readUint32BE();
	int y = -(int32)infoTagStream->readUint32BE();
	_pos = Common::Point(x,y);
	_w = infoTagStream->readUint32BE();
	_h = infoTagStream->readUint32BE();

	// Empty image
	if (_w < 0 || _h < 0) {
		_w = 0;
		_h = 0;
		_pixels = sharedPtrByteAlloc(1);
		memset(_pixels.get(), 0, 1);
		return true;
	}

	_pixels = sharedPtrByteAlloc(_w * _h);
	memset(_pixels.get(), 0, _w * _h);
	// TODO: check this
	_hotspot = Common::Point(_w / 2, _h / 2);

	Common::ScopedPtr<Common::SeekableReadStream> dataTagStream(dataTags.getFileStream(MKTAG('D', 'A', 'T', 'A')));

	if (!dataTagStream) {
		debug("Couldn't open DATA in image %s, index %d", col.getDebugName().c_str(), index);
		return false;
	}

	int linerem = _w;
	int line = 0;

	for (int pos = 0; pos < _w * _h && !dataTagStream->eos(); ) {
		byte rlelen = dataTagStream->readByte();
		byte rleval = dataTagStream->readByte();
		if (dataTagStream->eos()) {
			break;
		}
		if (rlelen != 0) {
			int len = rlelen;
			if (len > linerem) {
				len = linerem;
			}
			memset(_pixels.get() + pos, rleval, len);
			linerem -= len;
			pos += len;
			continue;
		}

		if (rleval != 0) {
			int len = rleval;
			if (len > linerem) {
				len = linerem;
			}
			dataTagStream->read(_pixels.get() + pos, len);
			linerem -= len;
			pos += len;
			continue;
		}

		// End of line
		line++;
		linerem = _w;
		pos = line * _w;
	}
	
	return true;
}

// Naive implementation as it's used very rarely
// Nearest neighbour, unoptimized
void PodImage::makeScale(int scale) const {
	struct ScaledVersion sv;
	sv._w = _w * scale / 100;
	sv._h = _h * scale / 100;
	sv._pixels = sharedPtrByteAlloc(sv._h * sv._w);
	for (int x = 0; x < sv._w; x++) {
		int ox = x * _w / sv._w;
		if (ox >= _w)
			ox = _w - 1;
		if (ox < 0)
			ox = 0;
		for (int y = 0; y < sv._h; y++) {
			int oy = y * _h / sv._h;
			if (oy >= _h)
				oy = _h - 1;
			if (oy < 0)
				oy = 0;
			sv._pixels.get()[x + y * sv._w] = _pixels.get()[ox + oy * _w];
		}
	}
	_scales[scale] = sv;
}
	
void PodImage::render(Common::SharedPtr<GfxContext> context,
		      Common::Point offset,
		      int colourScale,
		      int scale) const {
	byte *originalPalette = _palette.get();
	byte *scaledPalette = nullptr;
	if (colourScale != 0x100) {
		scaledPalette = new byte[_ncolors * 4];
		for (unsigned i = 0; (int)i < _ncolors; i++) {
			scaledPalette[4 * i] = originalPalette[4 * i];
			scaledPalette[4 * i + 1] = (originalPalette[4 * i + 1] * colourScale) >> 8;
			scaledPalette[4 * i + 2] = (originalPalette[4 * i + 2] * colourScale) >> 8;
			scaledPalette[4 * i + 3] = (originalPalette[4 * i + 3] * colourScale) >> 8;
		}
	}

	if (scale == 100)
		context->blitPodImage(_pixels.get(), _w, _w, _h,
				      scaledPalette ? scaledPalette : originalPalette, _ncolors, _pos + offset);
	else {
		if (!_scales.contains(scale))
			makeScale(scale);
		context->blitPodImage(_scales[scale]._pixels.get(), _scales[scale]._w, _scales[scale]._w, _scales[scale]._h,
				      scaledPalette ? scaledPalette : originalPalette, _ncolors, _pos * (scale / 100.0) + offset);		
	}
	if (scaledPalette)
		delete [] scaledPalette;
}

Common::Point PodImage::getOffset() const {
	return _pos;
}

uint16 PodImage::getWidth() const {
	return _w;
}

uint16 PodImage::getHeight() const {
	return _h;
}

uint16 PodImage::getHotspotX() const {
	return _hotspot.x;
}

uint16 PodImage::getHotspotY() const {
	return _hotspot.y;
}

byte PodImage::getKeyColor() const {
	return 0;
}

const byte *PodImage::getSurface() const {
	return _pixels.get();
}

const byte *PodImage::getPalette() const {
	return _paletteCursor.get();
}

byte PodImage::getPaletteStartIndex() const {
	return 0;
}

uint16 PodImage::getPaletteCount() const {
	return 256;
}

void PodImage::setHotspot(Common::Point hotspot) {
	_hotspot = hotspot;
}

}
