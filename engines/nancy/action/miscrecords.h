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

#ifndef NANCY_ACTION_RECORDTYPES_H
#define NANCY_ACTION_RECORDTYPES_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/enginedata.h"

namespace Nancy {

class NancyEngine;

namespace Action {

// Changes the palette for the current scene's background. TVD only.
class PaletteThisScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _paletteID;
	byte _unknownEnum; // enum w values 1-3
	uint16 _paletteStart;
	uint16 _paletteSize;

protected:
	Common::String getRecordTypeName() const override { return "PaletteThisScene"; }
};

// Changes the palette for the next scene's background. TVD only.
class PaletteNextScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _paletteID;

protected:
	Common::String getRecordTypeName() const override { return "PaletteNextScene"; }
};

// Turns on (temporary) lightning effect. TVD Only.
class LightningOn : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	int16 _distance;
	uint16 _pulseTime;
	int16 _rgbPercent;

protected:
	Common::String getRecordTypeName() const override { return "LightningOn"; }
};

// Requests either a fade between two scenes, or a fade to black; fade executes when scene is changed. Nancy2 and up.
class SpecialEffect : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _type = 1;
	uint16 _fadeToBlackTime = 0;
	uint16 _frameTime = 0;
	uint16 _totalTime = 0;
	Common::Rect _rect;

protected:
	Common::String getRecordTypeName() const override { return "SpecialEffect"; }
};

// Adds a caption to the textbox. The Nancy 11+ "autotext" variant (AR 81),
// carries an extra header that makes the record wait for a sound to finish
// or a timer to elapse before it completes; in both variants the body is
// either inline text or resolved from an AUTOTEXT key.
class TextBoxWrite : public ActionRecord {
public:
	enum WaitMode { kWaitNone = 0, kWaitForSound = 1, kWaitForTimer = 2 };

	TextBoxWrite(bool isAutotext = false) : _isAutotext(isAutotext) {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::String _text;

	// Nancy 11+ AR 81 only
	bool _isAutotext;
	int16 _waitMode = 0;
	uint16 _soundChannel = 0;
	uint32 _waitTimeMs = 0;

protected:
	Common::String getRecordTypeName() const override { return _isAutotext ? "AutotextTextBoxWrite" : "TextBoxWrite"; }

private:
	uint32 _endTime = 0;
};

// Clears the textbox. Used very rarely.
class TextboxClear : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "TextboxClear"; }
};

// Nancy 10+ replacement for TextBoxWrite. Pushes a line of conversation
// text into the new (UICO-driven) textbox
class FrameTextBox : public ActionRecord {
public:
	FrameTextBox(bool fullMode) : _fullMode(fullMode) {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	bool _fullMode;
	Common::String _text;

protected:
	Common::String getRecordTypeName() const override { return "FrameTextBox"; }
};

// Nancy 10+ opcode 29. Toggles whether one of the taskbar popups
// (inventory / notebook / cellphone) is enabled.
class ControlUIItems : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _uiButton = 0;
	byte _autoOpenOrBadgeSound = 0; // 1 = auto-open popup; 0/10 = notification-badge click-sound selector
	byte _flagB = 0;    // 0 = clear, 1 = enable+remember scene
	int16 _startScene = 0; // start scene id (9999 = none); also the auto-open cell phone's call target
	int16 _endScene = 0;   // end scene id (9999 = none)

	Common::String getRecordExtraInfo() const override {
		return Common::String::format("uiButton: %d, autoOpenOrBadgeSound: %d, flagB: %d, startScene: %d, endScene: %d",
									  _uiButton, _autoOpenOrBadgeSound, _flagB, _startScene, _endScene);
	}

protected:
	Common::String getRecordTypeName() const override { return "ControlUIItems"; }
};

// Nancy 10+ opcode 32. Prepares a UI popup
class UIPopupPrepScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	int32 _uiType = 0;
	int32 _signalValue = 0;

protected:
	Common::String getRecordTypeName() const override { return "UIPopupPrepScene"; }
};

// Nancy 10+ opcode 131. Pushes a new entry into either the cellphone
// search-results list (mode 0) or the URL link list (mode 1).
class AddSearchLink : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	int16 _mode = 0;
	Common::String _key;          // CVTX key for the list row text (both modes)
	Common::String _value;        // body CVTX key (mode 0/email); unused for mode 1
	int16 _extra = 0;             // page index (mode 1); unused for mode 0
	int16 _flag = 0;              // stored but unused by the original; reserved
	int16 _eventFlag = 0;         // event-flag index set when the entry is opened

	Common::String getRecordExtraInfo() const override {
		return Common::String::format("Key: %s, Value: %s, Mode: %d, Extra: %d, Flag: %d, EventFlag: %d",
			_key.c_str(), _value.c_str(), _mode, _extra, _flag, _eventFlag);
	}

protected:
	Common::String getRecordTypeName() const override { return "AddSearchLink"; }
};

// Sets the cellphone's battery/signal indicators. Modes 0/1 toggle the
// battery (normal / low) and 2/3 toggle the signal (normal / no signal).
class SetCellPhoneBatteryAndSignal : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _mode = 0;

protected:
	Common::String getRecordTypeName() const override { return "SetCellPhoneBatteryAndSignal"; }
};

// Adds a new entry to the cellphone directory, or overwrites an existing
// one matched by dial pattern. Used to unlock contacts as the player
// progresses (Nancy 10+).
class ChangeCellPhoneInfo : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	UICL::Contact _contact;

	Common::String getRecordExtraInfo() const override {
		return Common::String::format("Contact: %s", _contact.name.c_str());
	}

protected:
	Common::String getRecordTypeName() const override { return "ChangeCellPhoneInfo"; }
};

// Returns from a cellphone-driven conversation scene to the pre-call scene.
// sceneID == kNoScene pops the saved scene; any other sceneID overrides it.
class CellPhonePopCellSceneFromStack : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SceneChangeDescription _sceneChange;

protected:
	Common::String getRecordTypeName() const override { return "CellPhonePopCellSceneFromStack"; }
};

// Changes the in-game time. Used prior to the introduction of SetPlayerClock.
class BumpPlayerClock : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _relative;
	uint16 _hours;
	uint16 _minutes;

protected:
	Common::String getRecordTypeName() const override { return "BumpPlayerClock"; }
};

// Creates a Second Chance save.
class SaveContinueGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "SaveContinueGame"; }
};

// Stops the screen from rendering. Our rendering system is different from the original engine's,
// so we have no use for this.
class TurnOffMainRendering : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "TurnOffMainRendering"; }
};

// Restarts screen rendering. Our rendering system is different from the original engine's,
// so we have no use for this.
class TurnOnMainRendering : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "TurnOnMainRendering"; }
};

// Starts the timer. Used in combination with Dependency types that check for
// how much time has passed since the timer was started. From Nancy 11 onwards
// the record also carries a software-timer slot index (see TimerControl).
class ResetAndStartTimer : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _timerIndex = 0; // Nancy 11+ software-timer slot

protected:
	Common::String getRecordTypeName() const override { return "ResetAndStartTimer"; }
};

// Stops the timer.
class StopTimer : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _timerIndex = 0; // Nancy 11+ software-timer slot

protected:
	Common::String getRecordTypeName() const override { return "StopTimer"; }
};

// Nancy 11+ AR 69 (AT_TIMER_CONTROL). Issues a command to one of the 10
// software-timer slots (see TimerData::Timer). The fixed-size chunk
// (0xc4 header + count*4 flag entries) carries a slot index, a command, a
// target duration, an optional sound + caption, and the event flags to fire
// when the timer expires.
class TimerControl : public ActionRecord {
public:
	enum Command {
		kReset       = 0, // Clear the slot back to idle
		kStart       = 1, // Begin counting, with no target
		kPause       = 2, // Suspend counting
		kAddTime     = 3, // Add the duration to the elapsed time
		kSubtractTime = 4, // Subtract the duration from the elapsed time
		kConfigOneShot   = 5, // Set target/payload; fire once, then reset
		kConfigRepeating = 6  // Set target/payload; fire once, then keep running
	};

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	int16 _timerIndex = 0;
	int16 _command = 0;
	int16 _hours = 0;
	int16 _minutes = 0;
	int16 _seconds = 0;

	SoundDescription _sound;
	Common::String _autotextKey;
	Common::String _caption;
	Common::Array<FlagDescription> _flags;

protected:
	Common::String getRecordTypeName() const override { return "TimerControl"; }
};

// Nancy 11+ AR 30. Disables the player's ability to scroll/pan the viewport
// (both mouse-edge and keyboard movement). State persists across scene changes.
class StopPlayerScrolling : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "StopPlayerScrolling"; }
};

// Nancy 11+ AR 31. Re-enables the player's ability to scroll/pan the viewport.
class StartPlayerScrolling : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "StartPlayerScrolling"; }
};

// Returns the player back to the main menu
class GotoMenu : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "GotoMenu"; }
};

// Stops the game and boots the player back to the Menu screen, while also making sure
// they can't Continue. The devs took care to add Second Chance saves before every one
// of these, to make sure the player can return to a state just before the dangerous part.
class LoseGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "LoseGame"; }
};

// Adds a scene to the "stack" (which is just a single value). Used in combination with PopScene.
class PushScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "PushScene"; }
};

// Changes to the scene pushed onto the "stack". Scenes can be pushed via PushScene, or Conversation types.
class PopScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "PopScene"; }
};

// Ends the game and boots the player to the Credits screen.
// TODO: The original engine also sets a config option called PlayerWonTheGame,
// which in turn is used to trigger whichever event flag marks that the player
// has beat the game at least once, which in turn allows easter eggs to be shown.
// We currently support none of this.
class WinGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "WinGame"; }
};

// Checks how many hints the player is allowed to get. If they are still allowed hints,
// it selects an appropriate one and plays its sound/displays its caption in the Textbox.
// The hint system was _only_ used in nancy1, since it's pretty limited and overly punishing.
class HintSystem : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _characterID; // 0x00
	SoundDescription _genericSound; // 0x01

	const Hint *selectedHint;
	int16 _hintID;

	void selectHint();

protected:
	Common::String getRecordTypeName() const override { return "HintSystem"; }
};

// Added in Nancy12 (AR 132). Adjusts a UI overlay resource (from the UIRC boot
// chunk) at runtime.
class ResourceUse : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "ResourceUse"; }

	int16 _resourceIndex = 0;
	int16 _amount = 0;
	byte _mode = 0;        // 0 = set the resource, non-zero = add (clamped to >= 0)
	FlagDescription _flag; // event flag set when the change is applied
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MISCRECORDS_H
