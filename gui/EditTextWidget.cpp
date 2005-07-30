/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "gui/EditTextWidget.h"
#include "gui/dialog.h"
#include "gui/newgui.h"

namespace GUI {

EditTextWidget::EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, WidgetSize ws)
	: EditableWidget(boss, x, y - 1, w, h + 2, ws) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kEditTextWidget;

	setEditString(text);
}

void EditTextWidget::setEditString(const String &str) {
	EditableWidget::setEditString(str);
	_backupString = str;
}

void EditTextWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	x += _editScrollOffset;

	int width = 0;
	uint i;

	for (i = 0; i < _editString.size(); ++i) {
		width += g_gui.getCharWidth(_editString[i]);
		if (width >= x)
			break;
	}
	if (setCaretPos(i))
		draw();
}


void EditTextWidget::drawWidget(bool hilite) {
	// Draw a thin frame around us.
	g_gui.hLine(_x, _y, _x + _w - 1, g_gui._color);
	g_gui.hLine(_x, _y + _h - 1, _x +_w - 1, g_gui._shadowcolor);
	g_gui.vLine(_x, _y, _y + _h - 1, g_gui._color);
	g_gui.vLine(_x + _w - 1, _y, _y + _h - 1, g_gui._shadowcolor);

	// Draw the text
	adjustOffset();
	g_gui.drawString(_editString, _x + 2, _y + 2, getEditRect().width(), g_gui._textcolor, kTextAlignLeft, -_editScrollOffset, false);
}

Common::Rect EditTextWidget::getEditRect() const {
	Common::Rect r(2, 1, _w - 2, _h);

	return r;
}

void EditTextWidget::receivedFocusWidget() {
}

void EditTextWidget::lostFocusWidget() {
	// If we loose focus, 'commit' the user changes
	_backupString = _editString;
	drawCaret(true);
}

void EditTextWidget::startEditMode() {
}

void EditTextWidget::endEditMode() {
	releaseFocus();
}

void EditTextWidget::abortEditMode() {
	setEditString(_backupString);
	releaseFocus();
}

} // End of namespace GUI
