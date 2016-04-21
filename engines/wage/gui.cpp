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
#include "graphics/primitives.h"

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/entities.h"
#include "wage/gui.h"
#include "wage/macwindow.h"
#include "wage/macwindowmanager.h"
#include "wage/menu.h"
#include "wage/world.h"

namespace Wage {

static const byte palette[] = {
	0, 0, 0,           // Black
	0x80, 0x80, 0x80,  // Gray
	0xff, 0xff, 0xff,  // White
	0x00, 0xff, 0x00,  // Green
	0x00, 0xcf, 0x00   // Green2
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

	gui->_cursorRect.left = x;
	gui->_cursorRect.right = MIN<uint16>(x + 1, gui->_screen.w);
	gui->_cursorRect.top = y;
	gui->_cursorRect.bottom = MIN<uint16>(y + kCursorHeight, gui->_screen.h);

	gui->_cursorDirty = true;
}

static bool sceneWindowCallback(WindowClick click, Common::Event &event, void *gui);
static bool consoleWindowCallback(WindowClick click, Common::Event &event, void *gui);

Gui::Gui(WageEngine *engine) {
	_engine = engine;
	_scene = NULL;
	_sceneDirty = true;
	_consoleDirty = true;
	_bordersDirty = true;
	_menuDirty = true;
	_cursorDirty = false;
	_consoleFullRedraw = true;
	_screen.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());

	_wm.setScreen(&_screen);

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

	g_system->getPaletteManager()->setPalette(palette, 0, ARRAYSIZE(palette) / 3);

	CursorMan.replaceCursorPalette(palette, 0, 4);
	CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
	_cursorIsArrow = true;
	CursorMan.showMouse(true);

	for (int i = 0; i < ARRAYSIZE(fillPatterns); i++)
		_patterns.push_back(fillPatterns[i]);

	loadFonts();

	g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "wageCursor");

	_menu = new Menu(this);

	_sceneWindow = _wm.add(false);
	_sceneWindow->setCallback(sceneWindowCallback, this);

	_consoleWindow = _wm.add(true);
	_consoleWindow->setCallback(consoleWindowCallback, this);
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
	const Graphics::Font *font = 0;

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

void Gui::drawDesktop() {
	// Draw desktop
	Common::Rect r(0, 0, _screen.w - 1, _screen.h - 1);
	Design::drawFilledRoundRect(&_screen, r, kDesktopArc, kColorBlack, _patterns, kPatternCheckers);
	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, _screen.w, _screen.h);
}

void Gui::draw() {
	if (_engine->_isGameOver) {
		if (_menuDirty) {
			drawDesktop();
			_menu->render();
		}

		_menuDirty = false;

		return;
	}

	if (!_engine->_world->_player->_currentScene)
		return;

	if (_scene != _engine->_world->_player->_currentScene) {
		_sceneDirty = true;

		_scene = _engine->_world->_player->_currentScene;

		_sceneWindow->setDimensions(*_scene->_designBounds);
		_sceneWindow->setTitle(_scene->_name);
		_consoleWindow->setDimensions(*_scene->_textBounds);
	}

	if (_sceneDirty || _bordersDirty) {
		drawDesktop();
		_wm.setFullRefresh(true);
	}

	drawScene();
	drawConsole();

	_wm.draw();

	if (_menuDirty)
		_menu->render();

	if (_cursorDirty) {
		g_system->copyRectToScreen(_screen.getBasePtr(_cursorRect.left, _cursorRect.top), _screen.pitch,
				_cursorRect.left, _cursorRect.top, _cursorRect.width(), _cursorRect.height());

		_cursorDirty = false;
	}

	_sceneDirty = false;
	_consoleDirty = false;
	_bordersDirty = false;
	_menuDirty = false;
	_consoleFullRedraw = false;
}

void Gui::drawScene() {
	if (!_sceneDirty && !_bordersDirty)
		return;

	_scene->paint(_sceneWindow->getSurface(), 0, 0);
	_sceneWindow->setDirty(true);

	_sceneDirty = true;
	_consoleDirty = true;
	_menuDirty = true;
	_consoleFullRedraw = true;

	_sceneArea.left = _scene->_designBounds->left + kBorderWidth - 2;
	_sceneArea.top = _scene->_designBounds->top + kBorderWidth - 2;
	_sceneArea.setWidth(_scene->_designBounds->width() - 2 * kBorderWidth);
	_sceneArea.setHeight(_scene->_designBounds->height() - 2 * kBorderWidth);

	_consoleTextArea.left = _scene->_textBounds->left + kBorderWidth - 2;
	_consoleTextArea.top = _scene->_textBounds->top + kBorderWidth - 2;
	_consoleTextArea.setWidth(_scene->_textBounds->width() - 2 * kBorderWidth);
	_consoleTextArea.setHeight(_scene->_textBounds->height() - 2 * kBorderWidth);
}

static bool sceneWindowCallback(WindowClick click, Common::Event &event, void *g) {
	Gui *gui = (Gui *)g;

	return gui->processSceneEvents(click, event);
}

bool Gui::processSceneEvents(WindowClick click, Common::Event &event) {
	if (_cursorIsArrow == false) {
		CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
		_cursorIsArrow = true;
	}

	if (click == kBorderInner && event.type == Common::EVENT_LBUTTONUP) {
		Designed *obj = _scene->lookUpEntity(event.mouse.x - _sceneWindow->getDimensions().left,
												  event.mouse.y - _sceneWindow->getDimensions().top);

		if (obj != nullptr)
			_engine->processTurn(NULL, obj);

		return true;
	}

	return false;
}

// Render console
void Gui::drawConsole() {
	if (!_consoleDirty && !_consoleFullRedraw && !_bordersDirty && !_sceneDirty)
		return;

	renderConsole(_consoleWindow->getSurface(), Common::Rect(kBorderWidth - 2, kBorderWidth - 2,
				_scene->_textBounds->width() - kBorderWidth, _scene->_textBounds->height() - kBorderWidth));
	_consoleWindow->setDirty(true);
}

static bool consoleWindowCallback(WindowClick click, Common::Event &event, void *g) {
	Gui *gui = (Gui *)g;

	return gui->processConsoleEvents(click, event);
}

bool Gui::processConsoleEvents(WindowClick click, Common::Event &event) {
	if (click != kBorderInner && _cursorIsArrow == false) {
		CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
		_cursorIsArrow = true;
	}

	if (click == kBorderScrollUp || click == kBorderScrollDown) {
		if (event.type == Common::EVENT_LBUTTONDOWN) {
			int textFullSize = _lines.size() * _consoleLineHeight + _consoleTextArea.height();
			float scrollPos = (float)_scrollPos / textFullSize;
			float scrollSize = (float)_consoleTextArea.height() / textFullSize;

			_consoleWindow->setScroll(scrollPos, scrollSize);
		} else if (event.type == Common::EVENT_LBUTTONUP) {
			int oldScrollPos = _scrollPos;

			switch (click) {
			case kBorderScrollUp:
				_scrollPos = MAX<int>(0, _scrollPos - _consoleLineHeight);
				undrawCursor();
				_cursorY -= (_scrollPos - oldScrollPos);
				_consoleDirty = true;
				_consoleFullRedraw = true;
				break;
			case kBorderScrollDown:
				_scrollPos = MIN<int>((_lines.size() - 2) * _consoleLineHeight, _scrollPos + _consoleLineHeight);
				undrawCursor();
				_cursorY -= (_scrollPos - oldScrollPos);
				_consoleDirty = true;
				_consoleFullRedraw = true;
				break;
			default:
				break;
			}
		}

		return true;
	}

	if (click == kBorderInner) {
		if (event.type == Common::EVENT_LBUTTONDOWN) {
			startMarking(event.mouse.x, event.mouse.y);
		} else if (event.type == Common::EVENT_LBUTTONUP) {
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

					if (_selectionStartY == _selectionEndY && _selectionStartY == (int)_lines.size() - 1)
						cutAllowed = true;

					_menu->enableCommand(kMenuEdit, kMenuActionCut, cutAllowed);
					_menu->enableCommand(kMenuEdit, kMenuActionClear, cutAllowed);
				}
			}
		} else if (event.type == Common::EVENT_MOUSEMOVE) {
			if (_inTextSelection) {
				updateTextSelection(event.mouse.x, event.mouse.y);
				return true;
			}

			if (_cursorIsArrow) {
				CursorMan.replaceCursor(macCursorBeam, 11, 16, 3, 8, 3);
				_cursorIsArrow = false;
			}
		}

		return true;
	}

	return false;
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
}

void Gui::pushArrowCursor() {
	CursorMan.pushCursor(macCursorArrow, 11, 16, 1, 1, 3);
}

void Gui::popCursor() {
	CursorMan.popCursor();
}

bool Gui::processEvent(Common::Event &event) {
	if (_wm.processEvent(event))
		return true;

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		mouseMove(event.mouse.x, event.mouse.y);
		break;
	case Common::EVENT_LBUTTONDOWN:
		mouseDown(event.mouse.x, event.mouse.y);
		break;
	case Common::EVENT_LBUTTONUP:
		mouseUp(event.mouse.x, event.mouse.y);
		break;

	default:
		return false;
	}

	return true;
}

void Gui::mouseUp(int x, int y) {
	if (_menu->_menuActivated) {
		if (_menu->mouseRelease(x, y)) {
			_sceneDirty = true;
			_consoleDirty = true;
			_bordersDirty = true;
			_menuDirty = true;
		}

		return;
	}

	return;
}

void Gui::mouseDown(int x, int y) {
	if (_menu->mouseClick(x, y)) {
		_menuDirty = true;
	}
}

int Gui::calcTextX(int x, int textLine) {
	const Graphics::Font *font = getConsoleFont();

	if ((uint)textLine >= _lines.size())
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
