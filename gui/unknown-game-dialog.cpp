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
	kAddAnyway = 'adda',
	kScrollContainerReflow = 'SCRf'
};

UnknownGameDialog::UnknownGameDialog(const DetectedGame &detectedGame) :
		Dialog(30, 20, 260, 124),
		_detectedGame(detectedGame) {
	// For now place the buttons with a default place and size. They will be resized and moved when rebuild() is called.
	_closeButton = new ButtonWidget(this, 0, 0, 0, 0, detectedGame.canBeAdded ? _("Cancel") : _("Close"), 0, kClose);

	if (detectedGame.canBeAdded) {
		_addAnywayButton = new ButtonWidget(this, 0, 0, 0, 0, _("Add anyway"), 0, kAddAnyway);
	} else {
		_addAnywayButton = nullptr;
	}

	//Check if we have clipboard functionality
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		_copyToClipboardButton = new ButtonWidget(this, 0, 0, 0, 0, _("Copy to clipboard"), 0, kCopyToClipboard);
	} else
		_copyToClipboardButton = nullptr;

	//Check if we have support for opening URLs
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		_openBugTrackerUrlButton = new ButtonWidget(this, 0, 0, 0, 0, _("Report game"), 0, kOpenBugtrackerURL);
	} else
		_openBugTrackerUrlButton = nullptr;

	// Use a ScrollContainer for the report in case we have a lot of lines.
	_textContainer = new ScrollContainerWidget(this, 0, 0, 0, 0, kScrollContainerReflow);
	_textContainer->setTarget(this);

	rebuild();
}

void UnknownGameDialog::handleMouseWheel(int x, int y, int direction) {
	_textContainer->handleMouseWheel(x, y, direction);
}

void UnknownGameDialog::reflowLayout() {
	rebuild();
	Dialog::reflowLayout();
}

void UnknownGameDialog::rebuild() {
	// TODO: Use a theme layout dialog definition

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

	Common::String reportTranslated = generateUnknownGameReport(_detectedGame, true, true);

	// Check if we have clipboard functionality and expand the reportTranslated message if needed...
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		reportTranslated += "\n";
		reportTranslated += _("Use the button below to copy the required game information into your clipboard.");
	}
	// Check if we have support for opening URLs and expand the reportTranslated message if needed...
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		reportTranslated += "\n";
		reportTranslated += _("You can also directly report your game to the Bug Tracker.");
	}

	// We use a ScrollContainer to display the text, with a 2 * 8 pixels margin to the dialog border,
	// the scrollbar, and 2 * 10 margin for the text in the container.
	// We also keep 2 * 10 pixels between the screen border and the dialog.
	int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	Common::Array<Common::String> lines;
	int maxlineWidth = g_gui.getFont().wordWrapText(reportTranslated, screenW - 2 * 20 - 16 - scrollbarWidth, lines);

	int lineCount = lines.size() + 1;

	_h = MIN(screenH - 20, lineCount * kLineHeight + kLineHeight + buttonHeight + 24);

	int closeButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_closeButton->getLabel()) + 10);
	int copyToClipboardButtonWidth = 0, openBugtrackerURLButtonWidth = 0, addAnywayButtonWidth = 0;
	int totalButtonWidth = closeButtonWidth;

	if (_copyToClipboardButton) {
		copyToClipboardButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_copyToClipboardButton->getLabel()) + 10);
		totalButtonWidth += copyToClipboardButtonWidth + 10;
	}
	if (_openBugTrackerUrlButton) {
		openBugtrackerURLButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_openBugTrackerUrlButton->getLabel()) + 10);
		totalButtonWidth += openBugtrackerURLButtonWidth + 10;
	}
	if (_addAnywayButton) {
		addAnywayButtonWidth = MAX(buttonWidth, g_gui.getFont().getStringWidth(_addAnywayButton->getLabel()) + 10);
		totalButtonWidth += addAnywayButtonWidth + 10;
	}

	_w = MAX(MAX(maxlineWidth, 0) + 16 + scrollbarWidth, totalButtonWidth) + 20;

	// Center the dialog on the screen
	_x = (g_system->getOverlayWidth() - _w) / 2;
	_y = (g_system->getOverlayHeight() - _h) / 2;

	// Now move the buttons and text container to their proper place
	int buttonPos = _w - 10;
	if (_addAnywayButton) {
		buttonPos -= addAnywayButtonWidth + 5;
		_addAnywayButton->resize(buttonPos, _h - buttonHeight - 8, addAnywayButtonWidth, buttonHeight);
	}
	buttonPos -= closeButtonWidth + 5;
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

Common::String UnknownGameDialog::encodeUrlString(const Common::String &string) {
	Common::String encoded;
	for (uint i = 0 ; i < string.size() ; ++i) {
		char c = string[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A'  && c <= 'Z') || (c >= '0' && c <= '9') ||
			c == '~' || c == '-' || c == '.' || c == '_')
			encoded += c;
		else
			encoded += Common::String::format("%%%02X", c);
	}
	return encoded;
}

Common::String UnknownGameDialog::generateBugtrackerURL() {
	Common::String report = generateUnknownGameReport(_detectedGame, false, false);
	report = encodeUrlString(report);

	Common::String engineName = encodeUrlString(_detectedGame.engineName);

	return Common::String::format(
		"https://www.scummvm.org/unknowngame?"
		"engine=%s"
		"&description=%s",
		engineName.c_str(),
		report.c_str());
}

void UnknownGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case kCopyToClipboard: {
		Common::String report = generateUnknownGameReport(_detectedGame, false, false);

		if (g_system->setTextInClipboard(report)) {
			g_system->displayMessageOnOSD(
					_("All necessary information about your game has been copied into the clipboard"));
		} else {
			g_system->displayMessageOnOSD(_("Copying the game information to the clipboard has failed!"));
		}
		break;
	}
	case kClose:
		// The user cancelled adding the game
		setResult(-1);
		close();
		break;
	case kAddAnyway:
		// When the detection entry comes from the fallback detector, the game can be added / launched anyways.
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
