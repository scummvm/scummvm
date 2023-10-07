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
#include "engines/nancy/sound.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/miscrecords.h"

#include "engines/nancy/state/scene.h"

#include "common/events.h"
#include "common/config-manager.h"

namespace Nancy {
namespace Action {

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

void LightningOn::readData(Common::SeekableReadStream &stream) {
	_distance = stream.readSint16LE();
	_pulseTime = stream.readUint16LE();
	_rgbPercent = stream.readSint16LE();
	stream.skip(4);
}

void SpecialEffect::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() <= kGameTypeNancy6) {
		_type = stream.readByte();
		_fadeToBlackTime = stream.readUint16LE();
		_frameTime = stream.readUint16LE();
	} else {
		_type = stream.readByte();
		_totalTime = stream.readUint16LE();
		_fadeToBlackTime = stream.readUint16LE();
		readRect(stream, _rect);
	}
}

void SpecialEffect::execute() {
	if (g_nancy->getGameType() <= kGameTypeNancy6) {
		NancySceneState.specialEffect(_type, _fadeToBlackTime, _frameTime);
	} else {
		NancySceneState.specialEffect(_type, _totalTime, _fadeToBlackTime, _rect);
	}
	
	_isDone = true;
}

void TableIndexSetValueHS::readData(Common::SeekableReadStream &stream) {
	_tableIndex = stream.readUint16LE();
	_valueChangeType = stream.readByte();
	_entryCorrectFlagID = stream.readSint16LE();
	_allEntriesCorrectFlagID = stream.readSint16LE();

	_flags.readData(stream);
	_cursorType = stream.readUint16LE();
	uint16 numHotspots = stream.readUint16LE();
	_hotspots.resize(numHotspots);
	for (uint i = 0; i < numHotspots; ++i) {
		_hotspots[i].readData(stream);
	}
}

void TableIndexSetValueHS::execute() {
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
	case kActionTrigger: {
		TableData *playerTable = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
		assert(playerTable);
		auto *tabl = GetEngineData(TABL);
		assert(tabl);

		// Edit table. Values start from 1!
		switch (_valueChangeType) {
		case kNoChangeTableValue:
			break;
		case kIncrementTableValue:
			++playerTable->currentIDs[_tableIndex - 1];
			if (playerTable->currentIDs[_tableIndex - 1] >= playerTable->currentIDs.size() + 1) {
				playerTable->currentIDs[_tableIndex - 1] = 1;
			}
			break;
		case kDecrementTableValue:
			--playerTable->currentIDs[_tableIndex - 1];
			if (playerTable->currentIDs[_tableIndex - 1] == 0) {
				playerTable->currentIDs[_tableIndex - 1] = playerTable->currentIDs.size();
			}
			
			break;
		}

		// Check for correctness...

		// ...of current index only...
		if (playerTable->currentIDs[_tableIndex] == tabl->correctIDs[_tableIndex]) {
			NancySceneState.setEventFlag(_entryCorrectFlagID, g_nancy->_true);
		} else {
			NancySceneState.setEventFlag(_entryCorrectFlagID, g_nancy->_false);
		}

		// ..and of all indices
		bool allCorrect = true;
		for (uint i = 0; i < tabl->correctIDs.size(); ++i) {
			if (playerTable->currentIDs[i] != tabl->correctIDs[i]) {
				allCorrect = false;
				break;
			}
		}

		if (allCorrect) {
			NancySceneState.setEventFlag(_allEntriesCorrectFlagID, g_nancy->_true);
		} else {
			NancySceneState.setEventFlag(_allEntriesCorrectFlagID, g_nancy->_false);
		}

		_flags.execute();
		finishExecution();
	}
	}
}

void LightningOn::execute() {
	NancySceneState.beginLightning(_distance, _pulseTime, _rgbPercent);
	_isDone = true;
}

void TextBoxWrite::readData(Common::SeekableReadStream &stream) {
	uint16 size = stream.readUint16LE();

	if (size > 10000) {
		error("Action Record atTextboxWrite has too many text box chars: %d", size);
	}

	char *buf = new char[size];
	stream.read(buf, size);
	buf[size - 1] = '\0';

	assembleTextLine(buf, _text, size);

	delete[] buf;
}

void TextBoxWrite::execute() {
	auto &tb = NancySceneState.getTextbox();
	tb.clear();
	tb.addTextLine(_text);
	tb.setVisible(true);
	finishExecution();
}

void TextboxClear::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void TextboxClear::execute() {
	NancySceneState.getTextbox().clear();
	finishExecution();
}

void BumpPlayerClock::readData(Common::SeekableReadStream &stream) {
	_relative = stream.readByte();
	_hours = stream.readUint16LE();
	_minutes = stream.readUint16LE();
}

void BumpPlayerClock::execute() {
	NancySceneState.setPlayerTime(_hours * 3600000 + _minutes * 60000, _relative);
	finishExecution();
}

void SaveContinueGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void SaveContinueGame::execute() {
	g_nancy->secondChance();
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
	if (!_isTerse) {
		_flags.readData(stream);
	} else {
		// Terse version only has 2 flags
		_flags.descs[0].label = stream.readSint16LE();
		_flags.descs[0].flag = stream.readUint16LE();
		_flags.descs[1].label = stream.readSint16LE();
		_flags.descs[1].flag = stream.readUint16LE();
	}
}

void EventFlags::execute() {
	_flags.execute();
	_isDone = true;
}

void EventFlagsMultiHS::readData(Common::SeekableReadStream &stream) {
	EventFlags::readData(stream);

	if (_isCursor) {
		_hoverCursor = (CursorManager::CursorType)stream.readUint16LE();
	}

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

void GotoMenu::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void GotoMenu::execute() {
	if (!ConfMan.hasKey("original_menus") || ConfMan.getBool("original_menus")) {
		g_nancy->setState(NancyState::kMainMenu);
	} else {
		Common::Event ev;
		ev.type = Common::EVENT_RETURN_TO_LAUNCHER;
		g_system->getEventManager()->pushEvent(ev);
	}

	_isDone = true;
}


void LoseGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void LoseGame::execute() {
	g_nancy->_sound->stopAndUnloadSceneSpecificSounds();
	NancySceneState.setDestroyOnExit();

	if (!ConfMan.hasKey("original_menus") || ConfMan.getBool("original_menus")) {
		g_nancy->setState(NancyState::kMainMenu);
	} else {
		Common::Event ev;
		ev.type = Common::EVENT_RETURN_TO_LAUNCHER;
		g_system->getEventManager()->pushEvent(ev);
	}

	_isDone = true;
}

void PushScene::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void PushScene::execute() {
	NancySceneState.pushScene();
	_isDone = true;
}

void PopScene::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void PopScene::execute() {
	NancySceneState.popScene();
	_isDone = true;
}

void WinGame::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void WinGame::execute() {
	g_nancy->_sound->stopAndUnloadSceneSpecificSounds();
	NancySceneState.setDestroyOnExit();
	g_nancy->setState(NancyState::kCredits, NancyState::kMainMenu);

	_isDone = true;
}

void DifficultyLevel::readData(Common::SeekableReadStream &stream) {
	_difficulty = stream.readUint16LE();
	_flag.label = stream.readSint16LE();
	_flag.flag = stream.readUint16LE();
}

void DifficultyLevel::execute() {
	NancySceneState.setDifficulty(_difficulty);
	NancySceneState.setEventFlag(_flag);
	_isDone = true;
}

void HintSystem::readData(Common::SeekableReadStream &stream) {
	_characterID = stream.readByte();
	_genericSound.readNormal(stream);
}

void HintSystem::execute() {
	switch (_state) {
	case kBegin: {
		uint16 difficulty = NancySceneState.getDifficulty();
		selectHint();
		_genericSound.name = selectedHint->soundIDs[difficulty];

		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(g_nancy->getStaticData().hintTexts[selectedHint->textID * 3 + difficulty]);

		g_nancy->_sound->loadSound(_genericSound);
		g_nancy->_sound->playSound(_genericSound);
		_state = kRun;
		break;
	}
	case kRun:
		if (!g_nancy->_sound->isSoundPlaying(_genericSound)) {
			g_nancy->_sound->stopSound(_genericSound);
			_state = kActionTrigger;
		} else {
			break;
		}

		// fall through
	case kActionTrigger:
		NancySceneState.useHint(_characterID, _hintID);
		NancySceneState.getTextbox().clear();

		NancySceneState.changeScene(selectedHint->sceneChange);

		_isDone = true;
		break;
	}
}

void HintSystem::selectHint() {
	if (NancySceneState.getHintsRemaining() == 0) {
		selectedHint = &g_nancy->getStaticData().hints[_characterID][0];
	}

	// Start from 1 since the first hint is always the "I give up" option
	for (uint i = 1; i < g_nancy->getStaticData().hints[_characterID].size(); ++i) {
		const auto &hint = g_nancy->getStaticData().hints[_characterID][i];

		bool isSatisfied = true;

		for (const auto &cond : hint.conditions) {
			switch (cond.type) {
			case (byte)StaticDataConditionType::kEvent :
				if (!NancySceneState.getEventFlag(cond.label, cond.flag)) {
					isSatisfied = false;
				}

				break;
			case (byte)StaticDataConditionType::kInventory :
				if (NancySceneState.hasItem(cond.label) != cond.flag) {
					isSatisfied = false;
				}

				break;
			case (byte)StaticDataConditionType::kDifficulty :
				if (	(NancySceneState.getDifficulty() != cond.label && cond.flag != 0) ||
						(NancySceneState.getDifficulty() == cond.label && cond.flag == 0) ) {
					isSatisfied = false;
				}

				break;
			}

			if (!isSatisfied) {
				break;
			}
		}

		if (isSatisfied) {
			selectedHint = &hint;
			break;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
