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
#include "common/endian.h"
#include "common/file.h"
#include "common/math.h"
#include "common/ptr.h"
#include "common/random.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/translation.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/wincursor.h"
#include "graphics/managed_surface.h"

#include "image/bmp.h"

#include "audio/decoders/wave.h"
#include "audio/audiostream.h"

#include "video/avi_decoder.h"

#include "gui/message.h"

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

ScriptEnvironmentVars::ScriptEnvironmentVars() : lmb(false), lmbDrag(false), panInteractionID(0), fpsOverride(0), lastHighlightedItem(0) {
}

void Runtime::RenderSection::init(const Common::Rect &paramRect, const Graphics::PixelFormat &fmt) {
	rect = paramRect;
	surf.reset(new Graphics::ManagedSurface(paramRect.width(), paramRect.height(), fmt));
	surf->fillRect(Common::Rect(0, 0, surf->w, surf->h), 0xffffffff);
}

Runtime::OSEvent::OSEvent() : type(kOSEventTypeInvalid), keyCode(static_cast<Common::KeyCode>(0)) {
}

Runtime::Gyro::Gyro() {
	reset();
}

void Runtime::Gyro::reset() {
	currentState = 0;
	requiredState = 0;
	wrapAround = false;
	requireState = false;
	numPreviousStates = 0;
	numPreviousStatesRequired = 0;

	for (uint i = 0; i < kMaxPreviousStates; i++) {
		previousStates[i] = 0;
		requiredPreviousStates[i] = 0;
	}
}

void Runtime::Gyro::logState() {
	if (numPreviousStatesRequired > 0) {
		if (numPreviousStates < numPreviousStatesRequired)
			numPreviousStates++;
		else {
			for (uint i = 1; i < numPreviousStates; i++)
				previousStates[i - 1] = previousStates[i];
		}

		previousStates[numPreviousStates - 1] = currentState;
	}
}

Runtime::GyroState::GyroState() {
	reset();
}

void Runtime::GyroState::reset() {
	for (uint i = 0; i < kNumGyros; i++)
		gyros[i].reset();

	completeInteraction = 0;
	failureInteraction = 0;
	frameSeparation = 1;

	activeGyro = 0;
	dragMargin = 0;
	maxValue = 0;

	negAnim = AnimationDef();
	posAnim = AnimationDef();
	isVertical = false;

	dragBasePoint = Common::Point(0, 0);
	dragBaseState = 0;
	dragCurrentState = 0;
	isWaitingForAnimation = false;
}


SfxPlaylistEntry::SfxPlaylistEntry() : frame(0), balance(0), volume(0) {
}

SfxPlaylist::SfxPlaylist() {
}

SfxData::SfxData() {
}

void SfxData::reset() {
	playlists.clear();
	sounds.clear();
}

void SfxData::load(Common::SeekableReadStream &stream, Audio::Mixer *mixer) {
	Common::INIFile iniFile;

	iniFile.allowNonEnglishCharacters();
	if (!iniFile.loadFromStream(stream))
		warning("SfxData::load failed to parse INI file");

	const Common::INIFile::Section *samplesSection = nullptr;
	const Common::INIFile::Section *playlistsSection = nullptr;

	Common::INIFile::SectionList sections = iniFile.getSections();	// Why does this require a copy??

	for (const Common::INIFile::Section &section : sections) {
		if (section.name == "samples")
			samplesSection = &section;
		else if (section.name == "playlists")
			playlistsSection = &section;
	}

	if (samplesSection) {
		for (const Common::INIFile::KeyValue &keyValue : samplesSection->keys) {
			Common::SharedPtr<SfxSound> sample(new SfxSound());

			// Fix up the path delimiter
			Common::String sfxPath = keyValue.value;
			for (char &c : sfxPath) {
				if (c == '\\')
					c = '/';
			}

			sfxPath = Common::String("Sfx/") + sfxPath;

			Common::File f;
			if (!f.open(sfxPath))
				warning("SfxData::load: Could not open sample file '%s'", sfxPath.c_str());

			int64 size = f.size();
			if (size <= 0 || size > 0x1fffffffu) {
				warning("SfxData::load: File is oversized for some reason");
				continue;
			}

			sample->soundData.resize(static_cast<uint>(size));
			if (f.read(&sample->soundData[0], static_cast<uint32>(size)) != size) {
				warning("SfxData::load: Couldn't read file");
				continue;
			}

			sample->memoryStream.reset(new Common::MemoryReadStream(&sample->soundData[0], static_cast<uint32>(size)));
			sample->audioStream.reset(Audio::makeWAVStream(sample->memoryStream.get(), DisposeAfterUse::NO));
			sample->audioPlayer.reset(new AudioPlayer(mixer, sample->audioStream));

			this->sounds[keyValue.key] = sample;
		}
	}

	if (playlistsSection) {
		Common::SharedPtr<SfxPlaylist> playlist;

		for (const Common::INIFile::KeyValue &keyValue : playlistsSection->keys) {
			const Common::String &key = keyValue.key;

			if (key.size() == 0)
				continue;

			if (key.size() >= 2 && key.firstChar() == '\"' && key.lastChar() == '\"') {
				if (!playlist) {
					warning("Found playlist entry outside of a playlist");
					continue;
				}

				Common::String workKey = key.substr(1, key.size() - 2);

				Common::Array<Common::String> tokens;
				for (;;) {
					uint32 spaceSpanStart = workKey.find(' ');

					if (spaceSpanStart == Common::String::npos) {
						tokens.push_back(workKey);
						break;
					}

					uint32 spaceSpanEnd = spaceSpanStart;

					while (spaceSpanEnd < workKey.size() && workKey[spaceSpanEnd] == ' ')
						spaceSpanEnd++;

					tokens.push_back(workKey.substr(0, spaceSpanStart));
					workKey = workKey.substr(spaceSpanEnd, workKey.size() - spaceSpanEnd);
				}

				// Strip leading and trailing spaces
				while (tokens.size() > 0) {
					if (tokens[0].empty()) {
						tokens.remove_at(0);
						continue;
					}

					uint lastIndex = tokens.size() - 1;
					if (tokens[lastIndex].empty()) {
						tokens.remove_at(lastIndex);
						continue;
					}

					break;
				}

				if (tokens.size() != 4) {
					warning("Found unusual playlist entry: %s", key.c_str());
					continue;
				}

				unsigned int frameNum = 0;
				int balance = 0;
				unsigned int volume = 0;

				if (!sscanf(tokens[0].c_str(), "%u", &frameNum) || !sscanf(tokens[2].c_str(), "%i", &balance) || !sscanf(tokens[3].c_str(), "%u", &volume)) {
					warning("Malformed playlist entry: %s", key.c_str());
					continue;
				}

				SoundMap_t::const_iterator soundIt = this->sounds.find(tokens[1]);
				if (soundIt == this->sounds.end()) {
					warning("Playlist entry referenced non-existent sound: %s", tokens[1].c_str());
					continue;
				}

				SfxPlaylistEntry plEntry;
				plEntry.balance = balance;
				plEntry.frame = frameNum;
				plEntry.volume = volume;
				plEntry.sample = soundIt->_value;

				playlist->entries.push_back(plEntry);
			} else {
				playlist.reset(new SfxPlaylist());
				this->playlists[key] = playlist;
			}
		}
	}
}

CachedSound::CachedSound()
	: rampStartVolume(0), rampEndVolume(0), rampRatePerMSec(0), rampStartTime(0), rampTerminateOnCompletion(false),
	  volume(0), balance(0), effectiveBalance(0), effectiveVolume(0), is3D(false), x(0), y(0), z(0) {
}

CachedSound::~CachedSound() {
	// Dispose player first so playback stops
	this->player.reset();

	// Dispose loopingStream before stream because stream is not refcounted by loopingStream so we need to avoid late free
	this->loopingStream.reset();

	this->stream.reset();
}

TriggeredOneShot::TriggeredOneShot() : soundID(0), uniqueSlot(0) {
}

bool TriggeredOneShot::operator==(const TriggeredOneShot &other) const {
	return soundID == other.soundID && uniqueSlot == other.uniqueSlot;
}

bool TriggeredOneShot::operator!=(const TriggeredOneShot &other) const {
	return !((*this) == other);
}

StaticAnimParams::StaticAnimParams() : initialDelay(0), repeatDelay(0), lockInteractions(false) {
}

StaticAnimation::StaticAnimation() : currentAlternation(0), nextStartTime(0) {
}

FrameData::FrameData() : areaID{0, 0, 0, 0}, areaFrameIndex(0), frameIndex(0), frameType(0), roomNumber(0) {
}

FrameData2::FrameData2() : x(0), y(0), angle(0), frameNumberInArea(0), unknown(0) {
}

SoundParams3D::SoundParams3D() : minRange(0), maxRange(0), unknownRange(0) {
}

InventoryItem::InventoryItem() : itemID(0), highlighted(false) {
}

Runtime::Runtime(OSystem *system, Audio::Mixer *mixer, const Common::FSNode &rootFSNode, VCruiseGameID gameID)
	: _system(system), _mixer(mixer), _roomNumber(1), _screenNumber(0), _direction(0), _havePanAnimations(0), _loadedRoomNumber(0), _activeScreenNumber(0),
	  _gameState(kGameStateBoot), _gameID(gameID), _havePendingScreenChange(false), _forceScreenChange(false), _havePendingReturnToIdleState(false), _havePendingCompletionCheck(false),
	  _scriptNextInstruction(0), _escOn(false), _debugMode(false), _panoramaDirectionFlags(0),
	  _loadedAnimation(0), _animPendingDecodeFrame(0), _animDisplayingFrame(0), _animFirstFrame(0), _animLastFrame(0), _animStopFrame(0),
	  _animFrameRateLock(0), _animStartTime(0), _animFramesDecoded(0), _animDecoderState(kAnimDecoderStateStopped),
	  _animPlayWhileIdle(false), _idleIsOnInteraction(false), _idleHaveClickInteraction(false), _idleHaveDragInteraction(false), _idleInteractionID(0), _haveIdleStaticAnimation(false),
	  _loadedArea(0), _lmbDown(false), _lmbDragging(false), _lmbReleaseWasClick(false), _lmbDownTime(0),
	  _panoramaState(kPanoramaStateInactive),
	  _listenerX(0), _listenerY(0), _listenerAngle(0) {

	for (uint i = 0; i < kNumDirections; i++) {
		_haveIdleAnimations[i] = false;
		_havePanUpFromDirection[i] = false;
		_havePanDownFromDirection[i] = false;
	}

	for (uint i = 0; i < kPanCursorMaxCount; i++)
		_panCursors[i] = 0;

	_rng.reset(new Common::RandomSource("vcruise"));
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
		// For some reason most cursors map to their resource IDs, except for these
		_scriptCursorIDToResourceIDOverride[13] = 35;	// Points to finger (instead of back up)
		_scriptCursorIDToResourceIDOverride[22] = 13;	// Points to back up (instead of up arrow)

		_namedCursors["CUR_TYL"] = 22;		// Tyl = back
		//_namedCursors["CUR_NIC"] = ?		// Nic = nothing
		_namedCursors["CUR_WEZ"] = 90;		// Wez = call?  This is the pick-up hand.
		_namedCursors["CUR_LUPA"] = 21;		// Lupa = magnifier, could be 36 too?
		_namedCursors["CUR_NAC"] = 13;		// Nac = top?  Not sure.  But this is the finger pointer.
		_namedCursors["CUR_PRZOD"] = 1;		// Przod = forward

		// CUR_ZOSTAW is in the executable memory but appears to be unused

		_panCursors[kPanCursorDraggableHoriz | kPanCursorDraggableUp] = 2;
		_panCursors[kPanCursorDraggableHoriz | kPanCursorDraggableDown] = 3;
		_panCursors[kPanCursorDraggableHoriz] = 4;
		_panCursors[kPanCursorDraggableHoriz | kPanCursorDirectionRight] = 5;
		_panCursors[kPanCursorDraggableHoriz | kPanCursorDirectionLeft] = 6;
		_panCursors[kPanCursorDraggableUp] = 7;
		_panCursors[kPanCursorDraggableDown] = 8;
		_panCursors[kPanCursorDraggableUp | kPanCursorDirectionUp] = 9;
		_panCursors[kPanCursorDraggableDown | kPanCursorDirectionDown] = 10;
		_panCursors[kPanCursorDraggableUp | kPanCursorDraggableDown] = 11;
		_panCursors[kPanCursorDraggableHoriz | kPanCursorDraggableUp | kPanCursorDraggableDown] = 12;
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
			moreActions = bootGame(true);
			break;
		case kGameStateQuit:
			return false;
		case kGameStateIdle:
			moreActions = runIdle();
			break;
		case kGameStatePanLeft:
			moreActions = runHorizontalPan(false);
			break;
		case kGameStatePanRight:
			moreActions = runHorizontalPan(true);
			break;
		case kGameStateScript:
			moreActions = runScript();
			break;
		case kGameStateWaitingForAnimation:
			moreActions = runWaitForAnimation();
			break;
		case kGameStateWaitingForFacing:
			moreActions = runWaitForFacing();
			break;
		case kGameStateWaitingForFacingToAnim:
			moreActions = runWaitForFacingToAnim();
			break;
		case kGameStateGyroIdle:
			moreActions = runGyroIdle();
			break;
		case kGameStateGyroAnimation:
			moreActions = runGyroAnimation();
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

	uint32 timestamp = g_system->getMillis();

	updateSounds(timestamp);

	return true;
}

bool Runtime::bootGame(bool newGame) {
	assert(_gameState == kGameStateBoot);

	debug(1, "Booting V-Cruise game...");
	loadIndex();
	debug(1, "Index loaded OK");
	findWaves();
	debug(1, "Waves indexed OK");

	_trayBackgroundGraphic = loadGraphic("Pocket", true);
	_trayHighlightGraphic = loadGraphic("Select", true);
	_trayCompassGraphic = loadGraphic("Select_1", true);
	_trayCornerGraphic = loadGraphic("Select_2", true);

	_gameState = kGameStateIdle;

	if (newGame) {
		if (_gameID == GID_REAH) {
			// TODO: Change to the logo instead (0xb1) instead when menus are implemented
			changeToScreen(1, 0xb0);
		} else
			error("Couldn't figure out what screen to start on");
	}

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

	uint32 timestamp = g_system->getMillis();

	if (_animPlayWhileIdle) {
		assert(_haveIdleAnimations[_direction]);

		StaticAnimation &sanim = _idleAnimations[_direction];
		bool looping = (sanim.params.repeatDelay == 0);

		bool animEnded = false;
		continuePlayingAnimation(looping, false, animEnded);

		if (!looping && animEnded) {
			_animPlayWhileIdle = false;
			sanim.nextStartTime = timestamp + sanim.params.repeatDelay * 1000u;
			sanim.currentAlternation = 1 - sanim.currentAlternation;
		}
	} else if (_haveIdleAnimations[_direction]) {
		// Try to re-trigger
		StaticAnimation &sanim = _idleAnimations[_direction];
		if (sanim.nextStartTime <= timestamp) {
			changeAnimation(sanim.animDefs[sanim.currentAlternation], false);
			_animPlayWhileIdle = true;
		}
	}

	if (_debugMode)
		drawDebugOverlay();

	detectPanoramaMouseMovement(timestamp);

	OSEvent osEvent;
	while (popOSEvent(osEvent)) {
		if (osEvent.type == kOSEventTypeMouseMove) {
			detectPanoramaMouseMovement(osEvent.timestamp);

			bool changedState = dischargeIdleMouseMove();
			if (changedState)
				return true;

		} else if (osEvent.type == kOSEventTypeLButtonUp) {
			PanoramaState oldPanoramaState = _panoramaState;
			_panoramaState = kPanoramaStateInactive;

			// This is the correct place for matching the original game's behavior, not switching to panorama
			resetInventoryHighlights();

			if (_lmbReleaseWasClick) {
				bool changedState = dischargeIdleClick();
				if (changedState)
					return true;
			}

			// If the released from panorama mode, pick up any interactions at the new mouse location, and change the mouse back
			if (oldPanoramaState != kPanoramaStateInactive) {
				changeToCursor(_cursors[kCursorArrow]);

				// Clear idle interaction so that if a drag occurs but doesn't trigger a panorama or other state change,
				// interactions are re-detected here.
				_idleIsOnInteraction = false;

				bool changedState = dischargeIdleMouseMove();
				if (changedState)
					return true;
			}
		} else if (osEvent.type == kOSEventTypeLButtonDown) {
			bool changedState = dischargeIdleMouseDown();
			if (changedState)
				return true;
		}
	}

	// Yield
	return false;
}

bool Runtime::runHorizontalPan(bool isRight) {
	bool animEnded = false;
	continuePlayingAnimation(true, false, animEnded);

	Common::Point panRelMouse = _mousePos - _panoramaAnchor;

	if (!_lmbDown) {
		debug(1, "Terminating pan: LMB is not down");
		startTerminatingHorizontalPan(isRight);
		return true;
	}

	if (!isRight && panRelMouse.x > -kPanoramaPanningMarginX) {
		debug(1, "Terminating pan: Over threshold for left movement");
		startTerminatingHorizontalPan(isRight);
		return true;
	} else if (isRight && panRelMouse.x < kPanoramaPanningMarginX) {
		debug(1, "Terminating pan: Over threshold for right movement");
		startTerminatingHorizontalPan(isRight);
		return true;
	}

	OSEvent osEvent;
	while (popOSEvent(osEvent)) {
		if (osEvent.type == kOSEventTypeLButtonUp) {
			debug(1, "Terminating pan: LMB up");
			startTerminatingHorizontalPan(isRight);
			return true;
		} else if (osEvent.type == kOSEventTypeMouseMove) {
			panRelMouse = osEvent.pos - _panoramaAnchor;

			if (!isRight && panRelMouse.x > -kPanoramaPanningMarginX) {
				debug(1, "Terminating pan: Over threshold for left movement (from queue)");
				startTerminatingHorizontalPan(isRight);
				return true;
			} else if (isRight && panRelMouse.x < kPanoramaPanningMarginX) {
				debug(1, "Terminating pan: Over threshold for right movement (from queue)");
				startTerminatingHorizontalPan(isRight);
				return true;
			}
		}
	}

	// Didn't terminate, yield
	return false;
}

bool Runtime::runWaitForAnimation() {
	bool animEnded = false;
	continuePlayingAnimation(false, false, animEnded);

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

bool Runtime::runWaitForFacingToAnim() {
	bool animEnded = false;
	continuePlayingAnimation(true, true, animEnded);

	if (animEnded) {
		changeAnimation(_postFacingAnimDef, true);
		_gameState = kGameStateWaitingForAnimation;
		return true;
	}

	// Yield
	return false;
}

bool Runtime::runWaitForFacing() {
	bool animEnded = false;
	continuePlayingAnimation(true, true, animEnded);

	if (animEnded) {
		_gameState = kGameStateScript;
		return true;
	}

	// Yield
	return false;
}

bool Runtime::runGyroIdle() {
	if (!_lmbDown) {
		exitGyroIdle();
		return true;
	}

	int32 deltaCoordinate = 0;

	if (_gyros.isVertical)
		deltaCoordinate = _gyros.dragBasePoint.y - _mousePos.y;
	else
		deltaCoordinate = _gyros.dragBasePoint.x - _mousePos.x;


	// Start the first step at half margin
	int32 halfDragMargin = _gyros.dragMargin / 2;
	if (deltaCoordinate < 0)
		deltaCoordinate -= halfDragMargin;
	else
		deltaCoordinate += halfDragMargin;

	int32 deltaState = deltaCoordinate / static_cast<int32>(_gyros.dragMargin);
	int32 targetStateInitial = static_cast<int32>(_gyros.dragBaseState) + deltaState;

	Gyro &gyro = _gyros.gyros[_gyros.activeGyro];

	int32 targetState = 0;
	if (gyro.wrapAround) {
		targetState = targetStateInitial;
	} else {
		if (targetStateInitial > 0) {
			targetState = targetStateInitial;
			if (static_cast<uint>(targetState) > _gyros.maxValue)
				targetState = _gyros.maxValue;
		}
	}

	if (targetState < _gyros.dragCurrentState) {
		AnimationDef animDef = _gyros.negAnim;

		uint initialFrame = 0;

		if (gyro.wrapAround) {
			uint maxValuePlusOne = _gyros.maxValue + 1;
			initialFrame = animDef.firstFrame + ((maxValuePlusOne - gyro.currentState) % maxValuePlusOne * _gyros.frameSeparation);
		} else
			initialFrame = animDef.firstFrame + ((_gyros.maxValue - gyro.currentState) * _gyros.frameSeparation);

		// This is intentional instead of setting the stop frame, V-Cruise can overrun the end of the animation.
		// firstFrame is left alone so playlists are based correctly.
		animDef.lastFrame = initialFrame + _gyros.frameSeparation;

		changeAnimation(animDef, initialFrame, false);

		gyro.logState();
		gyro.currentState--;
		_gyros.dragCurrentState--;

		if (gyro.currentState < 0)
			gyro.currentState = _gyros.maxValue;

		_gameState = kGameStateGyroAnimation;
		_havePendingCompletionCheck = true;
		return true;
	} else if (targetState > _gyros.dragCurrentState) {
		AnimationDef animDef = _gyros.posAnim;

		uint initialFrame = animDef.firstFrame + gyro.currentState * _gyros.frameSeparation;

		// This is intentional instead of setting the stop frame, V-Cruise can overrun the end of the animation.
		// firstFrame is left alone so playlists are based correctly.
		animDef.lastFrame = initialFrame + _gyros.frameSeparation;

		changeAnimation(animDef, initialFrame, false);

		gyro.logState();
		gyro.currentState++;
		_gyros.dragCurrentState++;

		if (static_cast<uint>(gyro.currentState) > _gyros.maxValue)
			gyro.currentState = 0;

		_gameState = kGameStateGyroAnimation;
		_havePendingCompletionCheck = true;
		return true;
	}

	OSEvent evt;
	while (popOSEvent(evt)) {
		if (evt.type == kOSEventTypeLButtonUp) {
			exitGyroIdle();
			return true;
		}
	}

	// Yield
	return false;
}

bool Runtime::runGyroAnimation() {
	bool animEnded = false;
	continuePlayingAnimation(false, false, animEnded);

	if (animEnded) {
		_gameState = kGameStateGyroIdle;
		return true;
	}

	// Yield
	return false;
}

void Runtime::exitGyroIdle() {
	_gameState = kGameStateScript;
	_havePendingReturnToIdleState = true;

	// In Reah, gyro interactions stop the script.
	if (_gameID == GID_REAH)
		terminateScript();
}

void Runtime::continuePlayingAnimation(bool loop, bool useStopFrame, bool &outAnimationEnded) {
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

	uint32 millis = 0;

	// Avoid spamming event recorder as much if we don't actually need to fetch millis, but also only fetch it once here.
	if (_animFrameRateLock)
		millis = g_system->getMillis();

	for (;;) {
		bool needNewFrame = false;

		if (needsFirstFrame) {
			needNewFrame = true;
			needsFirstFrame = false;
		} else {
			if (_animFrameRateLock) {
				// if ((millis - startTime) / 1000 * frameRate) >= framesDecoded
				if ((millis - _animStartTime) * static_cast<uint64>(_animFrameRateLock) >= (static_cast<uint64>(_animFramesDecoded) * 1000u))
					needNewFrame = true;
				debug("FPS lock: New frame at %u millis and %u decoded? %s", static_cast<uint>(millis - _animStartTime), static_cast<uint>(_animFramesDecoded), needNewFrame ? "yes" : "no");
			} else {
				if (_animDecoder->getTimeToNextFrame() == 0)
					needNewFrame = true;
			}
		}

		if (!needNewFrame)
			break;

		// We check this here for timing reasons: The no-loop case after the draw terminates the animation as soon as the last frame
		// starts delaying without waiting for the time until the next frame to expire.
		// The loop check here DOES wait for the time until next frame to expire.
		if (loop && _animPendingDecodeFrame > _animLastFrame) {
			debug(4, "Looped animation at frame %u", _animLastFrame);
			if (!_animDecoder->seekToFrame(_animFirstFrame)) {
				outAnimationEnded = true;
				return;
			}

			_animPendingDecodeFrame = _animFirstFrame;
		}

		if (useStopFrame && _animPendingDecodeFrame == _animStopFrame) {
			outAnimationEnded = true;
			return;
		}

		debug(4, "Decoding animation frame %u", _animPendingDecodeFrame);

		const Graphics::Surface *surface = _animDecoder->decodeNextFrame();
		if (!surface) {
			outAnimationEnded = true;
			return;
		}

		_animDisplayingFrame = _animPendingDecodeFrame;
		_animPendingDecodeFrame++;
		_animFramesDecoded++;

		if (_animDisplayingFrame < _frameData2.size()) {
			const FrameData2 &fd2 = _frameData2[_animDisplayingFrame];

			_listenerX = fd2.x;
			_listenerY = fd2.y;
			_listenerAngle = fd2.angle;
		}

		update3DSounds();

		if (_animPlaylist) {
			uint decodeFrameInPlaylist = _animDisplayingFrame - _animFirstFrame;
			for (const SfxPlaylistEntry &playlistEntry : _animPlaylist->entries) {
				if (playlistEntry.frame == decodeFrameInPlaylist) {
					VCruise::AudioPlayer &audioPlayer = *playlistEntry.sample->audioPlayer;

					audioPlayer.stop();
					playlistEntry.sample->audioStream->seek(0);
					audioPlayer.play(playlistEntry.volume, playlistEntry.frame);

					// No break, it's possible for there to be multiple sounds in the same frame
				}
			}
		}

		Common::Rect copyRect = Common::Rect(0, 0, surface->w, surface->h);

		if (!_animConstraintRect.isEmpty())
			copyRect = copyRect.findIntersectingRect(_animConstraintRect);

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
			DISPATCH_OP(ItemCheck);
			DISPATCH_OP(ItemRemove);
			DISPATCH_OP(ItemHighlightSet);
			DISPATCH_OP(ItemAdd);
			DISPATCH_OP(ItemHaveSpace);
			DISPATCH_OP(SetCursor);
			DISPATCH_OP(SetRoom);
			DISPATCH_OP(LMB);
			DISPATCH_OP(LMB1);
			DISPATCH_OP(SoundS1);
			DISPATCH_OP(SoundS2);
			DISPATCH_OP(SoundS3);
			DISPATCH_OP(SoundL1);
			DISPATCH_OP(SoundL2);
			DISPATCH_OP(SoundL3);
			DISPATCH_OP(3DSoundS2);
			DISPATCH_OP(3DSoundL2);
			DISPATCH_OP(StopAL);
			DISPATCH_OP(Range);
			DISPATCH_OP(AddXSound);
			DISPATCH_OP(ClrXSound);
			DISPATCH_OP(StopSndLA);
			DISPATCH_OP(StopSndLO);

			DISPATCH_OP(Music);
			DISPATCH_OP(MusicUp);
			DISPATCH_OP(MusicDn);
			DISPATCH_OP(Parm0);
			DISPATCH_OP(Parm1);
			DISPATCH_OP(Parm2);
			DISPATCH_OP(Parm3);
			DISPATCH_OP(ParmG);
			DISPATCH_OP(SParmX);
			DISPATCH_OP(SAnimX);

			DISPATCH_OP(VolumeDn4);
			DISPATCH_OP(VolumeUp3);
			DISPATCH_OP(Random);
			DISPATCH_OP(Drop);
			DISPATCH_OP(Dup);
			DISPATCH_OP(Say1);
			DISPATCH_OP(Say3);
			DISPATCH_OP(Say3Get);
			DISPATCH_OP(SetTimer);
			DISPATCH_OP(GetTimer);
			DISPATCH_OP(Delay);
			DISPATCH_OP(LoSet);
			DISPATCH_OP(LoGet);
			DISPATCH_OP(HiSet);
			DISPATCH_OP(HiGet);

			DISPATCH_OP(Not);
			DISPATCH_OP(And);
			DISPATCH_OP(Or);
			DISPATCH_OP(Add);
			DISPATCH_OP(Sub);
			DISPATCH_OP(Negate);
			DISPATCH_OP(CmpEq);
			DISPATCH_OP(CmpGt);
			DISPATCH_OP(CmpLt);

			DISPATCH_OP(BitLoad);
			DISPATCH_OP(BitSet0);
			DISPATCH_OP(BitSet1);

			DISPATCH_OP(Disc1);
			DISPATCH_OP(Disc2);
			DISPATCH_OP(Disc3);

			DISPATCH_OP(Goto);

			DISPATCH_OP(EscOn);
			DISPATCH_OP(EscOff);
			DISPATCH_OP(EscGet);
			DISPATCH_OP(BackStart);
			DISPATCH_OP(SaveAs);

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

	if (_havePendingCompletionCheck) {
		_havePendingCompletionCheck = false;

		if (checkCompletionConditions())
			return;
	}

	if (_havePendingScreenChange)
		changeToScreen(_roomNumber, _screenNumber);
}

bool Runtime::checkCompletionConditions() {
	bool succeeded = true;
	for (uint i = 0; i < GyroState::kNumGyros; i++) {
		const Gyro &gyro = _gyros.gyros[i];
		if (gyro.requireState && gyro.currentState != gyro.requiredState) {
			succeeded = false;
			break;
		}

		if (gyro.numPreviousStates != gyro.numPreviousStatesRequired) {
			succeeded = false;
			break;
		}

		bool prevStatesMatch = true;
		for (uint j = 0; j < gyro.numPreviousStates; j++) {
			if (gyro.previousStates[j] != gyro.requiredPreviousStates[j]) {
				prevStatesMatch = false;
				break;
			}
		}

		if (!prevStatesMatch) {
			succeeded = false;
			break;
		}
	}

	// Activate the corresponding failure or success interaction if present
	if (_scriptSet) {
		RoomScriptSetMap_t::const_iterator roomScriptIt = _scriptSet->roomScripts.find(_roomNumber);
		if (roomScriptIt != _scriptSet->roomScripts.end()) {
			const ScreenScriptSetMap_t &screenScriptsMap = roomScriptIt->_value->screenScripts;
			ScreenScriptSetMap_t::const_iterator screenScriptIt = screenScriptsMap.find(_screenNumber);
			if (screenScriptIt != screenScriptsMap.end()) {
				const ScreenScriptSet &screenScriptSet = *screenScriptIt->_value;

				ScriptMap_t::const_iterator interactionScriptIt = screenScriptSet.interactionScripts.find(succeeded ? _gyros.completeInteraction : _gyros.failureInteraction);
				if (interactionScriptIt != screenScriptSet.interactionScripts.end()) {
					const Common::SharedPtr<Script> &script = interactionScriptIt->_value;
					if (script) {
						activateScript(script, ScriptEnvironmentVars());
						return true;
					}
				}
			}
		}
	}

	return false;
}

void Runtime::startTerminatingHorizontalPan(bool isRight) {
	// Figure out what slice this is.  The last frame is 1 less than usual.
	uint slice = (_animDisplayingFrame - _animFirstFrame) * kNumDirections / (_animLastFrame - _animFirstFrame + 1);

	// Compute an end frame at the end of the slice.
	_animLastFrame = (slice + 1) * (_animLastFrame - _animFirstFrame + 1) / kNumDirections + _animFirstFrame;

	debug(1, "Terminating pan at frame slice %u -> frame %u", slice, _animLastFrame);

	if (isRight)
		_direction = (slice + 1) % kNumDirections;
	else
		_direction = kNumDirections - 1 - slice;

	_gameState = kGameStateWaitingForAnimation;
	_panoramaState = kPanoramaStateInactive;

	// Need to return to idle after direction change
	_havePendingReturnToIdleState = true;
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

			_lmbReleaseWasClick = !_lmbDragging;
			_lmbDown = false;
			_lmbDragging = false;
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

void Runtime::findWaves() {
	Common::ArchiveMemberList waves;
	SearchMan.listMatchingMembers(waves, "Sfx/Waves-##/####*.wav", true);

	for (const Common::ArchiveMemberPtr &wave : waves) {
		Common::String name = wave->getName();

		// Strip .wav extension
		name = name.substr(0, name.size() - 4);

		// Make case-insensitive
		name.toLowercase();

		_waves[name] = wave;
	}
}

void Runtime::loadWave(uint soundID, const Common::String &soundName, const Common::ArchiveMemberPtr &archiveMemberPtr) {
	Common::SeekableReadStream *stream = archiveMemberPtr->createReadStream();
	if (!stream) {
		warning("Couldn't open read stream for sound '%s'", soundName.c_str());
		return;
	}

	Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Couldn't open audio stream for sound '%s'", soundName.c_str());
		return;
	}

	Common::SharedPtr<CachedSound> cachedSound(new CachedSound());
	_cachedSounds[soundID] = cachedSound;

	cachedSound->stream.reset(audioStream);
	cachedSound->name = soundName;
}

void Runtime::changeToScreen(uint roomNumber, uint screenNumber) {
	bool changedRoom = (roomNumber != _loadedRoomNumber) || _forceScreenChange;
	bool changedScreen = (screenNumber != _activeScreenNumber) || changedRoom;

	_forceScreenChange = false;

	_roomNumber = roomNumber;
	_screenNumber = screenNumber;

	_loadedRoomNumber = roomNumber;
	_activeScreenNumber = screenNumber;

	if (changedRoom) {
		// This shouldn't happen when running a script
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
		_gyros.reset();

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
		for (uint i = 0; i < kNumDirections; i++) {
			_havePanUpFromDirection[i] = false;
			_havePanDownFromDirection[i] = false;
		}

		for (uint i = 0; i < kNumDirections; i++)
			_haveIdleAnimations[i] = false;

		_havePendingReturnToIdleState = true;
		_haveIdleStaticAnimation = false;
	}
}

void Runtime::returnToIdleState() {
	debug(1, "Returned to idle state in room %u screen 0%x facing direction %u", _roomNumber, _screenNumber, _direction);

	uint32 timestamp = g_system->getMillis();

	_animPlayWhileIdle = false;

	if (_haveIdleAnimations[_direction]) {
		StaticAnimation &sanim = _idleAnimations[_direction];
		sanim.currentAlternation = 0;
		sanim.nextStartTime = timestamp + sanim.params.initialDelay * 1000u;

		if (sanim.params.initialDelay == 0) {
			changeAnimation(sanim.animDefs[0], false);
			_animPlayWhileIdle = true;
			sanim.currentAlternation = 1;
		}
	}

	_idleIsOnInteraction = false;
	_idleHaveClickInteraction = false;
	_idleHaveDragInteraction = false;

	// Do this before detectPanoramaMouseMovement so continuous panorama keeps the correct cursor
	changeToCursor(_cursors[kCursorArrow]);

	detectPanoramaDirections();

	_panoramaState = kPanoramaStateInactive;
	detectPanoramaMouseMovement(g_system->getMillis());

	(void) dischargeIdleMouseMove();
}

void Runtime::changeToCursor(const Common::SharedPtr<Graphics::WinCursorGroup> &cursor) {
	if (!cursor)
		CursorMan.showMouse(false);
	else {
		CursorMan.replaceCursor(cursor->cursors[0].cursor);
		CursorMan.showMouse(true);
	}
}

bool Runtime::dischargeIdleMouseMove() {
	const MapScreenDirectionDef *sdDef = _map.getScreenDirection(_screenNumber, _direction);

	if (_panoramaState == kPanoramaStateInactive) {
		Common::Point relMouse(_mousePos.x - _gameSection.rect.left, _mousePos.y - _gameSection.rect.top);

		bool isOnInteraction = false;
		uint interactionID = 0;
		if (sdDef) {
			for (const InteractionDef &idef : sdDef->interactions) {
				if (idef.objectType == 1 && idef.rect.contains(relMouse)) {
					isOnInteraction = true;
					interactionID = idef.interactionID;
					break;
				}
			}
		}

		if (_idleIsOnInteraction && (!isOnInteraction || interactionID != _idleInteractionID)) {
			// Mouse left the previous interaction
			_idleIsOnInteraction = false;
			_idleHaveClickInteraction = false;
			_idleHaveDragInteraction = false;
			changeToCursor(_cursors[kCursorArrow]);
			resetInventoryHighlights();
		}

		if (isOnInteraction && _idleIsOnInteraction == false) {
			_idleIsOnInteraction = true;
			_idleInteractionID = interactionID;

			// New interaction, is there a script?
			Common::SharedPtr<Script> script = findScriptForInteraction(interactionID);

			if (script) {
				activateScript(script, ScriptEnvironmentVars());
				return true;
			}
		}
	} else {
		uint interactionID = 0;

		Common::Point panRelMouse = _mousePos - _panoramaAnchor;
		if (_havePanAnimations) {
			if (panRelMouse.x <= -kPanoramaPanningMarginX)
				interactionID = kPanLeftInteraction;
			else if (panRelMouse.x >= kPanoramaPanningMarginX)
				interactionID = kPanRightInteraction;
			else if (panRelMouse.y <= -kPanoramaPanningMarginY)
				interactionID = kPanUpInteraction;
			else if (panRelMouse.y >= kPanoramaPanningMarginY)
				interactionID = kPanDownInteraction;

			if (interactionID) {
				// If there's an interaction script for this direction, execute it
				Common::SharedPtr<Script> script = findScriptForInteraction(interactionID);

				if (script) {
					resetInventoryHighlights();

					ScriptEnvironmentVars vars;
					vars.panInteractionID = interactionID;
					activateScript(script, vars);
					return true;
				}
			}
		}
	}

	// Didn't do anything
	return false;
}

bool Runtime::dischargeIdleMouseDown() {
	if (_idleIsOnInteraction && _idleHaveDragInteraction) {
		// Interaction, is there a script?
		Common::SharedPtr<Script> script = findScriptForInteraction(_idleInteractionID);

		_idleIsOnInteraction = false; // ?
		resetInventoryHighlights();

		if (script) {
			ScriptEnvironmentVars vars;
			vars.lmbDrag = true;

			activateScript(script, vars);
			return true;
		}
	}

	// Didn't do anything
	return false;
}

bool Runtime::dischargeIdleClick() {
	if (_idleIsOnInteraction && _idleHaveClickInteraction) {
		// Interaction, is there a script?
		Common::SharedPtr<Script> script = findScriptForInteraction(_idleInteractionID);

		_idleIsOnInteraction = false;	// ?

		if (script) {
			ScriptEnvironmentVars vars;
			vars.lmb = true;

			activateScript(script, vars);
			return true;
		}
	}

	// Didn't do anything
	return false;
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

				// QUIRK: The stone game in the tower in Reah (Room 06) has two 0cb screens and the second one is damaged,
				// so it must be ignored.
				if (!_map.screenDirections[screen][direction])
					_map.screenDirections[screen][direction] = screenDirectionDef;
			}
		}
	}
}

void Runtime::loadFrameData(Common::SeekableReadStream *stream) {
	int64 size = stream->size();
	if (size < 2048 || size > 0xffffffu)
		error("Unexpected DTA size");

	uint numFrameDatas = (static_cast<uint>(size) - 2048u) / 16;

	if (!stream->seek(2048))
		error("Error skipping DTA header");

	_frameData.resize(numFrameDatas);

	for (uint i = 0; i < numFrameDatas; i++) {
		byte frameData[16];

		if (stream->read(frameData, 16) != 16)
			error("Error reading DTA frame data");

		FrameData &fd = _frameData[i];
		fd.frameType = frameData[0];
		fd.frameIndex = frameData[1] | (frameData[2] << 8) | (frameData[3] << 16);
		fd.roomNumber = static_cast<int8>(frameData[4]);
		memcpy(fd.areaID, frameData + 8, 4);

		char decAreaFrameIndex[4];
		memcpy(decAreaFrameIndex, frameData + 12, 4);

		uint areaFrameIndex = 0;
		for (int digit = 0; digit < 4; digit++) {
			char c = decAreaFrameIndex[digit];
			if (c < '0' || c > '9')
				error("Invalid area frame index in DTA data");

			areaFrameIndex = areaFrameIndex * 10u + static_cast<uint>(c - '0');
		}

		fd.areaFrameIndex = areaFrameIndex;

		if (i != fd.frameIndex)
			error("DTA frame index was out-of-line, don't know how to handle this");
	}
}

void Runtime::loadFrameData2(Common::SeekableReadStream *stream) {
	int64 size = stream->size();
	if (size > 0xffffffu)
		error("Unexpected 2DT size");

	uint numFrameDatas = static_cast<uint>(size) / 16;

	if (numFrameDatas == 0)
		return;

	_frameData2.resize(numFrameDatas);

	uint32 numBytesToRead = numFrameDatas * 16;
	if (stream->read(&_frameData2[0], numBytesToRead) != numBytesToRead)
		error("Failed to read 2DT data");

	for (uint i = 0; i < numFrameDatas; i++) {
		FrameData2 &fd2 = _frameData2[i];
		fd2.x = READ_LE_INT32(&fd2.x);
		fd2.y = READ_LE_INT32(&fd2.y);
		fd2.angle = READ_LE_INT32(&fd2.angle);
		fd2.frameNumberInArea = READ_LE_UINT16(&fd2.frameNumberInArea);
		fd2.unknown = READ_LE_UINT16(&fd2.unknown);
	}
}

void Runtime::changeMusicTrack(int track) {
	_musicPlayer.reset();

	Common::String wavFileName = Common::String::format("Sfx/Music-%02i.wav", static_cast<int>(track));
	Common::File *wavFile = new Common::File();
	if (wavFile->open(wavFileName)) {
		if (Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(wavFile, DisposeAfterUse::YES)) {
			Common::SharedPtr<Audio::AudioStream> loopingStream(Audio::makeLoopingAudioStream(audioStream, 0));

			_musicPlayer.reset(new AudioPlayer(_mixer, loopingStream));
			_musicPlayer->play(100, 0);
		}
	} else {
		warning("Music file '%s' is missing", wavFileName.c_str());
		delete wavFile;
	}
}

void Runtime::changeAnimation(const AnimationDef &animDef, bool consumeFPSOverride) {
	changeAnimation(animDef, animDef.firstFrame, consumeFPSOverride);
}

void Runtime::changeAnimation(const AnimationDef &animDef, uint initialFrame, bool consumeFPSOverride) {
	debug("changeAnimation: %u -> %u  Initial %u", animDef.firstFrame, animDef.lastFrame, initialFrame);

	_animPlaylist.reset();

	int animFile = animDef.animNum;
	if (animFile < 0)
		animFile = -animFile;

	if (_loadedAnimation != static_cast<uint>(animFile)) {
		_loadedAnimation = animFile;
		_frameData.clear();
		_frameData2.clear();
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

		Common::String sfxFileName = Common::String::format("Sfx/Anim%04i.sfx", animFile);
		Common::File sfxFile;

		_sfxData.reset();

		if (sfxFile.open(sfxFileName))
			_sfxData.load(sfxFile, _mixer);
		sfxFile.close();

		Common::String dtaFileName = Common::String::format("Anims/Anim%04i.dta", animFile);
		Common::File dtaFile;

		if (dtaFile.open(dtaFileName))
			loadFrameData(&dtaFile);
		dtaFile.close();

		Common::String twoDtFileName = Common::String::format("Dta/Anim%04i.2dt", animFile);
		Common::File twoDtFile;

		if (twoDtFile.open(twoDtFileName))
			loadFrameData2(&twoDtFile);
		twoDtFile.close();
	}

	if (_animDecoderState == kAnimDecoderStatePlaying) {
		_animDecoder->pauseVideo(true);
		_animDecoderState = kAnimDecoderStatePaused;
	}

	assert(initialFrame >= animDef.firstFrame && initialFrame <= animDef.lastFrame);

	_animDecoder->seekToFrame(initialFrame);
	_animPendingDecodeFrame = initialFrame;
	_animFirstFrame = animDef.firstFrame;
	_animLastFrame = animDef.lastFrame;
	_animConstraintRect = animDef.constraintRect;
	_animFrameRateLock = 0;

	SfxData::PlaylistMap_t::const_iterator playlistIt = _sfxData.playlists.find(animDef.animName);

	if (playlistIt != _sfxData.playlists.end())
		_animPlaylist = playlistIt->_value;

	if (consumeFPSOverride) {
		_animFrameRateLock = _scriptEnv.fpsOverride;
		_animFramesDecoded = 0;
		_animStartTime = g_system->getMillis();
		_scriptEnv.fpsOverride = 0;
	}

	debug(1, "Animation last frame set to %u", animDef.lastFrame);
}

void Runtime::setSound3DParameters(uint soundID, int32 x, int32 y, const SoundParams3D &soundParams3D) {
	Common::HashMap<uint, Common::SharedPtr<CachedSound> >::iterator it = _cachedSounds.find(soundID);
	if (it == _cachedSounds.end()) {
		warning("Couldn't set sound parameters for sound ID %u, the sound wasn't loaded", soundID);
		return;
	}

	CachedSound &snd = *it->_value;
	snd.x = x;
	snd.y = y;
	snd.params3D = soundParams3D;
}

void Runtime::triggerSound(bool looping, uint soundID, uint volume, int32 balance, bool is3D) {
	Common::HashMap<uint, Common::SharedPtr<CachedSound> >::iterator it = _cachedSounds.find(soundID);
	if (it == _cachedSounds.end()) {
		warning("Couldn't trigger sound ID %u, the sound wasn't loaded", soundID);
		return;
	}

	CachedSound &snd = *it->_value;

	snd.volume = volume;
	snd.balance = balance;
	snd.is3D = is3D;

	computeEffectiveVolumeAndBalance(snd);

	// Reset if looping state changes
	if (snd.loopingStream && !looping) {
		snd.player.reset();
		snd.loopingStream.reset();
		snd.stream->rewind();
	}

	if (!snd.loopingStream && looping)
		snd.player.reset();

	// Construct looping stream if needed and none exists
	if (looping && !snd.loopingStream)
		snd.loopingStream.reset(new Audio::LoopingAudioStream(snd.stream.get(), 0, DisposeAfterUse::NO, true));

	if (snd.player) {
		// If there is already a player and this is non-looping, start over
		if (!looping) {
			snd.player->stop();
			snd.stream->rewind();
			snd.player->play(snd.effectiveVolume, snd.effectiveBalance);
		} else {
			// Adjust volume and balance at least
			snd.player->setVolumeAndBalance(snd.effectiveVolume, snd.effectiveBalance);
		}
	} else {
		snd.player.reset(new AudioPlayer(_mixer, looping ? snd.loopingStream.staticCast<Audio::AudioStream>() : snd.stream.staticCast<Audio::AudioStream>()));
		snd.player->play(snd.effectiveVolume, snd.effectiveBalance);
	}
}

void Runtime::triggerSoundRamp(uint soundID, uint durationMSec, uint newVolume, bool terminateOnCompletion) {
	Common::HashMap<uint, Common::SharedPtr<CachedSound> >::const_iterator it = _cachedSounds.find(soundID);
	if (it == _cachedSounds.end())
		return;

	CachedSound &snd = *it->_value;

	snd.rampStartVolume = snd.volume;
	snd.rampEndVolume = newVolume;
	snd.rampTerminateOnCompletion = terminateOnCompletion;
	snd.rampStartTime = g_system->getMillis();
	snd.rampRatePerMSec = 65536;

	if (durationMSec)
		snd.rampRatePerMSec = 65536 / durationMSec;
}

void Runtime::updateSounds(uint32 timestamp) {
	Common::Array<uint> condemnedSounds;

	for (const Common::HashMap<uint, Common::SharedPtr<CachedSound> >::Node &node : _cachedSounds) {
		CachedSound &snd = *node._value;

		if (snd.rampRatePerMSec) {
			uint ramp = snd.rampRatePerMSec * (timestamp - snd.rampStartTime);
			uint newVolume = snd.volume;
			if (ramp >= 65536) {
				snd.rampRatePerMSec = 0;
				newVolume = snd.rampEndVolume;
				if (snd.rampTerminateOnCompletion)
					condemnedSounds.push_back(node._key);
			} else {
				uint rampedVolume = (snd.rampStartVolume * (65536u - ramp)) + (snd.rampEndVolume * ramp);
				newVolume = rampedVolume >> 16;
			}

			if (snd.volume != newVolume) {
				snd.volume = newVolume;
				if (snd.player)
					snd.player->setVolume(snd.volume);
			}
		}
	}

	for (uint id : condemnedSounds)
		_cachedSounds.erase(id);
}

void Runtime::update3DSounds() {
	for (const Common::HashMap<uint, Common::SharedPtr<CachedSound> >::Node &node : _cachedSounds) {
		CachedSound &snd = *node._value;

		if (!snd.is3D)
			continue;

		bool changed = computeEffectiveVolumeAndBalance(snd);

		if (changed) {
			VCruise::AudioPlayer *player = snd.player.get();
			if (player)
				player->setVolumeAndBalance(snd.effectiveVolume, snd.effectiveBalance);
		}
	}
}

bool Runtime::computeEffectiveVolumeAndBalance(CachedSound &snd) {
	uint effectiveVolume = snd.volume;
	int32 effectiveBalance = snd.balance;

	double radians = Common::deg2rad<double>(_listenerAngle);
	int32 cosAngle = static_cast<int32>(cos(radians) * (1 << 15));
	int32 sinAngle = static_cast<int32>(sin(radians) * (1 << 15));

	if (snd.is3D) {
		int32 dx = snd.x - _listenerX;
		int32 dy = snd.y - _listenerY;

		uint distance = static_cast<uint>(sqrt(dx * dx + dy * dy));

		if (distance >= snd.params3D.maxRange)
			effectiveVolume = 0;
		else if (distance > snd.params3D.minRange) {
			uint rangeDelta = snd.params3D.maxRange - snd.params3D.minRange;

			effectiveVolume = (snd.params3D.maxRange - distance) * effectiveVolume / rangeDelta;
		}

		int32 dxNormalized = 0;
		int32 dyNormalized = 0;
		if (distance > 0) {
			dxNormalized = dx * (1 << 10) / static_cast<int32>(distance);
			dyNormalized = dy * (1 << 10) / static_cast<int32>(distance);
		}

		int32 balance16 = (sinAngle * dxNormalized - cosAngle * dyNormalized) >> 9;

		// Reduce to 3/5 intensity.  This means that at full balance, the opposing volume will be 1/4 the facing volume.
		balance16 = (balance16 * 9830 + (1 << 13)) >> 14;

		if (balance16 > 65536)
			balance16 = 65536;
		else if (balance16 < -65536)
			balance16 = -65536;

		uint rightVolume = ((65536u + balance16) * effectiveVolume) >> 16;
		uint leftVolume = ((65536u - balance16) * effectiveVolume) >> 16;

		if (leftVolume == 0 && rightVolume == 0) {
			// This should never happen
			effectiveVolume = 0;
			effectiveBalance = 0;
		} else {
			if (leftVolume <= rightVolume) {
				effectiveVolume = rightVolume;
				effectiveBalance = 127 - (leftVolume * 127 / rightVolume);
			} else {
				effectiveVolume = leftVolume;
				effectiveBalance = (rightVolume * 127 / leftVolume) - 127;
			}
		}
	}

	bool changed = (effectiveVolume != snd.effectiveVolume || effectiveBalance != snd.effectiveBalance);

	snd.effectiveVolume = effectiveVolume;
	snd.effectiveBalance = effectiveBalance;

	return changed;
}

AnimationDef Runtime::stackArgsToAnimDef(const StackValue_t *args) const {
	AnimationDef def;
	def.animNum = args[0];
	def.firstFrame = args[1];
	def.lastFrame = args[2];

	def.constraintRect.left = args[3];
	def.constraintRect.top = args[4];
	def.constraintRect.right = args[5];
	def.constraintRect.bottom = args[6];

	def.animName = _animDefNames[args[7]];

	return def;
}

void Runtime::pushAnimDef(const AnimationDef &animDef) {
	_scriptStack.push_back(animDef.animNum);
	_scriptStack.push_back(animDef.firstFrame);
	_scriptStack.push_back(animDef.lastFrame);

	_scriptStack.push_back(animDef.constraintRect.left);
	_scriptStack.push_back(animDef.constraintRect.top);
	_scriptStack.push_back(animDef.constraintRect.right);
	_scriptStack.push_back(animDef.constraintRect.bottom);

	uint animNameIndex = 0;
	Common::HashMap<Common::String, uint>::const_iterator nameIt = _animDefNameToIndex.find(animDef.animName);
	if (nameIt == _animDefNameToIndex.end()) {
		animNameIndex = _animDefNames.size();
		_animDefNameToIndex[animDef.animName] = animNameIndex;
		_animDefNames.push_back(animDef.animName);
	} else
		animNameIndex = nameIt->_value;

	_scriptStack.push_back(animNameIndex);
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
		
		AnimationDef &animDef = _roomDefs[roomNumber]->animations[key];
		animDef.animNum = animNum;
		animDef.firstFrame = firstFrame;
		animDef.lastFrame = lastFrame;
		animDef.animName = key;
	} break;
	case kIndexParseTypeRRoom: {
		Common::String name;


		int left = 0;
		int top = 0;
		int width = 0;
		int height = 0;

		int numValuesRead = sscanf(value.c_str(), "%i, %i, %i, %i", &left, &top, &width, &height);

		if (numValuesRead == 4) {
			AnimationDef &animDef = _roomDefs[roomNumber]->animations[key];

			animDef.constraintRect = Common::Rect(left, top, left + width, top + height);
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

			Common::String label = Common::String::format("0%x %i", static_cast<int>(idef.interactionID), static_cast<int>(idef.objectType));

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

	if (_havePanDownFromDirection[_direction])
		_panoramaDirectionFlags |= kPanoramaDownFlag;

	if (_havePanUpFromDirection[_direction])
		_panoramaDirectionFlags |= kPanoramaUpFlag;
}

void Runtime::detectPanoramaMouseMovement(uint32 timestamp) {
	if (_panoramaState == kPanoramaStateInactive && (_lmbDragging || (_lmbDown && (timestamp - _lmbDownTime) >= 500)))
		panoramaActivate();
}

void Runtime::panoramaActivate() {
	assert(_panoramaState == kPanoramaStateInactive);
	_panoramaState = kPanoramaStatePanningUncertainDirection;
	_panoramaAnchor = _mousePos;

	uint cursorID = 0;
	if (_havePanAnimations) {
		uint panCursor = 0;
		if (_panoramaDirectionFlags & kPanoramaHorizFlags)
			panCursor |= kPanCursorDraggableHoriz;
		if (_panoramaDirectionFlags & kPanoramaUpFlag)
			panCursor |= kPanCursorDraggableUp;
		if (_panoramaDirectionFlags & kPanoramaDownFlag)
			panCursor |= kPanCursorDraggableDown;

		cursorID = _panCursors[panCursor];
	}

	debug(1, "Changing cursor to panorama cursor %u", cursorID);
	changeToCursor(_cursors[cursorID]);

	// We don't reset inventory highlights here.  It'd make sense, but doesn't match the original game's behavior.
	// Inventory highlights only reset from panoramas if a rotation occurs, or the mouse button is released.
}

bool Runtime::computeFaceDirectionAnimation(uint desiredDirection, const AnimationDef *&outAnimDef, uint &outInitialFrame, uint &outStopFrame) {
	if (_direction == desiredDirection)
		return false;

	uint leftPanDistance = ((_direction + kNumDirections) - desiredDirection) % kNumDirections;
	uint rightPanDistance = ((desiredDirection + kNumDirections) - _direction) % kNumDirections;

	if (rightPanDistance <= leftPanDistance) {
		uint currentSlice = _direction;
		uint desiredSlice = desiredDirection;

		outAnimDef = &_panRightAnimationDef;
		outInitialFrame = currentSlice * (_panRightAnimationDef.lastFrame - _panRightAnimationDef.firstFrame) / kNumDirections + _panRightAnimationDef.firstFrame;
		outStopFrame = desiredSlice * (_panRightAnimationDef.lastFrame - _panRightAnimationDef.firstFrame) / kNumDirections + _panRightAnimationDef.firstFrame;
	} else {
		uint reverseCurrentSlice = (kNumDirections - _direction);
		if (reverseCurrentSlice == kNumDirections)
			reverseCurrentSlice = 0;

		uint reverseDesiredSlice = (kNumDirections - desiredDirection);
		if (reverseDesiredSlice == kNumDirections)
			reverseDesiredSlice = 0;

		outAnimDef = &_panLeftAnimationDef;
		outInitialFrame = reverseCurrentSlice * (_panLeftAnimationDef.lastFrame - _panLeftAnimationDef.firstFrame) / kNumDirections + _panLeftAnimationDef.firstFrame;
		outStopFrame = reverseDesiredSlice * (_panLeftAnimationDef.lastFrame - _panLeftAnimationDef.firstFrame) / kNumDirections + _panLeftAnimationDef.firstFrame;
	}

	return true;
}

void Runtime::inventoryAddItem(uint item) {
	uint firstOpenSlot = kNumInventorySlots;

	for (uint i = 0; i < kNumInventorySlots; i++) {
		if (_inventory[i].itemID == item)
			return;
		if (_inventory[i].itemID == 0 && firstOpenSlot == kNumInventorySlots)
			firstOpenSlot = i;
	}

	if (firstOpenSlot == kNumInventorySlots)
		error("Tried to add an inventory item but ran out of slots");

	Common::String itemFileName = getFileNameForItemGraphic(item);

	_inventory[firstOpenSlot].itemID = item;
	_inventory[firstOpenSlot].graphic = loadGraphic(itemFileName, false);

	drawInventory(firstOpenSlot);
}

void Runtime::inventoryRemoveItem(uint itemID) {
	for (uint slot = 0; slot < kNumInventorySlots; slot++) {
		InventoryItem &item = _inventory[slot];

		if (item.itemID == static_cast<uint>(itemID)) {
			item.highlighted = false;
			item.itemID = 0;
			item.graphic.reset();
			drawInventory(slot);
		}
	}
}

void Runtime::drawInventory(uint slot) {
	Common::Rect trayRect = _traySection.rect;
	trayRect.translate(-trayRect.left, -trayRect.top);

	const uint slotWidth = 79;
	const uint firstItemX = 82;

	const uint slotStartX = firstItemX + slot * slotWidth;
	Common::Rect sliceRect = Common::Rect(slotStartX, 0, slotStartX + slotWidth, trayRect.height());

	const bool highlighted = _inventory[slot].highlighted;

	if (highlighted)
		_traySection.surf->blitFrom(*_trayHighlightGraphic, sliceRect, sliceRect);
	else
		_traySection.surf->fillRect(sliceRect, 0);

	const Graphics::Surface *surf = _inventory[slot].graphic.get();

	// TODO: Highlighted items
	if (surf) {
		const uint itemWidth = surf->w;
		const uint itemHeight = surf->h;

		const uint itemTopY = (static_cast<uint>(trayRect.height()) - itemHeight) / 2u;
		const uint itemLeftY = slotStartX + (slotWidth - itemWidth) / 2u;

		if (highlighted) {
			uint32 blackColor = surf->format.ARGBToColor(255, 0, 0, 0);
			_traySection.surf->transBlitFrom(*surf, Common::Point(itemLeftY, itemTopY), blackColor);
		} else
			_traySection.surf->blitFrom(*surf, Common::Point(itemLeftY, itemTopY));
	}

	commitSectionToScreen(_traySection, sliceRect);
}

void Runtime::resetInventoryHighlights() {
	for (uint slot = 0; slot < kNumInventorySlots; slot++) {
		InventoryItem &item = _inventory[slot];
		if (item.highlighted) {
			item.highlighted = false;
			drawInventory(slot);
		}
	}
}

Common::String Runtime::getFileNameForItemGraphic(uint itemID) const {
	if (_gameID == GID_REAH)
		return Common::String::format("Thing%u", itemID);
	else if (_gameID == GID_SCHIZM)
		return Common::String::format("Item%u", itemID);
	else {
		error("Unknown game, can't format inventory item");
		return "";
	}
}

Common::SharedPtr<Graphics::Surface> Runtime::loadGraphic(const Common::String &graphicName, bool required) {
	Common::String filePath = Common::String("Gfx/") + graphicName + ".bmp";

	Common::File f;
	if (!f.open(filePath)) {
		warning("Couldn't open BMP file '%s'", filePath.c_str());
		return nullptr;
	}

	Image::BitmapDecoder bmpDecoder;
	if (!bmpDecoder.loadStream(f)) {
		warning("Failed to load BMP file '%s'", filePath.c_str());
		return nullptr;
	}

	Common::SharedPtr<Graphics::Surface> surf(new Graphics::Surface());
	surf->copyFrom(*bmpDecoder.getSurface());

	return surf;
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

bool Runtime::canSave() const {
	return _gameState == kGameStateIdle;
}

bool Runtime::canLoad() const {
	return _gameState == kGameStateIdle;
}

void Runtime::saveGame(Common::WriteStream *stream) const {
	stream->writeUint32BE(kSaveGameIdentifier);
	stream->writeUint32BE(kSaveGameCurrentVersion);

	stream->writeUint32BE(_roomNumber);
	stream->writeUint32BE(_screenNumber);
	stream->writeUint32BE(_direction);

	Common::Array<uint32> variableIDs;
	Common::Array<uint> timerIDs;

	uint32 timeBase = g_system->getMillis();

	for (const Common::HashMap<uint32, int32>::Node &varNode : _variables)
		variableIDs.push_back(varNode._key);

	for (const Common::HashMap<uint, uint32>::Node &timerNode : _timers)
		timerIDs.push_back(timerNode._key);

	Common::sort(variableIDs.begin(), variableIDs.end());
	Common::sort(timerIDs.begin(), timerIDs.end());

	stream->writeUint32BE(variableIDs.size());
	stream->writeUint32BE(timerIDs.size());

	for (uint32 variableKey : variableIDs) {
		Common::HashMap<uint32, int32>::const_iterator it = _variables.find(variableKey);
		assert(it != _variables.end());

		stream->writeUint32BE(variableKey);
		stream->writeSint32BE(it->_value);
	}

	for (uint timerKey : timerIDs) {
		Common::HashMap<uint, uint32>::const_iterator it = _timers.find(timerKey);
		assert(it != _timers.end());

		stream->writeUint32BE(timerKey);
		stream->writeUint32BE(it->_value - timeBase);
	}

	for (const InventoryItem &item : _inventory)
		stream->writeUint32BE(item.itemID);
}

Runtime::LoadGameOutcome Runtime::loadGame(Common::ReadStream *stream) {
	assert(canLoad());

	uint32 saveGameID = stream->readUint32BE();
	uint32 saveVersion = stream->readUint32BE();

	if (stream->err() || stream->eos())
		return kLoadGameOutcomeMissingVersion;

	if (saveGameID != kSaveGameIdentifier)
		return kLoadGameOutcomeInvalidVersion;

	if (saveVersion > kSaveGameCurrentVersion)
		return kLoadGameOutcomeSaveIsTooNew;

	if (saveVersion < kSaveGameEarliestSupportedVersion)
		return kLoadGameOutcomeSaveIsTooOld;

	uint32 timeBase = g_system->getMillis();

	uint32 roomNumber = stream->readUint32BE();
	uint32 screenNumber = stream->readUint32BE();
	uint32 direction = stream->readUint32BE();

	uint32 numVars = stream->readUint32BE();
	uint32 numTimers = stream->readUint32BE();

	if (stream->err() || stream->eos())
		return kLoadGameOutcomeSaveDataCorrupted;

	Common::HashMap<uint32, int32> vars;
	Common::HashMap<uint, uint32> timers;

	for (uint32 i = 0; i < numVars; i++) {
		uint32 varID = stream->readUint32BE();
		int32 varValue = stream->readSint32BE();

		vars[varID] = varValue;
	}

	for (uint32 i = 0; i < numTimers; i++) {
		uint timerID = stream->readUint32BE();
		uint32 timerValue = stream->readUint32BE();

		timers[timerID] = timerValue + timeBase;
	}

	uint inventoryItems[kNumInventorySlots];

	for (uint i = 0; i < kNumInventorySlots; i++)
		inventoryItems[i] = stream->readUint32BE();

	if (stream->err() || stream->eos())
		return kLoadGameOutcomeSaveDataCorrupted;

	if (direction >= kNumDirections)
		return kLoadGameOutcomeSaveDataCorrupted;

	// Load succeeded
	_variables = Common::move(vars);
	_timers = Common::move(timers);

	for (uint i = 0; i < kNumInventorySlots; i++) {
		_inventory[i].itemID = inventoryItems[i];
		_inventory[i].highlighted = false;
		_inventory[i].graphic.reset();

		if (inventoryItems[i] != 0)
			_inventory[i].graphic = loadGraphic(getFileNameForItemGraphic(inventoryItems[i]), false);

		drawInventory(i);
	}

	_direction = direction;
	changeToScreen(roomNumber, screenNumber);
	_havePendingReturnToIdleState = true;

	return kLoadGameOutcomeSucceeded;
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

void Runtime::scriptOpAngle(ScriptArg_t arg) {
	TAKE_STACK(1);

	_scriptStack.push_back((stackArgs[0] == static_cast<StackValue_t>(_direction)) ? 1 : 0);
}

void Runtime::scriptOpAngleGGet(ScriptArg_t arg) {
	TAKE_STACK(1);

	if (stackArgs[0] < 0 || stackArgs[0] >= static_cast<StackValue_t>(GyroState::kNumGyros))
		error("Invalid gyro index in angleGGet op");

	_scriptStack.push_back(_gyros.gyros[stackArgs[0]].currentState);
}

void Runtime::scriptOpSpeed(ScriptArg_t arg) {
	TAKE_STACK(1);

	_scriptEnv.fpsOverride = stackArgs[0];
}

void Runtime::scriptOpSAnimL(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + 2);

	if (stackArgs[kAnimDefStackArgs] != 0)
		warning("sanimL second operand wasn't zero (what does that do?)");

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);
	uint direction = stackArgs[kAnimDefStackArgs + 1];

	if (direction >= kNumDirections)
		error("sanimL invalid direction");

	_haveIdleAnimations[direction] = true;

	StaticAnimation &outAnim = _idleAnimations[direction];

	outAnim = StaticAnimation();
	outAnim.animDefs[0] = animDef;
	outAnim.animDefs[1] = animDef;
}

void Runtime::scriptOpChangeL(ScriptArg_t arg) {
	TAKE_STACK(1);

	// ChangeL changes the screen number, but it also forces screen entry scripts to replay, which is
	// needed for things like the fountain.
	_screenNumber = stackArgs[0];
	_havePendingScreenChange = true;
	_forceScreenChange = true;
}

void Runtime::scriptOpAnimR(ScriptArg_t arg) {
	bool isRight = false;

	if (_scriptEnv.panInteractionID == kPanLeftInteraction) {
		debug(1, "Pan-left interaction from direction %u", _direction);

		uint reverseDirectionSlice = (kNumDirections - _direction);
		if (reverseDirectionSlice == kNumDirections)
			reverseDirectionSlice = 0;

		uint initialFrame = reverseDirectionSlice * (_panLeftAnimationDef.lastFrame - _panLeftAnimationDef.firstFrame) / kNumDirections + _panLeftAnimationDef.firstFrame;

		AnimationDef trimmedAnimation = _panLeftAnimationDef;
		trimmedAnimation.lastFrame--;

		debug(1, "Running frame loop of %u - %u from frame %u", trimmedAnimation.firstFrame, trimmedAnimation.lastFrame, initialFrame);

		changeAnimation(trimmedAnimation, initialFrame, false);
		_gameState = kGameStatePanLeft;
	} else if (_scriptEnv.panInteractionID == kPanRightInteraction) {
		debug(1, "Pan-right interaction from direction %u", _direction);

		uint initialFrame = _direction * (_panRightAnimationDef.lastFrame - _panRightAnimationDef.firstFrame) / kNumDirections + _panRightAnimationDef.firstFrame;

		AnimationDef trimmedAnimation = _panRightAnimationDef;
		trimmedAnimation.lastFrame--;

		debug(1, "Running frame loop of %u - %u from frame %u", trimmedAnimation.firstFrame, trimmedAnimation.lastFrame, initialFrame);

		changeAnimation(trimmedAnimation, initialFrame, false);
		_gameState = kGameStatePanRight;

		isRight = true;
	}

	
	uint cursorID = 0;
	if (_havePanAnimations) {
		uint panCursor = 0;
		if (_panoramaDirectionFlags & kPanoramaHorizFlags)
			panCursor |= kPanCursorDraggableHoriz;
		if (_panoramaDirectionFlags & kPanoramaUpFlag)
			panCursor |= kPanCursorDraggableUp;
		if (_panoramaDirectionFlags & kPanoramaUpFlag)
			panCursor |= kPanCursorDraggableDown;

		if (isRight)
			panCursor |= kPanCursorDirectionRight;
		else
			panCursor |= kPanCursorDirectionLeft;

		cursorID = _panCursors[panCursor];
	}

	changeToCursor(_cursors[cursorID]);
}

void Runtime::scriptOpAnimF(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + 3);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);

	const AnimationDef *faceDirectionAnimDef = nullptr;
	uint initialFrame = 0;
	uint stopFrame = 0;
	if (computeFaceDirectionAnimation(stackArgs[kAnimDefStackArgs + 2], faceDirectionAnimDef, initialFrame, stopFrame)) {
		_postFacingAnimDef = animDef;
		_animStopFrame = stopFrame;
		changeAnimation(*faceDirectionAnimDef, initialFrame, false);
		_gameState = kGameStateWaitingForFacingToAnim;
	} else {
		changeAnimation(animDef, true);
		_gameState = kGameStateWaitingForAnimation;
	}
	_screenNumber = stackArgs[kAnimDefStackArgs + 0];
	_direction = stackArgs[kAnimDefStackArgs + 1];
	_havePendingScreenChange = true;

	changeToCursor(_cursors[kCursorArrow]);
}

void Runtime::scriptOpAnimN(ScriptArg_t arg) {
	TAKE_STACK(1);

	const AnimationDef *faceDirectionAnimDef = nullptr;
	uint initialFrame = 0;
	uint stopFrame = 0;
	if (computeFaceDirectionAnimation(stackArgs[0], faceDirectionAnimDef, initialFrame, stopFrame)) {
		_animStopFrame = stopFrame;
		changeAnimation(*faceDirectionAnimDef, initialFrame, false);
		_gameState = kGameStateWaitingForFacing;
	}

	_direction = stackArgs[0];
	_havePendingScreenChange = true;

	changeToCursor(_cursors[kCursorArrow]);
}

void Runtime::scriptOpAnimG(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs * 2 + 1);

	_gyros.posAnim = stackArgsToAnimDef(stackArgs + 0);
	_gyros.negAnim = stackArgsToAnimDef(stackArgs + kAnimDefStackArgs);
	_gyros.isVertical = (stackArgs[kAnimDefStackArgs * 2 + 0] != 0);

	if (_gyros.isVertical)
		changeToCursor(_cursors[_panCursors[kPanCursorDraggableUp | kPanCursorDraggableDown]]);
	else
		changeToCursor(_cursors[_panCursors[kPanCursorDraggableHoriz]]);

	_gyros.dragBasePoint = _mousePos;
	_gyros.dragBaseState = _gyros.gyros[_gyros.activeGyro].currentState;
	_gyros.dragCurrentState = _gyros.dragBaseState;

	_gameState = kGameStateGyroIdle;
}

void Runtime::scriptOpAnimS(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + 2);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);

	// Static animations start on the last frame
	changeAnimation(animDef, animDef.lastFrame, false);

	_gameState = kGameStateWaitingForAnimation;
	_screenNumber = stackArgs[kAnimDefStackArgs + 0];
	_direction = stackArgs[kAnimDefStackArgs + 1];
	_havePendingScreenChange = true;

	changeToCursor(_cursors[kCursorArrow]);
}

void Runtime::scriptOpAnim(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs + 2);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);
	changeAnimation(animDef, true);

	_gameState = kGameStateWaitingForAnimation;
	_screenNumber = stackArgs[kAnimDefStackArgs + 0];
	_direction = stackArgs[kAnimDefStackArgs + 1];
	_havePendingScreenChange = true;

	changeToCursor(_cursors[kCursorArrow]);
}

void Runtime::scriptOpStatic(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs);

	// QUIRK/BUG WORKAROUND: Static animations don't override other static animations!
	//
	// In Reah Room05, the script for 0b8 (NGONG) sets the static animation to :NNAWA_NGONG and then
	// to :NSWIT_SGONG, but NNAWA_NGONG is the correct one, so we must ignore the second static animation
	if (_haveIdleStaticAnimation)
		return;

	AnimationDef animDef = stackArgsToAnimDef(stackArgs);

	// QUIRK: In the Reah citadel rotor puzzle, all of the "BKOLO" screens execute :DKOLO1_BKOLO1 static but
	// doing that would replace the transition animation's last frame with the new static animation frame,
	// blanking out the puzzle, so we must detect if the new static animation is the same as the existing
	// one and if so, ignore it.
	if (animDef.animName == _idleCurrentStaticAnimation)
		return;

	changeAnimation(animDef, animDef.lastFrame, false);

	_havePendingReturnToIdleState = true;
	_havePanAnimations = false;
	_haveIdleStaticAnimation = true;
	_idleCurrentStaticAnimation = animDef.animName;

	_gameState = kGameStateWaitingForAnimation;
}

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

void Runtime::scriptOpItemCheck(ScriptArg_t arg) {
	TAKE_STACK(1);

	for (const InventoryItem &item : _inventory) {
		if (item.itemID == static_cast<uint>(stackArgs[0])) {
			_scriptEnv.lastHighlightedItem = item.itemID;
			_scriptStack.push_back(1);
			return;
		}
	}

	_scriptStack.push_back(0);
}

void Runtime::scriptOpItemRemove(ScriptArg_t arg) {
	TAKE_STACK(1);

	inventoryRemoveItem(stackArgs[0]);
}

void Runtime::scriptOpItemHighlightSet(ScriptArg_t arg) {
	TAKE_STACK(2);

	bool isHighlighted = (stackArgs[1] != 0);

	for (uint slot = 0; slot < kNumInventorySlots; slot++) {
		InventoryItem &item = _inventory[slot];

		if (item.itemID == static_cast<uint>(stackArgs[0])) {
			item.highlighted = isHighlighted;
			drawInventory(slot);
		}
	}
}

void Runtime::scriptOpItemAdd(ScriptArg_t arg) {
	TAKE_STACK(1);

	if (stackArgs[0] == 0) {
		// Weird special case, happens in Reah when breaking the glass barrier, this is called with 0 as the parameter.
		// This can't be an inventory clear because it will not clear the crutch, but it does take away the gong beater,
		// so the only explanation I can think of is that it clears the previously-checked inventory item.
		inventoryRemoveItem(_scriptEnv.lastHighlightedItem);
	} else
		inventoryAddItem(stackArgs[0]);
}

void Runtime::scriptOpItemHaveSpace(ScriptArg_t arg) {
	for (const InventoryItem &item : _inventory) {
		if (item.itemID == 0) {
			_scriptStack.push_back(1);
			return;
		}
	}

	_scriptStack.push_back(0);
}

void Runtime::scriptOpSetCursor(ScriptArg_t arg) {
	TAKE_STACK(1);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= _cursors.size())
		error("Invalid cursor ID");

	uint resolvedCursorID = stackArgs[0];

	Common::HashMap<StackValue_t, uint>::const_iterator overrideIt = _scriptCursorIDToResourceIDOverride.find(resolvedCursorID);
	if (overrideIt != _scriptCursorIDToResourceIDOverride.end())
		resolvedCursorID = overrideIt->_value;

	changeToCursor(_cursors[resolvedCursorID]);
}

void Runtime::scriptOpSetRoom(ScriptArg_t arg) {
	TAKE_STACK(1);

	_roomNumber = stackArgs[0];
}

void Runtime::scriptOpLMB(ScriptArg_t arg) {
	if (!_scriptEnv.lmb) {
		_idleHaveClickInteraction = true;
		terminateScript();
	}
}

void Runtime::scriptOpLMB1(ScriptArg_t arg) {
	if (!_scriptEnv.lmbDrag) {
		_idleHaveDragInteraction = true;
		terminateScript();
	}
}

void Runtime::scriptOpSoundS1(ScriptArg_t arg) {
	TAKE_STACK(1);

	triggerSound(false, stackArgs[0], 100, 0, false);
}

void Runtime::scriptOpSoundS2(ScriptArg_t arg) {
	TAKE_STACK(2);

	triggerSound(false, stackArgs[0], stackArgs[1], 0, false);
}

void Runtime::scriptOpSoundS3(ScriptArg_t arg) {
	TAKE_STACK(3);

	triggerSound(false, stackArgs[0], stackArgs[1], stackArgs[2], false);
}

void Runtime::scriptOpSoundL1(ScriptArg_t arg) {
	TAKE_STACK(1);

	triggerSound(true, stackArgs[0], 100, 0, false);
}

void Runtime::scriptOpSoundL2(ScriptArg_t arg) {
	TAKE_STACK(2);

	triggerSound(true, stackArgs[0], stackArgs[1], 0, false);
}

void Runtime::scriptOpSoundL3(ScriptArg_t arg) {
	TAKE_STACK(3);

	triggerSound(true, stackArgs[0], stackArgs[1], stackArgs[2], false);
}

void Runtime::scriptOp3DSoundL2(ScriptArg_t arg) {
	TAKE_STACK(4);

	setSound3DParameters(stackArgs[0], stackArgs[2], stackArgs[3], _pendingSoundParams3D);
	triggerSound(true, stackArgs[0], stackArgs[1], 0, true);
}

void Runtime::scriptOp3DSoundS2(ScriptArg_t arg) {
	TAKE_STACK(4);

	setSound3DParameters(stackArgs[0], stackArgs[2], stackArgs[3], _pendingSoundParams3D);
	triggerSound(false, stackArgs[0], stackArgs[1], 0, true);
}

void Runtime::scriptOpStopAL(ScriptArg_t arg) {
	warning("stopaL not implemented yet");
}

void Runtime::scriptOpAddXSound(ScriptArg_t arg) {
	TAKE_STACK(4);

	warning("AddXSound not implemented yet");
	(void)stackArgs;
}

void Runtime::scriptOpClrXSound(ScriptArg_t arg) {
	warning("ClrXSound not implemented yet");
}

void Runtime::scriptOpStopSndLA(ScriptArg_t arg) {
	warning("StopSndLA not implemented yet");
}

void Runtime::scriptOpStopSndLO(ScriptArg_t arg) {
	TAKE_STACK(1);

	warning("StopSndLO not implemented yet");
	(void)stackArgs;
}

void Runtime::scriptOpRange(ScriptArg_t arg) {
	TAKE_STACK(3);

	_pendingSoundParams3D.minRange = stackArgs[0];
	_pendingSoundParams3D.maxRange = stackArgs[1];
	_pendingSoundParams3D.unknownRange = stackArgs[2];
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

void Runtime::scriptOpParm0(ScriptArg_t arg) {
	TAKE_STACK(4);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= GyroState::kNumGyros)
		error("Invalid gyro index for Parm0");

	uint gyroIndex = stackArgs[0];

	Gyro &gyro = _gyros.gyros[gyroIndex];
	gyro.numPreviousStatesRequired = 3;
	for (uint i = 0; i < 3; i++)
		gyro.requiredPreviousStates[i] = stackArgs[i + 1];
}

void Runtime::scriptOpParm1(ScriptArg_t arg) {
	TAKE_STACK(3);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= GyroState::kNumGyros)
		error("Invalid gyro index for Parm1");

	uint gyroIndex = stackArgs[0];

	Gyro &gyro = _gyros.gyros[gyroIndex];
	gyro.currentState = stackArgs[1];
	gyro.requiredState = stackArgs[2];

	gyro.requireState = true;
}

void Runtime::scriptOpParm2(ScriptArg_t arg) {
	TAKE_STACK(3);

	_gyros.completeInteraction = stackArgs[0];
	_gyros.failureInteraction = stackArgs[1];
	_gyros.frameSeparation = stackArgs[2];

	if (_gyros.frameSeparation <= 0)
		error("Invalid gyro frame separation");
}

void Runtime::scriptOpParm3(ScriptArg_t arg) {
	TAKE_STACK(1);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= GyroState::kNumGyros)
		error("Invalid gyro index for Parm3");

	uint gyroIndex = stackArgs[0];

	Gyro &gyro = _gyros.gyros[gyroIndex];
	gyro.wrapAround = true;
}

void Runtime::scriptOpParmG(ScriptArg_t arg) {
	TAKE_STACK(3);

	int32 gyroSlot = stackArgs[0];
	int32 dragMargin = stackArgs[1];
	int32 maxValue = stackArgs[2];

	if (gyroSlot < 0 || static_cast<uint>(gyroSlot) >= GyroState::kNumGyros)
		error("Invalid gyro slot from ParmG op");

	_gyros.activeGyro = gyroSlot;
	_gyros.dragMargin = dragMargin;
	_gyros.maxValue = maxValue;
}

void Runtime::scriptOpSParmX(ScriptArg_t arg) {
	TAKE_STACK(3);

	_pendingStaticAnimParams.initialDelay = stackArgs[0];
	_pendingStaticAnimParams.repeatDelay = stackArgs[1];
	_pendingStaticAnimParams.lockInteractions = (stackArgs[2] != 0);

	if (_pendingStaticAnimParams.lockInteractions)
		error("Locking interactions for animation is not implemented yet");
}

void Runtime::scriptOpSAnimX(ScriptArg_t arg) {
	TAKE_STACK(kAnimDefStackArgs * 2 + 1);

	AnimationDef animDef1 = stackArgsToAnimDef(stackArgs + 0);
	AnimationDef animDef2 = stackArgsToAnimDef(stackArgs + kAnimDefStackArgs);

	uint direction = stackArgs[kAnimDefStackArgs * 2 + 0];

	if (direction >= kNumDirections)
		error("sanimX invalid direction");

	_haveIdleAnimations[direction] = true;

	StaticAnimation &outAnim = _idleAnimations[direction];

	outAnim = StaticAnimation();
	outAnim.animDefs[0] = animDef1;
	outAnim.animDefs[1] = animDef2;
	outAnim.params = _pendingStaticAnimParams;
}

void Runtime::scriptOpVolumeUp3(ScriptArg_t arg) {
	TAKE_STACK(3);

	triggerSoundRamp(stackArgs[0], stackArgs[1] * 100, stackArgs[2], false);
}

void Runtime::scriptOpVolumeDn4(ScriptArg_t arg) {
	TAKE_STACK(4);

	triggerSoundRamp(stackArgs[0], stackArgs[1] * 100, stackArgs[2], stackArgs[3] != 0);
}

void Runtime::scriptOpRandom(ScriptArg_t arg) {
	TAKE_STACK(1);

	if (stackArgs[0] == 0)
		_scriptStack.push_back(0);
	else
		_scriptStack.push_back(_rng->getRandomNumber(stackArgs[0] - 1));
}

void Runtime::scriptOpDrop(ScriptArg_t arg) {
	TAKE_STACK(1);
	(void)stackArgs;
}

void Runtime::scriptOpDup(ScriptArg_t arg) {
	TAKE_STACK(1);

	_scriptStack.push_back(stackArgs[0]);
	_scriptStack.push_back(stackArgs[0]);
}

void Runtime::scriptOpSay1(ScriptArg_t arg) {
	TAKE_STACK(3);

	warning("Say1 cycles are not implemented yet, playing first sound in the cycle");

	uint soundID = stackArgs[0];
	// uint cycleLength = stackArgs[2];

	triggerSound(false, soundID, 100, 0, false);
}

void Runtime::scriptOpSay3(ScriptArg_t arg) {
	TAKE_STACK(3);

	TriggeredOneShot oneShot;
	oneShot.soundID = stackArgs[0];
	oneShot.uniqueSlot = stackArgs[1];

	// The third param seems to control sound interruption, but say3 is a Reah-only op and it's only ever 1.
	if (stackArgs[2] != 1)
		error("Invalid interrupt arg for say3, only 1 is supported.");

	if (Common::find(_triggeredOneShots.begin(), _triggeredOneShots.end(), oneShot) == _triggeredOneShots.end()) {
		triggerSound(false, oneShot.soundID, 100, 0, false);
		_triggeredOneShots.push_back(oneShot);
	}
}

void Runtime::scriptOpSay3Get(ScriptArg_t arg) {
	TAKE_STACK(3);

	TriggeredOneShot oneShot;
	oneShot.soundID = stackArgs[0];
	oneShot.uniqueSlot = stackArgs[1];

	// The third param seems to control sound interruption, but say3 is a Reah-only op and it's only ever 1.
	if (stackArgs[2] != 1)
		error("Invalid interrupt arg for say3, only 1 is supported.");

	if (Common::find(_triggeredOneShots.begin(), _triggeredOneShots.end(), oneShot) == _triggeredOneShots.end()) {
		triggerSound(false, oneShot.soundID, 100, 0, false);
		_triggeredOneShots.push_back(oneShot);
		_scriptStack.push_back(oneShot.soundID);
	} else
		_scriptStack.push_back(0);
}

void Runtime::scriptOpSetTimer(ScriptArg_t arg) {
	TAKE_STACK(2);

	_timers[static_cast<uint>(stackArgs[0])] = g_system->getMillis() + static_cast<uint32>(stackArgs[1]) * 1000u;
}

void Runtime::scriptOpGetTimer(ScriptArg_t arg) {
	TAKE_STACK(1);

	bool isCompleted = true;

	Common::HashMap<uint, uint32>::const_iterator timerIt = _timers.find(stackArgs[0]);
	if (timerIt != _timers.end())
		isCompleted = (g_system->getMillis() >= timerIt->_value);

	_scriptStack.push_back(isCompleted ? 1 : 0);
}

void Runtime::scriptOpDelay(ScriptArg_t arg) {
	TAKE_STACK(1);

	warning("Delay opcode is not implemented yet");
	(void)stackArgs;
}

void Runtime::scriptOpLoSet(ScriptArg_t arg) {
	scriptOpVerticalPanSet(_havePanDownFromDirection);
}

void Runtime::scriptOpLoGet(ScriptArg_t arg) {
	scriptOpVerticalPanGet();
}

void Runtime::scriptOpHiSet(ScriptArg_t arg) {
	scriptOpVerticalPanSet(_havePanUpFromDirection);
}

void Runtime::scriptOpHiGet(ScriptArg_t arg) {
	scriptOpVerticalPanGet();
}

void Runtime::scriptOpVerticalPanSet(bool *flags) {
	TAKE_STACK(2);

	uint baseDirection = static_cast<uint>(stackArgs[0]) % kNumDirections;
	uint radius = stackArgs[1];

	flags[baseDirection] = true;

	uint rDir = baseDirection;
	uint lDir = baseDirection;
	for (uint i = 1; i <= radius; i++) {
		rDir++;
		if (rDir == kNumDirections)
			rDir = 0;

		if (lDir == 0)
			lDir = kNumDirections;
		lDir--;

		flags[lDir] = true;
		flags[rDir] = true;
	}
}

void Runtime::scriptOpVerticalPanGet() {
	TAKE_STACK(2);

	// In any scenario where this is used, there is a corresponding hi/lo set and this only ever triggers off of interactions,
	// so don't really even need to check anything other than the facing direction?
	uint baseDirection = static_cast<uint>(stackArgs[0]) % kNumDirections;
	uint radius = stackArgs[1];

	uint rtDirection = (baseDirection + kNumDirections - _direction) % kNumDirections;
	uint lfDirection = (_direction + kNumDirections - baseDirection) % kNumDirections;

	bool isInRadius = (rtDirection <= radius || lfDirection <= radius);

	_scriptStack.push_back(isInRadius ? 1 : 0);
}

void Runtime::scriptOpSaveAs(ScriptArg_t arg) {
	TAKE_STACK(4);

	// Just ignore this op, it looks like it's for save room remapping of some sort but we allow
	// saves at any idle screen.
	(void)stackArgs;
}

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

void Runtime::scriptOpAdd(ScriptArg_t arg) {
	TAKE_STACK(2);

	_scriptStack.push_back(stackArgs[0] + stackArgs[1]);
}

void Runtime::scriptOpSub(ScriptArg_t arg) {
	TAKE_STACK(2);

	_scriptStack.push_back(stackArgs[0] - stackArgs[1]);
}

void Runtime::scriptOpNegate(ScriptArg_t arg) {
	TAKE_STACK(1);

	_scriptStack.push_back(-stackArgs[0]);
}

void Runtime::scriptOpCmpEq(ScriptArg_t arg) {
	TAKE_STACK(2);

	_scriptStack.push_back((stackArgs[0] == stackArgs[1]) ? 1 : 0);
}

void Runtime::scriptOpCmpLt(ScriptArg_t arg) {
	TAKE_STACK(2);

	_scriptStack.push_back((stackArgs[0] < stackArgs[1]) ? 1 : 0);
}

void Runtime::scriptOpCmpGt(ScriptArg_t arg) {
	TAKE_STACK(2);

	_scriptStack.push_back((stackArgs[0] > stackArgs[1]) ? 1 : 0);
}

void Runtime::scriptOpBitLoad(ScriptArg_t arg) {
	TAKE_STACK(2);


	_scriptStack.push_back((stackArgs[0] >> stackArgs[1]) & 1);
}

void Runtime::scriptOpBitSet0(ScriptArg_t arg) {
	TAKE_STACK(2);

	ScriptArg_t bitMask = static_cast<ScriptArg_t>(1) << stackArgs[1];
	_scriptStack.push_back(stackArgs[0] & ~bitMask);
}

void Runtime::scriptOpBitSet1(ScriptArg_t arg) {
	TAKE_STACK(2);

	ScriptArg_t bitMask = static_cast<ScriptArg_t>(1) << stackArgs[1];
	_scriptStack.push_back(stackArgs[0] | bitMask);
}

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

void Runtime::scriptOpGoto(ScriptArg_t arg) {
	TAKE_STACK(1);

	uint newInteraction = static_cast<uint>(stackArgs[0]);

	Common::SharedPtr<Script> newScript = nullptr;
	
	if (_scriptSet) {
		RoomScriptSetMap_t::const_iterator roomScriptIt = _scriptSet->roomScripts.find(_roomNumber);
		if (roomScriptIt != _scriptSet->roomScripts.end()) {
			const ScreenScriptSetMap_t &screenScriptsMap = roomScriptIt->_value->screenScripts;
			ScreenScriptSetMap_t::const_iterator screenScriptIt = screenScriptsMap.find(_screenNumber);
			if (screenScriptIt != screenScriptsMap.end()) {
				const ScreenScriptSet &screenScriptSet = *screenScriptIt->_value;

				ScriptMap_t::const_iterator interactionScriptIt = screenScriptSet.interactionScripts.find(newInteraction);
				if (interactionScriptIt != screenScriptSet.interactionScripts.end())
					newScript = interactionScriptIt->_value;
			}
		}
	}

	if (newScript) {
		_scriptNextInstruction = 0;
		_activeScript = newScript;
	} else {
		error("Goto target %u couldn't be resolved", newInteraction);
	}
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
	if (_roomNumber >= _roomDefs.size())
		error("Can't resolve animation for room, room number was invalid");

	Common::SharedPtr<RoomDef> roomDef = _roomDefs[_roomNumber];
	if (!roomDef)
		error("Can't resolve animation for room, room number was invalid");


	Common::HashMap<Common::String, AnimationDef>::const_iterator it = roomDef->animations.find(_scriptSet->strings[arg]);
	if (it == roomDef->animations.end())
		error("Can't resolve animation for room, couldn't find animation '%s'", _scriptSet->strings[arg].c_str());

	pushAnimDef(it->_value);
}

void Runtime::scriptOpValueName(ScriptArg_t arg) {
	if (_roomNumber >= _roomDefs.size())
		error("Invalid room number for var name op");

	const RoomDef *roomDef = _roomDefs[_roomNumber].get();
	if (!roomDef)
		error("Room def doesn't exist");

	const Common::String &varName = _scriptSet->strings[arg];

	Common::HashMap<Common::String, int>::const_iterator it = roomDef->values.find(varName);
	if (it == roomDef->values.end())
		error("Value '%s' doesn't exist in room %i", varName.c_str(), static_cast<int>(_roomNumber));

	_scriptStack.push_back(it->_value);
}

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
	Common::String sndName = _scriptSet->strings[arg];

	uint soundID = 0;
	for (uint i = 0; i < 4; i++)
		soundID = soundID * 10u + (sndName[i] - '0');

	sndName.toLowercase();

	Common::HashMap<uint, Common::SharedPtr<CachedSound> >::const_iterator cachedSoundIt = _cachedSounds.find(soundID);
	if (cachedSoundIt != _cachedSounds.end() && cachedSoundIt->_value->name != sndName) {
		_cachedSounds.erase(cachedSoundIt);
		cachedSoundIt = _cachedSounds.end();
	}

	if (cachedSoundIt == _cachedSounds.end()) {
		Common::HashMap<Common::String, Common::ArchiveMemberPtr>::const_iterator waveIt = _waves.find(sndName);

		if (waveIt != _waves.end())
			loadWave(soundID, sndName, waveIt->_value);
	}

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
