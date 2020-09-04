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
#include "graphics/macgui/macfontmanager.h"
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

static bool sceneWindowCallback(WindowClick click, Common::Event &event, void *gui);
static void menuCommandsCallback(int action, Common::String &text, void *data);


Gui::Gui(WageEngine *engine) {
	_engine = engine;
	_scene = NULL;
	_sceneDirty = true;
	_screen.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());

	_wm.setScreen(&_screen);

	_menu = _wm.addMenu();

	_menu->setCommandsCallback(menuCommandsCallback, this);

	_menu->addStaticMenus(menuSubItems);
	_menu->addSubMenu(nullptr, kMenuAbout);
	_menu->addMenuItem(_menu->getSubmenu(nullptr, kMenuAbout), _engine->_world->getAboutMenuItemName(), kMenuActionAbout);

	_commandsMenuId = _menu->addMenuItem(nullptr, _engine->_world->_commandsMenuName);
	regenCommandsMenu();

	if (!_engine->_world->_weaponMenuDisabled) {
		_weaponsMenuId = _menu->addMenuItem(nullptr, _engine->_world->_weaponsMenuName);

		regenWeaponsMenu();
	} else {
		_weaponsMenuId = -1;
	}

	_menu->calcDimensions();

	if (g_system->hasTextInClipboard()) {
		_menu->enableCommand(kMenuEdit, kMenuActionPaste, true);
	}

	_sceneWindow = _wm.addWindow(false, false, false);
	_sceneWindow->setCallback(sceneWindowCallback, this);

	//TODO: Make the font we use here work
	// (currently MacFontRun::getFont gets called with the fonts being uninitialized,
	// so it initializes them by itself with default params, and not those here)
	const Graphics::MacFont *font = new Graphics::MacFont(Graphics::kMacFontChicago, 8);

	uint maxWidth = _screen.w;

	_consoleWindow = _wm.addTextWindow(font, kColorBlack, kColorWhite, maxWidth, Graphics::kTextAlignLeft, _menu);

	loadBorders();
}

Gui::~Gui() {
	_screen.free();
	_console.free();
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

	_wm.draw();

	_sceneDirty = false;
}

void Gui::drawScene() {
	if (!_sceneDirty)
		return;

	_scene->paint(_sceneWindow->getWindowSurface(), 0, 0);
	_sceneWindow->setDirty(true);

	_sceneDirty = true;
	_menu->setDirty(true);
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

////////////////
// Menu stuff
////////////////
void Gui::regenCommandsMenu() {
	_menu->createSubMenuFromString(_commandsMenuId, _engine->_world->_commandsMenu.c_str(), kMenuActionCommand);
}

void Gui::regenWeaponsMenu() {
	if (_engine->_world->_weaponMenuDisabled)
		return;

	_menu->clearSubMenu(_weaponsMenuId);

	Chr *player = _engine->_world->_player;
	if (!player) {
		warning("regenWeaponsMenu: player is not defined");
		return;
	}
	ObjArray *weapons = player->getWeapons(true);

	bool empty = true;

	Graphics::MacMenuSubMenu *submenu = _menu->getSubmenu(nullptr, _weaponsMenuId);
	if (submenu == nullptr)
		submenu = _menu->addSubMenu(nullptr, _weaponsMenuId);

	for (uint i = 0; i < weapons->size(); i++) {
		Obj *obj = (*weapons)[i];
		if (obj->_type == Obj::REGULAR_WEAPON ||
			obj->_type == Obj::THROW_WEAPON ||
			obj->_type == Obj::MAGICAL_OBJECT) {
			Common::String command(obj->_operativeVerb);
			command += " ";
			command += obj->_name;

			_menu->addMenuItem(submenu, command, kMenuActionCommand, 0, 0, true);

			empty = false;
		}
	}
	delete weapons;

	if (empty)
		_menu->addMenuItem(submenu, "You have no weapons", 0, 0, 0, false);
}

bool Gui::processEvent(Common::Event &event) {
	if (event.type == Common::EVENT_CLIPBOARD_UPDATE) {
		_menu->enableCommand(kMenuEdit, kMenuActionPaste, true);
	}

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
		warning("STUB: executeMenuCommand: action: %d", action);
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

	case kMenuActionCommand: {
			_engine->_inputText = text;
			Common::String inp = text + '\n';

			appendText(inp.c_str());

			_consoleWindow->clearInput();

			_engine->processTurn(&text, NULL);
			break;
		}
	default:
		warning("Unknown action: %d", action);

	}
}

void Gui::loadBorders() {
	loadBorder(_sceneWindow, "wage_border_inact.bmp", false);
	loadBorder(_sceneWindow, "wage_border_act-noscrollbar.bmp", true);
	loadBorder(_consoleWindow, "wage_border_inact.bmp", false);
	loadBorder(_consoleWindow, "wage_border_act.bmp", true);
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

		target->loadBorder(*stream, active);

		borderfile.close();

		delete stream;
	}
}

//////////////////
// Console stuff
//////////////////
const Graphics::MacFont *Gui::getConsoleMacFont() {
	Scene *scene = _engine->_world->_player->_currentScene;

	return scene->getFont();
}

const Graphics::Font *Gui::getConsoleFont() {
	return _wm._fontMan->getFont(*getConsoleMacFont());
}

void Gui::appendText(const char *s) {
	_consoleWindow->appendText(s, getConsoleMacFont());
}

void Gui::clearOutput() {
	_consoleWindow->clearText();
}

void Gui::actionCopy() {
	g_system->setTextInClipboard(Common::convertUtf32ToUtf8(_consoleWindow->getSelection()));

	_menu->enableCommand(kMenuEdit, kMenuActionPaste, true);
}

void Gui::actionPaste() {
	if (g_system->hasTextInClipboard()) {
		_undobuffer = _engine->_inputText;

		_consoleWindow->appendInput(g_system->getTextFromClipboard());

		_menu->enableCommand(kMenuEdit, kMenuActionUndo, true);
	}
}

void Gui::actionUndo() {
	_consoleWindow->clearInput();
	_consoleWindow->appendInput(_undobuffer);

	_menu->enableCommand(kMenuEdit, kMenuActionUndo, false);
}

void Gui::actionClear() {
	if (_consoleWindow->getSelectedText()->endY == -1)
		return;

	Common::String input = Common::convertFromU32String(_consoleWindow->getInput());

	_consoleWindow->cutSelection();

	_undobuffer = input;

	_menu->enableCommand(kMenuEdit, kMenuActionUndo, true);
}

void Gui::actionCut() {
	if (_consoleWindow->getSelectedText()->endY == -1)
		return;

	Common::String input = Common::convertFromU32String(_consoleWindow->getInput());

	g_system->setTextInClipboard(_consoleWindow->cutSelection());

	_undobuffer = input;

	_menu->enableCommand(kMenuEdit, kMenuActionUndo, true);
	_menu->enableCommand(kMenuEdit, kMenuActionPaste, true);
}

void Gui::disableUndo() {
	_menu->enableCommand(kMenuEdit, kMenuActionUndo, false);
}

void Gui::disableAllMenus() {
	_menu->disableAllMenus();
}

void Gui::enableNewGameMenus() {
	_menu->enableCommand(kMenuFile, kMenuActionNew, true);
	_menu->enableCommand(kMenuFile, kMenuActionOpen, true);
	_menu->enableCommand(kMenuFile, kMenuActionQuit, true);
}

} // End of namespace Wage
