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
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macmenu.h"

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/entities.h"
#include "wage/gui.h"
#include "wage/world.h"

namespace Wage {

static const Graphics::MacMenuData menuSubItems[] = {
	{ kMenuHighLevel, "File",	0, 0, false },
	{ kMenuHighLevel, "Edit",	0, 0, false },
	{ kMenuFile, "New",			kMenuActionNew, 0, false },
	{ kMenuFile, "Open...",		kMenuActionOpen, 0, true },
	{ kMenuFile, "Close",		kMenuActionClose, 0, true },
	{ kMenuFile, "Save",		kMenuActionSave, 0, true },
	{ kMenuFile, "Save as...",	kMenuActionSaveAs, 0, true },
	{ kMenuFile, "Revert",		kMenuActionRevert, 0, false },
	{ kMenuFile, "Quit",		kMenuActionQuit, 0, true },

	{ kMenuEdit, "Undo",		kMenuActionUndo, 'Z', false },
	{ kMenuEdit, NULL,			0, 0, false },
	{ kMenuEdit, "Cut",			kMenuActionCut, 'K', false },
	{ kMenuEdit, "Copy",		kMenuActionCopy, 'C', false },
	{ kMenuEdit, "Paste",		kMenuActionPaste, 'V', false },
	{ kMenuEdit, "Clear",		kMenuActionClear, 'B', false },

	{ 0, NULL,			0, 0, false }
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
	int h = kCursorHeight;

	if (y + h > gui->_consoleWindow->getInnerDimensions().bottom) {
		h = gui->_consoleWindow->getInnerDimensions().bottom - y;
	}

	if (h > 0)
		gui->_screen.vLine(x, y, y + h, gui->_cursorState ? kColorBlack : kColorWhite);

	if (!gui->_cursorOff)
		gui->_cursorState = !gui->_cursorState;

	gui->_cursorRect.left = x;
	gui->_cursorRect.right = MIN<uint16>(x + 1, gui->_screen.w);
	gui->_cursorRect.top = MIN<uint16>(y - 1, gui->_consoleWindow->getInnerDimensions().top);
	gui->_cursorRect.bottom = MIN<uint16>(MIN<uint16>(y + h, gui->_screen.h), gui->_consoleWindow->getInnerDimensions().bottom);

	gui->_cursorDirty = true;
}

static bool sceneWindowCallback(WindowClick click, Common::Event &event, void *gui);
static bool consoleWindowCallback(WindowClick click, Common::Event &event, void *gui);
static void menuCommandsCallback(int action, Common::String &text, void *data);


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

	g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "wageCursor");

	_menu = _wm.addMenu();

	_menu->setCommandsCallback(menuCommandsCallback, this);

	_menu->addStaticMenus(menuSubItems);
	_menu->addMenuSubItem(kMenuAbout, _engine->_world->getAboutMenuItemName(), kMenuActionAbout);

	_commandsMenuId = _menu->addMenuItem(_engine->_world->_commandsMenuName.c_str());
	regenCommandsMenu();

	if (!_engine->_world->_weaponMenuDisabled) {
		_weaponsMenuId = _menu->addMenuItem(_engine->_world->_weaponsMenuName.c_str());

		regenWeaponsMenu();
	} else {
		_weaponsMenuId = -1;
	}

	_menu->calcDimensions();

	_sceneWindow = _wm.addWindow(false, false, false);
	_sceneWindow->setCallback(sceneWindowCallback, this);

	_consoleWindow = _wm.addWindow(true, true, true);
	_consoleWindow->setCallback(consoleWindowCallback, this);

	loadBorders();

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
		int x = _cursorRect.left, y = _cursorRect.top, w = _cursorRect.width(), h = _cursorRect.height();
		if (x < 0) {
			w += x;
			x = 0;
		}
		if (y < 0) {
			h += y;
			y = 0;
		}
		if (x + w > _screen.w) w = _screen.w - x;
		if (y + h > _screen.h) h = _screen.h - y;
		if (w != 0 && h != 0)
			g_system->copyRectToScreen(_screen.getBasePtr(x, y), _screen.pitch, x, y, w, h);

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

////////////////
// Menu stuff
////////////////
void Gui::regenCommandsMenu() {
	_menu->createSubMenuFromString(_commandsMenuId, _engine->_world->_commandsMenu.c_str());
}

void Gui::regenWeaponsMenu() {
	if (_engine->_world->_weaponMenuDisabled)
		return;

	_menu->clearSubMenu(_weaponsMenuId);

	Chr *player = _engine->_world->_player;
	ObjArray *weapons = player->getWeapons(true);

	bool empty = true;

	for (uint i = 0; i < weapons->size(); i++) {
		Obj *obj = (*weapons)[i];
		if (obj->_type == Obj::REGULAR_WEAPON ||
			obj->_type == Obj::THROW_WEAPON ||
			obj->_type == Obj::MAGICAL_OBJECT) {
			Common::String command(obj->_operativeVerb);
			command += " ";
			command += obj->_name;

			_menu->addMenuSubItem(_weaponsMenuId, command.c_str(), kMenuActionCommand, 0, 0, true);

			empty = false;
		}
	}
	delete weapons;

	if (empty)
		_menu->addMenuSubItem(_weaponsMenuId, "You have no weapons", 0, 0, 0, false);
}

bool Gui::processEvent(Common::Event &event) {
	return _wm.processEvent(event);
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	Gui *g = (Gui *)data;

	g->executeMenuCommand(action, text);
}

void Gui::executeMenuCommand(int action, Common::String &text) {
	switch(action) {
	case kMenuActionAbout:
	case kMenuActionNew:
	case kMenuActionClose:
	case kMenuActionRevert:
	case kMenuActionQuit:
		break;

	case kMenuActionOpen:
		_engine->scummVMSaveLoadDialog(false);
		break;

	case kMenuActionSave:
	case kMenuActionSaveAs:
		_engine->scummVMSaveLoadDialog(true);
		break;

	case kMenuActionUndo:
		actionUndo();
		break;
	case kMenuActionCut:
		actionCut();
		break;
	case kMenuActionCopy:
		actionCopy();
		break;
	case kMenuActionPaste:
		actionPaste();
		break;
	case kMenuActionClear:
		actionClear();
		break;

	case kMenuActionCommand:
		_engine->processTurn(&text, NULL);
		break;

	default:
		warning("Unknown action: %d", action);

	}
}

void Gui::loadBorders() {
	// Do not load borders for now
	//loadBorder(_sceneWindow, "border_inac.bmp", false);
	//loadBorder(_sceneWindow, "border_act.bmp", true);
}

void Gui::loadBorder(Graphics::MacWindow *target, Common::String filename, bool active) {
	Common::File borderfile;

	if (!borderfile.open(filename)) {
		debug(1, "Cannot open border file");
		return;
	}

	Image::BitmapDecoder bmpDecoder;
	Common::SeekableReadStream *stream = borderfile.readStream(borderfile.size());
	if (stream) {

		target->loadBorder(*stream, active, 10, 10, 1, 1);

		borderfile.close();

		delete stream;
	}
}

} // End of namespace Wage
