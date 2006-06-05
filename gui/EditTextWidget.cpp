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
#include "gui/EditTextWidget.h"
#include "gui/dialog.h"
#include "gui/eval.h"
#include "gui/newgui.h"

namespace GUI {

EditTextWidget::EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text)
	: EditableWidget(boss, x, y - 1, w, h + 2) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kEditTextWidget;

	handleScreenChanged();
	setEditString(text);
}

EditTextWidget::EditTextWidget(GuiObject *boss, const String &name, const String &text)
	: EditableWidget(boss, name) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kEditTextWidget;
	_hints |= THEME_HINT_USE_SHADOW;

	handleScreenChanged();
	setEditString(text);
}

void EditTextWidget::setEditString(const String &str) {
	EditableWidget::setEditString(str);
	_backupString = str;
}

void EditTextWidget::handleScreenChanged() {
	EditableWidget::handleScreenChanged();
	_leftPadding = g_gui.evaluator()->getVar("EditTextWidget.leftPadding", 0);
	_rightPadding = g_gui.evaluator()->getVar("EditTextWidget.rightPadding", 0);

	_font = (Theme::FontStyle)g_gui.evaluator()->getVar("EditTextWidget.font", Theme::kFontStyleNormal);
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
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x+_w, _y+_h), _hints, Theme::kWidgetBackgroundEditText);

	// Draw the text
	adjustOffset();
	g_gui.theme()->drawText(Common::Rect(_x+2+ _leftPadding,_y+2, _x+_leftPadding+getEditRect().width()+2, _y+_h-2), _editString, Theme::kStateEnabled, Theme::kTextAlignLeft, false, -_editScrollOffset, false, _font);
}

Common::Rect EditTextWidget::getEditRect() const {
	Common::Rect r(2 + _leftPadding, 1, _w - 2 - _leftPadding - _rightPadding, _h-1);

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
