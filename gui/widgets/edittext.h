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

#ifndef GUI_WIDGETS_EDITTEXT_H
#define GUI_WIDGETS_EDITTEXT_H

#include "gui/widgets/editable.h"
#include "common/str.h"
#include "gui/dialog.h"

namespace GUI {

/* EditTextWidget */
class EditTextWidget : public EditableWidget {
protected:
	Common::U32String _backupString;

	int				_leftPadding;
	int				_rightPadding;

public:
	EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint32 finishCmd = 0, ThemeEngine::FontStyle font = ThemeEngine::kFontStyleNormal);
	EditTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint32 finishCmd = 0, ThemeEngine::FontStyle font = ThemeEngine::kFontStyleNormal);

	void setEditString(const Common::U32String &str) override;

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
