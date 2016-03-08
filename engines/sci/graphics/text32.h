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

#ifndef SCI_GRAPHICS_TEXT32_H
#define SCI_GRAPHICS_TEXT32_H

#include "sci/graphics/celobj32.h"
#include "sci/graphics/frameout.h"

namespace Sci {

enum TextAlign {
	kTextAlignLeft   = 0,
	kTextAlignCenter = 1,
	kTextAlignRight  = 2
};

enum BitmapFlags {
	kBitmapRemap = 2
};

#define BITMAP_PROPERTY(size, property, offset)\
inline uint##size get##property() const {\
	return READ_SCI11ENDIAN_UINT##size(_bitmap + (offset));\
}\
inline void set##property(uint##size value) {\
	WRITE_SCI11ENDIAN_UINT##size(_bitmap + (offset), (value));\
}

/**
 * A convenience class for creating and modifying in-memory
 * bitmaps.
 */
class BitmapResource {
	byte *_bitmap;
	reg_t _object;

	/**
	 * Gets the size of the bitmap header for the current
	 * engine version.
	 */
	static inline uint16 getBitmapHeaderSize() {
		// TODO: These values are accurate for each engine, but there may be no reason
		// to not simply just always use size 40, since SCI2.1mid does not seem to
		// actually store any data above byte 40, and SCI2 did not allow bitmaps with
		// scaling resolutions other than the default (320x200). Perhaps SCI3 used
		// the extra bytes, or there is some reason why they tried to align the header
		// size with other headers like pic headers?
//		uint32 bitmapHeaderSize;
//		if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
//			bitmapHeaderSize = 46;
//		} else if (getSciVersion() == SCI_VERSION_2_1_EARLY) {
//			bitmapHeaderSize = 40;
//		} else {
//			bitmapHeaderSize = 36;
//		}
//		return bitmapHeaderSize;
		return 46;
	}

	/**
	 * Gets the byte size of a bitmap with the given width
	 * and height.
	 */
	static inline uint32 getBitmapSize(const uint16 width, const uint16 height) {
		return width * height + getBitmapHeaderSize();
	}

public:
	/**
	 * Create a bitmap resource for an existing bitmap.
	 * Ownership of the bitmap is retained by the caller.
	 */
	inline BitmapResource(reg_t bitmap) :
		_bitmap(g_sci->getEngineState()->_segMan->getHunkPointer(bitmap)),
		_object(bitmap) {
			if (_bitmap == nullptr || getUncompressedDataOffset() != getBitmapHeaderSize()) {
				error("Invalid Text bitmap %04x:%04x", PRINT_REG(bitmap));
			}
	}

	/**
	 * Allocates and initialises a new bitmap in the given
	 * segment manager.
	 */
	inline BitmapResource(SegManager *segMan, const int16 width, const int16 height, const uint8 skipColor, const int16 displaceX, const int16 displaceY, const int16 scaledWidth, const int16 scaledHeight, const uint32 hunkPaletteOffset, const bool remap) {

		_object = segMan->allocateHunkEntry("Bitmap()", getBitmapSize(width, height));
		_bitmap = segMan->getHunkPointer(_object);

		const uint16 bitmapHeaderSize = getBitmapHeaderSize();

		setWidth(width);
		setHeight(height);
		setDisplace(Common::Point(displaceX, displaceY));
		setSkipColor(skipColor);
		_bitmap[9] = 0;
		WRITE_SCI11ENDIAN_UINT16(_bitmap + 10, 0);
		setRemap(remap);
		setDataSize(width * height);
		WRITE_SCI11ENDIAN_UINT32(_bitmap + 16, 0);
		setHunkPaletteOffset(hunkPaletteOffset);
		setDataOffset(bitmapHeaderSize);
		setUncompressedDataOffset(bitmapHeaderSize);
		setControlOffset(0);
		setScaledWidth(scaledWidth);
		setScaledHeight(scaledHeight);
	}

	reg_t getObject() const {
		return _object;
	}

	BITMAP_PROPERTY(16, Width, 0);
	BITMAP_PROPERTY(16, Height, 2);

	inline Common::Point getDisplace() const {
		return Common::Point(
			(int16)READ_SCI11ENDIAN_UINT16(_bitmap + 4),
			(int16)READ_SCI11ENDIAN_UINT16(_bitmap + 6)
		);
	}

	inline void setDisplace(const Common::Point &displace) {
		WRITE_SCI11ENDIAN_UINT16(_bitmap + 4, (uint16)displace.x);
		WRITE_SCI11ENDIAN_UINT16(_bitmap + 6, (uint16)displace.y);
	}

	inline uint8 getSkipColor() const {
		return _bitmap[8];
	}

	inline void setSkipColor(const uint8 skipColor) {
		_bitmap[8] = skipColor;
	}

	inline bool getRemap() const {
		return READ_SCI11ENDIAN_UINT16(_bitmap + 10) & kBitmapRemap;
	}

	inline void setRemap(const bool remap) {
		uint16 flags = READ_SCI11ENDIAN_UINT16(_bitmap + 10);
		if (remap) {
			flags |= kBitmapRemap;
		} else {
			flags &= ~kBitmapRemap;
		}
		WRITE_SCI11ENDIAN_UINT16(_bitmap + 10, flags);
	}

	BITMAP_PROPERTY(32, DataSize, 12);

	inline uint32 getHunkPaletteOffset() const {
		return READ_SCI11ENDIAN_UINT32(_bitmap + 20);
	}

	void setHunkPaletteOffset(uint32 hunkPaletteOffset) {
		if (hunkPaletteOffset) {
			hunkPaletteOffset += getBitmapHeaderSize();
		}

		WRITE_SCI11ENDIAN_UINT32(_bitmap + 20, hunkPaletteOffset);
	}

	BITMAP_PROPERTY(32, DataOffset, 24);

	// NOTE: This property is used as a "magic number" for
	// validating that a block of memory is a valid bitmap,
	// and so is always set to the size of the header.
	BITMAP_PROPERTY(32, UncompressedDataOffset, 28);

	// NOTE: This property always seems to be zero
	BITMAP_PROPERTY(32, ControlOffset, 32);

	inline uint16 getScaledWidth() const {
		if (getDataOffset() >= 40) {
			return READ_SCI11ENDIAN_UINT16(_bitmap + 36);
		}

		// SCI2 bitmaps did not have scaling ability
		return 320;
	}

	inline void setScaledWidth(uint16 scaledWidth) {
		if (getDataOffset() >= 40) {
			WRITE_SCI11ENDIAN_UINT16(_bitmap + 36, scaledWidth);
		}
	}

	inline uint16 getScaledHeight() const {
		if (getDataOffset() >= 40) {
			return READ_SCI11ENDIAN_UINT16(_bitmap + 38);
		}

		// SCI2 bitmaps did not have scaling ability
		return 200;
	}

	inline void setScaledHeight(uint16 scaledHeight) {
		if (getDataOffset() >= 40) {
			WRITE_SCI11ENDIAN_UINT16(_bitmap + 38, scaledHeight);
		}
	}

	inline byte *getPixels() {
		return _bitmap + getUncompressedDataOffset();
	}
};

class GfxFont;

/**
 * This class handles text calculation and rendering for
 * SCI32 games. The text calculation system in SCI32 is
 * nearly the same as SCI16, which means this class behaves
 * similarly. Notably, GfxText32 maintains drawing
 * parameters across multiple calls.
 */
class GfxText32 {
private:
	SegManager *_segMan;
	GfxCache *_cache;

	/**
	 * The resource ID of the default font used by the game.
	 *
	 * @todo Check all SCI32 games to learn what their
	 * default font is.
	 */
	static int16 _defaultFontId;

	/**
	 * The width and height of the currently active text
	 * bitmap, in text-system coordinates.
	 *
	 * @note These are unsigned in the actual engine.
	 */
	int16 _width, _height;

	/**
	 * The color used to draw text.
	 */
	uint8 _foreColor;

	/**
	 * The background color of the text box.
	 */
	uint8 _backColor;

	/**
	 * The transparent color of the text box. Used when
	 * compositing the bitmap onto the screen.
	 */
	uint8 _skipColor;

	/**
	 * The rect where the text is drawn within the bitmap.
	 * This rect is clipped to the dimensions of the bitmap.
	 */
	Common::Rect _textRect;

	/**
	 * The text being drawn to the currently active text
	 * bitmap.
	 */
	Common::String _text;

	/**
	 * The font being used to draw the text.
	 */
	GuiResourceId _fontId;

	/**
	 * The color of the text box border.
	 */
	int16 _borderColor;

	/**
	 * TODO: Document
	 */
	bool _dimmed;

	/**
	 * The text alignment for the drawn text.
	 */
	TextAlign _alignment;

	int16 _field_20;

	/**
	 * TODO: Document
	 */
	int16 _field_22;

	int _field_2C, _field_30, _field_34, _field_38;

	int16 _field_3C;

	/**
	 * The position of the text draw cursor.
	 */
	Common::Point _drawPosition;

	void drawFrame(const Common::Rect &rect, const int16 size, const uint8 color, const bool doScaling);

	void drawChar(const char charIndex);
	void drawText(const uint index, uint length);

	/**
	 * Gets the length of the longest run of text available
	 * within the currently loaded text, starting from the
	 * given `charIndex` and running for up to `maxWidth`
	 * pixels. Returns the number of characters that can be
	 * written, and mutates the value pointed to by
	 * `charIndex` to point to the index of the next
	 * character to render.
	 */
	uint getLongest(uint *charIndex, const int16 maxWidth);

	/**
	 * Gets the pixel width of a substring of the currently
	 * loaded text, without scaling.
	 */
	int16 getTextWidth(const uint index, uint length) const;

	inline Common::Rect scaleRect(const Common::Rect &rect) {
		Common::Rect scaledRect(rect);
		int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
		Ratio scaleX(_scaledWidth, scriptWidth);
		Ratio scaleY(_scaledHeight, scriptHeight);
		mulinc(scaledRect, scaleX, scaleY);
		return scaledRect;
	}

public:
	GfxText32(SegManager *segMan, GfxCache *fonts);

	/**
	 * The memory handle of the currently active bitmap.
	 */
	reg_t _bitmap;

	/**
	 * The size of the x-dimension of the coordinate system
	 * used by the text renderer.
	 */
	int16 _scaledWidth;

	/**
	 * The size of the y-dimension of the coordinate system
	 * used by the text renderer.
	 */
	int16 _scaledHeight;

	/**
	 * The currently active font resource used to write text
	 * into the bitmap.
	 *
	 * @note SCI engine builds the font table directly
	 * inside of FontMgr; we use GfxFont instead.
	 */
	GfxFont *_font;

	/**
	 * Creates a plain font bitmap with a flat color
	 * background.
	 */
	reg_t createFontBitmap(int16 width, int16 height, const Common::Rect &rect, const Common::String &text, const uint8 foreColor, const uint8 backColor, const uint8 skipColor, const GuiResourceId fontId, TextAlign alignment, const int16 borderColor, bool dimmed, const bool doScaling);

	/**
	 * Creates a font bitmap with a view background.
	 */
	reg_t createFontBitmap(const CelInfo32 &celInfo, const Common::Rect &rect, const Common::String &text, const int16 foreColor, const int16 backColor, const GuiResourceId fontId, const int16 skipColor, const int16 borderColor, const bool dimmed);

	/**
	 * Creates a font bitmap with a title.
	 */
	reg_t createTitledBitmap(const int16 width, const int16 height, const Common::Rect &textRect, const Common::String &text, const int16 foreColor, const int16 backColor, const int16 skipColor, const GuiResourceId fontId, const TextAlign alignment, const int16 borderColor, Common::String &title, const int16 titleForeColor, const int16 titleBackColor, const GuiResourceId titleFontId, const bool doScaling);

	inline int scaleUpWidth(int value) const {
		const int scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		return (value * scriptWidth + _scaledWidth - 1) / _scaledWidth;
	}

	inline int scaleUpHeight(int value) const {
		const int scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
		return (value * scriptHeight + _scaledHeight - 1) / _scaledHeight;
	}

	/**
	 * Draws the text to the bitmap.
	 */
	void drawTextBox();

	/**
	 * Draws the given text to the bitmap.
	 *
	 * @note The original engine holds a reference to a
	 * shared string which lets the text be updated from
	 * outside of the font manager. Instead, we give this
	 * extra signature to send the text to draw.
	 *
	 * TODO: Use shared string instead?
	 */
	void drawTextBox(const Common::String &text);

	/**
	 * Erases the given rect by filling with the background
	 * color.
	 */
	void erase(const Common::Rect &rect, const bool doScaling);

	void invertRect(const reg_t bitmap, const int16 bitmapStride, const Common::Rect &rect, const uint8 foreColor, const uint8 backColor, const bool doScaling);

	/**
	 * Sets the font to be used for rendering and
	 * calculation of text dimensions.
	 */
	void setFont(const GuiResourceId fontId);

	/**
	 * Gets the width of a character.
	 */
	uint16 getCharWidth(const char charIndex, const bool doScaling) const;

	/**
	 * Retrieves the width and height of a block of text.
	 */
	Common::Rect getTextSize(const Common::String &text, const int16 maxWidth, bool doScaling);

	/**
	 * Gets the pixel width of a substring of the currently
	 * loaded text, with scaling.
	 */
	int16 getTextWidth(const Common::String &text, const uint index, const uint length);

	/**
	 * Retrieves the width of a line of text.
	 */
	int16 getStringWidth(const Common::String &text);
};

} // End of namespace Sci

#endif
