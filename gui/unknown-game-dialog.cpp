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
#include "gui/widgets/scrollcontainer.h"

namespace GUI {

enum {
	kCopyToClipboard = 'cpcl',
	kOpenBugtrackerURL = 'ourl',
	kClose = 'clse',
	kScrollContainerReflow = 'SCRf'
};

UnknownGameDialog::UnknownGameDialog(const DetectionResults &detectionResults) :
		Dialog(30, 20, 260, 124),
		_detectionResults(detectionResults) {
	// For now place the buttons with a default place and size. They will be resized and moved when rebuild() is called.
	_closeButton = new ButtonWidget(this, 0, 0, 0, 0, _("Close"), 0, kClose);

	//Check if we have clipboard functionality
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		_copyToClipboardButton = new ButtonWidget(this, 0, 0, 0, 0, _("Copy to clipboard"), 0, kCopyToClipboard);
	} else
		_copyToClipboardButton = nullptr;

#if 0
	// Do not create the button for reporting the game directly to the bugtracker
	// for now until we find a proper solution for the problem that a change
	// to our bugtracker system might break the URL generation. A possible approach
	// for solving this would be to have a ULR under the .scummvm.org (of the type
	// https://www.scummvm.org/unknowngame?engine=Foo&description=Bar) that would
	// redirect to whatever our bugtracker system is.

	//Check if we have support for opening URLs
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		buttonPos -= openBugtrackerURLButtonWidth + 5;
		_openBugTrackerUrlButton = new ButtonWidget(this, 0, 0, 0, 0, _("Report game"), 0, kOpenBugtrackerURL);
		//Formatting the reportData for bugtracker submission [replace line breaks]...
		_bugtrackerGameData = _reportData;
		while (_bugtrackerGameData.contains("\n")) {
			Common::replace(_bugtrackerGameData, "\n", "%0A");
		}
	} else
#endif
		_openBugTrackerUrlButton = nullptr;

	// Use a ScrollContainer for the report in case we have a lot of lines.
	_textContainer = new ScrollContainerWidget(this, 0, 0, 0, 0, kScrollContainerReflow);
	_textContainer->setTarget(this);

	rebuild();
}

void UnknownGameDialog::reflowLayout() {
	rebuild();
	Dialog::reflowLayout();
}

void UnknownGameDialog::rebuild() {
	// First remove the old text widgets
	for (uint i = 0; i < _textWidgets.size() ; i++) {
		_textContainer->removeWidget(_textWidgets[i]);
		delete _textWidgets[i];
	}
	_textWidgets.clear();

	// Work out dialog size and position of the various elements in the dialog.
	// Limit the width of the dialog to 600 - 2 * 10 pixels.
	const int screenW = MIN((int)g_system->getOverlayWidth(), 600);
	const int screenH = g_system->getOverlayHeight();

	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);
	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);

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

	// We use a ScrollContainer to display the text, with a 2 * 8 pixels margin to the dialog border,
	// the scrollbar, and 2 * 10 margin for the text in the container.
	// We also keep 2 * 10 pixels between the screen border and the dialog.
	int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	Common::Array<Common::String> lines;
	int maxlineWidth = g_gui.getFont().wordWrapText(reportTranslated, screenW - 2 * 20 - 16 - scrollbarWidth, lines);

	int lineCount = lines.size() + 1;

	_h = MIN(screenH - 20, lineCount * kLineHeight + kLineHeight + buttonHeight + 24);

	int closeButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_closeButton->getLabel()) + 10);
	int copyToClipboardButtonWidth = 0, openBugtrackerURLButtonWidth = 0, totalButtonWidth = closeButtonWidth;
	if (_copyToClipboardButton) {
		copyToClipboardButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_copyToClipboardButton->getLabel()) + 10);
		totalButtonWidth += copyToClipboardButtonWidth + 10;
	}
	if (_openBugTrackerUrlButton) {
		openBugtrackerURLButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_openBugTrackerUrlButton->getLabel()) + 10);
		totalButtonWidth += openBugtrackerURLButtonWidth + 10;
	}

	_w = MAX(MAX(maxlineWidth, 0) + 16 + scrollbarWidth, totalButtonWidth) + 20;

	// Center the dialog on the screen
	_x = (g_system->getOverlayWidth() - _w) / 2;
	_y = (g_system->getOverlayHeight() - _h) / 2;

	// Now move the buttons and text container to their proper place
	int buttonPos = _w - closeButtonWidth - 10;
	_closeButton->resize(buttonPos, _h - buttonHeight - 8, closeButtonWidth, buttonHeight);
	if (_copyToClipboardButton) {
		buttonPos -= copyToClipboardButtonWidth + 5;
		_copyToClipboardButton->resize(buttonPos, _h - buttonHeight - 8, copyToClipboardButtonWidth, buttonHeight);
	}
	if (_openBugTrackerUrlButton) {
		buttonPos -= openBugtrackerURLButtonWidth + 5;
		_openBugTrackerUrlButton->resize(buttonPos, _h - buttonHeight - 8, openBugtrackerURLButtonWidth, buttonHeight);
	}

	int containerHeight = _h - kLineHeight - buttonHeight - 16;
	_textContainer->resize(8, 8, _w - 16, containerHeight);

	// And create text widgets
	uint y = 8;
	for (uint i = 0; i < lines.size() ; i++) {
		StaticTextWidget *widget = new StaticTextWidget(_textContainer, 10, y, _w - 36 - scrollbarWidth, kLineHeight, lines[i], Graphics::kTextAlignLeft);
		_textWidgets.push_back(widget);
		y += kLineHeight;
	}
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
	case kScrollContainerReflow:
		for (uint i = 0; i < _textWidgets.size() ; i++)
			_textWidgets[i]->setVisible(true);
		break;
	}
}

} // End of namespace GUI
