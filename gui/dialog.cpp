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

#include "gui/newgui.h"
#include "gui/dialog.h"
#include "gui/widget.h"
#include "gui/PopUpWidget.h"

#include "common/system.h"

namespace GUI {

/*
 * TODO list
 * - add some sense of the window being "active" (i.e. in front) or not. If it
 *   was inactive and just became active, reset certain vars (like who is focused).
 *   Maybe we should just add lostFocus and receivedFocus methods to Dialog, just
 *   like we have for class Widget?
 * ...
 */

Dialog::Dialog(int x, int y, int w, int h, bool dimsInactive_)
	: GuiObject(x, y, w, h),
	  _mouseWidget(0), _focusedWidget(0), _dragWidget(0), _visible(false), _drawingHints(0),
	  _dimsInactive(dimsInactive_) {
	_drawingHints = THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND;
}

Dialog::Dialog(Common::String name, bool dimsInactive_)
	: GuiObject(name),
	  _mouseWidget(0), _focusedWidget(0), _dragWidget(0), _visible(false), _drawingHints(0),
	  _dimsInactive(dimsInactive_) {
	_drawingHints = THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND;
}

Dialog::~Dialog() {
	delete _firstWidget;
	_firstWidget = 0;
}

int Dialog::runModal() {
	// Open up
	open();

	// Start processing events
	g_gui.runLoop();

	// Return the result code
	return _result;
}

void Dialog::open() {
	Widget *w = _firstWidget;

	_result = 0;
	_visible = true;
	g_gui.openDialog(this);

	// Search for the first objects that wantsFocus() (if any) and give it the focus
	while (w && !w->wantsFocus()) {
		w = w->_next;
	}

	if (w) {
		w->receivedFocus();
		_focusedWidget = w;
	}
}

void Dialog::close() {
	_visible = false;
	g_gui.closeTopDialog();

	if (_mouseWidget) {
		_mouseWidget->handleMouseLeft(0);
		_mouseWidget = 0;
	}
	releaseFocus();
}

void Dialog::handleScreenChanged() {
	// The screen has changed. That means the screen visual may also have
	// changed, so any cached image may be invalid. The subsequent redraw
	// should be treated as the very first draw.

	_drawingHints |= THEME_HINT_FIRST_DRAW; 
	Widget *w = _firstWidget;
	while (w) {
		w->handleScreenChanged();
		w->setHints(THEME_HINT_FIRST_DRAW);
		w = w->_next;
	}

	GuiObject::handleScreenChanged();
}

void Dialog::releaseFocus() {
	if (_focusedWidget) {
		_focusedWidget->lostFocus();
		_focusedWidget = 0;
	}
}

void Dialog::draw() {
	g_gui._needRedraw = true;
}

void Dialog::drawDialog() {

	if (!isVisible())
		return;

	g_gui.theme()->drawDialogBackground(Common::Rect(_x, _y, _x+_w, _y+_h), _drawingHints);
	_drawingHints &= ~THEME_HINT_FIRST_DRAW;

	// Draw all children
	Widget *w = _firstWidget;
	while (w) {
		w->draw();
		w = w->_next;
	}
}

void Dialog::handleMouseDown(int x, int y, int button, int clickCount) {
	Widget *w;

	w = findWidget(x, y);

	_dragWidget = w;

	// If the click occured inside a widget which is not the currently
	// focused one, change the focus to that widget.
	if (w && w != _focusedWidget && w->wantsFocus()) {
		// The focus will change. Tell the old focused widget (if any)
		// that it lost the focus.
		releaseFocus();

		// Tell the new focused widget (if any) that it just gained the focus.
		if (w)
			w->receivedFocus();

		_focusedWidget = w;
	}

	if (w)
		w->handleMouseDown(x - (w->getAbsX() - _x), y - (w->getAbsY() - _y), button, clickCount);
}

void Dialog::handleMouseUp(int x, int y, int button, int clickCount) {
	Widget *w;

	if (_focusedWidget) {
		//w = _focusedWidget;

		// Lose focus on mouseup unless the widget requested to retain the focus
		if (! (_focusedWidget->getFlags() & WIDGET_RETAIN_FOCUS )) {
			releaseFocus();
		}
	}

	w = _dragWidget;

	if (w)
		w->handleMouseUp(x - (w->getAbsX() - _x), y - (w->getAbsY() - _y), button, clickCount);

	_dragWidget = 0;
}

void Dialog::handleMouseWheel(int x, int y, int direction) {
	Widget *w;

	// This may look a bit backwards, but I think it makes more sense for
	// the mouse wheel to primarily affect the widget the mouse is at than
	// the widget that happens to be focused.

	w = findWidget(x, y);
	if (!w)
		w = _focusedWidget;
	if (w)
		w->handleMouseWheel(x, y, direction);
}

void Dialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (_focusedWidget) {
		if (_focusedWidget->handleKeyDown(ascii, keycode, modifiers))
			return;
	}

	// Hotkey handling
	if (ascii != 0) {
		Widget *w = _firstWidget;
		ascii = toupper(ascii);
		while (w) {
			if (w->_type == kButtonWidget && ascii == toupper(((ButtonWidget *)w)->_hotkey)) {
				// The hotkey for widget w was pressed. We fake a mouse click into the
				// button by invoking the appropriate methods.
				w->handleMouseDown(0, 0, 1, 1);
				w->handleMouseUp(0, 0, 1, 1);
				return;
			}
			w = w->_next;
		}
	}

	// ESC closes all dialogs by default
	if (keycode == 27) {
		setResult(-1);
		close();
	}

	// TODO: tab/shift-tab should focus the next/previous focusable widget
}

void Dialog::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
	// Focused widget receives keyup events
	if (_focusedWidget)
		_focusedWidget->handleKeyUp(ascii, keycode, modifiers);
}

void Dialog::handleMouseMoved(int x, int y, int button) {
	Widget *w;

	//if (!button)
	//	_dragWidget = 0;

	if (_focusedWidget && !_dragWidget) {
		w = _focusedWidget;
		int wx = w->getAbsX() - _x;
		int wy = w->getAbsY() - _y;

		// We still send mouseEntered/Left messages to the focused item
		// (but to no other items).
		bool mouseInFocusedWidget = (x >= wx && x < wx + w->_w && y >= wy && y < wy + w->_h);
		if (mouseInFocusedWidget && _mouseWidget != w) {
			if (_mouseWidget)
				_mouseWidget->handleMouseLeft(button);
			_mouseWidget = w;
			w->handleMouseEntered(button);
		} else if (!mouseInFocusedWidget && _mouseWidget == w) {
			_mouseWidget = 0;
			w->handleMouseLeft(button);
		}

		w->handleMouseMoved(x - wx, y - wy, button);
	}

	// While a "drag" is in process (i.e. mouse is moved while a button is pressed),
	// only deal with the widget in which the click originated.
	if (_dragWidget)
		w = _dragWidget;
	else
		w = findWidget(x, y);

	if (_mouseWidget != w) {
		if (_mouseWidget)
			_mouseWidget->handleMouseLeft(button);
		if (w)
			w->handleMouseEntered(button);
		_mouseWidget = w;
	}

	if (w && (w->getFlags() & WIDGET_TRACK_MOUSE)) {
		w->handleMouseMoved(x - (w->getAbsX() - _x), y - (w->getAbsY() - _y), button);
	}
}

void Dialog::handleTickle() {
	// Focused widget receives tickle notifications
	if (_focusedWidget && _focusedWidget->getFlags() & WIDGET_WANT_TICKLE) {
		_focusedWidget->handleTickle();
	}
}

void Dialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCloseCmd:
		close();
		break;
	}
}

/*
 * Determine the widget at location (x,y) if any. Assumes the coordinates are
 * in the local coordinate system, i.e. relative to the top left of the dialog.
 */
Widget *Dialog::findWidget(int x, int y) {
	return Widget::findWidgetInChain(_firstWidget, x, y);
}

Widget *Dialog::findWidget(const char *name) {
	return Widget::findWidgetInChain(_firstWidget, name);
}

void Dialog::deleteWidget(Widget *del) {
	Widget *w = _firstWidget;

	if (del == _firstWidget) {
		_firstWidget = _firstWidget->_next;
		return;
	}

	w = _firstWidget;
	while (w) {
		if (w->_next == del) {
			w->_next = w->_next->_next;
			return;
		}
		w = w->_next;
	}
}

ButtonWidget *Dialog::addButton(GuiObject *boss, int x, int y, const Common::String &label, uint32 cmd, char hotkey, WidgetSize ws) {
	int w = kButtonWidth;
	int h = kButtonHeight;
	if (ws == kBigWidgetSize) {
		w = kBigButtonWidth;
		h = kBigButtonHeight;
	}
	return new ButtonWidget(boss, x, y, w, h, label, cmd, hotkey, ws);
}

uint32 GuiObject::getMillis() {
	return g_system->getMillis();
}

} // End of namespace GUI
