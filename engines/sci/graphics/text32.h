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

#include "sci/engine/state.h"
#include "sci/graphics/celobj32.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/helpers.h"

namespace Sci {

enum TextAlign {
	kTextAlignDefault = -1,
	kTextAlignLeft    = 0,
	kTextAlignCenter  = 1,
	kTextAlignRight   = 2
};

enum ScrollDirection {
	kScrollUp,
	kScrollDown
};

class GfxFont;

/**
 * This class handles text calculation and rendering for SCI32 games. The text
 * calculation system in SCI32 is nearly the same as SCI16, which means this
 * class behaves similarly. Notably, GfxText32 maintains drawing parameters
 * across multiple calls, instead of requiring all text parameters to be
 * provided on every draw call.
 */
class GfxText32 {
private:
	SegManager *_segMan;
	GfxCache *_cache;

	/**
	 * The width and height of the currently active text bitmap, in text-system
	 * coordinates.
	 *
	 * @note These are unsigned in SSCI.
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
	 * The transparent color of the text box. Used when compositing the bitmap
	 * onto the screen.
	 */
	uint8 _skipColor;

	/**
	 * The rect where the text is drawn within the bitmap. This rect is clipped
	 * to the dimensions of the bitmap.
	 */
	Common::Rect _textRect;

	/**
	 * The text being drawn to the currently active text bitmap.
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
	 * If true, text will be drawn using a dither that draws only every other
	 * pixel of the text.
	 */
	bool _dimmed;

	/**
	 * The text alignment for the drawn text.
	 */
	TextAlign _alignment;

	/**
	 * The position of the text draw cursor.
	 */
	Common::Point _drawPosition;

	void drawFrame(const Common::Rect &rect, const int16 size, const uint8 color, const bool doScaling);

	void drawChar(const uint16 charIndex);
	void drawText(const uint index, uint length);

	/**
	 * Gets the length of the longest run of text available within the currently
	 * loaded text, starting from the given `charIndex` and running for up to
	 * `maxWidth` pixels. Returns the number of characters that can be written,
	 * and mutates the value pointed to by `charIndex` to point to the index of
	 * the next character to render.
	 */
	uint getLongest(uint *charIndex, const int16 maxWidth);

	/**
	 * Gets the pixel width of a substring of the currently loaded text, without
	 * scaling.
	 */
	int16 getTextWidth(const uint index, uint length) const;

	/**
	* Gets the pixel dimensions of a substring of the currently loaded text,
	* without scaling.
	*/
	void getTextDimensions(const uint index, uint length, int16 &width, int16& height) const;

	inline Common::Rect scaleRect(const Common::Rect &rect) {
		Common::Rect scaledRect(rect);
		const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
		const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();
		const Ratio scaleX(_xResolution, scriptWidth);
		const Ratio scaleY(_yResolution, scriptHeight);
		mulinc(scaledRect, scaleX, scaleY);
		return scaledRect;
	}

public:
	GfxText32(SegManager *segMan, GfxCache *fonts);

	/**
	 * Initialises static GfxText32 members.
	 */
	static void init();

	/**
	 * The memory handle of the currently active bitmap.
	 */
	reg_t _bitmap;

	/**
	 * The size of the x-dimension of the coordinate system used by the text
	 * renderer. Static since it was global in SSCI.
	 */
	static int16 _xResolution;

	/**
	 * The size of the y-dimension of the coordinate system used by the text
	 * renderer. Static since it was global in SSCI.
	 */
	static int16 _yResolution;

	/**
	 * The currently active font resource used to write text into the bitmap.
	 *
	 * @note SSCI builds the font table directly inside of FontMgr; we use
	 * GfxFont instead.
	 */
	GfxFont *_font;

	/**
	 * Creates a plain font bitmap with a flat color background.
	 */
	reg_t createFontBitmap(int16 width, int16 height, const Common::Rect &rect, const Common::String &text, const uint8 foreColor, const uint8 backColor, const uint8 skipColor, const GuiResourceId fontId, TextAlign alignment, const int16 borderColor, bool dimmed, const bool doScaling, const bool gc);

	/**
	 * Creates a font bitmap with a view background.
	 */
	reg_t createFontBitmap(const CelInfo32 &celInfo, const Common::Rect &rect, const Common::String &text, const int16 foreColor, const int16 backColor, const GuiResourceId fontId, const int16 skipColor, const int16 borderColor, const bool dimmed, const bool gc);

	/**
	 * Creates a titled font bitmap with a flat color background.
	 */
	reg_t createTitledFontBitmap(int16 width, int16 height, const Common::Rect &textRect, const Common::String &text, const uint8 foreColor, const uint8 backColor, const uint8 skipColor, const GuiResourceId fontId, const TextAlign alignment, const int16 borderColor, const Common::String &title, const uint8 titleForeColor, const uint8 titleBackColor, const GuiResourceId titleFontId, const bool doScaling, const bool gc);

	inline int scaleUpWidth(int value) const {
		const int scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
		return (value * scriptWidth + _xResolution - 1) / _xResolution;
	}

	inline int scaleUpHeight(int value) const {
		const int scriptHeight = g_sci->_gfxFrameout->getScriptHeight();
		return (value * scriptHeight + _yResolution - 1) / _yResolution;
	}

	/**
	 * Draws the text to the bitmap.
	 */
	void drawTextBox();

	/**
	 * Draws the given text to the bitmap.
	 *
	 * @note SSCI holds a reference to a shared string which lets the text be
	 * updated from outside of the font manager. Instead, we use this extra
	 * signature to send the text to draw.
	 */
	void drawTextBox(const Common::String &text);

	/**
	 * Erases the given rect by filling with the background color.
	 */
	void erase(const Common::Rect &rect, const bool doScaling);

	void invertRect(const reg_t bitmap, const int16 bitmapStride, const Common::Rect &rect, const uint8 foreColor, const uint8 backColor, const bool doScaling);

	/**
	 * Sets the font to be used for rendering and calculation of text
	 * dimensions.
	 */
	void setFont(const GuiResourceId fontId);

	/**
	 * Gets the pixel height of the currently loaded font.
	 */
	int16 getScaledFontHeight() const;

	/**
	 * Gets the width of a character.
	 */
	uint16 getCharWidth(const uint16 charIndex, const bool doScaling) const;

	/**
	 * Retrieves the width and height of a block of text.
	 */
	Common::Rect getTextSize(const Common::String &text, const int16 maxWidth, bool doScaling);

	/**
	 * Gets the pixel width of a substring of the currently loaded text, with
	 * scaling.
	 */
	int16 getTextWidth(const Common::String &text, const uint index, const uint length);

	/**
	 * Retrieves the width of a line of text.
	 */
	int16 getStringWidth(const Common::String &text);

	/**
	 * Gets the number of characters of `text`, starting from `index`, that can
	 * be safely rendered into `textRect`.
	 */
	int16 getTextCount(const Common::String &text, const uint index, const Common::Rect &textRect, const bool doScaling);

	/**
	 * Gets the number of characters of `text`, starting from `index`, that can
	 * be safely rendered into `textRect` using the given font.
	 */
	int16 getTextCount(const Common::String &text, const uint index, const GuiResourceId fontId, const Common::Rect &textRect, const bool doScaling);

	/**
	 * Scroll up/down one line. `numLines` is the number of the lines in the
	 * textarea, and `textLine` contains the text to draw as the newly visible
	 * line. Originally FontMgr::DrawOneLine and FontMgr::UpOneLine.
	 */
	void scrollLine(const Common::String &textLine, int numLines, uint8 color, TextAlign align, GuiResourceId fontId, ScrollDirection dir);

	bool SwitchToFont1001OnKorean(const char *text);

};

} // End of namespace Sci

#endif
