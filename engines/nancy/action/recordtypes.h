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

#include "engines/nancy/commontypes.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

class NancyEngine;

namespace Action {

class Unimplemented : public ActionRecord {
	void execute() override;
};

class SceneChange : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SceneChangeDescription _sceneChange;

protected:
	Common::String getRecordTypeName() const override { return "SceneChange"; }
};

class HotMultiframeSceneChange : public SceneChange {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	Common::String getRecordTypeName() const override { return "HotMultiframeSceneChange"; }
};

class Hot1FrSceneChange : public SceneChange {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	HotspotDescription _hotspotDesc;

protected:
	Common::String getRecordTypeName() const override { return "Hot1FrSceneChange"; }
};

class Hot1FrExitSceneChange : public Hot1FrSceneChange {
	CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExit; }

protected:
	Common::String getRecordTypeName() const override { return "Hot1FrExitSceneChange"; }
};

class HotMultiframeMultisceneChange : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "HotMultiframeMultisceneChange"; }
};

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

class PaletteNextScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _paletteID;

protected:
	Common::String getRecordTypeName() const override { return "PaletteNextScene"; }
};

class StartFrameNextScene : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "StartFrameNextScene"; }
};

class StartStopPlayerScrolling : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;
	// TODO add a Start and Stop subclass

	byte _type = 0;

protected:
	Common::String getRecordTypeName() const override { return "StartStopPlayerScrolling"; }
};

class LightningOn : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "LightningOn"; }
};

class LightningOff : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "LightningOff"; }
};

class AmbientLightUp : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "AmbientLightUp"; }
};

class AmbientLightDown : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "AmbientLightDown"; }
};

class AmbientLightToTod : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "AmbientLightToTod"; }
};

class AmbientLightToTodOff : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "AmbientLightToTodOff"; }
};

class FlickerOn : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "FlickerOn"; }
};

class FlickerOff : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "FlickerOff"; }
};

class MapCall : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExit; }

protected:
	Common::String getRecordTypeName() const override { return "MapCall"; }
};

class MapCallHot1Fr : public MapCall {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	HotspotDescription _hotspotDesc;

protected:
	Common::String getRecordTypeName() const override { return "MapCallHot1Fr"; }
};

class MapCallHotMultiframe : public MapCall {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	Common::String getRecordTypeName() const override { return "MapCallHotMultiframe"; }
};

class MapLocationAccess : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "MapLocationAccess"; }
};

class MapLightning : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "MapLightning"; }
};

class MapLightningOff : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "MapLightningOff"; }
};

class MapSound : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "MapSound"; }
};

class MapAviOverride : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "MapAviOverride"; }
};

class MapAviOverrideOff : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "MapAviOverrideOff"; }
};

class TextBoxWrite : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "TextBoxWrite"; }
};

class TextBoxClear : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "TextBoxClear"; }
};

class BumpPlayerClock : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "BumpPlayerClock"; }
};

class SaveContinueGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "SaveContinueGame"; }
};

class TurnOffMainRendering : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "TurnOffMainRendering"; }
};

class TurnOnMainRendering : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "TurnOnMainRendering"; }
};

class ResetAndStartTimer : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "ResetAndStartTimer"; }
};

class StopTimer : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "StopTimer"; }
};

class EventFlags : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	MultiEventFlagDescription _flags;

protected:
	Common::String getRecordTypeName() const override { return "EventFlags"; }
};

class EventFlagsMultiHS : public EventFlags {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	Common::String getRecordTypeName() const override { return "EventFlagsMultiHS"; }
};

class LoseGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "LoseGame"; }
};

class PushScene : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "PushScene"; }
};

class PopScene : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "PopScene"; }
};

class WinGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "WinGame"; }
};

class AddInventoryNoHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint _itemID;

protected:
	Common::String getRecordTypeName() const override { return "AddInventoryNoHS"; }
};

class RemoveInventoryNoHS : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "RemoveInventoryNoHS"; }
};

class DifficultyLevel : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _difficulty = 0;
	EventFlagDescription _flag;

protected:
	Common::String getRecordTypeName() const override { return "DifficultyLevel"; }
};

class ShowInventoryItem : public ActionRecord, public RenderObject {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	ShowInventoryItem() : RenderObject(9) {}
	virtual ~ShowInventoryItem() { _fullSurface.free(); }

	void init() override;
	void onPause(bool pause) override;

	uint16 _objectID = 0;
	Common::String _imageName;
	Common::Array<BitmapDescription> _bitmaps;

	int16 _drawnFrameID = -1;
	Graphics::ManagedSurface _fullSurface;

protected:
	Common::String getRecordTypeName() const override { return "ShowInventoryItem"; }
	bool isViewportRelative() const override { return true; }
};

class PlayDigiSoundAndDie : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	// TODO subclass into Play and Stop (?)

	SoundDescription _sound;
	SceneChangeDescription _sceneChange;
	EventFlagDescription _flagOnTrigger;

protected:
	Common::String getRecordTypeName() const override { return "PlayDigiSoundAndDie"; }
};

class PlaySoundPanFrameAnchorAndDie : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound;

protected:
	Common::String getRecordTypeName() const override { return "PlaySoundPanFrameAnchorAndDie"; }
};

class PlaySoundMultiHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound; // 0x0
	SceneChangeDescription _sceneChange; // 0x22
	EventFlagDescription _flag; // 0x2A
	Common::Array<HotspotDescription> _hotspots; // 0x31

protected:
	Common::String getRecordTypeName() const override { return "PlaySoundMultiHS"; }
};

class HintSystem : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _characterID; // 0x00
	SoundDescription _genericSound; // 0x01

	Common::String _text;
	SceneChangeDescription _sceneChange;
	uint16 _hintID;
	int16 _hintWeight;

	void selectHint();
	void getHint(uint hint, uint difficulty);

protected:
	Common::String getRecordTypeName() const override { return "HintSystem"; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_RECORDTYPES_H
