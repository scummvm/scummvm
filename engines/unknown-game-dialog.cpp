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

#include "common/translation.h"
#include "common/str-array.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/ThemeEval.h"
#include "gui/widgets/popup.h"
#include "engines/unknown-game-dialog.h"
#include "backends/platform/sdl/sdl.h"

namespace GUI {

enum {
	kCopyToClipboard = 'cpcl',
	kOpenBugtrackerURL = 'ourl',
	kClose = 'clse'
};

UnknownGameDialog::UnknownGameDialog(const Common::String &reportData, const Common::String &reportTranslated, const Common::String &bugtrackerAffectedEngine)
	: Dialog(30, 20, 260, 124) {

	_reportData = reportData;
	_reportTranslated = reportTranslated;
	_bugtrackerAffectedEngine = bugtrackerAffectedEngine;

	//Check if we have clipboard functionality and expand the reportTranslated message if needed...
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		_reportTranslated += "\n";
		_reportTranslated += _("Use the button below to copy the required game information into your clipboard.");
	}

	//Check if we have support for opening URLs and expand the reportTranslated message if needed...
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		_reportTranslated += "\n";
		_reportTranslated += _("You can also directly report your game to the Bug Tracker!");
	}

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	//Calculate the size the dialog needs
	Common::Array<Common::String> lines;
	int maxlineWidth = g_gui.getFont().wordWrapText(_reportTranslated, screenW - 2 * 20, lines);
	int lineCount = lines.size() + 1;

	_h =  3 * kLineHeight;
	_h += lineCount * kLineHeight;

	// Buttons
	int closeButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_("Close")) + 10);
	int copyToClipboardButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_("Copy to clipboard")) + 10);
	int openBugtrackerURLButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_("Report game")) + 10);
	int totalButtonWidth = closeButtonWidth;
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport))
		totalButtonWidth += 10 + copyToClipboardButtonWidth;
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl))
		totalButtonWidth += 10 + openBugtrackerURLButtonWidth;

	_w = MAX(MAX(maxlineWidth, 0), totalButtonWidth) + 20;

	int closeButtonPos = _w - closeButtonWidth - 10;
	int copyToClipboardButtonPos = _w - copyToClipboardButtonWidth - closeButtonWidth - 15;
	int openBugtrackerURLButtonPos = _w - copyToClipboardButtonWidth - closeButtonWidth - openBugtrackerURLButtonWidth - 20;
	_closeButton = new ButtonWidget(this, closeButtonPos, _h - buttonHeight - 8, buttonWidth, buttonHeight, _("Close"), 0, kClose);

	//Check if we have clipboard functionality
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		new ButtonWidget(this, copyToClipboardButtonPos, _h - buttonHeight - 8, copyToClipboardButtonWidth, buttonHeight, _("Copy to clipboard"), 0, kCopyToClipboard);
	} else {
			//Set the bug tracker position to the one of the disabled copy-to-clipboard button
			openBugtrackerURLButtonPos = copyToClipboardButtonPos;
	}

	//Check if we have support for opening URLs
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		new ButtonWidget(this, openBugtrackerURLButtonPos, _h - buttonHeight - 8, openBugtrackerURLButtonWidth, buttonHeight, _("Report game"), 0, kOpenBugtrackerURL);
		//Formatting the reportData for bugtracker submission [replace line breaks]...
		_bugtrackerGameData = _reportData;
		while (_bugtrackerGameData.contains("\n")) {
			Common::replace(_bugtrackerGameData, "\n", "%0A");
		}
	}

	// Each line is represented by one static text item.
	uint y = 10;
	for (uint i = 0; i < lines.size(); i++) {
		new StaticTextWidget(this, 10, y, _w, kLineHeight,
								lines[i], Graphics::kTextAlignLeft);
		y += kLineHeight;
	}

	y += kLineHeight;
}

void UnknownGameDialog::reflowLayout() {
	_x = (g_system->getOverlayWidth() - _w) / 2;
	_y = (g_system->getOverlayHeight() - _h) / 2;
	GUI::Dialog::reflowLayout();
}

Common::String UnknownGameDialog::generateBugtrackerURL() {
		return Common::String::format((
		"https://bugs.scummvm.org/newticket?"
		"summary=[UNK] Unknown game for engine %s:"
		"&description=%s"
		"&component=Engine%3A%s"
		"&type=enhancement"
		"&keywords=unknown-game,%s"),
		_bugtrackerAffectedEngine.c_str(), _bugtrackerGameData.c_str(), _bugtrackerAffectedEngine.c_str(), _bugtrackerAffectedEngine.c_str());
}

void UnknownGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case kCopyToClipboard:
	{
		g_system->setTextInClipboard(_reportData);
		if (g_system->setTextInClipboard(_reportData)) {
			g_system->displayMessageOnOSD(_("All necessary information about your game has been copied into the clipboard"));
		} else {
			g_system->displayMessageOnOSD(_("Copying the game information to the clipboard has failed!"));
		}
		break;
	}
	case kClose:
	{
		close();
		break;
	}
	case kOpenBugtrackerURL:
	{
		g_system->openUrl(generateBugtrackerURL());
		break;
	}
}
}
} //End of namespace GUI