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

#include "common/random.h"
#include "common/config-manager.h"
#include "common/serializer.h"
#include "common/memstream.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/action/conversation.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

ConversationSound::ConversationSound() :
		RenderActionRecord(8),
		_noResponse(g_nancy->getGameType() <= kGameTypeNancy2 ? 10 : 20),
		_hasDrawnTextbox(false),
		_pickedResponse(-1) {
	_conditionalResponseCharacterID = _noResponse;
	_goodbyeResponseCharacterID = _noResponse;
}

ConversationSound::~ConversationSound() {
	if (NancySceneState.getActiveConversation() == this) {
		NancySceneState.setActiveConversation(nullptr);
	}
}

void ConversationSound::init() {
	RenderObject::init();
}

void ConversationSound::readData(Common::SeekableReadStream &stream) {
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	if (ser.getVersion() >= kGameTypeNancy2) {
		_sound.readNormal(stream);
	}

	readCaptionText(stream);

	if (ser.getVersion() <= kGameTypeNancy1) {
		_sound.readNormal(stream);
	}

	_responseGenericSound.readNormal(stream);
	ser.skip(1); // RESPONSE_STARTUP_CLEAR_ALL, RESPONSE_STARTUP_KEEP_OLD; never used (tested up to nancy5)
	ser.syncAsByte(_conditionalResponseCharacterID);
	ser.syncAsByte(_goodbyeResponseCharacterID);
	ser.syncAsByte(_defaultNextScene);
	ser.syncAsByte(_popNextScene);
	_sceneChange.readData(stream, ser.getVersion() == kGameTypeVampire);
	ser.skip(0x32, kGameTypeVampire, kGameTypeNancy1);
	ser.skip(2, kGameTypeNancy2);

	uint16 numResponses = 0;
	ser.syncAsUint16LE(numResponses);

	_responses.resize(numResponses);
	for (uint i = 0; i < numResponses; ++i) {
		ResponseStruct &response = _responses[i];
		response.conditionFlags.read(stream);
		readResponseText(stream, response);
		readFilename(stream, response.soundName);
		ser.syncAsByte(response.addRule);
		response.sceneChange.readData(stream, ser.getVersion() == kGameTypeVampire);
		ser.syncAsSint16LE(response.flagDesc.label);
		ser.syncAsByte(response.flagDesc.flag);
		ser.skip(0x32, kGameTypeVampire, kGameTypeNancy1);
		ser.skip(2, kGameTypeNancy2);
	}

	uint16 numSceneBranchStructs = stream.readUint16LE();
	_sceneBranchStructs.resize(numSceneBranchStructs);
	for (uint i = 0; i < numSceneBranchStructs; ++i) {
		_sceneBranchStructs[i].conditions.read(stream);
		_sceneBranchStructs[i].sceneChange.readData(stream, g_nancy->getGameType() == kGameTypeVampire);
		ser.skip(0x32, kGameTypeVampire, kGameTypeNancy1);
		ser.skip(2, kGameTypeNancy2);
	}

	uint16 numFlagsStructs = stream.readUint16LE();
	_flagsStructs.resize(numFlagsStructs);
	for (uint i = 0; i < numFlagsStructs; ++i) {
		FlagsStruct &flagsStruct = _flagsStructs[i];
		flagsStruct.conditions.read(stream);
		flagsStruct.flagToSet.type = stream.readByte();
		flagsStruct.flagToSet.flag.label = stream.readSint16LE();
		flagsStruct.flagToSet.flag.flag = stream.readByte();
	}
}

void ConversationSound::readTerseData(Common::SeekableReadStream &stream) {
	readFilename(stream, _sound.name);
	_sound.volume = stream.readUint16LE();
	_sound.channelID = 12; // hardcoded
	_sound.numLoops = 1;

	_responseGenericSound.volume = _sound.volume;
	_responseGenericSound.numLoops = 1;
	_responseGenericSound.channelID = 13; // hardcoded

	readTerseCaptionText(stream);

	_conditionalResponseCharacterID = stream.readByte();
	_goodbyeResponseCharacterID = stream.readByte();

	_defaultNextScene = stream.readByte();

	_sceneChange.sceneID = stream.readUint16LE();
	_sceneChange.continueSceneSound = kContinueSceneSound;

	uint16 numResponses = stream.readUint16LE();
	_responses.resize(numResponses);
	for (uint i = 0; i < numResponses; ++i) {
		ResponseStruct &response = _responses[i];
		response.conditionFlags.read(stream);
		readTerseResponseText(stream, response);
		readFilename(stream, response.soundName);
		response.sceneChange.sceneID = stream.readUint16LE();
		response.sceneChange.continueSceneSound = kContinueSceneSound;
	}

	// No scene branches
	uint16 numFlagsStructs = stream.readUint16LE();
	_flagsStructs.resize(numFlagsStructs);
	for (uint i = 0; i < numFlagsStructs; ++i) {
		FlagsStruct &flagsStruct = _flagsStructs[i];
		flagsStruct.conditions.read(stream);
		flagsStruct.flagToSet.type = stream.readByte();
		flagsStruct.flagToSet.flag.label = stream.readSint16LE();
		flagsStruct.flagToSet.flag.flag = stream.readByte();
	}
}

void ConversationSound::readCaptionText(Common::SeekableReadStream &stream) {
	char *rawText = new char[1500];
	stream.read(rawText, 1500);
	assembleTextLine(rawText, _text, 1500);
	delete[] rawText;
}

void ConversationSound::readResponseText(Common::SeekableReadStream &stream, ResponseStruct &response) {
	char *rawText = new char[400];
	stream.read(rawText, 400);
	assembleTextLine(rawText, response.text, 400);
	delete[] rawText;
}

void ConversationSound::readTerseCaptionText(Common::SeekableReadStream &stream) {
	Common::String key;
	readFilename(stream, key);

	const CVTX *convo = (const CVTX *)g_nancy->getEngineData("CONVO");
	assert(convo);

	_text = convo->texts[key];
}

void ConversationSound::readTerseResponseText(Common::SeekableReadStream &stream, ResponseStruct &response) {
	Common::String key;
	readFilename(stream, key);

	const CVTX *convo = (const CVTX *)g_nancy->getEngineData("CONVO");
	assert(convo);

	response.text = convo->texts[key];
}

void ConversationSound::execute() {
	ConversationSound *activeConversation = NancySceneState.getActiveConversation();
	if (activeConversation != this && activeConversation != nullptr) {
		if (	!activeConversation->_isDone ||
				activeConversation->_defaultNextScene == kDefaultNextSceneEnabled ||
				activeConversation->_pickedResponse != -1	) {

			return;
		} else {
			// Chained videos, hide the previous one and start this
			activeConversation->setVisible(false);
			NancySceneState.setActiveConversation(this);
		}
	}

	switch (_state) {
	case kBegin: {
		init();
		g_nancy->_sound->loadSound(_sound);

		if (!ConfMan.getBool("speech_mute") && ConfMan.getBool("character_speech")) {
			g_nancy->_sound->playSound(_sound);
		}

		// Remove held item and re-add it to inventory
		NancySceneState.setNoHeldItem();

		// Move the mouse to the default position defined in CURS
		const Common::Point initialMousePos = g_nancy->_cursor->getPrimaryVideoInitialPos();
		const Common::Point cursorHotspot = g_nancy->_cursor->getCurrentCursorHotspot();
		Common::Point adjustedMousePos = g_nancy->_input->getInput().mousePos;
		adjustedMousePos.x -= cursorHotspot.x;
		adjustedMousePos.y -= cursorHotspot.y - 1;
		if (g_nancy->_cursor->getPrimaryVideoInactiveZone().bottom > adjustedMousePos.y) {
			g_nancy->_cursor->warpCursor(Common::Point(initialMousePos.x + cursorHotspot.x, initialMousePos.y + cursorHotspot.y));
			g_nancy->_cursor->setCursorType(CursorManager::kNormalArrow);
		}

		_state = kRun;
		NancySceneState.setActiveConversation(this);

		// Do not draw first frame since video won't be loaded yet
		g_nancy->_graphics->suppressNextDraw();

		if (g_nancy->getGameType() < kGameTypeNancy6) {
			// Do not fall through to give the execution one loop for event flag changes
			// This fixes TVD scene 750
			break;
		}

		// However, nancy6 scene 1299 requires us to fall through in order to get the correct caption.
		// By that point Conversation scenes weren't the tangled mess they were in earlier games,
		// so hopefully this won't break anything
	}
		// fall through
	case kRun:
		if (!_hasDrawnTextbox) {
			_hasDrawnTextbox = true;
			auto *textboxData = GetEngineData(TBOX);
			assert(textboxData);
			NancySceneState.getTextbox().clear();
			NancySceneState.getTextbox().setOverrideFont(textboxData->conversationFontID);

			if (ConfMan.getBool("subtitles")) {
				NancySceneState.getTextbox().addTextLine(_text);
			}

			Common::Array<uint> responsesToAdd;
			for (uint i = 0; i < _responses.size(); ++i) {
				auto &res = _responses[i];

				if (res.conditionFlags.isSatisfied()) {
					int foundIndex = -1;
					for (uint j = 0; j < responsesToAdd.size(); ++j) {
						if (_responses[responsesToAdd[j]].text.compareToIgnoreCase(res.text) == 0) {
							foundIndex = j;
							break;
						}
					}
					switch(res.addRule) {
					case ResponseStruct::kAddIfNotFound:
						if (foundIndex == -1) {
							responsesToAdd.push_back(i);
						}

						break;
					case ResponseStruct::kRemoveAndAddToEnd:
						if (foundIndex != -1) {
							responsesToAdd.remove_at(foundIndex);
						}

						responsesToAdd.push_back(i);
						break;
					case ResponseStruct::kRemove:
						if (foundIndex != -1) {
							responsesToAdd.remove_at(foundIndex);
						}

						break;
					}
				}
			}

			uint numRegularResponses = _responses.size();

			// Add responses when conditions have been satisfied
			if (_conditionalResponseCharacterID != _noResponse) {
				addConditionalDialogue();
			}

			if (_goodbyeResponseCharacterID != _noResponse) {
				addGoodbye();
			}

			// If conditionals/goodbyes added, make sure to send them to Textbox
			for (uint i = numRegularResponses; i < _responses.size(); ++i) {
				responsesToAdd.push_back(i);
			}

			for (uint i : responsesToAdd) {
				NancySceneState.getTextbox().addTextLine(_responses[i].text);
				_responses[i].isOnScreen = true;
			}
		}

		if (!g_nancy->_sound->isSoundPlaying(_sound) && isVideoDonePlaying()) {
			g_nancy->_sound->stopSound(_sound);

			bool hasResponses = false;
			for (auto &res : _responses) {
				if (res.isOnScreen) {
					hasResponses = true;
					break;
				}
			}

			if (!hasResponses) {
				// NPC has finished talking with no responses available, auto-advance to next scene
				_state = kActionTrigger;
			} else {
				// NPC has finished talking, we have responses
				for (uint i = 0; i < 30; ++i) {
					if (NancySceneState.getLogicCondition(i, g_nancy->_true)) {
						_pickedResponse = i;

						// Adjust to account for hidden responses
						for (uint j = 0; j < _responses.size(); ++j) {
							if (!_responses[j].isOnScreen) {
								++_pickedResponse;
							}

							if ((int)j == _pickedResponse) {
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
		if (!g_nancy->_sound->isSoundPlaying(_responseGenericSound)) {
			// process flags structs
			for (auto &flags : _flagsStructs) {
				if (flags.conditions.isSatisfied()) {
					flags.flagToSet.set();
				}
			}

			if (_pickedResponse != -1) {
				// Set response's event flag, if any
				NancySceneState.setEventFlag(_responses[_pickedResponse].flagDesc);
			}

			g_nancy->_sound->stopSound(_responseGenericSound);

			if (_pickedResponse != -1) {
				NancySceneState.changeScene(_responses[_pickedResponse].sceneChange);
			} else {
				for (uint i = 0; i < _sceneBranchStructs.size(); ++i) {
					if (_sceneBranchStructs[i].conditions.isSatisfied()) {
						NancySceneState.changeScene(_sceneBranchStructs[i].sceneChange);
						break;
					}
				}

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

void ConversationSound::addConditionalDialogue() {
	for (const auto &res : g_nancy->getStaticData().conditionalDialogue[_conditionalResponseCharacterID]) {
		bool isSatisfied = true;

		for (const auto &cond : res.conditions) {
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
			_responses.push_back(ResponseStruct());
			ResponseStruct &newResponse = _responses.back();
			newResponse.soundName = res.soundID;

			if (g_nancy->getGameType() <= kGameTypeNancy5) {
				// String is also inside nancy.dat
				newResponse.text = g_nancy->getStaticData().conditionalDialogueTexts[res.textID];
			} else {
				// String is inside the CVTX chunk in the CONVO file. Sound ID doubles as string key
				const CVTX *convo = (const CVTX *)g_nancy->getEngineData("CONVO");
				assert(convo);

				newResponse.text = convo->texts[res.soundID];
			}

			newResponse.sceneChange.sceneID = res.sceneID;
			newResponse.sceneChange.continueSceneSound = kContinueSceneSound;
			newResponse.sceneChange.listenerFrontVector.set(0, 0, 1);

			// Check if the response is a repeat. This can happen when multiple condition combinations
			// trigger the same response.
			for (uint i = 0; i < _responses.size() - 1; ++i) {
				if (	_responses[i].soundName == newResponse.soundName &&
						_responses[i].text == newResponse.text &&
						_responses[i].sceneChange.sceneID == newResponse.sceneChange.sceneID) {
					_responses.pop_back();
					break;
				}
			}
		}
	}
}

void ConversationSound::addGoodbye() {
	auto &res = g_nancy->getStaticData().goodbyes[_goodbyeResponseCharacterID];
	_responses.push_back(ResponseStruct());
	ResponseStruct &newResponse = _responses.back();
	newResponse.soundName = res.soundID;

	if (g_nancy->getGameType() <= kGameTypeNancy5) {
		// String is also inside nancy.dat
		newResponse.text = g_nancy->getStaticData().goodbyeTexts[_goodbyeResponseCharacterID];
	} else {
		// String is inside the CVTX chunk in the CONVO file. Sound ID doubles as string key
		const CVTX *convo = (const CVTX *)g_nancy->getEngineData("CONVO");
		assert(convo);

		newResponse.text = convo->texts[res.soundID];
	}

	// Evaluate conditions to pick from the collection of replies
	uint sceneChangeID = 0;
	for (uint i = 0; i < res.sceneChanges.size(); ++i) {
		const GoodbyeSceneChange &sc = res.sceneChanges[i];
		if (sc.conditions.size() == 0) {
			// No conditions, default choice
			sceneChangeID = i;
			break;
		} else {
			bool isSatisfied = true;

			for (const auto &cond : sc.conditions) {
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
				sceneChangeID = i;
				break;
			}
		}
	}

	const GoodbyeSceneChange &sceneChange = res.sceneChanges[sceneChangeID];

	// The reply from the character is picked randomly
	newResponse.sceneChange.sceneID = sceneChange.sceneIDs[g_nancy->_randomSource->getRandomNumber(sceneChange.sceneIDs.size() - 1)];
	newResponse.sceneChange.continueSceneSound = kContinueSceneSound;
	newResponse.sceneChange.listenerFrontVector.set(0, 0, 1);

	// Set an event flag if applicable
	// Assumes flagToSet is an event flag
	NancySceneState.setEventFlag(sceneChange.flagToSet.label, sceneChange.flagToSet.flag);
}

void ConversationSound::ConversationFlag::read(Common::SeekableReadStream &stream) {
	type = stream.readByte();
	flag.label = stream.readSint16LE();
	flag.flag = stream.readByte();
	orFlag = stream.readByte();
}

bool ConversationSound::ConversationFlag::isSatisfied() const {
	switch (type) {
	case kFlagEvent:
		return NancySceneState.getEventFlag(flag);
	case kFlagInventory:
		return NancySceneState.hasItem(flag.label) == flag.flag;
	default:
		return false;
	}
}

void ConversationSound::ConversationFlag::set() const {
	switch (type) {
	case kFlagEvent:
		NancySceneState.setEventFlag(flag);
		break;
	case kFlagInventory:
		if (flag.flag == g_nancy->_true) {
			NancySceneState.addItemToInventory(flag.label);
		} else {
			NancySceneState.removeItemFromInventory(flag.label);
		}

		break;
	default:
		break;
	}
}

void ConversationSound::ConversationFlags::read(Common::SeekableReadStream &stream) {
	uint16 numFlags = stream.readUint16LE();

	conditionFlags.resize(numFlags);
	for (uint i = 0; i < numFlags; ++i) {
		conditionFlags[i].read(stream);
	}
}

bool ConversationSound::ConversationFlags::isSatisfied() const {
	Common::Array<bool> conditionsMet(conditionFlags.size(), false);

	for (uint i = 0; i < conditionFlags.size(); ++i) {
		if (conditionFlags[i].isSatisfied()) {
			conditionsMet[i] = true;
		}
	}

	for (uint i = 0; i < conditionsMet.size(); ++i) {
		if (conditionFlags[i].orFlag) {
			bool foundSatisfied = false;
			for (uint j = 0; j < conditionFlags.size(); ++j) {
				if (conditionsMet[j]) {
					foundSatisfied = true;
					break;
				}

				// Found end of orFlag chain
				if (!conditionFlags[j].orFlag) {
					break;
				}
			}

			if (foundSatisfied) {
				for (; i < conditionsMet.size(); ++i) {
					conditionsMet[i] = true;
					if (!conditionFlags[i].orFlag) {
						// End of orFlag chain
						break;
					}
				}
			}
		}
	}

	for (uint i = 0; i < conditionsMet.size(); ++i) {
		if (conditionsMet[i] == false) {
			return false;
		}
	}

	return true;
}

void ConversationVideo::init() {
	if (!_decoder.loadFile(Common::Path(_videoName + ".avf"))) {
		error("Couldn't load video file %s", _videoName.c_str());
	}

	_decoder.seekToFrame(_firstFrame);

	if (!_paletteName.empty()) {
		GraphicsManager::loadSurfacePalette(_drawSurface, _paletteName);
		setTransparent(true);
	}

	ConversationSound::init();
	registerGraphics();
}

void ConversationVideo::readData(Common::SeekableReadStream &stream) {
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	readFilename(stream, _videoName);
	readFilename(ser, _paletteName, kGameTypeVampire, kGameTypeVampire);

	ser.skip(2, kGameTypeVampire, kGameTypeNancy1);
	ser.syncAsUint16LE(_videoFormat);
	ser.skip(3); // Quality
	ser.skip(4, kGameTypeVampire, kGameTypeVampire); // paletteStart, paletteSize
	ser.syncAsUint16LE(_firstFrame);
	ser.syncAsUint16LE(_lastFrame);
	ser.skip(8, kGameTypeVampire, kGameTypeNancy1);
	ser.skip(6, kGameTypeNancy2);

	ser.skip(0x10); // Bounds
	readRect(stream, _screenPosition);

	ConversationSound::readData(stream);
}

void ConversationVideo::updateGraphics() {
	if (!_decoder.isVideoLoaded()) {
		return;
	}

	if (!_decoder.isPlaying()) {
		_decoder.start();
	}

	if (_decoder.getCurFrame() == _lastFrame) {
		_decoder.pauseVideo(true);
	}

	if (_decoder.needsUpdate()) {
		GraphicsManager::copyToManaged(*_decoder.decodeNextFrame(), _drawSurface, _videoFormat == kSmallVideoFormat);

		_needsRedraw = true;
	}

	RenderObject::updateGraphics();
}

void ConversationVideo::onPause(bool pause) {
	_decoder.pauseVideo(pause);
	RenderActionRecord::onPause(pause);
}

bool ConversationVideo::isVideoDonePlaying() {
	return _decoder.endOfVideo() || _decoder.getCurFrame() == _lastFrame;
}

Common::String ConversationVideo::getRecordTypeName() const {
	if (g_nancy->getGameType() <= kGameTypeNancy1) {
		return "PlayPrimaryVideo";
	} else {
		return "ConversationVideo";
	}
}

class ConversationCelLoader : public DeferredLoader {
public:
	ConversationCelLoader(ConversationCel &owner) : _owner(owner) {}

private:
	bool loadInner() override;

	ConversationCel &_owner;
};

bool ConversationCelLoader::loadInner() {
	for (uint i = _owner._curFrame; i < _owner._celNames[0].size(); ++i) {
		for (uint j = 0; j < _owner._celRObjects.size(); ++j) {
			if (!_owner._celCache.contains(_owner._celNames[j][i])) {
				_owner.loadCel(_owner._celNames[j][i], _owner._treeNames[j]);
				return false;
			}
		}
	}

	return true;
}

ConversationCel::~ConversationCel() {
	// Make sure there isn't a single-frame gap between conversation scenes where
	// the character is invisible
	g_nancy->_graphics->suppressNextDraw();
}

void ConversationCel::init() {
	_curFrame = _firstFrame;
	_nextFrameTime = g_nancy->getTotalPlayTime();
	ConversationSound::init();

	_loaderPtr.reset(new ConversationCelLoader(*this));
	auto castedPtr = _loaderPtr.dynamicCast<DeferredLoader>();
	g_nancy->addDeferredLoader(castedPtr);

	for (uint i = 0; i < _treeNames.size(); ++i) {
		if (_treeNames[i].size()) {
			_celRObjects.push_back(RenderedCel());
		} else break;
	}

	registerGraphics();
}

void ConversationCel::registerGraphics() {
	for (uint i = 0; i < _celRObjects.size(); ++i) {
		_celRObjects[i]._z = 9 + _drawingOrder[i];
		_celRObjects[i].setVisible(true);
		_celRObjects[i].setTransparent(true);
		_celRObjects[i].registerGraphics();
	}

	RenderActionRecord::registerGraphics();
}

void ConversationCel::updateGraphics() {
	uint32 currentTime = g_nancy->getTotalPlayTime();

	if (_state == kRun && currentTime > _nextFrameTime && _curFrame < MIN<uint>(_lastFrame + 1, _celNames[0].size())) {
		for (uint i = 0; i < _celRObjects.size(); ++i) {
			Cel &cel = loadCel(_celNames[i][_curFrame], _treeNames[i]);
			if (_overrideTreeRects[i] == kCelOverrideTreeRectsOn) {
				_celRObjects[i]._drawSurface.create(cel.surf, _overrideRectSrcs[i]);
				_celRObjects[i].moveTo(_overrideRectDests[i]);
			} else {
				_celRObjects[i]._drawSurface.create(cel.surf, cel.src);
				_celRObjects[i].moveTo(cel.dest);
			}
		}

		_nextFrameTime += _frameTime;
		++_curFrame;
	}
}

void ConversationCel::readData(Common::SeekableReadStream &stream) {
	Common::String xsheetName;
	readFilename(stream, xsheetName);

	readFilenameArray(stream, _treeNames, 4);
	readXSheet(stream, xsheetName);

	// Continue reading the AR stream

	// Something related to quality
	stream.skip(3);

	_firstFrame = stream.readUint16LE();
	_lastFrame = stream.readUint16LE();

	stream.skip(6);

	_drawingOrder.resize(4);
	for (uint i = 0; i < 4; ++i) {
		_drawingOrder[i] = stream.readByte();
	}

	_overrideTreeRects.resize(4);
	for (uint i = 0; i < 4; ++i) {
		_overrideTreeRects[i] = stream.readByte();
	}

	readRectArray(stream, _overrideRectSrcs, 4);
	readRectArray(stream, _overrideRectDests, 4);

	ConversationSound::readData(stream);
}

void ConversationCel::readXSheet(Common::SeekableReadStream &stream, const Common::String &xsheetName) {
	Common::SeekableReadStream *xsheet = SearchMan.createReadStreamForMember(Common::Path(xsheetName));

	// Read the xsheet and load all images into the arrays
	// Completely unoptimized, the original engine uses a buffer
	xsheet->seek(0);
	Common::String signature = xsheet->readString('\0', 18);
	if (signature != "XSHEET WayneSikes") {
		warning("XSHEET signature doesn't match!");
		return;
	}

	xsheet->seek(0x22);
	uint numFrames = xsheet->readUint16LE();
	xsheet->skip(2);
	_frameTime = xsheet->readUint16LE();
	xsheet->skip(2);

	_celNames.resize(4, Common::Array<Common::Path>(numFrames));
	for (uint i = 0; i < numFrames; ++i) {
		for (uint j = 0; j < _celNames.size(); ++j) {
			readFilename(*xsheet, _celNames[j][i]);
		}

		// 4 unknown values
		xsheet->skip(8);
	}

	delete xsheet;
}

bool ConversationCel::isVideoDonePlaying() {
	return _curFrame >= MIN<uint>(_lastFrame, _celNames[0].size()) && _nextFrameTime <= g_nancy->getTotalPlayTime();
}

ConversationCel::Cel &ConversationCel::loadCel(const Common::Path &name, const Common::String &treeName) {
	// Assumes head and body cels will be named differently
	if (_celCache.contains(name)) {
		return _celCache[name];
	}

	Cel &newCel = _celCache.getOrCreateVal(name);
	g_nancy->_resource->loadImage(name, newCel.surf, treeName, &newCel.src, &newCel.dest);
	return _celCache[name];
}

void ConversationSoundTerse::readData(Common::SeekableReadStream &stream) {
	readTerseData(stream);
}

void ConversationCelTerse::readData(Common::SeekableReadStream &stream) {
	Common::String xsheetName;
	readFilename(stream, xsheetName);

	readFilenameArray(stream, _treeNames, 2); // Only 2
	readXSheet(stream, xsheetName);

	_lastFrame = stream.readUint16LE();
	_drawingOrder = { 1, 0, 2, 3 };
	_overrideTreeRects.resize(4, kCelOverrideTreeRectsOff);

	readTerseData(stream);
}

} // End of namespace Action
} // End of namespace Nancy
