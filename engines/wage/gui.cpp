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

#include "common/timer.h"
#include "common/unzip.h"
#include "graphics/cursorman.h"
#include "graphics/fonts/bdf.h"
#include "graphics/palette.h"

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/entities.h"
#include "wage/menu.h"
#include "wage/gui.h"
#include "wage/world.h"

namespace Wage {

static const byte palette[] = {
	0, 0, 0,           // Black
	0x80, 0x80, 0x80,  // Gray
	0xff, 0xff, 0xff,  // White
	0x00, 0xff, 0x00   // Green
};

static byte fillPatterns[][8] = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, // kPatternSolid
								  { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 }, // kPatternStripes
								  { 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 }, // kPatternCheckers
								  { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa }  // kPatternCheckers2
};

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

static void cursorTimerHandler(void *refCon) {
    Gui *gui = (Gui *)refCon;

	int x = gui->_cursorX;
	int y = gui->_cursorY;

	if (x == 0 && y == 0)
		return;

	if (!gui->_screen.getPixels())
		return;

	x += gui->_consoleTextArea.left;
	y += gui->_consoleTextArea.top;

	gui->_screen.vLine(x, y, y + kCursorHeight, gui->_cursorState ? kColorBlack : kColorWhite);

	if (!gui->_cursorOff)
		gui->_cursorState = !gui->_cursorState;

	g_system->copyRectToScreen(gui->_screen.getBasePtr(x, y), gui->_screen.pitch, x, y, 1, kCursorHeight);
}

Gui::Gui(WageEngine *engine) {
	_engine = engine;
	_scene = NULL;
	_sceneDirty = true;
	_consoleDirty = true;
	_bordersDirty = true;
	_menuDirty = true;
	_consoleFullRedraw = true;
	_screen.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());

	_scrollPos = 0;
	_consoleLineHeight = 8; // Dummy value which makes sense
	_consoleNumLines = 24; // Dummy value
	_builtInFonts = false;
	_sceneIsActive = false;

	_cursorX = 0;
	_cursorY = 0;
	_cursorState = false;
	_cursorOff = false;

	_inTextSelection = false;
	_selectionStartX = _selectionStartY = -1;
	_selectionEndX = _selectionEndY = -1;

	_inputTextLineNum = 0;

	g_system->getPaletteManager()->setPalette(palette, 0, 4);

	CursorMan.replaceCursorPalette(palette, 0, 4);
	CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
	_cursorIsArrow = true;
	CursorMan.showMouse(true);

	for (int i = 0; i < ARRAYSIZE(fillPatterns); i++)
		_patterns.push_back(fillPatterns[i]);

	loadFonts();

	g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "wageCursor");

	_menu = new Menu(this);
}

Gui::~Gui() {
	_screen.free();
	_console.free();
	g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
	delete _menu;
}

void Gui::undrawCursor() {
	_cursorOff = true;
	_cursorState = false;
	cursorTimerHandler(this);
	_cursorOff = false;
}

const Graphics::Font *Gui::getFont(const char *name, Graphics::FontManager::FontUsage fallback) {
	const Graphics::Font *font;

	if (!_builtInFonts) {
		font = FontMan.getFontByName(name);

		if (!font)
			warning("Cannot load font %s", name);
	}

	if (_builtInFonts || !font)
		font = FontMan.getFontByUsage(fallback);

	return font;
}

const Graphics::Font *Gui::getTitleFont() {
	return getFont("Chicago-12", Graphics::FontManager::kBigGUIFont);
}

void Gui::draw() {
	if (_scene != _engine->_world->_player->_currentScene || _sceneDirty) {
		_scene = _engine->_world->_player->_currentScene;

		// Draw desktop
		Common::Rect r(0, 0, _screen.w - 1, _screen.h - 1);
		Design::drawFilledRoundRect(&_screen, r, kDesktopArc, kColorBlack, _patterns, kPatternCheckers);
		g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, _screen.w, _screen.h);

		_sceneDirty = true;
		_consoleDirty = true;
		_menuDirty = true;
		_consoleFullRedraw = true;

		_scene->paint(&_screen, _scene->_designBounds->left, _scene->_designBounds->top);

		_sceneArea.left = _scene->_designBounds->left + kBorderWidth - 2;
		_sceneArea.top = _scene->_designBounds->top + kBorderWidth - 2;
		_sceneArea.setWidth(_scene->_designBounds->width() - 2 * kBorderWidth);
		_sceneArea.setHeight(_scene->_designBounds->height() - 2 * kBorderWidth);

		_consoleTextArea.left = _scene->_textBounds->left + kBorderWidth - 2;
		_consoleTextArea.top = _scene->_textBounds->top + kBorderWidth - 2;
		_consoleTextArea.setWidth(_scene->_textBounds->width() - 2 * kBorderWidth);
		_consoleTextArea.setHeight(_scene->_textBounds->height() - 2 * kBorderWidth);
	}

	if (_scene && (_bordersDirty || _sceneDirty))
		paintBorder(&_screen, _sceneArea, kWindowScene);

	// Render console
	if (_consoleDirty || _consoleFullRedraw)
		renderConsole(&_screen, _consoleTextArea);

	if (_bordersDirty || _consoleDirty || _consoleFullRedraw)
		paintBorder(&_screen, _consoleTextArea, kWindowConsole);

	if (_menuDirty)
		_menu->render();

	_sceneDirty = false;
	_consoleDirty = false;
	_bordersDirty = false;
	_menuDirty = false;
	_consoleFullRedraw = false;
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

void Gui::paintBorder(Graphics::Surface *g, Common::Rect &r, WindowType windowType) {
	bool active, scrollable, closeable, closeBoxPressed, drawTitle;
	const int size = kBorderWidth;
	int x = r.left - size;
	int y = r.top - size;
	int width = r.width() + 2 * size;
	int height = r.height() + 2 * size;

	switch (windowType) {
	case kWindowScene:
		active = _sceneIsActive;
		scrollable = false;
		closeable = _sceneIsActive;
		closeBoxPressed = false;
		drawTitle = true;
		break;
	case kWindowConsole:
		active = !_sceneIsActive;
		scrollable = true;
		closeable = !_sceneIsActive;
		closeBoxPressed = false;
		drawTitle = false;
		break;
	}

	drawBox(g, x,                    y,                     size,                 size);
	drawBox(g, x + width - size - 1, y,                     size,                 size);
	drawBox(g, x + width - size - 1, y + height - size - 1, size,                 size);
	drawBox(g, x,                    y + height - size - 1, size,                 size);
	drawBox(g, x + size,             y + 2,                 width - 2 * size - 1, size - 4);
	drawBox(g, x + size,             y + height - size + 1, width - 2 * size - 1, size - 4);
	drawBox(g, x + 2,                y + size,              size - 4,             height - 2 * size - 1);
	drawBox(g, x + width - size + 1, y + size,              size - 4,             height - 2 * size - 1);

	if (active) {
		fillRect(g, x + size, y + 5,           width - 2 * size - 1, 8);
		fillRect(g, x + size, y + height - 13, width - 2 * size - 1, 8);
		fillRect(g, x + 5,    y + size,        8,                    height - 2 * size - 1);
		if (!scrollable) {
			fillRect(g, x + width - 13, y + size, 8, height - 2 * size - 1);
		} else {
			int x1 = x + width - 15;
			int y1 = y + size + 1;
			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++) {
					if (arrowPixels[yy][xx] != 0) {
						g->hLine(x1 + xx, y1 + yy, x1 + xx, kColorBlack);
					} else {
						g->hLine(x1 + xx, y1 + yy, x1 + xx, kColorWhite);
					}
				}
			}
			fillRect(g, x + width - 13, y + size + ARROW_H, 8, height - 2 * size - 1 - ARROW_H * 2);
			y1 += height - 2 * size - ARROW_H - 2;
			for (int yy = 0; yy < ARROW_H; yy++) {
				for (int xx = 0; xx < ARROW_W; xx++) {
					if (arrowPixels[ARROW_H - yy - 1][xx] != 0) {
						g->hLine(x1 + xx, y1 + yy, x1 + xx, kColorBlack);
					} else {
						g->hLine(x1 + xx, y1 + yy, x1 + xx, kColorWhite);
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
		const Graphics::Font *font = getTitleFont();
		int yOff = _builtInFonts ? 3 : 1;

		int w = font->getStringWidth(_scene->_name) + 10;
		int maxWidth = width - size * 2 - 7;
		if (w > maxWidth)
			w = maxWidth;
		drawBox(g, x + (width - w) / 2, y, w, size);
		font->drawString(g, _scene->_name, x + (width - w) / 2 + 5, y + yOff, w, kColorBlack);
	}

	if (x + width > _screen.w)
		width = _screen.w - x;
	if (y + height > _screen.h)
		height = _screen.h - y;

	g_system->copyRectToScreen(g->getBasePtr(x, y), g->pitch, x, y, width, height);
}

void Gui::loadFonts() {
	Common::Archive *dat;

	dat = Common::makeZipArchive("wage.dat");

	if (!dat) {
		warning("Could not find wage.dat. Falling back to built-in fonts");
		_builtInFonts = true;

		return;
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

void Gui::regenCommandsMenu() {
	_menu->regenCommandsMenu();
}

void Gui::regenWeaponsMenu() {
	_menu->regenWeaponsMenu();
}

void Gui::processMenuShortCut(byte flags, uint16 ascii) {
	_menu->processMenuShortCut(flags, ascii);
}

void Gui::mouseMove(int x, int y) {
	if (_menu->_menuActivated) {
		if (_menu->mouseMove(x, y))
			_menuDirty = true;

		return;
	}

	if (_inTextSelection) {
		updateTextSelection(x, y);
		return;
	}

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

void Gui::pushArrowCursor() {
	CursorMan.pushCursor(macCursorArrow, 11, 16, 1, 1, 3);
}

void Gui::popCursor() {
	CursorMan.popCursor();
}

Designed *Gui::mouseUp(int x, int y) {
	if (_menu->_menuActivated) {
		if (_menu->mouseRelease(x, y)) {
			_sceneDirty = true;
			_consoleDirty = true;
			_bordersDirty = true;
			_menuDirty = true;
		}

		return NULL;
	}

	if (_inTextSelection) {
		_inTextSelection = false;

		if (_selectionEndY == -1 ||
				(_selectionEndX == _selectionStartX && _selectionEndY == _selectionStartY)) {
			_selectionStartY = _selectionEndY = -1;
			_consoleFullRedraw = true;
			_menu->enableCommand(kMenuEdit, kMenuActionCopy, false);
		} else {
			_menu->enableCommand(kMenuEdit, kMenuActionCopy, true);

			bool cutAllowed = false;

			if (_selectionStartY == _selectionEndY && _selectionStartY == _lines.size() - 1)
				cutAllowed = true;

			_menu->enableCommand(kMenuEdit, kMenuActionCut, cutAllowed);
			_menu->enableCommand(kMenuEdit, kMenuActionClear, cutAllowed);
		}
	}

	if (_sceneArea.contains(x, y)) {
		if (!_sceneIsActive) {
			_sceneIsActive = true;
			_bordersDirty = true;
		}

		for (ObjList::const_iterator it = _scene->_objs.begin(); it != _scene->_objs.end(); ++it) {
			if ((*it)->_design->isPointOpaque(x - _sceneArea.left + kBorderWidth, y - _sceneArea.top + kBorderWidth))
				return *it;
		}

		for (ChrList::const_iterator it = _scene->_chrs.begin(); it != _scene->_chrs.end(); ++it) {
			if ((*it)->_design->isPointOpaque(x - _sceneArea.left + kBorderWidth, y - _sceneArea.top + kBorderWidth))
				return *it;
		}
	} else if (_consoleTextArea.contains(x, y)) {
		if (_sceneIsActive) {
			_sceneIsActive = false;
			_bordersDirty = true;
		}
	}

	return NULL;
}

void Gui::mouseDown(int x, int y) {
	if (_menu->mouseClick(x, y)) {
		_menuDirty = true;
	} else if (_consoleTextArea.contains(x, y)) {
		startMarking(x, y);
	}
}

int Gui::calcTextX(int x, int textLine) {
	const Graphics::Font *font = getConsoleFont();

	if (textLine >= _lines.size())
		return 0;

	Common::String str = _lines[textLine];

	x -= _consoleTextArea.left;

	for (int i = str.size(); i >= 0; i--) {
		if (font->getStringWidth(str) < x) {
			return i;
		}

		str.deleteLastChar();
	}

	return 0;
}

int Gui::calcTextY(int y) {
	y -= _consoleTextArea.top;

	if (y < 0)
		y = 0;

	const int firstLine = _scrollPos / _consoleLineHeight;
	int textLine = (y - _scrollPos % _consoleLineHeight) / _consoleLineHeight + firstLine;

	return textLine;
}

void Gui::startMarking(int x, int y) {
	_selectionStartY = calcTextY(y);
	_selectionStartX = calcTextX(x, _selectionStartY);

	_selectionEndY = -1;

	_inTextSelection = true;
}

void Gui::updateTextSelection(int x, int y) {
	_selectionEndY = calcTextY(y);
	_selectionEndX = calcTextX(x, _selectionEndY);

	_consoleFullRedraw = true;
}

} // End of namespace Wage
