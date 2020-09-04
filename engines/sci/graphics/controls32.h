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

enum MessageBoxStyle {
	kMessageBoxOK    = 0x0,
	kMessageBoxYesNo = 0x4
};

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
	 * Whether or not the cursor is currently drawn to the screen.
	 */
	bool cursorIsDrawn;

	/**
	 * The rectangle for drawing the input cursor, in bitmap pixels.
	 */
	Common::Rect cursorRect;

	/**
	 * The maximum allowed text length, in characters.
	 */
	uint16 maxLength;
};

/**
 * A single block of text written to a ScrollWindow.
 */
struct ScrollWindowEntry {
	/**
	 * ID of the line. In SSCI this was actually a memory handle for the string
	 * of this line. We use a simple numeric ID instead.
	 */
	reg_t id;

	/**
	 * The alignment to use when rendering this line of text. If -1, the default
	 * alignment from the corresponding ScrollWindow will be used.
	 */
	TextAlign alignment;

	/**
	 * The color to use to render this line of text. If -1, the default
	 * foreground color from the corresponding ScrollWindow will be used.
	 */
	int16 foreColor;

	/**
	 * The font to use to render this line of text. If -1, the default font from
	 * the corresponding ScrollWindow will be used.
	 */
	GuiResourceId fontId;

	/**
	 * The text.
	 */
	Common::String text;
};

class ScreenItem;

/**
 * A scrollable text window.
 */
class ScrollWindow {
public:
	ScrollWindow(SegManager *segMan, const Common::Rect &gameRect, const Common::Point &position, const reg_t planeObj, const uint8 defaultForeColor, const uint8 defaultBackColor, const GuiResourceId defaultFontId, const TextAlign defaultAlignment, const int16 defaultBorderColor, const uint16 maxNumEntries);
	~ScrollWindow();

	/**
	 * Adds a new text entry to the window. If `fontId`, `foreColor`, or
	 * `alignment` are `-1`, the ScrollWindow's default values will be used.
	 */
	reg_t add(const Common::String &text, const GuiResourceId fontId, const int16 foreColor, const TextAlign alignment, const bool scrollTo);

	/**
	 * Modifies an existing text entry with the given ID. If `fontId`,
	 * `foreColor`, or `alignment` are `-1`, the ScrollWindow's default values
	 * will be used.
	 */
	reg_t modify(const reg_t id, const Common::String &text, const GuiResourceId fontId, const int16 foreColor, const TextAlign alignment, const bool scrollTo);

	/**
	 * Shows the ScrollWindow if it is not already visible.
	 */
	void show();

	/**
	 * Hides the ScrollWindow if it is currently visible.
	 */
	void hide();

	/**
	 * Gets the number of lines that the content of a ScrollWindow is scrolled
	 * upward, as a ratio of the total number of lines of content.
	 */
	Ratio where() const;

	/**
	 * Scrolls the window to a specific location.
	 */
	void go(const Ratio location);

	/**
	 * Scrolls the window to the top.
	 */
	void home();

	/**
	 * Scrolls the window to the bottom.
	 */
	void end();

	/**
	 * Scrolls the window up one line.
	 */
	void upArrow();

	/**
	 * Scrolls the window down one line.
	 */
	void downArrow();

	/**
	 * Scrolls the window up by one page.
	 */
	void pageUp();

	/**
	 * Scrolls the window down by one page.
	 */
	void pageDown();

	/**
	 * Gets a reference to the in-memory bitmap that is used to render the text
	 * in the ScrollWindow.
	 */
	const reg_t getBitmap() const { return _bitmap; }

private:
	SegManager *_segMan;

	typedef Common::Array<ScrollWindowEntry> EntriesList;

	/**
	 * A convenience function that fills a ScrollWindowEntry's properties.
	 */
	void fillEntry(ScrollWindowEntry &entry, const Common::String &text, const GuiResourceId fontId, const int16 foreColor, const TextAlign alignment);

	/**
	 * Rescans the entire text of the ScrollWindow when an entry is added or
	 * modified, calculating the character offsets of all line endings, the
	 * total number of lines of text, the height of the viewport (in lines of
	 * text), the last character visible in the viewport (assuming the viewport
	 * is scrolled to the top), and the line index of the bottommost visible
	 * line (assuming the viewport is scrolled to the top).
	 */
	void computeLineIndices();

	/**
	 * Calculates which text is visible within the ScrollWindow's viewport and
	 * renders the text to the internal bitmap.
	 *
	 * If `doFrameOut` is true, the screen will be refreshed immediately instead
	 * of waiting for the next call to `kFrameOut`.
	 */
	void update(const bool doFrameOut);

	/**
	 * The text renderer.
	 */
	GfxText32 _gfxText32;

	/**
	 * The individual text entries added to the ScrollWindow.
	 */
	EntriesList _entries;

	/**
	 * The maximum number of entries allowed. Once this limit is reached, the
	 * oldest entry will be removed when a new entry is added.
	 */
	uint _maxNumEntries;

	/**
	 * A mapping from a line index to the line's character offset in `_text`.
	 */
	Common::Array<int> _startsOfLines;

	/**
	 * All text added to the window.
	 */
	Common::String _text;

	/**
	 * Text that is within the viewport of the ScrollWindow.
	 */
	Common::String _visibleText;

	/**
	 * The offset of the first visible character in `_text`.
	 */
	int _firstVisibleChar;

	/**
	 * The index of the line that is at the top of the viewport.
	 */
	int _topVisibleLine;

	/**
	 * The index of the last visible character in `_text`, or -1 if there is no
	 * text.
	 */
	int _lastVisibleChar;

	/**
	 * The index of the line that is at the bottom of the viewport, or -1 if
	 * there is no text.
	 */
	int _bottomVisibleLine;

	/**
	 * The total number of lines in the backbuffer. This number may be higher
	 * than the total number of entries if an entry contains newlines.
	 */
	int _numLines;

	/**
	 * The number of lines that are currently visible in the text area of the
	 * window.
	 */
	int _numVisibleLines;

	/**
	 * The plane in which the ScrollWindow should be rendered.
	 */
	reg_t _plane;

	/**
	 * The default text color.
	 */
	uint8 _foreColor;

	/**
	 * The default background color of the text bitmap.
	 */
	uint8 _backColor;

	/**
	 * The default border color of the text bitmap. If -1, the viewport will
	 * have no border.
	 */
	int16 _borderColor;

	/**
	 * The default font used for rendering text into the ScrollWindow.
	 */
	GuiResourceId _fontId;

	/**
	 * The default text alignment used for rendering text into the ScrollWindow.
	 */
	TextAlign _alignment;

	/**
	 * The visibility of the ScrollWindow.
	 */
	bool _visible;

	/**
	 * The dimensions of the text box inside the font bitmap, in text-system
	 * coordinates.
	 */
	Common::Rect _textRect;

	/**
	 * The top-left corner of the ScrollWindow's screen item, in game script
	 * coordinates, relative to the parent plane.
	 */
	Common::Point _position;

	/**
	 * The height of the default font in screen pixels. All fonts rendered into
	 * the ScrollWindow must have this same height.
	 */
	uint8 _pointSize;

	/**
	 * The bitmap used to render text.
	 */
	reg_t _bitmap;

	/**
	 * A monotonically increasing ID used to identify text entries added to the
	 * ScrollWindow.
	 */
	uint16 _nextEntryId;

	/**
	 * The ScrollWindow's screen item.
	 */
	ScreenItem *_screenItem;
};

/**
 * Controls class, handles drawing of UI controls in SCI32 games that use kernel
 * controls instead of custom script controls.
 */
class GfxControls32 {
public:
	GfxControls32(SegManager *segMan, GfxCache *cache, GfxText32 *text);
	~GfxControls32();

private:
	SegManager *_segMan;
	GfxCache *_gfxCache;
	GfxText32 *_gfxText32;

#pragma mark -
#pragma mark Text input control
public:
	reg_t kernelEditText(const reg_t controlObject);
	reg_t kernelInputText(const reg_t textObject, const reg_t titleTextObject, const int16 maxTextLength);

private:
	/**
	 * If true, typing will overwrite text that already exists at the text
	 * cursor's current position.
	 */
	bool _overwriteMode;

	/**
	 * The tick at which the text cursor should be toggled by `flashCursor`.
	 */
	uint32 _nextCursorFlashTick;

	/**
	 * Draws the text cursor for the given editor.
	 */
	void drawCursor(TextEditor &editor);

	/**
	 * Erases the text cursor for the given editor.
	 */
	void eraseCursor(TextEditor &editor);

	/**
	 * Toggles the text cursor for the given editor to be either drawn or
	 * erased.
	 */
	void flashCursor(TextEditor &editor);

	/**
	* Processes an edit text event during a text box event loop. Returns true if
	* the event changed the text.
	*/
	bool processEditTextEvent(const SciEvent &event, TextEditor &editor, ScreenItem *screenItem, bool &clearTextOnInput);

#pragma mark -
#pragma mark Scrollable window control
public:
	/**
	 * Creates a new scrollable window and returns the ID for the new window,
	 * which is used by game scripts to interact with scrollable windows.
	 */
	reg_t makeScrollWindow(const Common::Rect &gameRect, const Common::Point &position, const reg_t plane, const uint8 defaultForeColor, const uint8 defaultBackColor, const GuiResourceId defaultFontId, const TextAlign defaultAlignment, const int16 defaultBorderColor, const uint16 maxNumEntries);

	/**
	 * Gets a registered ScrollWindow instance by ID.
	 */
	ScrollWindow *getScrollWindow(const reg_t id);

	/**
	 * Destroys the scroll window with the given ID.
	 */
	void destroyScrollWindow(const reg_t id);

private:
	typedef Common::HashMap<uint16, ScrollWindow *> ScrollWindowMap;

	/**
	 * Monotonically increasing ID used to identify ScrollWindow instances.
	 */
	uint16 _nextScrollWindowId;

	/**
	 * A lookup table for registered ScrollWindow instances.
	 */
	ScrollWindowMap _scrollWindows;

#pragma mark -
#pragma mark Message box
public:
	/**
	 * Displays an OS-level message dialog.
	 */
	reg_t kernelMessageBox(const Common::String &message, const Common::String &title, const uint16 style);

private:
	/**
	 * Convenience function for creating and showing a message box.
	 */
	int16 showMessageBox(const Common::U32String &message, const Common::U32String &okLabel, const Common::U32String &altLabel, const int16 okValue, const int16 altValue);
};

} // End of namespace Sci

#endif
