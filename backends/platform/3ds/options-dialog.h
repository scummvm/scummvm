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

#ifndef OPTIONS_DIALOG_3DS_H
#define OPTIONS_DIALOG_3DS_H


#include "common/scummsys.h"
#include "common/str.h"
#include "gui/object.h"
#include "gui/widget.h"
#include "gui/dialog.h"
#include "gui/widgets/tab.h"
#include "scumm/dialogs.h"

namespace _3DS {

enum {
	kSave = 0x10000000,
	kScreenRadioGroup,
	kScreenTop,
	kScreenBottom,
	kScreenBoth,
};

extern bool optionMenuOpened;

class OptionsDialog : public GUI::Dialog {

public:
	OptionsDialog();
	~OptionsDialog();

	// GuiObject API
	void reflowLayout() override;

	bool getShowCursor() const;
	bool getSnapToBorder() const;
	bool getStretchToFit() const;
	int getSensitivity() const;
	int getScreen() const;
protected:
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	GUI::CheckboxWidget *_showCursorCheckbox;
	GUI::CheckboxWidget *_snapToBorderCheckbox;
	GUI::CheckboxWidget *_stretchToFitCheckbox;

	GUI::RadiobuttonGroup *_screenRadioGroup;
	GUI::RadiobuttonWidget *_screenTopRadioWidget;
	GUI::RadiobuttonWidget *_screenBottomRadioWidget;
	GUI::RadiobuttonWidget *_screenBothRadioWidget;
};

} // namespace _3DS

#endif // OPTIONS_DIALOG_3DS_H
