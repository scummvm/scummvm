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

namespace Graphics {

class MacWindowManager;

struct MacFontRun {
	Common::String text;

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
};

struct MacTextLine {
	int width;
	int height;
	int y;

	Common::Array<MacFontRun> chunks;

	MacTextLine() {
		width = height = -1;
		y = 0;
	}
};

class MacText {
public:
	MacText(Common::String s, MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor,
			int maxWidth = -1, TextAlign textAlignment = kTextAlignLeft, int interlinear = 0);
			// 0 pixels between the lines by default
	~MacText();

	int getInterLinear() { return _interLinear; }
	void setInterLinear(int interLinear);
	void setMaxWidth(int maxWidth);
	void setDefaultFormatting(uint16 fontId_, byte textSlant_, uint16 fontSize_,
			uint16 palinfo1_, uint16 palinfo2_, uint16 palinfo3_) {
				_defaultFormatting.setValues(_defaultFormatting.wm, fontId_, textSlant_, fontSize_, palinfo1_, palinfo2_, palinfo3_);
			}

	void draw(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff);
	void appendText(Common::String str, int fontId = kMacFontChicago, int fontSize = 12, int fontSlant = kMacFontRegular, bool skipAdd = false);
	void appendTextDefault(Common::String str, bool skipAdd = false);
	void clearText();
	void replaceLastLine(Common::String str);
	void removeLastLine();
	int getLineCount() { return _textLines.size(); }
	int getTextHeight() { return _textMaxHeight; }
	int getLineHeight(int line);

	void render();
	Graphics::ManagedSurface *getSurface() { return _surface; }

	void getRowCol(int x, int y, int *sx, int *sy, int *row, int *col);

	Common::String getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted = false, bool newlines = true);

private:
	void splitString(Common::String &s);
	void render(int from, int to);
	void recalcDims();
	void reallocSurface();
	int getLineWidth(int line, bool enforce = false);

private:
	MacWindowManager *_wm;

	Common::String _str;
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
