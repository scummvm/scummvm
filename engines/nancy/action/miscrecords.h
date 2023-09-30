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

namespace Nancy {

class NancyEngine;

namespace Action {

class Unimplemented : public ActionRecord {
	void execute() override;
};

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

protected:
	Common::String getRecordTypeName() const override { return "SpecialEffect"; }
};

// Changes the selected value inside the TableData. Value can be incremented, decremented, or not changed.
// Also responsible for checking whether all values are correct (as described in the TABL chunk). Nancy6 and up.
class TableIndexSetValueHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return (CursorManager::CursorType)_cursorType; }

protected:
	Common::String getRecordTypeName() const override { return "TableIndexSetValueHS"; }

	uint16 _tableIndex = 0;
	byte _valueChangeType = kNoChangeTableValue;
	int16 _entryCorrectFlagID = -1;
	int16 _allEntriesCorrectFlagID = -1;

	MultiEventFlagDescription _flags;
	uint16 _cursorType = 1;
	Common::Array<HotspotDescription> _hotspots;
};

// Adds a caption to the textbox.
class TextBoxWrite : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::String _text;

protected:
	Common::String getRecordTypeName() const override { return "TextBoxWrite"; }
};

// Clears the textbox. Used very rarely.
class TextboxClear : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "TextboxClear"; }
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
// how much time has passed since the timer was started.
class ResetAndStartTimer : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "ResetAndStartTimer"; }
};

// Stops the timer.
class StopTimer : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "StopTimer"; }
};

// Sets up to 10 flags at once.
class EventFlags : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	MultiEventFlagDescription _flags;

protected:
	Common::String getRecordTypeName() const override { return "EventFlags"; }
};

// Sets up to 10 flags when clicked. Hotspot can move alongside background frame.
class EventFlagsMultiHS : public EventFlags {
public:
	EventFlagsMultiHS(bool isCursor) : _isCursor(isCursor) {}
	virtual ~EventFlagsMultiHS() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return _hoverCursor; }

	CursorManager::CursorType _hoverCursor = CursorManager::kHotspot;
	Common::Array<HotspotDescription> _hotspots;

	bool _isCursor;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return _isCursor ? "EventFlagsCursorHS" : "EventFlagsMultiHS"; }
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

// Simply adds an item to the player's inventory.
class AddInventoryNoHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _itemID = 0;
	bool _setCursor = false;
	bool _forceCursor = false;

protected:
	Common::String getRecordTypeName() const override { return "AddInventoryNoHS"; }
};

// Simply removes an item from the player's inventory.
class RemoveInventoryNoHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint _itemID;

protected:
	Common::String getRecordTypeName() const override { return "RemoveInventoryNoHS"; }
};

// Sets the difficulty level for the current save. Only appears at the start of the game.
// First appears in nancy1. Nancy1 and nancy2 have three difficulty values, while later games
// only have two: 0 and 2.
class DifficultyLevel : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _difficulty = 0;
	FlagDescription _flag;

protected:
	Common::String getRecordTypeName() const override { return "DifficultyLevel"; }
};

// Displays a static image inside the viewport. The static image corresponds to an
// inventory item, and is only displayed if the item is not in the player's possesion.
// On click, it hides the image and adds the item to the inventory.
class ShowInventoryItem : public RenderActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	ShowInventoryItem() : RenderActionRecord(9) {}
	virtual ~ShowInventoryItem() { _fullSurface.free(); }

	void init() override;

	uint16 _objectID = 0;
	Common::String _imageName;
	Common::Array<FrameBlitDescription> _blitDescriptions;

	int16 _drawnFrameID = -1;
	Graphics::ManagedSurface _fullSurface;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "ShowInventoryItem"; }
	bool isViewportRelative() const override { return true; }
};

// When clicking an ActionRecord hotspot with a kItem dependency, the engine
// checks if the required item is currently being held; when it isn't, it plays
// a specific sound to inform the player they need some item. This AR changes that
// sound and its related caption (or stops it from playing entirely).
class InventorySoundOverride : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _command = 0;
	uint16 _itemID = 0;
	SoundDescription _sound;
	Common::String _caption;

protected:
	Common::String getRecordTypeName() const override { return "InventorySoundOverride"; }
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

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_RECORDTYPES_H
