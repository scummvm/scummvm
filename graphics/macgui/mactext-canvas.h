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

#ifndef GRAPHICS_MACGUI_MACTEXTCANVAS_H
#define GRAPHICS_MACGUI_MACTEXTCANVAS_H

#include "graphics/macgui/macwindowmanager.h"

namespace Graphics {

class MacText;

struct MacFontRun {
	Common::U32String text;

	uint16 fontId;
	byte textSlant;
	uint16 fontSize;
	uint16 palinfo1;
	uint16 palinfo2;
	uint16 palinfo3;
	uint32 fgcolor;
	const Font *font;
	MacWindowManager *wm;
	Common::String link;  // Substitute to return when hover or click

	MacFontRun() {
		wm = nullptr;
		fontId = textSlant = fontSize = 0;
		palinfo1 = palinfo2 = palinfo3 = 0;
		fgcolor = 0;
		font = nullptr;
	}

	MacFontRun(MacWindowManager *wm_) {
		wm = wm_;
		fontId = textSlant = fontSize = 0;
		palinfo1 = palinfo2 = palinfo3 = 0;
		fgcolor = 0;
		font = nullptr;
	}

	MacFontRun(MacWindowManager *wm_, uint16 fontId_, byte textSlant_, uint16 fontSize_,
			uint16 palinfo1_, uint16 palinfo2_, uint16 palinfo3_) {
		setValues(wm_, fontId_, textSlant_, fontSize_, palinfo1_, palinfo2_, palinfo3_);
	}

	MacFontRun(MacWindowManager *wm_, const Font *font_, byte textSlant_, uint16 fontSize_,
			uint16 palinfo1_, uint16 palinfo2_, uint16 palinfo3_) {
		setValues(wm_, 0, textSlant_, fontSize_, palinfo1_, palinfo2_, palinfo3_);
		font = font_;
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

	bool equals(const MacFontRun *y) {
		return (fontId    == y->fontId &&
				textSlant == y->textSlant &&
				fontSize  == y->fontSize &&
				palinfo1  == y->palinfo1 &&
				palinfo2  == y->palinfo2 &&
				palinfo3  == y->palinfo3 &&
				fgcolor   == y->fgcolor);
	}

	void debugPrint();
};

struct MacTextLine;

class MacTextCanvas {
public:
	Common::Array<MacTextLine> _text;
	ManagedSurface *_surface = nullptr, *_shadowSurface = nullptr;
	int _maxWidth = 0;
	int _textMaxWidth = 0;
	int _textMaxHeight = 0;
	TextAlign _textAlignment = kTextAlignLeft;
	int _interLinear = 0;
	int _textShadow = 0;
	MacWindowManager *_wm = nullptr;
	uint32 _tfgcolor = 0;
	uint32 _tbgcolor = 0;
	bool _macFontMode = true;
	MacText *_macText;
	MacFontRun _defaultFormatting;

public:
	~MacTextCanvas();

	void recalcDims();
	void reallocSurface();
	void render(int from, int to);
	void render(int from, int to, int shadow);
	int getAlignOffset(int row);

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
	int getLineHeight(int line);
	int getLineCharWidth(int line, bool enforce = false);

	void splitString(const Common::U32String &str, int curLine, MacFontRun &defaultFormatting);
	const Common::U32String::value_type *splitString(const Common::U32String::value_type *s, int curLine, MacFontRun &defaultFormatting);

	void chopChunk(const Common::U32String &str, int *curLinePtr, int indent, int maxWidth);
	Common::U32String getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted = false, bool newlines = true);

	/**
	 * Rewraps paragraph containing given text row.
	 * When text is modified, we redo whole thing again without touching
	 * other paragraphs. Also, cursor position is returned in the arguments
	 */
	void reshuffleParagraph(int *row, int *col, MacFontRun &defaultFormatting);
	void setMaxWidth(int maxWidth, MacFontRun &defaultFormatting);

	void debugPrint(const char *prefix = nullptr);

private:
	void processTable(int line, int maxWidth);
	void parsePicExt(const Common::U32String &ext, uint16 &w, uint16 &h, int defpercent);
};

struct MacTextTableRow {
	Common::Array<MacTextCanvas> cells;
	int heght = -1;
};

struct MacTextLine {
	int width = -1;
	int height = -1;
	int minWidth = -1;
	int y = 0;
	int charwidth = -1;
	bool paragraphEnd = false;
	bool wordContinuation = false;
	int indent = 0; // in units
	int firstLineIndent = 0; // in pixels
	Common::Path picfname;
	Common::U32String picalt, pictitle, picext;
	uint16 picpercent = 50;
	Common::Array<MacTextTableRow> *table = nullptr;
	ManagedSurface *tableSurface = nullptr;

	Common::Array<MacFontRun> chunks;

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

} // End of namespace Graphics

#endif
