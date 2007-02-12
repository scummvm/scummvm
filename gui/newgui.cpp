/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/cursorman.h"
#include "gui/newgui.h"
#include "gui/dialog.h"
#include "gui/eval.h"
#include "gui/ThemeModern.h"
#include "gui/ThemeClassic.h"

#include "common/config-manager.h"

DECLARE_SINGLETON(GUI::NewGui);

namespace GUI {

/*
 * TODO list
 * - add more widgets: edit field, popup, radio buttons, ...
 *
 * Other ideas:
 * - allow multi line (l/c/r aligned) text via StaticTextWidget ?
 * - add "close" widget to all dialogs (with a flag to turn it off) ?
 * - make dialogs "moveable" ?
 * - come up with a new look & feel / theme for the GUI
 * - ...
 */

enum {
	kDoubleClickDelay = 500, // milliseconds
	kCursorAnimateDelay = 250,
	kKeyRepeatInitialDelay = 400,
	kKeyRepeatSustainDelay = 100
};

// HACK. FIXME. This doesn't belong here. But otherwise it creates compilation problems
GuiObject::GuiObject(const Common::String &name) : _firstWidget(0) {
	_name = name;
}

void GuiObject::reflowLayout() {
	if (!_name.empty()) {
		if ((_x = g_gui.evaluator()->getVar(_name + ".x")) == EVAL_UNDEF_VAR)
			error("Undefined variable %s.x", _name.c_str());
		if ((_y = g_gui.evaluator()->getVar(_name + ".y")) == EVAL_UNDEF_VAR)
			error("Undefined variable %s.y", _name.c_str());
		_w = g_gui.evaluator()->getVar(_name + ".w");
		_h = g_gui.evaluator()->getVar(_name + ".h");
	
		if (_x < 0)
			error("Widget <%s> has x < 0", _name.c_str());
		if (_x >= g_system->getOverlayWidth())
			error("Widget <%s> has x > %d", _name.c_str(), g_system->getOverlayWidth());
		if (_x + _w > g_system->getOverlayWidth())
			error("Widget <%s> has x + w > %d (%d)", _name.c_str(), g_system->getOverlayWidth(), _x + _w);
		if (_y < 0)
			error("Widget <%s> has y < 0", _name.c_str());
		if (_y >= g_system->getOverlayWidth())
			error("Widget <%s> has y > %d", _name.c_str(), g_system->getOverlayHeight());
		if (_y + _h > g_system->getOverlayWidth())
			error("Widget <%s> has y + h > %d (%d)", _name.c_str(), g_system->getOverlayHeight(), _y + _h);
	}
}

// Constructor
NewGui::NewGui() : _needRedraw(false),
	_stateIsSaved(false), _cursorAnimateCounter(0), _cursorAnimateTimer(0) {
	_theme = 0;
	_useStdCursor = false;

	_system = g_system;
	_lastScreenChangeID = _system->getScreenChangeID();

	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));

	// Reset key repeat
	_currentKeyDown.keycode = 0;

	bool loadClassicTheme = true;
#ifndef DISABLE_FANCY_THEMES
	ConfMan.registerDefault("gui_theme", "default");
	Common::String style(ConfMan.get("gui_theme"));
	// The default theme for now is the 'modern' theme.
	if (style.compareToIgnoreCase("default") == 0)
		style = "modern";

	Common::String styleType;
	Common::ConfigFile cfg;
	if (loadNewTheme(style)) {
	   loadClassicTheme = false;
	} else {
	   loadClassicTheme = true;
	   warning("falling back to classic style");
	}
#endif
	
	if (loadClassicTheme) {
		_theme = new ThemeClassic(_system);
		assert(_theme);
		if (!_theme->init()) {
			error("Couldn't initialize classic theme");
		}
	}

	_theme->resetDrawArea();
	_themeChange = false;
}

bool NewGui::loadNewTheme(const Common::String &style) {
	Common::String styleType;
	Common::ConfigFile cfg;

	Common::String oldTheme = (_theme != 0) ? _theme->getStylefileName() : "";

	if (_theme)
		_theme->disable();

	if (_useStdCursor) {
		CursorMan.popCursorPalette();
		CursorMan.popCursor();
	}

	delete _theme;

	if (style.compareToIgnoreCase("classic (builtin)") == 0) {
		_theme = new ThemeClassic(_system, style);
	} else {	
		if (Theme::themeConfigUseable(style, "", &styleType, &cfg)) {
			if (0 == styleType.compareToIgnoreCase("classic"))
				_theme = new ThemeClassic(_system, style, &cfg);
#ifndef DISABLE_FANCY_THEMES
			else if (0 == styleType.compareToIgnoreCase("modern"))
				_theme = new ThemeModern(_system, style, &cfg);
#endif
			else
				warning("Unsupported theme type '%s'", styleType.c_str());
		} else {
			warning("Config '%s' is NOT usable for themes or not found", style.c_str());
		}
	}
	cfg.clear();

	if (!_theme)
		return (!oldTheme.empty() ? loadNewTheme(oldTheme) : false);

	if (!_theme->init()) {
		warning("Could not initialize your preferred theme");
		delete _theme;
		_theme = 0;
		loadNewTheme(oldTheme);
		return false;
	}
	_theme->resetDrawArea();

	if (!oldTheme.empty())
		screenChange();

	_themeChange = true;

	return true;
}

void NewGui::redraw() {
	int i;

	// Restore the overlay to its initial state, then draw all dialogs.
	// This is necessary to get the blending right.
	_theme->clearAll();

	for (i = 0; i < _dialogStack.size(); ++i) {
		_theme->closeDialog();
	}
	for (i = 0; i < _dialogStack.size(); i++) {
		// Special treatment when topmost dialog has dimsInactive() set to false
		// This is the case for PopUpWidget which should not dim a dialog
		// which it belongs to
		if ((i == _dialogStack.size() - 2) && !_dialogStack[i + 1]->dimsInactive())
			_theme->openDialog(true);
		else if ((i != (_dialogStack.size() - 1)) || !_dialogStack[i]->dimsInactive())
			_theme->openDialog(false);
		else
			_theme->openDialog(true);

		_dialogStack[i]->drawDialog();
	}

	_theme->drawAll();
}

Dialog *NewGui::getTopDialog() const {
	if (_dialogStack.empty())
		return 0;
	return _dialogStack.top();
}

void NewGui::runLoop() {
	Dialog *activeDialog = getTopDialog();
	bool didSaveState = false;
	int button;

	if (activeDialog == 0)
		return;

	if (!_stateIsSaved) {
		saveState();
		_theme->enable();
		didSaveState = true;

		_useStdCursor = !_theme->ownCursor();
		if (_useStdCursor)
			setupCursor();
	}

	while (!_dialogStack.empty() && activeDialog == getTopDialog()) {
		if (_needRedraw) {
			redraw();
			_needRedraw = false;
		}

		// Don't "tickle" the dialog until the theme has had a chance
		// to re-allocate buffers in case of a scaler change.

		activeDialog->handleTickle();

		if (_useStdCursor)
			animateCursor();
		_theme->drawAll();
		_system->updateScreen();

		OSystem::Event event;
		uint32 time = _system->getMillis();

		while (_system->pollEvent(event)) {
			if (activeDialog != getTopDialog() && event.type != OSystem::EVENT_QUIT && event.type != OSystem::EVENT_SCREEN_CHANGED)
				continue;

			Common::Point mouse(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y);

			// HACK to change the cursor to the new themes one
			if (_themeChange) {
				_theme->enable();

				_useStdCursor = !_theme->ownCursor();
				if (_useStdCursor)
					setupCursor();

				_theme->refresh();

				_themeChange = false;
				redraw();
			}
			
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
#if !defined(PALMOS_MODE)
				// init continuous event stream
				// not done on PalmOS because keyboard is emulated and keyup is not generated
				_currentKeyDown.ascii = event.kbd.ascii;
				_currentKeyDown.keycode = event.kbd.keycode;
				_currentKeyDown.flags = event.kbd.flags;
				_keyRepeatTime = time + kKeyRepeatInitialDelay;
#endif
				activeDialog->handleKeyDown(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
				break;
			case OSystem::EVENT_KEYUP:
				activeDialog->handleKeyUp(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
				if (event.kbd.keycode == _currentKeyDown.keycode)
					// only stop firing events if it's the current key
					_currentKeyDown.keycode = 0;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				activeDialog->handleMouseMoved(mouse.x, mouse.y, 0);
				break;
			// We don't distinguish between mousebuttons (for now at least)
			case OSystem::EVENT_LBUTTONDOWN:
			case OSystem::EVENT_RBUTTONDOWN:
				button = (event.type == OSystem::EVENT_LBUTTONDOWN ? 1 : 2);
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
			case OSystem::EVENT_LBUTTONUP:
			case OSystem::EVENT_RBUTTONUP:
				button = (event.type == OSystem::EVENT_LBUTTONUP ? 1 : 2);
				activeDialog->handleMouseUp(mouse.x, mouse.y, button, _lastClick.count);
				break;
			case OSystem::EVENT_WHEELUP:
				activeDialog->handleMouseWheel(mouse.x, mouse.y, -1);
				break;
			case OSystem::EVENT_WHEELDOWN:
				activeDialog->handleMouseWheel(mouse.x, mouse.y, 1);
				break;
			case OSystem::EVENT_QUIT:
				_system->quit();
				return;
			case OSystem::EVENT_SCREEN_CHANGED:
				screenChange();
				break;
			}
		}

		// check if event should be sent again (keydown)
		if (_currentKeyDown.keycode != 0 && activeDialog == getTopDialog()) {
			if (_keyRepeatTime < time) {
				// fire event
				activeDialog->handleKeyDown(_currentKeyDown.ascii, _currentKeyDown.keycode, _currentKeyDown.flags);
				_keyRepeatTime = time + kKeyRepeatSustainDelay;
			}
		}

		// Delay for a moment
		_system->delayMillis(10);
	}

	_theme->closeDialog();

	if (didSaveState) {
		_theme->disable();
		restoreState();
		_useStdCursor = false;
	}
}

#pragma mark -

void NewGui::saveState() {
	// Backup old cursor
	_currentKeyDown.keycode = 0;
	_lastClick.x = _lastClick.y = 0;
	_lastClick.time = 0;
	_lastClick.count = 0;

	_stateIsSaved = true;
}

void NewGui::restoreState() {
	if (_useStdCursor) {
		CursorMan.popCursor();
		CursorMan.popCursorPalette();
	}

	_system->updateScreen();

	_stateIsSaved = false;
}

void NewGui::openDialog(Dialog *dialog) {
	_dialogStack.push(dialog);
	_needRedraw = true;

	// We reflow the dialog just before opening it. If the screen changed
	// since the last time we looked, also refresh the loaded theme,
	// and reflow all other open dialogs, too.
	int tmpScreenChangeID = _system->getScreenChangeID();
	if (_lastScreenChangeID != tmpScreenChangeID) {
		_lastScreenChangeID = tmpScreenChangeID;

		// reinit the whole theme
		_theme->refresh();
		// refresh all dialogs
		for (int i = 0; i < _dialogStack.size(); ++i) {
			_dialogStack[i]->reflowLayout();
		}
	} else {
		dialog->reflowLayout();
	}
}

void NewGui::closeTopDialog() {
	// Don't do anything if no dialog is open
	if (_dialogStack.empty())
		return;

	// Remove the dialog from the stack
	_dialogStack.pop();
	_needRedraw = true;
}

void NewGui::setupCursor() {
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

void NewGui::animateCursor() {
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

WidgetSize NewGui::getWidgetSize() {
	return (WidgetSize)(_theme->_evaluator->getVar("widgetSize"));
}

void NewGui::clearDragWidget() {
	if (!_dialogStack.empty())
		_dialogStack.top()->_dragWidget = 0;
}

void NewGui::screenChange() {
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
	redraw();
}

} // End of namespace GUI
