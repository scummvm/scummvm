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
#include "nancy/ui/taskbar.h"

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

void LightningOn::execute() {
	NancySceneState.beginLightning(_distance, _pulseTime, _rgbPercent);
	_isDone = true;
}

void TextBoxWrite::readData(Common::SeekableReadStream &stream) {
	int16 size = stream.readSint16LE();

	if (size > 10000) {
		error("Action Record atTextboxWrite has too many text box chars: %d", size);
	}

	if (size == -1) {
		Common::String stringID;
		readFilename(stream, stringID);

		const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
		assert(autotext);

		_text = autotext->texts[stringID];
	} else {
		char *buf = new char[size];
		stream.read(buf, size);
		buf[size - 1] = '\0';

		assembleTextLine(buf, _text, size);

		delete[] buf;
	}
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

void FrameTextBox::readData(Common::SeekableReadStream &stream) {
	const int16 size = stream.readSint16LE();

	if (size > 10000)
		error("FrameTextBox: too many text characters: %d", size);

	if (size == -1) {
		// CVTX-keyed lookup, same as the Nancy 6+ TextBoxWrite path.
		Common::String stringID;
		readFilename(stream, stringID);

		const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
		assert(autotext);

		// TODO: we probably ought to be doing something more robust here
		// to detect missing keys, but for now just return an empty string
		// if the key isn't found.
		_text = autotext->texts.getValOrDefault(stringID, "");
	} else if (size > 0) {
		char *buf = new char[size];
		stream.read(buf, size);
		buf[size - 1] = '\0';

		assembleTextLine(buf, _text, size);

		delete[] buf;
	}

	// Trailing two int16 fields: meaning differs slightly between the
	// three opcodes that reuse this layout, but are safe to capture as a
	// pair until UICO conversation rendering is wired up.
	_flags = stream.readSint16LE();
	_slot  = stream.readSint16LE();
}

void FrameTextBox::execute() {
	// TODO: UICO-driven conversation rendering isn't ready yet; route the
	// line into the legacy textbox so subtitles still surface (the textbox
	// is kept off-screen on Nancy 10+ but addTextLine is harmless).
	auto &tb = NancySceneState.getTextbox();
	tb.clear();
	if (!_text.empty()) {
		tb.addTextLine(_text);
	}
	finishExecution();
}

void ControlUIItems::readData(Common::SeekableReadStream &stream) {
	_uiButton = stream.readUint16LE();
	_flagA = stream.readByte();
	_flagB  = stream.readByte();
	_scene1 = stream.readSint16LE();
	_scene2 = stream.readSint16LE();
}

void ControlUIItems::execute() {
	// TODO: finish this

	NancySceneState.getTaskbar()->toggleButton(_uiButton, _flagA != 0);
	debug("ControlUIItems: UIButton=%d, flagA=%d, flagB=%d, scene1=%d, scene2=%d", _uiButton, _flagA, _flagB, _scene1, _scene2);

	finishExecution();
}

void UIPopupPrepScene::readData(Common::SeekableReadStream &stream) {
	_uiType      = stream.readSint32LE();
	_signalValue = stream.readSint32LE();
}

void UIPopupPrepScene::execute() {
	// TODO: finish this

	debug("UIPopupPrepScene: UIType=%d, signalValue=%d", _uiType, _signalValue);

	finishExecution();
}

void AddSearchLink::readData(Common::SeekableReadStream &stream) {
	_mode = stream.readSint16LE();

	readFilename(stream, _key);
	readFilename(stream, _value);

	_extra  = stream.readSint16LE();
	_flag = stream.readSint16LE();
	_scene = stream.readSint16LE();
}

void AddSearchLink::execute() {
	// TODO: finish this

	debug("AddSearchLink: mode=%d, key=%s, value=%s, extra=%d, scene1=%d, scene2=%d", _mode, _key.c_str(), _value.c_str(), _extra, _flag, _scene);

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

void GotoMenu::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void GotoMenu::execute() {
	//NancySceneState.setDestroyOnExit();
	g_nancy->setState(NancyState::kMainMenu);

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
	// Set ConfMan value that will stay persistent across future playthroughs.
	// Default value in original is StillWorkingOnIt, but we just don't set it instead.
	ConfMan.set("PlayerWonTheGame", "AcedTheGame", ConfMan.getActiveDomainName());
	ConfMan.flushToDisk();

	g_nancy->_sound->stopAndUnloadSceneSpecificSounds();
	NancySceneState.setDestroyOnExit();
	g_nancy->setState(NancyState::kCredits, NancyState::kMainMenu);

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
