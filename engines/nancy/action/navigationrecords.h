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

#ifndef NANCY_ACTION_NAVIGATIONRECORDS_H
#define NANCY_ACTION_NAVIGATIONRECORDS_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Simply changes the scene
class SceneChange : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SceneChangeDescription _sceneChange;

protected:
	Common::String getRecordTypeName() const override { return "SceneChange"; }
};

// Changes the scene when clicked. Hotspot can move along with scene background frame.
// Nancy4 introduced several sub-types with a specific mouse cursor to show when
// hovering; all of them are handled in this class as well.
class HotMultiframeSceneChange : public SceneChange {
public:
	HotMultiframeSceneChange(CursorManager::CursorType hoverCursor) : _hoverCursor(hoverCursor) {}
	virtual ~HotMultiframeSceneChange() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return _hoverCursor; }

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

// Changes the scene when clicked; does _not_ move with scene background.
// Nancy4 introduced several sub-types with a specific mouse cursor to show when
// hovering; all of them are handled in this class as well.
class Hot1FrSceneChange : public SceneChange {
public:
	Hot1FrSceneChange(CursorManager::CursorType hoverCursor) : _hoverCursor(hoverCursor) {}
	virtual ~Hot1FrSceneChange() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return _hoverCursor; }

	HotspotDescription _hotspotDesc;
	bool _isTerse = false;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override {
		if (_isTerse) {
			return "HotSceneChangeTerse";
		}

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
		case CursorManager::kMoveLeft:
			return "Hot1FrLeftSceneChange";
		case CursorManager::kMoveRight:
			return "Hot1FrRightSceneChange";
		default:
			return "Hot1FrSceneChange";
		}
	}

	CursorManager::CursorType _hoverCursor;
};

// Changes the scene when clicked. Hotspot can move along with scene background frame.
// However, the scene it changes to can be one of two options, picked based on
// a provided condition.
class HotMultiframeMultisceneChange : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	SceneChangeWithFlag _onTrue;
	SceneChangeWithFlag _onFalse;
	byte _condType;
	uint16 _conditionID;
	byte _conditionPayload;
	Common::Array<HotspotDescription> _hotspots;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "HotMultiframeMultisceneChange"; }
};

// Changes the scene when clicked. Hotspot can move along with scene background frame.
// However, the scene it changes to can be one of several options, picked based on
// the item the player is currently holding.
class HotMultiframeMultisceneCursorTypeSceneChange : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<SceneChangeDescription> _scenes;
	Common::Array<uint16> _cursorTypes;

	SceneChangeDescription _defaultScene;
	Common::Array<HotspotDescription> _hotspots;

protected:
	Common::String getRecordTypeName() const override { return "HotMultiframeMultisceneCursorTypeSceneChange"; }
};

// Simply switches to the Map state. TVD/nancy1 only.
class MapCall : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	CursorManager::CursorType getHoverCursor() const override { return CursorManager::kExit; }

protected:
	Common::String getRecordTypeName() const override { return "MapCall"; }
};

// Switches to the Map state when clicked; does _not_ move with background frame. TVD/nancy1 only.
class MapCallHot1Fr : public MapCall {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	HotspotDescription _hotspotDesc;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "MapCallHot1Fr"; }
};

// Switches to the Map state when clicked. Hotspot can move along with scene background frame. TVD/nancy1 only.
class MapCallHotMultiframe : public MapCall {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Array<HotspotDescription> _hotspots;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "MapCallHotMultiframe"; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_NAVIGATIONRECORDS_H
