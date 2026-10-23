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
#include "engines/nancy/puzzledata.h"

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
	enum WriteType { kTextBoxWrite, kAutotextWrite };

	TextBoxWrite(WriteType writeType) : _writeType(writeType) {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::String _text;

	// Nancy 11+ AR 81 only
	WriteType _writeType;
	int16 _waitMode = 0;
	uint16 _soundChannel = 0;
	uint32 _waitTimeMs = 0;

protected:
	Common::String getRecordTypeName() const override { return _writeType == kAutotextWrite ? "AutotextTextBoxWrite" : "TextBoxWrite"; }

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
	enum BoxMode { kNormalBox, kFullBox };

	FrameTextBox(BoxMode boxMode) : _boxMode(boxMode) {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	BoxMode _boxMode;
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
	byte _characterIndex = kPlayerCharacterActive; // Nancy15+: whose taskbar the disable applies to
	byte _autoOpenOrBadgeSound = 0; // 1 = auto-open popup; 0/10 = notification-badge click-sound selector
	byte _flagB = 0;    // 0 = clear, 1 = enable+remember scene
	int16 _startScene = 0; // start scene id (9999 = none); also the auto-open cell phone's call target
	int16 _endScene = 0;   // end scene id (9999 = none)

	Common::String getRecordExtraInfo() const override {
		return Common::String::format("uiButton: %d, character: %d, autoOpenOrBadgeSound: %d, flagB: %d, startScene: %d, endScene: %d",
									  _uiButton, _characterIndex, _autoOpenOrBadgeSound, _flagB, _startScene, _endScene);
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
	SearchLink _link;

	Common::String getRecordExtraInfo() const override {
		return Common::String::format("Key: %s, Value: %s, Mode: %d, Extra: %d, Flag: %d, EventFlag: %d",
			_link.key.c_str(), _link.value.c_str(), _mode, _link.extra, _link.flag, _link.eventFlag);
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
	int16 _hours;
	int16 _minutes;

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

// Nancy9 AR 148, moved to AR 141 in Nancy12. Saves a cropped picture of the
// current frame to a PNG file. Used as an easter egg, e.g. on the sandcastle
// screen in Danger on Deception Island.
class MakeScreenFile : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::String _filename;
	Common::Rect _cropRect;

	Common::String getRecordExtraInfo() const override {
		return Common::String::format("Filename: %s.png, crop rect: (%d, %d, %d, %d)",
			_filename.c_str(), _cropRect.left, _cropRect.top, _cropRect.right, _cropRect.bottom);
	}

protected:
	Common::String getRecordTypeName() const override { return "MakeScreenFile"; }
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
// how much time has passed since the timer was started. Nancy 11 also carries a
// software-timer slot index (see TimerControl). From Nancy 12 the record became
// a general "Control a Timer" command: a slot index plus a command whose value
// selects a variable-size payload.
class ResetAndStartTimer : public ActionRecord {
public:
	enum Command {
		kStart           = 0, // Begin counting up from the current time
		kClear           = 1, // Reset the slot back to idle
		kConfigOneShot   = 2, // Set target/payload; fire once, then reset
		kConfigRepeating = 3, // Set target/payload; fire once, then keep counting
		kPause           = 4, // Suspend counting
		kAddTime         = 5, // Add the duration to the elapsed time
		kSubtractTime    = 6, // Subtract the duration from the elapsed time
		kSetTime         = 7  // Set the elapsed time to the duration
	};

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	int16 _timerIndex = 0;   // Software-timer slot (Nancy 11+)
	int16 _command = kStart; // Nancy 12+
	int16 _hours = 0;
	int16 _minutes = 0;
	int16 _seconds = 0;
	int16 _milliseconds = 0;
	SoundDescription _sound;               // Played on expiry when configured
	Common::Array<FlagDescription> _flags; // Fired on expiry when configured

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

// Nancy16+. Hides the cursor and blocks mouse input for as long as the record exists.
class HideCursor : public ActionRecord {
public:
	~HideCursor() override;

	void readData(Common::SeekableReadStream &stream) override {}
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "HideCursor"; }

	bool _isHiding = false;
};

// Nancy16 record that sends an action to a panel or control of the UI, the same way
// the UI's own controls do when they are clicked
class UIControl : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "UIControl"; }

	int32 _actionType = 0;
	Common::String _target;
	Common::String _textKey;	// AUTOTEXT key
	double _value = 0;
};

// Nancy16 journal entry. The page names a panel of the journal, and its text is the
// title of the page.
class AddJournalEntry : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "AddJournalEntry"; }

	Common::String _pageName;
	Common::String _textKey;	// AUTOTEXT key
};

// Nancy16 task list entry, which can be checked off once its flag is set
class AddTasklistEntry : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "AddTasklistEntry"; }

	Common::String _textKey;	// AUTOTEXT key
	int16 _flag = kEvNoEvent;
};

// Nancy16 fade of the viewport to a solid color and back, covering the next scene
// change. The fade itself is owned by the scene; the record only arms it, waiting
// for any previous fade to end first.
class Fader : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "Fader"; }

	uint32 _holdTime = 0;	// ms
	uint32 _color = 0;		// RGB
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
// chunk) at runtime -- e.g. paying coins from the purse (resource 0). Applying
// the change plays a sound, optionally shows a transient overlay (a sprite and/or
// the resource's numeric value) and can change scene on success.
class ResourceUse : public RenderActionRecord {
public:
	ResourceUse() : RenderActionRecord(7) {}

	void init() override;
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "ResourceUse"; }

	// Applies the resource change (respecting affordability), sets the event
	// flag and starts the matching outcome sound.
	void applyChange();

	// The protagonist whose resources this record changes
	byte getCharacterIndex() const;

	enum ResourceUseMode : byte {
		kSetValue = 0,			// set the resource to _amount
		kAddValue = 1,			// add _amount, if the resource can cover it
		kAddTableValue = 2,		// same, with the amount taken from the table
		kSetTableValue = 3		// set to the table value at _amount
	};

	// Nancy15 pays from a named protagonist's resources; kPlayerCharacterActive
	// (and every earlier game) means whoever is being played.
	byte _characterIndex = kPlayerCharacterActive;

	int16 _resourceIndex = 0;
	int16 _amount = 0;     // an amount, or a table index in the table modes
	byte _mode = kSetValue;
	FlagDescription _flag; // event flag set when the change is applied

	Common::String _failSoundName;    // played when the change can't be applied
	Common::String _successSoundName; // played when it is applied

	// When this rect is non-degenerate the change is interactive: the player
	// clicks it (e.g. a coin slot) to pay. A degenerate rect applies at once.
	Common::Rect _paymentHotspot;
	byte _useResourceCursor = 0;      // 0 = normal cursor, else the resource's own hover cursor

	uint16 _sceneID = kNoScene;       // scene entered on success (9999 = none)
	uint16 _continueSceneSound = 0;

	bool _drawResourceOverlay = false; // blit the resource's UIRC sprite
	Common::Point _overlayDest;
	bool _drawResourceValue = false;   // draw the resource's numeric value
	Common::Point _valueDest;

	SoundDescription _sound;
	bool _hasSound = false;
	bool _interactive = false;
	bool _paymentResolved = false;
	bool _paymentApplied = false;
};

// Added in Nancy15 (AR 134). Hands control to one of the game's protagonists
// (Nancy, Frank or Joe), which swaps in that character's own copy of the popup
// UI, and optionally replaces the current scene's background with one showing
// the location from the incoming character's point of view.
class PlayChar : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "PlayChar"; }

	byte _characterIndex = 0;	// index into the PCUI character list
	Common::Path _videoFile;	// replacement scene background, empty to keep the current one
};

// Nancy17. Covers a part of the viewport with a color that changes into another
// one over time, e.g. from transparent to black. It stays at the last color for
// as long as the scene does.
class FadeOverlay : public RenderActionRecord {
public:
	FadeOverlay() : RenderActionRecord(7) {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "FadeOverlay"; }

	float _seconds = 0;
	uint32 _fromColor = 0;	// ARGB
	uint32 _toColor = 0;	// ARGB
	Common::Rect _bounds;
	FlagDescription _doneFlag;

	uint32 _startTime = 0;
	bool _hasEnded = false;
};

// Nancy17. Sets one of several flags, each of which has a percent chance to be the one
class SetRandomFlag : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "SetRandomFlag"; }

	struct Choice {
		FlagDescription flag;
		int32 percent = 0;
	};

	Common::Array<Choice> _choices;
};

// Nancy17. A collection is a named list of numbers (or of names, from Nancy18 on),
// which keeps the most recent ones once it is full. Scenes use them for combinations, e.g. to add every
// button the player presses and check the last few against the solution.
class CollectionControl : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	enum Operation { kCreate = 0, kDelete = 1 };

	Common::String getRecordTypeName() const override { return "CollectionControl"; }

	byte _operation = kCreate;
	Common::String _name;
	bool _replaceExisting = false;
	uint32 _maxSize = 0;
};

// Nancy17. Adds numbers to a collection
class CollectionEdit : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "CollectionEdit"; }

	Common::String _name;
	CollectionData::Values _values;
};

// Nancy17. Compares a collection against a list of numbers or a size, and sets
// one of its flags by the result
class CollectionCheck : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	enum CheckType {
		kContainsAll	= 0,	// every listed number is in the collection, in any order
		kIsEqual		= 1,
		kSize			= 2,
		kFollowsPattern	= 3		// the collection goes through the list, over and over
	};

	Common::String getRecordTypeName() const override { return "CollectionCheck"; }

	bool check(const CollectionData::Values &collection) const;

	byte _checkType = kContainsAll;
	Common::String _name;

	// Match and mismatch; the size check has one for a bigger, an equal and a
	// smaller collection
	FlagDescription _flags[3];
	int32 _size = 0;
	CollectionData::Values _values;
};

// Nancy17. Adds a contact to the cell phone of the character being played
class AddPhoneEntry : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "AddPhoneEntry"; }

	PhoneBookData::Entry _entry;
};

// Nancy17. A hotspot that sets a different event flag for every item it can get
// clicked with, one for any other item, and one for an empty hand
class HeldItemFlags : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return _hoverCursor; }
	bool cursorSetFromScript() const override { return true; }
	bool canHaveHotspot() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "HeldItemFlags"; }

	struct ItemFlag {
		int16 itemID = -1;
		FlagDescription flag;
	};

	CursorManager::CursorType _hoverCursor = CursorManager::kHotspot;
	FlagDescription _noItemFlag;
	Common::Array<ItemFlag> _itemFlags;
	FlagDescription _otherItemFlag;
	Common::Array<HotspotDescription> _hotspots;
};

// Nancy17. Rain over a part of the viewport: a number of drops, which get placed
// anew a few times a second. The further away a drop is, the shorter and the
// fainter it gets drawn.
class Rain : public RenderActionRecord {
public:
	Rain() : RenderActionRecord(7) {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void updateGraphics() override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "Rain"; }

	void drawDrops();
	void drawDrop(const Common::Rect &src, const Common::Point &pos, int height, byte alpha);

	Common::Path _imageName;
	Common::Array<Common::Rect> _dropSrcs;
	Common::Array<HotspotDescription> _areas;	// where it rains, for every frame of the scene
	float _minDistance = 0;
	float _maxDistance = 1;
	int32 _numDrops = 0;
	uint32 _interval = 0;

	Graphics::ManagedSurface _image;
	int _areaIndex = -1;
	uint32 _nextDrawTime = 0;
};

// Nancy18. A hotspot that turns the player around to face the opposite way
class UTurn : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return _hoverCursor; }
	bool cursorSetFromScript() const override { return true; }
	bool canHaveHotspot() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "UTurn"; }

	bool isActiveOnFrame(uint16 frame) const;

	Common::Rect _hotspotRect;
	CursorManager::CursorType _hoverCursor = CursorManager::kHotspot;
	bool _excludesFrames = false;	// the hotspot is on every frame but the listed ones
	Common::Array<int16> _frames;
};

// Nancy18. A chase over a map: the places are linked to each other, and the
// record moves the one being chased to a place next to theirs, picked at random,
// that the player isn't at. Both places are kept in the value table, and the
// kind of link that got taken sets one of three event flags.
class GraphChase : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	static const uint kNumLinkTypes = 3;

	struct Link {
		int32 nodeID = 0;	// counted from 1, as are the places in the value table
		byte type = 0;
	};

	typedef Common::Array<Link> Node;

	Common::String getRecordTypeName() const override { return "GraphChase"; }

	bool canMoveTo(const Link &link, int16 chaserNode) const;

	int16 _chaserTableIndex = 0;
	int16 _chasedTableIndex = 0;
	bool _avoidsAdjacentNodes = false;	// keeps a place away from the player's, while it can
	int16 _linkTypeFlags[kNumLinkTypes] = { -1, -1, -1 };
	Common::Array<Node> _nodes;
};

// Nancy18. Throws away what a puzzle has kept under a name
class ClearSavedData : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "ClearSavedData"; }

	Common::String _name;
};

// Nancy18. Saves the game under a name, replacing the save that has it. The
// scenes make their own second chance saves with it.
class SaveNamedGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "SaveNamedGame"; }

	Common::String _name;
};

// Nancy18. Loads the game that was saved under a name
class LoadNamedGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "LoadNamedGame"; }

	Common::String _name;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MISCRECORDS_H
