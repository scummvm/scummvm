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

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_NAVIGATIONRECORDS_H
