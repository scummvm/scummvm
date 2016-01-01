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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/system.h"
#include "common/unzip.h"
#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/bdf.h"
#include "graphics/palette.h"
#include "wage/wage.h"
#include "wage/design.h"
#include "wage/entities.h"
#include "wage/gui.h"

namespace Wage {

enum {
	kMenuHeight = 19,
	kMenuPadding = 6,
	kMenuItemHeight = 19,
	kBorderWidth = 17
};

static const byte palette[] = {
	0, 0, 0,           // Black
	0x80, 0x80, 0x80,  // Gray
	0xff, 0xff, 0xff,  // White
	0x00, 0xff, 0x00   // Green
};

static byte checkers[8] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };

static const byte macCursorArrow[] = {
	2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 0, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 0, 0, 2, 3, 3, 3, 3, 3, 3, 3,
	2, 0, 0, 0, 2, 3, 3, 3, 3, 3, 3,
	2, 0, 0, 0, 0, 2, 3, 3, 3, 3, 3,
	2, 0, 0, 0, 0, 0, 2, 3, 3, 3, 3,
	2, 0, 0, 0, 0, 0, 0, 2, 3, 3, 3,
	2, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3,
	2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3,
	2, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2,
	2, 0, 0, 2, 0, 0, 2, 3, 3, 3, 3,
	2, 0, 2, 3, 2, 0, 0, 2, 3, 3, 3,
	2, 2, 3, 3, 2, 0, 0, 2, 3, 3, 3,
	2, 3, 3, 3, 3, 2, 0, 0, 2, 3, 3,
	3, 3, 3, 3, 3, 2, 0, 0, 2, 3, 3,
	3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 3
};

static const byte macCursorBeam[] = {
	0, 0, 3, 3, 3, 0, 0, 3, 3, 3, 3,
	3, 3, 0, 3, 0, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 0, 3, 0, 3, 3, 3, 3, 3, 3,
	0, 0, 3, 3, 3, 0, 0, 3, 3, 3, 3,
};

Gui::Gui() {
	_scene = NULL;
	_sceneDirty = true;
	_screen.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());

	Patterns p;
	p.push_back(checkers);
	Common::Rect r(0, 0, _screen.w, _screen.h);

	_scrollPos = 0;
	_builtInFonts = false;

	g_system->getPaletteManager()->setPalette(palette, 0, 4);

	CursorMan.replaceCursorPalette(palette, 0, 4);
	CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
	_cursorIsArrow = true;
	CursorMan.showMouse(true);

	Design::drawFilledRect(&_screen, r, kColorBlack, p, 1);

	loadFonts();
}

Gui::~Gui() {
}

void Gui::setScene(Scene *scene) {
	if (_scene != scene)
		_sceneDirty = true;

	_scene = scene;
}

void Gui::appendText(String &str) {
	if (!str.contains('\n')) {
		_out.push_back(str);
		return;
	}

	// Okay, we got new lines, need to split it
	// and push substrings individually
	Common::String tmp = "";

	for (int i = 0; i < str.size(); i++) {
		if (str[i] == '\n') {
			_out.push_back(tmp);
			tmp = "";
			continue;
		}

		tmp += str[i];
	}

	_out.push_back(tmp);
}

void Gui::draw() {
	if (_scene != NULL && _sceneDirty) {
		_scene->paint(&_screen, 0, kMenuHeight);
		paintBorder(&_screen, 0, kMenuHeight, _scene->_design->getBounds()->width(), _scene->_design->getBounds()->height(),
				kWindowScene);

		_sceneDirty = false;
	}

	// Render console
	int sceneW = _scene->_design->getBounds()->width();
	int consoleW = _screen.w - sceneW - 2 * kBorderWidth;
	int consoleH = _scene->_design->getBounds()->height() - 2 * kBorderWidth;

	renderConsole(&_screen, sceneW + kBorderWidth, kMenuHeight + kBorderWidth, consoleW, consoleH);
	paintBorder(&_screen, sceneW, kMenuHeight, _screen.w - sceneW, _scene->_design->getBounds()->height(),
		kWindowConsole);

	// Blit to screen
	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, _screen.w, _screen.h);
}

void Gui::drawBox(Graphics::Surface *g, int x, int y, int w, int h) {
	Common::Rect r(x, y, x + w + 1, y + h + 1);

	g->fillRect(r, kColorWhite);
	g->frameRect(r, kColorBlack);
}

void Gui::fillRect(Graphics::Surface *g, int x, int y, int w, int h) {
	Common::Rect r(x, y, x + w, y + h);

	g->fillRect(r, kColorBlack);
}

#define ARROW_W 12
#define ARROW_H 6
const int arrowPixels[ARROW_H][ARROW_W] = {
		{0,0,0,0,0,1,1,0,0,0,0,0},
		{0,0,0,0,1,1,1,1,0,0,0,0},
		{0,0,0,1,1,1,1,1,1,0,0,0},
		{0,0,1,1,1,1,1,1,1,1,0,0},
		{0,1,1,1,1,1,1,1,1,1,1,0},
		{1,1,1,1,1,1,1,1,1,1,1,1}};

void Gui::paintBorder(Graphics::Surface *g, int x, int y, int width, int height, WindowType windowType) {
	bool active, scrollable, closeable, closeBoxPressed, drawTitle;

	switch (windowType) {
	case kWindowScene:
		active = false;
		scrollable = false;
		closeable = false;
		closeBoxPressed = false;
		drawTitle = true;
		break;
	case kWindowConsole:
		active = true;
		scrollable = true;
		closeable = true;
		closeBoxPressed = false;
		drawTitle = false;
		break;
	}

	const int size = kBorderWidth;
	drawBox(g, x, y, size, size);
	drawBox(g, x+width-size-1, y, size, size);
	drawBox(g, x+width-size-1, y+height-size-1, size, size);
	drawBox(g, x, y+height-size-1, size, size);
	drawBox(g, x + size, y + 2, width - 2*size - 1, size - 4);
	drawBox(g, x + size, y + height - size + 1, width - 2*size - 1, size - 4);
	drawBox(g, x + 2, y + size, size - 4, height - 2*size - 1);
	drawBox(g, x + width - size + 1, y + size, size - 4, height - 2*size-1);

	if (active) {
		fillRect(g, x + size, y + 5, width - 2*size - 1, 8);
		fillRect(g, x + size, y + height - 13, width - 2*size - 1, 8);
		fillRect(g, x + 5, y + size, 8, height - 2*size - 1);
		if (!scrollable) {
			fillRect(g, x + width - 13, y + size, 8, height - 2*size - 1);
		} else {
			int x1 = x + width - 15;
			int y1 = y + size + 1;
			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++) {
					if (arrowPixels[yy][xx] != 0) {
						g->hLine(x1+xx, y1+yy, x1+xx, kColorBlack);
					} else {
						g->hLine(x1+xx, y1+yy, x1+xx, kColorWhite);
					}
				}
			}
			fillRect(g, x + width - 13, y + size + ARROW_H, 8, height - 2*size - 1 - ARROW_H*2);
			y1 += height - 2*size - ARROW_H - 2;
			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++) {
					if (arrowPixels[ARROW_H-yy-1][xx] != 0) {
						g->hLine(x1+xx, y1+yy, x1+xx, kColorBlack);
					} else {
						g->hLine(x1+xx, y1+yy, x1+xx, kColorWhite);
					}
				}
			}
		}
		if (closeable) {
			if (closeBoxPressed) {
				fillRect(g, x + 6, y + 6, 6, 6);
			} else {
				drawBox(g, x + 5, y + 5, 7, 7);
			}
		}
	}

	if (drawTitle) {
		const Graphics::Font *font;
		int yOff = 1;

		if (!_builtInFonts) {
			font = FontMan.getFontByName("Chicago-12");

			if (!font)
				warning("Cannot load font Chicago-12");
		}

		if (_builtInFonts || !font) {
			font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
			yOff = 3;
		}

		int w = font->getStringWidth(_scene->_name) + 6;
		int maxWidth = width - size*2 - 7;
		if (w > maxWidth)
			w = maxWidth;
		drawBox(g, x + (width - w) / 2, y, w, size);
		font->drawString(g, _scene->_name, x + (width - w) / 2 + 3, y + yOff, w, kColorBlack);
	}
}

enum {
	kConWOverlap = 20,
	kConHOverlap = 20,
	kConWPadding = 2,
	kConHPadding = 10,
	kConOverscan = 3,
	kLineSpacing = 0
};

void Gui::renderConsole(Graphics::Surface *g, int x, int y, int width, int height) {
	bool fullRedraw = false;
	bool textReflow = false;
	int surfW = width + kConWOverlap * 2;
	int surfH = height + kConHOverlap * 2;

	Common::Rect boundsR(kConWOverlap - kConOverscan, kConHOverlap - kConOverscan,
					width + kConWOverlap + kConOverscan, height + kConHOverlap + kConOverscan);
	Common::Rect fullR(0, 0, surfW, surfH);

	if (_console.w != surfW || _console.h != surfH) {
		if (_console.w != surfW)
			textReflow = true;

		_console.free();

		_console.create(surfW, surfH, Graphics::PixelFormat::createFormatCLUT8());
		fullRedraw = true;
	}

	if (fullRedraw)
		_console.fillRect(fullR, kColorWhite);

	const Graphics::Font *font;

	if (!_builtInFonts) {
		char fontName[128];

		snprintf(fontName, 128, "%s-%d", _scene->getFontName(), _scene->_fontSize);
		font = FontMan.getFontByName(fontName);

		if (!font)
			warning("Cannot load font %s", fontName);
	}

	if (_builtInFonts || !font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);

	int lineHeight = font->getFontHeight() + kLineSpacing;
	int textW = width - kConWPadding * 2;
	int textH = height - kConHPadding * 2;

	if (textReflow) {
		_lines.clear();

		for (int i = 0; i < _out.size(); i++) {
			Common::StringArray wrappedLines;

			font->wordWrapText(_out[i], textW, wrappedLines);

			for (Common::StringArray::const_iterator j = wrappedLines.begin(); j != wrappedLines.end(); ++j)
				_lines.push_back(*j);
		}
	}

	const int firstLine = _scrollPos / lineHeight;
	const int lastLine = MIN((_scrollPos + textH) / lineHeight + 1, _lines.size());
	const int xOff = kConWOverlap;
	const int yOff = kConHOverlap;
	int x1 = xOff + kConWPadding;
	int y1 = yOff - (_scrollPos % lineHeight) + kConHPadding;

	for (int line = firstLine; line < lastLine; line++) {
		const char *str = _lines[line].c_str();

		if (*str)
			font->drawString(&_console, _lines[line], x1, y1, textW, kColorBlack);

		y1 += lineHeight;
	}

	_consoleTextArea.left = x;
	_consoleTextArea.top = y;
	_consoleTextArea.right = x + width;
	_consoleTextArea.bottom = y + height;

	g->copyRectToSurface(_console, x - kConOverscan, y - kConOverscan, boundsR);
}

void Gui::loadFonts() {
	Common::Archive *dat;

	dat = Common::makeZipArchive("wage.dat");

	if (!dat) {
		warning("Could not find wage.dat. Falling back to built-in fonts");
		_builtInFonts = true;
	}

	Common::ArchiveMemberList list;
	dat->listMembers(list);

	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) {
		Common::SeekableReadStream *stream = dat->createReadStreamForMember((*it)->getName());

		Graphics::BdfFont *font = Graphics::BdfFont::loadFont(*stream);

		delete stream;

		Common::String fontName = (*it)->getName();

		// Trim the .bdf extension
		for (int i = fontName.size() - 1; i >= 0; --i) {
			if (fontName[i] == '.') {
				while ((uint)i < fontName.size()) {
					fontName.deleteLastChar();
				}
				break;
			}
		}

		FontMan.assignFontToName(fontName, font);

		debug(2, " %s", fontName.c_str());
	}

	_builtInFonts = false;

	delete dat;
}

void Gui::mouseMove(int x, int y) {
	if (_consoleTextArea.contains(x, y)) {
		if (_cursorIsArrow) {
			CursorMan.replaceCursor(macCursorBeam, 11, 16, 3, 8, 3);
			_cursorIsArrow = false;
		}
	} else if (_cursorIsArrow == false) {
		CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
		_cursorIsArrow = true;
	}
}

} // End of namespace Wage
