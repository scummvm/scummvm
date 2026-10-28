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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/convo.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/scenecontext.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Script {

static const char *const kConvoResponseType = "ND.ConvoResponse";
static const char *const kNavLogicType = "ND.NavLogic";

static const uint16 kCharacterChannel = 24;
static const uint16 kPlayerChannel = 25;
static const uint16 kConvoVolume = 85;

// The sound of a video conversation is named after its XSheet. In Nancy20 the
// two names end differently: GFL01_XS goes with the sound GFL01_SFX.
static Common::String xsheetToSoundName(const Common::String &xsheetName) {
	if (g_nancy->getGameType() <= kGameTypeNancy19) {
		return xsheetName;
	}

	Common::String soundName = xsheetName;
	soundName.toUppercase();
	Common::replace(soundName, "_XS", "_SFX");
	return soundName;
}

static Common::String soundToXSheetName(const Common::String &soundName) {
	if (g_nancy->getGameType() <= kGameTypeNancy19) {
		return soundName;
	}

	Common::String xsheetName = soundName;
	xsheetName.toUppercase();
	Common::replace(xsheetName, "_SFX", "_XS");
	return xsheetName;
}

// The record a conversation or a response keeps for later, which has to be a NavLogic
static Record *checkHeldRecord(lua_State *L, int index) {
	Record *record = checkRecord(L, index);
	if (strcmp(record->getTypeName(), kNavLogicType) != 0) {
		luaL_typerror(L, index, kNavLogicType);
	}

	record->hold();
	return record;
}

ConvoRecord::ConvoRecord(SceneContext &context, bool isVideo) :
		Record(context, getRecordTypeName(isVideo ? kRecordVideoConvo : kRecordSoundConvo)),
		_isVideo(isVideo) {}

ConvoRecord::~ConvoRecord() {
	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (_hasDialog && ui) {
		ui->removeEngineTarget("Engine_Convo");
		ui->endConversation();
	}
}

void ConvoRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "responses")) {
		if (!lua_istable(L, -1)) {
			luaL_error(L, "field %s must be a table", "responses");
		}

		const int numResponses = (int)lua_objlen(L, -1);
		for (int i = 1; i <= numResponses; ++i) {
			lua_rawgeti(L, -1, i);
			_responses.push_back(*(ConvoResponse **)checkUserData(L, -1, kConvoResponseType));
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}

	Common::String xsheetName;
	if (readField(L, tableIndex, "xsheet", xsheetName)) {
		_soundNames.push_back(xsheetToSoundName(xsheetName));
	}

	if (pushField(L, tableIndex, "sounds")) {
		readStringList(L, -1, _soundNames);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "responseFiles")) {
		readStringList(L, -1, _responseFiles);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "OnDone")) {
		if (!lua_isfunction(L, -1)) {
			luaL_error(L, "field %s must be a function", "OnDone");
		}

		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "default")) {
		_default = checkHeldRecord(L, -1);
		lua_pop(L, 1);
	}

	if (_soundNames.empty()) {
		luaL_error(L, "A conversation needs an xsheet or sounds");
	}
}

void ConvoRecord::onZChanged() {
	_xsheet.setZ(getZ(), getContext());
}

void ConvoRecord::updateGraphics() {
	_xsheet.setVisible(hasStarted());
}

// The line is over once the XSheet has reached its last frame, and the parts
// that play by themselves have played through
bool ConvoRecord::isVideoDone() const {
	if (_isLineSkipped) {
		return true;
	}

	if (!_xsheet.isFreeRunningDone()) {
		return false;
	}

	return _xsheet.getNumFrames() == 0 || _xsheet.getFrame() == _xsheet.getNumFrames() - 1;
}

// The character stops talking, and the video goes to the end of the line, where
// the responses wait to be picked. A response that is being said stops as well.
bool ConvoRecord::skipSpeech() {
	if (!isActive() || isDone()) {
		return false;
	}

	switch (_state) {
	case kWaitForLine:
	case kPlayLine:
		if (_isLineSkipped) {
			return false;
		}

		g_nancy->_sound->stopSound(kCharacterChannel);
		_hasSoundStarted = false;
		_isLineSkipped = true;

		// The parts that play by themselves, like the head that moves its lips,
		// stop on their last frame
		_xsheet.parkFreeRunning();

		if (_isVideo && _xsheet.getNumFrames() > 0) {
			_xsheet.showFrame(_xsheet.getNumFrames() - 1);
		}

		return true;
	case kWaitForResponse:
	case kPlayResponse:
		g_nancy->_sound->stopSound(kPlayerChannel);
		_state = kPlayResponse;
		return true;
	default:
		return false;
	}
}

void ConvoRecord::playSound(const Common::String &name, uint16 channel) {
	_sound = SoundDescription();
	_sound.name = name;
	_sound.channelID = channel;
	_sound.numLoops = 1;
	_sound.volume = kConvoVolume;

	g_nancy->_sound->loadSound(_sound);
	g_nancy->_sound->playSound(_sound);
	_hasSoundStarted = g_nancy->_sound->isSoundPlaying(_sound);
}

bool ConvoRecord::isResponseOnOffer(const ConvoResponse &response) {
	if (response.activeFunction < 0) {
		return true;
	}

	lua_State *L = getLua();
	lua_rawgeti(L, LUA_REGISTRYINDEX, response.activeFunction);
	if (!LuaState::call(L, 0, 1, "Error in active function")) {
		return false;
	}

	const bool isOnOffer = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);
	return isOnOffer;
}

void ConvoRecord::runOnce() {
	lua_State *L = getLua();
	SceneContext &context = getContext();

	// A scene only gets to have one conversation
	if (context.getConvo()) {
		lua_pushnil(L);
		storeValue("OnDone");
		finish();
		return;
	}

	context.setConvo(this);

	if (_isVideo) {
		_xsheet.load(soundToXSheetName(_soundNames[0]));
		onZChanged();
	}

	// The scripts named by the record add the responses they have to its own
	for (const Common::String &fileName : _responseFiles) {
		context.include(fileName);
	}

	for (ConvoResponse *response : _responses) {
		if (isResponseOnOffer(*response)) {
			_offeredResponses.push_back(response);
		}
	}

	if (_offeredResponses.empty() && !_default) {
		warning("Conversation %s has neither responses nor a default", _soundNames[0].c_str());
	}

	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (ui) {
		Common::String caption;
		findText(_soundNames[0], caption);

		Common::Array<Common::String> responseTexts;
		for (const ConvoResponse *response : _offeredResponses) {
			Common::String text = response->soundName;
			if (!findText(response->soundName, text)) {
				warning("Missing convo text for %s", response->soundName.c_str());
			}

			responseTexts.push_back(text);
		}

		ui->startConversation(caption, responseTexts);
		ui->addEngineTarget("Engine_Convo", this);
		_hasDialog = true;
	}
}

void ConvoRecord::onEngineAction(const UI::NDUI::Action &action) {
	const int index = (int)action.value;
	if (action.type == UI::NDUI::kActionSetValue && _pickedResponse < 0 && index >= 0 && index < (int)_offeredResponses.size()) {
		_pickedResponse = index;
	}
}

void ConvoRecord::run(double sceneTime, double elapsedTime) {
	if (isDone()) {
		// There was a conversation in the scene already
		return;
	}

	const bool isSoundPlaying = g_nancy->_sound->isSoundPlaying(_sound);
	UI::NDUI::Manager *ui = NancySceneState.getNDUI();

	switch (_state) {
	case kStartLine:
		_isLineSkipped = false;
		playSound(_soundNames[_curSound], kCharacterChannel);

		if (_curSound > 0 && ui) {
			Common::String caption;
			findText(_soundNames[_curSound], caption);
			ui->takeAction(UI::NDUI::kActionSetText, "ConvoCCText", caption);
		}

		_state = kWaitForLine;
		break;
	case kWaitForLine:
		// A sound that is missing counts as one that is over
		if (_hasSoundStarted && !isSoundPlaying) {
			break;
		}

		_lineStartTime = sceneTime;
		_xsheet.startFreeRunning();
		_state = kPlayLine;
		break;
	case kPlayLine: {
		if (!_isLineSkipped) {
			_xsheet.updateFreeRunning();
		}

		const uint numFrames = _xsheet.getNumFrames();
		if (_isVideo && numFrames > 0 && !_isLineSkipped) {
			// The frames keep up with the sound, and go on one at a time after it
			uint frame = _xsheet.getFrame() + 1;
			if (isSoundPlaying) {
				frame = (uint)MAX(0.0, (sceneTime - _lineStartTime) * _xsheet.getFramesPerSecond());
			}

			_xsheet.showFrame(MIN<uint>(frame, numFrames - 1));
		}

		// The player has to pick a response before anything else happens. The
		// sounds of a conversation without a video play on in the meantime.
		if (!_offeredResponses.empty() && _pickedResponse < 0) {
			if (!_isVideo && !isSoundPlaying && _curSound + 1 < _soundNames.size()) {
				++_curSound;
				_state = kStartLine;
			}

			break;
		}

		if (_isVideo ? isVideoDone() : !isSoundPlaying) {
			if (!_isVideo && _curSound + 1 < _soundNames.size()) {
				++_curSound;
				_state = kStartLine;
			} else {
				_state = kWaitForLineEnd;
			}
		}

		break;
	}
	case kWaitForLineEnd:
		if (!isSoundPlaying) {
			_state = kStartResponse;
		}

		break;
	case kStartResponse:
		if (_pickedResponse < 0) {
			conclude();
			break;
		}

		playSound(_offeredResponses[_pickedResponse]->soundName, kPlayerChannel);
		if (ui) {
			ui->disableOtherResponses((uint)_pickedResponse);
		}

		_state = _hasSoundStarted ? kWaitForResponse : kPlayResponse;
		break;
	case kWaitForResponse:
		if (isSoundPlaying) {
			_state = kPlayResponse;
		}

		break;
	case kPlayResponse:
		if (!isSoundPlaying) {
			conclude();
		}

		break;
	}
}

// The record of the response that was picked runs, or the default without one
void ConvoRecord::conclude() {
	g_nancy->_sound->stopSound(kCharacterChannel);
	g_nancy->_sound->stopSound(kPlayerChannel);

	Record *next = _pickedResponse >= 0 ? _offeredResponses[_pickedResponse]->onFinished : _default;
	if (next) {
		next->release();
	}

	finish();
}

// ConvoResponse:New{ sounds = <name>, onFinished = <NavLogic>, active = <function> }
static int convoResponseNew(lua_State *L) {
	if (!lua_istable(L, 2)) {
		return luaL_argerror(L, 2, "Can only make a ConvoResponse from a table");
	}

	SceneContext *context = LuaState::getContext(L);
	if (!context) {
		return luaL_error(L, "A ConvoResponse can only be made in a scene");
	}

	Common::Array<Common::String> soundNames;
	if (pushField(L, 2, "sounds")) {
		if (!lua_isstring(L, -1) && !lua_istable(L, -1)) {
			return luaL_error(L, "'sounds' field in ConvoResponse must be a string or a table");
		}

		readStringList(L, -1, soundNames);
		lua_pop(L, 1);
	}

	if (soundNames.empty()) {
		return luaL_error(L, "'sounds' table must have at least one field");
	}

	ConvoResponse *response = new ConvoResponse();
	context->addConvoResponse(response);
	response->soundName = soundNames[0];

	if (pushField(L, 2, "onFinished")) {
		response->onFinished = checkHeldRecord(L, -1);
		lua_pop(L, 1);
	}

	if (pushField(L, 2, "active")) {
		if (!lua_isfunction(L, -1)) {
			return luaL_argerror(L, 2, "'active' field in ConvoResponse must be a function");
		}

		response->activeFunction = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	*(ConvoResponse **)pushUserData(L, sizeof(ConvoResponse *), kConvoResponseType) = response;
	return 1;
}

// Convo:Add(response), for the scripts a conversation runs to get its responses
static int convoAdd(lua_State *L) {
	ConvoResponse *response = *(ConvoResponse **)checkUserData(L, 2, kConvoResponseType);

	SceneContext *context = LuaState::getContext(L);
	ConvoRecord *convo = context ? context->getConvo() : nullptr;
	if (!convo) {
		return luaL_error(L, "There is no conversation to add a response to");
	}

	convo->addResponse(response);
	return 0;
}

void registerConvoAPI(lua_State *L) {
	static const luaL_Reg kResponseFunctions[] = {
		{ nullptr, nullptr }
	};

	static const luaL_Reg kResponseHelperFunctions[] = {
		{ "__index", indexFunctions },
		{ "New", convoResponseNew },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kConvoFunctions[] = {
		{ "__index", indexFunctions },
		{ "Add", convoAdd },
		{ nullptr, nullptr }
	};

	registerType(L, kConvoResponseType, kResponseFunctions);
	registerGlobal(L, "ConvoResponse", "ND.ConvoResponseHelper", kResponseHelperFunctions);
	registerGlobal(L, "Convo", "ND.Convo", kConvoFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
