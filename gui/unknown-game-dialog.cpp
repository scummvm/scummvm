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

#include "gui/unknown-game-dialog.h"

#include "common/translation.h"
#include "common/str-array.h"
#include "common/system.h"

#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/ThemeEval.h"
#include "gui/widgets/popup.h"

namespace GUI {

enum {
	kCopyToClipboard = 'cpcl',
	kOpenBugtrackerURL = 'ourl',
	kClose = 'clse'
};

UnknownGameDialog::UnknownGameDialog(const DetectionResults &detectionResults) :
		Dialog(30, 20, 260, 124),
		_detectionResults(detectionResults) {
	Common::String reportTranslated = _detectionResults.generateUnknownGameReport(true);

	// Check if we have clipboard functionality and expand the reportTranslated message if needed...
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		reportTranslated += "\n";
		reportTranslated += _("Use the button below to copy the required game information into your clipboard.");
	}

#if 0
	// Check if we have support for opening URLs and expand the reportTranslated message if needed...
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		reportTranslated += "\n";
		reportTranslated += _("You can also directly report your game to the Bug Tracker.");
	}
#endif

	const int screenW = g_system->getOverlayWidth();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	// Calculate the size the dialog needs
	Common::Array<Common::String> lines;
	int maxlineWidth = g_gui.getFont().wordWrapText(reportTranslated, screenW - 2 * 20, lines);
	int lineCount = lines.size() + 1;

	_h =  3 * kLineHeight + lineCount * kLineHeight;

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

	int buttonPos = _w - closeButtonWidth - 10;
	new ButtonWidget(this, buttonPos, _h - buttonHeight - 8, buttonWidth, buttonHeight, _("Close"), 0, kClose);

	// Check if we have clipboard functionality
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		buttonPos -= copyToClipboardButtonWidth + 5;
		new ButtonWidget(this, buttonPos, _h - buttonHeight - 8, copyToClipboardButtonWidth, buttonHeight, _("Copy to clipboard"), 0, kCopyToClipboard);
	}

#if 0
	// Do not create the button for reporting the game directly to the bugtracker
	// for now until we find a proper solution for the problem that a change
	// to our bugtracker system might break the URL generation. A possible approach
	// for solving this would be to have a ULR under the .scummvm.org (of the type
	// https://www.scummvm.org/unknowngame?engine=Foo&description=Bar) that would
	// redirect to whatever our bugtracker system is.

	// Check if we have support for opening URLs
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		buttonPos -= openBugtrackerURLButtonWidth + 5;
		new ButtonWidget(this, buttonPos, _h - buttonHeight - 8, openBugtrackerURLButtonWidth, buttonHeight, _("Report game"), 0, kOpenBugtrackerURL);
	}
#endif

	// Each line is represented by one static text item.
	// TODO: Use a ScrollContainer widget instead of truncated text.
	uint y = 10;
	for (uint i = 0; i < lines.size(); i++) {
		new StaticTextWidget(this, 10, y, _w, kLineHeight, lines[i], Graphics::kTextAlignLeft);
		y += kLineHeight;
	}
}

void UnknownGameDialog::reflowLayout() {
	_x = (g_system->getOverlayWidth() - _w) / 2;
	_y = (g_system->getOverlayHeight() - _h) / 2;
	Dialog::reflowLayout();
}

Common::String UnknownGameDialog::generateBugtrackerURL() {
	// TODO: Remove the filesystem path from the bugtracker report
	Common::String report = _detectionResults.generateUnknownGameReport(false);

	// Formatting the report for bugtracker submission [replace line breaks]...
	while (report.contains("\n")) {
		Common::replace(report, "\n", "%0A");
	}

	return Common::String::format(
		"https://bugs.scummvm.org/newticket?"
		"&description=%s"
		"&type=enhancement"
		"&keywords=unknown-game",
		report.c_str());
}

void UnknownGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case kCopyToClipboard: {
		// TODO: Remove the filesystem path from the report
		Common::String report = _detectionResults.generateUnknownGameReport(false);

		if (g_system->setTextInClipboard(report)) {
			g_system->displayMessageOnOSD(
					_("All necessary information about your game has been copied into the clipboard"));
		} else {
			g_system->displayMessageOnOSD(_("Copying the game information to the clipboard has failed!"));
		}
		break;
	}
	case kClose:
		// When the detection entry comes from the fallback detector, the game can be added / launched anyways.
		// TODO: Add a button to cancel adding the game. And make it clear that launching the game may not work properly.
		close();
		break;
	case kOpenBugtrackerURL:
		g_system->openUrl(generateBugtrackerURL());
		break;
	}
}

} // End of namespace GUI
