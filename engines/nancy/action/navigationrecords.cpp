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
#include "engines/nancy/util.h"

#include "engines/nancy/action/navigationrecords.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void SceneChange::readData(Common::SeekableReadStream &stream) {
	_sceneChange.readData(stream);
}

void SceneChange::execute() {
	NancySceneState.changeScene(_sceneChange);
	_isDone = true;
}

void HotMultiframeSceneChange::readData(Common::SeekableReadStream &stream) {
	SceneChange::readData(stream);
	uint16 numHotspots = stream.readUint16LE();

	_hotspots.reserve(numHotspots);
	for (uint i = 0; i < numHotspots; ++i) {
		_hotspots.push_back(HotspotDescription());
		HotspotDescription &newDesc = _hotspots[i];
		newDesc.readData(stream);
	}
}

void HotMultiframeSceneChange::execute() {
	switch (_state) {
	case kBegin:
		// turn main rendering on
		_state = kRun;
		// fall through
	case kRun:
		_hasHotspot = false;
		for (uint i = 0; i < _hotspots.size(); ++i) {
			if (_hotspots[i].frameID == NancySceneState.getSceneInfo().frameID) {
				_hasHotspot = true;
				_hotspot = _hotspots[i].coords;
			}
		}
		break;
	case kActionTrigger:
		SceneChange::execute();
		break;
	}
}

void Hot1FrSceneChange::readData(Common::SeekableReadStream &stream) {
	if (!_isTerse) {
		SceneChange::readData(stream);
		_hotspotDesc.readData(stream);
	} else {
		_sceneChange.sceneID = stream.readUint16LE();
		_sceneChange.continueSceneSound = kContinueSceneSound;
		_sceneChange.listenerFrontVector.set(0, 0, 1);
		readRect(stream, _hotspotDesc.coords);
	}
}

void Hot1FrSceneChange::execute() {
	switch (_state) {
	case kBegin:
		_hotspot = _hotspotDesc.coords;
		_state = kRun;
		// fall through
	case kRun:
		if (_hotspotDesc.frameID == NancySceneState.getSceneInfo().frameID) {
			_hasHotspot = true;
		} else {
			_hasHotspot = false;
		}
		break;
	case kActionTrigger:
		SceneChange::execute();
		break;
	}
}

void HotMultiframeMultisceneChange::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() <= kGameTypeNancy2) {
		_onTrue._sceneChange.readData(stream);
		_onFalse._sceneChange.readData(stream);
	} else {
		_onTrue.readData(stream, true);
		_onFalse.readData(stream, true);
	}

	_condType = stream.readByte();
	_conditionID = stream.readUint16LE();
	_conditionPayload = stream.readByte();
	uint numHotspots = stream.readUint16LE();

	_hotspots.resize(numHotspots);

	for (uint i = 0; i < numHotspots; ++i) {
		_hotspots[i].readData(stream);
	}
}

void HotMultiframeMultisceneChange::execute() {
	switch (_state) {
	case kBegin:
		// set something to 1
		_state = kRun;
		// fall through
	case kRun:
		_hasHotspot = false;

		for (HotspotDescription &desc : _hotspots) {
			if (desc.frameID == NancySceneState.getSceneInfo().frameID) {
				_hotspot = desc.coords;
				_hasHotspot = true;
			}
		}

		break;
	case kActionTrigger: {
		bool conditionMet = false;
		switch (_condType) {
		case kFlagEvent:
			if (NancySceneState.getEventFlag(_conditionID, _conditionPayload)) {
				conditionMet = true;
			}
			break;
		case kFlagInventory:
			if (NancySceneState.hasItem(_conditionID) == _conditionPayload) {
				conditionMet = true;
			}
			break;
		case kFlagCursor:
			if (NancySceneState.getHeldItem() == _conditionPayload) {
				conditionMet = true;
			}
			break;
		}

		if (conditionMet) {
			_onTrue.execute();
		} else {
			_onFalse.execute();
		}

		break;
	}
	}
}

void HotMultiframeMultisceneCursorTypeSceneChange::readData(Common::SeekableReadStream &stream) {
	uint16 numScenes = stream.readUint16LE();
	_scenes.resize(numScenes);
	_cursorTypes.resize(numScenes);
	for (uint i = 0; i < numScenes; ++i) {
		_cursorTypes[i] = stream.readUint16LE();
		_scenes[i].readData(stream);
	}

	stream.skip(2);
	_defaultScene.readData(stream);

	uint16 numHotspots = stream.readUint16LE();
	_hotspots.resize(numHotspots);
	for (uint i = 0; i < numHotspots; ++i) {
		_hotspots[i].readData(stream);
	}
}

void HotMultiframeMultisceneCursorTypeSceneChange::execute() {
	switch (_state) {
	case kBegin:
		// turn main rendering on
		_state = kRun;
		// fall through
	case kRun:
		_hasHotspot = false;
		for (uint i = 0; i < _hotspots.size(); ++i) {
			if (_hotspots[i].frameID == NancySceneState.getSceneInfo().frameID) {
				_hasHotspot = true;
				_hotspot = _hotspots[i].coords;
			}
		}
		break;
	case kActionTrigger:
		for (uint i = 0; i < _cursorTypes.size(); ++i) {
			if (NancySceneState.getHeldItem() == _cursorTypes[i]) {
				NancySceneState.changeScene(_scenes[i]);

				_isDone = true;
				return;
			}
		}

		NancySceneState.changeScene(_defaultScene);
		_isDone = true;
		break;
	}
}

void MapCall::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void MapCall::execute() {
	_execType = kRepeating;
	NancySceneState.requestStateChange(NancyState::kMap);
	finishExecution();
}

void MapCallHot1Fr::readData(Common::SeekableReadStream &stream) {
	_hotspotDesc.readData(stream);
}

void MapCallHot1Fr::execute() {
	switch (_state) {
	case kBegin:
		_hotspot = _hotspotDesc.coords;
		_state = kRun;
		// fall through
	case kRun:
		if (_hotspotDesc.frameID == NancySceneState.getSceneInfo().frameID) {
			_hasHotspot = true;
		}
		break;
	case kActionTrigger:
		MapCall::execute();
		break;
	}
}

void MapCallHotMultiframe::readData(Common::SeekableReadStream &stream) {
	uint16 numDescs = stream.readUint16LE();
	_hotspots.reserve(numDescs);
	for (uint i = 0; i < numDescs; ++i) {
		_hotspots.push_back(HotspotDescription());
		_hotspots[i].readData(stream);
	}
}

void MapCallHotMultiframe::execute() {
	switch (_state) {
	case kBegin:
		_state = kRun;
		// fall through
	case kRun:
		_hasHotspot = false;
		for (uint i = 0; i < _hotspots.size(); ++i) {
			if (_hotspots[i].frameID == NancySceneState.getSceneInfo().frameID) {
				_hasHotspot = true;
				_hotspot = _hotspots[i].coords;
			}
		}
		break;
	case kActionTrigger:
		MapCall::execute();
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
