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
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/primitives.h"

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/entities.h"
#include "wage/gui.h"
#include "wage/macwindow.h"
#include "wage/macwindowmanager.h"
#include "wage/macmenu.h"
#include "wage/world.h"

namespace Wage {

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

	x += gui->_consoleWindow->getInnerDimensions().left;
	y += gui->_consoleWindow->getInnerDimensions().top;

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
	_cursorDirty = false;
	_consoleFullRedraw = true;
	_screen.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());

	_wm.setScreen(&_screen);

	_scrollPos = 0;
	_consoleLineHeight = 8; // Dummy value which makes sense
	_consoleNumLines = 24; // Dummy value

	_cursorX = 0;
	_cursorY = 0;
	_cursorState = false;
	_cursorOff = false;

	_inTextSelection = false;
	_selectionStartX = _selectionStartY = -1;
	_selectionEndX = _selectionEndY = -1;

	_inputTextLineNum = 0;

	CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
	_cursorIsArrow = true;
	CursorMan.showMouse(true);

	g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "wageCursor");

	_menu = _wm.addMenu(this);

	_sceneWindow = _wm.addWindow(false, false);
	_sceneWindow->setCallback(sceneWindowCallback, this);

	_consoleWindow = _wm.addWindow(true, true);
	_consoleWindow->setCallback(consoleWindowCallback, this);
}

Gui::~Gui() {
	_screen.free();
	_console.free();
	g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
}

void Gui::undrawCursor() {
	_cursorOff = true;
	_cursorState = false;
	cursorTimerHandler(this);
	_cursorOff = false;
}

void Gui::draw() {
	if (_engine->_isGameOver) {
		_wm.draw();

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

		_wm.setFullRefresh(true);
	}

	drawScene();
	drawConsole();

	_wm.draw();

	if (_cursorDirty && _cursorRect.left < _screen.w && _cursorRect.bottom < _screen.h) {
		g_system->copyRectToScreen(_screen.getBasePtr(_cursorRect.left, _cursorRect.top), _screen.pitch,
				_cursorRect.left, _cursorRect.top, _cursorRect.width(), _cursorRect.height());

		_cursorDirty = false;
	}

	_sceneDirty = false;
	_consoleDirty = false;
	_consoleFullRedraw = false;
}

void Gui::drawScene() {
	if (!_sceneDirty)
		return;

	_scene->paint(_sceneWindow->getSurface(), 0, 0);
	_sceneWindow->setDirty(true);

	_sceneDirty = true;
	_consoleDirty = true;
	_menu->setDirty(true);
	_consoleFullRedraw = true;
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
	if (!_consoleDirty && !_consoleFullRedraw && !_sceneDirty)
		return;

	renderConsole(_consoleWindow->getSurface(), Common::Rect(kBorderWidth - 2, kBorderWidth - 2,
				_consoleWindow->getDimensions().width(), _consoleWindow->getDimensions().height()));
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
			int consoleHeight = _consoleWindow->getInnerDimensions().height();
			int textFullSize = _lines.size() * _consoleLineHeight + consoleHeight;
			float scrollPos = (float)_scrollPos / textFullSize;
			float scrollSize = (float)consoleHeight / textFullSize;

			_consoleWindow->setScroll(scrollPos, scrollSize);

			return true;
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
				return false;
			}

			return true;
		}

		return false;
	}

	if (click == kBorderResizeButton) {
		_consoleDirty = true;
		_consoleFullRedraw = true;

		return true;
	}

	if (click == kBorderInner) {
		if (event.type == Common::EVENT_LBUTTONDOWN) {
			startMarking(event.mouse.x, event.mouse.y);

			return true;
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

			return true;
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

		return false;
	}

	return false;
}

void Gui::regenCommandsMenu() {
	_menu->regenCommandsMenu();
}

void Gui::regenWeaponsMenu() {
	_menu->regenWeaponsMenu();
}

void Gui::pushArrowCursor() {
	CursorMan.pushCursor(macCursorArrow, 11, 16, 1, 1, 3);
}

void Gui::popCursor() {
	CursorMan.popCursor();
}

bool Gui::processEvent(Common::Event &event) {
	return _wm.processEvent(event);
}

int Gui::calcTextX(int x, int textLine) {
	const Graphics::Font *font = getConsoleFont();

	if ((uint)textLine >= _lines.size())
		return 0;

	Common::String str = _lines[textLine];

	x -= _consoleWindow->getInnerDimensions().left;

	for (int i = str.size(); i >= 0; i--) {
		if (font->getStringWidth(str) < x) {
			return i;
		}

		str.deleteLastChar();
	}

	return 0;
}

int Gui::calcTextY(int y) {
	y -= _consoleWindow->getInnerDimensions().top;

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
