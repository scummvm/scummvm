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

#include "common/config-manager.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/recordtypes.h"
#include "engines/nancy/action/responses.cpp"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void Unimplemented::execute() {
	debugC(Nancy::kDebugActionRecord, "Unimplemented Action Record type %s", getRecordTypeName().c_str());
	_isDone = true;
}

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
	SceneChange::readData(stream);
	_hotspotDesc.readData(stream);
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
	stream.seek(0x14, SEEK_CUR);
	uint size = stream.readUint16LE() * 0x12;
	stream.skip(size);
}

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

void StartFrameNextScene::readData(Common::SeekableReadStream &stream) {
	stream.skip(4);
}

void StartStopPlayerScrolling::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void LightningOn::readData(Common::SeekableReadStream &stream) {
	stream.skip(0xA);
}

void LightningOff::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void AmbientLightUp::readData(Common::SeekableReadStream &stream) {
	stream.skip(0x12);
}

void AmbientLightDown::readData(Common::SeekableReadStream &stream) {
	stream.skip(0x12);
}

void AmbientLightToTod::readData(Common::SeekableReadStream &stream) {
	stream.skip(0x1C);
}

void AmbientLightToTodOff::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void FlickerOn::readData(Common::SeekableReadStream &stream) {
	stream.skip(0xA);
}

void FlickerOff::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
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

void MapLocationAccess::readData(Common::SeekableReadStream &stream) {
	stream.skip(4);
}

void MapLightning::readData(Common::SeekableReadStream &stream) {
	stream.skip(0xA);
}

void MapLightningOff::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void MapSound::readData(Common::SeekableReadStream &stream) {
	stream.skip(0x10);
}

void MapAviOverride::readData(Common::SeekableReadStream &stream) {
	stream.skip(2);
}

void MapAviOverrideOff::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void TextBoxWrite::readData(Common::SeekableReadStream &stream) {
	uint16 size = stream.readUint16LE();
	stream.skip(size);

	if (size > 10000) {
		error("Action Record atTextboxWrite has too many text box chars: %d", size);
	}
}

void TextBoxClear::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void BumpPlayerClock::readData(Common::SeekableReadStream &stream) {
	stream.skip(5);
}

void SaveContinueGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void SaveContinueGame::execute() {
	if (ConfMan.getBool("second_chance")) {
		if (_state == kBegin) {
			// Slight hack, skip first call to let the graphics render once and provide the correct thumbnail
			_state = kRun;
			return;
		}

		g_nancy->saveGameState(g_nancy->getAutosaveSlot(), "Second Chance", true);
	}
	_isDone = true;
}

void TurnOffMainRendering::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void TurnOnMainRendering::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void ResetAndStartTimer::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void ResetAndStartTimer::execute() {
	NancySceneState.resetAndStartTimer();
	_isDone = true;
}

void StopTimer::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void StopTimer::execute() {
	NancySceneState.stopTimer();
	_isDone = true;
}

void EventFlags::readData(Common::SeekableReadStream &stream) {
	_flags.readData(stream);
}

void EventFlags::execute() {
	_flags.execute();
	_isDone = true;
}

void EventFlagsMultiHS::readData(Common::SeekableReadStream &stream) {
	EventFlags::readData(stream);
	uint16 numHotspots = stream.readUint16LE();

	_hotspots.reserve(numHotspots);
	for (uint16 i = 0; i < numHotspots; ++i) {
		_hotspots.push_back(HotspotDescription());
		HotspotDescription &newDesc = _hotspots[i];
		newDesc.readData(stream);
	}
}

void EventFlagsMultiHS::execute() {
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
		_hasHotspot = false;
		EventFlags::execute();
		finishExecution();
		break;
	}
}

void LoseGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void LoseGame::execute() {
	g_nancy->_sound->stopAndUnloadSpecificSounds();
	g_nancy->setState(NancyState::kMainMenu);
	NancySceneState.resetStateToInit();
	_isDone = true;
}

void PushScene::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void PopScene::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void WinGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void WinGame::execute() {
	g_nancy->_sound->stopAndUnloadSpecificSounds();
	g_nancy->setState(NancyState::kCredits, NancyState::kMainMenu);

	// TODO replace with destroy()?
	NancySceneState.resetStateToInit();
	_isDone = true;
}

void AddInventoryNoHS::readData(Common::SeekableReadStream &stream) {
	_itemID = stream.readUint16LE();
}

void AddInventoryNoHS::execute() {
	if (NancySceneState.hasItem(_itemID) == kFalse) {
		NancySceneState.addItemToInventory(_itemID);
	}

	_isDone = true;
}

void RemoveInventoryNoHS::readData(Common::SeekableReadStream &stream) {
	stream.skip(2);
}

void DifficultyLevel::readData(Common::SeekableReadStream &stream) {
	_difficulty = stream.readUint16LE();
	_flag.label = stream.readSint16LE();
	_flag.flag = (NancyFlag)stream.readUint16LE();
}

void DifficultyLevel::execute() {
	NancySceneState.setDifficulty(_difficulty);
	NancySceneState.setEventFlag(_flag);
	_isDone = true;
}

void ShowInventoryItem::init() {
	g_nancy->_resource->loadImage(_imageName, _fullSurface);

	_drawSurface.create(_fullSurface, _bitmaps[0].src);

	RenderObject::init();
}

void ShowInventoryItem::readData(Common::SeekableReadStream &stream) {
	_objectID = stream.readUint16LE();
	readFilename(stream, _imageName);

	uint16 numFrames = stream.readUint16LE();

	_bitmaps.reserve(numFrames);
	for (uint i = 0; i < numFrames; ++i) {
		_bitmaps.push_back(BitmapDescription());
		_bitmaps[i].readData(stream);
	}
}

void ShowInventoryItem::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		int newFrame = -1;

		for (uint i = 0; i < _bitmaps.size(); ++i) {
			if (_bitmaps[i].frameID == NancySceneState.getSceneInfo().frameID) {
				newFrame = i;
				break;
			}
		}

		if (newFrame != _drawnFrameID) {
			_drawnFrameID = newFrame;

			if (newFrame != -1) {
				_hasHotspot = true;
				_hotspot = _bitmaps[newFrame].dest;
				_drawSurface.create(_fullSurface, _bitmaps[newFrame].src);
				_screenPosition = _bitmaps[newFrame].dest;
				setVisible(true);
			} else {
				_hasHotspot = false;
				setVisible(false);
			}
		}

		break;
	}
	case kActionTrigger:
		g_nancy->_sound->playSound("BUOK");
		NancySceneState.addItemToInventory(_objectID);
		setVisible(false);
		_hasHotspot = false;
		finishExecution();
		break;
	}
}

void ShowInventoryItem::onPause(bool pause) {
	if (!pause) {
		registerGraphics();
	}
}

void PlayDigiSoundAndDie::readData(Common::SeekableReadStream &stream) {
	_sound.read(stream, SoundDescription::kDIGI);
	_sceneChange.readData(stream);

	if (g_nancy->getGameType() == kGameTypeVampire) {
		stream.skip(1);
		_sceneChange.doNotStartSound = stream.readUint16LE();
	}
	
	_flagOnTrigger.label = stream.readSint16LE();
	_flagOnTrigger.flag = (NancyFlag)stream.readByte();
	stream.skip(2);
}

void PlayDigiSoundAndDie::execute() {
	switch (_state) {
	case kBegin:
		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);
		_state = kRun;
		break;
	case kRun:
		if (!g_nancy->_sound->isSoundPlaying(_sound)) {
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger:
		if (_sceneChange.sceneID != 9999) {
			NancySceneState.changeScene(_sceneChange);
		}

		NancySceneState.setEventFlag(_flagOnTrigger);
		g_nancy->_sound->stopSound(_sound);

		finishExecution();
		break;
	}
}

void PlaySoundPanFrameAnchorAndDie::readData(Common::SeekableReadStream &stream) {
	_sound.read(stream, SoundDescription::kDIGI);
	stream.skip(2);
}

void PlaySoundPanFrameAnchorAndDie::execute() {
	g_nancy->_sound->loadSound(_sound, true);
	g_nancy->_sound->playSound(_sound);
	_isDone = true;
}

void PlaySoundMultiHS::readData(Common::SeekableReadStream &stream) {
	_sound.read(stream, SoundDescription::kNormal);

	if (g_nancy->getGameType() != kGameTypeVampire) {
		_sceneChange.readData(stream);
		_flag.label = stream.readSint16LE();
		_flag.flag = (NancyFlag)stream.readByte();
		stream.skip(2);
	} else {
		_flag.label = -1;
		_sceneChange.sceneID = 9999;
	}

	uint16 numHotspots = stream.readUint16LE();

	_hotspots.reserve(numHotspots);
	for (uint i = 0; i < numHotspots; ++i) {
		_hotspots.push_back(HotspotDescription());
		_hotspots.back().frameID = stream.readUint16LE();
		readRect(stream, _hotspots.back().coords);
	}
}

void PlaySoundMultiHS::execute() {
	switch (_state) {
	case kBegin:
		_state = kRun;
		// fall through
	case kRun: {
		_hasHotspot = false;
		uint currentFrame = NancySceneState.getSceneInfo().frameID;

		for (uint i = 0; i < _hotspots.size(); ++i) {
			if (_hotspots[i].frameID == currentFrame) {
				_hotspot = _hotspots[i].coords;
				_hasHotspot = true;
				break;
			}
		}

		break;
	}
	case kActionTrigger:
		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);
		NancySceneState.changeScene(_sceneChange);
		NancySceneState.setEventFlag(_flag);
		finishExecution();
		break;
	}
}

void HintSystem::readData(Common::SeekableReadStream &stream) {
	_characterID = stream.readByte();
	_genericSound.read(stream, SoundDescription::kNormal);
}

void HintSystem::execute() {
	switch (_state) {
	case kBegin:
		if (NancySceneState.getHintsRemaining() > 0) {
			selectHint();
		} else {
			getHint(0, NancySceneState.getDifficulty());
		}

		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(_text);

		g_nancy->_sound->loadSound(_genericSound);
		g_nancy->_sound->playSound(_genericSound);
		_state = kRun;
		break;
	case kRun:
		if (!g_nancy->_sound->isSoundPlaying(_genericSound)) {
			g_nancy->_sound->stopSound(_genericSound);
			_state = kActionTrigger;
		} else {
			break;
		}

		// fall through
	case kActionTrigger:
		NancySceneState.useHint(_hintID, _hintWeight);
		NancySceneState.getTextbox().clear();

		NancySceneState.changeScene(_sceneChange);

		_isDone = true;
		break;
	}
}

void HintSystem::selectHint() {
	for (const auto &hint : nancy1Hints) {
		if (hint.characterID != _characterID) {
			continue;
		}

		bool satisfied = true;

		for (const auto &flag : hint.flagConditions) {
			if (flag.label == -1) {
				break;
			}

			if (!NancySceneState.getEventFlag(flag.label, flag.flag)) {
				satisfied = false;
				break;
			}
		}

		for (const auto &inv : hint.inventoryCondition) {
			if (inv.label == -1) {
				break;
			}

			if (NancySceneState.hasItem(inv.label) != inv.flag) {
				satisfied = false;
				break;
			}
		}

		if (satisfied) {
			getHint(hint.hintID, NancySceneState.getDifficulty());
			break;
		}
	}
}

void HintSystem::getHint(uint hint, uint difficulty) {
	uint fileOffset = 0;
	if (_characterID < 3) {
		fileOffset = nancy1HintOffsets[_characterID];
	}

	fileOffset += 0x288 * hint;

	Common::File file;
	file.open("game.exe");
	file.seek(fileOffset);

	_hintID = file.readSint16LE();
	_hintWeight = file.readSint16LE();

	file.seek(difficulty * 10, SEEK_CUR);

	readFilename(file, _genericSound.name);

	file.seek(-(difficulty * 10) - 10, SEEK_CUR);
	file.seek(30 + difficulty * 200, SEEK_CUR);

	char textBuf[200];
	file.read(textBuf, 200);
	textBuf[199] = '\0';
	_text = textBuf;

	file.seek(-(difficulty * 200) - 200, SEEK_CUR);
	file.seek(600, SEEK_CUR);

	_sceneChange.readData(file);
}

} // End of namespace Action
} // End of namespace Nancy
