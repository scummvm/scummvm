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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "options-dialog.h"
#include "config.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"
#include "gui/widgets/list.h"
#include "gui/widgets/tab.h"
#include "osystem.h"
#include "gui/widgets/popup.h"

#include "common/translation.h"
#include "common/ustr.h"

namespace _3DS {

bool optionMenuOpened = false;

OptionsDialog::OptionsDialog() : GUI::Dialog(20, 20, 280, 200) {

	optionMenuOpened = true;

	new GUI::ButtonWidget(this, 120, 180, 72, 16, _("~C~lose"), Common::U32String(), GUI::kCloseCmd);
	new GUI::ButtonWidget(this, 200, 180, 72, 16, _("~S~ave"), Common::U32String(), GUI::kOKCmd);

	_showCursorCheckbox = new GUI::CheckboxWidget(this, 5, 5, 130, 20, _("Show mouse cursor"), Common::U32String(), 0, 'T');
	_showCursorCheckbox->setState(config.showCursor);

	_snapToBorderCheckbox = new GUI::CheckboxWidget(this, 5, 22, 130, 20, _("Snap to edges"), Common::U32String(), 0, 'T');
	_snapToBorderCheckbox->setState(config.snapToBorder);

	_stretchToFitCheckbox = new GUI::CheckboxWidget(this, 140, 5, 130, 20, _("Stretch to fit"), Common::U32String(), 0, 'T');
	_stretchToFitCheckbox->setState(config.stretchToFit);

	new GUI::StaticTextWidget(this, 0, 60, 110, 15, _("Use Screen:"), Graphics::kTextAlignRight);
	_screenRadioGroup = new GUI::RadiobuttonGroup(this, kScreenRadioGroup);
	_screenTopRadioWidget = new GUI::RadiobuttonWidget(this, 120, 50, 60, 20, _screenRadioGroup, kScreenTop, _c("Top", "3ds-screen"));
	_screenBottomRadioWidget = new GUI::RadiobuttonWidget(this, 190, 50, 80, 20, _screenRadioGroup, kScreenBottom, _c("Bottom", "3ds-screen"));
	_screenBothRadioWidget = new GUI::RadiobuttonWidget(this, 155, 70, 80, 20, _screenRadioGroup, kScreenBoth, _c("Both", "3ds-screen"));
	_screenRadioGroup->setValue(config.screen);
}

OptionsDialog::~OptionsDialog() {
	optionMenuOpened = false;
}

bool OptionsDialog::getShowCursor() const {
	return _showCursorCheckbox->getState();
}

bool OptionsDialog::getSnapToBorder() const {
	return _snapToBorderCheckbox->getState();
}

bool OptionsDialog::getStretchToFit() const {
	return _stretchToFitCheckbox->getState();
}

int OptionsDialog::getScreen() const {
	return _screenRadioGroup->getValue();
}

void OptionsDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	// Center the dialog
	_x = (screenW - getWidth()) / 2;
	_y = (screenH - getHeight()) / 2;

	GUI::Dialog::reflowLayout();
}

void OptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case GUI::kOKCmd:
		setResult(1);
		// Fall through
	case GUI::kCloseCmd:
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

} // namespace _3DS
