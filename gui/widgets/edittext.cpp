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
 */

#include "common/system.h"
#include "common/unicode-bidi.h"
#include "gui/widgets/edittext.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

EditTextWidget::EditTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, const Common::U32String &tooltip, uint32 cmd, uint32 finishCmd, ThemeEngine::FontStyle font)
	: EditableWidget(boss, x, y - 1, w, h + 2, scale, tooltip, cmd) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kEditTextWidget;
	_finishCmd = finishCmd;

	_leftPadding = _rightPadding = 0;

	setEditString(text);
	setFontStyle(font);
}

EditTextWidget::EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip, uint32 cmd, uint32 finishCmd, ThemeEngine::FontStyle font)
	: EditTextWidget(boss, x, y, w, h, false, text, tooltip, cmd, finishCmd, font) {
}

EditTextWidget::EditTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip, uint32 cmd, uint32 finishCmd, ThemeEngine::FontStyle font)
	: EditableWidget(boss, name, tooltip, cmd) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kEditTextWidget;
	_finishCmd = finishCmd;

	_leftPadding = _rightPadding = 0;
	_shiftPressed = _isDragging = false;

	setEditString(text);
	setFontStyle(font);
}

void EditTextWidget::setEditString(const Common::U32String &str) {
	EditableWidget::setEditString(str);
	_backupString = str;
}

void EditTextWidget::reflowLayout() {
	_leftPadding = g_gui.xmlEval()->getVar("Globals.EditTextWidget.Padding.Left", 0);
	_rightPadding = g_gui.xmlEval()->getVar("Globals.EditTextWidget.Padding.Right", 0);

	EditableWidget::reflowLayout();
}

void EditTextWidget::drawWidget() {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h),
	                                    ThemeEngine::kWidgetBackgroundEditText);

	// Draw the text
	adjustOffset();
	Common::Rect drawRect = getEditRect();
	drawRect.translate(_x, _y);
	setTextDrawableArea(drawRect);

	int x = -_editScrollOffset;
	int y = drawRect.top;

	int selBegin = _selCaretPos;
	int selEnd = _selOffset + _selCaretPos;
	if (selBegin > selEnd)
		SWAP(selBegin, selEnd);
	selBegin = MAX(selBegin, 0);
	selEnd = MAX(selEnd, 0);
	
	if (!g_gui.useRTL()) {
		Common::UnicodeBiDiText utxt(_editString);
		Common::U32String left = Common::U32String(utxt.visual.c_str(), utxt.visual.c_str() + selBegin);
		Common::U32String selected = Common::U32String(utxt.visual.c_str() + selBegin, selEnd - selBegin);
		Common::U32String right = Common::U32String(utxt.visual.c_str() + selEnd);
		Common::U32StringArray parts {left, selected, right};
		int scrollOffset = _editScrollOffset;
		for (uint i = 0; i < parts.size(); i++) {
			if (!parts[i].size())
				continue;
			Common::U32String part = parts[i];
			int partW = g_gui.getStringWidth(part, _font);
			int clipL = drawRect.left + (scrollOffset < 0 ? -scrollOffset : 0);
			int clipR = MIN(clipL + partW, (int)drawRect.right);
			if (x + partW > 0 && x < _w && clipL < drawRect.right) {
				int sO = scrollOffset < 0 ? 0 : -scrollOffset;
				_inversion = i == 1 ? ThemeEngine::kTextInversionFocus : ThemeEngine::kTextInversionNone;
				g_gui.theme()->drawText(Common::Rect(clipL, y, clipR, y + drawRect.height()), part, _state,
				                        _drawAlign, _inversion, sO, false, _font, ThemeEngine::kFontColorNormal, 
				                        true, _textDrawableArea);
			}
			x += partW;
			scrollOffset -= partW;
		}
	} else {
		// The above method does not render RTL languages correctly, so fallback to default method
		// There are only two possible cases, either the whole string has been selected
		// or nothing has been selected.
		_inversion = _selOffset ? ThemeEngine::kTextInversionFocus : ThemeEngine::kTextInversionNone;
		g_gui.theme()->drawText(drawRect, _editString, _state, _drawAlign, _inversion, 
		                        -_editScrollOffset, false, _font, ThemeEngine::kFontColorNormal, true, 
		                        _textDrawableArea);
	}
}

Common::Rect EditTextWidget::getEditRect() const {
	// Calculate (right - left) difference for editRect's X-axis coordinates:
	// (_w - 1 - _rightPadding) - (2 + _leftPadding)
	int editWidth = _w - _rightPadding - _leftPadding - 3;
	int editHeight = _h - 2;
	// Ensure r will always be a valid rect
	if (editWidth < 0) {
		editWidth = 0;
	}
	if (editHeight < 0) {
		editHeight = 0;
	}
	Common::Rect r(2 + _leftPadding, 1, 2 + _leftPadding + editWidth, 1 + editHeight);

	return r;
}

void EditTextWidget::receivedFocusWidget() {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
}

void EditTextWidget::lostFocusWidget() {
	// If we lose focus, 'commit' the user changes and clear selection
	_backupString = _editString;
	drawCaret(true);
	clearSelection();

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
