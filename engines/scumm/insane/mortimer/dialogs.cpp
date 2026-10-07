/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/insane/mortimer/dialogs.h"

#include "common/translation.h"
#include "common/util.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/message.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/popup.h"

namespace Scumm {

MortimerDifficultyDialog::MortimerDifficultyDialog(bool isDemo, uint flightDifficulty, uint riddleDifficulty, bool girl, bool newPilot)
	: GUI::Dialog("MortimerDifficultyDialog"), _character(nullptr), _pilotName(nullptr) {
	new GUI::StaticTextWidget(this, _name + ".Title", newPilot ? _("New pilot") : _("Select difficulty"));
	if (newPilot) {
		new GUI::StaticTextWidget(this, _name + ".NameLabel", _("Pilot name:"));
		_pilotName = new GUI::EditTextWidget(this, _name + ".Name", Common::U32String());
	}
	if (!isDemo) {
		new GUI::StaticTextWidget(this, _name + ".CharacterLabel", _("Character:"));
		_character = new GUI::PopUpWidget(this, _name + ".Character");
		_character->appendEntry(_("Sid"), 0);
		_character->appendEntry(_("Sally"), 1);
		_character->setSelectedTag(girl ? 1 : 0);
	}
	new GUI::StaticTextWidget(this, _name + ".FlightLabel", _("Flight difficulty:"));
	new GUI::StaticTextWidget(this, _name + ".RiddleLabel", _("Riddle difficulty:"));
	_flight = new GUI::PopUpWidget(this, _name + ".Flight", _("Choose how difficult flying and recovering from crashes should be."));
	_riddle = new GUI::PopUpWidget(this, _name + ".Riddle", isDemo ?
		_("The demo includes only one set of riddles.") : _("Choose the riddle difficulty and the cost and number of guesses."));
	const char *const levels[] = {_s("Easy"), _s("Medium"), _s("Hard")};
	for (uint i = 0; i < ARRAYSIZE(levels); ++i) {
		_flight->appendEntry(_(levels[i]), i);
		if (!isDemo)
			_riddle->appendEntry(_(levels[i]), i);
	}
	if (isDemo) {
		_riddle->appendEntry(_("Fixed (demo)"), 0);
		_riddle->setEnabled(false);
	}
	_flight->setSelectedTag(MIN(flightDifficulty, 2U));
	_riddle->setSelectedTag(isDemo ? 0 : MIN(riddleDifficulty, 2U));
	new GUI::ButtonWidget(this, _name + ".Cancel", _("~C~ancel"), Common::U32String(), GUI::kCloseCmd);
	new GUI::ButtonWidget(this, _name + ".Start", _("Start"), Common::U32String(), GUI::kOKCmd, Common::ASCII_RETURN);
}

uint MortimerDifficultyDialog::flightDifficulty() const {
	return _flight->getSelectedTag();
}

uint MortimerDifficultyDialog::riddleDifficulty() const {
	return _riddle->getSelectedTag();
}

bool MortimerDifficultyDialog::girl() const {
	return _character && _character->getSelectedTag() == 1;
}

Common::String MortimerDifficultyDialog::pilotName() const {
	Common::String name = _pilotName ? _pilotName->getEditString().encode() : Common::String();
	name.trim();
	return name;
}

void MortimerDifficultyDialog::reflowLayout() {
	GUI::ThemeEval &layouts = *g_gui.xmlEval();
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	const int width = 240 * g_gui.getScaleFactor();
	layouts.addDialog(_name, "screen_center")
		.addLayout(GUI::ThemeLayout::kLayoutVertical, 8)
			.addPadding(8, 8, 8, 8)
			.addWidget("Title", "", width, lineHeight, Graphics::kTextAlignCenter);
	if (_pilotName)
		layouts.addWidget("NameLabel", "", -1, lineHeight).addWidget("Name", "EditTextWidget");
	if (_character)
		layouts.addWidget("CharacterLabel", "", -1, lineHeight).addWidget("Character", "PopUp");
	layouts.addWidget("FlightLabel", "", -1, lineHeight)
			.addWidget("Flight", "PopUp")
			.addWidget("RiddleLabel", "", -1, lineHeight)
			.addWidget("Riddle", "PopUp")
			.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 8, GUI::ThemeLayout::kItemAlignEnd)
				.addPadding(0, 0, 0, 0)
				.addSpace()
				.addWidget("Cancel", "Button")
				.addWidget("Start", "Button")
			.closeLayout()
		.closeLayout()
	.closeDialog();
	GUI::Dialog::reflowLayout();
}

void MortimerDifficultyDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == GUI::kOKCmd) {
		if (_pilotName && (pilotName().empty() || pilotName().size() > 31)) {
			GUI::MessageDialog message(pilotName().empty() ? _("Please enter a pilot name.") : _("Please enter a shorter pilot name."));
			message.runModal();
			return;
		}
		setResult(1);
		close();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

MortimerPilotDialog::MortimerPilotDialog(const SaveStateList &pilots) : GUI::Dialog("MortimerPilotDialog") {
	new GUI::StaticTextWidget(this, _name + ".Title", _("Select pilot"));
	new GUI::StaticTextWidget(this, _name + ".Hint", _("Progress is saved after each riddle is solved."));
	_pilot = new GUI::PopUpWidget(this, _name + ".Pilot");
	_pilot->appendEntry(_("New pilot"), kNewPilot);
	for (uint i = 0; i < pilots.size(); ++i)
		_pilot->appendEntry(pilots[i].getDescription(), pilots[i].getSaveSlot());
	_pilot->setSelected(pilots.empty() ? 0 : 1);
	new GUI::ButtonWidget(this, _name + ".Cancel", _("~C~ancel"), Common::U32String(), GUI::kCloseCmd);
	new GUI::ButtonWidget(this, _name + ".Start", _("Start"), Common::U32String(), GUI::kOKCmd, Common::ASCII_RETURN);
}

int MortimerPilotDialog::selectedPilot() const {
	return _pilot->getSelectedTag();
}

void MortimerPilotDialog::reflowLayout() {
	GUI::ThemeEval &layouts = *g_gui.xmlEval();
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	const int width = 280 * g_gui.getScaleFactor();
	layouts.addDialog(_name, "screen_center")
		.addLayout(GUI::ThemeLayout::kLayoutVertical, 8)
			.addPadding(8, 8, 8, 8)
			.addWidget("Title", "", width, lineHeight, Graphics::kTextAlignCenter)
			.addWidget("Pilot", "PopUp")
			.addWidget("Hint", "", -1, lineHeight, Graphics::kTextAlignCenter)
			.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 8, GUI::ThemeLayout::kItemAlignEnd)
				.addPadding(0, 0, 0, 0)
				.addSpace()
				.addWidget("Cancel", "Button")
				.addWidget("Start", "Button")
			.closeLayout()
		.closeLayout()
	.closeDialog();
	GUI::Dialog::reflowLayout();
}

void MortimerPilotDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == GUI::kOKCmd) {
		setResult(1);
		close();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace Scumm
