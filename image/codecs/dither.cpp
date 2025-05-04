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

#include "image/codecs/dither.h"

#include "common/list.h"

namespace Image {

namespace {

/**
 * Add a color to the QuickTime dither table check queue if it hasn't already been found.
 */
inline void addColorToQueue(uint16 color, uint16 index, byte *checkBuffer, Common::List<uint16> &checkQueue) {
	if ((READ_UINT16(checkBuffer + color * 2) & 0xFF) == 0) {
		// Previously unfound color
		WRITE_UINT16(checkBuffer + color * 2, index);
		checkQueue.push_back(color);
	}
}

inline byte adjustColorRange(byte currentColor, byte correctColor, byte palColor) {
	return CLIP<int>(currentColor - palColor + correctColor, 0, 255);
}

inline uint16 makeQuickTimeDitherColor(byte r, byte g, byte b) {
	// RGB554
	return ((r & 0xF8) << 6) | ((g & 0xF8) << 1) | (b >> 4);
}

} // End of anonymous namespace

DitherCodec::DitherCodec(Codec *codec, DisposeAfterUse::Flag disposeAfterUse)
  : _codec(codec), _disposeAfterUse(disposeAfterUse), _dirtyPalette(false),
    _forcedDitherPalette(0), _ditherTable(0), _ditherFrame(0), _srcPalette(nullptr) {
}

DitherCodec::~DitherCodec() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _codec;

	delete[] _ditherTable;

	if (_ditherFrame) {
		_ditherFrame->free();
		delete _ditherFrame;
	}
}

namespace {

// Default template to convert a dither color
inline uint16 readQT_RGB(uint32 srcColor, const Graphics::PixelFormat& format, const byte *palette) {
	byte r, g, b;
	format.colorToRGB(srcColor, r, g, b);
	return makeQuickTimeDitherColor(r, g, b);
}

// Specialized version for 8bpp
inline uint16 readQT_Palette(uint8 srcColor, const Graphics::PixelFormat& format, const byte *palette) {
	return makeQuickTimeDitherColor(palette[srcColor * 3], palette[srcColor * 3 + 1], palette[srcColor * 3 + 2]);
}

// Specialized version for RGB554
inline uint16 readQT_RGB554(uint16 srcColor, const Graphics::PixelFormat& format, const byte *palette) {
	return srcColor;
}

// Specialized version for RGB555 and ARGB1555
inline uint16 readQT_RGB555(uint16 srcColor, const Graphics::PixelFormat& format, const byte *palette) {
	return (srcColor >> 1) & 0x3FFF;
}

template<typename PixelInt, class Fn>
void ditherQuickTimeFrame(const Graphics::Surface &src, Graphics::Surface &dst, const byte *ditherTable, Fn fn, const byte *palette = 0) {
	static const uint16 colorTableOffsets[] = { 0x0000, 0xC000, 0x4000, 0x8000 };

	for (int y = 0; y < dst.h; y++) {
		const PixelInt *srcPtr = (const PixelInt *)src.getBasePtr(0, y);
		byte *dstPtr = (byte *)dst.getBasePtr(0, y);
		uint16 colorTableOffset = colorTableOffsets[y & 3];

		for (int x = 0; x < dst.w; x++) {
			uint16 color = fn(*srcPtr++, src.format, palette);
			*dstPtr++ = ditherTable[colorTableOffset + color];
			colorTableOffset += 0x4000;
		}
	}
}

} // End of anonymous namespace

const Graphics::Surface *DitherCodec::decodeFrame(Common::SeekableReadStream &stream) {
	const Graphics::Surface *frame = _codec->decodeFrame(stream);
	if (!frame || _forcedDitherPalette.empty())
		return frame;

	const byte *curPalette = _codec->containsPalette() ? _codec->getPalette() : _srcPalette;

	if (frame->format.isCLUT8() && curPalette) {
		// This should always be true, but this is for sanity
		if (!curPalette)
			return frame;

		// If the palettes match, bail out
		if (memcmp(_forcedDitherPalette.data(), curPalette, 256 * 3) == 0)
			return frame;
	}

	// Need to create a new one
	if (!_ditherFrame) {
		_ditherFrame = new Graphics::Surface();
		_ditherFrame->create(frame->w, frame->h, Graphics::PixelFormat::createFormatCLUT8());
	}

	if (frame->format.isCLUT8() && curPalette)
		ditherQuickTimeFrame<byte>(*frame, *_ditherFrame, _ditherTable, readQT_Palette, curPalette);
	else if (frame->format == Graphics::PixelFormat(2, 5, 5, 4, 0, 9, 4, 0, 0))
		ditherQuickTimeFrame<uint16>(*frame, *_ditherFrame, _ditherTable, readQT_RGB554);
	else if (frame->format == Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0) ||
	         frame->format == Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 1))
		ditherQuickTimeFrame<uint16>(*frame, *_ditherFrame, _ditherTable, readQT_RGB555);
	else if (frame->format.bytesPerPixel == 2)
		ditherQuickTimeFrame<uint16>(*frame, *_ditherFrame, _ditherTable, readQT_RGB);
	else if (frame->format.bytesPerPixel == 4)
		ditherQuickTimeFrame<uint32>(*frame, *_ditherFrame, _ditherTable, readQT_RGB);

	return _ditherFrame;
}

Graphics::PixelFormat DitherCodec::getPixelFormat() const {
	if (_forcedDitherPalette.empty())
		return _codec->getPixelFormat();
	return Graphics::PixelFormat::createFormatCLUT8();
}

bool DitherCodec::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	if (_forcedDitherPalette.empty())
		return _codec->setOutputPixelFormat(format);
	return format.isCLUT8();
}

bool DitherCodec::containsPalette() const {
	if (_forcedDitherPalette.empty())
		return _codec->containsPalette();
	return true;
}

const byte *DitherCodec::getPalette() {
	if (_forcedDitherPalette.empty())
		return _codec->getPalette();
	_dirtyPalette = false;
	return _forcedDitherPalette.data();
}

bool DitherCodec::hasDirtyPalette() const {
	if (_forcedDitherPalette.empty())
		return _codec->hasDirtyPalette();
	return _dirtyPalette;
}

bool DitherCodec::canDither(DitherType type) const {
	return _codec->canDither(type) || (type == kDitherTypeQT);
}

void DitherCodec::setDither(DitherType type, const byte *palette) {
	if (_codec->canDither(type)) {
		_codec->setDither(type, palette);
	} else {
		assert(type == kDitherTypeQT);
		assert(_forcedDitherPalette.empty());

		// Forced dither
		_forcedDitherPalette.resize(256, false);
		_forcedDitherPalette.set(palette, 0, 256);
		_dirtyPalette = true;

		_ditherTable = createQuickTimeDitherTable(_forcedDitherPalette.data(), 256);

		// Prefer RGB554 or RGB555 to avoid extra conversion when dithering
		if (!_codec->setOutputPixelFormat(Graphics::PixelFormat(2, 5, 5, 4, 0, 9, 4, 0, 0)))
			_codec->setOutputPixelFormat(Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));
	}
}

void DitherCodec::setCodecAccuracy(CodecAccuracy accuracy) {
	return _codec->setCodecAccuracy(accuracy);
}

byte *DitherCodec::createQuickTimeDitherTable(const byte *palette, uint colorCount) {
	byte *buf = new byte[0x10000]();

	Common::List<uint16> checkQueue;

	bool foundBlack = false;
	bool foundWhite = false;

	const byte *palPtr = palette;

	// See what colors we have, and add them to the queue to check
	for (uint i = 0; i < colorCount; i++) {
		byte r = *palPtr++;
		byte g = *palPtr++;
		byte b = *palPtr++;
		uint16 n = (i << 8) | 1;
		uint16 col = makeQuickTimeDitherColor(r, g, b);

		if (col == 0) {
			// Special case for close-to-black
			// The original did more here, but it effectively discarded the value
			// due to a poor if-check (whole 16-bit value instead of lower 8-bits).
			WRITE_UINT16(buf, n);
			foundBlack = true;
		} else if (col == 0x3FFF) {
			// Special case for close-to-white
			// The original did more here, but it effectively discarded the value
			// due to a poor if-check (whole 16-bit value instead of lower 8-bits).
			WRITE_UINT16(buf + 0x7FFE, n);
			foundWhite = true;
		} else {
			// Previously unfound color
			addColorToQueue(col, n, buf, checkQueue);
		}
	}

	// More special handling for white
	if (foundWhite)
		checkQueue.push_front(0x3FFF);

	// More special handling for black
	if (foundBlack)
		checkQueue.push_front(0);

	// Go through the list of colors we have and match up similar colors
	// to fill in the table as best as we can.
	while (!checkQueue.empty()) {
		uint16 col = checkQueue.front();
		checkQueue.pop_front();
		uint16 index = READ_UINT16(buf + col * 2);

		uint32 x = col << 4;
		if ((x & 0xFF) < 0xF0)
			addColorToQueue((x + 0x10) >> 4, index, buf, checkQueue);
		if ((x & 0xFF) >= 0x10)
			addColorToQueue((x - 0x10) >> 4, index, buf, checkQueue);

		uint32 y = col << 7;
		if ((y & 0xFF00) < 0xF800)
			addColorToQueue((y + 0x800) >> 7, index, buf, checkQueue);
		if ((y & 0xFF00) >= 0x800)
			addColorToQueue((y - 0x800) >> 7, index, buf, checkQueue);

		uint32 z = col << 2;
		if ((z & 0xFF00) < 0xF800)
			addColorToQueue((z + 0x800) >> 2, index, buf, checkQueue);
		if ((z & 0xFF00) >= 0x800)
			addColorToQueue((z - 0x800) >> 2, index, buf, checkQueue);
	}

	// Contract the table back to just palette entries
	for (int i = 0; i < 0x4000; i++)
		buf[i] = READ_UINT16(buf + i * 2) >> 8;

	// Now go through and distribute the error to three more pixels
	byte *bufPtr = buf;
	for (uint realR = 0; realR < 0x100; realR += 8) {
		for (uint realG = 0; realG < 0x100; realG += 8) {
			for (uint realB = 0; realB < 0x100; realB += 16) {
				byte palIndex = *bufPtr;
				byte r = realR;
				byte g = realG;
				byte b = realB;

				byte palR = palette[palIndex * 3] & 0xF8;
				byte palG = palette[palIndex * 3 + 1] & 0xF8;
				byte palB = palette[palIndex * 3 + 2] & 0xF0;

				r = adjustColorRange(r, realR, palR);
				g = adjustColorRange(g, realG, palG);
				b = adjustColorRange(b, realB, palB);
				palIndex = buf[makeQuickTimeDitherColor(r, g, b)];
				bufPtr[0x4000] = palIndex;

				palR = palette[palIndex * 3] & 0xF8;
				palG = palette[palIndex * 3 + 1] & 0xF8;
				palB = palette[palIndex * 3 + 2] & 0xF0;

				r = adjustColorRange(r, realR, palR);
				g = adjustColorRange(g, realG, palG);
				b = adjustColorRange(b, realB, palB);
				palIndex = buf[makeQuickTimeDitherColor(r, g, b)];
				bufPtr[0x8000] = palIndex;

				palR = palette[palIndex * 3] & 0xF8;
				palG = palette[palIndex * 3 + 1] & 0xF8;
				palB = palette[palIndex * 3 + 2] & 0xF0;

				r = adjustColorRange(r, realR, palR);
				g = adjustColorRange(g, realG, palG);
				b = adjustColorRange(b, realB, palB);
				palIndex = buf[makeQuickTimeDitherColor(r, g, b)];
				bufPtr[0xC000] = palIndex;

				bufPtr++;
			}
		}
	}

	return buf;
}

} // End of namespace Image

