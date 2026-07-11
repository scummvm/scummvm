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

#include "comfy/comfy.h"
#include "comfy/detection.h"
#include "comfy/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

namespace Comfy {

ComfyEngine *g_engine;

static ComfyEngineVersion getEngineVersion(const ADGameDescription *gameDesc) {
	if (!strcmp(gameDesc->gameId, "comfyland")) {
		if (gameDesc->extra && !strcmp(gameDesc->extra, "1999"))
			return kEngineVersion3;

		return kEngineVersion1;
	}

	if (!strcmp(gameDesc->gameId, "boo") || !strcmp(gameDesc->gameId, "first") ||
			!strcmp(gameDesc->gameId, "panther"))
		return kEngineVersion2;

	return kEngineVersion3;
}

ComfyEngine::ComfyEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _engineVersion(getEngineVersion(gameDesc)),
	_language(2), _multiLanguage(true),
	_randomSource("Comfy"), _screen(nullptr), _framebufPtr(nullptr), _presentBuffer(nullptr),
	_logicalScreenWidth(!strcmp(gameDesc->gameId, "panther") ? COMFY_PANTHER_SCREEN_WIDTH : COMFY_SCREEN_WIDTH),
	_logicalScreenHeight(!strcmp(gameDesc->gameId, "panther") ? COMFY_PANTHER_SCREEN_HEIGHT : COMFY_SCREEN_HEIGHT),
	_resolutionChangeCount(0), _renderDirtyCount(0), _renderInterleaved(false),
	_colorDatStream(nullptr), _paletteDataPtr(nullptr), _vsyncPending(0), _fadeMax(0), _fadeStep(0),
	_palettePage(0), _paletteFading(false),
	_timerCurrent(1), _timer0(1), _timer1(1), _timer2(1),
	_midiTimeCounter(0), _midiInstanceEventTime(0), _midiEventBaseTime(0),
	_midiTimeScale(0x400), _midiTimeDelta(0), _midiCounterAdjustment(0), _midiPlyrDriver(nullptr),
	_keyBits(nullptr), _keyBitsSize(0),
	_keyboardMapLoaded(false), _keyboardActiveMask(0), _keyboardLatchedMask(0),
	_keymapperActiveMask(0), _keymapperLatchedMask(0), _toyKeyboardActiveMask(0),
	_toyKeyboardLatchedMask(0), _toyKeyboardHoldMask(0), _lptPrevScanState(0), _inputDeviceMode(0),
	_keyboardUiInitialized(false), _keyboardUiVisible(true),
	_stringCount(0), _sceneCount(0), _keyBitCount(0), _resourceHandleCount(0), _midiEntryCount(0),
	_picDataSize(0), _usesAnimFile(false), _sceneOpen(false),
	_sceneMidiInstanceOffset(0), _sceneEntryListOffset(0), _sceneActorPcOffset(0), _sceneStringTableOffset(0),
	_sceneHandlesOffset(0), _sceneActorsOffset(0), _sceneKeyBitsOffset(0), _scenePoolCursor(0),
	_scenePoolEvictCursor(0), _activeSceneCount(0), _sceneEntryCount(0), _sceneEntryFrameSize(0),
	_numObjects(0), _numFrames(0), _numSprites(0), _envNumSprites(0), _midiFileMode(0), _mirrorMode(false),
	_currentActor(0), _pendingScene(0), _musicEventMask(0), _musicEventFlag(0), _musicEnabled(false),
	_usesWcomfy99ScriptOps(false), _actorDestroyedCurrent(false), _lastKey(0xFFFF),
	_soundEventIndex(0), _soundEventMaximum(0), _soundEventSubIndex(0xFFFF),
	_soundEventPreviousSubIndex(0xFFFF), _midiInstanceTrackBase(1),
	_soundTileStride(0), _soundSampleRate(0x2B11), _soundNextCue(0), _soundCompressed(false), _soundPaused(false),
	_exprStackTop(0), _scriptFault(false),
	_gameInitialized(false), _videoInitialized(false),
	_timerInitialized(false), _lptKeyboardInitialized(false), _mainLoopRunning(false) {
	memset(_paletteFadeSource, 0, sizeof(_paletteFadeSource));
	memset(_paletteTarget, 0, sizeof(_paletteTarget));
	memset(_paletteDisplay, 0, sizeof(_paletteDisplay));
	memset(_logicalPalette, 0, sizeof(_logicalPalette));
	memset(&_inputQueue, 0, sizeof(_inputQueue));
	memset(_keyboardKeyToBit, 0xFF, sizeof(_keyboardKeyToBit));
	memset(_vocQueue, 0, sizeof(_vocQueue));
	memset(&_midiEvents, 0, sizeof(_midiEvents));
	memset(&_midiTracks, 0, sizeof(_midiTracks));
	memset(_midiChannels, 0, sizeof(_midiChannels));
	g_engine = this;
}

ComfyEngine::~ComfyEngine() {
	gameShutdown();
}

Common::String ComfyEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error ComfyEngine::run() {
	setDebugger(new Console());

	Common::Error result = gameInit();
	if (result.getCode() != Common::kNoError)
		return result;

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	uint16 currentScene = _language;
	uint16 chooserScene = currentScene;
	if (_multiLanguage && (ConfMan.getBool("force_language_setup") ||
			!ConfMan.getBool("comfy_language_chosen"))) {
		if (!iniGetGameDataPath(0x63)) {
			gameShutdown();
			return Common::kNoGameDataFoundError;
		}

		uint16 selectedLanguage = sceneRun(currentScene, true, false);
		if (selectedLanguage) {
			currentScene = selectedLanguage;
			chooserScene = selectedLanguage;
			iniWriteLanguage(selectedLanguage);
		}

		ConfMan.setBool("comfy_language_chosen", true);
		ConfMan.flushToDisk();
	}

	if (iniGetGameDataPath(0))
		sceneRun(currentScene, false, true);

	while (currentScene && !shouldQuit()) {
		if (!iniGetGameDataPath(currentScene)) {
			gameShutdown();
			return Common::kNoGameDataFoundError;
		}

		if (currentScene == 0x63)
			currentScene = sceneRun(chooserScene, _multiLanguage, false);
		else {
			chooserScene = currentScene;
			currentScene = sceneRun(currentScene, _multiLanguage, false);
		}

		if (currentScene && currentScene != 0x63)
			iniWriteLanguage(currentScene);
	}

	gameShutdown();
	return Common::kNoError;
}

Common::Error ComfyEngine::gameInit() {
	if (_gameInitialized)
		return Common::kNoError;

	gameConfigInit();
	if (!iniReadGameConfig())
		return Common::kNoGameDataFoundError;

	videoInit();
	timerInit();
	_gameInitialized = true;
	return Common::kNoError;
}

void ComfyEngine::gameShutdown() {
	if (_lptKeyboardInitialized)
		lptKeyboardShutdown();

	sceneClose();
	assetsUnload();

	midiPlyrStop();

	if (_timerInitialized)
		timerShutdown();

	if (_videoInitialized)
		videoShutdown();

	_mainLoopRunning = false;
	_gameInitialized = false;
}

void ComfyEngine::timerInit() {
	if (_timerInitialized)
		return;

	_timerInitialized = true;
}

void ComfyEngine::timerShutdown() {
	_timerInitialized = false;
}

void ComfyEngine::gameMainLoop() {
	_mainLoopRunning = true;

	while (_mainLoopRunning && !shouldQuit()) {
		processEvents();
		if (shouldQuit())
			break;

		gameMainLoopTick();
	}

	_mainLoopRunning = false;
}

uint16 ComfyEngine::sceneRun(uint16 sceneId, bool checkNext, bool exitFlag) {
	if (!assetsLoad() || !sceneOpen()) {
		assetsUnload();
		return 0;
	}

	keyBitSet(2);
	lptKeyboardInit();
	paletteVsyncFlip();
	paletteVsyncFlip();
	if (_activeSceneCount && _midiHandles.size() > 1)
		_midiHandles[1] += sceneId;

	gameMainLoop();
	uint16 nextScene = 0;
	if (checkNext && _activeSceneCount > 1 && _midiHandles.size() > 1)
		nextScene = _midiHandles[1];

	if (nextScene == sceneId || int16(nextScene) <= 0 || int16(nextScene) >= 0x65)
		nextScene = 0;

	lptKeyboardShutdown();
	sceneClose();
	assetsUnload();
	inputQueueReset();
	if (!exitFlag && nextScene)
		renderSetDirty();

	return nextScene;
}

void ComfyEngine::processEvents() {
	Common::Event event;

	while (!shouldQuit() && _system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (event.customType >= kActionKeyboardContact0 && event.customType <= kActionKeyboardContactLast)
				setKeyboardContact(event.customType - kActionKeyboardContact0, true, true);
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			if (event.customType >= kActionKeyboardContact0 && event.customType <= kActionKeyboardContactLast)
				setKeyboardContact(event.customType - kActionKeyboardContact0, false, true);
			break;
		case Common::EVENT_KEYDOWN:
			if (!event.kbdRepeat) {
				hostKeyboardSetKeyState(hostKeyboardVirtualKey(event.kbd.keycode), true);
				if (event.kbd.keycode == Common::KEYCODE_UP)
					inputQueuePushKey(0x48);
				else if (event.kbd.keycode == Common::KEYCODE_DOWN)
					inputQueuePushKey(0x50);
				else if (event.kbd.keycode == Common::KEYCODE_LEFT)
					inputQueuePushKey(0x4B);
				else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
					inputQueuePushKey(0x4D);
				else if (event.kbd.ascii)
					inputQueuePushChar(event.kbd.ascii);
			}
			break;
		case Common::EVENT_KEYUP:
			hostKeyboardSetKeyState(hostKeyboardVirtualKey(event.kbd.keycode), false);
			break;
		default:
			break;
		}
	}
}

void ComfyEngine::gameMainLoopTick() {
	_pendingScene = 0;
	_musicEventMask = 0;
	_musicEventFlag = 0;
	uint16 ticks = timerTick();

	midiTrackTickAndRemove();
	animFileTickCommands();
	sceneTickEvent();
	if (!_musicEnabled)
		midiPollChannels(ticks);
	paletteFadeStep(ticks);
	lptKeyboardScanAndProcess();
	actorTickTree();
	renderFrame();
	processInput();
	processMusicEvents();
	processSceneTransition();
}

void ComfyEngine::midiTrackTickAndRemove() {
	_midiTracks.baseTime++;
	_midiInstanceTrackBase = _midiTracks.baseTime;
	while (_midiTracks.nextIndex != 0x03E7 && _midiTracks.baseTime >= _midiTracks.nextTime) {
		keyBitSet(_midiTracks.entries[_midiTracks.nextIndex].id);
		_midiTracks.count--;
		_midiTracks.entries[_midiTracks.nextIndex] = _midiTracks.entries[_midiTracks.count];
		midiFindNext(_midiTracks);
	}
}

void ComfyEngine::animFileTickCommands() {
}

void ComfyEngine::sceneTickEvent() {
	soundAdvanceTick();
	if (_soundEventIndex == _soundEventMaximum)
		return;

	VocQueueEntry &entry = _vocQueue[_soundEventIndex % COMFY_VOC_QUEUE_CAPACITY];
	if (entry.state == 0xFFFF) {
		entry.state = 0;
		_soundEventSubIndex = 0xFFFF;
		soundPlayEntry(entry.soundId);
		keyBitSet(1);
		keyBitSet(4);
		keyBitClear(2);
		keyBitClear(3);
	} else if (_soundEventSubIndex != 0xFFFF && _soundEventSubIndex != _soundEventPreviousSubIndex) {
		if (!_soundEventSubIndex) {
			if (entry.argumentCount)
				keyBitSet(entry.arguments[0]);

			_soundEventIndex = (_soundEventIndex + 1) % COMFY_VOC_QUEUE_CAPACITY;
			if (_soundEventIndex == _soundEventMaximum) {
				keyBitClear(1);
				keyBitClear(4);
				keyBitSet(2);
				keyBitSet(3);
			}
		} else if (_soundEventSubIndex == 1) {
			keyBitSet(3);
			keyBitClear(4);
		} else if (_soundEventSubIndex == 2) {
			keyBitSet(4);
			keyBitClear(3);
		} else if (_soundEventSubIndex - 2 < entry.argumentCount) {
			keyBitSet(entry.arguments[_soundEventSubIndex - 2]);
		}
	}

	_soundEventPreviousSubIndex = _soundEventSubIndex;
}

void ComfyEngine::midiPollChannels(uint16 ticks) {
	if (!_midiPlyrDriver)
		return;

	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		MidiChannelState &state = _midiChannels[channel];
		if (!state.entryCount)
			continue;

		if (state.playing && !_midiPlyrDriver->musicIsSongPlaying(channel))
			state.playing = false;

		if (!state.playing)
			midiFinishChannel(channel);

		int16 volume = midiApproachTarget(state.volumeCurrent, state.volumeTarget, state.volumeTicksLeft, ticks);
		if ((state.volumeCurrent & 0x7F00) != (volume & 0x7F00))
			_midiPlyrDriver->musicSetVolume(uint16(volume >> 8), channel);

		state.volumeCurrent = volume;
		int16 pitch = midiApproachTarget(state.pitchCurrent, state.pitchTarget, state.pitchTicksLeft, ticks);
		if (pitch != state.pitchCurrent)
			_midiPlyrDriver->musicSetPitch(pitch, channel);

		state.pitchCurrent = pitch;
		int16 rate = midiApproachTarget(state.rateCurrent, state.rateTarget, state.rateTicksLeft, ticks);
		if (rate != state.rateCurrent)
			_midiPlyrDriver->musicSetRate(rate, channel);

		state.rateCurrent = rate;
	}
}

void ComfyEngine::actorTickTree() {
	if (_sceneHandles.size() > 1 && _sceneHandles[1])
		actorTickTreeInternal(_sceneHandles[1]);

	scenePackRuntimeState();
}

void ComfyEngine::renderFrame() {
	Actor *root = actorGet(0);
	uint16 frame = root ? actorReadDword(*root, kActorSpriteSelector) : 0;
	if (frame) {
		SpriteResource *background = spriteGet(int16(frame));
		if (background && background->header.width == _logicalScreenWidth &&
				background->header.height == _logicalScreenHeight && !background->pixels.empty()) {
			spriteBlitRle(&background->pixels[0], background->pixels.size());
		} else {
			uint16 clear = background && background->pixels.size() > 3 ? background->pixels[3] : 0;
			framebufClear(clear);
		}
	}

	uint16 rootIndex = _sceneHandles.size() > 1 ? _sceneHandles[1] : 0;
	if (!rootIndex || actorDraw(rootIndex, 0, 0)) {
		paletteVsyncFlip();
		videoPresentFrame();
	} else {
		renderSetDirty();
	}
}

void ComfyEngine::processInput() {
	_lastKey = 0xFFFF;
	if (inputQueueHasItems())
		_lastKey = lptReadKeyOrNext();

	lptKeyToFlags(_lastKey);
	if (_lastKey == 0x0101) {
		_mainLoopRunning = false;
		return;
	}

	uint16 rootIndex = _sceneHandles.size() > 1 ? _sceneHandles[1] : 0;
	Actor *root = actorGet(rootIndex);
	if (root && !actorReadByte(*root, kActorActive))
		_mainLoopRunning = false;
}

void ComfyEngine::processMusicEvents() {
	if (!_musicEventMask)
		return;

	if (_musicEventFlag)
		vocQueuePlayAll();

	if (vocQueueIsIdle() || _musicEnabled) {
		uint16 track = 1;
		while ((_musicEventMask = uint16(int16(_musicEventMask) >> 1)) != 0) {
			if (_musicEventMask & 1)
				environmentStore(track);

			track++;
		}
	}
}

void ComfyEngine::processSceneTransition() {
	keyBitClear(0x42);
	if (!_pendingScene) {
		actorSetAllDirty();
		return;
	}

	byte keySnapshot[15];
	memset(keySnapshot, 0, sizeof(keySnapshot));
	if (_keyBits)
		memcpy(keySnapshot, _keyBits, MIN<uint32>(sizeof(keySnapshot), _keyBitsSize));

	if (!_musicEnabled && _midiPlyrDriver)
		_midiPlyrDriver->musicStopAll(1);

	if (environmentLoad(_pendingScene)) {
		if (!_musicEnabled) {
			for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
				_midiChannels[channel].playing = false;
				if (_midiChannels[channel].entryCount)
					midiStartChannel(channel);
			}
		}

		Actor *root = actorGet(0);
		if (root) {
			paletteLoadWithFade(actorReadWord(*root, kActorXFixed), 0);
			if (actorReadDword(*root, kActorYFixed))
				paletteApplyBrightness(actorReadWord(*root, kActorYFixed));
		}
	}

	if (_keyBits)
		memcpy(_keyBits, keySnapshot, MIN<uint32>(sizeof(keySnapshot), _keyBitsSize));

	keyBitSet(0x42);
}

void ComfyEngine::vocQueuePush(uint16 soundId, uint16 argumentCount, uint32 pc) {
	uint16 next = (_soundEventMaximum + 1) % COMFY_VOC_QUEUE_CAPACITY;
	if (next == _soundEventIndex)
		return;

	VocQueueEntry &entry = _vocQueue[_soundEventMaximum % COMFY_VOC_QUEUE_CAPACITY];
	entry.soundId = soundId;
	entry.argumentCount = MIN<uint16>(argumentCount, COMFY_VOC_ARG_CAPACITY);
	entry.state = 0xFFFF;
	memset(entry.arguments, 0, sizeof(entry.arguments));
	for (uint i = 0; i < entry.argumentCount; i++)
		entry.arguments[i] = scriptReadWord(pc + i * 2);

	_soundEventMaximum = next;
	keyBitSet(1);
	keyBitSet(4);
	keyBitClear(2);
	keyBitClear(3);
}

void ComfyEngine::vocQueuePlayAll() {
	_mixer->stopHandle(_soundHandle);
	uint16 slot = _soundEventIndex;
	while (slot != _soundEventMaximum) {
		VocQueueEntry &entry = _vocQueue[slot];
		for (uint i = 0; i < entry.argumentCount && i < COMFY_VOC_ARG_CAPACITY; i++)
			keyBitSet(entry.arguments[i]);

		slot = (slot + 1) % COMFY_VOC_QUEUE_CAPACITY;
	}

	_soundEventMaximum = _soundEventIndex;
	_soundPcm.clear();
	keyBitClear(1);
	keyBitClear(4);
	keyBitSet(2);
	keyBitSet(3);
}

bool ComfyEngine::vocQueueIsIdle() {
	return _soundEventIndex == _soundEventMaximum;
}

Common::Error ComfyEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Comfy
