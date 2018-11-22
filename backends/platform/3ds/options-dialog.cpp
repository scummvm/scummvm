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

namespace _3DS {

bool optionMenuOpened = false;

OptionsDialog::OptionsDialog() : GUI::Dialog(20, 20, 280, 200) {

	optionMenuOpened = true;

	new GUI::ButtonWidget(this, 120, 180, 72, 16, _("~C~lose"), 0, GUI::kCloseCmd);
	new GUI::ButtonWidget(this, 200, 180, 72, 16, _("~S~ave"), 0, GUI::kOKCmd);

	_showCursorCheckbox = new GUI::CheckboxWidget(this, 5, 5, 130, 20, _("Show mouse cursor"), 0, 0, 'T');
	_showCursorCheckbox->setState(config.showCursor);

	_snapToBorderCheckbox = new GUI::CheckboxWidget(this, 5, 22, 130, 20, _("Snap to edges"), 0, 0, 'T');
	_snapToBorderCheckbox->setState(config.snapToBorder);

	_stretchToFitCheckbox = new GUI::CheckboxWidget(this, 140, 5, 130, 20, _("Stretch to fit"), 0, 0, 'T');
	_stretchToFitCheckbox->setState(config.stretchToFit);

	new GUI::StaticTextWidget(this, 0, 60, 110, 15, _("Use Screen:"), Graphics::kTextAlignRight);
	_screenRadioGroup = new GUI::RadiobuttonGroup(this, kScreenRadioGroup);
	_screenTopRadioWidget = new GUI::RadiobuttonWidget(this, 120, 50, 60, 20, _screenRadioGroup, kScreenTop, _("Top"));
	_screenBottomRadioWidget = new GUI::RadiobuttonWidget(this, 190, 50, 80, 20, _screenRadioGroup, kScreenBottom, _("Bottom"));
	_screenBothRadioWidget = new GUI::RadiobuttonWidget(this, 155, 70, 80, 20, _screenRadioGroup, kScreenBoth, _("Both"));
	_screenRadioGroup->setValue(config.screen);

	new GUI::StaticTextWidget(this, 0, 100, 110, 15, _("C-Pad Sensitivity:"), Graphics::kTextAlignRight);
	_sensitivity = new GUI::SliderWidget(this, 115, 100, 160, 15, "TODO: Add tooltip", 1);
	_sensitivity->setMinValue(-15);
	_sensitivity->setMaxValue(30);
	_sensitivity->setValue(config.sensitivity);
	_sensitivity->setFlags(GUI::WIDGET_CLEARBG);
}

OptionsDialog::~OptionsDialog() {
	optionMenuOpened = false;
}

void OptionsDialog::updateConfigManager() {
	config.showCursor = _showCursorCheckbox->getState();
	config.snapToBorder = _snapToBorderCheckbox->getState();
	config.stretchToFit = _stretchToFitCheckbox->getState();
	config.sensitivity = _sensitivity->getValue();
	config.screen = _screenRadioGroup->getValue();
	saveConfig();
	loadConfig();
}

void OptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case GUI::kOKCmd:
		updateConfigManager();
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
