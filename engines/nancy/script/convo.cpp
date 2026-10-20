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

#include "common/archive.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

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

// The parts of an XSheet
static const uint kXSheetFrameCountOffset = 0x22;
static const uint kXSheetMaxTrees = 4;
//static const uint kXSheetNameSize = 33;

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

	for (Tree *tree : _trees) {
		delete tree;
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

bool ConvoRecord::loadXSheet(const Common::String &name) {
	Common::SeekableReadStream *xsheet = SearchMan.createReadStreamForMember(Common::Path(name));
	if (!xsheet) {
		warning("Could not open XSheet file %s", name.c_str());
		return false;
	}

	xsheet->seek(kXSheetFrameCountOffset);
	_numFrames = xsheet->readUint16LE();
	const uint numTrees = MIN<uint>(xsheet->readUint16LE(), kXSheetMaxTrees);

	Common::Array<Common::String> treeNames(kXSheetMaxTrees);
	for (Common::String &treeName : treeNames) {
		readFilename(*xsheet, treeName);
	}

	// Nancy20 lines the rectangles up on four bytes
	if (g_nancy->getGameType() >= kGameTypeNancy20) {
		xsheet->skip(2);
	}

	// Where each part goes in the viewport; the right and bottom edge are left out
	Common::Array<Common::Rect> treeRects;
	readRectArray(*xsheet, treeRects, kXSheetMaxTrees);

	_framesPerSecond = xsheet->readUint32LE();

	for (uint i = 0; i < numTrees; ++i) {
		Tree *tree = new Tree();
		tree->frames.resize(_numFrames);
		_trees.push_back(tree);
	}

	// Every frame has a movie frame for each of the parts there can be
	for (uint frame = 0; frame < _numFrames; ++frame) {
		for (uint i = 0; i < kXSheetMaxTrees; ++i) {
			const int32 movieFrame = xsheet->readSint32LE();
			if (i < numTrees) {
				_trees[i]->frames[frame] = movieFrame;
			}
		}

		xsheet->skip(8);
	}

	delete xsheet;

	for (uint i = 0; i < numTrees; ++i) {
		Tree &tree = *_trees[i];
		Common::Rect dest = treeRects[i];
		if (!dest.isEmpty()) {
			--dest.right;
			--dest.bottom;
		}

		if (dest.isEmpty() || !tree.player.loadFile(Common::Path(treeNames[i]))) {
			warning("Could not load conversation movie %s of XSheet %s", treeNames[i].c_str(), name.c_str());
			return false;
		}

		// The movies come with an alpha plane, which they are drawn by
		tree.dest = dest;
		tree.graphic.init();
		tree.graphic.moveTo(dest);
		tree.graphic.setTransparent(false);
		tree.graphic.setVisible(false);
		tree.graphic.registerGraphics();

		showMovieFrame(tree, (tree.frames.empty() || isTreeFreeRunning(tree)) ? 0 : tree.frames[0]);
	}

	onZChanged();
	return true;
}

// The first part is drawn over the ones after it
void ConvoRecord::onZChanged() {
	for (uint i = 0; i < _trees.size(); ++i) {
		_trees[i]->graphic.setScriptZ(getZ() + (i == 0 ? 1 : 0), getContext());
	}
}

void ConvoRecord::updateGraphics() {
	for (Tree *tree : _trees) {
		tree->graphic.setVisible(hasStarted());
	}
}

// The movies are padded to a multiple of 16 pixels; the picture itself is as
// large as the place it goes to
void ConvoRecord::drawTree(Tree &tree, const Graphics::Surface &frame) {
	const Common::Rect picture(MIN<int>(frame.w, tree.dest.width()), MIN<int>(frame.h, tree.dest.height()));
	GraphicsManager::copyToManaged(frame.getSubArea(picture), tree.graphic._drawSurface);
	tree.graphic.setNeedsRedraw(true);
}

void ConvoRecord::showMovieFrame(Tree &tree, int32 movieFrame) {
	if (movieFrame == tree.shownFrame) {
		return;
	}

	const Graphics::Surface *frame = tree.player.decodeNextFrame(movieFrame);
	if (frame) {
		drawTree(tree, *frame);
		tree.shownFrame = movieFrame;
	}
}

bool ConvoRecord::isTreeFreeRunning(const Tree &tree) const {
	return _frame < tree.frames.size() && tree.frames[_frame] == kFreeRunning;
}

void ConvoRecord::showFrame(uint frame) {
	if (frame == _frame) {
		return;
	}

	_frame = frame;

	for (Tree *tree : _trees) {
		if (frame < tree->frames.size() && tree->frames[frame] != kFreeRunning) {
			showMovieFrame(*tree, tree->frames[frame]);
		}
	}
}

// The line is over once the XSheet has reached its last frame, and the parts
// that play by themselves have played through
bool ConvoRecord::isVideoDone() const {
	if (_isLineSkipped) {
		return true;
	}

	for (const Tree *tree : _trees) {
		if (isTreeFreeRunning(*tree) && !tree->player.endOfVideo()) {
			return false;
		}
	}

	return _numFrames == 0 || _frame == _numFrames - 1;
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
		// stop on their last frame, so they stay together with the rest
		for (Tree *tree : _trees) {
			if (isTreeFreeRunning(*tree) && tree->player.getFrameCount() > 0) {
				const Graphics::Surface *movieFrame = tree->player.decodeNextFrame(tree->player.getFrameCount() - 1);
				if (movieFrame) {
					drawTree(*tree, *movieFrame);
				}
			}
		}

		if (_isVideo && _numFrames > 0) {
			showFrame(_numFrames - 1);
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
		loadXSheet(soundToXSheetName(_soundNames[0]));
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
		for (Tree *tree : _trees) {
			// The parts that play by themselves keep their own time
			if (isTreeFreeRunning(*tree)) {
				tree->player.start();
			}
		}

		_state = kPlayLine;
		break;
	case kPlayLine: {
		for (Tree *tree : _trees) {
			if (!_isLineSkipped && isTreeFreeRunning(*tree) && tree->player.needsUpdate()) {
				const Graphics::Surface *movieFrame = tree->player.decodeNextFrame();
				if (movieFrame) {
					drawTree(*tree, *movieFrame);
				}
			}
		}

		if (_isVideo && _numFrames > 0 && !_isLineSkipped) {
			// The frames keep up with the sound, and go on one at a time after it
			uint frame = _frame + 1;
			if (isSoundPlaying) {
				frame = (uint)MAX(0.0, (sceneTime - _lineStartTime) * _framesPerSecond);
			}

			showFrame(MIN<uint>(frame, _numFrames - 1));
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
