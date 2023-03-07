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

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void PlayPrimaryVideoChan0::ConditionFlag::read(Common::SeekableReadStream &stream) {
	type = stream.readByte();
	flag.label = stream.readSint16LE();
	flag.flag = stream.readByte();
	orFlag = stream.readByte();
}

bool PlayPrimaryVideoChan0::ConditionFlag::isSatisfied() const {
	switch (type) {
	case kFlagEvent:
		return NancySceneState.getEventFlag(flag);
	case kFlagInventory:
		return NancySceneState.hasItem(flag.label) == flag.flag;
	default:
		return false;
	}
}

void PlayPrimaryVideoChan0::ConditionFlag::set() const {
	switch (type) {
	case kFlagEvent:
		NancySceneState.setEventFlag(flag);
		break;
	case kFlagInventory:
		if (flag.flag == kInvHolding) {
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
		GraphicsManager::copyToManaged(*_decoder.decodeNextFrame(), _drawSurface, _videoFormat == 1);

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

	ser.skip(0x10); // Bounds
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
	ser.syncAsByte(_defaultNextScene);
	ser.syncAsByte(_popNextScene);
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
		response.sceneChange.readData(stream, ser.getVersion() == kGameTypeVampire);
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
		flagsStruct.flagToSet.type = stream.readByte();
		flagsStruct.flagToSet.flag.label = stream.readSint16LE();
		flagsStruct.flagToSet.flag.flag = stream.readByte();
	}
}

void PlayPrimaryVideoChan0::execute() {
	PlayPrimaryVideoChan0 *activeVideo = NancySceneState.getActivePrimaryVideo();
	if (activeVideo != this && activeVideo != nullptr) {
		if (    !activeVideo->_isDone ||
				activeVideo->_defaultNextScene == kDefaultNextSceneEnabled ||
				activeVideo->_pickedResponse != -1  ) {

			return;
		} else {
			// Chained videos, hide the previous one and start this
			activeVideo->setVisible(false);
			NancySceneState.setActivePrimaryVideo(this);
		}
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
				addConditionalDialogue();
			}

			if (_goodbyeResponseCharacterID != 10) {
				addGoodbye();
			}

			for (uint i = 0; i < _responses.size(); ++i) {
				auto &res = _responses[i];

				if (res.conditionFlags.isSatisfied()) {
					NancySceneState.getTextbox().addTextLine(res.text);
                    res.isOnScreen = true;
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
					if (NancySceneState.getLogicCondition(i, kLogUsed)) {
                        int pickedOnScreenResponse = _pickedResponse = i;

                        // Adjust to account for hidden responses
                        for (uint j = 0; j < _responses.size(); ++j) {
                            if (!_responses[j].isOnScreen) {
                                ++_pickedResponse;
                            }

                            if ((int)j == pickedOnScreenResponse) {
                                break;
                            }
                        }

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

				if (_defaultNextScene == kDefaultNextSceneEnabled) {
					NancySceneState.changeScene(_sceneChange);
				} else if (_popNextScene == kPopNextScene) {
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

void PlayPrimaryVideoChan0::addConditionalDialogue() {
	for (const auto &res : g_nancy->getStaticData().conditionalDialogue[_conditionalResponseCharacterID]) {
		bool isSatisfied = true;

		for (const auto &cond : res.flagConditions) {
			if (!NancySceneState.getEventFlag(cond.label, cond.flag)) {
				isSatisfied = false;
				break;
			}
		}

		for (const auto &cond : res.inventoryConditions) {
			if (NancySceneState.hasItem(cond.label) != cond.flag) {
				isSatisfied = false;
				break;
			}
		}

		if (isSatisfied) {
			_responses.push_back(ResponseStruct());
			ResponseStruct &newResponse = _responses.back();
			newResponse.soundName = res.soundID;
			newResponse.text = g_nancy->getStaticData().conditionalDialogueTexts[res.textID];
			newResponse.sceneChange.sceneID = res.sceneID;
			newResponse.sceneChange.continueSceneSound = kContinueSceneSound;
		}
	}
}

void PlayPrimaryVideoChan0::addGoodbye() {
	auto &res = g_nancy->getStaticData().goodbyes[_goodbyeResponseCharacterID];
	_responses.push_back(ResponseStruct());
	ResponseStruct &newResponse = _responses.back();
	newResponse.soundName = res.soundID;
	newResponse.text = g_nancy->getStaticData().goodbyeTexts[_goodbyeResponseCharacterID];
	
	// Evaluate conditions to pick from the collection of replies
	uint sceneChangeID = 0;
	for (uint i = 0; i < res.sceneChanges.size(); ++i) {
		const GoodbyeSceneChange &sc = res.sceneChanges[i];
		if (sc.flagConditions.size() == 0) {
			// No conditions, default choice
			sceneChangeID = i;
			break;
		} else {
			bool isSatisfied = true;

			for (const auto &cond : sc.flagConditions) {
				if (!NancySceneState.getEventFlag(cond.label, cond.flag)) {
					isSatisfied = false;
					break;
				}
			}

			if (isSatisfied) {
				sceneChangeID = i;
				break;
			}
		}
	}

	const GoodbyeSceneChange &sceneChange = res.sceneChanges[sceneChangeID];
	
	// The reply from the character is picked randomly
	newResponse.sceneChange.sceneID = sceneChange.sceneIDs[g_nancy->_randomSource->getRandomNumber(sceneChange.sceneIDs.size() - 1)];

	// Set an event flag if applicable
	NancySceneState.setEventFlag(sceneChange.flagToSet);

	newResponse.sceneChange.continueSceneSound = kContinueSceneSound;
}

} // End of namespace Action
} // End of namespace Nancy
