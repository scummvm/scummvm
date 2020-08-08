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

#ifndef GRAPHICS_MACGUI_MACTEXT_H
#define GRAPHICS_MACGUI_MACTEXT_H

#include "common/timer.h"
#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindow.h"

namespace Graphics {

class MacMenu;
class MacText;
class MacWidget;
class MacWindow;
class MacWindowManager;

struct MacFontRun {
	Common::U32String text;

	uint16 fontId;
	byte textSlant;
	uint16 fontSize;
	uint16 palinfo1;
	uint16 palinfo2;
	uint16 palinfo3;
	uint16 fgcolor;

	const Font *font;
	MacWindowManager *wm;

	MacFontRun() {
		wm = nullptr;
		fontId = textSlant = fontSize = 0;
		palinfo1 = palinfo2  = palinfo3 = 0;
		font = nullptr;
	}

	MacFontRun(MacWindowManager *wm_, uint16 fontId_, byte textSlant_, uint16 fontSize_,
			uint16 palinfo1_, uint16 palinfo2_, uint16 palinfo3_) {
		setValues(wm_, fontId_, textSlant_, fontSize_, palinfo1_, palinfo2_, palinfo3_);
	}

	void setValues(MacWindowManager *wm_, uint16 fontId_, byte textSlant_, uint16 fontSize_,
			uint16 palinfo1_, uint16 palinfo2_, uint16 palinfo3_) {
		wm        = wm_;
		fontId    = fontId_;
		textSlant = textSlant_;
		fontSize  = fontSize_;
		palinfo1  = palinfo1_;
		palinfo2  = palinfo2_;
		palinfo3  = palinfo3_;
		fgcolor   = wm_->findBestColor(palinfo1_ & 0xff, palinfo2_ & 0xff, palinfo3_ & 0xff);
		font      = nullptr;
	}

	const Font *getFont();

	const Common::String toString();
	bool equals(MacFontRun &to);
};

struct MacTextLine {
	int width;
	int height;
	int y;
	int charwidth;
	bool paragraphEnd;

	Common::Array<MacFontRun> chunks;

	MacTextLine() {
		width = height = charwidth = -1;
		y = 0;
		paragraphEnd = false;
	}

	MacFontRun &firstChunk() { return chunks[0]; }
	MacFontRun &lastChunk() { return chunks[chunks.size() - 1]; }

	/**
	 * Search for a chunk at given char column.
	 *
	 * @param col Requested column, gets modified with in-chunk column
	 * @returns Chunk number
	 *
	 * @note If requested column is too big, returns last character in the line
	 */
	uint getChunkNum(int *col);
};

struct SelectedText {
	int startX, startY;
	int endX, endY;
	int startRow, startCol;
	int endRow, endCol;

	SelectedText() {
		startX = startY = -1;
		endX = endY = -1;
		startRow = startCol = -1;
		endRow = endCol = -1;
	}

	bool needsRender() {
		return startX != endX || startY != endY;
	}
};

class MacText : public MacWidget {
public:
	MacText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment = kTextAlignLeft, int interlinear = 0, uint16 border = 0, uint16 gutter = 0, uint16 boxShadow = 0, uint16 textShadow = 0);
	MacText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::String &s, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment = kTextAlignLeft, int interlinear = 0, uint16 border = 0, uint16 gutter = 0, uint16 boxShadow = 0, uint16 textShadow = 0);
	// 0 pixels between the lines by default

	MacText(const Common::U32String &s, MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear = 0);
	MacText(const Common::String &s, MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear = 0);

	virtual ~MacText();

	virtual void resize(int w, int h);
	virtual bool processEvent(Common::Event &event) override;

	virtual bool needsRedraw() override { return _contentIsDirty || _cursorDirty; }

	void render();
	void undrawCursor();
	void draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff);
	virtual bool draw(ManagedSurface *g, bool forceRedraw = false) override;
	virtual bool draw(bool forceRedraw = false) override;
	void drawToPoint(ManagedSurface *g, Common::Rect srcRect, Common::Point dstPoint);
	void drawToPoint(ManagedSurface *g, Common::Point dstPoint);

	Graphics::ManagedSurface *getSurface() { return _surface; }
	int getInterLinear() { return _interLinear; }
	void setInterLinear(int interLinear);
	void setMaxWidth(int maxWidth);
	void setDefaultFormatting(uint16 fontId, byte textSlant, uint16 fontSize,
														uint16 palinfo1, uint16 palinfo2, uint16 palinfo3);
	const MacFontRun &getDefaultFormatting() { return _defaultFormatting; }

	void setAlignOffset(TextAlign align);
	TextAlign getAlign() { return _textAlignment; }
	virtual Common::Point calculateOffset();
	virtual void setActive(bool active) override;
	void setEditable(bool editable);

	void appendText(const Common::U32String &str, int fontId = kMacFontChicago, int fontSize = 12, int fontSlant = kMacFontRegular, bool skipAdd = false);
	void appendText(const Common::String &str, int fontId = kMacFontChicago, int fontSize = 12, int fontSlant = kMacFontRegular, bool skipAdd = false);
	void appendTextDefault(const Common::U32String &str, bool skipAdd = false);
	void appendTextDefault(const Common::String &str, bool skipAdd = false);
	void clearText();
	void removeLastLine();
	int getLineCount() { return _textLines.size(); }
	int getLineCharWidth(int line, bool enforce = false);
	int getTextHeight() { return _textMaxHeight; }
	int getLineHeight(int line);

	void deletePreviousChar(int *row, int *col);
	void addNewLine(int *row, int *col);
	void insertChar(byte c, int *row, int *col);

	void getRowCol(int x, int y, int *sx, int *sy, int *row, int *col);
	Common::U32String getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted = false, bool newlines = true);

	Common::U32String getSelection(bool formatted = false, bool newlines = true);
	uint getSelectionIndex(bool start);
	void clearSelection();
	Common::U32String cutSelection();
	const SelectedText *getSelectedText() { return &_selectedText; }

	void setSelection(int pos, bool start);

	Common::U32String getEditedString();

private:
	void init();
	bool isCutAllowed();

	/**
	 * Returns line width in pixels. This takes into account chunks.
	 * The result is cached for faster subsequent calls.
	 *
	 * @param line Line number
	 * @param enforce Flag for indicating skipping the cache and computing the width,
	 *                must be called when text gets changed
	 * @param col Compute line width up to specified column, including this column
	 * @return line width in pixels, or 0 for non-existent lines
	 */
	int getLineWidth(int line, bool enforce = false, int col = -1);

	/**
	 * Rewraps paragraph containing given text row.
	 * When text is modified, we redo whole thing again without touching
	 * other paragraphs. Also, cursor position is returned in the arguments
	 */
	void reshuffleParagraph(int *row, int *col);

	void chopChunk(const Common::U32String &str, int *curLine);
	void splitString(const Common::U32String &s, int curLine = -1);
	void render(int from, int to);
	void recalcDims();
	void reallocSurface();

	void scroll(int delta);

	void drawSelection();
	void updateCursorPos();

	void startMarking(int x, int y);
	void updateTextSelection(int x, int y);

public:
	int _cursorX, _cursorY;
	bool _cursorState;
	int _cursorRow, _cursorCol;

	bool _cursorDirty;
	Common::Rect *_cursorRect;
	bool _cursorOff;
	bool _selectable;

	int _scrollPos;

	bool _fullRefresh;

protected:
	Common::Point _alignOffset;

	Common::U32String _str;
	const MacFont *_macFont;

	int _maxWidth;
	int _interLinear;
	int _textShadow;

	int _textMaxWidth;
	int _textMaxHeight;

	ManagedSurface *_surface;

	TextAlign _textAlignment;

	Common::Array<MacTextLine> _textLines;
	MacFontRun _defaultFormatting;
	MacFontRun _currentFormatting;

private:
	ManagedSurface *_cursorSurface;
	ManagedSurface *_cursorSurface2;

	int _editableRow;

	bool _inTextSelection;
	SelectedText _selectedText;

	MacMenu *_menu;
};

} // End of namespace Graphics

#endif
