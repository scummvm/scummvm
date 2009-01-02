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
 * $URL$
 * $Id$
 */

#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "common/config-manager.h"

#include "gui/newgui.h"
#include "gui/dialog.h"
#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"

#include "graphics/cursorman.h"

DECLARE_SINGLETON(GUI::GuiManager);

namespace GUI {

enum {
	kDoubleClickDelay = 500, // milliseconds
	kCursorAnimateDelay = 250
};

// Constructor
GuiManager::GuiManager() : _redrawStatus(kRedrawDisabled),
	_stateIsSaved(false), _cursorAnimateCounter(0), _cursorAnimateTimer(0) {
	_theme = 0;
	_useStdCursor = false;

	_system = g_system;
	_lastScreenChangeID = _system->getScreenChangeID();

	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));


	ConfMan.registerDefault("gui_theme", "scummmodern.zip");
	Common::String themefile(ConfMan.get("gui_theme"));
	if (themefile.compareToIgnoreCase("default") == 0)
		themefile = "builtin";

	ConfMan.registerDefault("gui_renderer", ThemeEngine::findModeConfigName(ThemeEngine::_defaultRendererMode));
	ThemeEngine::GraphicsMode gfxMode = (ThemeEngine::GraphicsMode)ThemeEngine::findMode(ConfMan.get("gui_renderer"));

	// Try to load the theme
	if (!loadNewTheme(themefile, gfxMode)) {
		// Loading the theme failed, try to load the built-in theme
		if (!loadNewTheme("builtin", gfxMode)) {
			// Loading the built-in theme failed as well. Bail out
			error("Failed to load any GUI theme, aborting");
		}
	}
	_themeChange = false;
}

GuiManager::~GuiManager() {
	delete _theme;
}

bool GuiManager::loadNewTheme(Common::String filename, ThemeEngine::GraphicsMode gfx) {
	// If we are asked to reload the currently active theme, just do nothing
	// FIXME: Actually, why? It might be desirable at times to force a theme reload...
	if (_theme && filename == _theme->getThemeId() && gfx == _theme->getGraphicsMode())
		return true;

	ThemeEngine *newTheme = 0;

	if (gfx == ThemeEngine::kGfxDisabled)
		gfx = ThemeEngine::_defaultRendererMode;

	// Try to load the new theme
	newTheme = new ThemeEngine(filename, gfx);
	assert(newTheme);

	if (!newTheme->init())
		return false;

	//
	// Disable and delete the old theme
	//
	if (_theme)
		_theme->disable();
	delete _theme;

	if (_useStdCursor) {
		CursorMan.popCursorPalette();
		CursorMan.popCursor();
	}

	//
	// Enable the new theme
	//
	_theme = newTheme;
	_themeChange = true;

	// refresh all dialogs
	for (int i = 0; i < _dialogStack.size(); ++i) {
		_dialogStack[i]->reflowLayout();
	}

	// We need to redraw immediately. Otherwise
	// some other event may cause a widget to be
	// redrawn before redraw() has been called.
	_redrawStatus = kRedrawFull;
//	redraw();
//	_system->updateScreen();

	return true;
}

void GuiManager::redraw() {
	int i;

	if (_redrawStatus == kRedrawDisabled)
		return;

	if (_dialogStack.empty())
		return;

	switch (_redrawStatus) {
		case kRedrawCloseDialog:
		case kRedrawFull:
		case kRedrawTopDialog:
			_theme->clearAll();
			_theme->openDialog(true);

			for (i = 0; i < _dialogStack.size() - 1; i++) {
				_dialogStack[i]->drawDialog();
			}

			_theme->finishBuffering();
			_theme->updateScreen();

		case kRedrawOpenDialog:
			_theme->openDialog(true, (ThemeEngine::ShadingStyle)xmlEval()->getVar("Dialog." + _dialogStack.top()->_name + ".Shading", 0));
			_dialogStack.top()->drawDialog();
			_theme->finishBuffering();
			break;

		default:
			return;
	}

	_theme->updateScreen();
	_redrawStatus = kRedrawDisabled;
}

Dialog *GuiManager::getTopDialog() const {
	if (_dialogStack.empty())
		return 0;
	return _dialogStack.top();
}

void GuiManager::runLoop() {
	Dialog *activeDialog = getTopDialog();
	bool didSaveState = false;
	int button;
	uint32 time;

	if (activeDialog == 0)
		return;

	if (!_stateIsSaved) {
		saveState();
		_theme->enable();
		didSaveState = true;

		_useStdCursor = !_theme->ownCursor();
		if (_useStdCursor)
			setupCursor();

//		_theme->refresh();

		_themeChange = false;
		_redrawStatus = kRedrawFull;
		redraw();
	}

	Common::EventManager *eventMan = _system->getEventManager();
	uint32 lastRedraw = 0;
	const uint32 waitTime = 1000 / 45;

	while (!_dialogStack.empty() && activeDialog == getTopDialog()) {
		redraw();

		// Don't "tickle" the dialog until the theme has had a chance
		// to re-allocate buffers in case of a scaler change.

		activeDialog->handleTickle();

		if (_useStdCursor)
			animateCursor();
//		_theme->updateScreen();
//		_system->updateScreen();

		if (lastRedraw + waitTime < _system->getMillis()) {
			_theme->updateScreen();
			_system->updateScreen();
			lastRedraw = _system->getMillis();
		}

		Common::Event event;
		while (eventMan->pollEvent(event)) {

			// The top dialog can change during the event loop. In that case, flush all the
			// dialog-related events since they were probably generated while the old dialog
			// was still visible, and therefore not intended for the new one.
			//
			// This hopefully fixes strange behaviour/crashes with pop-up widgets. (Most easily
			// triggered in 3x mode or when running ScummVM under Valgrind.)
			if (activeDialog != getTopDialog() && event.type != Common::EVENT_SCREEN_CHANGED)
				continue;

			Common::Point mouse(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y);

			// HACK to change the cursor to the new themes one
			if (_themeChange) {
				_theme->enable();

				_useStdCursor = !_theme->ownCursor();
				if (_useStdCursor)
					setupCursor();

//				_theme->refresh();

				_themeChange = false;
				_redrawStatus = kRedrawFull;
				redraw();
			}

			if (lastRedraw + waitTime < _system->getMillis()) {
				_theme->updateScreen();
				_system->updateScreen();
				lastRedraw = _system->getMillis();
			}

			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				activeDialog->handleKeyDown(event.kbd);
				break;
			case Common::EVENT_KEYUP:
				activeDialog->handleKeyUp(event.kbd);
				break;
			case Common::EVENT_MOUSEMOVE:
				activeDialog->handleMouseMoved(mouse.x, mouse.y, 0);
				break;
			// We don't distinguish between mousebuttons (for now at least)
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
				button = (event.type == Common::EVENT_LBUTTONDOWN ? 1 : 2);
				time = _system->getMillis();
				if (_lastClick.count && (time < _lastClick.time + kDoubleClickDelay)
							&& ABS(_lastClick.x - event.mouse.x) < 3
							&& ABS(_lastClick.y - event.mouse.y) < 3) {
					_lastClick.count++;
				} else {
					_lastClick.x = event.mouse.x;
					_lastClick.y = event.mouse.y;
					_lastClick.count = 1;
				}
				_lastClick.time = time;
				activeDialog->handleMouseDown(mouse.x, mouse.y, button, _lastClick.count);
				break;
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_RBUTTONUP:
				button = (event.type == Common::EVENT_LBUTTONUP ? 1 : 2);
				activeDialog->handleMouseUp(mouse.x, mouse.y, button, _lastClick.count);
				break;
			case Common::EVENT_WHEELUP:
				activeDialog->handleMouseWheel(mouse.x, mouse.y, -1);
				break;
			case Common::EVENT_WHEELDOWN:
				activeDialog->handleMouseWheel(mouse.x, mouse.y, 1);
				break;
			case Common::EVENT_QUIT:
				return;
			case Common::EVENT_SCREEN_CHANGED:
				screenChange();
				break;
			default:
				break;
			}
		}

		// Delay for a moment
		_system->delayMillis(10);
	}

	if (didSaveState) {
		_theme->disable();
		restoreState();
		_useStdCursor = false;
	}
}

#pragma mark -

void GuiManager::saveState() {
	// Backup old cursor
	_lastClick.x = _lastClick.y = 0;
	_lastClick.time = 0;
	_lastClick.count = 0;

	_stateIsSaved = true;
}

void GuiManager::restoreState() {
	if (_useStdCursor) {
		CursorMan.popCursor();
		CursorMan.popCursorPalette();
	}

	_system->updateScreen();

	_stateIsSaved = false;
}

void GuiManager::openDialog(Dialog *dialog) {
	_dialogStack.push(dialog);
	if (_redrawStatus != kRedrawFull)
		_redrawStatus = kRedrawOpenDialog;

	// We reflow the dialog just before opening it. If the screen changed
	// since the last time we looked, also refresh the loaded theme,
	// and reflow all other open dialogs, too.
	if (!checkScreenChange())
		dialog->reflowLayout();
}

void GuiManager::closeTopDialog() {
	// Don't do anything if no dialog is open
	if (_dialogStack.empty())
		return;

	// Remove the dialog from the stack
	_dialogStack.pop();
	if (_redrawStatus != kRedrawFull)
		_redrawStatus = kRedrawCloseDialog;
}

void GuiManager::setupCursor() {
	const byte palette[] = {
		255, 255, 255, 0,
		255, 255, 255, 0,
		171, 171, 171, 0,
		 87,  87,  87, 0
	};

	CursorMan.pushCursorPalette(palette, 0, 4);
	CursorMan.pushCursor(NULL, 0, 0, 0, 0);
	CursorMan.showMouse(true);
}

// Draw the mouse cursor (animated). This is pretty much the same as in old
// SCUMM games, but the code no longer resembles what we have in cursor.cpp
// very much. We could plug in a different cursor here if we like to.

void GuiManager::animateCursor() {
	int time = _system->getMillis();
	if (time > _cursorAnimateTimer + kCursorAnimateDelay) {
		for (int i = 0; i < 15; i++) {
			if ((i < 6) || (i > 8)) {
				_cursor[16 * 7 + i] = _cursorAnimateCounter;
				_cursor[16 * i + 7] = _cursorAnimateCounter;
			}
		}

		CursorMan.replaceCursor(_cursor, 16, 16, 7, 7);

		_cursorAnimateTimer = time;
		_cursorAnimateCounter = (_cursorAnimateCounter + 1) % 4;
	}
}

bool GuiManager::checkScreenChange() {
	int tmpScreenChangeID = _system->getScreenChangeID();
	if (_lastScreenChangeID != tmpScreenChangeID) {
		GuiManager::screenChange();
		return true;
	}
	return false;
}

void GuiManager::screenChange() {
	_lastScreenChangeID = _system->getScreenChangeID();

	// reinit the whole theme
	_theme->refresh();

	// refresh all dialogs
	for (int i = 0; i < _dialogStack.size(); ++i) {
		_dialogStack[i]->reflowLayout();
	}
	// We need to redraw immediately. Otherwise
	// some other event may cause a widget to be
	// redrawn before redraw() has been called.
	_redrawStatus = kRedrawFull;
	redraw();
	_system->updateScreen();
}

} // End of namespace GUI
