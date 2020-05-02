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

#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/font.h"
#include "graphics/macgui/macfontmanager.h"

namespace Graphics {

class MacWindowManager;

struct MacFontRun {
	Common::U32String text;

	uint16 fontId;
	byte textSlant;
	uint16 fontSize;
	uint16 palinfo1;
	uint16 palinfo2;
	uint16 palinfo3;

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

class MacText {
	friend class MacEditableText;

public:
	MacText(const Common::U32String &s, MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor,
			int maxWidth = -1, TextAlign textAlignment = kTextAlignLeft, int interlinear = 0);
	MacText(const Common::String &s, MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor,
			int maxWidth = -1, TextAlign textAlignment = kTextAlignLeft, int interlinear = 0);
			// 0 pixels between the lines by default
	~MacText();

	int getInterLinear() { return _interLinear; }
	void setInterLinear(int interLinear);
	void setMaxWidth(int maxWidth);
	void setDefaultFormatting(uint16 fontId, byte textSlant, uint16 fontSize,
			uint16 palinfo1, uint16 palinfo2, uint16 palinfo3);

	const MacFontRun &getDefaultFormatting() { return _defaultFormatting; }

	void draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff);
	void drawToPoint(ManagedSurface *g, Common::Rect srcRect, Common::Point dstPoint);
	void drawToPoint(ManagedSurface *g, Common::Point dstPoint);
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

	void render();
	Graphics::ManagedSurface *getSurface() { return _surface; }

	void getRowCol(int x, int y, int *sx, int *sy, int *row, int *col);

	Common::U32String getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted = false, bool newlines = true);

private:
	void chopChunk(const Common::U32String &str, int *curLine);
	void splitString(const Common::U32String &s, int curLine = -1);
	void render(int from, int to);
	void recalcDims();
	void reallocSurface();
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

protected:
	MacWindowManager *_wm;

	Common::U32String _str;
	const MacFont *_macFont;
	int _fgcolor, _bgcolor;

	int _maxWidth;
	int _interLinear;

	int _textMaxWidth;
	int _textMaxHeight;

	Graphics::ManagedSurface *_surface;
	bool _fullRefresh;

	TextAlign _textAlignment;

	Common::Array<MacTextLine> _textLines;
	MacFontRun _defaultFormatting;
	MacFontRun _currentFormatting;
};

} // End of namespace Graphics

#endif
