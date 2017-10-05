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
#include "common/translation.h"
#include "common/updates.h"
#include "common/config-manager.h"

#include "gui/updates-dialog.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widgets/popup.h"

namespace GUI {

enum {
	kYesCmd = 'YES ',
	kNoCmd = 'NO  ',
	kCheckBoxCmd = 'CHK '
};


UpdatesDialog::UpdatesDialog() : Dialog(30, 20, 260, 124) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	const char *message = _(
		"ScummVM now supports automatic check for updates\n"
		"which requires access to the Internet.\n"
  		"\n"
		"Would you like to enable this feature?");
	const char *message2 = _("You can always enable it in the options dialog on the Misc tab.");

	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	Common::Array<Common::String> lines, lines2;
	int maxlineWidth = g_gui.getFont().wordWrapText(message, screenW - 2 * 20, lines);
	int maxlineWidth2 = g_gui.getFont().wordWrapText(message2, screenW - 2 * 20, lines2);

	_w = MAX(MAX(maxlineWidth, maxlineWidth2), (2 * buttonWidth) + 10) + 20;

	int lineCount = lines.size() + 1 + lines2.size();

	_h = 16 + 3 * kLineHeight;
	_h += buttonHeight + 8;

	_h += lineCount * kLineHeight;

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	// Each line is represented by one static text item.
	uint y = 10;
	for (uint i = 0; i < lines.size(); i++) {
		new StaticTextWidget(this, 10, y, maxlineWidth, kLineHeight,
								lines[i], Graphics::kTextAlignLeft);
		y += kLineHeight;
	}
	for (uint i = 0; i < lines2.size(); i++) {
		new StaticTextWidget(this, 10, y, maxlineWidth2, kLineHeight,
								lines2[i], Graphics::kTextAlignLeft);
		y += kLineHeight;
	}

	y += kLineHeight;
	_updatesCheckbox = new CheckboxWidget(this, 10, y, _w, kLineHeight, _("Check for updates automatically"), 0, kCheckBoxCmd);

	y += kLineHeight + 3;

	_updatesPopUp = new PopUpWidget(this, 10, y, _w - 20, g_gui.xmlEval()->getVar("Globals.PopUp.Height", kLineHeight));

	const int *vals = Common::UpdateManager::getUpdateIntervals();

	while (*vals != -1) {
		_updatesPopUp->appendEntry(Common::UpdateManager::updateIntervalToString(*vals), *vals);
		vals++;
	}

	_updatesPopUp->setSelectedTag(Common::UpdateManager::kUpdateIntervalOneWeek);

	int yesButtonPos = (_w - (buttonWidth * 2)) / 2;
	int noButtonPos = ((_w - (buttonWidth * 2)) / 2) + buttonWidth + 10;

	_yesButton = new ButtonWidget(this, yesButtonPos, _h - buttonHeight - 8, buttonWidth, buttonHeight,
				_("Proceed"), 0, kYesCmd, Common::ASCII_RETURN);
	_noButton = new ButtonWidget(this, noButtonPos, _h - buttonHeight - 8, buttonWidth, buttonHeight,
				_("Cancel"), 0, kNoCmd, Common::ASCII_ESCAPE);

	_updatesPopUp->setEnabled(false);
	_yesButton->setEnabled(false);
}

void UpdatesDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kYesCmd) {
		ConfMan.setInt("updates_check", _updatesPopUp->getSelectedTag());

		if (g_system->getUpdateManager()) {
			if (_updatesCheckbox->getState() == false ||
					_updatesPopUp->getSelectedTag() == Common::UpdateManager::kUpdateIntervalNotSupported) {
				g_system->getUpdateManager()->setAutomaticallyChecksForUpdates(Common::UpdateManager::kUpdateStateDisabled);
			} else {
				g_system->getUpdateManager()->setAutomaticallyChecksForUpdates(Common::UpdateManager::kUpdateStateEnabled);
				g_system->getUpdateManager()->setUpdateCheckInterval(_updatesPopUp->getSelectedTag());
			}
		}
		close();
	} else if (cmd == kNoCmd) {
		ConfMan.setInt("updates_check", Common::UpdateManager::kUpdateIntervalNotSupported);
		g_system->getUpdateManager()->setAutomaticallyChecksForUpdates(Common::UpdateManager::kUpdateStateDisabled);

		close();
	} else if (cmd == kCheckBoxCmd) {
		_updatesPopUp->setEnabled(_updatesCheckbox->getState());
		_yesButton->setEnabled(_updatesCheckbox->getState());

		draw();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
