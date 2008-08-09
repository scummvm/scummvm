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
#include "graphics/cursorman.h"
#include "gui/newgui.h"
#include "gui/dialog.h"
#include "gui/eval.h"
#include "gui/ThemeModern.h"
#include "gui/ThemeClassic.h"
#include "gui/ThemeRenderer.h"

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
	kCursorAnimateDelay = 250
};

void GuiObject::reflowLayout() {
	if (!_name.empty()) {
		if (!g_gui.xmlEval()->getWidgetData(_name, _x, _y, _w, _h)) {
			warning("Could not load widget position for '%s'", _name.c_str());
		}
		
		return;

		if (_x < 0)
			error("Widget <%s> has x < 0: %d", _name.c_str(), _x);
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
NewGui::NewGui() : _redrawStatus(kRedrawDisabled),
	_stateIsSaved(false), _cursorAnimateCounter(0), _cursorAnimateTimer(0) {
	_theme = 0;
	_useStdCursor = false;

	_system = g_system;
	_lastScreenChangeID = _system->getScreenChangeID();

	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));


	ConfMan.registerDefault("gui_theme", "default");
	Common::String style(ConfMan.get("gui_theme"));
	if (style.compareToIgnoreCase("default") == 0)
		style = "builtin";
		
	//DEBUG:
//	style = "scummodern";

	loadNewTheme(style);

	_theme->resetDrawArea();
	_themeChange = false;
}

NewGui::~NewGui() {
	delete _theme;
}

bool NewGui::loadNewTheme(const Common::String &filename) {
	Common::String oldTheme = (_theme != 0) ? _theme->getThemeFileName() : "";

	if (_theme)
		_theme->disable();

	if (_useStdCursor) {
		CursorMan.popCursorPalette();
		CursorMan.popCursor();
	}

	delete _theme;
	_theme = 0;

	_theme = new ThemeRenderer(filename, GUI::ThemeRenderer::kGfxAntialias16bit);

	if (!_theme)
		return (!oldTheme.empty() ? loadNewTheme(oldTheme) : false);

	_theme->init();
	_theme->resetDrawArea();

	if (!oldTheme.empty())
		screenChange();

	_themeChange = true;

	return true;
}

void NewGui::redraw() {
	int i;

	if (_redrawStatus == kRedrawDisabled)
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
			_theme->openDialog(true, (Theme::ShadingStyle)xmlEval()->getVar("Dialog." + _dialogStack.top()->_name + ".Shading", 0));
			_dialogStack.top()->drawDialog();
			_theme->finishBuffering();
			break;

		default:
			return;
	}

	_theme->updateScreen();
	_redrawStatus = kRedrawDisabled;
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
			if (activeDialog != getTopDialog() && event.type != Common::EVENT_QUIT && event.type != Common::EVENT_SCREEN_CHANGED)
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
				_system->quit();
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

void NewGui::saveState() {
	// Backup old cursor
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
	_redrawStatus = kRedrawOpenDialog;
	
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
	_redrawStatus = kRedrawCloseDialog;
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
	return (WidgetSize)(g_gui.xmlEval()->getVar("Globals.WidgetSize"));
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
	_redrawStatus = kRedrawFull;
	redraw();
}

} // End of namespace GUI
