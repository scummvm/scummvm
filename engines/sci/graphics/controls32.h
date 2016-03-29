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

#ifndef SCI_GRAPHICS_CONTROLS32_H
#define SCI_GRAPHICS_CONTROLS32_H

#include "sci/graphics/text32.h"

namespace Sci {

class GfxCache;
class GfxScreen;
class GfxText32;


struct TextEditor {
	/**
	 * The bitmap where the editor is rendered.
	 */
	reg_t bitmap;

	/**
	 * The width of the editor, in bitmap pixels.
	 */
	int16 width;

	/**
	 * The text in the editor.
	 */
	Common::String text;

	/**
	 * The rect where text should be drawn into the editor,
	 * in bitmap pixels.
	 */
	Common::Rect textRect;

	/**
	 * The color of the border. -1 indicates no border.
	 */
	int16 borderColor;

	/**
	 * The text color.
	 */
	uint8 foreColor;

	/**
	 * The background color.
	 */
	uint8 backColor;

	/**
	 * The transparent color.
	 */
	uint8 skipColor;

	/**
	 * The font used to render the text in the editor.
	 */
	GuiResourceId fontId;

	/**
	 * The current position of the cursor within the editor.
	 */
	uint16 cursorCharPosition;

	/**
	 * Whether or not the cursor is currently drawn to the
	 * screen.
	 */
	bool cursorIsDrawn;

	/**
	 * The rectangle for drawing the input cursor, in bitmap
	 * pixels.
	 */
	Common::Rect cursorRect;

	/**
	 * The maximum allowed text length, in characters.
	 */
	uint16 maxLength;
};


struct ScrollWindowLine {
	/**
	 * ID of the line. In SSCI this was actually a memory handle for the
	 * string of this line. We use a numeric ID instead.
	 */
	reg_t _id;

	// Text properties
	int _alignment;
	int _foreColor;
	GuiResourceId _fontId;

	// Actual text
	Common::String _str;
};

class ScreenItem;

class ScrollWindow {
public:
	ScrollWindow(SegManager *segMan, const Common::Rect &rect,
	             const Common::Point &point, reg_t plane, uint8 fore,
	             uint8 back, GuiResourceId font, TextAlign align, uint8 border);
	~ScrollWindow();

	reg_t add(const Common::String &str, GuiResourceId font, int fore,
	          int align, bool scrollTo);
	void show();
	void hide();
	Common::Rational where() const;

	void upArrow();
	void downArrow();

	reg_t getBitmap() const { return _bitmap; }

protected:

	void update(bool doFrameOut);
	void getLineIndices();


	GfxText32 _gfxText32;

	Common::Array<ScrollWindowLine> _lines;
	Common::Array<int> _startsOfLines;

	Common::String _text;
	Common::String _visibleText;

	int _firstVisibleChar;
	int _topVisibleLine;

	int _lastVisibleChar;
	int _bottomVisibleLine;

	int _numLines;
	int _numVisibleLines;

	reg_t _plane;

	uint8 _foreColor;
	uint8 _backColor;
	uint8 _borderColor;

	GuiResourceId _fontId;
	TextAlign _alignment;

	int16 _fontScaledWidth;
	int16 _fontScaledHeight;

	bool _visible;

	Common::Rect _textRect;
	Common::Rect _screenRect;

	Common::Point _position;

	int _pointSize;

	reg_t _bitmap;

	reg_t _lastLineId;

	ScreenItem *_screenItem;
};

/**
 * Controls class, handles drawing of controls in SCI32 (SCI2, SCI2.1, SCI3) games
 */
class GfxControls32 {
public:
	GfxControls32(SegManager *segMan, GfxCache *cache, GfxText32 *text);

	reg_t kernelEditText(const reg_t controlObject);

	reg_t registerScrollWindow(ScrollWindow *scrollWindow);
	ScrollWindow *getScrollWindow(reg_t id);
	void deregisterScrollWindow(reg_t id);

	Common::Array<reg_t> listObjectReferences();

private:
	SegManager *_segMan;
	GfxCache *_gfxCache;
	GfxText32 *_gfxText32;

	bool _overwriteMode;
	uint32 _nextCursorFlashTick;
	void drawCursor(TextEditor &editor);
	void eraseCursor(TextEditor &editor);
	void flashCursor(TextEditor &editor);

	reg_t _lastScrollWindowId;

	Common::HashMap<int, ScrollWindow *> _scrollWindows;
};

} // End of namespace Sci

#endif
