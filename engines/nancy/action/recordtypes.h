/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	virtual void execute() override;
};

class SceneChange : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	SceneChangeDescription _sceneChange;

protected:
	virtual Common::String getRecordTypeName() const override { return "SceneChange"; }
};

class HotMultiframeSceneChange : public SceneChange {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	virtual Common::String getRecordTypeName() const override { return "HotMultiframeSceneChange"; }
};

class Hot1FrSceneChange : public SceneChange {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	HotspotDescription _hotspotDesc;

protected:
	virtual Common::String getRecordTypeName() const override { return "Hot1FrSceneChange"; }
};

class Hot1FrExitSceneChange : public Hot1FrSceneChange {
	virtual CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExit; }

protected:
	virtual Common::String getRecordTypeName() const override { return "Hot1FrExitSceneChange"; }
};

class HotMultiframeMultisceneChange : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "HotMultiframeMultisceneChange"; }
};

class PaletteThisScene : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	byte _paletteID;
	byte _unknownEnum; // enum w values 1-3
	uint16 _paletteStart;
	uint16 _paletteSize;

protected:
	virtual Common::String getRecordTypeName() const override { return "PaletteThisScene"; }
};

class PaletteNextScene : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	byte _paletteID;

protected:
	virtual Common::String getRecordTypeName() const override { return "PaletteNextScene"; }
};

class StartFrameNextScene : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "StartFrameNextScene"; }
};

class StartStopPlayerScrolling : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	// TODO add a Start and Stop subclass

	byte _type = 0;

protected:
	virtual Common::String getRecordTypeName() const override { return "StartStopPlayerScrolling"; }
};

class LightningOn : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "LightningOn"; }
};

class LightningOff : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "LightningOff"; }
};

class AmbientLightUp : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "AmbientLightUp"; }
};

class AmbientLightDown : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "AmbientLightDown"; }
};

class AmbientLightToTod : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "AmbientLightToTod"; }
};

class AmbientLightToTodOff : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "AmbientLightToTodOff"; }
};

class FlickerOn : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "FlickerOn"; }
};

class FlickerOff : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "FlickerOff"; }
};

class MapCall : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	virtual CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExit; }

protected:
	virtual Common::String getRecordTypeName() const override { return "MapCall"; }
};

class MapCallHot1Fr : public MapCall {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	HotspotDescription _hotspotDesc;

protected:
	virtual Common::String getRecordTypeName() const override { return "MapCallHot1Fr"; }
};

class MapCallHotMultiframe : public MapCall {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	virtual Common::String getRecordTypeName() const override { return "MapCallHotMultiframe"; }
};

class MapLocationAccess : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "MapLocationAccess"; }
};

class MapLightning : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "MapLightning"; }
};

class MapLightningOff : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "MapLightningOff"; }
};

class MapSound : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "MapSound"; }
};

class MapAviOverride : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "MapAviOverride"; }
};

class MapAviOverrideOff : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "MapAviOverrideOff"; }
};

class TextBoxWrite : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "TextBoxWrite"; }
};

class TextBoxClear : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "TextBoxClear"; }
};

class BumpPlayerClock : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "BumpPlayerClock"; }
};

class SaveContinueGame : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

protected:
	virtual Common::String getRecordTypeName() const override { return "SaveContinueGame"; }
};

class TurnOffMainRendering : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "TurnOffMainRendering"; }
};

class TurnOnMainRendering : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "TurnOnMainRendering"; }
};

class ResetAndStartTimer : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

protected:
	virtual Common::String getRecordTypeName() const override { return "ResetAndStartTimer"; }
};

class StopTimer : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

protected:
	virtual Common::String getRecordTypeName() const override { return "StopTimer"; }
};

class EventFlags : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	MultiEventFlagDescription _flags;

protected:
	virtual Common::String getRecordTypeName() const override { return "EventFlags"; }
};

class EventFlagsMultiHS : public EventFlags {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	virtual Common::String getRecordTypeName() const override { return "EventFlagsMultiHS"; }
};

class LoseGame : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

protected:
	virtual Common::String getRecordTypeName() const override { return "LoseGame"; }
};

class PushScene : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "PushScene"; }
};

class PopScene : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "PopScene"; }
};

class WinGame : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

protected:
	virtual Common::String getRecordTypeName() const override { return "WinGame"; }
};

class AddInventoryNoHS : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	uint _itemID;

protected:
	virtual Common::String getRecordTypeName() const override { return "AddInventoryNoHS"; }
};

class RemoveInventoryNoHS : public Unimplemented {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;

protected:
	virtual Common::String getRecordTypeName() const override { return "RemoveInventoryNoHS"; }
};

class DifficultyLevel : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	uint16 _difficulty = 0;
	EventFlagDescription _flag;

protected:
	virtual Common::String getRecordTypeName() const override { return "DifficultyLevel"; }
};

class ShowInventoryItem : public ActionRecord, public RenderObject {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	ShowInventoryItem(RenderObject &redrawFrom) : RenderObject(redrawFrom, 9) {}
	virtual ~ShowInventoryItem() { _fullSurface.free(); }

	virtual void init() override;
	virtual void onPause(bool pause) override;

	uint16 _objectID = 0;
	Common::String _imageName;
	Common::Array<BitmapDescription> _bitmaps;

	int16 _drawnFrameID = -1;
	Graphics::ManagedSurface _fullSurface;

protected:
	virtual Common::String getRecordTypeName() const override { return "ShowInventoryItem"; }
	virtual bool isViewportRelative() const override { return true; }
};

class PlayDigiSoundAndDie : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;
	// TODO subclass into Play and Stop (?)

	SoundDescription _sound;
	SceneChangeDescription _sceneChange;
	EventFlagDescription _flagOnTrigger;

protected:
	virtual Common::String getRecordTypeName() const override { return "PlayDigiSoundAndDie"; }
};

class PlaySoundPanFrameAnchorAndDie : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	SoundDescription _sound;

protected:
	virtual Common::String getRecordTypeName() const override { return "PlaySoundPanFrameAnchorAndDie"; }
};

class PlaySoundMultiHS : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	SoundDescription _sound; // 0x0
	SceneChangeDescription _sceneChange; // 0x22
	EventFlagDescription _flag; // 0x2A
	Common::Array<HotspotDescription> _hotspots; // 0x31

protected:
	virtual Common::String getRecordTypeName() const override { return "PlaySoundMultiHS"; }
};

class HintSystem : public ActionRecord {
public:
	virtual void readData(Common::SeekableReadStream &stream) override;
	virtual void execute() override;

	byte _characterID; // 0x00
	SoundDescription _genericSound; // 0x01

	Common::String _text;
	SceneChangeDescription _sceneChange;
	uint16 _hintID;
	int16 _hintWeight;

	void selectHint();
	void getHint(uint hint, uint difficulty);

protected:
	virtual Common::String getRecordTypeName() const override { return "HintSystem"; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_RECORDTYPES_H
