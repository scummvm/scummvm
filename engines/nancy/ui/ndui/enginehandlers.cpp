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
		if (action.type == kActionSetText && (int)action.value == kPlayerClockTimer) {
			for (const Common::String &display : _clockDisplays) {
				if (display.equalsIgnoreCase(action.sender)) {
					return;
				}
			}

			_clockDisplays.push_back(action.sender);
		}
	} else if (action.target.equalsIgnoreCase("Engine_ClosedCaption")) {
		if (action.type == kActionQuery) {
			takeAction(kActionSetValue, action.sender, ConfMan.getBool("subtitles") ? 1.0 : 0.0);
		} else if (action.type == kActionSetValue) {
			ConfMan.setBool("subtitles", action.value != 0);
			takeAction(action.value != 0 ? kActionShow : kActionHide, "CCText", action.value);
		}
	} else if (action.target.equalsIgnoreCase("Engine_Index")) {
		if (action.type == kActionSetText) {
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
		if (action.type == kActionSetValue) {
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
	} else if (action.target.equalsIgnoreCase("Engine_VoiceVolume")) {
		onVolumeAction(action, "speech_volume");
	} else if (action.target.equalsIgnoreCase("Engine_FXVolume")) {
		onVolumeAction(action, "sfx_volume");
	} else if (action.target.equalsIgnoreCase("Engine_MusicVolume")) {
		onVolumeAction(action, "music_volume");
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

	SceneChangeDescription scene;
	scene.sceneID = button->getScene();
	scene.continueSceneSound = kContinueSceneSound;

	switch (button->getKind()) {
	case InvButton::kItemView:
		NancySceneState.viewInventoryItem(itemID);
		NancySceneState.changeScene(scene);
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

void Manager::showInventoryItem(int16 itemID, bool show) {
	takeAction(show ? kActionShowItem : kActionHideItem, "InvDialog", (double)itemID);
}

void Manager::registerValueDisplay(int16 index, const Common::String &display) {
	ValueDisplay valueDisplay;
	valueDisplay.index = index;
	valueDisplay.name = display;
	_valueDisplays.push_back(valueDisplay);
}

// Value displays follow the value table
void Manager::update() {
	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (!table) {
		return;
	}

	for (ValueDisplay &display : _valueDisplays) {
		int16 value = table->getValue(display.index);
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
	takeAction(kActionSetText, "CCText", text);
}

// The time of day is shown as "1:05 pm", grayed out at night
void Manager::setPlayerTime(uint32 minutesOfDay, bool isDaytime) {
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
