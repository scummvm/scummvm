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
#include "common/file.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
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

ScriptEnvironmentVars::ScriptEnvironmentVars() : lmb(false) {
}

void Runtime::RenderSection::init(const Common::Rect &paramRect, const Graphics::PixelFormat &fmt) {
	rect = paramRect;
	surf.reset(new Graphics::ManagedSurface(paramRect.width(), paramRect.height(), fmt));
	surf->fillRect(Common::Rect(0, 0, surf->w, surf->h), 0xffffffff);
}

Runtime::OSEvent::OSEvent() : type(kOSEventTypeInvalid), keyCode(static_cast<Common::KeyCode>(0)) {
}

Runtime::Runtime(OSystem *system, Audio::Mixer *mixer, const Common::FSNode &rootFSNode, VCruiseGameID gameID)
	: _system(system), _mixer(mixer), _roomNumber(1), _screenNumber(0), _direction(0), _havePanAnimations(0), _loadedRoomNumber(0), _activeScreenNumber(0),
	  _gameState(kGameStateBoot), _gameID(gameID), _havePendingScreenChange(false), _havePendingReturnToIdleState(false), _scriptNextInstruction(0),
	  _escOn(false), _debugMode(false), _panoramaDirectionFlags(0),
	  _loadedAnimation(0), _animPendingDecodeFrame(0), _animDisplayingFrame(0), _animFirstFrame(0), _animLastFrame(0), _animDecoderState(kAnimDecoderStateStopped),
	  _animPlayWhileIdle(false), _idleIsOnInteraction(false), _idleInteractionID(0),
	  _lmbDown(false), _lmbDragging(false), _lmbDownTime(0) {

	for (uint i = 0; i < kNumDirections; i++)
		_haveIdleAnimations[i] = false;
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
	for (const Common::WinResourceID &id : cursorGroupIDs) {
		Common::SharedPtr<Graphics::WinCursorGroup> cursorGroup(Graphics::WinCursorGroup::createCursorGroup(winRes.get(), id));
		if (!winRes) {
			warning("Couldn't load cursor group");
			continue;
		}

		Common::String nameStr = id.getString();
		if (nameStr.matchString("CURSOR_#")) {
			char c = nameStr[7];

			uint shortID = c - '0';
			if (shortID >= _cursorsShort.size())
				_cursorsShort.resize(shortID + 1);
			_cursorsShort[shortID] = cursorGroup;
		} else if (nameStr.matchString("CURSOR_CUR_##")) {
			char c1 = nameStr[11];
			char c2 = nameStr[12];

			uint longID = (c1 - '0') * 10 + (c2 - '0');
			if (longID >= _cursors.size())
				_cursors.resize(longID + 1);
			_cursors[longID] = cursorGroup;
		}
	}

	if (_gameID == GID_REAH) {
		_namedCursors["CUR_TYL"] = 13;		// Tyl = back
		//_namedCursors["CUR_NIC"] = ?		// Nic = nothing
		//_namedCursors["CUR_WEZ"] = 50		// Wez = call? FIXME
		_namedCursors["CUR_LUPA"] = 21;		// Lupa = magnifier, could be 36 too?
		_namedCursors["CUR_NAC"] = 35;		// Nac = top?  Not sure.  But this is the finger pointer.
		_namedCursors["CUR_PRZOD"] = 1;		// Przod = forward

		// CUR_ZOSTAW is in the executable memory but appears to be unused
	}
}

void Runtime::setDebugMode(bool debugMode) {
	if (debugMode) {
		_debugMode = true;
		_gameDebugBackBuffer.init(_gameSection.rect, _gameSection.surf->format);
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

	// Discard any unconsumed OS events
	OSEvent evt;
	while (popOSEvent(evt)) {
		// Do nothing
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

		_havePendingReturnToIdleState = true;

		changeToScreen(_roomNumber, _screenNumber);
		return true;
	}

	if (_havePendingReturnToIdleState) {
		_havePendingReturnToIdleState = false;

		returnToIdleState();
		return true;
	}

	if (_animPlayWhileIdle) {
		bool animEnded = false;
		continuePlayingAnimation(true, animEnded);
	}

	if (_debugMode)
		drawDebugOverlay();

	OSEvent osEvent;
	while (popOSEvent(osEvent)) {
		if (osEvent.type == kOSEventTypeMouseMove)
			dischargeIdleMouseMove();
	}

	return false;
}

bool Runtime::runWaitForAnimation() {
	bool animEnded = false;
	continuePlayingAnimation(false, animEnded);

	if (animEnded) {
		_gameState = kGameStateScript;
		return true;
	}

	// Still waiting, check events
	OSEvent evt;
	while (popOSEvent(evt)) {
		if (evt.type == kOSEventTypeKeyDown && evt.keyCode == Common::KEYCODE_ESCAPE) {
			if (_escOn) {
				// Terminate the animation
				if (_animDecoderState == kAnimDecoderStatePlaying) {
					_animDecoder->pauseVideo(true);
					_animDecoderState = kAnimDecoderStatePaused;
				}
				_gameState = kGameStateScript;
				return true;
			}
		}
	}

	// Yield
	return false;
}

void Runtime::continuePlayingAnimation(bool loop, bool &outAnimationEnded) {
	outAnimationEnded = false;

	if (!_animDecoder) {
		outAnimationEnded = true;
		return;
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

		// We check this here for timing reasons: The no-loop case after the draw terminates the animation as soon as the last frame
		// starts delaying without waiting for the time until the next frame to expire.
		// The loop check here DOES wait for the time until next frame to expire.
		if (loop && _animPendingDecodeFrame > _animLastFrame) {
			if (!_animDecoder->seekToFrame(_animFirstFrame)) {
				outAnimationEnded = true;
				return;
			}

			_animPendingDecodeFrame = _animFirstFrame;
		}

		const Graphics::Surface *surface = _animDecoder->decodeNextFrame();
		if (!surface) {
			outAnimationEnded = true;
			return;
		}

		_animDisplayingFrame = _animPendingDecodeFrame;
		_animPendingDecodeFrame++;

		Common::Rect copyRect = Common::Rect(0, 0, surface->w, surface->h);

		Common::Rect constraintRect = Common::Rect(0, 0, _gameSection.rect.width(), _gameSection.rect.height());

		copyRect = copyRect.findIntersectingRect(constraintRect);

		if (copyRect.isValidRect() || !copyRect.isEmpty()) {
			_gameSection.surf->blitFrom(*surface, copyRect, copyRect);
			drawSectionToScreen(_gameSection, copyRect);
		}

		if (!loop) {
			if (_animDisplayingFrame >= _animLastFrame) {
				_animDecoder->pauseVideo(true);
				_animDecoderState = kAnimDecoderStatePaused;

				outAnimationEnded = true;
				return;
			}
		}
	}
}

void Runtime::drawSectionToScreen(const RenderSection &section, const Common::Rect &rect) {
	if (_debugMode && (&_gameSection == &section)) {
		_gameDebugBackBuffer.surf->blitFrom(*section.surf, rect, rect);
		commitSectionToScreen(_gameDebugBackBuffer, rect);
	} else
		commitSectionToScreen(section, rect);
}

void Runtime::commitSectionToScreen(const RenderSection &section, const Common::Rect &rect) {
	_system->copyRectToScreen(section.surf->getBasePtr(rect.left, rect.top), section.surf->pitch, rect.left + section.rect.left, rect.top + section.rect.top, rect.width(), rect.height());
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

bool Runtime::popOSEvent(OSEvent &evt) {
	OSEvent tempEvent;

	while (_pendingEvents.size() > 0) {
		tempEvent = _pendingEvents[0];
		_pendingEvents.remove_at(0);

		// Button events automatically inject a mouse move position
		if (tempEvent.type == kOSEventTypeMouseMove) {
			// If this didn't actually change the mouse position, which is common with synthetic mouse events,
			// discard the event.
			if (_mousePos == tempEvent.pos)
				continue;

			if (_lmbDown && tempEvent.pos != _lmbDownPos)
				_lmbDragging = true;

			_mousePos = tempEvent.pos;
		} else if (tempEvent.type == kOSEventTypeLButtonDown) {
			// Discard LButtonDown events missing a matching release (can happen if e.g. user holds button down
			// and then alt-tabs out of the process on Windows)
			if (_lmbDown)
				continue;

			_lmbDown = true;
			_lmbDragging = false;
			_lmbDownTime = tempEvent.timestamp;
			_lmbDownPos = tempEvent.pos;
		} else if (tempEvent.type == kOSEventTypeLButtonUp) {
			// Discard LButtonUp events missing a matching down
			if (!_lmbDown)
				continue;

			_lmbDown = false;
			// Don't reset _lmbDragging - Want that available to determine if this was a drag release or click
		}

		evt = tempEvent;
		return true;
	}

	return false;
}

void Runtime::queueOSEvent(const OSEvent &evt) {
	OSEvent timedEvt = evt;
	timedEvt.timestamp = g_system->getMillis();

	_pendingEvents.push_back(timedEvt);
}

void Runtime::loadIndex() {
	const char *indexPath = "Log/Index.txt";

	Common::INIFile iniFile;
	iniFile.allowNonEnglishCharacters();
	if (!iniFile.loadFromFile(indexPath))
		error("Failed to load main logic index");

	IndexParseType indexParseType = kIndexParseTypeNone;

	static const IndexPrefixTypePair parsePrefixes[] = {
		{"Room", kIndexParseTypeRoom},
		{"RRoom", kIndexParseTypeRRoom},
		{"YRoom", kIndexParseTypeYRoom},
		{"VRoom", kIndexParseTypeVRoom},
		{"TRoom", kIndexParseTypeTRoom},
		{"CRoom", kIndexParseTypeCRoom},
		{"SRoom", kIndexParseTypeSRoom},
	};

	for (const Common::INIFile::Section &section : iniFile.getSections()) {
		uint roomNumber = 0;

		if (section.name == "NameRoom") {
			indexParseType = kIndexParseTypeNameRoom;
		} else {
			bool foundType = false;
			uint prefixLen = 0;
			for (const IndexPrefixTypePair &prefixTypePair : parsePrefixes) {
				if (section.name.hasPrefix(prefixTypePair.prefix)) {
					indexParseType = prefixTypePair.parseType;
					foundType = true;
					prefixLen = strlen(prefixTypePair.prefix);
					break;
				}
			}

			if (!foundType)
				error("Unknown index heading type %s", section.name.c_str());


			if (!sscanf(section.name.c_str() + prefixLen, "%u", &roomNumber))
				error("Malformed room def '%s'", section.name.c_str());

			allocateRoomsUpTo(roomNumber);
		}

		for (const Common::INIFile::KeyValue &keyValue : section.getKeys()) {
			if (!parseIndexDef(indexParseType, roomNumber, keyValue.key, keyValue.value))
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

		Common::String logicFileName = Common::String::format("Log/Room%02i.log", static_cast<int>(roomNumber));
		Common::File logicFile;
		if (logicFile.open(logicFileName)) {
			_scriptSet = compileLogicFile(logicFile, static_cast<uint>(logicFile.size()), logicFileName);
			logicFile.close();
		}

		_map.clear();

		Common::String mapFileName = Common::String::format("Map/Room%02i.map", static_cast<int>(roomNumber));
		Common::File mapFile;

		if (mapFile.open(mapFileName)) {
			loadMap(&mapFile);
			mapFile.close();
		}
	}

	if (changedScreen) {
		if (_scriptSet) {
			RoomScriptSetMap_t::const_iterator roomScriptIt = _scriptSet->roomScripts.find(_roomNumber);
			if (roomScriptIt != _scriptSet->roomScripts.end()) {
				const ScreenScriptSetMap_t &screenScriptsMap = roomScriptIt->_value->screenScripts;
				ScreenScriptSetMap_t::const_iterator screenScriptIt = screenScriptsMap.find(_screenNumber);
				if (screenScriptIt != screenScriptsMap.end()) {
					const Common::SharedPtr<Script> &script = screenScriptIt->_value->entryScript;
					if (script)
						activateScript(script, ScriptEnvironmentVars());
				}
			}
		}

		_havePanAnimations = false;

		for (uint i = 0; i < kNumDirections; i++)
			_haveIdleAnimations[i] = false;

		_havePendingReturnToIdleState = true;
	}
}

void Runtime::returnToIdleState() {
	_animPlayWhileIdle = false;

	if (_haveIdleAnimations[_direction]) {
		changeAnimation(_idleAnimations[_direction]);
		_animPlayWhileIdle = true;
	}

	_idleIsOnInteraction = false;

	detectPanoramaDirections();

	changeToCursor(_cursors[kCursorArrow]);
	dischargeIdleMouseMove();
}

void Runtime::changeToCursor(const Common::SharedPtr<Graphics::WinCursorGroup> &cursor) {
	if (!cursor)
		CursorMan.showMouse(false);
	else {
		CursorMan.replaceCursor(cursor->cursors[0].cursor);
		CursorMan.showMouse(true);
	}
}

void Runtime::dischargeIdleMouseMove() {
	const MapScreenDirectionDef *sdDef = _map.getScreenDirection(_screenNumber, _direction);

	Common::Point relMouse(_mousePos.x - _gameSection.rect.left, _mousePos.y - _gameSection.rect.top);

	bool isOnInteraction = false;
	uint interactionID = 0;
	if (sdDef) {
		for (const InteractionDef &idef : sdDef->interactions) {
			if (idef.rect.contains(relMouse)) {
				isOnInteraction = true;
				interactionID = idef.interactionID;
				break;
			}
		}
	}

	if (isOnInteraction && (_idleIsOnInteraction == false || interactionID != _idleInteractionID)) {
		_idleIsOnInteraction = true;
		_idleInteractionID = interactionID;

		// New interaction, is there a script?
		Common::SharedPtr<Script> script = findScriptForInteraction(interactionID);

		if (script)
			activateScript(script, ScriptEnvironmentVars());
	} else if (!isOnInteraction && _idleIsOnInteraction) {
		_idleIsOnInteraction = false;
		changeToCursor(_cursors[kCursorArrow]);
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

	Common::String wavFileName = Common::String::format("Sfx/Music-%02i.wav", static_cast<int>(track));
	Common::File *wavFile = new Common::File();
	if (wavFile->open(wavFileName)) {
		if (Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(wavFile, DisposeAfterUse::YES)) {
			Common::SharedPtr<Audio::AudioStream> loopingStream(Audio::makeLoopingAudioStream(audioStream, 0));

			_musicPlayer.reset(new AudioPlayer(_mixer, loopingStream, 255, 0));
		}
	} else {
		warning("Music file '%s' is missing", wavFileName.c_str());
		delete wavFile;
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

		Common::String aviFileName = Common::String::format("Anims/Anim%04i.avi", animFile);
		Common::File *aviFile = new Common::File();

		if (aviFile->open(aviFileName)) {
			_animDecoder.reset(new Video::AVIDecoder());
			if (!_animDecoder->loadStream(aviFile)) {
				warning("Animation file %i could not be loaded", animFile);
				return;
			}
		} else {
			warning("Animation file %i is missing", animFile);
			delete aviFile;
		}
	}

	if (_animDecoderState == kAnimDecoderStatePlaying) {
		_animDecoder->pauseVideo(true);
		_animDecoderState = kAnimDecoderStatePaused;
	}

	_animDecoder->seekToFrame(animDef.firstFrame);
	_animPendingDecodeFrame = animDef.firstFrame;
	_animFirstFrame = animDef.firstFrame;
	_animLastFrame = animDef.lastFrame;
}

AnimationDef Runtime::stackArgsToAnimDef(const StackValue_t *args) const {
	AnimationDef def;
	def.animNum = args[0];
	def.firstFrame = args[1];
	def.lastFrame = args[2];

	return def;
}

void Runtime::activateScript(const Common::SharedPtr<Script> &script, const ScriptEnvironmentVars &envVars) {
	if (script->instrs.size() == 0)
		return;

	_scriptEnv = envVars;
	_activeScript = script;
	_scriptNextInstruction = 0;
	_gameState = kGameStateScript;
}

bool Runtime::parseIndexDef(IndexParseType parseType, uint roomNumber, const Common::String &key, const Common::String &value) {
	switch (parseType) {
	case kIndexParseTypeNameRoom: {
		uint nameRoomNumber = 0;

		if (!sscanf(value.c_str(), "%u", &nameRoomNumber))
			error("Malformed NameRoom def '%s'", value.c_str());

		allocateRoomsUpTo(nameRoomNumber);
		_roomDefs[nameRoomNumber]->name = key;
	} break;
	case kIndexParseTypeRoom: {
		int animNum = 0;
		uint firstFrame = 0;
		uint lastFrame = 0;
		if (sscanf(value.c_str(), "%i, %u, %u", &animNum, &firstFrame, &lastFrame) != 3)
			error("Malformed room animation def '%s'", value.c_str());

		
		AnimationDef animDef;
		animDef.animNum = animNum;
		animDef.firstFrame = firstFrame;
		animDef.lastFrame = lastFrame;

		_roomDefs[roomNumber]->animations[key] = animDef;
	} break;
	case kIndexParseTypeRRoom: {
		Common::String name;


		int left = 0;
		int top = 0;
		int width = 0;
		int height = 0;

		int numValuesRead = sscanf(value.c_str(), "%i, %i, %i, %i", &left, &top, &width, &height);

		if (numValuesRead == 4) {
			_roomDefs[roomNumber]->rects[key] = Common::Rect(left, top, left + width, top + height);
		} else {
			// Line 4210 in Reah contains an animation def instead of a rect def, so we need to tolerate invalid values here
			warning("Invalid rect def in logic index '%s'", value.c_str());
		}
	} break;
	case kIndexParseTypeYRoom: {
		uint varSlot = 0;

		if (!sscanf(value.c_str(), "%u", &varSlot))
			error("Malformed var def '%s'", value.c_str());
			
		_roomDefs[roomNumber]->vars[key] = varSlot;
	} break;
	case kIndexParseTypeVRoom: {
		Common::String name;

		int val = 0;

		if (!sscanf(value.c_str(), "%i", &val))
			error("Malformed value def '%s'", value.c_str());

		_roomDefs[roomNumber]->values[key] = val;
	} break;
	case kIndexParseTypeTRoom: {
		_roomDefs[roomNumber]->texts[key] = value;
	} break;
	case kIndexParseTypeCRoom: {
		// This is only used for one entry ("PrzedDrzwiamiDoZsypu" = "In front of the door to the chute") in Reah
		// and doesn't seem to be referenced in any scripts or anything else.  Discard it.
	} break;
	case kIndexParseTypeSRoom: {
		Common::String name;

		int soundID = 0;

		if (!sscanf(value.c_str(), "%i", &soundID))
			error("Malformed sound def '%s'", value.c_str());

		_roomDefs[roomNumber]->values[key] = soundID;
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

void Runtime::drawDebugOverlay() {
	if (!_debugMode)
		return;

	const Graphics::PixelFormat pixFmt = _gameDebugBackBuffer.surf->format;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);

	uint32 whiteColor = pixFmt.ARGBToColor(255, 255, 255, 255);
	uint32 blackColor = pixFmt.ARGBToColor(255, 0, 0, 0);

	const MapScreenDirectionDef *sdDef = _map.getScreenDirection(_screenNumber, _direction);
	if (sdDef) {
		for (const InteractionDef &idef : sdDef->interactions) {
			Common::Rect rect = idef.rect;

			Common::String label = Common::String::format("0%x", static_cast<int>(idef.interactionID));

			Graphics::ManagedSurface *surf = _gameDebugBackBuffer.surf.get();

			if (font) {
				Common::Point pt = Common::Point(rect.left + 2, rect.top + 2);

				font->drawString(surf, label, pt.x + 1, pt.y + 1, rect.width(), blackColor);
				font->drawString(surf, label, pt.x, pt.y, rect.width(), whiteColor);
			}

			surf->frameRect(Common::Rect(rect.left + 1, rect.top + 1, rect.right + 1, rect.bottom + 1), blackColor);
			surf->frameRect(rect, whiteColor);
		}
	}

	commitSectionToScreen(_gameDebugBackBuffer, Common::Rect(0, 0, _gameDebugBackBuffer.rect.width(), _gameDebugBackBuffer.rect.height()));
}

Common::SharedPtr<Script> Runtime::findScriptForInteraction(uint interactionID) const {
	if (_scriptSet) {
		RoomScriptSetMap_t::const_iterator roomScriptIt = _scriptSet->roomScripts.find(_roomNumber);

		if (roomScriptIt != _scriptSet->roomScripts.end()) {
			const RoomScriptSet &roomScriptSet = *roomScriptIt->_value;

			ScreenScriptSetMap_t::const_iterator screenScriptIt = roomScriptSet.screenScripts.find(_screenNumber);
			if (screenScriptIt != roomScriptSet.screenScripts.end()) {
				const ScreenScriptSet &screenScriptSet = *screenScriptIt->_value;

				ScriptMap_t::const_iterator interactionScriptIt = screenScriptSet.interactionScripts.find(interactionID);
				if (interactionScriptIt != screenScriptSet.interactionScripts.end())
					return interactionScriptIt->_value;
			}
		}
	}

	return nullptr;
}

void Runtime::detectPanoramaDirections() {
	_panoramaDirectionFlags = 0;

	if (_havePanAnimations)
		_panoramaDirectionFlags |= kPanoramaHorizFlags;
}

void Runtime::onLButtonDown(int16 x, int16 y) {
	onMouseMove(x, y);

	OSEvent evt;
	evt.type = kOSEventTypeLButtonDown;
	evt.pos = Common::Point(x, y);

	queueOSEvent(evt);
}

void Runtime::onLButtonUp(int16 x, int16 y) {
	onMouseMove(x, y);

	OSEvent evt;
	evt.type = kOSEventTypeLButtonUp;
	evt.pos = Common::Point(x, y);

	queueOSEvent(evt);
}

void Runtime::onMouseMove(int16 x, int16 y) {
	OSEvent evt;
	evt.type = kOSEventTypeMouseMove;
	evt.pos = Common::Point(x, y);

	queueOSEvent(evt);
}

void Runtime::onKeyDown(Common::KeyCode keyCode) {
	OSEvent evt;
	evt.type = kOSEventTypeKeyDown;
	evt.keyCode = keyCode;

	queueOSEvent(evt);
}

#ifdef PEEK_STACK
#error "PEEK_STACK is already defined"
#endif

#ifdef TAKE_STACK
#error "TAKE_STACK is already defined"
#endif

#ifdef OPCODE_STUB
#error "OPCODE_STUB is already defined"
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

#define OPCODE_STUB(op)                           \
	void Runtime::scriptOp##op(ScriptArg_t arg) { \
		error("Unimplemented opcode '" #op "'");  \
	}

void Runtime::scriptOpNumber(ScriptArg_t arg) {
	_scriptStack.push_back(arg);
}

void Runtime::scriptOpRotate(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + kAnimDefStackArgs);

	_panLeftAnimationDef = stackArgsToAnimDef(stackArgs + 0);
	_panRightAnimationDef = stackArgsToAnimDef(stackArgs + kAnimDefStackArgs);
	_havePanAnimations = true;
}

OPCODE_STUB(Angle)
OPCODE_STUB(AngleGGet)
OPCODE_STUB(Speed)

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

OPCODE_STUB(ChangeL)

OPCODE_STUB(AnimR)
OPCODE_STUB(AnimF)
OPCODE_STUB(AnimN)
OPCODE_STUB(AnimG)

void Runtime::scriptOpAnimS(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + 2);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);
	animDef.lastFrame = animDef.firstFrame;	// Static animation

	changeAnimation(animDef);

	_gameState = kGameStateWaitingForAnimation;
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

OPCODE_STUB(Static)

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

void Runtime::scriptOpSetCursor(ScriptArg_t arg) {
	TAKE_STACK(1);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= _cursors.size())
		error("Invalid cursor ID");

	changeToCursor(_cursors[stackArgs[0]]);
}

OPCODE_STUB(SetRoom)

void Runtime::scriptOpLMB(ScriptArg_t arg) {
	if (!_scriptEnv.lmb)
		terminateScript();
}

OPCODE_STUB(LMB1)
OPCODE_STUB(SoundS1)

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

	warning("Music volume ramp up is not implemented");
	(void)stackArgs;
}

void Runtime::scriptOpMusicDn(ScriptArg_t arg) {
	TAKE_STACK(2);

	warning("Music volume ramp down is not implemented");
	(void)stackArgs;
}


OPCODE_STUB(Parm1)
OPCODE_STUB(Parm2)
OPCODE_STUB(Parm3)
OPCODE_STUB(ParmG)

OPCODE_STUB(VolumeDn4)
OPCODE_STUB(VolumeUp3)
OPCODE_STUB(Random)

void Runtime::scriptOpDrop(ScriptArg_t arg) {
	TAKE_STACK(1);
	(void)stackArgs;
}

void Runtime::scriptOpDup(ScriptArg_t arg) {
	TAKE_STACK(1);

	_scriptStack.push_back(stackArgs[0]);
	_scriptStack.push_back(stackArgs[0]);
}

OPCODE_STUB(Say3)
OPCODE_STUB(SetTimer)
OPCODE_STUB(LoSet)
OPCODE_STUB(LoGet)
OPCODE_STUB(HiSet)
OPCODE_STUB(HiGet)

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

OPCODE_STUB(BitLoad)
OPCODE_STUB(BitSet0)
OPCODE_STUB(BitSet1)

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

OPCODE_STUB(EscGet)
OPCODE_STUB(BackStart)

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


OPCODE_STUB(ValueName)

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

void Runtime::scriptOpCursorName(ScriptArg_t arg) {
	const Common::String &cursorName = _scriptSet->strings[arg];

	Common::HashMap<Common::String, StackValue_t>::const_iterator namedCursorIt = _namedCursors.find(cursorName);
	if (namedCursorIt == _namedCursors.end()) {
		error("Unimplemented cursor name '%s'", cursorName.c_str());
		return;
	}

	_scriptStack.push_back(namedCursorIt->_value);
}

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

#undef TAKE_STACK
#undef PEEK_STACK
#undef OPCODE_STUB


void Runtime::drawFrame() {
	_system->updateScreen();
}


} // End of namespace VCruise
