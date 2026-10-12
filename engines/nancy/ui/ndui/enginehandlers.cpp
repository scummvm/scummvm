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

#include "common/debug.h"
#include "common/config-manager.h"
#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/ndui/manager.h"
#include "engines/nancy/ui/ndui/panel.h"
#include "engines/nancy/ui/ndui/button.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// The Nancy19 channels the UI plays a sound on for the player to hear a new volume with
static const uint16 kSampleChannelMusic = 4;
static const uint16 kSampleChannelFX = 22;
static const uint16 kSampleChannelVoice = 28;

// The timer whose displays show the time of day
static const int kPlayerClockTimer = -1;

void Manager::addEngineTarget(const Common::String &name, EngineTarget *target) {
	_engineTargets.setVal(name, target);
}

void Manager::removeEngineTarget(const Common::String &name) {
	_engineTargets.erase(name);
}

void Manager::takeEngineAction(const Action &action) {
	if (_engineTargets.contains(action.target)) {
		_engineTargets[action.target]->onEngineAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_Timer")) {
		// A control asks to show the value of a timer
		if (action.type == kActionSetText && (int)action.value >= 0) {
			for (const ValueDisplay &display : _timerDisplays) {
				if (display.name.equalsIgnoreCase(action.sender)) {
					return;
				}
			}

			ValueDisplay timerDisplay;
			timerDisplay.index = (int16)action.value;
			timerDisplay.name = action.sender;
			_timerDisplays.push_back(timerDisplay);
		} else if (action.type == kActionSetText && (int)action.value == kPlayerClockTimer) {
			for (const Common::String &display : _clockDisplays) {
				if (display.equalsIgnoreCase(action.sender)) {
					return;
				}
			}

			_clockDisplays.push_back(action.sender);
			updatePlayerTime();
		}
	} else if (action.target.equalsIgnoreCase("Engine_ClosedCaption")) {
		if (action.type == kActionQuery) {
			takeAction(kActionSetValue, action.sender, ConfMan.getBool("subtitles") ? 1.0 : 0.0);
		} else if (action.type == kActionSetValue) {
			ConfMan.setBool("subtitles", action.value != 0);
			takeAction(action.value != 0 ? kActionShow : kActionHide, "CCText", action.value);
		}
	} else if (action.target.equalsIgnoreCase("Engine_Index")) {
		if (action.type == kActionSetText && g_nancy->getGameType() >= kGameTypeNancy19) {
			registerValueDisplay(action.text, action.sender);
		} else if (action.type == kActionSetText) {
			registerValueDisplay((int16)action.value, action.sender);
		} else if (action.type == kActionDisable) {
			for (uint i = 0; i < _valueDisplays.size(); ++i) {
				if (_valueDisplays[i].name.equalsIgnoreCase(action.sender)) {
					_valueDisplays.remove_at(i);
					--i;
				}
			}
		}
	} else if (action.target.equalsIgnoreCase("Engine_Inv")) {
		if (g_nancy->getGameType() >= kGameTypeNancy17) {
			// The buttons send their name; the panel behind them only takes a held item back
			if (action.type == kActionSetText) {
				onInventoryClicked((int16)action.value);
			} else if (action.type == kActionSetValue) {
				NancySceneState.addItemToInventory(NancySceneState.getHeldItem());
			}
		} else if (action.type == kActionSetValue) {
			onInventoryClicked((int16)action.value);
		}
	} else if (action.target.equalsIgnoreCase("Engine_Convo")) {
		// The player picked a response, by its place in the list
		if (action.type == kActionSetValue && _isInConversation) {
			NancySceneState.setLogicCondition((int16)action.value, g_nancy->_true);
		}
	} else if (action.target.equalsIgnoreCase("Engine_Journal")) {
		onJournalAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_Tasklist")) {
		onTasklistAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_Loader")) {
		onLoaderAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_Saver")) {
		onSaverAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_SaveQuery")) {
		onSaveQueryAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_GameWindow")) {
		onGameWindowAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_Matte")) {
		// Nancy17 gave the matte color a target of its own
		if (action.type == kActionSetValue || action.type == kActionQuery) {
			onGameWindowAction(action);
		}
	} else if (action.target.equalsIgnoreCase("Engine_CellPhone")) {
		onCellPhoneAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_VoiceVolume")) {
		onVolumeAction(action, "speech_volume", kSampleChannelVoice);
	} else if (action.target.equalsIgnoreCase("Engine_FXVolume")) {
		onVolumeAction(action, "sfx_volume", kSampleChannelFX);
	} else if (action.target.equalsIgnoreCase("Engine_MusicVolume")) {
		onVolumeAction(action, "music_volume", kSampleChannelMusic);
	} else if (action.target.equalsIgnoreCase("Engine_Flags")) {
		onFlagsAction(action);
	} else if (action.target.equalsIgnoreCase("Engine_Sound")) {
		onSoundAction(action);
	} else {
		debugC(3, kDebugEngine, "Unhandled NDUI engine action %d on \"%s\" from \"%s\"", action.type, action.target.c_str(), action.sender.c_str());
	}
}

// A click puts a held item back. Otherwise, the clicked item gets picked up, or
// opens the scene that shows it.
void Manager::onInventoryClicked(int16 itemID) {
	int16 heldItem = NancySceneState.getHeldItem();
	if (heldItem != -1) {
		NancySceneState.addItemToInventory(heldItem);
		return;
	}

	InvButton *button = findItemButton(itemID);
	if (!button) {
		return;
	}

	if (g_nancy->getGameType() >= kGameTypeNancy17) {
		onItemStreamClicked(*button);
		return;
	}

	SceneChangeDescription scene;
	scene.sceneID = button->getScene();
	scene.continueSceneSound = kContinueSceneSound;

	switch (button->getKind()) {
	case InvButton::kItemView:
		viewInventoryItem(itemID);
		break;
	case InvButton::kItemOpenCountedScene:
	case InvButton::kItemOpenScene:
		NancySceneState.pushScene();
		NancySceneState.changeScene(scene);
		break;
	default:
		NancySceneState.removeItemFromInventory(itemID, true);
		break;
	}
}

void Manager::viewInventoryItem(int16 itemID) {
	// Nancy17 does it the same way as for a click on the item
	if (g_nancy->getGameType() >= kGameTypeNancy17) {
		onInventoryClicked(itemID);
		return;
	}

	InvButton *button = findItemButton(itemID);
	if (!button || button->getKind() != InvButton::kItemView) {
		return;
	}

	SceneChangeDescription scene;
	scene.sceneID = button->getScene();
	scene.continueSceneSound = kContinueSceneSound;

	NancySceneState.viewInventoryItem(itemID);
	NancySceneState.changeScene(scene);
}

// Nancy17 shows the scene of an item in a stream of its own, on top of the scene
// the player is in. That scene ends its stream, and puts back an item that was
// taken out to be looked at.
void Manager::onItemStreamClicked(const InvButton &button) {
	const int16 itemID = button.getItemID();
	const Common::String streamName = "INVSTR_" + button.getName();
	const Common::String sceneName = Common::String::format("s%u", button.getScene());

	// On top of everything the scenes below draw
	static const int32 kItemStreamZOffset = 15;

	State::StreamFlags flags;
	flags.capturesInput = true;
	flags.runsFirst = true;
	flags.zOffset = kItemStreamZOffset;

	switch (button.getKind()) {
	case InvButton::kItemView: {
		// One item is looked at at a time
		const INVD *items = GetEngineData(INVD);
		for (const INVD::Item &item : items->items) {
			if (item.kind == InvButton::kItemView && item.id != itemID) {
				NancySceneState.getStreams().end("INVSTR_" + item.name);
			}
		}

		if (!NancySceneState.getStreams().isRunning(streamName)) {
			NancySceneState.getStreams().begin(streamName, sceneName, 0, flags);
			NancySceneState.removeItemFromInventory(itemID, false);
		}

		break;
	}
	case InvButton::kItemOpenCountedScene:
	case InvButton::kItemOpenScene:
		if (!NancySceneState.getStreams().isRunning(streamName)) {
			NancySceneState.getStreams().begin(streamName, sceneName, 0, flags);
		}

		break;
	default:
		NancySceneState.removeItemFromInventory(itemID, true);
		break;
	}
}

// The contacts are buttons made from a template, named after the contact
static const char *const kPhoneBookEntryPrefix = "PB:";

void Manager::showPhoneBookEntry(const PhoneBookData::Entry &entry, bool isNew) {
	const Common::String buttonName = kPhoneBookEntryPrefix + entry.name;

	if (isNew) {
		createObject(buttonName, "PhoneBookEntry", "CellPhoneDialog");
	}

	takeAction(kActionSetText, buttonName, entry.getDisplayText());
}

void Manager::createPhoneBook() {
	auto *data = (PhoneBookData *)NancySceneState.getPuzzleData(PhoneBookData::getTag());
	for (const PhoneBookData::Entry &entry : data->entries[g_nancy->getPlayerCharacter()]) {
		showPhoneBookEntry(entry, true);
	}
}

// Picking a contact calls them: the call is a scene in a stream of its own, and
// the phone can't be used until that scene enables it again
void Manager::onCellPhoneAction(const Action &action) {
	if (action.type != kActionSetValue) {
		return;
	}

	auto *data = (PhoneBookData *)NancySceneState.getPuzzleData(PhoneBookData::getTag());
	for (const PhoneBookData::Entry &entry : data->entries[g_nancy->getPlayerCharacter()]) {
		if (!action.text.equalsIgnoreCase(kPhoneBookEntryPrefix + entry.name)) {
			continue;
		}

		takeAction(kActionDisable, "CellPhoneDialog", 0.0);

		State::StreamFlags flags;
		flags.capturesInput = true;
		flags.runsFirst = true;
		NancySceneState.getStreams().begin("CellPhoneStream", entry.sceneName, 0, flags);
		return;
	}
}

// An item that was taken out to be looked at goes back once its scene is left
void Manager::onSceneChange(uint16 sceneID) {
	int16 viewedItem = NancySceneState.getPushedInvItemID();
	if (viewedItem == -1) {
		return;
	}

	InvButton *button = findItemButton(viewedItem);
	if (!button || button->getScene() != sceneID) {
		NancySceneState.returnViewedInventoryItem();
	}
}

// Nancy17 makes the buttons of the inventory out of the item table
void Manager::createItemButtons() {
	Panel *panel = findPanel("InvDialog");

	// Nancy19 makes them one at a time, with the script of the panel
	if (panel && g_nancy->getGameType() >= kGameTypeNancy19) {
		const INVD *items = GetEngineData(INVD);
		assert(items);

		for (const INVD::Item &item : items->items) {
			panel->addItemButton(item);
		}

		return;
	}

	if (panel && _templates.contains("InvButton")) {
		panel->addItemButtons(_templates["InvButton"]);
	}
}

void Manager::showInventoryItem(int16 itemID, bool show) {
	takeAction(show ? kActionShowItem : kActionHideItem, "InvDialog", (double)itemID);
}

void Manager::registerValueDisplay(int16 index, const Common::String &display) {
	ValueDisplay valueDisplay;
	valueDisplay.index = index;
	valueDisplay.name = display;
	_valueDisplays.push_back(valueDisplay);
}

// A control keeps asking for its value, and goes on showing the one it asked for last
void Manager::registerValueDisplay(const Common::String &valueName, const Common::String &display) {
	for (ValueDisplay &valueDisplay : _valueDisplays) {
		if (valueDisplay.name.equalsIgnoreCase(display)) {
			if (!valueDisplay.valueName.equalsIgnoreCase(valueName)) {
				valueDisplay.valueName = valueName;
				valueDisplay.hasShownValue = false;
			}

			return;
		}
	}

	ValueDisplay valueDisplay;
	valueDisplay.valueName = valueName;
	valueDisplay.name = display;
	_valueDisplays.push_back(valueDisplay);
}

// Value displays follow the value table, and timer displays what is left of their timer
void Manager::update() {
	if (g_nancy->getGameType() >= kGameTypeNancy19) {
		ScriptValueData *values = (ScriptValueData *)NancySceneState.getPuzzleData(ScriptValueData::getTag());
		for (ValueDisplay &display : _valueDisplays) {
			int32 value = values ? values->values.getValOrDefault(display.valueName, 0) : 0;
			if (!display.hasShownValue || display.shownValue != value) {
				display.hasShownValue = true;
				display.shownValue = value;
				takeAction(kActionSetValue, display.name, (double)value);
			}
		}

		return;
	}

	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (table) {
		for (ValueDisplay &display : _valueDisplays) {
			int16 value = table->getValue(display.index);
			if (!display.hasShownValue || display.shownValue != value) {
				display.hasShownValue = true;
				display.shownValue = value;
				takeAction(kActionSetValue, display.name, (double)value);
			}
		}
	}

	for (ValueDisplay &display : _timerDisplays) {
		int16 value = (int16)NancySceneState.getSoftwareTimerPercentLeft(display.index);
		if (!display.hasShownValue || display.shownValue != value) {
			display.hasShownValue = true;
			display.shownValue = value;
			takeAction(kActionSetValue, display.name, (double)value);
		}
	}
}

// The journal has one page per topic. The title of a topic is the text of its page's
// name, and picking a title in the list shows the page.
void Manager::addJournalEntry(const Common::String &pageName, const Common::String &textKey) {
	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	assert(autotext);

	Common::String title = autotext->texts.getValOrDefault(pageName);
	if (!_journalPages.contains(title)) {
		takeAction(kActionAddText, "JournalItems", title);
		_journalPages.setVal(title, pageName);
	}

	createObject(textKey, "JournalEntry", pageName);
	takeAction(kActionAddText, textKey, autotext->texts.getValOrDefault(textKey));
}

void Manager::onJournalAction(const Action &action) {
	if (action.type == kActionShow) {
		if (!_shownJournalPage.empty()) {
			takeAction(kActionShow, _shownJournalPage, 0.0);
		}
	} else if (action.type == kActionSetText) {
		if (!_journalPages.contains(action.text)) {
			return;
		}

		for (auto &page : _journalPages) {
			takeAction(kActionHide, page._value, 0.0);
		}

		_shownJournalPage = _journalPages[action.text];
		takeAction(kActionSetValue, _shownJournalPage, 0.0);
		takeAction(kActionShow, _shownJournalPage, 0.0);
	}
}

// Task list entries get checked off by the player, but only once the task is done,
// and they can't be unchecked
void Manager::addTasklistEntry(const Common::String &textKey, int16 flag) {
	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	assert(autotext);

	_tasklistFlags.setVal(textKey, flag);
	createObject(textKey, "TasklistEntry", "TasklistDialog");
	takeAction(kActionSetText, textKey, autotext->texts.getValOrDefault(textKey));

	TasklistData *tasklistData = (TasklistData *)NancySceneState.getPuzzleData(TasklistData::getTag());
	for (const Common::String &entry : tasklistData->checkedEntries) {
		if (entry.equalsIgnoreCase(textKey)) {
			takeAction(kActionSetValue, textKey, 1.0);
			break;
		}
	}
}

void Manager::onTasklistAction(const Action &action) {
	if (action.type == kActionHide) {
		playTasklistSound(0, false);
		playTasklistSound(1, false);
		return;
	}

	if (action.type != kActionSetValue) {
		return;
	}

	if (action.value == 0) {
		takeAction(kActionSetValue, action.sender, 1.0);
		return;
	}

	int16 flag = _tasklistFlags.getValOrDefault(action.sender, kEvNoEvent);
	if (flag != kEvNoEvent && NancySceneState.getEventFlag(flag, g_nancy->_false)) {
		takeAction(kActionSetValue, action.sender, 0.0);
	} else {
		TasklistData *tasklistData = (TasklistData *)NancySceneState.getPuzzleData(TasklistData::getTag());
		tasklistData->checkedEntries.push_back(action.sender);
	}

	playTasklistSound(0, true);
}

// Pauses and resumes the sound on a channel, e.g. the Geiger counter's clicking. Showing
// only resumes a channel the UI paused itself, and hiding only pauses one that is playing.
void Manager::onSoundAction(const Action &action) {
	uint16 channel = (uint16)action.value;
	int index = -1;
	for (uint i = 0; i < _pausedChannels.size(); ++i) {
		if (_pausedChannels[i] == channel) {
			index = i;
			break;
		}
	}

	bool pause;
	switch (action.type) {
	case kActionShow:
		pause = false;
		break;
	case kActionHide:
		pause = true;
		break;
	case kActionToggle:
		pause = index == -1;
		break;
	default:
		return;
	}

	if (pause) {
		if (index == -1 && g_nancy->_sound->isSoundPlaying(channel)) {
			g_nancy->_sound->pauseSound(channel, true);
			_pausedChannels.push_back(channel);
		}
	} else if (index != -1) {
		g_nancy->_sound->pauseSound(channel, false);
		_pausedChannels.remove_at(index);
	}
}

void Manager::playTasklistSound(uint bank, bool play) {
	const TSKL *tasklistSounds = GetEngineData(TSKL);
	if (!tasklistSounds) {
		return;
	}

	const TSKL::SoundBank &soundBank = tasklistSounds->soundBanks[bank];
	SoundDescription sound;
	sound.channelID = soundBank.channelID;

	if (!play) {
		g_nancy->_sound->stopSound(sound);
		return;
	}

	if (soundBank.soundNames.empty()) {
		return;
	}

	sound.name = soundBank.soundNames[g_nancy->_randomSource->getRandomNumber(soundBank.soundNames.size() - 1)].toString();
	sound.numLoops = soundBank.numLoops;
	sound.volume = soundBank.volume;
	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);
}

// The responses are made from a template, and picked by the order they are in.
// The inventory gets closed, and can't be opened while talking.
void Manager::startConversation(const Common::String &caption, const Common::Array<Common::String> &responses) {
	takeAction(kActionSetText, "ConvoCCText", caption);

	for (uint i = 0; i < responses.size(); ++i) {
		Common::String name = Common::String::format("ConvoResponse%u", i);
		createObject(name, "ConvoResponse", "ConvoDialog");
		takeAction(kActionSetText, name, responses[i]);
		_responseNames.push_back(name);
	}

	takeAction(kActionHide, "InvDialog", 0.0);
	takeAction(kActionDisable, "ShowInv", 0.0);
	_isInConversation = true;
	takeAction(kActionSetValue, "ConvoDialog", 0.0);
	takeAction(kActionShow, "ConvoDialog", 0.0);
}

void Manager::disableOtherResponses(uint pickedResponse) {
	for (uint i = 0; i < _responseNames.size(); ++i) {
		if (i != pickedResponse) {
			takeAction(kActionDisable, _responseNames[i], 0.0);
		}
	}
}

void Manager::endConversation() {
	if (!_isInConversation) {
		return;
	}

	_isInConversation = false;
	takeAction(kActionHide, "ConvoDialog", 0.0);
	takeAction(kActionEnable, "ShowInv", 0.0);

	for (const Common::String &name : _responseNames) {
		destroyObject(name);
	}

	_responseNames.clear();
	takeAction(kActionSetText, "ConvoCCText", Common::String());
}

void Manager::setCaption(const Common::String &text) {
	_caption = text;
	takeAction(kActionSetText, "CCText", text);
}

// The time of day is shown as "1:05 pm", grayed out at night
void Manager::updatePlayerTime() {
	const BSUM *bootSummary = GetEngineData(BSUM);
	assert(bootSummary);

	const Time playerTime = NancySceneState.getPlayerTime();
	uint32 minutesOfDay = playerTime.getHours() * 60 + playerTime.getMinutes();
	bool isDaytime = minutesOfDay >= (uint32)bootSummary->dayStartMinutes && minutesOfDay <= (uint32)bootSummary->dayEndMinutes;
	uint32 hours = minutesOfDay / 60;
	uint32 displayHours = hours % 12 == 0 ? 12 : hours % 12;
	Common::String text = Common::String::format("%u:%02u %s", displayHours, minutesOfDay % 60, hours < 12 ? "am" : "pm");

	for (const Common::String &display : _clockDisplays) {
		takeAction(kActionSetText, display, text);
		takeAction(isDaytime ? kActionEnable : kActionDisable, display, 0.0);
	}
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
