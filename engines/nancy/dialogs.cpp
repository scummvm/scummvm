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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/winexe.h"
#include "common/translation.h"
#include "common/stream.h"

#include "gui/widgets/tab.h"
#include "gui/widgets/edittext.h"

#include "gui/ThemeEval.h"

#include "engines/nancy/dialogs.h"
#include "engines/nancy/iff.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {

NancyOptionsWidget::NancyOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "NancyOptionsDialog", false, domain) {
	_playerSpeechCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "NancyOptionsDialog.PlayerSpeech", _("Player Speech"), _("Enable player speech. Only works if speech is enabled in the Audio settings."));
	_characterSpeechCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "NancyOptionsDialog.CharacterSpeech", _("Character Speech"), _("Enable NPC speech. Only works if speech is enabled in the Audio settings."));
	_originalMenusCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "NancyOptionsDialog.OriginalMenus", _("Use original menus"), _("Use the original engine's main, save/load, and setup menus. ScummVM's Global Main Menu can still be accessed through its keymap."));

	// I18N: Second Chance is the name of the original engine's autosave system
	_secondChanceCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "NancyOptionsDialog.SecondChance", _("Enable Second Chance"), _("Enable the Second Chance feature, which automatically saves at specific scenes. Enabling this disables timed autosaves."));

	new GUI::StaticTextWidget(widgetsBoss(), "NancyOptionsDialog.SpeechSettingsLabel", _("Speech Options"));
	new GUI::StaticTextWidget(widgetsBoss(), "NancyOptionsDialog.EngineSettingsLabel", _("Engine Options"));
}

void NancyOptionsWidget::load() {
	_playerSpeechCheckbox->setState(ConfMan.getBool("player_speech", _domain));
	_characterSpeechCheckbox->setState(ConfMan.getBool("character_speech", _domain));
	_originalMenusCheckbox->setState(ConfMan.getBool("original_menus", _domain));
	_secondChanceCheckbox->setState(ConfMan.getBool("second_chance", _domain));
}

bool NancyOptionsWidget::save() {
	ConfMan.setBool("player_speech", _playerSpeechCheckbox->getState(), _domain);
	ConfMan.setBool("character_speech", _characterSpeechCheckbox->getState(), _domain);
	ConfMan.setBool("original_menus", _originalMenusCheckbox->getState(), _domain);
	ConfMan.setBool("second_chance", _secondChanceCheckbox->getState(), _domain);

	return true;
}

void NancyOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
			.addPadding(16, 16, 16, 16)
			.addWidget("SpeechSettingsLabel", "OptionsLabel")
			.addWidget("PlayerSpeech", "Checkbox")
			.addWidget("CharacterSpeech", "Checkbox")
			.addSpace(16)
			.addWidget("EngineSettingsLabel", "OptionsLabel")
			.addWidget("OriginalMenus", "Checkbox")
			.addWidget("SecondChance", "Checkbox")
		.closeLayout()
	.closeDialog();
}

bool NancyOptionsWidget::isInGame() const {
	return _domain.equals(ConfMan.getActiveDomainName());
}

// TODO rewrite this class so its layout is not hardcoded
CheatDialog::CheatDialog() : GUI::Dialog(20, 20, 600, 440) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
	Common::WinResources *res = Common::WinResources::createFromEXE("game.exe");
	Common::Array<Common::WinResourceID> dialogIDs = res->getIDList(Common::kWinDialog);
	State::SceneInfo scene = NancySceneState.getSceneInfo();
	Time playerTime = NancySceneState._timers.playerTime;
	Time timerTime = NancySceneState._timers.timerTime;
	bool timerIsActive = NancySceneState._timers.timerIsActive;
	if (!timerIsActive) {
		timerTime = 0;
	}

	GUI::TabWidget *_tabs = new GUI::TabWidget(this, 0, 0, 600, 370);
	new GUI::ButtonWidget(this, 420, 410, 60, 20, _("Cancel"), Common::U32String(), GUI::kCloseCmd);
	new GUI::ButtonWidget(this, 520, 410, 60, 20, _("OK"), Common::U32String(), GUI::kOKCmd);

	_tabs->addTab(_("General"), "Cheat.General");

	new GUI::StaticTextWidget(_tabs, 30, 20, 150, 20, _("Scene Data"), Graphics::kTextAlignLeft);
	_restartScene = new GUI::CheckboxWidget(_tabs, 35, 50, 150, 20, _("Restart the Scene"));
	_scene = new GUI::EditTextWidget(_tabs, 35, 75, 45, 20, Common::U32String::format("%u", scene.sceneID), Common::U32String(), kInputSceneNr, kInputSceneNr);
	new GUI::StaticTextWidget(_tabs, 85, 75, 150, 20, _("Scene Number"), Graphics::kTextAlignLeft);
	_frame = new GUI::EditTextWidget(_tabs, 35, 100, 45, 20, Common::U32String::format("%u", scene.frameID), Common::U32String(), kInputFrameNr, kInputFrameNr);
	new GUI::StaticTextWidget(_tabs, 85, 100, 150, 20, _("Frame Number"), Graphics::kTextAlignLeft);
	_offset = new GUI::EditTextWidget(_tabs, 35, 125, 45, 20, Common::U32String::format("%u", scene.verticalOffset), Common::U32String(), kInputScroll, kInputScroll);

	// I18N: The Y position (a.k.a vertical scroll) of the background
	new GUI::StaticTextWidget(_tabs, 85, 125, 150, 20, _("Background Top (Y)"), Graphics::kTextAlignLeft);

	new GUI::StaticTextWidget(_tabs, 30, 160, 150, 20, _("Hints Remaining"), Graphics::kTextAlignLeft);
	new GUI::StaticTextWidget(_tabs, 35, 185, 45, 20, _("Easy"), Graphics::kTextAlignLeft);
	_hintsRemainingEasy = new GUI::EditTextWidget(_tabs, 35, 205, 45, 20, Common::U32String::format("%u", NancySceneState._hintsRemaining[0]), Common::U32String(), kInputHintsEasy, kInputHintsEasy);
	new GUI::StaticTextWidget(_tabs, 85, 185, 45, 20, _("Medium"), Graphics::kTextAlignLeft);
	_hintsRemainingMedium = new GUI::EditTextWidget(_tabs, 85, 205, 45, 20, Common::U32String::format("%u", NancySceneState._hintsRemaining[1]), Common::U32String(), kInputHintsMedium, kInputHintsMedium);
	new GUI::StaticTextWidget(_tabs, 135, 185, 45, 20, _("Hard"), Graphics::kTextAlignLeft);
	_hintsRemainingHard = new GUI::EditTextWidget(_tabs, 135, 205, 45, 20, Common::U32String::format("%u", NancySceneState._hintsRemaining[2]), Common::U32String(), kInputHintsHard, kInputHintsHard);

	new GUI::StaticTextWidget(_tabs, 250, 20, 150, 20, _("Player Data"), Graphics::kTextAlignLeft);
	new GUI::StaticTextWidget(_tabs, 255, 50, 150, 20, _("Player Time:"), Graphics::kTextAlignLeft);
	_playerTimeDays = new GUI::EditTextWidget(_tabs, 255, 75, 35, 20, Common::U32String::format("%u", playerTime.getDays()), Common::U32String(), kInputPlayerTime, kInputPlayerTime);
	new GUI::StaticTextWidget(_tabs, 295, 75, 40, 20, _("Days"), Graphics::kTextAlignLeft);
	_playerTimeHours = new GUI::EditTextWidget(_tabs, 335, 75, 35, 20, Common::U32String::format("%u", playerTime.getHours()), Common::U32String(), kInputPlayerTime, kInputPlayerTime);
	new GUI::StaticTextWidget(_tabs, 375, 75, 40, 20, _("Hours"), Graphics::kTextAlignLeft);
	_playerTimeMinutes = new GUI::EditTextWidget(_tabs, 415, 75, 35, 20, Common::U32String::format("%u", playerTime.getMinutes()), Common::U32String(), kInputPlayerTime, kInputPlayerTime);
	new GUI::StaticTextWidget(_tabs, 455, 75, 50, 20, _("Minutes"), Graphics::kTextAlignLeft);
	_difficulty = new GUI::EditTextWidget(_tabs, 255, 105, 35, 20, Common::U32String::format("%u", NancySceneState._difficulty), Common::U32String(), kInputDifficulty, kInputDifficulty);
	new GUI::StaticTextWidget(_tabs, 295, 105, 150, 20, _("Player Difficulty Level"), Graphics::kTextAlignLeft);

	new GUI::StaticTextWidget(_tabs, 250, 140, 150, 20, _("Software Timer"), Graphics::kTextAlignLeft);
	_timerOn = new GUI::CheckboxWidget(_tabs, 255, 170, 150, 20, _("Timer On"));
	_timerOn->setState(timerIsActive);
	_timerHours = new GUI::EditTextWidget(_tabs, 255, 195, 35, 20, Common::U32String::format("%u", timerTime.getTotalHours()), Common::U32String(), kInputTimer, kInputTimer);
	new GUI::StaticTextWidget(_tabs, 295, 195, 40, 20, _("Hours"), Graphics::kTextAlignLeft);
	_timerMinutes = new GUI::EditTextWidget(_tabs, 335, 195, 35, 20, Common::U32String::format("%u", timerTime.getMinutes()), Common::U32String(), kInputTimer, kInputTimer);
	new GUI::StaticTextWidget(_tabs, 375, 195, 50, 20, _("Minutes"), Graphics::kTextAlignLeft);
	_timerSeconds = new GUI::EditTextWidget(_tabs, 425, 195, 35, 20, Common::U32String::format("%u", timerTime.getSeconds()), Common::U32String(), kInputTimer, kInputTimer);
	new GUI::StaticTextWidget(_tabs, 465, 195, 50, 20, _("Seconds"), Graphics::kTextAlignLeft);

	_tabs->addTab(_("Inventory"), "Cheat.Inventory");

	for (uint i = 0; i < dialogIDs.size(); ++i) {
		Common::SeekableReadStream *resStream = res->getResource(Common::kWinDialog, dialogIDs[i].getID());

		Common::String idString;
		resStream->skip(0x16);
		while (true) {
			char add = resStream->readByte();
			if (add != 0) {
				idString += add;
				resStream->skip(1);
			} else {
				resStream->skip(1);
				break;
			}
		}

		if (!idString.hasPrefix("Inventory")) {
			continue;
		}

		idString.trim();
		uint numItems = 0;

		while (resStream->pos() < resStream->size()) {
			if (resStream->readUint16LE() == 0xFFFF && resStream->readSint16LE() == 0x80) {
				// Found a resource, read its string id
				Common::String itemLabel;

				while (true) {
					char add = resStream->readByte();
					if (add != 0) {
						itemLabel += add;
						resStream->skip(1);
					} else {
						resStream->skip(1);
						break;
					}
				}
				GUI::CheckboxWidget *box = new GUI::CheckboxWidget(_tabs, 250 * (numItems / 10) + 20, (350 / 10) * (numItems % 10) + 15, 250, 250/10, Common::U32String(itemLabel));
				box->setState(NancySceneState.hasItem(numItems) == kTrue);
				_inventory.push_back(box);

				++numItems;
			}
		}

		break;
	}

	_tabs->setActiveTab(0);
}

void CheatDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case GUI::kOKCmd: {
		if (_restartScene->getState()) {
			uint sceneID = atoi(Common::String(_scene->getEditString()).c_str());
			IFF iff(Common::String::format("S%u", sceneID));
			if (iff.load()) {
				NancySceneState.changeScene(
					atoi(Common::String(_scene->getEditString()).c_str()),
					atoi(Common::String(_frame->getEditString()).c_str()),
					atoi(Common::String(_offset->getEditString()).c_str()),
					true);
			} else {
				new GUI::StaticTextWidget(this, 20, 410, 150, 20, _("Invalid Scene ID!"), Graphics::kTextAlignLeft);
				return;
			}
		}

		if (_timerOn->getState()) {
			NancySceneState._timers.timerIsActive = true;
			Time &timer = NancySceneState._timers.timerTime;
			timer = 0;
			timer += 1000 * atoi(Common::String(_timerSeconds->getEditString()).c_str());
			timer += 60000 * atoi(Common::String(_timerMinutes->getEditString()).c_str());
			timer += 3600000 * atoi(Common::String(_timerHours->getEditString()).c_str());
		} else {
			NancySceneState.stopTimer();
		}

		Time &playerTime = NancySceneState._timers.timerTime;
		playerTime = 0;
		playerTime += 60000 * atoi(Common::String(_playerTimeMinutes->getEditString()).c_str());
		playerTime += 3600000 * atoi(Common::String(_playerTimeHours->getEditString()).c_str());
		playerTime += 86400000 * atoi(Common::String(_playerTimeMinutes->getEditString()).c_str());

		NancySceneState._difficulty = atoi(Common::String(_difficulty->getEditString()).c_str());
		NancySceneState._hintsRemaining[0] = atoi(Common::String(_hintsRemainingEasy->getEditString()).c_str());
		NancySceneState._hintsRemaining[1] = atoi(Common::String(_hintsRemainingMedium->getEditString()).c_str());
		NancySceneState._hintsRemaining[2] = atoi(Common::String(_hintsRemainingHard->getEditString()).c_str());

		for (uint i = 0; i < _inventory.size(); ++i) {
			if (NancySceneState.hasItem(i) == kTrue && !_inventory[i]->getState()) {
				NancySceneState.removeItemFromInventory(i, false);
			} else if (NancySceneState.hasItem(i) == kFalse && _inventory[i]->getState()) {
				NancySceneState.addItemToInventory(i);
			}
		}
		cmd = GUI::kCloseCmd;

		break;
	}
	case kInputSceneNr:
		sanitizeInput(_scene);
		break;
	case kInputFrameNr:
		sanitizeInput(_frame);
		break;
	case kInputScroll:
		sanitizeInput(_offset);
		break;
	case kInputDifficulty:
		sanitizeInput(_scene, 2);
		break;
	case kInputHintsEasy:
		sanitizeInput(_hintsRemainingEasy);
		break;
	case kInputHintsMedium:
		sanitizeInput(_hintsRemainingMedium);
		break;
	case kInputHintsHard:
		sanitizeInput(_hintsRemainingHard);
		break;
	case kInputPlayerTime:
		sanitizeInput(_playerTimeMinutes, 59);
		sanitizeInput(_playerTimeHours, 23);
		sanitizeInput(_playerTimeDays);
		break;
	case kInputTimer:
		sanitizeInput(_timerSeconds, 59);
		sanitizeInput(_timerMinutes, 59);
		sanitizeInput(_timerHours, 23);
		break;
	default:
		break;
	}

	Dialog::handleCommand(sender, cmd, data);
}

void CheatDialog::sanitizeInput(GUI::EditTextWidget *textWidget, int maxValue) {
	const Common::U32String &str = textWidget->getEditString();
	for (uint i = 0; i < str.size(); ++i) {
		if (!Common::isDigit(str[i])) {
			textWidget->setEditString(str.substr(0, i));
			break;
		}
	}

	if (maxValue > -1) {
		int number = atoi(Common::String(str).c_str());
		if (number > maxValue) {
			textWidget->setEditString(Common::U32String::format("%d", maxValue));
		}
	}

	textWidget->setCaretPos(str.size());
}

EventFlagDialog::EventFlagDialog() : GUI::Dialog(20, 20, 600, 440) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
	Common::WinResources *res = Common::WinResources::createFromEXE("game.exe");
	Common::Array<Common::WinResourceID> dialogIDs = res->getIDList(Common::kWinDialog);

	GUI::TabWidget *_tabs = new GUI::TabWidget(this, 0, 0, 600, 370);
	new GUI::ButtonWidget(this, 520, 410, 60, 20, _("Close"), Common::U32String(), GUI::kCloseCmd);

	for (uint i = 0; i < dialogIDs.size(); ++i) {
		Common::SeekableReadStream *resStream = res->getResource(Common::kWinDialog, dialogIDs[i].getID());

		Common::String idString;
		resStream->skip(0x16);
		while (true) {
			char add = resStream->readByte();
			if (add != 0) {
				idString += add;
				resStream->skip(1);
			} else {
				resStream->skip(1);
				break;
			}
		}

		if (!idString.hasPrefix("Event")) {
			continue;
		}

		idString.trim();
		_tabs->addTab(idString, Common::String("tab " + idString));
		uint numFlags = 0;

		while (resStream->pos() < resStream->size()) {
			if (resStream->readUint16LE() == 0xFFFF && resStream->readSint16LE() == 0x80) {
				// Found a resource, read its string id
				Common::String flagLabel;

				while (true) {
					char add = resStream->readByte();
					if (add != 0) {
						flagLabel += add;
						resStream->skip(1);
					} else {
						resStream->skip(1);
						break;
					}
				}

				// String begins with number so we assume it's an event flag radio button
				if (Common::isDigit(flagLabel.firstChar())) {
					Common::String num;
					uint j = 0;
					while (true) {
						if (Common::isDigit(flagLabel[j])) {
							num += flagLabel[j];
						} else {
							break;
						}
						++j;
					}

					uint32 command = atoi(num.c_str()) << 16 | 'ev';

					GUI::CheckboxWidget *box = new GUI::CheckboxWidget(_tabs, 300 * (numFlags / 12) + 20, (350 / 12) * (numFlags % 12) + 15, 300, 350/12, Common::U32String(flagLabel), Common::U32String(), command);
					box->setState(NancySceneState.getEventFlag(command >> 16));

					++numFlags;
				}
			}
		}
	}

	_tabs->setActiveTab(0);
}

void EventFlagDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	Dialog::handleCommand(sender, cmd, data);
	if (cmd & 'ev') {
		cmd >>= 16;
		NancySceneState.setEventFlag(cmd, data == 0 ? kFalse : kTrue);
	}
}

} // End of namespace Nancy
