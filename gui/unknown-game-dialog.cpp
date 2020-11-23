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
	kAddAnyway = 'adda'
};

UnknownGameDialog::UnknownGameDialog(const DetectedGame &detectedGame) :
		Dialog("UnknownGameDialog"),
		_detectedGame(detectedGame) {

	if (detectedGame.canBeAdded) {
		_addAnywayButton = new ButtonWidget(this, "UnknownGameDialog.Add", _("Add anyway"), Common::U32String(), kAddAnyway);
	} else {
		_addAnywayButton = nullptr;
	}

	_closeButton = new ButtonWidget(this, "UnknownGameDialog.Close", detectedGame.canBeAdded ? _("Cancel") : _("Close"), Common::U32String(), kClose);

	//Check if we have clipboard functionality
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		_copyToClipboardButton = new ButtonWidget(this, "UnknownGameDialog.Copy", _("Copy to clipboard"), Common::U32String(), kCopyToClipboard);
	} else
		_copyToClipboardButton = nullptr;

	//Check if we have support for opening URLs
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		_openBugTrackerUrlButton = new ButtonWidget(this, "UnknownGameDialog.Report", _("Report game"), Common::U32String(), kOpenBugtrackerURL);
	} else
		_openBugTrackerUrlButton = nullptr;

	// Use a ScrollContainer for the report in case we have a lot of lines.
	_textContainer = new ScrollContainerWidget(this, "UnknownGameDialog.TextContainer", "");
	_textContainer->setTarget(this);

	rebuild();
}

void UnknownGameDialog::handleMouseWheel(int x, int y, int direction) {
	_textContainer->handleMouseWheel(x, y, direction);
}

void UnknownGameDialog::reflowLayout() {
	Dialog::reflowLayout();
	rebuild();
}

void UnknownGameDialog::rebuild() {
	// First remove the old text widgets
	for (uint i = 0; i < _textWidgets.size() ; i++) {
		_textContainer->removeWidget(_textWidgets[i]);

		// Also remove the widget from the dialog for the case it was
		// the active widget.
		removeWidget(_textWidgets[i]);
		delete _textWidgets[i];
	}
	_textWidgets.clear();

	Common::U32String reportTranslated = generateUnknownGameReport(_detectedGame, true, true);

	// Check if we have clipboard functionality and expand the reportTranslated message if needed...
	if (g_system->hasFeature(OSystem::kFeatureClipboardSupport)) {
		reportTranslated += Common::U32String("\n");
		reportTranslated += _("Use the button below to copy the required game information into your clipboard.");
	}
	// Check if we have support for opening URLs and expand the reportTranslated message if needed...
	if (g_system->hasFeature(OSystem::kFeatureOpenUrl)) {
		reportTranslated += Common::U32String("\n");
		reportTranslated += _("You can also directly report your game to the Bug Tracker.");
	}

	// We use a ScrollContainer to display the text, with a 2 * 10 margin for the text in the container.
	Common::Array<Common::U32String> lines;
	g_gui.getFont().wordWrapText(reportTranslated, _textContainer->getWidth() - 20, lines);

	// Create text widgets
	uint y = 8;
	for (uint i = 0; i < lines.size() ; i++) {
		StaticTextWidget *widget = new StaticTextWidget(_textContainer, 10, y, _textContainer->getWidth() - 20, kLineHeight, lines[i], Graphics::kTextAlignLeft);
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

	Common::String engineId = encodeUrlString(_detectedGame.engineId);

	return Common::String::format(
		"https://www.scummvm.org/unknowngame?"
		"engine=%s"
		"&description=%s",
		engineId.c_str(),
		report.c_str());
}

void UnknownGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case kCopyToClipboard: {
		Common::U32String report = generateUnknownGameReport(_detectedGame, false, false);

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
	default:
		break;
	}
}

} // End of namespace GUI
