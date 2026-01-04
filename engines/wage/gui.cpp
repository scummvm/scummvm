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
#include "common/config-manager.h"

#include "audio/softsynth/pcspk.h"

#include "graphics/primitives.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macdialog.h"
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
	{ kMenuFile, "New",			kMenuActionNew, 0, true },
	{ kMenuFile, "Open...",		kMenuActionOpen, 0, true },
	{ kMenuFile, "Close",		kMenuActionClose, 0, false },
	{ kMenuFile, "Save",		kMenuActionSave, 0, false },
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

static bool consoleWindowCallback(WindowClick click, Common::Event &event, void *gui);
static bool sceneWindowCallback(WindowClick click, Common::Event &event, void *gui);
static void menuCommandsCallback(int action, Common::String &text, void *data);


Gui::Gui(WageEngine *engine) {
	_engine = engine;
	_scene = NULL;
	_sceneDirty = true;
	_screen.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());

	_wm = new Graphics::MacWindowManager(Graphics::kWMNoScummVMWallpaper);
	_wm->_fontMan->loadFonts(Common::Path(engine->getGameFile()));
	_wm->setScreen(&_screen);

	_menu = _wm->addMenu();

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

	_sceneWindow = _wm->addWindow(false, false, false);
	_sceneWindow->setCallback(sceneWindowCallback, this);

	//TODO: Make the font we use here work
	// (currently MacFontRun::getFont gets called with the fonts being uninitialized,
	// so it initializes them by itself with default params, and not those here)
	const Graphics::MacFont *font = new Graphics::MacFont(Graphics::kMacFontSystem, 8);

	uint maxWidth = _screen.w;

	_consoleWindow = _wm->addTextWindow(font, kColorBlack, kColorWhite, maxWidth, Graphics::kTextAlignLeft, _menu, 4);
	_consoleWindow->setCallback(consoleWindowCallback, this);
	_consoleWindow->setBorderColor(kColorWhite);
	_consoleWindow->setEditable(true);

	_selectedMenuItem = -1;

	loadBorders();
}

Gui::~Gui() {
	_screen.free();
	_console.free();
	delete _wm;
}

void Gui::draw() {
	if (_engine->_isGameOver) {
		_wm->draw();

		return;
	}

	if (!_engine->_world->_player->_currentScene)
		return;

	if (_scene != _engine->_world->_player->_currentScene) {
		_sceneDirty = true;

		_scene = _engine->_world->_player->_currentScene;

		Common::Rect sceneBounds = *_scene->_designBounds;
		const Graphics::BorderOffsets &offsets = _sceneWindow->getBorderOffsets();

		int maxTitleWidth = sceneBounds.width() - (kWindowMinWidth - offsets.right);
		Common::String displayTitle = _scene->_name;

		if (maxTitleWidth > 0) {
			const Graphics::Font *titleFont = getTitleFont();
			if (titleFont) {
				// keep deleting the last character untill the title fits
				while (displayTitle.size() > 0 && titleFont->getStringWidth(displayTitle) > maxTitleWidth) {
					displayTitle.deleteLastChar();
				}
			}
		} else {
			displayTitle.clear();
		}

		_sceneWindow->setTitle(displayTitle);
		_sceneWindow->setDimensions(sceneBounds);
		_consoleWindow->setDimensions(*_scene->_textBounds);

		_wm->setActiveWindow(_consoleWindow->getId());
		_wm->setFullRefresh(true);
	}

	drawScene();

	_wm->draw();

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

static bool consoleWindowCallback(WindowClick click, Common::Event &event, void *g) {
	Gui *gui = (Gui *)g;

	return gui->processConsoleEvents(click, event);
}

bool Gui::processConsoleEvents(WindowClick click, Common::Event &event) {
	if (click == kBorderCloseButton && event.type == Common::EVENT_LBUTTONUP) {
		_engine->quitGame();
		return true;
	}

	return false;
}

static bool sceneWindowCallback(WindowClick click, Common::Event &event, void *g) {
	Gui *gui = (Gui *)g;

	return gui->processSceneEvents(click, event);
}

bool Gui::processSceneEvents(WindowClick click, Common::Event &event) {
	if (click == kBorderInner && event.type == Common::EVENT_LBUTTONUP) {
		Designed *obj = _scene->lookUpEntity(event.mouse.x, event.mouse.y);

		if (obj != nullptr)
			_engine->processTurn(NULL, obj);

		return true;
	}
	if (click == kBorderCloseButton && event.type == Common::EVENT_LBUTTONUP) {
		_engine->quitGame();
		return true;
	}

	if (event.type == Common::EVENT_KEYDOWN) {
		return _consoleWindow->processEvent(event);
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

	if (event.type == Common::EVENT_MOUSEMOVE) {
		bool mouseOnItem = false;
		for (int i = 0; i < _menu->numberOfMenus(); i++) {
			Graphics::MacMenuItem *menuItem = _menu->getMenuItem(i);

			if (menuItem->enabled && menuItem->bbox.contains(event.mouse.x, event.mouse.y)) {
				if (_selectedMenuItem != i) {
					_engine->sayText(menuItem->text, Common::TextToSpeechManager::INTERRUPT);
					_selectedMenuItem = i;
				}

				mouseOnItem = true;
				break;
			}
		}

		if (!mouseOnItem) {
			_selectedMenuItem = -1;
		}
	}

	if (event.type == Common::EVENT_KEYDOWN) {
		_wm->setActiveWindow(_consoleWindow->getId());
	}

	return _wm->processEvent(event);
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	Gui *g = (Gui *)data;

	g->executeMenuCommand(action, text);
}

void Gui::executeMenuCommand(int action, Common::String &text) {
	switch(action) {
	case kMenuActionAbout:
		aboutDialog();
		break;

	case kMenuActionNew:
		_engine->_restartRequested = true;
		break;

	case kMenuActionClose:
		// This is a no-op as we do not let new game to be opened
		break;

	case kMenuActionRevert:
		if (_engine->_defaultSaveSlot != -1) {
			_engine->_isGameOver = false;

			_engine->_world->_weaponMenuDisabled = false;
			_engine->loadGameState(_engine->_defaultSaveSlot);

			_scene = nullptr; 	// To force current scene to be redrawn
			_engine->redrawScene();
			g_system->updateScreen();
		}
		break;

	case kMenuActionOpen:
		_engine->scummVMSaveLoadDialog(false);
		break;

	case kMenuActionQuit:
		_engine->quitGame();
		break;

	case kMenuActionSave:
		_engine->saveGame();
		break;

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

			_engine->sayText(text, Common::TextToSpeechManager::QUEUE);

			appendText(inp.c_str());

			_consoleWindow->clearInput();

			_engine->processTurn(&text, NULL);
			break;
		}
	default:
		warning("Unknown action: %d", action);

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
	return _wm->_fontMan->getFont(*getConsoleMacFont());
}

const Graphics::Font *Gui::getTitleFont() {
	return _wm->_fontMan->getFont(Graphics::MacFont(Graphics::kMacFontSystem, 12));
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

void Gui::enableSave() {
	_menu->enableCommand(kMenuFile, kMenuActionSave, true);
	_menu->enableCommand(kMenuFile, kMenuActionSaveAs, true);
}

void Gui::enableRevert() {
	_menu->enableCommand(kMenuFile, kMenuActionRevert, true);
}

class AboutDialog : public Graphics::MacDialog {
public:
	AboutDialog(Graphics::ManagedSurface *screen, Graphics::MacWindowManager *wm, int width, Graphics::MacText *mactext, int maxTextWidth, Graphics::MacDialogButtonArray *buttons, uint defaultButton);
	virtual ~AboutDialog() {
		if (_volumeChanged)
			ConfMan.flushToDisk();
	}

	virtual void paint() override;
	virtual bool processEvent(const Common::Event &event) override;

private:
	Common::Rect _volBbox;

	const int kVolWidth = 160;
	bool _volumeChanged = false;
};

AboutDialog::AboutDialog(Graphics::ManagedSurface *screen, Graphics::MacWindowManager *wm, int width, Graphics::MacText *mactext, int maxTextWidth, Graphics::MacDialogButtonArray *buttons, uint defaultButton)
		: Graphics::MacDialog(screen, wm, width, mactext, maxTextWidth, buttons, defaultButton) {
	_volBbox = Common::Rect(0, 0, kVolWidth, 12);
	_volBbox.moveTo(_bbox.left + (_bbox.width() - kVolWidth) / 2, _bbox.bottom - 32);
}


void AboutDialog::paint() {
	Graphics::MacDialog::paint();

	const char *volumeText = "-     Volume     +";
	int w = _font->getStringWidth(volumeText);
	int x = _bbox.left + (_bbox.width() - w) / 2;
	int y = _bbox.bottom - 52;

	_font->drawString(_screen, volumeText, x, y, _bbox.width(), kColorBlack);

	uint32 volume = ConfMan.getInt("sfx_volume");

	Graphics::Primitives &primitives = _wm->getDrawPrimitives();

	Common::Rect volBox(0, 0, volume * kVolWidth / 256, 12);
	volBox.moveTo(_bbox.left + (_bbox.width() - kVolWidth) / 2, _bbox.bottom - 32);

	Graphics::MacPlotData pd(_screen, nullptr, &_wm->getPatterns(), 1, 0, 0, 1, _wm->_colorBlack, false);
	primitives.drawFilledRect1(volBox, kColorBlack, &pd);
	primitives.drawRect1(_volBbox, kColorBlack, &pd);
}

bool AboutDialog::processEvent(const Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		if (_volBbox.contains(event.mouse.x, event.mouse.y)) {
			int delta = event.mouse.x - _volBbox.left;

			int volume = delta * 256 / kVolWidth;
			ConfMan.setInt("sfx_volume", volume);
			_volumeChanged = true;

			_needsRedraw = true;

			g_wage->syncSoundSettings();

			g_wage->_speaker->play(Audio::PCSpeaker::kWaveFormSquare, 500, 150);

			return true;
		}
	}

	return false;
}

void Gui::aboutDialog() {
	Common::U32String messageText(_engine->_world->_aboutMessage, Common::kMacRoman);
	Common::U32String disclaimer("\n\n\n\nThis adventure was produced with World Builder\xAA\nthe adventure game creation system.\n\xA9 Copyright 1986 by William C. Appleton, All Right Reserved\nPublished by Silicon Beach Software, Inc.", Common::kMacRoman);

	_engine->sayText(_engine->_world->_aboutMessage);
	_engine->sayText(disclaimer, Common::TextToSpeechManager::QUEUE);
	messageText += disclaimer;

	Graphics::MacFont font(Graphics::kMacFontGeneva, 9, 0);
	Graphics::MacText aboutMessage(messageText, _wm, &font, Graphics::kColorBlack,
											 Graphics::kColorWhite, 400, Graphics::kTextAlignCenter);

	Graphics::MacDialogButtonArray buttons;

	buttons.push_back(new Graphics::MacDialogButton("OK", 191, aboutMessage.getTextHeight() + 30, 68, 28));
	// add a dummy button to push volume slider position down
	// to avoid the overlapping of volume slider with OK button in the about section
	buttons.push_back(new Graphics::MacDialogButton("", 0, aboutMessage.getTextHeight() + 100, 0, 0));

	AboutDialog about(&_screen, _wm, 450, &aboutMessage, 400, &buttons, 0);

	delete buttons.back();
	buttons.pop_back();
	// close the menu before calling run because it blocks execution
	if (_menu)
		_menu->closeMenu();

	int button = about.run();

	if (button == Graphics::kMacDialogQuitRequested)
		_engine->_shouldQuit = true;
}

void Gui::gameOver() {
	Graphics::MacDialogButtonArray buttons;

	buttons.push_back(new Graphics::MacDialogButton("OK", 66, 67, 68, 28));

	Graphics::MacFont font;

	Graphics::MacText gameOverMessage(*_engine->_world->_gameOverMessage, _wm, &font, Graphics::kColorBlack,
									  Graphics::kColorWhite, 199, Graphics::kTextAlignCenter);

	_engine->sayText(*_engine->_world->_gameOverMessage, Common::TextToSpeechManager::QUEUE);

	Graphics::MacDialog gameOverDialog(&_screen, _wm,  199, &gameOverMessage, 199, &buttons, 0);

	int button = gameOverDialog.run();

	if (button == Graphics::kMacDialogQuitRequested)
		_engine->_shouldQuit = true;

	_engine->doClose();

	disableAllMenus();
	enableNewGameMenus();
}

bool Gui::saveDialog() {
	Graphics::MacDialogButtonArray buttons;

	buttons.push_back(new Graphics::MacDialogButton("No", 19, 67, 68, 28));
	buttons.push_back(new Graphics::MacDialogButton("Yes", 112, 67, 68, 28));
	buttons.push_back(new Graphics::MacDialogButton("Cancel", 205, 67, 68, 28));

	Graphics::MacFont font;

	Graphics::MacText saveBeforeCloseMessage(*_engine->_world->_saveBeforeCloseMessage, _wm, &font, Graphics::kColorBlack,
									  Graphics::kColorWhite, 250, Graphics::kTextAlignCenter);

	_engine->sayText(*_engine->_world->_saveBeforeCloseMessage);

	Graphics::MacDialog save(&_screen, _wm, 291, &saveBeforeCloseMessage, 250, &buttons, 1);

	int button = save.run();

	if (button == Graphics::kMacDialogQuitRequested)
		_engine->_shouldQuit = true;
	else if (button == 2) // Cancel
		return false;
	else if (button == 1)
		_engine->saveGame();

	_engine->doClose();

	return true;
}

} // End of namespace Wage
