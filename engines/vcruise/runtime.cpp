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

#include "common/formats/winexe.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/stream.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"
#include "graphics/managed_surface.h"

#include "audio/decoders/wave.h"
#include "audio/audiostream.h"

#include "video/avi_decoder.h"

#include "vcruise/audio_player.h"
#include "vcruise/runtime.h"
#include "vcruise/script.h"
#include "vcruise/textparser.h"


namespace VCruise {

AnimationDef::AnimationDef() : animNum(0), firstFrame(0), lastFrame(0) {
}

InteractionDef::InteractionDef() : objectType(0), interactionID(0) {
}

void MapDef::clear() {
	for (uint screen = 0; screen < kNumScreens; screen++)
		for (uint direction = 0; direction < kNumDirections; direction++)
			screenDirections[screen][direction].reset();
}

const MapScreenDirectionDef *MapDef::getScreenDirection(uint screen, uint direction) {
	if (screen < kFirstScreen)
		return nullptr;

	screen -= kFirstScreen;

	if (screen >= kNumScreens)
		return nullptr;

	return screenDirections[screen][direction].get();
}

void Runtime::RenderSection::init(const Common::Rect &paramRect, const Graphics::PixelFormat &fmt) {
	rect = paramRect;
	surf.reset(new Graphics::ManagedSurface(paramRect.width(), paramRect.height(), fmt));
	surf->fillRect(Common::Rect(0, 0, surf->w, surf->h), 0xffffffff);
}

Runtime::Runtime(OSystem *system, Audio::Mixer *mixer, const Common::FSNode &rootFSNode, VCruiseGameID gameID)
	: _system(system), _mixer(mixer), _roomNumber(1), _screenNumber(0), _direction(0), _havePanAnimations(0), _loadedRoomNumber(0), _activeScreenNumber(0),
	  _gameState(kGameStateBoot), _gameID(gameID), _rootFSNode(rootFSNode), _havePendingScreenChange(false), _scriptNextInstruction(0),
	  _escOn(false), _loadedAnimation(0), _animFrameNumber(0), _animLastFrame(0), _animDecoderState(kAnimDecoderStateStopped) {

	for (uint i = 0; i < kNumDirections; i++)
		_haveIdleAnimations[i] = false;

	_logDir = _rootFSNode.getChild("Log");
	if (!_logDir.exists() || !_logDir.isDirectory())
		error("Couldn't resolve Log directory");

	_mapDir = _rootFSNode.getChild("Map");
	if (!_mapDir.exists() || !_mapDir.isDirectory())
		error("Couldn't resolve Map directory");

	_sfxDir = _rootFSNode.getChild("Sfx");
	if (!_sfxDir.exists() || !_sfxDir.isDirectory())
		error("Couldn't resolve Sfx directory");

	_animsDir = _rootFSNode.getChild("Anims");
	if (!_animsDir.exists() || !_animsDir.isDirectory())
		error("Couldn't resolve Anims directory");
}

Runtime::~Runtime() {
}

void Runtime::initSections(Common::Rect gameRect, Common::Rect menuRect, Common::Rect trayRect, const Graphics::PixelFormat &pixFmt) {
	_gameSection.init(gameRect, pixFmt);
	_menuSection.init(menuRect, pixFmt);
	_traySection.init(trayRect, pixFmt);
}

void Runtime::loadCursors(const char *exeName) {
	Common::SharedPtr<Common::WinResources> winRes(Common::WinResources::createFromEXE(exeName));
	if (!winRes)
		error("Couldn't open executable file %s", exeName);

	Common::Array<Common::WinResourceID> cursorGroupIDs = winRes->getIDList(Common::kWinGroupCursor);
	for (Common::Array<Common::WinResourceID>::const_iterator it = cursorGroupIDs.begin(), itEnd = cursorGroupIDs.end(); it != itEnd; ++it) {
		const Common::WinResourceID &id = *it;

		Common::SharedPtr<Graphics::WinCursorGroup> cursorGroup(Graphics::WinCursorGroup::createCursorGroup(winRes.get(), *it));
		if (!winRes) {
			warning("Couldn't load cursor group");
			continue;
		}

		Common::String nameStr = id.getString();
		if (nameStr.size() == 8 && nameStr.substr(0, 7) == "CURSOR_") {
			char c = nameStr[7];
			if (c >= '0' && c <= '9') {
				uint shortID = c - '0';
				if (shortID >= _cursorsShort.size())
					_cursorsShort.resize(shortID + 1);
				_cursorsShort[shortID] = cursorGroup;
			}
		} else if (nameStr.size() == 13 && nameStr.substr(0, 11) == "CURSOR_CUR_") {
			char c1 = nameStr[11];
			char c2 = nameStr[12];
			if (c1 >= '0' && c1 <= '9' && c2 >= '0' && c2 <= '9') {
				uint longID = (c1 - '0') * 10 + (c2 - '0');
				if (longID >= _cursors.size())
					_cursors.resize(longID + 1);
				_cursors[longID] = cursorGroup;
			}
		}
	}
}

bool Runtime::runFrame() {
	bool moreActions = true;
	while (moreActions) {
		moreActions = false;
		switch (_gameState) {
		case kGameStateBoot:
			moreActions = bootGame();
			break;
		case kGameStateQuit:
			return false;
		case kGameStateIdle:
			moreActions = runIdle();
			break;
		case kGameStateScript:
			moreActions = runScript();
			break;
		case kGameStateWaitingForAnimation:
			moreActions = runWaitForAnimation();
			break;
		default:
			error("Unknown game state");
			return false;
		}
	}

	return true;
}

bool Runtime::bootGame() {
	debug(1, "Booting V-Cruise game...");
	loadIndex();
	debug(1, "Index loaded OK");

	_gameState = kGameStateIdle;

	if (_gameID == GID_REAH) {
		// TODO: Change to the logo instead (0xb1) instead when menus are implemented
		changeToScreen(1, 0xb0);
	} else
		error("Couldn't figure out what screen to start on");

	return true;
}

bool Runtime::runIdle() {

	if (_havePendingScreenChange) {
		_havePendingScreenChange = false;

		changeToScreen(_roomNumber, _screenNumber);
		return true;
	}

	return false;
}

bool Runtime::runWaitForAnimation() {
	if (!_animDecoder) {
		_gameState = kGameStateScript;
		return true;
	}

	bool needsFirstFrame = false;
	if (_animDecoderState == kAnimDecoderStatePaused) {
		_animDecoder->pauseVideo(false);
		_animDecoderState = kAnimDecoderStatePlaying;
		needsFirstFrame = true;
	} else if (_animDecoderState == kAnimDecoderStateStopped) {
		_animDecoder->start();
		_animDecoderState = kAnimDecoderStatePlaying;
		needsFirstFrame = true;
	}

	for (;;) {
		bool needNewFrame = needsFirstFrame || (_animDecoder->getTimeToNextFrame() == 0);
		needsFirstFrame = false;

		if (!needNewFrame)
			break;

		const Graphics::Surface *surface = _animDecoder->decodeNextFrame();
		if (!surface) {
			_gameState = kGameStateScript;
			return true;
		}

		Common::Rect copyRect = Common::Rect(0, 0, surface->w, surface->h);

		Common::Rect constraintRect = Common::Rect(0, 0, _gameSection.rect.width(), _gameSection.rect.height());

		copyRect = copyRect.findIntersectingRect(constraintRect);

		if (copyRect.isValidRect() || !copyRect.isEmpty()) {
			_gameSection.surf->blitFrom(*surface, copyRect, copyRect);
			_system->copyRectToScreen(_gameSection.surf->getBasePtr(copyRect.left, copyRect.top), _gameSection.surf->pitch, copyRect.left + _gameSection.rect.left, copyRect.top + _gameSection.rect.top, copyRect.width(), copyRect.height());
		}

		if (_animDecoder->getCurFrame() >= static_cast<int>(_animLastFrame)) {
			_animDecoder->pauseVideo(true);
			_animDecoderState = kAnimDecoderStatePaused;

			_gameState = kGameStateScript;
			return true;
		}
	}

	// Pump and handle events
	return false;
}

#ifdef DISPATCH_OP
#error "DISPATCH_OP already defined"
#endif

#define DISPATCH_OP(op) \
	case ScriptOps::k##op: this->scriptOp##op(arg); break

bool Runtime::runScript() {
	while (_gameState == kGameStateScript) {
		uint instrNum = _scriptNextInstruction;
		if (!_activeScript || instrNum >= _activeScript->instrs.size()) {
			terminateScript();
			return true;
		}

		_scriptNextInstruction++;

		const Instruction &instr = _activeScript->instrs[instrNum];
		int32 arg = instr.arg;

		switch (instr.op) {
			DISPATCH_OP(Number);
			DISPATCH_OP(Rotate);
			DISPATCH_OP(Angle);
			DISPATCH_OP(AngleGGet);
			DISPATCH_OP(Speed);
			DISPATCH_OP(SAnimL);
			DISPATCH_OP(ChangeL);

			DISPATCH_OP(AnimR);
			DISPATCH_OP(AnimF);
			DISPATCH_OP(AnimN);
			DISPATCH_OP(AnimG);
			DISPATCH_OP(AnimS);
			DISPATCH_OP(Anim);

			DISPATCH_OP(Static);
			DISPATCH_OP(VarLoad);
			DISPATCH_OP(VarStore);
			DISPATCH_OP(SetCursor);
			DISPATCH_OP(SetRoom);
			DISPATCH_OP(LMB);
			DISPATCH_OP(LMB1);
			DISPATCH_OP(SoundS1);
			DISPATCH_OP(SoundL2);

			DISPATCH_OP(Music);
			DISPATCH_OP(MusicUp);
			DISPATCH_OP(MusicDn);
			DISPATCH_OP(Parm1);
			DISPATCH_OP(Parm2);
			DISPATCH_OP(Parm3);
			DISPATCH_OP(ParmG);

			DISPATCH_OP(VolumeDn4);
			DISPATCH_OP(VolumeUp3);
			DISPATCH_OP(Random);
			DISPATCH_OP(Drop);
			DISPATCH_OP(Dup);
			DISPATCH_OP(Say3);
			DISPATCH_OP(SetTimer);
			DISPATCH_OP(LoSet);
			DISPATCH_OP(LoGet);
			DISPATCH_OP(HiSet);
			DISPATCH_OP(HiGet);

			DISPATCH_OP(Not);
			DISPATCH_OP(And);
			DISPATCH_OP(Or);
			DISPATCH_OP(CmpEq);

			DISPATCH_OP(BitLoad);
			DISPATCH_OP(BitSet0);
			DISPATCH_OP(BitSet1);

			DISPATCH_OP(Disc1);
			DISPATCH_OP(Disc2);
			DISPATCH_OP(Disc3);

			DISPATCH_OP(EscOn);
			DISPATCH_OP(EscOff);
			DISPATCH_OP(EscGet);
			DISPATCH_OP(BackStart);

			DISPATCH_OP(AnimName);
			DISPATCH_OP(ValueName);
			DISPATCH_OP(VarName);
			DISPATCH_OP(SoundName);
			DISPATCH_OP(CursorName);

			DISPATCH_OP(CheckValue);
			DISPATCH_OP(Jump);

		default:
			error("Unimplemented opcode %i", static_cast<int>(instr.op));
		}
	}

	return true;
}

#undef DISPATCH_OP

void Runtime::terminateScript() {
	_activeScript.reset();
	_scriptNextInstruction = 0;

	if (_gameState == kGameStateScript)
		_gameState = kGameStateIdle;

	if (_havePendingScreenChange)
		changeToScreen(_roomNumber, _screenNumber);
}

void Runtime::loadIndex() {
	Common::FSNode indexFSNode = _logDir.getChild("Index.txt");

	Common::ReadStream *stream = indexFSNode.createReadStream();
	if (!stream)
		error("Failed to open main index");

	Common::String blamePath = indexFSNode.getPath();

	TextParser parser(stream);

	Common::String token;
	TextParserState state;

	static const IndexPrefixTypePair parsePrefixes[] = {
		{"Room", kIndexParseTypeRoom},
		{"RRoom", kIndexParseTypeRRoom},
		{"YRoom", kIndexParseTypeYRoom},
		{"VRoom", kIndexParseTypeVRoom},
		{"TRoom", kIndexParseTypeTRoom},
		{"CRoom", kIndexParseTypeCRoom},
		{"SRoom", kIndexParseTypeSRoom},
	};

	IndexParseType indexParseType = kIndexParseTypeNone;
	uint currentRoomNumber = 0;

	for (;;) {
		char firstCh = 0;
		if (!parser.skipWhitespaceAndComments(firstCh, state))
			break;

		if (firstCh == '[') {
			if (!parser.parseToken(token, state))
				error("Index open bracket wasn't terminated");

			if (token == "NameRoom") {
				indexParseType = kIndexParseTypeNameRoom;
			} else {
				bool foundType = false;
				uint prefixLen = 0;
				for (const IndexPrefixTypePair &prefixTypePair : parsePrefixes) {
					uint len = strlen(prefixTypePair.prefix);
					if (token.size() > len && !memcmp(token.c_str(), prefixTypePair.prefix, len)) {
						indexParseType = prefixTypePair.parseType;
						foundType = true;
						prefixLen = len;
						break;
					}
				}

				if (!foundType)
					error("Unknown index heading type %s", token.c_str());

				currentRoomNumber = 0;
				for (uint i = prefixLen; i < token.size(); i++) {
					char digit = token[i];
					if (digit < '0' || digit > '9')
						error("Malformed room def");
					currentRoomNumber = currentRoomNumber * 10 + (token[i] - '0');
				}
			}

			parser.expect("]", blamePath);

			allocateRoomsUpTo(currentRoomNumber);
		} else {
			parser.requeue(&firstCh, 1, state);

			if (!parseIndexDef(parser, indexParseType, currentRoomNumber, blamePath))
				break;
		}
	}
}

void Runtime::changeToScreen(uint roomNumber, uint screenNumber) {
	bool changedRoom = (roomNumber != _loadedRoomNumber);
	bool changedScreen = (screenNumber != _activeScreenNumber) || changedRoom;

	_roomNumber = roomNumber;
	_screenNumber = screenNumber;

	_loadedRoomNumber = roomNumber;
	_activeScreenNumber = screenNumber;

	if (changedRoom) {
		// Scripts are not allowed
		assert(!_activeScript);

		_scriptSet.reset();

		Common::String logFileName = Common::String::format("Room%02i.log", static_cast<int>(roomNumber));
		Common::FSNode logFileNode = _logDir.getChild(logFileName);
		if (logFileNode.exists()) {
			if (Common::SeekableReadStream *logicFile = logFileNode.createReadStream()) {
				_scriptSet = compileLogicFile(*logicFile, static_cast<uint>(logicFile->size()), logFileNode.getPath());
				delete logicFile;
			}
		}

		_map.clear();

		Common::String mapFileName = Common::String::format("Room%02i.map", static_cast<int>(roomNumber));
		Common::FSNode mapFileNode = _mapDir.getChild(mapFileName);
		if (mapFileNode.exists()) {
			if (Common::SeekableReadStream *mapFile = mapFileNode.createReadStream()) {
				loadMap(mapFile);
				delete mapFile;
			}
		}
	}

	if (changedScreen) {
		if (_scriptSet) {
			Common::HashMap<uint, Common::SharedPtr<RoomScriptSet> >::const_iterator roomScriptIt = _scriptSet->roomScripts.find(_roomNumber);
			if (roomScriptIt != _scriptSet->roomScripts.end()) {
				Common::HashMap<uint, Common::SharedPtr<ScreenScriptSet> > &screenScriptsMap = roomScriptIt->_value->screenScripts;
				Common::HashMap<uint, Common::SharedPtr<ScreenScriptSet> >::const_iterator screenScriptIt = screenScriptsMap.find(_screenNumber);
				if (screenScriptIt != screenScriptsMap.end()) {
					const Common::SharedPtr<Script> &script = screenScriptIt->_value->entryScript;
					if (script)
						activateScript(script);
				}
			}
		}

		_havePanAnimations = false;

		for (uint i = 0; i < kNumDirections; i++)
			_haveIdleAnimations[i] = false;
	}
}

void Runtime::loadMap(Common::SeekableReadStream *stream) {
	byte screenDefOffsets[MapDef::kNumScreens * kNumDirections * 4];

	if (!stream->seek(16))
		error("Error skipping map file header");

	if (stream->read(screenDefOffsets, sizeof(screenDefOffsets)) != sizeof(screenDefOffsets))
		error("Error reading map offset table");

	for (uint screen = 0; screen < MapDef::kNumScreens; screen++) {
		for (uint direction = 0; direction < kNumDirections; direction++) {
			uint32 offset = READ_LE_UINT32(screenDefOffsets + (kNumDirections * screen + direction) * 4);
			if (!offset)
				continue;

			if (!stream->seek(offset))
				error("Error seeking to screen data");

			byte screenDefHeader[16];
			if (stream->read(screenDefHeader, 16) != 16)
				error("Error reading screen def header");

			uint16 numInteractions = READ_LE_UINT16(screenDefHeader + 0);

			if (numInteractions > 0) {
				Common::SharedPtr<MapScreenDirectionDef> screenDirectionDef(new MapScreenDirectionDef());
				screenDirectionDef->interactions.resize(numInteractions);

				for (uint i = 0; i < numInteractions; i++) {
					InteractionDef &idef = screenDirectionDef->interactions[i];

					byte interactionData[12];
					if (stream->read(interactionData, 12) != 12)
						error("Error reading interaction data");

					idef.rect = Common::Rect(READ_LE_INT16(interactionData + 0), READ_LE_INT16(interactionData + 2), READ_LE_INT16(interactionData + 4), READ_LE_INT16(interactionData + 6));
					idef.interactionID = READ_LE_UINT16(interactionData + 8);
					idef.objectType = READ_LE_UINT16(interactionData + 10);
				}

				_map.screenDirections[screen][direction] = screenDirectionDef;
			}
		}
	}
}

void Runtime::changeMusicTrack(int track) {
	_musicPlayer.reset();

	Common::String wavFileName = Common::String::format("Music-%02i.wav", static_cast<int>(track));
	Common::FSNode wavFileNode = _sfxDir.getChild(wavFileName);
	if (wavFileNode.exists()) {
		if (Common::SeekableReadStream *wavFile = wavFileNode.createReadStream()) {
			if (Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(wavFile, DisposeAfterUse::YES)) {
				Common::SharedPtr<Audio::AudioStream> loopingStream(Audio::makeLoopingAudioStream(audioStream, 0));

				_musicPlayer.reset(new AudioPlayer(_mixer, loopingStream, 255, 0));
			}
		}
	}
}

void Runtime::changeAnimation(const AnimationDef &animDef) {
	int animFile = animDef.animNum;
	if (animFile < 0)
		animFile = -animFile;

	if (_loadedAnimation != static_cast<uint>(animFile)) {
		_loadedAnimation = animFile;
		_animDecoder.reset();
		_animDecoderState = kAnimDecoderStateStopped;

		Common::String aviFileName = Common::String::format("Anim%04i.avi", animFile);
		Common::FSNode aviFileNode = _animsDir.getChild(aviFileName);
		if (!aviFileNode.exists()) {
			warning("Animation file %i is missing", animFile);
			return;
		}

		if (Common::SeekableReadStream *aviFile = aviFileNode.createReadStream()) {
			_animDecoder.reset(new Video::AVIDecoder());
			if (!_animDecoder->loadStream(aviFile)) {
				warning("Animation file %i could not be loaded", animFile);
				return;
			}
		} else {
			warning("Animation file %i is missing", animFile);
			return;
		}
	}

	if (_animDecoderState == kAnimDecoderStatePlaying) {
		_animDecoder->pauseVideo(true);
		_animDecoderState = kAnimDecoderStatePaused;
	}

	_animDecoder->seekToFrame(animDef.firstFrame);
	_animFrameNumber = animDef.firstFrame;
	_animLastFrame = animDef.lastFrame;
}

AnimationDef Runtime::stackArgsToAnimDef(const StackValue_t *args) const {
	AnimationDef def;
	def.animNum = args[0];
	def.firstFrame = args[1];
	def.lastFrame = args[2];

	return def;
}

void Runtime::activateScript(const Common::SharedPtr<Script> &script) {
	if (script->instrs.size() == 0)
		return;

	_activeScript = script;
	_scriptNextInstruction = 0;
	_gameState = kGameStateScript;
}

bool Runtime::parseIndexDef(TextParser &parser, IndexParseType parseType, uint roomNumber, const Common::String &blamePath) {
	Common::String lineText;
	parser.expectLine(lineText, blamePath, true);

	Common::MemoryReadStream lineStream(reinterpret_cast<const byte *>(lineText.c_str()), lineText.size(), DisposeAfterUse::NO);
	TextParser strParser(&lineStream);

	switch (parseType) {
	case kIndexParseTypeNameRoom: {
			uint nameRoomNumber = 0;
			Common::String name;
			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectUInt(nameRoomNumber, blamePath);

			allocateRoomsUpTo(nameRoomNumber);
			_roomDefs[nameRoomNumber]->name = name;
		} break;
	case kIndexParseTypeRoom: {
			Common::String name;

			AnimationDef animDef;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectInt(animDef.animNum, blamePath);
			strParser.expect(",", blamePath);
			strParser.expectUInt(animDef.firstFrame, blamePath);
			strParser.expect(",", blamePath);
			strParser.expectUInt(animDef.lastFrame, blamePath);
			_roomDefs[roomNumber]->animations[name] = animDef;
		} break;
	case kIndexParseTypeRRoom: {
			Common::String name;

			Common::Rect rect;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectShort(rect.left, blamePath);
			strParser.expect(",", blamePath);
			strParser.expectShort(rect.top, blamePath);
			strParser.expect(",", blamePath);
			strParser.expectShort(rect.right, blamePath);

			// Line 4210 in Reah contains an animation def instead of a rect def, detect this and discard
			if (!strParser.checkEOL()) {
				strParser.expect(",", blamePath);
				strParser.expectShort(rect.bottom, blamePath);

				_roomDefs[roomNumber]->rects[name] = rect;
			}

		} break;
	case kIndexParseTypeYRoom: {
			Common::String name;

			uint varSlot = 0;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectUInt(varSlot, blamePath);

			_roomDefs[roomNumber]->vars[name] = varSlot;
		} break;
	case kIndexParseTypeVRoom: {
			Common::String name;

			int value = 0;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectInt(value, blamePath);

			_roomDefs[roomNumber]->values[name] = value;
		} break;
	case kIndexParseTypeTRoom: {
			Common::String name;
			Common::String value;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectLine(value, blamePath, false);

			_roomDefs[roomNumber]->texts[name] = value;
		} break;
	case kIndexParseTypeCRoom: {
			Common::String name;
			int value;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectInt(value, blamePath);

			_roomDefs[roomNumber]->consts[name] = value;
		} break;
	case kIndexParseTypeSRoom: {
			Common::String name;
			int value;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectInt(value, blamePath);

			_roomDefs[roomNumber]->sounds[name] = value;
		} break;
	default:
		assert(false);
		return false;
	}

	return true;
}

void Runtime::allocateRoomsUpTo(uint roomNumber) {
	while (_roomDefs.size() <= roomNumber) {
		_roomDefs.push_back(Common::SharedPtr<RoomDef>(new RoomDef()));
	}
}

void Runtime::onLButtonDown(int16 x, int16 y) {
}

void Runtime::onLButtonUp(int16 x, int16 y) {
}

void Runtime::onMouseMove(int16 x, int16 y) {
}

void Runtime::onKeyDown(Common::KeyCode keyCode) {
	if (keyCode == Common::KEYCODE_ESCAPE) {
		if (_gameState == kGameStateWaitingForAnimation) {
			if (_escOn) {
				// Terminate the animation
				if (_animDecoderState == kAnimDecoderStatePlaying) {
					_animDecoder->pauseVideo(true);
					_animDecoderState = kAnimDecoderStatePaused;
				}
				_gameState = kGameStateScript;
				return;
			}
		}
	}
}

#ifdef CHECK_STACK
#error "CHECK_STACK is already defined"
#endif

#define PEEK_STACK(n)                                                                         \
	if (this->_scriptStack.size() < (n)) {                                                      \
		error("Script stack underflow");                                                      \
		return;                                                                               \
	}                                                                                         \
	const ScriptArg_t *stackArgs = &this->_scriptStack[this->_scriptStack.size() - (n)]


#define TAKE_STACK(n)                                                                         \
	StackValue_t stackArgs[n];                                                                \
	do {                                                                                      \
		const uint stackSize = _scriptStack.size();                                           \
		if (stackSize < (n)) {                                                                \
			error("Script stack underflow");                                                  \
			return;                                                                           \
		}                                                                                     \
		const StackValue_t *stackArgsPtr = &this->_scriptStack[stackSize - (n)];              \
		for (uint i = 0; i < (n); i++)                                                        \
			stackArgs[i] = stackArgsPtr[i];                                                   \
		this->_scriptStack.resize(stackSize - (n));                                           \
	} while (false)

void Runtime::scriptOpNumber(ScriptArg_t arg) {
	_scriptStack.push_back(arg);
}

void Runtime::scriptOpRotate(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + kAnimDefStackArgs);

	_panLeftAnimationDef = stackArgsToAnimDef(stackArgs + 0);
	_panRightAnimationDef = stackArgsToAnimDef(stackArgs + kAnimDefStackArgs);
	_havePanAnimations = true;
}

void Runtime::scriptOpAngle(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpAngleGGet(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpSpeed(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpSAnimL(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + 2);

	if (stackArgs[kAnimDefStackArgs] != 0)
		warning("sanimL second operand wasn't zero (what does that do?)");

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);
	uint direction = stackArgs[kAnimDefStackArgs + 1];

	if (direction >= kNumDirections)
		error("sanimL invalid direction");

	_haveIdleAnimations[direction] = true;
	_idleAnimations[direction] = animDef;
}

void Runtime::scriptOpChangeL(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpAnimR(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpAnimF(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpAnimN(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpAnimG(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpAnimS(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + 2);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);
	animDef.lastFrame = animDef.firstFrame;	// Static animation

	changeAnimation(animDef);

	_gameState = kGameStateWaitingForAnimation;	// FIXME
	_screenNumber = stackArgs[kAnimDefStackArgs + 0];
	_direction = stackArgs[kAnimDefStackArgs + 1];
	_havePendingScreenChange = true;
}

void Runtime::scriptOpAnim(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + 2);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);
	changeAnimation(animDef);

	_gameState = kGameStateWaitingForAnimation;
	_screenNumber = stackArgs[kAnimDefStackArgs + 0];
	_direction = stackArgs[kAnimDefStackArgs + 1];
	_havePendingScreenChange = true;
}

void Runtime::scriptOpStatic(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpVarLoad(ScriptArg_t arg) {
	TAKE_STACK(1);

	uint32 varID = (static_cast<uint32>(_roomNumber) << 16) | static_cast<uint32>(stackArgs[0]);

	Common::HashMap<uint32, int32>::const_iterator it = _variables.find(varID);
	if (it == _variables.end())
		_scriptStack.push_back(0);
	else
		_scriptStack.push_back(it->_value);
}

void Runtime::scriptOpVarStore(ScriptArg_t arg) {
	TAKE_STACK(2);

	uint32 varID = (static_cast<uint32>(_roomNumber) << 16) | static_cast<uint32>(stackArgs[1]);

	_variables[varID] = stackArgs[0];
}

void Runtime::scriptOpSetCursor(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpSetRoom(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpLMB(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpLMB1(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpSoundS1(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpSoundL2(ScriptArg_t arg) {
	TAKE_STACK(2);

	warning("Sound loop not implemented yet");
	(void)stackArgs;
}

void Runtime::scriptOpMusic(ScriptArg_t arg) {
	TAKE_STACK(1);

	changeMusicTrack(stackArgs[0]);
}

void Runtime::scriptOpMusicUp(ScriptArg_t arg) {
	TAKE_STACK(2);

	warning("Music volume changes are not implemented");
	(void)stackArgs;
}

void Runtime::scriptOpMusicDn(ScriptArg_t arg) {
	TAKE_STACK(2);

	warning("Music volume changes are not implemented");
	(void)stackArgs;
}

void Runtime::scriptOpParm1(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpParm2(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpParm3(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpParmG(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpVolumeDn4(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpVolumeUp3(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpRandom(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpDrop(ScriptArg_t arg) {
	TAKE_STACK(1);
	(void)stackArgs;
}

void Runtime::scriptOpDup(ScriptArg_t arg) {
	TAKE_STACK(1);

	_scriptStack.push_back(stackArgs[0]);
	_scriptStack.push_back(stackArgs[0]);
}

void Runtime::scriptOpSay3(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpSetTimer(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpLoSet(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpLoGet(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpHiSet(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpHiGet(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpNot(ScriptArg_t arg) {
	TAKE_STACK(1);

	_scriptStack.push_back((stackArgs[0] == 0) ? 1 : 0);
}

void Runtime::scriptOpAnd(ScriptArg_t arg) {
	TAKE_STACK(2);

	_scriptStack.push_back((stackArgs[0] != 0 && stackArgs[1] != 0) ? 1 : 0);
}

void Runtime::scriptOpOr(ScriptArg_t arg) {
	TAKE_STACK(2);

	_scriptStack.push_back((stackArgs[0] != 0 || stackArgs[1] != 0) ? 1 : 0);
}

void Runtime::scriptOpCmpEq(ScriptArg_t arg) {
	TAKE_STACK(2);

	_scriptStack.push_back((stackArgs[0] == stackArgs[1]) ? 1 : 0);
}

void Runtime::scriptOpBitLoad(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpBitSet0(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpBitSet1(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpDisc1(ScriptArg_t arg) {
	// Disc check, always pass
	TAKE_STACK(1);
	(void)stackArgs;
	_scriptStack.push_back(1);
}

void Runtime::scriptOpDisc2(ScriptArg_t arg) {
	// Disc check, always pass
	TAKE_STACK(2);
	(void)stackArgs;
	_scriptStack.push_back(1);
}

void Runtime::scriptOpDisc3(ScriptArg_t arg) {
	// Disc check, always pass
	TAKE_STACK(3);
	(void)stackArgs;
	_scriptStack.push_back(1);
}

void Runtime::scriptOpEscOn(ScriptArg_t arg) {
	TAKE_STACK(1);

	_escOn = (stackArgs[0] != 0);
}

void Runtime::scriptOpEscOff(ScriptArg_t arg) {
	_escOn = false;
}

void Runtime::scriptOpEscGet(ScriptArg_t arg) { error("Unimplemented opcode"); }
void Runtime::scriptOpBackStart(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpAnimName(ScriptArg_t arg) {
	// I doubt this is actually how it works internally but whatever
	if (_roomNumber >= _roomDefs.size())
		error("Can't resolve animation for room, room number was invalid");

	Common::SharedPtr<RoomDef> roomDef = _roomDefs[_roomNumber];
	if (!roomDef)
		error("Can't resolve animation for room, room number was invalid");


	Common::HashMap<Common::String, AnimationDef>::const_iterator it = roomDef->animations.find(_scriptSet->strings[arg]);
	if (it == roomDef->animations.end())
		error("Can't resolve animation for room, couldn't find animation '%s'", _scriptSet->strings[arg].c_str());

	_scriptStack.push_back(it->_value.animNum);
	_scriptStack.push_back(it->_value.firstFrame);
	_scriptStack.push_back(it->_value.lastFrame);
}


void Runtime::scriptOpValueName(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpVarName(ScriptArg_t arg) {
	if (_roomNumber >= _roomDefs.size())
		error("Invalid room number for var name op");

	const RoomDef *roomDef = _roomDefs[_roomNumber].get();
	if (!roomDef)
		error("Room def doesn't exist");

	const Common::String &varName = _scriptSet->strings[arg];

	Common::HashMap<Common::String, uint>::const_iterator it = roomDef->vars.find(varName);
	if (it == roomDef->vars.end())
		error("Var '%s' doesn't exist in room %i", varName.c_str(), static_cast<int>(_roomNumber));

	_scriptStack.push_back(it->_value);
}

void Runtime::scriptOpSoundName(ScriptArg_t arg) {
	const Common::String sndName = _scriptSet->strings[arg];

	warning("Sound IDs are not implemented yet");

	int32 soundID = 0;
	for (uint i = 0; i < 4; i++)
		soundID = soundID * 10 + (sndName[i] - '0');

	_scriptStack.push_back(soundID);
}

void Runtime::scriptOpCursorName(ScriptArg_t arg) { error("Unimplemented opcode"); }

void Runtime::scriptOpCheckValue(ScriptArg_t arg) {
	PEEK_STACK(1);

	if (arg == stackArgs[0])
		_scriptStack.pop_back();
	else
		_scriptNextInstruction++;
}

void Runtime::scriptOpJump(ScriptArg_t arg) {
	_scriptNextInstruction = arg;
}

void Runtime::drawFrame() {
	_system->updateScreen();
}


} // End of namespace VCruise
