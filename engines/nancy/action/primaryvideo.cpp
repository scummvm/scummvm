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

#include "common/system.h"
#include "common/random.h"
#include "common/config-manager.h"
#include "common/serializer.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/action/primaryvideo.h"
#include "engines/nancy/action/responses.cpp"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

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

	if (NancySceneState.getActivePrimaryVideo() == this) {
		NancySceneState.setActivePrimaryVideo(nullptr);
	}

	NancySceneState.setShouldClearTextbox(true);
	NancySceneState.getTextbox().setVisible(false);
}

void PlayPrimaryVideoChan0::init() {
	if (!_decoder.loadFile(_videoName + ".avf")) {
		error("Couldn't load video file %s", _videoName.c_str());
	}

	_drawSurface.create(_src.width(), _src.height(), _decoder.getPixelFormat());

	if (!_paletteName.empty()) {
		GraphicsManager::loadSurfacePalette(_drawSurface, _paletteName);
		setTransparent(true);
	}

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
		if (_videoFormat == 2) {
			_drawSurface.blitFrom(*_decoder.decodeNextFrame(), _src, Common::Point());
		} else if (_videoFormat == 1) {
			// This seems to be the only place in the engine where format 1 videos
			// are scaled with arbitrary sizes; everything else uses double size
			Graphics::Surface *scaledFrame = _decoder.decodeNextFrame()->getSubArea(_src).scale(_screenPosition.width(), _screenPosition.height());
			GraphicsManager::copyToManaged(*scaledFrame, _drawSurface, true);
			scaledFrame->free();
			delete scaledFrame;
		}

		_needsRedraw = true;
	}

	RenderObject::updateGraphics();
}

void PlayPrimaryVideoChan0::onPause(bool pause) {
	_decoder.pauseVideo(pause);

	if (!pause) {
		registerGraphics();
	}
}

void PlayPrimaryVideoChan0::readData(Common::SeekableReadStream &stream) {
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	readFilename(stream, _videoName);

	if (ser.getVersion() == kGameTypeVampire) {
		readFilename(stream, _paletteName);
	}

	ser.skip(2);
	ser.syncAsUint16LE(_videoFormat);
	ser.skip(0x13, kGameTypeVampire, kGameTypeVampire);
	ser.skip(0xF, kGameTypeNancy1);

	readRect(stream, _src);
	readRect(stream, _screenPosition);

	char *rawText = new char[1500];
	ser.syncBytes((byte *)rawText, 1500);
	UI::Textbox::assembleTextLine(rawText, _text, 1500);
	delete[] rawText;

	_sound.read(stream, SoundDescription::kNormal);
	_responseGenericSound.read(stream, SoundDescription::kNormal);
	ser.skip(1);
	ser.syncAsByte(_conditionalResponseCharacterID);
	ser.syncAsByte(_goodbyeResponseCharacterID);
	ser.syncAsByte(_isDialogueExitScene);
	ser.syncAsByte(_doNotPop);
	_sceneChange.readData(stream);

	ser.skip(0x35, kGameTypeVampire, kGameTypeVampire);
	ser.skip(0x32, kGameTypeNancy1);

	uint16 numResponses = 0;
	ser.syncAsUint16LE(numResponses);
	rawText = new char[400];

	_responses.reserve(numResponses);
	for (uint i = 0; i < numResponses; ++i) {
		_responses.push_back(ResponseStruct());
		ResponseStruct &response = _responses.back();
		response.conditionFlags.read(stream);
		ser.syncBytes((byte*)rawText, 400);
		UI::Textbox::assembleTextLine(rawText, response.text, 400);
		readFilename(stream, response.soundName);
		ser.skip(1);
		response.sceneChange.readData(stream);
		ser.skip(3, kGameTypeVampire, kGameTypeVampire);
		ser.syncAsSint16LE(response.flagDesc.label);
		ser.syncAsByte(response.flagDesc.flag);
		ser.skip(0x32);
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
	PlayPrimaryVideoChan0 *activeVideo = NancySceneState.getActivePrimaryVideo();
	if (activeVideo != this && activeVideo != nullptr) {
		return;
	}

	switch (_state) {
	case kBegin: {
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_sound);

		if (!ConfMan.getBool("speech_mute") && ConfMan.getBool("character_speech")) {
			g_nancy->_sound->playSound(_sound);
		}

		// Remove held item and re-add it to inventory
		int heldItem = NancySceneState.getHeldItem();
		if (heldItem != -1) {
			NancySceneState.addItemToInventory(heldItem);
			NancySceneState.setHeldItem(-1);
		}

		// Move the mouse to the default position defined in CURS
		const Common::Point initialMousePos = g_nancy->_cursorManager->getPrimaryVideoInitialPos();
		const Common::Point cursorHotspot = g_nancy->_cursorManager->getCurrentCursorHotspot();
		Common::Point adjustedMousePos = g_nancy->_input->getInput().mousePos;
		adjustedMousePos.x -= cursorHotspot.x;
		adjustedMousePos.y -= cursorHotspot.y - 1;
		if (g_nancy->_cursorManager->getPrimaryVideoInactiveZone().bottom > adjustedMousePos.y) {
			g_system->warpMouse(initialMousePos.x + cursorHotspot.x, initialMousePos.y + cursorHotspot.y);
			g_nancy->_cursorManager->setCursorType(CursorManager::kNormalArrow);
		}

		_state = kRun;
		NancySceneState.setActivePrimaryVideo(this);
	}
		// fall through
	case kRun:
		if (!_hasDrawnTextbox) {
			_hasDrawnTextbox = true;
			NancySceneState.getTextbox().clear();

			if (ConfMan.getBool("subtitles")) {
				NancySceneState.getTextbox().addTextLine(_text);
			}

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
					// Player has picked response, play sound file and change state
					_responseGenericSound.name = _responses[_pickedResponse].soundName;
					g_nancy->_sound->loadSound(_responseGenericSound);

					if (!ConfMan.getBool("speech_mute") && ConfMan.getBool("player_speech")) {
						g_nancy->_sound->playSound(_responseGenericSound);
					}

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

void PlayPrimaryVideoChan0::handleInput(NancyInput &input) {
	const Common::Rect &inactiveZone = g_nancy->_cursorManager->getPrimaryVideoInactiveZone();
	const Common::Point cursorHotspot = g_nancy->_cursorManager->getCurrentCursorHotspot();
	Common::Point adjustedMousePos = input.mousePos;
	adjustedMousePos.y -= cursorHotspot.y;

	if (inactiveZone.bottom > adjustedMousePos.y) {
		input.mousePos.y = inactiveZone.bottom + cursorHotspot.y;
		g_system->warpMouse(input.mousePos.x, input.mousePos.y);
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
