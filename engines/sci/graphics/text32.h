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
	kTextAlignRight  = -1
};

/**
 * This class handles text calculation and rendering for
 * SCI32 games. The text calculation system in SCI32 is
 * nearly the same as SCI16, which means this class behaves
 * similarly. Notably, GfxText32 maintains drawing
 * parameters across multiple calls.
 */
class GfxText32 {
private:
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
	 */
	int16 _width, _height;

	/**
	 * The colour used to draw text.
	 */
	uint8 _foreColor;

	/**
	 * The background colour of the text box.
	 */
	uint8 _backColor;

	/**
	 * The transparent colour of the text box. Used when
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
	 * The colour of the text box border.
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

	/**
	 * The memory handle of the currently active bitmap.
	 */
	reg_t _bitmap;

	/**
	 * TODO: Document
	 */
	int _field_22;

	/**
	 * The currently active font resource used to write text
	 * into the bitmap.
	 *
	 * @note SCI engine builds the font table directly
	 * inside of FontMgr; we use GfxFont instead.
	 */
	GfxFont *_font;

	// TODO: This is general for all CelObjMem and should be
	// put in a single location, like maybe as a static
	// method of CelObjMem?!
	void buildBitmapHeader(byte *bitmap, const int16 width, const int16 height, const uint8 skipColor, const int16 displaceX, const int16 displaceY, const int16 scaledWidth, const int16 scaledHeight, const uint32 hunkPaletteOffset, const bool useRemap) const;

	void drawFrame(const Common::Rect &rect, const int size, const uint8 color, const bool doScaling);
	void drawTextBox();
	void erase(const Common::Rect &rect, const bool doScaling);

	inline Common::Rect scaleRect(const Common::Rect &rect) {
		Common::Rect scaledRect(rect);
		int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
		int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
		Ratio scaleX(_scaledWidth, scriptWidth);
		Ratio scaleY(_scaledHeight, scriptHeight);
		mul(scaledRect, scaleX, scaleY);
		return scaledRect;
	}

public:
	GfxText32(SegManager *segMan, GfxCache *fonts, GfxScreen *screen);

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

	reg_t createFontBitmap(int16 width, int16 height, const Common::Rect &rect, const Common::String &text, const uint8 foreColor, const uint8 backColor, const uint8 skipColor, const GuiResourceId fontId, TextAlign alignment, const int16 borderColor, bool dimmed, const bool doScaling, reg_t *outBitmapObject);

	reg_t createTitledFontBitmap(CelInfo32 &celInfo, Common::Rect &rect, Common::String &text, int16 foreColor, int16 backColor, int font, int16 skipColor, int16 borderColor, bool dimmed, void *unknown1);

#pragma mark -
#pragma mark Old stuff

	reg_t createTextBitmap(reg_t textObject, uint16 maxWidth = 0, uint16 maxHeight = 0, reg_t prevHunk = NULL_REG);
	reg_t createScrollTextBitmap(Common::String text, reg_t textObject, uint16 maxWidth = 0, uint16 maxHeight = 0, reg_t prevHunk = NULL_REG);
	void drawTextBitmap(int16 x, int16 y, Common::Rect planeRect, reg_t textObject);
	void drawScrollTextBitmap(reg_t textObject, reg_t hunkId, uint16 x, uint16 y);
	void disposeTextBitmap(reg_t hunkId);
	int16 GetLongest(const char *text, int16 maxWidth, GfxFont *font);

	void kernelTextSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);

private:
	reg_t createTextBitmapInternal(Common::String &text, reg_t textObject, uint16 maxWidth, uint16 maxHeight, reg_t hunkId);
	void drawTextBitmapInternal(int16 x, int16 y, Common::Rect planeRect, reg_t textObject, reg_t hunkId);
	int16 Size(Common::Rect &rect, const char *text, GuiResourceId fontId, int16 maxWidth);
	void Width(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight, bool restoreFont);
	void StringWidth(const char *str, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight);

	SegManager *_segMan;
	GfxCache *_cache;
	GfxScreen *_screen;
};

} // End of namespace Sci

#endif
