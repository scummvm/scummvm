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
 */

#include "common/system.h"
#include "gui/widgets/edittext.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

EditTextWidget::EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const U32String &text, const U32String &tooltip, uint32 cmd, uint32 finishCmd, ThemeEngine::FontStyle font)
	: EditableWidget(boss, x, y - 1, w, h + 2, tooltip, cmd) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kEditTextWidget;
	_finishCmd = finishCmd;

	setEditString(text);
	setFontStyle(font);

	_leftPadding = _rightPadding = 0;
}

EditTextWidget::EditTextWidget(GuiObject *boss, const String &name, const U32String &text, const U32String &tooltip, uint32 cmd, uint32 finishCmd, ThemeEngine::FontStyle font)
	: EditableWidget(boss, name, tooltip, cmd) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kEditTextWidget;
	_finishCmd = finishCmd;

	setEditString(text);
	setFontStyle(font);

	_leftPadding = _rightPadding = 0;
}

void EditTextWidget::setEditString(const U32String &str) {
	EditableWidget::setEditString(str);
	_backupString = str;
}

void EditTextWidget::reflowLayout() {
	_leftPadding = g_gui.xmlEval()->getVar("Globals.EditTextWidget.Padding.Left", 0);
	_rightPadding = g_gui.xmlEval()->getVar("Globals.EditTextWidget.Padding.Right", 0);

	EditableWidget::reflowLayout();
}

void EditTextWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (!isEnabled())
		return;

	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	if (g_gui.useRTL()) {
		x = _w - x;
	}

	x += _editScrollOffset;
	int width = 0;
	if (_drawAlign == Graphics::kTextAlignRight)
		width = _editScrollOffset + getEditRect().width() - g_gui.getStringWidth(_editString, _font);

	uint i;

	uint last = 0;
	for (i = 0; i < _editString.size(); ++i) {
		const uint cur = _editString[i];
		width += g_gui.getCharWidth(cur, _font) + g_gui.getKerningOffset(last, cur, _font);
		if (width >= x && width > _editScrollOffset + _leftPadding)
			break;
		last = cur;
	}

	setCaretPos(i);
	markAsDirty();
}

void EditTextWidget::drawWidget() {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h),
	                                    ThemeEngine::kWidgetBackgroundEditText);

	// Draw the text
	adjustOffset();

	const Common::Rect &r = Common::Rect(_x + 2 + _leftPadding, _y + 2, _x + _leftPadding + getEditRect().width() + 8, _y + _h);
	setTextDrawableArea(r);

	g_gui.theme()->drawText(
			Common::Rect(_x + 2 + _leftPadding, _y + 1, _x + _leftPadding + getEditRect().width() + 2, _y + _h),
			_editString, _state, _drawAlign, ThemeEngine::kTextInversionNone,
			-_editScrollOffset, false, _font, ThemeEngine::kFontColorNormal, true, _textDrawableArea);
}

Common::Rect EditTextWidget::getEditRect() const {
	Common::Rect r(2 + _leftPadding, 2, _w - 2 - _leftPadding - _rightPadding, _h);

	return r;
}

void EditTextWidget::receivedFocusWidget() {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
}

void EditTextWidget::lostFocusWidget() {
	// If we loose focus, 'commit' the user changes
	_backupString = _editString;
	drawCaret(true);

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void EditTextWidget::startEditMode() {
}

void EditTextWidget::endEditMode() {
	releaseFocus();

	sendCommand(_finishCmd, 0);
}

void EditTextWidget::abortEditMode() {
	setEditString(_backupString);
	sendCommand(_cmd, 0);

	releaseFocus();
}

} // End of namespace GUI
