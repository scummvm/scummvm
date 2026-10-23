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
#include "engines/nancy/graphics.h"
#include "engines/nancy/font.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/miscrecords.h"
#include "engines/nancy/misc/viewportfader.h"
#include "engines/nancy/ui/ndui/manager.h"

#include "engines/nancy/state/scene.h"

#include "common/events.h"
#include "common/config-manager.h"
#include "common/random.h"
#include "common/file.h"

#include "image/png.h"

#include "nancy/ui/taskbar/taskbar.h"

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

		// The spoken lines have their captions with the conversation texts
		const CVTX *convo = (const CVTX *)g_nancy->getEngineData("CONVO");
		if (out.empty() && convo) {
			out = convo->texts.getValOrDefault(stringID, "");
		}
	} else if (size > 0) {
		char *buf = new char[size];
		stream.read(buf, size);
		buf[size - 1] = '\0';

		assembleTextLine(buf, out, size);

		delete[] buf;
	}
}

void TextBoxWrite::readData(Common::SeekableReadStream &stream) {
	if (_writeType == kAutotextWrite) {
		// AR 81 prefixes the body with a wait header (runtime state at 0x00 is
		// always 0 in the data, so skip it)
		stream.skip(2);
		_waitMode = stream.readSint16LE();
		_soundChannel = stream.readUint16LE();
		_waitTimeMs = stream.readUint32LE();
	}

	readTextboxText(stream, _text);

	if (_writeType == kAutotextWrite) {
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

		if (_writeType == kTextBoxWrite) {
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

	tb.setFullMode(_boxMode == kFullBox);
	finishExecution();
}

void ControlUIItems::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() >= kGameTypeNancy15) {
		_uiButton = stream.readByte();
		_characterIndex = stream.readByte();
	} else {
		_uiButton = stream.readUint16LE();
	}
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
		// Only one popup shows at a time. If the player left one open (e.g. the
		// inventory), close it before this AR auto-opens its target, so the
		// game-over incoming call doesn't stack the phone over another popup.
		NancySceneState.closeActivePopups();

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
		// Nancy15 keeps separate overrides per player character. Only the
		// active character's taskbar is modeled, so skip the others'.
		uint characterIndex = _characterIndex == kPlayerCharacterActive ?
			g_nancy->getPlayerCharacter() : _characterIndex;
		UI::Taskbar *taskbar = NancySceneState.getTaskbar();
		if (taskbar && characterIndex == g_nancy->getPlayerCharacter()) {
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

	readFilename(stream, _link.key);
	readFilename(stream, _link.value);

	_link.extra = stream.readSint16LE();
	_link.flag = stream.readSint16LE();
	_link.eventFlag = stream.readSint16LE();
}

void AddSearchLink::execute() {
	NancySceneState.getCellPhonePopup().addSearchLink(_mode, _link);

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
	readContact(stream, _contact);
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
	_hours = stream.readSint16LE();
	_minutes = stream.readSint16LE();

	if (g_nancy->getGameType() >= kGameTypeNancy16) {
		// Value table indices to take the hours and minutes from instead,
		// never used in the game data
		stream.skip(4);
	}
}

void BumpPlayerClock::execute() {
	NancySceneState.setPlayerTime((uint32)(_hours * 3600000 + _minutes * 60000), _relative);
	finishExecution();
}

void Fader::readData(Common::SeekableReadStream &stream) {
	stream.skip(4);	// fade time, unused: the fade speed is fixed
	_holdTime = stream.readUint32LE();
	_color = stream.readUint32BE() & 0xFFFFFF;
}

void Fader::execute() {
	Misc::ViewportFader *fader = NancySceneState.getViewportFader();
	assert(fader);

	if (fader->isIdle()) {
		fader->arm(_color, _holdTime);
		finishExecution();
	}
}

void SaveContinueGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void SaveContinueGame::execute() {
	g_nancy->secondChance();
	_isDone = true;
}

void MakeScreenFile::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _filename);

	readRect(stream, _cropRect);

	if (g_nancy->getGameType() <= kGameTypeNancy15) {
		// Image format selector, unused
		stream.skip(1);
	}
}

void MakeScreenFile::execute() {
	Graphics::ManagedSurface screenshot;
	g_nancy->_graphics->screenshotScreen(screenshot);

	Common::Rect cropRect = _cropRect;

	if (cropRect.isValidRect()) {
		cropRect.clip(Common::Rect(screenshot.w, screenshot.h));

		if (!cropRect.isEmpty()) {
			Common::Path outName(_filename + ".png");
			Common::DumpFile outFile;

			if (outFile.open(outName)) {
				if (!Image::writePNG(outFile, screenshot.getSubArea(cropRect))) {
					warning("Could not write screen file %s", outName.toString().c_str());
				}

				outFile.finalize();
				outFile.close();
			} else {
				warning("Could not create screen file %s", outName.toString().c_str());
			}
		}
	}

	finishExecution();
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

// Nancy 12+: adds a trigger to a running timer, unless an identical one already
// exists (so re-running the record on scene re-entry doesn't stack duplicates)
// or the timer's kNumTriggers slots are full.
static void addTimerTrigger(TimerData::Timer &timer, TimerData::Trigger::Type type,
		uint32 durationMs, const SoundDescription &sound, const Common::Array<FlagDescription> &flags) {
	for (uint i = 0; i < timer.triggers.size(); ++i) {
		const TimerData::Trigger &t = timer.triggers[i];
		if (t.type != type || t.durationMs != durationMs ||
			t.sound.name != sound.name || t.sound.channelID != sound.channelID ||
			t.sound.volume != sound.volume) {
			continue;
		}

		bool flagsMatch = true;
		for (uint j = 0; j < ARRAYSIZE(t.flags); ++j) {
			FlagDescription incoming = j < flags.size() ? flags[j] : FlagDescription();
			if (t.flags[j].label != incoming.label || t.flags[j].flag != incoming.flag) {
				flagsMatch = false;
				break;
			}
		}

		if (flagsMatch) {
			return;
		}
	}

	if (timer.triggers.size() >= TimerData::kNumTriggers) {
		return;
	}

	TimerData::Trigger trigger;
	trigger.type = type;
	trigger.durationMs = durationMs;
	trigger.hasFired = false;
	trigger.sound = sound;
	for (uint i = 0; i < ARRAYSIZE(trigger.flags); ++i) {
		trigger.flags[i] = i < flags.size() ? flags[i] : FlagDescription();
	}

	timer.triggers.push_back(trigger);
}

// Nancy 12-13 add to, subtract from or set the elapsed time's hours, minutes and
// seconds separately (clamping each at 0 when subtracting), dropping any
// fraction of a second
static void adjustTimerComponents(TimerData::Timer &timer, int16 command, int16 hours, int16 minutes, int16 seconds) {
	int32 h = timer.currentTimeMs / 3600000;
	int32 m = (timer.currentTimeMs / 60000) % 60;
	int32 s = (timer.currentTimeMs / 1000) % 60;

	switch (command) {
	case ResetAndStartTimer::kAddTime:
		h += hours;
		m += minutes;
		s += seconds;
		break;
	case ResetAndStartTimer::kSubtractTime:
		h = MAX<int32>(h - hours, 0);
		m = MAX<int32>(m - minutes, 0);
		s = MAX<int32>(s - seconds, 0);
		break;
	case ResetAndStartTimer::kSetTime:
		h = hours;
		m = minutes;
		s = seconds;
		break;
	default:
		return;
	}

	timer.currentTimeMs = MAX<int32>((h * 60 + m) * 60 + s, 0) * 1000;
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

void ResetAndStartTimer::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() < kGameTypeNancy12) {
		_timerIndex = stream.readByte();
		return;
	}

	_timerIndex = stream.readSint16LE();    // 0x00
	_command = stream.readSint16LE();        // 0x02

	switch (_command) {
	case kAddTime:
	case kSubtractTime:
	case kSetTime:
		_hours = stream.readSint16LE();          // 0x04
		_minutes = stream.readSint16LE();        // 0x06
		_seconds = stream.readSint16LE();        // 0x08
		_milliseconds = stream.readSint16LE();   // 0x0a
		break;
	case kConfigOneShot:
	case kConfigRepeating: {
		_hours = stream.readSint16LE();          // 0x04
		_minutes = stream.readSint16LE();        // 0x06
		_seconds = stream.readSint16LE();        // 0x08
		_milliseconds = stream.readSint16LE();   // 0x0a

		Common::Array<Common::String> names;
		if (g_nancy->getGameType() >= kGameTypeNancy16) {
			// Up to three expiry-sound names, preceded by their count and
			// followed by the sound settings
			uint16 numNames = stream.readUint16LE();
			for (uint i = 0; i < numNames; ++i) {
				Common::String name = readFixedSizeString(stream, 0x21);
				if (!name.empty() && !name.equalsIgnoreCase("NO SOUND")) {
					names.push_back(name);
				}
			}

			_sound.channelID = stream.readUint16LE();
			_sound.numLoops = stream.readUint32LE();
			_sound.volume = stream.readUint16LE();
		} else {
			_sound.volume = stream.readUint16LE();   // 0x0c
			_sound.channelID = stream.readUint16LE(); // 0x0e
			_sound.numLoops = 1;

			// Three candidate expiry-sound names (0x10, 33 bytes each); one is
			// picked at random, "NO SOUND" marks an empty slot
			for (uint i = 0; i < 3; ++i) {
				Common::String name = readFixedSizeString(stream, 0x21);
				if (!name.empty() && !name.equalsIgnoreCase("NO SOUND")) {
					names.push_back(name);
				}
			}
		}

		if (!names.empty()) {
			_sound.name = names[g_nancy->_randomSource->getRandomNumber(names.size() - 1)];
		}

		// Event flags fired on expiry (count at 0x73)
		uint16 numFlags = stream.readUint16LE();
		_flags.resize(numFlags);
		for (uint i = 0; i < numFlags; ++i) {
			_flags[i].label = stream.readSint16LE();
			_flags[i].flag = (byte)stream.readSint16LE();
		}
		break;
	}
	default:
		// kStart, kClear and kPause carry no further data
		break;
	}
}

void ResetAndStartTimer::execute() {
	if (g_nancy->getGameType() < kGameTypeNancy12) {
		NancySceneState.resetAndStartTimer();

		// Nancy 11 also resets and starts one of the software-timer slots
		if (g_nancy->getGameType() >= kGameTypeNancy11) {
			TimerData::Timer *timer = getSoftwareTimer(_timerIndex);
			if (timer) {
				timer->reset();
				timer->state = TimerData::Timer::kRunning;
			}
		}

		finishExecution();
		return;
	}

	// Nancy 12+ issues a command to one of the software-timer slots. Every
	// command other than kStart only takes effect while the slot is running
	// (Nancy16: also while it's paused).
	TimerData::Timer *timer = getSoftwareTimer(_timerIndex);
	if (timer) {
		const bool acceptsCommands = g_nancy->getGameType() >= kGameTypeNancy16 ?
			timer->state != TimerData::Timer::kIdle : timer->state == TimerData::Timer::kRunning;

		if (_command == kStart) {
			timer->state = TimerData::Timer::kRunning;
		} else if (acceptsCommands) {
			const bool isNancy14 = g_nancy->getGameType() >= kGameTypeNancy14;

			// Seconds from kTimerDurationIndexBase (Nancy14+: above it) index
			// the BSUM duration table
			int16 seconds = _seconds;
			if (isNancy14 ? seconds > kTimerDurationIndexBase : seconds >= kTimerDurationIndexBase) {
				auto *bootSummary = GetEngineData(BSUM);
				uint index = seconds - kTimerDurationIndexBase;
				if (bootSummary && index < bootSummary->timerDurations.size()) {
					seconds = bootSummary->timerDurations[index];
				}
			}

			// Only Nancy14+ uses the milliseconds field
			const uint32 durationMs = ((uint32)_hours * 3600 + (uint32)_minutes * 60 + (uint32)seconds) * 1000 +
				(isNancy14 ? _milliseconds : 0);

			switch (_command) {
			case kClear:
				timer->reset();
				break;
			case kPause:
				timer->state = TimerData::Timer::kPaused;
				break;
			case kAddTime:
				if (!isNancy14) {
					adjustTimerComponents(*timer, _command, _hours, _minutes, seconds);
				} else {
					timer->currentTimeMs += durationMs;
				}

				break;
			case kSubtractTime:
			case kSetTime:
				if (!isNancy14) {
					adjustTimerComponents(*timer, _command, _hours, _minutes, seconds);
				} else if (_command == kSubtractTime) {
					timer->currentTimeMs = timer->currentTimeMs > durationMs ? timer->currentTimeMs - durationMs : 0;
				} else {
					timer->currentTimeMs = durationMs;
				}

				// Reset triggers whose target time is now later than the
				// elapsed time, so they fire again once the timer reaches them
				for (TimerData::Trigger &trigger : timer->triggers) {
					if (timer->currentTimeMs < trigger.durationMs) {
						trigger.hasFired = false;
					}
				}

				break;
			case kConfigOneShot:
			case kConfigRepeating:
				addTimerTrigger(*timer,
					(_command == kConfigRepeating) ? TimerData::Trigger::kRepeating : TimerData::Trigger::kOneShot,
					durationMs, _sound, _flags);
				break;
			default:
				break;
			}
		}
	}

	// Repeating records (e.g. gradually refilling a timer) run again next frame
	finishExecution();
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

	finishExecution();
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
		finishExecution();
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

	finishExecution();
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

HideCursor::~HideCursor() {
	if (_isHiding) {
		g_nancy->setMouseEnabled(true);
	}
}

void HideCursor::execute() {
	if (_state == kBegin) {
		g_nancy->setMouseEnabled(false);
		_isHiding = true;
		_state = kRun;
	}
}

void UIControl::readData(Common::SeekableReadStream &stream) {
	_actionType = stream.readSint32LE();
	readFilename(stream, _target);
	readFilename(stream, _textKey);
	_value = stream.readDoubleLE();
}

void UIControl::execute() {
	UI::NDUI::Action action;
	action.eventType = UI::NDUI::kEventActionRecord;
	action.type = _actionType;
	action.target = _target;
	action.hasValue = true;
	action.value = _value;
	action.sender = "ActionRecord";

	if (!_textKey.empty()) {
		const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
		assert(autotext);
		action.text = autotext->texts.getValOrDefault(_textKey);
	}

	NancySceneState.getNDUI()->takeAction(action);
	finishExecution();
}

void AddJournalEntry::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _pageName);
	readFilename(stream, _textKey);
}

void AddJournalEntry::execute() {
	NancySceneState.getNDUI()->addJournalEntry(_pageName, _textKey);
	finishExecution();
}

void AddTasklistEntry::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _textKey);
	_flag = stream.readSint16LE();
}

void AddTasklistEntry::execute() {
	NancySceneState.getNDUI()->addTasklistEntry(_textKey, _flag);
	finishExecution();
}

void LoseGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void LoseGame::execute() {
	if (g_nancy->getGameType() >= kGameTypeNancy16) {
		NancySceneState.loadSecondChance();
		_isDone = true;
		return;
	}

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
	if (g_nancy->getGameType() >= kGameTypeNancy15) {
		_characterIndex = stream.readByte();     // whose resources are changed
	}

	_resourceIndex = stream.readSint16LE();      // which UIRC resource to change
	_amount = stream.readSint16LE();             // value / delta, or a table index
	_mode = stream.readByte();                   // see ResourceUseMode
	_flag.label = stream.readSint16LE();         // event flag set on success
	_flag.flag = stream.readByte();

	readFilename(stream, _failSoundName);        // played when the change can't be applied
	readFilename(stream, _successSoundName);     // played when it is applied

	readRect(stream, _paymentHotspot);
	_useResourceCursor = stream.readByte();

	_sceneID = stream.readUint16LE();
	_continueSceneSound = stream.readUint16LE();

	_drawResourceOverlay = stream.readByte() != 0;
	_overlayDest.x = stream.readSint32LE();
	_overlayDest.y = stream.readSint32LE();

	_drawResourceValue = stream.readByte() != 0;
	_valueDest.x = stream.readSint32LE();
	_valueDest.y = stream.readSint32LE();
}

byte ResourceUse::getCharacterIndex() const {
	return _characterIndex == kPlayerCharacterActive ? (byte)g_nancy->getPlayerCharacter() : _characterIndex;
}

void ResourceUse::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	moveTo(screenBounds);

	const UIRC *uirc = GetEngineData(UIRC);
	const bool haveItem = uirc && _resourceIndex >= 0 && (uint)_resourceIndex < uirc->items.size();

	if (haveItem && _drawResourceOverlay) {
		const UIRC::ItemRecord &item = uirc->items[_resourceIndex];
		Graphics::ManagedSurface image;
		g_nancy->_resource->loadImage(item.overlayName, image);
		image.setTransparentColor(_drawSurface.getTransparentColor());

		Common::Rect src = item.rect;
		Common::Rect dest(_overlayDest.x, _overlayDest.y,
			_overlayDest.x + src.width(), _overlayDest.y + src.height());
		_drawSurface.blitFrom(image, src, dest);
	}

	if (haveItem && _drawResourceValue) {
		// Unlike the taskbar coin purse, the overlay prints the bare number:
		// no currency symbol and no decimal point, whatever the record says
		const UIRC::ItemRecord &item = uirc->items[_resourceIndex];
		const Font *font = g_nancy->_graphics->getFont(item.fontID);
		if (font) {
			const Common::String text =
				Common::String::format("%d", NancySceneState.getUIResource(_resourceIndex, getCharacterIndex()));

			// The record's y is the bottom row the glyphs are aligned on
			const int y = _valueDest.y - font->getFontHeight() + 1;
			font->drawString(&_drawSurface, text, _valueDest.x, y, screenBounds.width() - _valueDest.x, 0);
		}
	}

	setVisible(_drawResourceOverlay || _drawResourceValue);
	registerGraphics();
}

void ResourceUse::applyChange() {
	const byte characterIndex = getCharacterIndex();

	if (_mode > kSetTableValue) {
		warning("Unknown ResourceUse mode %u, treating it as an addition", _mode);
	}

	// In the table modes the amount is a table index, not the amount itself.
	int32 amount = _amount;
	if (_mode == kAddTableValue || _mode == kSetTableValue) {
		auto *tableData = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
		amount = (tableData && _amount != tableData->getNoIndex()) ? tableData->getValue(_amount) : 0;
	}

	if (_mode == kSetValue || _mode == kSetTableValue) {
		// Set the resource outright.
		NancySceneState.setUIResource(_resourceIndex, amount, characterIndex);
		NancySceneState.setEventFlag(_flag);
		_paymentApplied = true;
	} else {
		// Add the (signed) amount, but never let the resource go negative —
		// the original skips the change (e.g. when Nancy can't afford it).
		const int32 result = NancySceneState.getUIResource(_resourceIndex, characterIndex) + amount;
		_paymentApplied = result >= 0;
		if (_paymentApplied) {
			NancySceneState.setUIResource(_resourceIndex, result, characterIndex);
			NancySceneState.setEventFlag(_flag);
		}
	}

	// Start the sound matching the outcome, if there is one.
	const Common::String &soundName = _paymentApplied ? _successSoundName : _failSoundName;
	if (!soundName.empty() && !soundName.equalsIgnoreCase("NO SOUND") && !soundName.equalsIgnoreCase("NO SOUND PLAY")) {
		_sound.name = soundName;
		_sound.numLoops = 1;

		const UIRC *uirc = GetEngineData(UIRC);
		if (uirc && _resourceIndex >= 0 && (uint)_resourceIndex < uirc->items.size()) {
			_sound.channelID = uirc->items[_resourceIndex].soundChannel;
			_sound.volume = uirc->items[_resourceIndex].soundVolume;
		}

		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);
		_hasSound = true;
	}

	_paymentResolved = true;
}

void ResourceUse::handleInput(NancyInput &input) {
	if (!_interactive || _state != kRun || _paymentResolved) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_paymentHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(_useResourceCursor ?
			(CursorManager::CursorType)(_resourceIndex + 0x1f) : CursorManager::kNormal, true);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			applyChange();
		}
	}
}

void ResourceUse::execute() {
	switch (_state) {
	case kBegin:
		init();

		// A non-degenerate hotspot means the player has to click it (e.g. a coin
		// slot) to trigger the change; otherwise it happens immediately.
		_interactive = _paymentHotspot.top != _paymentHotspot.bottom;
		if (!_interactive) {
			applyChange();
		}

		_state = kRun;
		break;
	case kRun:
		// Interactive changes wait for the player to click the hotspot.
		if (_interactive && !_paymentResolved) {
			return;
		}

		// Keep the overlay up until the outcome sound has finished.
		if (_hasSound && g_nancy->_sound->isSoundPlaying(_sound)) {
			return;
		}

		_state = kActionTrigger;
		break;
	case kActionTrigger:
		if (_hasSound) {
			g_nancy->_sound->stopSound(_sound);
		}

		setVisible(false);

		// Only a successful change advances the scene.
		if (_paymentApplied && _sceneID != kNoScene) {
			SceneChangeDescription sceneChange;
			sceneChange.sceneID = _sceneID;
			sceneChange.continueSceneSound = _continueSceneSound;
			NancySceneState.changeScene(sceneChange);
		}

		finishExecution();
		break;
	}
}

void PlayChar::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() >= kGameTypeNancy17) {
		// Nancy17 names the character, and has no background to swap in
		Common::String name;
		readFilename(stream, name);

		_characterIndex = kPlayerCharacterActive;
		const PCUI *pcui = GetEngineData(PCUI);
		for (uint i = 0; pcui && i < pcui->characters.size(); ++i) {
			if (pcui->characters[i].name.equalsIgnoreCase(name)) {
				_characterIndex = i;
			}
		}

		return;
	}

	_characterIndex = stream.readByte();
	readFilename(stream, _videoFile);
}

void PlayChar::execute() {
	const PCUI *pcui = GetEngineData(PCUI);
	if (!pcui || _characterIndex >= pcui->characters.size()) {
		warning("PlayChar: no player character %u", _characterIndex);
		finishExecution();
		return;
	}

	// Every character owns an event flag that marks them as the one being
	// played; conditions elsewhere in the game branch on those
	for (uint i = 0; i < pcui->characters.size(); ++i) {
		const uint16 flagLabel = pcui->characters[i].id;
		if (flagLabel != 0) {
			NancySceneState.setEventFlag(flagLabel, i == _characterIndex ? g_nancy->_true : g_nancy->_false);
		}
	}

	NancySceneState.changePlayerCharacter(_characterIndex);

	auto *playerChar = (PlayerCharacterData *)NancySceneState.getPuzzleData(PlayerCharacterData::getTag());
	if (playerChar) {
		playerChar->characterIndex = _characterIndex;
	}

	// The scene itself doesn't change; only the video showing it does, so that
	// the location is seen through the incoming character's eyes
	if (!_videoFile.empty()) {
		NancySceneState.changeSceneVideo(_videoFile);
	}

	finishExecution();
}

void FadeOverlay::readData(Common::SeekableReadStream &stream) {
	_seconds = stream.readFloatLE();
	_z = stream.readUint32LE();
	_fromColor = stream.readUint32BE();
	_toColor = stream.readUint32BE();
	readExclusiveRect(stream, _bounds);
	_doneFlag.label = stream.readSint16LE();
	_doneFlag.flag = stream.readByte();
}

void FadeOverlay::execute() {
	if (_state == kBegin) {
		_drawSurface.create(_bounds.width(), _bounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
		moveTo(_bounds);
		RenderObject::init();
		registerGraphics();

		_startTime = g_nancy->getTotalPlayTime();
		_state = kRun;
	}

	if (_hasEnded) {
		return;
	}

	float seconds = (g_nancy->getTotalPlayTime() - _startTime) / 1000.0f;
	float progress = _seconds > 0 ? CLIP<float>(seconds / _seconds, 0, 1) : 1;

	// The color eases in and out of the change
	float amount = 0.5f + 0.5f * sin((progress - 0.5f) * M_PI);

	uint32 color = 0;
	for (uint shift = 0; shift < 32; shift += 8) {
		float from = (_fromColor >> shift) & 0xFF;
		float to = (_toColor >> shift) & 0xFF;
		color |= (uint32)(from + (to - from) * amount) << shift;
	}

	_drawSurface.clear(argbToColor(_drawSurface.format, color));
	_needsRedraw = true;

	if (seconds > _seconds) {
		_hasEnded = true;
		NancySceneState.setEventFlag(_doneFlag);
	}
}

void SetRandomFlag::readData(Common::SeekableReadStream &stream) {
	_choices.resize(stream.readUint16LE());
	for (Choice &choice : _choices) {
		choice.flag.label = stream.readSint16LE();
		choice.flag.flag = stream.readByte();
		choice.percent = stream.readSint32LE();
	}
}

void SetRandomFlag::execute() {
	// The percentages add up to 100
	int roll = g_nancy->_randomSource->getRandomNumber(99);
	for (const Choice &choice : _choices) {
		roll -= choice.percent;
		if (roll < 0) {
			NancySceneState.setEventFlag(choice.flag);
			break;
		}
	}

	_isDone = true;
}

void AddPhoneEntry::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _entry.name);
	readFilename(stream, _entry.number);
	readFilename(stream, _entry.sceneName);
	_entry.nameFlag = stream.readSint16LE();
}

void AddPhoneEntry::execute() {
	auto *data = (PhoneBookData *)NancySceneState.getPuzzleData(PhoneBookData::getTag());
	Common::Array<PhoneBookData::Entry> &entries = data->entries[g_nancy->getPlayerCharacter()];

	bool isNew = true;
	for (const PhoneBookData::Entry &entry : entries) {
		if (entry.name.equalsIgnoreCase(_entry.name)) {
			isNew = false;
		}
	}

	if (isNew) {
		entries.push_back(_entry);
	}

	NancySceneState.getNDUI()->showPhoneBookEntry(_entry, isNew);
	_isDone = true;
}

// The items of a collection record take up the rest of its data, and are either
// numbers or names. The original also has collections of flags, which no scene
// makes use of.
static void readCollectionValues(Common::SeekableReadStream &stream, CollectionData::Values &values) {
	const uint kNumberSize = 8;
	const uint kNameSize = 33;

	const uint16 numValues = stream.readUint16LE();
	if (!numValues) {
		return;
	}

	const uint itemSize = (stream.size() - stream.pos()) / numValues;
	if (itemSize == kNumberSize) {
		values.numbers.resize(numValues);
		for (double &value : values.numbers) {
			value = stream.readDoubleLE();
		}
	} else if (itemSize == kNameSize) {
		values.names.resize(numValues);
		for (Common::String &name : values.names) {
			readFilename(stream, name);
		}
	} else {
		warning("Unsupported collection item type");
	}
}

void CollectionControl::readData(Common::SeekableReadStream &stream) {
	_operation = stream.readByte();
	readFilename(stream, _name);

	if (_operation == kCreate) {
		_replaceExisting = stream.readByte();
		stream.skip(1);
		_maxSize = stream.readUint32LE();
		stream.skip(1);		// Item type, always numbers
		stream.skip(21);	// A flag, a rectangle and a color
	}
}

void CollectionControl::execute() {
	auto *data = (CollectionData *)NancySceneState.getPuzzleData(CollectionData::getTag());

	if (_operation == kDelete) {
		data->collections.erase(_name);
	} else if (_replaceExisting || !data->collections.contains(_name)) {
		CollectionData::Collection &collection = data->collections[_name];
		collection.maxSize = _maxSize;
		collection.values.clear();
	}

	_isDone = true;
}

void CollectionEdit::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);	// Edit type, always "add"
	readFilename(stream, _name);
	readCollectionValues(stream, _values);
}

void CollectionEdit::execute() {
	auto *data = (CollectionData *)NancySceneState.getPuzzleData(CollectionData::getTag());

	if (data->collections.contains(_name)) {
		CollectionData::Collection &collection = data->collections[_name];
		collection.values.numbers.push_back(_values.numbers);
		collection.values.names.push_back(_values.names);

		// The oldest values make room for the new ones
		while (collection.values.numbers.size() > collection.maxSize) {
			collection.values.numbers.remove_at(0);
		}

		while (collection.values.names.size() > collection.maxSize) {
			collection.values.names.remove_at(0);
		}
	} else {
		warning("%s: tried to add but collection does not exist", _name.c_str());
	}

	_isDone = true;
}

void CollectionCheck::readData(Common::SeekableReadStream &stream) {
	_checkType = stream.readByte();
	readFilename(stream, _name);

	const uint numFlags = _checkType == kSize ? 3 : 2;
	for (uint i = 0; i < numFlags; ++i) {
		_flags[i].label = stream.readSint16LE();
		_flags[i].flag = stream.readByte();
	}

	if (_checkType == kSize) {
		_size = stream.readSint32LE();
	} else {
		readCollectionValues(stream, _values);
	}
}

template<typename T>
static bool containsAllValues(const Common::Array<T> &collection, const Common::Array<T> &values) {
	Common::Array<T> remaining = collection;
	for (const T &value : values) {
		bool found = false;
		for (uint i = 0; i < remaining.size() && !found; ++i) {
			if (remaining[i] == value) {
				remaining.remove_at(i);
				found = true;
			}
		}

		if (!found) {
			return false;
		}
	}

	return true;
}

template<typename T>
static bool followsValuePattern(const Common::Array<T> &collection, const Common::Array<T> &values) {
	for (uint i = 0; i < collection.size(); ++i) {
		if (values.empty() || collection[i] != values[i % values.size()]) {
			return false;
		}
	}

	return true;
}

bool CollectionCheck::check(const CollectionData::Values &collection) const {
	switch (_checkType) {
	case kContainsAll:
		return containsAllValues(collection.numbers, _values.numbers) && containsAllValues(collection.names, _values.names);
	case kIsEqual:
		return collection.numbers == _values.numbers && collection.names == _values.names;
	case kFollowsPattern:
		return followsValuePattern(collection.numbers, _values.numbers) && followsValuePattern(collection.names, _values.names);
	default:
		return false;
	}
}

void CollectionCheck::execute() {
	auto *data = (CollectionData *)NancySceneState.getPuzzleData(CollectionData::getTag());

	if (data->collections.contains(_name)) {
		const CollectionData::Values &collection = data->collections[_name].values;

		if (_checkType == kSize) {
			const int32 collectionSize = collection.numbers.size() + collection.names.size();
			if (_size < collectionSize) {
				NancySceneState.setEventFlag(_flags[0]);
			} else if (_size == collectionSize) {
				NancySceneState.setEventFlag(_flags[1]);
			} else {
				NancySceneState.setEventFlag(_flags[2]);
			}
		} else {
			NancySceneState.setEventFlag(_flags[check(collection) ? 0 : 1]);
		}
	} else {
		warning("%s: tried to check collection does not exist", _name.c_str());
	}

	_isDone = true;
}

static void readWideFlag(Common::SeekableReadStream &stream, FlagDescription &flag) {
	flag.label = stream.readSint16LE();
	flag.flag = stream.readUint16LE();
}

void HeldItemFlags::readData(Common::SeekableReadStream &stream) {
	_hoverCursor = (CursorManager::CursorType)stream.readUint32LE();
	readWideFlag(stream, _noItemFlag);

	_itemFlags.resize(stream.readUint16LE());
	for (ItemFlag &itemFlag : _itemFlags) {
		itemFlag.itemID = stream.readSint32LE();
		readWideFlag(stream, itemFlag.flag);
	}

	readWideFlag(stream, _otherItemFlag);

	_hotspots.resize(stream.readUint16LE());
	for (HotspotDescription &hotspot : _hotspots) {
		hotspot.readData(stream);
	}
}

void HeldItemFlags::execute() {
	if (_state != kActionTrigger) {
		_state = kRun;
		_hasHotspot = false;

		for (const HotspotDescription &hotspot : _hotspots) {
			if (hotspot.frameID == NancySceneState.getSceneInfo().frameID) {
				_hasHotspot = true;
				_hotspot = hotspot.coords;
			}
		}

		return;
	}

	FlagDescription flag = _noItemFlag;
	int16 heldItem = NancySceneState.getHeldItem();
	if (heldItem != -1) {
		flag = _otherItemFlag;
		for (const ItemFlag &itemFlag : _itemFlags) {
			if (itemFlag.itemID == heldItem) {
				flag = itemFlag.flag;
			}
		}
	}

	NancySceneState.setEventFlag(flag);
	_hasHotspot = false;
	finishExecution();
}

void UTurn::readData(Common::SeekableReadStream &stream) {
	readExclusiveRect(stream, _hotspotRect);
	_hoverCursor = (CursorManager::CursorType)stream.readSint16LE();
	_excludesFrames = stream.readByte();

	_frames.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (int16 &frame : _frames) {
		frame = stream.readSint16LE();
	}
}

bool UTurn::isActiveOnFrame(uint16 frame) const {
	bool isListed = false;
	for (int16 listed : _frames) {
		if (listed == frame) {
			isListed = true;
		}
	}

	if (_excludesFrames) {
		return !isListed;
	}

	return isListed || _frames.empty();
}

void UTurn::execute() {
	// A node is a full turn made out of this many frames
	const uint kFramesPerTurn = 20;

	if (_state == kActionTrigger) {
		UI::Viewport &viewport = NancySceneState.getViewport();
		uint frame = (viewport.getCurFrame() + kFramesPerTurn / 2) % kFramesPerTurn;
		if (frame < viewport.getFrameCount()) {
			viewport.setFrame(frame);
		}
	}

	_state = kRun;
	_hasHotspot = isActiveOnFrame(NancySceneState.getSceneInfo().frameID);
	_hotspot = _hotspotRect;
}

void GraphChase::readData(Common::SeekableReadStream &stream) {
	_chaserTableIndex = stream.readSint16LE();
	_chasedTableIndex = stream.readSint16LE();
	_avoidsAdjacentNodes = stream.readByte();

	for (uint i = 0; i < kNumLinkTypes; ++i) {
		_linkTypeFlags[i] = stream.readSint16LE();
	}

	_nodes.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (Node &node : _nodes) {
		stream.skip(2);	// The node's ID, which is its place in the list

		node.resize(MAX<int16>(stream.readSint16LE(), 0));
		for (Link &link : node) {
			link.nodeID = stream.readSint32LE();
			link.type = stream.readByte();
		}
	}
}

bool GraphChase::canMoveTo(const Link &link, int16 chaserNode) const {
	if (link.nodeID == chaserNode) {
		return false;
	}

	if (_avoidsAdjacentNodes) {
		for (const Link &chaserLink : _nodes[chaserNode - 1]) {
			if (chaserLink.nodeID == link.nodeID) {
				return false;
			}
		}
	}

	return true;
}

void GraphChase::execute() {
	_isDone = true;

	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (!table) {
		return;
	}

	const int16 chaserNode = table->getSingleValue(_chaserTableIndex);
	const int16 chasedNode = table->getSingleValue(_chasedTableIndex);
	if (chaserNode < 1 || chaserNode > (int16)_nodes.size() || chasedNode < 1 || chasedNode > (int16)_nodes.size()) {
		warning("GraphChase: nodes %d and %d are not on the map", chaserNode, chasedNode);
		return;
	}

	// Links get tried at random until one will do. Once none of them does,
	// the places next to the player's stop being avoided, for good.
	Node remaining = _nodes[chasedNode - 1];
	while (!remaining.empty()) {
		uint pick = g_nancy->_randomSource->getRandomNumber(remaining.size() - 1);
		const Link link = remaining[pick];

		if (canMoveTo(link, chaserNode)) {
			table->setSingleValue(_chasedTableIndex, link.nodeID);
			for (uint i = 0; i < kNumLinkTypes; ++i) {
				NancySceneState.setEventFlag(_linkTypeFlags[i], link.type == i ? g_nancy->_true : g_nancy->_false);
			}

			return;
		}

		remaining.remove_at(pick);
		if (remaining.empty() && _avoidsAdjacentNodes) {
			_avoidsAdjacentNodes = false;
			remaining = _nodes[chasedNode - 1];
		}
	}
}

void ClearSavedData::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _name);
}

void ClearSavedData::execute() {
	auto *data = (SavedStateData *)NancySceneState.getPuzzleData(SavedStateData::getTag());
	data->states.erase(_name);
	_isDone = true;
}

void SaveNamedGame::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _name);
}

void SaveNamedGame::execute() {
	NancySceneState.getNDUI()->requestSave(_name);
	_isDone = true;
}

void LoadNamedGame::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _name);
}

void LoadNamedGame::execute() {
	NancySceneState.getNDUI()->requestLoad(_name);
	_isDone = true;
}

void Rain::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_dropSrcs.resize(stream.readUint16LE());
	for (Common::Rect &src : _dropSrcs) {
		readRect(stream, src);
	}

	_areas.resize(stream.readUint16LE());
	for (HotspotDescription &area : _areas) {
		area.frameID = stream.readUint16LE();
		readExclusiveRect(stream, area.coords);
	}

	_z = stream.readSint32LE();
	_minDistance = stream.readFloatLE();
	_maxDistance = stream.readFloatLE();
	_numDrops = stream.readSint32LE();
	_interval = (uint32)(stream.readFloatLE() * 1000);
}

void Rain::execute() {
	if (_state == kBegin) {
		g_nancy->_resource->loadImage(_imageName, _image);
		RenderObject::init();
		registerGraphics();
		setVisible(false);
		_state = kRun;
	}

	int areaIndex = -1;
	for (uint i = 0; i < _areas.size(); ++i) {
		if (_areas[i].frameID == NancySceneState.getSceneInfo().frameID) {
			areaIndex = i;
		}
	}

	if (areaIndex != _areaIndex) {
		_areaIndex = areaIndex;
		_nextDrawTime = 0;
		setVisible(areaIndex != -1);

		if (areaIndex != -1) {
			const Common::Rect &area = _areas[areaIndex].coords;
			_drawSurface.create(area.width(), area.height(), g_nancy->_graphics->getTransparentPixelFormat());
			moveTo(area);
		}
	}

	if (_areaIndex != -1 && !_dropSrcs.empty() && g_nancy->getTotalPlayTime() >= _nextDrawTime) {
		_nextDrawTime = g_nancy->getTotalPlayTime() + _interval;
		drawDrops();
	}
}

// The rain stops once the record's dependencies don't hold anymore, e.g. in the
// scenes that show something up close
void Rain::updateGraphics() {
	if (!_isActive && _areaIndex != -1) {
		_areaIndex = -1;
		setVisible(false);
	}
}

void Rain::drawDrops() {
	static const float kDistanceCurve = 0.85f;
	static const float kDistanceFade = 0.75f;

	_drawSurface.clear(0);

	const float exponent = 1.0f - (_maxDistance - _minDistance) * kDistanceCurve;

	for (int i = 0; i < _numDrops; ++i) {
		const Common::Rect &src = _dropSrcs[g_nancy->_randomSource->getRandomNumber(_dropSrcs.size() - 1)];

		float distance = _minDistance + (_maxDistance - _minDistance) * g_nancy->_randomSource->getRandomNumber(32767) / 32767.0f;
		distance = pow(distance, exponent);

		byte alpha = (byte)((1.0f - kDistanceFade * distance) * 255);
		int height = MAX<int>((int)((1.0f - distance) * src.height()), 1);

		// Anywhere that leaves a part of the drop inside
		int rangeX = _drawSurface.w + src.width() - 2;
		int rangeY = _drawSurface.h + height - 2;
		if (rangeX <= 0 || rangeY <= 0) {
			continue;
		}

		Common::Point pos;
		pos.x = 1 - src.width() + g_nancy->_randomSource->getRandomNumber(rangeX - 1);
		pos.y = 1 - height + g_nancy->_randomSource->getRandomNumber(rangeY - 1);

		drawDrop(src, pos, height, alpha);
	}

	_needsRedraw = true;
}

// Draws a drop squeezed to the given height, on top of the ones already there
void Rain::drawDrop(const Common::Rect &src, const Common::Point &pos, int height, byte alpha) {
	for (int y = 0; y < height; ++y) {
		int destY = pos.y + y;
		int srcY = src.top + y * src.height() / height;
		if (destY < 0 || destY >= _drawSurface.h || srcY >= _image.h) {
			continue;
		}

		for (int x = 0; x < src.width(); ++x) {
			int destX = pos.x + x;
			int srcX = src.left + x;
			if (destX < 0 || destX >= _drawSurface.w || srcX >= _image.w) {
				continue;
			}

			byte a, r, g, b;
			_image.format.colorToARGB(_image.getPixel(srcX, srcY), a, r, g, b);
			a = a * alpha / 255;
			if (a == 0) {
				continue;
			}

			byte destA, destR, destG, destB;
			_drawSurface.format.colorToARGB(_drawSurface.getPixel(destX, destY), destA, destR, destG, destB);

			uint outA = a + destA * (255 - a) / 255;
			r = (r * a + destR * destA * (255 - a) / 255) / outA;
			g = (g * a + destG * destA * (255 - a) / 255) / outA;
			b = (b * a + destB * destA * (255 - a) / 255) / outA;

			_drawSurface.setPixel(destX, destY, _drawSurface.format.ARGBToColor(outA, r, g, b));
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
