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

#ifndef GUI_WIDGETS_EDITTEXT_H
#define GUI_WIDGETS_EDITTEXT_H

#include "gui/widgets/editable.h"
#include "common/str.h"
#include "gui/dialog.h"

namespace GUI {

/* EditTextWidget */
class EditTextWidget : public EditableWidget {
protected:
	typedef Common::String String;
	typedef Common::U32String U32String;

	U32String		_backupString;

	int				_leftPadding;
	int				_rightPadding;

public:
	EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const U32String &text, const U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint32 finishCmd = 0, ThemeEngine::FontStyle font = ThemeEngine::kFontStyleNormal);
	EditTextWidget(GuiObject *boss, const String &name, const U32String &text, const U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint32 finishCmd = 0, ThemeEngine::FontStyle font = ThemeEngine::kFontStyleNormal);

	void setEditString(const U32String &str) override;

	void handleMouseDown(int x, int y, int button, int clickCount) override;

	bool wantsFocus() override { return true; }

	void reflowLayout() override;

protected:
	void drawWidget() override;
	void receivedFocusWidget() override;
	void lostFocusWidget() override;

	void startEditMode() override;
	void endEditMode() override;
	void abortEditMode() override;

	Common::Rect getEditRect() const override;

	uint32 _finishCmd;
};

} // End of namespace GUI

#endif
