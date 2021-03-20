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

#include "engines/nancy/action/primaryvideo.h"

#include "engines/nancy/action/responses.cpp"
#include "engines/nancy/action/actionmanager.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "common/file.h"
#include "common/random.h"

namespace Nancy {
namespace Action {

PlayPrimaryVideoChan0 *PlayPrimaryVideoChan0::_activePrimaryVideo = nullptr;

void PlayPrimaryVideoChan0::ConditionFlag::read(Common::SeekableReadStream &stream) {
	type = (ConditionType)stream.readByte();
	flag.label = stream.readSint16LE();
	flag.flag = (NancyFlag)stream.readByte();
	orFlag = stream.readByte();
}

bool PlayPrimaryVideoChan0::ConditionFlag::isSatisfied() const {
	switch (type) {
	case ConditionFlag::kEventFlags:
		return NancySceneState.getEventFlag(flag);
	case ConditionFlag::kInventory:
		return NancySceneState.hasItem(flag.label) == flag.flag;
	default:
		return false;
	}
}

void PlayPrimaryVideoChan0::ConditionFlag::set() const {
	switch (type) {
	case ConditionFlag::kEventFlags:
		NancySceneState.setEventFlag(flag);
		break;
	case ConditionFlag::kInventory:
		if (flag.flag == kTrue) {
			NancySceneState.addItemToInventory(flag.label);
		} else {
			NancySceneState.removeItemFromInventory(flag.label);
		}

		break;
	default:
		break;
	}
}

void PlayPrimaryVideoChan0::ConditionFlags::read(Common::SeekableReadStream &stream) {
	uint16 numFlags = stream.readUint16LE();

	conditionFlags.reserve(numFlags);
	for (uint i = 0; i < numFlags; ++i) {
		conditionFlags.push_back(ConditionFlag());
		conditionFlags.back().read(stream);
	}
}

bool PlayPrimaryVideoChan0::ConditionFlags::isSatisfied() const {
	bool orFlag = false;

	for (uint i = 0; i < conditionFlags.size(); ++i) {
		const ConditionFlag &cur = conditionFlags[i];

		if (!cur.isSatisfied()) {
			if (orFlag) {
				return false;
			} else {
				orFlag = true;
			}
		}
	}

	if (orFlag) {
		return false;
	} else {
		return true;
	}
}

PlayPrimaryVideoChan0::~PlayPrimaryVideoChan0() {
	_decoder.close();

	if (_activePrimaryVideo == this) {
		_activePrimaryVideo = nullptr;
	}

	NancySceneState.setShouldClearTextbox(true);
	NancySceneState.getTextbox().setVisible(false);
}

void PlayPrimaryVideoChan0::init() {
	_decoder.loadFile(_videoName + ".avf");
	_drawSurface.create(_src.width(), _src.height(), _decoder.getPixelFormat());

	RenderObject::init();

	NancySceneState.setShouldClearTextbox(false);
}

void PlayPrimaryVideoChan0::updateGraphics() {
	if (!_decoder.isVideoLoaded()) {
		return;
	}

	if (!_decoder.isPlaying()) {
		_decoder.start();
	}

	if (_decoder.needsUpdate()) {
		_drawSurface.blitFrom(*_decoder.decodeNextFrame(), _src, Common::Point());
		_needsRedraw = true;
	}

	RenderObject::updateGraphics();
}

void PlayPrimaryVideoChan0::onPause(bool pause) {
	_decoder.pauseVideo(pause);

	if (pause) {
		registerGraphics();
	}
}

void PlayPrimaryVideoChan0::readData(Common::SeekableReadStream &stream) {
	uint16 beginOffset = stream.pos();

	readFilename(stream, _videoName);

	stream.skip(0x13);

	readRect(stream, _src);
	readRect(stream, _screenPosition);

	char *rawText = new char[1500];
	stream.read(rawText, 1500);
	UI::Textbox::assembleTextLine(rawText, _text, 1500);
	delete[] rawText;

	_sound.read(stream, SoundDescription::kNormal);
	_responseGenericSound.read(stream, SoundDescription::kNormal);
	stream.skip(1);
	_conditionalResponseCharacterID = stream.readByte();
	_goodbyeResponseCharacterID = stream.readByte();
	_isDialogueExitScene = (NancyFlag)stream.readByte();
	_doNotPop = (NancyFlag)stream.readByte();
	_sceneChange.readData(stream);

	stream.seek(beginOffset + 0x69C);

	uint16 numResponses = stream.readUint16LE();
	rawText = new char[400];

	_responses.reserve(numResponses);
	for (uint i = 0; i < numResponses; ++i) {
		_responses.push_back(ResponseStruct());
		ResponseStruct &response = _responses[i];
		response.conditionFlags.read(stream);
		stream.read(rawText, 400);
		UI::Textbox::assembleTextLine(rawText, response.text, 400);
		readFilename(stream, response.soundName);
		stream.skip(1);
		response.sceneChange.readData(stream);
		response.flagDesc.label = stream.readSint16LE();
		response.flagDesc.flag = (NancyFlag)stream.readByte();

		stream.skip(0x32);
	}

	delete[] rawText;

	uint16 numSceneBranchStructs = stream.readUint16LE();
	if (numSceneBranchStructs > 0) {
		// TODO
	}

	uint16 numFlagsStructs = stream.readUint16LE();
	_flagsStructs.reserve(numFlagsStructs);
	for (uint16 i = 0; i < numFlagsStructs; ++i) {
		_flagsStructs.push_back(FlagsStruct());
		FlagsStruct &flagsStruct = _flagsStructs.back();
		flagsStruct.conditions.read(stream);
		flagsStruct.flagToSet.type = (ConditionFlag::ConditionType)stream.readByte();
		flagsStruct.flagToSet.flag.label = stream.readSint16LE();
		flagsStruct.flagToSet.flag.flag = (NancyFlag)stream.readByte();
	}
}

void PlayPrimaryVideoChan0::execute() {
	if (_activePrimaryVideo != this && _activePrimaryVideo != nullptr) {
		return;
	}

	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);
		_state = kRun;
		_activePrimaryVideo = this;
		// fall through
	case kRun:
		if (!_hasDrawnTextbox) {
			_hasDrawnTextbox = true;
			NancySceneState.getTextbox().clear();
			NancySceneState.getTextbox().addTextLine(_text);

			// Add responses when conditions have been satisfied
			if (_conditionalResponseCharacterID != 10) {
				addConditionalResponses();
			}

			if (_goodbyeResponseCharacterID != 10) {
				addGoodbye();
			}

			for (uint i = 0; i < _responses.size(); ++i) {
				auto &res = _responses[i];

				if (res.conditionFlags.isSatisfied()) {
					NancySceneState.getTextbox().addTextLine(res.text);
				}
			}
		}

		if (!g_nancy->_sound->isSoundPlaying(_sound) && _decoder.endOfVideo()) {
			g_nancy->_sound->stopSound(_sound);

			if (_responses.size() == 0) {
				// NPC has finished talking with no responses available, auto-advance to next scene
				_state = kActionTrigger;
			} else {
				// NPC has finished talking, we have responses
				for (uint i = 0; i < 30; ++i) {
					if (NancySceneState.getLogicCondition(i, kTrue)) {
						_pickedResponse = i;
						break;
					}
				}

				if (_pickedResponse != -1) {
					// Player has picked response, play sound file and change _state
					_responseGenericSound.name = _responses[_pickedResponse].soundName;
					// TODO this is probably not correct
					g_nancy->_sound->loadSound(_responseGenericSound);
					g_nancy->_sound->playSound(_responseGenericSound);
					_state = kActionTrigger;
				}
			}
		}
		break;
	case kActionTrigger:
		// process flags structs
		for (auto flags : _flagsStructs) {
			if (flags.conditions.isSatisfied()) {
				flags.flagToSet.set();
			}
		}

		if (_pickedResponse != -1) {
			// Set response's event flag, if any
			NancySceneState.setEventFlag(_responses[_pickedResponse].flagDesc);
		}

		if (!g_nancy->_sound->isSoundPlaying(_responseGenericSound)) {
			g_nancy->_sound->stopSound(_responseGenericSound);

			if (_pickedResponse != -1) {
				NancySceneState.changeScene(_responses[_pickedResponse].sceneChange);
			} else {
				// Evaluate scene branch structs here

				if (_isDialogueExitScene == kFalse) {
					NancySceneState.changeScene(_sceneChange);
				} else if (_doNotPop == kFalse) {
					// Exit dialogue
					NancySceneState.popScene();
				}
			}

			finishExecution();
		}

		break;
	}
}

void PlayPrimaryVideoChan0::addConditionalResponses() {
	for (const auto &res : nancy1ConditionalResponses) {
		if (res.characterID == _conditionalResponseCharacterID) {
			bool isSatisfied = true;
			for (const auto & cond : res.conditions) {
				if (cond.label == -1) {
					break;
				}

				if (!NancySceneState.getEventFlag(cond.label, cond.flag)) {
					isSatisfied = false;
					break;
				}
			}

			if (isSatisfied) {
				Common::File file;
				char snd[9];

				file.open("game.exe");
				file.seek(nancy1ResponseBaseFileOffset + res.fileOffset);
				file.read(snd, 8);
				snd[8] = '\0';

				_responses.push_back(ResponseStruct());
				ResponseStruct &newResponse = _responses.back();
				newResponse.soundName = snd;
				newResponse.text = file.readString();
				newResponse.sceneChange.sceneID = res.sceneID;
				newResponse.sceneChange.doNotStartSound = true;

				file.close();
			}
		}
	}
}

void PlayPrimaryVideoChan0::addGoodbye() {
	for (const auto &res : nancy1Goodbyes) {
		if (res.characterID == _goodbyeResponseCharacterID) {
			Common::File file;
			char snd[9];

			file.open("game.exe");
			file.seek(nancy1ResponseBaseFileOffset + res.fileOffset);
			file.read(snd, 8);
			snd[8] = '\0';

			_responses.push_back(ResponseStruct());
			ResponseStruct &newResponse = _responses.back();
			newResponse.soundName = snd;
			newResponse.text = file.readString();
			// response is picked randomly
			newResponse.sceneChange.sceneID = res.sceneIDs[g_nancy->_randomSource->getRandomNumber(3)];
			newResponse.sceneChange.doNotStartSound = true;

			file.close();
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
