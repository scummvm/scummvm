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

#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/palette.h"
#include "graphics/wincursor.h"

namespace Graphics {

/** A Windows cursor. */
class WinCursor : public Cursor {
public:
	WinCursor();
	~WinCursor();

	/** Return the cursor's width. */
	uint16 getWidth() const override;
	/** Return the cursor's height. */
	uint16 getHeight() const override;
	/** Return the cursor's hotspot's x coordinate. */
	uint16 getHotspotX() const override;
	/** Return the cursor's hotspot's y coordinate. */
	uint16 getHotspotY() const override;
	/** Return the cursor's transparent key. */
	byte getKeyColor() const override;

	const byte *getSurface() const override { return _surface; }
	const byte *getMask() const override { return _mask; }

	const byte *getPalette() const override { return _palette.data; }
	byte getPaletteStartIndex() const override { return 0; }
	uint16 getPaletteCount() const override { return _palette.size; }

	/** Read the cursor's data out of a stream. */
	bool readFromStream(Common::SeekableReadStream &stream);

private:
	byte *_surface;
	byte *_mask;
	Palette _palette;

	uint16 _width;    ///< The cursor's width.
	uint16 _height;   ///< The cursor's height.
	uint16 _hotspotX; ///< The cursor's hotspot's x coordinate.
	uint16 _hotspotY; ///< The cursor's hotspot's y coordinate.
	byte   _keyColor; ///< The cursor's transparent key

	/** Clear the cursor. */
	void clear();
};

WinCursor::WinCursor() : _surface(nullptr), _mask(nullptr), _palette(256),
	_width(0), _height(0), _hotspotX(0), _hotspotY(0), _keyColor(0) {
}

WinCursor::~WinCursor() {
	clear();
}

uint16 WinCursor::getWidth() const {
	return _width;
}

uint16 WinCursor::getHeight() const {
	return _height;
}

uint16 WinCursor::getHotspotX() const {
	return _hotspotX;
}

uint16 WinCursor::getHotspotY() const {
	return _hotspotY;
}

byte WinCursor::getKeyColor() const {
	return _keyColor;
}

bool WinCursor::readFromStream(Common::SeekableReadStream &stream) {
	clear();

	const bool supportOpacity = g_system->hasFeature(OSystem::kFeatureCursorMask);
	const bool supportInvert = g_system->hasFeature(OSystem::kFeatureCursorMaskInvert);

	_hotspotX = stream.readUint16LE();
	_hotspotY = stream.readUint16LE();

	// Check header size
	if (stream.readUint32LE() != 40)
		return false;

	// Check dimensions
	_width = stream.readUint32LE();
	_height = stream.readUint32LE() / 2;

	if (_width & 3) {
		// Cursors should always be a power of 2
		// Of course, it wouldn't be hard to handle but if we have no examples...
		warning("Non-divisible-by-4 width cursor found");
		return false;
	}

	// Color planes
	if (stream.readUint16LE() != 1)
		return false;

	// Only 1bpp, 4bpp and 8bpp supported
	uint16 bitsPerPixel = stream.readUint16LE();
	if (bitsPerPixel != 1 && bitsPerPixel != 4 && bitsPerPixel != 8)
		return false;

	// Compression
	if (stream.readUint32LE() != 0)
		return false;

	// Image size + X resolution + Y resolution
	stream.skip(12);

	uint32 numColors = stream.readUint32LE();

	// If the color count is 0, then it uses up the maximum amount
	if (numColors == 0)
		numColors = 1 << bitsPerPixel;

	// Reading the palette
	stream.seek(40 + 4);
	for (uint32 i = 0 ; i < numColors; i++) {
		_palette.data[i * 3 + 2] = stream.readByte();
		_palette.data[i * 3 + 1] = stream.readByte();
		_palette.data[i * 3] = stream.readByte();
		stream.readByte();
	}

	// Reading the bitmap data
	uint32 dataSize = stream.size() - stream.pos();
	byte *initialSource = new byte[dataSize];
	stream.read(initialSource, dataSize);

	// Parse the XOR map
	const byte *src = initialSource;
	_surface = new byte[_width * _height];
	if (supportOpacity)
		_mask = new byte[_width * _height];
	byte *dest = _surface + _width * (_height - 1);
	uint32 imagePitch = _width * bitsPerPixel / 8;

	for (uint32 i = 0; i < _height; i++) {
		byte *rowDest = dest;

		if (bitsPerPixel == 1) {
			// 1bpp
			for (uint16 j = 0; j < (_width / 8); j++) {
				byte p = src[j];

				for (int k = 0; k < 8; k++, rowDest++, p <<= 1) {
					if ((p & 0x80) == 0x80)
						*rowDest = 1;
					else
						*rowDest = 0;
				}
			}
		} else if (bitsPerPixel == 4) {
			// 4bpp
			for (uint16 j = 0; j < (_width / 2); j++) {
				byte p = src[j];
				*rowDest++ = p >> 4;
				*rowDest++ = p & 0x0f;
			}
		} else {
			// 8bpp
			memcpy(rowDest, src, _width);
		}

		dest -= _width;
		src += imagePitch;
	}

	// Calculate our key color
	if (numColors < 256) {
		// If we're not using the maximum colors in a byte, we can fit it in
		_keyColor = numColors;
	} else {
		// HACK: Try to find a color that's not being used so it can become
		// our keycolor. It's quite impossible to fit 257 entries into 256...
		for (uint32 i = 0; i < 256; i++) {
			for (int j = 0; j < _width * _height; j++) {
				// TODO: Also check to see if the space is transparent

				if (_surface[j] == i)
					break;

				if (j == _width * _height - 1) {
					_keyColor = i;
					i = 256;
					break;
				}
			}
		}
	}

	// Now go through and apply the AND map to get the transparency
	uint32 andWidth = (_width + 7) / 8;
	src += andWidth * (_height - 1);

	for (uint32 y = 0; y < _height; y++) {
		for (uint32 x = 0; x < _width; x++) {
			byte &surfaceByte = _surface[y * _width + x];
			if (src[x / 8] & (1 << (7 - x % 8))) {
				const byte *paletteEntry = &_palette.data[surfaceByte * 3];

				// Per WDDM spec, white with 1 in the AND mask is inverted, any other color with 1 is transparent.
				// Riven depends on this behavior for proper cursor transparency, since it uses cursors where the
				// transparent pixels have a non-zero non-black color.
				const bool isTransparent = (paletteEntry[0] != 255 || paletteEntry[1] != 255 || paletteEntry[2] != 255);

				if (_mask) {
					byte &maskByte = _mask[y * _width + x];

					if (isTransparent) {
						maskByte = 0;
					} else {
						// Inverted, if the backend supports invert then emit an inverted pixel, otherwise opaque
						maskByte = supportInvert ? kCursorMaskInvert : kCursorMaskOpaque;
					}
				} else {
					// Don't support mask or invert, leave this as opaque if it's XOR so it's visible
					if (isTransparent)
						surfaceByte = _keyColor;
				}
			} else {
				// Opaque pixel
				if (_mask)
					_mask[y * _width + x] = kCursorMaskOpaque;
			}
		}

		src -= andWidth;
	}

	delete[] initialSource;
	return true;
}

void WinCursor::clear() {
	delete[] _surface; _surface = nullptr;
	delete[] _mask; _mask = nullptr;
}

WinCursorGroup::WinCursorGroup() {
}

WinCursorGroup::~WinCursorGroup() {
	for (uint32 i = 0; i < cursors.size(); i++)
		delete cursors[i].cursor;
}

WinCursorGroup *WinCursorGroup::createCursorGroup(Common::WinResources *exe, const Common::WinResourceID &id) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(exe->getResource(Common::kWinGroupCursor, id));

	if (!stream || stream->size() <= 6)
		return 0;

	stream->skip(4);
	uint32 cursorCount = stream->readUint16LE();
	if ((uint32)stream->size() < (6 + cursorCount * 14))
		return 0;

	WinCursorGroup *group = new WinCursorGroup();
	group->cursors.reserve(cursorCount);

	for (uint32 i = 0; i < cursorCount; i++) {
		stream->readUint16LE(); // width
		stream->readUint16LE(); // height
		stream->readUint16LE(); // x hotspot
		stream->readUint16LE(); // y hotspot
		stream->readUint32LE(); // data size
		uint32 cursorId = stream->readUint16LE();

		Common::ScopedPtr<Common::SeekableReadStream> cursorStream(exe->getResource(Common::kWinCursor, cursorId));
		if (!cursorStream) {
			delete group;
			return 0;
		}

		WinCursor *cursor = new WinCursor();
		if (!cursor->readFromStream(*cursorStream)) {
			delete cursor;
			delete group;
			return 0;
		}

		CursorItem item;
		item.id = cursorId;
		item.cursor = cursor;
		group->cursors.push_back(item);
	}

	return group;
}

/**
 * The default Windows cursor
 */
class DefaultWinCursor : public Cursor {
public:
	DefaultWinCursor() {}
	~DefaultWinCursor() {}

	uint16 getWidth() const override { return 12; }
	uint16 getHeight() const override { return 20; }
	uint16 getHotspotX() const override { return 0; }
	uint16 getHotspotY() const override { return 0; }
	byte getKeyColor() const override { return 0; }

	const byte *getSurface() const override {
		static const byte defaultCursor[] = {
			1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0,
			1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0,
			1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0,
			1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0,
			1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0,
			1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0,
			1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
			1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
			1, 2, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0,
			1, 2, 2, 1, 1, 2, 2, 1, 0, 0, 0, 0,
			1, 2, 1, 0, 1, 1, 2, 2, 1, 0, 0, 0,
			1, 1, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0,
			1, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
			0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0
		};

		return defaultCursor;
	}

	const byte *getPalette() const override {
		static const byte bwPalette[] = {
			0x00, 0x00, 0x00,	// Black
			0xFF, 0xFF, 0xFF	// White
		};

		return bwPalette;
	}
	byte getPaletteStartIndex() const override { return 1; }
	uint16 getPaletteCount() const override { return 2; }
};

Cursor *makeDefaultWinCursor() {
	return new DefaultWinCursor();
}

/**
 * The Windows busy cursor
 */
class BusyWinCursor : public Cursor {
public:
	BusyWinCursor() {}
	~BusyWinCursor() {}

	uint16 getWidth() const override { return 15; }
	uint16 getHeight() const override { return 27; }
	uint16 getHotspotX() const override { return 7; }
	uint16 getHotspotY() const override { return 13; }
	byte getKeyColor() const override { return 0; }

	const byte *getSurface() const override {
		static const byte busyCursor[] = {
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
			0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0,
			0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0,
			0, 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 1, 0,
			0, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 0,
			0, 1, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2, 1, 1, 0,
			0, 0, 1, 1, 2, 2, 1, 2, 1, 2, 2, 1, 1, 0, 0,
			0, 0, 0, 1, 1, 2, 2, 1, 2, 2, 1, 1, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0,
			0, 0, 1, 1, 2, 2, 2, 1, 2, 2, 2, 1, 1, 0, 0,
			0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0,
			0, 1, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 1, 0,
			0, 1, 1, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 1, 0,
			0, 1, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2, 1, 1, 0,
			0, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 0,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
			1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
		};

		return busyCursor;
	}

	const byte *getPalette() const override {
		static const byte bwPalette[] = {
			0x00, 0x00, 0x00,	// Black
			0xFF, 0xFF, 0xFF	// White
		};

		return bwPalette;
	}
	byte getPaletteStartIndex() const override { return 1; }
	uint16 getPaletteCount() const override { return 2; }
};

Cursor *makeBusyWinCursor() {
	return new BusyWinCursor();
}

} // End of namespace Graphics
