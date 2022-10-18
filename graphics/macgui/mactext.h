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
	uint32 fgcolor;
	// to determine whether the next word is part of this one
	bool wordContinuation;
	const Font *font;
	MacWindowManager *wm;

	MacFontRun() {
		wm = nullptr;
		fontId = textSlant = fontSize = 0;
		palinfo1 = palinfo2 = palinfo3 = 0;
		fgcolor = 0;
		font = nullptr;
		wordContinuation = false;
	}

	MacFontRun(MacWindowManager *wm_) {
		wm = wm_;
		fontId = textSlant = fontSize = 0;
		palinfo1 = palinfo2 = palinfo3 = 0;
		fgcolor = 0;
		font = nullptr;
		wordContinuation = false;
	}

	MacFontRun(MacWindowManager *wm_, uint16 fontId_, byte textSlant_, uint16 fontSize_,
			uint16 palinfo1_, uint16 palinfo2_, uint16 palinfo3_) {
		setValues(wm_, fontId_, textSlant_, fontSize_, palinfo1_, palinfo2_, palinfo3_);
		wordContinuation = false;
	}

	MacFontRun(MacWindowManager *wm_, const Font *font_, byte textSlant_, uint16 fontSize_,
			uint16 palinfo1_, uint16 palinfo2_, uint16 palinfo3_) {
		setValues(wm_, 0, textSlant_, fontSize_, palinfo1_, palinfo2_, palinfo3_);
		font = font_;
		wordContinuation = false;
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

	Common::CodePage getEncoding();
	bool plainByteMode();
	Common::String getEncodedText();
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
	MacText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *font, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment = kTextAlignLeft, int interlinear = 0, uint16 border = 0, uint16 gutter = 0, uint16 boxShadow = 0, uint16 textShadow = 0, bool fixedDims = true);
	// 0 pixels between the lines by default

	MacText(const Common::U32String &s, MacWindowManager *wm, const MacFont *font, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear = 0, bool fixedDims = true);

	MacText(const Common::U32String &s, MacWindowManager *wm, const Font *font, uint32 fgcolor, uint32 bgcolor, int maxWidth, TextAlign textAlignment, int interlinear = 0, bool fixedDims = true);

	virtual ~MacText();

	virtual void resize(int w, int h);
	bool processEvent(Common::Event &event) override;

	bool needsRedraw() override { return _contentIsDirty || _cursorDirty; }

	void render();
	void undrawCursor();
	void draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff);
	bool draw(ManagedSurface *g, bool forceRedraw = false) override;
	bool draw(bool forceRedraw = false) override;
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
	void setActive(bool active) override;
	void setEditable(bool editable);

	void setColors(uint32 fg, uint32 bg) override;
	// set fgcolor for line x
	void setTextColor(uint32 color, uint32 line);
	void setTextColor(uint32 color, uint32 start, uint32 end);

	void appendText(const Common::U32String &str, int fontId = kMacFontChicago, int fontSize = 12, int fontSlant = kMacFontRegular, bool skipAdd = false);
	void appendText(const Common::U32String &str, int fontId = kMacFontChicago, int fontSize = 12, int fontSlant = kMacFontRegular, uint16 r = 0, uint16 g = 0, uint16 b = 0, bool skipAdd = false);
	void appendText(const Common::U32String &str, const Font *font, uint16 r = 0, uint16 g = 0, uint16 b = 0, bool skipAdd = false);

	int getTextFont() { return _defaultFormatting.fontId; }
	void enforceTextFont(uint16 fontId);

	// because currently, we are counting linespacing as font height
	int getTextSize() { return _defaultFormatting.fontSize; }
	void setTextSize(int textSize);

	int getTextSize(int start, int end);
	void setTextSize(int textSize, int start, int end);

	uint32 getTextColor() { return _defaultFormatting.fgcolor; }
	uint32 getTextColor(int start, int end);

	int getTextFont(int start, int end);
	void setTextFont(int fontId, int start, int end);

	int getTextSlant(int start, int end);
	void setTextSlant(int textSlant, int start, int end);
	void enforceTextSlant(int textSlant);

	// director text related-functions
	int getMouseChar(int x, int y);
	int getMouseWord(int x, int y);
	int getMouseItem(int x, int y);
	int getMouseLine(int x, int y);

private:
	MacFontRun getTextChunks(int start, int end);
	void setTextChunks(int start, int end, int param, void (*callback)(MacFontRun &, int));

	void appendText_(const Common::U32String &strWithFont, uint oldLen);
	void deletePreviousCharInternal(int *row, int *col);
	void insertTextFromClipboard();
	// getStringWidth for mactext version, because we may have the plain bytes mode
	int getStringWidth(MacFontRun &format, const Common::U32String &str);
	int getAlignOffset(int row);
	MacFontRun getFgColor();

public:
	void appendTextDefault(const Common::U32String &str, bool skipAdd = false);
	void appendTextDefault(const Common::String &str, bool skipAdd = false);
	void clearText();
	void removeLastLine();
	int getLineCount() { return _textLines.size(); }
	int getLineCharWidth(int line, bool enforce = false);
	int getLastLineWidth();
	int getTextHeight() { return _textMaxHeight; }
	int getLineHeight(int line);
	int getTextMaxWidth() { return _textMaxWidth; }

	void setText(const Common::U32String &str);

	void setFixDims(bool fixed) { _fixedDims = fixed; }
	bool getFixDims() { return _fixedDims; }

	void deleteSelection();
	void deletePreviousChar(int *row, int *col);
	void addNewLine(int *row, int *col);
	void insertChar(byte c, int *row, int *col);

	void getChunkPosFromIndex(int index, uint &lineNum, uint &chunkNum, uint &offset);
	void getRowCol(int x, int y, int *sx, int *sy, int *row, int *col);
	Common::U32String getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted = false, bool newlines = true);

	Common::U32String getSelection(bool formatted = false, bool newlines = true);
	uint getSelectionIndex(bool start);
	void clearSelection();
	Common::U32String cutSelection();
	const SelectedText *getSelectedText() { return &_selectedText; }

	int getLineSpacing() { return _interLinear; }

	/**
	 * set the selection of mactext
	 * @param pos pos of selection, 0 represent first, -1 represent the end of text
	 * @param start selection start or selection end
	 */
	void setSelection(int pos, bool start);

	Common::U32String getEditedString();
	Common::U32String getText() { return _str; }
	Common::U32String getPlainText();

	void setSelRange(int selStart, int selEnd);

	void scroll(int delta);

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
	void splitString(const Common::U32String &str, int curLine = -1);
	void render(int from, int to, int shadow);
	void render(int from, int to);
	void recalcDims();
	void reallocSurface();

	void drawSelection(int xoff, int yoff);
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
	Common::U32String _str;
	const MacFont *_macFont;

	int _maxWidth;
	int _interLinear;
	int _textShadow;

	bool _fixedDims;

	int _selEnd;
	int _selStart;

	int _textMaxWidth;
	int _textMaxHeight;

	ManagedSurface *_surface;
	ManagedSurface *_shadowSurface;

	TextAlign _textAlignment;

	Common::Array<MacTextLine> _textLines;
	MacFontRun _defaultFormatting;
	MacFontRun _currentFormatting;

	bool _macFontMode;

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
