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
	kProceedCmd = 'PROC'
};


UpdatesDialog::UpdatesDialog() : Dialog(30, 20, 260, 124) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	Common::U32String message = _(
		"ScummVM now supports automatic check for updates\n"
		"which requires access to the Internet. Would you\n"
		"like to enable this feature?");
	Common::U32String message2 = _("You can change this setting later in the Misc tab\n"
		"in the Options dialog.");

	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	Common::Array<Common::U32String> lines, lines2;
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

	y += kLineHeight;

	Common::U32String updMessage = _("Update check:");

	int updatelineWidth = g_gui.getFont().getStringWidth(updMessage) + 5;

	new StaticTextWidget(this, 10, y, updatelineWidth, kLineHeight, updMessage, Graphics::kTextAlignLeft);

	_updatesPopUp = new PopUpWidget(this, 10 + updatelineWidth, y, _w - 20 - updatelineWidth, g_gui.xmlEval()->getVar("Globals.PopUp.Height", kLineHeight));

	const int *vals = Common::UpdateManager::getUpdateIntervals();

	while (*vals != -1) {
		_updatesPopUp->appendEntry(Common::UpdateManager::updateIntervalToString(*vals), *vals);
		vals++;
	}

	_updatesPopUp->setSelectedTag(Common::UpdateManager::kUpdateIntervalOneWeek);

	y += kLineHeight * 2;

	for (uint i = 0; i < lines2.size(); i++) {
		new StaticTextWidget(this, 10, y, maxlineWidth2, kLineHeight,
								lines2[i], Graphics::kTextAlignLeft);
		y += kLineHeight;
	}

	y += kLineHeight + 3;

	int buttonPos = _w - buttonWidth - 10;

	_proceedButton = new ButtonWidget(this, buttonPos, _h - buttonHeight - 8, buttonWidth, buttonHeight,
				_("Proceed"), Common::U32String(), kProceedCmd, Common::ASCII_RETURN);
}

void UpdatesDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kProceedCmd) {
		ConfMan.setInt("updates_check", _updatesPopUp->getSelectedTag());

		if (g_system->getUpdateManager()) {
			if (_updatesPopUp->getSelectedTag() == Common::UpdateManager::kUpdateIntervalNotSupported) {
				g_system->getUpdateManager()->setAutomaticallyChecksForUpdates(Common::UpdateManager::kUpdateStateDisabled);
			} else {
				g_system->getUpdateManager()->setAutomaticallyChecksForUpdates(Common::UpdateManager::kUpdateStateEnabled);
				g_system->getUpdateManager()->setUpdateCheckInterval(_updatesPopUp->getSelectedTag());
			}
		}
		close();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
