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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/miscrecords.h"

#include "engines/nancy/state/scene.h"

#include "common/events.h"
#include "common/config-manager.h"
#include "nancy/ui/taskbar.h"

namespace Nancy {
namespace Action {

void PaletteThisScene::readData(Common::SeekableReadStream &stream) {
	_paletteID = stream.readByte();
	_unknownEnum = stream.readByte();
	_paletteStart = stream.readUint16LE();
	_paletteSize = stream.readUint16LE();
}

void PaletteThisScene::execute() {
	NancySceneState.getSceneInfo().paletteID = _paletteID;
	const State::Scene::SceneSummary &ssum = NancySceneState.getSceneSummary();

	if (_unknownEnum > 1 && _unknownEnum < 3) {
		// Not sure what the difference is between the 3 types
		NancySceneState.getViewport().setPalette(ssum.palettes[_paletteID], _paletteStart, _paletteSize);
	} else {
		NancySceneState.getViewport().setPalette(ssum.palettes[_paletteID]);
	}

	finishExecution();
}

void PaletteNextScene::readData(Common::SeekableReadStream &stream) {
	// Structure is the same as PaletteThisScene, but the original engine only uses the palette ID
	_paletteID = stream.readByte();
	stream.skip(5);
}

void PaletteNextScene::execute() {
	NancySceneState.getNextSceneInfo().paletteID = _paletteID;
	_isDone = true;
}

void LightningOn::readData(Common::SeekableReadStream &stream) {
	_distance = stream.readSint16LE();
	_pulseTime = stream.readUint16LE();
	_rgbPercent = stream.readSint16LE();
	stream.skip(4);
}

void SpecialEffect::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() <= kGameTypeNancy6) {
		_type = stream.readByte();
		_fadeToBlackTime = stream.readUint16LE();
		_frameTime = stream.readUint16LE();
	} else {
		_type = stream.readByte();
		_totalTime = stream.readUint16LE();
		_fadeToBlackTime = stream.readUint16LE();
		readRect(stream, _rect);
	}
}

void SpecialEffect::execute() {
	if (g_nancy->getGameType() <= kGameTypeNancy6) {
		NancySceneState.specialEffect(_type, _fadeToBlackTime, _frameTime);
	} else {
		NancySceneState.specialEffect(_type, _totalTime, _fadeToBlackTime, _rect);
	}

	_isDone = true;
}

void LightningOn::execute() {
	NancySceneState.beginLightning(_distance, _pulseTime, _rgbPercent);
	_isDone = true;
}

// Reads a textbox caption: an int16 length, then either a CVTX (AUTOTEXT) key
// to resolve (length == -1) or that many bytes of inline text. Shared by the
// textbox-writing action records.
static void readTextboxText(Common::SeekableReadStream &stream, Common::String &out) {
	const int16 size = stream.readSint16LE();

	if (size > 10000) {
		error("Textbox write action record has too many text box chars: %d", size);
	}

	if (size == -1) {
		Common::String stringID;
		readFilename(stream, stringID);

		const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
		assert(autotext);

		out = autotext->texts.getValOrDefault(stringID, "");
	} else if (size > 0) {
		char *buf = new char[size];
		stream.read(buf, size);
		buf[size - 1] = '\0';

		assembleTextLine(buf, out, size);

		delete[] buf;
	}
}

void TextBoxWrite::readData(Common::SeekableReadStream &stream) {
	if (_isAutotext) {
		// AR 81 prefixes the body with a wait header (runtime state at 0x00 is
		// always 0 in the data, so skip it)
		stream.skip(2);
		_waitMode = stream.readSint16LE();
		_soundChannel = stream.readUint16LE();
		_waitTimeMs = stream.readUint32LE();
	}

	readTextboxText(stream, _text);

	if (_isAutotext) {
		// The original terminates the body with the "<e>" end-of-line hypertext tag
		_text += "<e>";
	}
}

void TextBoxWrite::execute() {
	if (_state == kBegin) {
		auto &tb = NancySceneState.getTextbox();
		tb.clear();
		if (!_text.empty()) {
			tb.addTextLine(_text);
		}
		tb.setVisible(true);

		if (!_isAutotext) {
			// Plain TextBoxWrite completes immediately
			finishExecution();
			return;
		}

		if (_waitMode == kWaitForTimer) {
			_endTime = g_nancy->getTotalPlayTime() + _waitTimeMs;
		}

		_state = kRun;
		return;
	}

	// Nancy 11+ AR 81 waits for a sound or a timer before completing
	switch (_state) {
	case kRun:
		switch (_waitMode) {
		case kWaitForSound:
			if (!g_nancy->_sound->isSoundPlaying(_soundChannel)) {
				_state = kActionTrigger;
			}
			break;
		case kWaitForTimer:
			if (g_nancy->getTotalPlayTime() >= _endTime) {
				_state = kActionTrigger;
			}
			break;
		default:
			_state = kActionTrigger;
			break;
		}
		break;
	case kActionTrigger:
		finishExecution();
		break;
	default:
		break;
	}
}

void TextboxClear::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void TextboxClear::execute() {
	NancySceneState.getTextbox().clear();
	finishExecution();
}

void FrameTextBox::readData(Common::SeekableReadStream &stream) {
	readTextboxText(stream, _text);

	// The original appends the "<e>" end-of-line hypertext tag to the caption
	_text += "<e>";
}

void FrameTextBox::execute() {
	auto &tb = NancySceneState.getTextbox();
	tb.clear();
	if (!_text.empty())
		tb.addTextLine(_text);

	tb.setFullMode(_fullMode);
	finishExecution();
}

void ControlUIItems::readData(Common::SeekableReadStream &stream) {
	_uiButton = stream.readUint16LE();
	_autoOpenOrBadgeSound = stream.readByte();
	_flagB  = stream.readByte();
	_startScene = stream.readSint16LE();
	_endScene = stream.readSint16LE();
}

void ControlUIItems::execute() {
	// Value 1 auto-opens the popup selected by _uiButton. For the cell
	// phone, _startScene (when set) is the scene to jump to once it opens,
	// which places a call that starts a conversation there.
	if (_autoOpenOrBadgeSound == 1) {
		switch (_uiButton) {
		case kUITypeInventory:
			NancySceneState.getInventoryPopup().open();
			break;
		case kUITypeNotebook:
			NancySceneState.getNotebookPopup().open();
			break;
		case kUITypeCellphone: {
			UI::CellPhonePopup &phone = NancySceneState.getCellPhonePopup();

			if (_startScene != (int16)kNoScene) {
				SceneChangeDescription scene;
				scene.sceneID = _startScene;
				scene.frameID = 0;
				scene.verticalOffset = 0;
				// The destination scene's sound carries the conversation audio.
				scene.continueSceneSound = kLoadSceneSound;
				// Phone rings, picks up, and changeScenes into `scene`.
				phone.startIncomingCall(scene);
			} else {
				phone.open();
			}
			break;
		}
		default:
			break;
		}
	} else {
		// Otherwise this AR toggles whether the button is disabled while the
		// player is in scene range [_startScene, _endScene]. _flagB != 0 sets
		// the toggle (a _startScene of 9997 means "from scene 0", with the
		// range capped at 9997); _flagB == 0 clears it once a bound is 9999
		// (kNoScene). _flagB != 0 also sets the disabled button's rejection-sound
		// mode from _autoOpenOrBadgeSound (which clickSoundName line plays when
		// the button is clicked while its popup is unavailable).
		UI::Taskbar *taskbar = NancySceneState.getTaskbar();
		if (taskbar) {
			if (_flagB != 0) {
				int16 start = _startScene;
				int16 end = _endScene;
				if (_startScene == 9997) {
					start = 0;
					end = 9997;
				}
				taskbar->setDisabledRange(_uiButton, start, end);
				taskbar->setClickSoundMode(_uiButton, _autoOpenOrBadgeSound);
			} else if (_startScene == (int16)kNoScene || _endScene == (int16)kNoScene) {
				taskbar->clearButtonOverride(_uiButton);
			}
		}
	}

	finishExecution();
}

void UIPopupPrepScene::readData(Common::SeekableReadStream &stream) {
	_uiType      = stream.readSint32LE();
	_signalValue = stream.readSint32LE();
}

void UIPopupPrepScene::execute() {
	// Terminates a UI prep scene chain: the entry-adding ARs in the prep
	// scene(s) have run, so signal the scene to restore the pre-open scene and
	// open the (now populated) popup. A no-op if no prep is currently running.
	NancySceneState.finishUIPrepScene();

	finishExecution();
}

void AddSearchLink::readData(Common::SeekableReadStream &stream) {
	_mode = stream.readSint16LE();

	readFilename(stream, _key);
	readFilename(stream, _value);

	_extra  = stream.readSint16LE();
	_flag = stream.readSint16LE();
	_eventFlag = stream.readSint16LE();
}

void AddSearchLink::execute() {
	NancySceneState.getCellPhonePopup().addSearchLink(
		_mode, _key, _value, _extra, _flag, _eventFlag);

	// Cellphone taskbar badge: mode 0 = new email (sub-cat 1), mode != 0
	// = new web search topic (sub-cat 2).
	if (UI::Taskbar *taskbar = NancySceneState.getTaskbar()) {
		taskbar->setNotification(kTaskButtonCellphone, _mode == 0 ? 1 : 2);
	}

	finishExecution();
}

void SetCellPhoneBatteryAndSignal::readData(Common::SeekableReadStream &stream) {
	_mode = stream.readUint16LE();
}

void SetCellPhoneBatteryAndSignal::execute() {
	UI::CellPhonePopup &popup = NancySceneState.getCellPhonePopup();
	switch (_mode) {
	case 0: popup.setBatteryLow(false); break;
	case 1: popup.setBatteryLow(true);  break;
	case 2: popup.setNoSignal(false);   break;
	case 3: popup.setNoSignal(true);    break;
	default:
		warning("SetCellPhoneBatteryAndSignal: unknown mode %u", _mode);
		break;
	}
	finishExecution();
}

void ChangeCellPhoneInfo::readData(Common::SeekableReadStream &stream) {
	stream.read(_contact.unknownPrefix, sizeof(_contact.unknownPrefix));

	char nameBuf[21];
	stream.read(nameBuf, 20);
	nameBuf[20] = '\0';
	_contact.name = nameBuf;

	stream.read(_contact.unknownSuffix, sizeof(_contact.unknownSuffix));
}

void ChangeCellPhoneInfo::execute() {
	NancySceneState.getCellPhonePopup().upsertContact(_contact);

	// Cellphone taskbar badge: a new/updated contact triggers sub-cat 0.
	if (UI::Taskbar *taskbar = NancySceneState.getTaskbar()) {
		taskbar->setNotification(kTaskButtonCellphone, 0);
	}

	finishExecution();
}

void CellPhonePopCellSceneFromStack::readData(Common::SeekableReadStream &stream) {
	_sceneChange.sceneID = stream.readUint16LE();
}

void CellPhonePopCellSceneFromStack::execute() {
	UI::CellPhonePopup &phone = NancySceneState.getCellPhonePopup();

	if (_sceneChange.sceneID != kNoScene) {
		NancySceneState.changeScene(_sceneChange);
	} else {
		// Restore the pre-call scene if one was saved. If there's no saved
		// scene (e.g. the conversation was entered without a phone call),
		// do nothing — popping the global scene stack here would clobber
		// closeup / inventory pushes that have nothing to do with the phone.
		SceneChangeDescription returnScene;
		if (phone.consumeReturnScene(returnScene))
			NancySceneState.changeScene(returnScene);
	}

	// Conversation is over. An incoming call closes the phone; a player-placed
	// call leaves it open at the welcome screen.
	phone.endCall();

	finishExecution();
}

void BumpPlayerClock::readData(Common::SeekableReadStream &stream) {
	_relative = stream.readByte();
	_hours = stream.readUint16LE();
	_minutes = stream.readUint16LE();
}

void BumpPlayerClock::execute() {
	NancySceneState.setPlayerTime(_hours * 3600000 + _minutes * 60000, _relative);
	finishExecution();
}

void SaveContinueGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void SaveContinueGame::execute() {
	g_nancy->secondChance();
	_isDone = true;
}

void TurnOffMainRendering::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void TurnOnMainRendering::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

// Returns the Nancy 11+ software-timer slot at the given index, lazily creating
// the TimerData puzzle-data chunk. nullptr if the index is out of range.
static TimerData::Timer *getSoftwareTimer(int16 index) {
	if (index < 0 || (uint)index >= TimerData::kNumTimers) {
		return nullptr;
	}

	TimerData *timerData = (TimerData *)NancySceneState.getPuzzleData(TimerData::getTag());
	return timerData ? &timerData->timers[index] : nullptr;
}

void ResetAndStartTimer::readData(Common::SeekableReadStream &stream) {
	_timerIndex = stream.readByte();
}

void ResetAndStartTimer::execute() {
	NancySceneState.resetAndStartTimer();

	// Nancy 11 also resets and starts one of the software-timer slots
	if (g_nancy->getGameType() >= kGameTypeNancy11) {
		TimerData::Timer *timer = getSoftwareTimer(_timerIndex);
		if (timer) {
			timer->reset();
			timer->state = TimerData::Timer::kRunning;
		}
	}

	_isDone = true;
}

void StopTimer::readData(Common::SeekableReadStream &stream) {
	_timerIndex = stream.readByte();
}

void StopTimer::execute() {
	NancySceneState.stopTimer();

	if (g_nancy->getGameType() >= kGameTypeNancy11) {
		TimerData::Timer *timer = getSoftwareTimer(_timerIndex);
		if (timer) {
			timer->reset();
		}
	}

	_isDone = true;
}

// Reads exactly size bytes from the stream, returning the text up to the first
// null byte. Used for the fixed-size string fields inside a TimerControl chunk.
static Common::String readFixedSizeString(Common::SeekableReadStream &stream, uint size) {
	Common::String result;
	bool ended = false;
	for (uint i = 0; i < size; ++i) {
		byte b = stream.readByte();
		if (b == 0) {
			ended = true;
		}
		if (!ended) {
			result += (char)b;
		}
	}

	return result;
}

void TimerControl::readData(Common::SeekableReadStream &stream) {
	const int64 startPos = stream.pos();

	_timerIndex = stream.readSint16LE();    // 0x00
	_command = stream.readSint16LE();       // 0x02
	_hours = stream.readSint16LE();         // 0x04
	_minutes = stream.readSint16LE();       // 0x06
	_seconds = stream.readSint16LE();       // 0x08
	stream.skip(2);                         // 0x0a, unused

	// Sound to play on expiry (0x0c)
	_sound.readDIGI(stream);

	// Autotext key for the caption (0x3d, 33 bytes)
	stream.skip((startPos + 0x3d) - stream.pos());
	_autotextKey = readFixedSizeString(stream, 0x21);

	// Inline caption text (0x5e, 100 bytes)
	_caption = readFixedSizeString(stream, 0x64);

	// Event flags to fire on expiry (count at 0xc2, then count*4 bytes)
	uint16 numFlags = stream.readUint16LE();
	_flags.resize(numFlags);
	for (uint i = 0; i < numFlags; ++i) {
		_flags[i].label = stream.readSint16LE();
		_flags[i].flag = (byte)stream.readSint16LE();
	}
}

void TimerControl::execute() {
	TimerData::Timer *timer = getSoftwareTimer(_timerIndex);
	if (!timer) {
		_isDone = true;
		return;
	}

	const uint32 durationMs = ((uint32)_hours * 3600 + (uint32)_minutes * 60 + (uint32)_seconds) * 1000;

	switch (_command) {
	case kReset:
		timer->reset();
		break;
	case kStart:
		timer->state = TimerData::Timer::kRunning;
		break;
	case kPause:
		timer->state = TimerData::Timer::kPaused;
		break;
	case kAddTime:
		if (timer->state != TimerData::Timer::kIdle) {
			timer->currentTimeMs += durationMs;
		}
		break;
	case kSubtractTime:
		if (timer->state != TimerData::Timer::kIdle) {
			timer->currentTimeMs = timer->currentTimeMs > durationMs ? timer->currentTimeMs - durationMs : 0;
		}
		break;
	case kConfigOneShot:
	case kConfigRepeating:
		// A timer can only be configured once it has been started
		if (timer->state == TimerData::Timer::kRunning) {
			timer->durationMs = durationMs;
			timer->hasFired = false;
			timer->sound = _sound;
			timer->autotextKey = _autotextKey;
			timer->caption = _caption;

			for (uint i = 0; i < ARRAYSIZE(timer->flags); ++i) {
				timer->flags[i] = i < _flags.size() ? _flags[i] : FlagDescription();
			}

			timer->state = _command;
		}
		break;
	default:
		break;
	}

	_isDone = true;
}

void StopPlayerScrolling::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void StopPlayerScrolling::execute() {
	NancySceneState.setPlayerScrolling(false);
	_isDone = true;
}

void StartPlayerScrolling::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void StartPlayerScrolling::execute() {
	NancySceneState.setPlayerScrolling(true);
	_isDone = true;
}

void GotoMenu::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void GotoMenu::execute() {
	//NancySceneState.setDestroyOnExit();
	g_nancy->setState(NancyState::kMainMenu);

	_isDone = true;
}

void LoseGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void LoseGame::execute() {
	g_nancy->_sound->stopAndUnloadSceneSpecificSounds();
	NancySceneState.setDestroyOnExit();

	if (!ConfMan.hasKey("original_menus") || ConfMan.getBool("original_menus")) {
		g_nancy->setState(NancyState::kMainMenu);
	} else {
		Common::Event ev;
		ev.type = Common::EVENT_RETURN_TO_LAUNCHER;
		g_system->getEventManager()->pushEvent(ev);
	}

	_isDone = true;
}

void PushScene::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void PushScene::execute() {
	NancySceneState.pushScene();
	_isDone = true;
}

void PopScene::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void PopScene::execute() {
	NancySceneState.popScene();
	_isDone = true;
}

void WinGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void WinGame::execute() {
	// Set ConfMan value that will stay persistent across future playthroughs.
	// Default value in original is StillWorkingOnIt, but we just don't set it instead.
	ConfMan.set("PlayerWonTheGame", "AcedTheGame", ConfMan.getActiveDomainName());
	ConfMan.flushToDisk();

	g_nancy->_sound->stopAndUnloadSceneSpecificSounds();
	NancySceneState.setDestroyOnExit();
	g_nancy->setState(NancyState::kCredits, NancyState::kMainMenu);

	_isDone = true;
}

void HintSystem::readData(Common::SeekableReadStream &stream) {
	_characterID = stream.readByte();
	_genericSound.readNormal(stream);
}

void HintSystem::execute() {
	switch (_state) {
	case kBegin: {
		uint16 difficulty = NancySceneState.getDifficulty();
		selectHint();
		_genericSound.name = selectedHint->soundIDs[difficulty];

		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(g_nancy->getStaticData().hintTexts[selectedHint->textID * 3 + difficulty]);

		g_nancy->_sound->loadSound(_genericSound);
		g_nancy->_sound->playSound(_genericSound);
		_state = kRun;
		break;
	}
	case kRun:
		if (!g_nancy->_sound->isSoundPlaying(_genericSound)) {
			g_nancy->_sound->stopSound(_genericSound);
			_state = kActionTrigger;
		} else {
			break;
		}

		// fall through
	case kActionTrigger:
		NancySceneState.useHint(_characterID, _hintID);
		NancySceneState.getTextbox().clear();

		NancySceneState.changeScene(selectedHint->sceneChange);

		_isDone = true;
		break;
	}
}

void HintSystem::selectHint() {
	if (NancySceneState.getHintsRemaining() == 0) {
		selectedHint = &g_nancy->getStaticData().hints[_characterID][0];
	}

	// Start from 1 since the first hint is always the "I give up" option
	for (uint i = 1; i < g_nancy->getStaticData().hints[_characterID].size(); ++i) {
		const auto &hint = g_nancy->getStaticData().hints[_characterID][i];

		bool isSatisfied = true;

		for (const auto &cond : hint.conditions) {
			switch (cond.type) {
			case (byte)StaticDataConditionType::kEvent :
				if (!NancySceneState.getEventFlag(cond.label, cond.flag)) {
					isSatisfied = false;
				}

				break;
			case (byte)StaticDataConditionType::kInventory :
				if (NancySceneState.hasItem(cond.label) != cond.flag) {
					isSatisfied = false;
				}

				break;
			case (byte)StaticDataConditionType::kDifficulty :
				if (	(NancySceneState.getDifficulty() != cond.label && cond.flag != 0) ||
						(NancySceneState.getDifficulty() == cond.label && cond.flag == 0) ) {
					isSatisfied = false;
				}

				break;
			}

			if (!isSatisfied) {
				break;
			}
		}

		if (isSatisfied) {
			selectedHint = &hint;
			break;
		}
	}
}

void ResourceUse::readData(Common::SeekableReadStream &stream) {
	_resourceIndex = stream.readSint16LE();   // which UIRC resource to change
	_amount = stream.readSint16LE();           // value / delta
	_mode = stream.readByte();                 // 0 = set, non-zero = add
	_flag.label = stream.readSint16LE();       // event flag set on success
	_flag.flag = stream.readByte();
	// The rest of the 113-byte block (success/fail sounds, an optional scene
	// change at +0x5b, and a transient "UIResource_Overlay" sprite) isn't
	// handled yet.
	stream.skip(0x71 - 8);
}

void ResourceUse::execute() {
	if (_mode == 0) {
		// Set the resource outright.
		NancySceneState.setUIResource(_resourceIndex, _amount);
		NancySceneState.setEventFlag(_flag);
	} else {
		// Add the (signed) amount, but never let the resource go negative —
		// the original skips the change (e.g. when Nancy can't afford it).
		const int32 result = NancySceneState.getUIResource(_resourceIndex) + _amount;
		if (result >= 0) {
			NancySceneState.setUIResource(_resourceIndex, result);
			NancySceneState.setEventFlag(_flag);
		}
	}

	_isDone = true;
}

} // End of namespace Action
} // End of namespace Nancy
