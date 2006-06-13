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
#include "graphics/paletteman.h"
#include "gui/newgui.h"
#include "gui/dialog.h"
#include "gui/eval.h"

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
	handleScreenChanged();
}

void GuiObject::handleScreenChanged() {
	if (_name != "") {
		if ((_x = g_gui.evaluator()->getVar(_name + ".x")) == EVAL_UNDEF_VAR)
			error("Undefined variable %s.x", _name.c_str());
		if ((_y = g_gui.evaluator()->getVar(_name + ".y")) == EVAL_UNDEF_VAR)
			error("Undefined variable %s.y", _name.c_str());
		_w = g_gui.evaluator()->getVar(_name + ".w");
		_h = g_gui.evaluator()->getVar(_name + ".h");
	
		if(_x < 0)
			error("Widget <%s> has x < 0", _name.c_str());
		if(_x >= g_system->getOverlayWidth())
			error("Widget <%s> has x > %d", _name.c_str(), g_system->getOverlayWidth());
		if(_x + _w > g_system->getOverlayWidth())
			error("Widget <%s> has x + w > %d (%d)", _name.c_str(), g_system->getOverlayWidth(), _x + _w);
		if(_y < 0)
			error("Widget <%s> has y < 0", _name.c_str());
		if(_y >= g_system->getOverlayWidth())
			error("Widget <%s> has y > %d", _name.c_str(), g_system->getOverlayHeight());
		if(_y + _h > g_system->getOverlayWidth())
			error("Widget <%s> has y + h > %d (%d)", _name.c_str(), g_system->getOverlayHeight(), _y + _h);
	}
}

// Constructor
NewGui::NewGui() : _needRedraw(false),
	_stateIsSaved(false), _cursorAnimateCounter(0), _cursorAnimateTimer(0) {

	_system = g_system;

	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));

	// Reset key repeat
	_currentKeyDown.keycode = 0;

#ifndef DISABLE_FANCY_THEMES
	ConfMan.registerDefault("gui_theme", "default");
	Common::String style(ConfMan.get("gui_theme"));
	// The default theme for now is the 'modern' theme.
	if (scumm_stricmp(style.c_str(), "default") == 0)
		style = "modern";

	if (scumm_stricmp(style.c_str(), "classic") == 0) {
#endif
		_theme = new ThemeClassic(_system);
#ifndef DISABLE_FANCY_THEMES
	} else {
		_theme = new ThemeNew(_system, style.c_str());
	}
#endif
	assert(_theme);

	// Init the theme
	if (!_theme->init()) {
		warning("Could not initialize your preferred theme, falling back to classic style");
		delete _theme;
		_theme = new ThemeClassic(_system);
		assert(_theme);
		if (!_theme->init()) {
			error("Couldn't initialize classic theme");
		}
	}
	_theme->resetDrawArea();
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
}

void NewGui::runLoop() {
	Dialog *activeDialog = _dialogStack.top();
	bool didSaveState = false;
	int button;

	if (activeDialog == 0)
		return;

	if (!_stateIsSaved) {
		saveState();
		_theme->enable();
		didSaveState = true;
	}

	bool useStandardCurs = !_theme->ownCursor();

	if (useStandardCurs) {
		const byte palette[] = {
			255, 255, 255, 0,
			255, 255, 255, 0,
			171, 171, 171, 0,
			 87,  87,  87, 0
		};

		PaletteMan.pushCursorPalette(palette, 0, 4);
		CursorMan.pushCursor(NULL, 0, 0, 0, 0);
		CursorMan.showMouse(true);
	}

	while (!_dialogStack.empty() && activeDialog == _dialogStack.top()) {
		if (_needRedraw) {
			redraw();
			_needRedraw = false;
		}

		// Don't "tickle" the dialog until the theme has had a chance
		// to re-allocate buffers in case of a scaler change.

		activeDialog->handleTickle();

		if (useStandardCurs)
			animateCursor();
		_theme->drawAll();
		_system->updateScreen();

		OSystem::Event event;
		uint32 time = _system->getMillis();

		while (_system->pollEvent(event)) {
			Common::Point mouse(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y);
			
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
				// reinit the whole theme
				_theme->refresh();
				// refresh all dialogs
				for (int i = 0; i < _dialogStack.size(); ++i) {
					_dialogStack[i]->handleScreenChanged();
				}
				// We need to redraw immediately. Otherwise
				// some other event may cause a widget to be
				// redrawn before redraw() has been called.
				redraw();
				break;
			}
		}

		// check if event should be sent again (keydown)
		if (_currentKeyDown.keycode != 0) {
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
	if (useStandardCurs)
		PaletteMan.popCursorPalette();

	if (didSaveState) {
		_theme->disable();
		restoreState();
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
	CursorMan.popCursor();

	_system->updateScreen();

	_stateIsSaved = false;
}

void NewGui::openDialog(Dialog *dialog) {
	_dialogStack.push(dialog);
	_needRedraw = true;
}

void NewGui::closeTopDialog() {
	// Don't do anything if no dialog is open
	if (_dialogStack.empty())
		return;

	// Remove the dialog from the stack
	_dialogStack.pop();
	_needRedraw = true;
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

void NewGui::handleScreenChange() {
	_theme->refresh();
}

void NewGui::clearDragWidget() {
	_dialogStack.top()->_dragWidget = 0;
}

} // End of namespace GUI
