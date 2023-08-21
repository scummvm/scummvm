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
	HotMultiframeSceneChange(CursorManager::CursorType hoverCursor) : _hoverCursor(hoverCursor) {}
	virtual ~HotMultiframeSceneChange() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override {
		switch (_hoverCursor) {
		case CursorManager::kMoveForward:
			return "HotMultiframeForwardSceneChange";
		case CursorManager::kMoveUp:
			return "HotMultiframeUpSceneChange";
		case CursorManager::kMoveDown:
			return "HotMultiframeDownSceneChange";
		default:
			return "HotMultiframeSceneChange";
		}
	}

	CursorManager::CursorType _hoverCursor;
};

class Hot1FrSceneChange : public SceneChange {
public:
	Hot1FrSceneChange(CursorManager::CursorType hoverCursor) : _hoverCursor(hoverCursor) {}
	virtual ~Hot1FrSceneChange() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	
	CursorManager::CursorType getHoverCursor() const override { return _hoverCursor; }

	HotspotDescription _hotspotDesc;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override {
		switch (_hoverCursor) {
		case CursorManager::kExit:
			return "Hot1FrExitSceneChange";
		case CursorManager::kMoveForward:
			return "Hot1FrForwardSceneChange";
		case CursorManager::kMoveBackward:
			return "Hot1FrBackSceneChange";
		case CursorManager::kMoveUp:
			return "Hot1FrUpSceneChange";
		case CursorManager::kMoveDown:
			return "Hot1FrDownSceneChange";
		case CursorManager::kTurnLeft:
			return "Hot1FrLeftSceneChange";
		case CursorManager::kTurnRight:
			return "Hot1FrUpSceneChange";
		default:
			return "Hot1FrSceneChange";
		}
	}

	CursorManager::CursorType _hoverCursor;
};

class HotMultiframeMultisceneChange : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SceneChangeDescription _onTrue;
	SceneChangeDescription _onFalse;
	byte _condType;
	uint16 _conditionID;
	byte _conditionPayload;
	Common::Array<HotspotDescription> _hotspots;

protected:
	bool canHaveHotspot() const override { return true; }
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
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "MapCallHot1Fr"; }
};

class MapCallHotMultiframe : public MapCall {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "MapCallHotMultiframe"; }
};

class TextBoxWrite : public ActionRecord {
public:
	virtual ~TextBoxWrite();

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::String _text;

protected:
	Common::String getRecordTypeName() const override { return "TextBoxWrite"; }
};

class TextBoxClear : public Unimplemented {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "TextBoxClear"; }
};

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
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "EventFlagsMultiHS"; }
};

class LoseGame : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "LoseGame"; }
};

class PushScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "PushScene"; }
};

class PopScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

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

class RemoveInventoryNoHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint _itemID;

protected:
	Common::String getRecordTypeName() const override { return "RemoveInventoryNoHS"; }
};

class DifficultyLevel : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _difficulty = 0;
	FlagDescription _flag;

protected:
	Common::String getRecordTypeName() const override { return "DifficultyLevel"; }
};

class ShowInventoryItem : public RenderActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	ShowInventoryItem() : RenderActionRecord(9) {}
	virtual ~ShowInventoryItem() { _fullSurface.free(); }

	void init() override;

	uint16 _objectID = 0;
	Common::String _imageName;
	Common::Array<BitmapDescription> _bitmaps;

	int16 _drawnFrameID = -1;
	Graphics::ManagedSurface _fullSurface;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "ShowInventoryItem"; }
	bool isViewportRelative() const override { return true; }
};

class PlayDigiSoundAndDie : public ActionRecord {
public:
	PlayDigiSoundAndDie() {}
	~PlayDigiSoundAndDie() { delete _soundEffect; }
	
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SoundDescription _sound;
	SoundEffectDescription *_soundEffect = nullptr;
	SceneChangeDescription _sceneChange;
	FlagDescription _flagOnTrigger;

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
	FlagDescription _flag; // 0x2A
	Common::Array<HotspotDescription> _hotspots; // 0x31

protected:
	Common::String getRecordTypeName() const override { return "PlaySoundMultiHS"; }
};

class StopSound : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint _channelID;
	SceneChangeWithFlag _sceneChange;

protected:
	Common::String getRecordTypeName() const override { return "StopSound"; }
};

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
