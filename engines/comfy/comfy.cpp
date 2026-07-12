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
			!strcmp(gameDesc->gameId, "match"))
		return kEngineVersion2;

	return kEngineVersion3;
}

ComfyEngine::ComfyEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _engineVersion(getEngineVersion(gameDesc)),
	_randomSource("Comfy"),
	_logicalScreenWidth(!strcmp(gameDesc->gameId, "panther") ? COMFY_PANTHER_SCREEN_WIDTH : COMFY_SCREEN_WIDTH),
	_logicalScreenHeight(!strcmp(gameDesc->gameId, "panther") ? COMFY_PANTHER_SCREEN_HEIGHT : COMFY_SCREEN_HEIGHT) {
	memset(_paletteFadeSource, 0, sizeof(_paletteFadeSource));
	memset(_paletteTarget, 0, sizeof(_paletteTarget));
	memset(_paletteDisplay, 0, sizeof(_paletteDisplay));
	memset(_logicalPalette, 0, sizeof(_logicalPalette));
	memset(_keyboardKeyToBit, 0xFF, sizeof(_keyboardKeyToBit));
	memset(_selectorPoolEntries, 0, sizeof(_selectorPoolEntries));
	memset(_sceneEntryVolumes, 0, sizeof(_sceneEntryVolumes));
	memset(_sceneEntryCompletionKeys, 0, sizeof(_sceneEntryCompletionKeys));
	memset(_wcomfy99FeatureWords, 0, sizeof(_wcomfy99FeatureWords));
	memset(_animFrameHeader, 0, sizeof(_animFrameHeader));
	memset(_animFrameCommandData, 0, sizeof(_animFrameCommandData));
	memset(_animStorageChunkFileOffsets, 0, sizeof(_animStorageChunkFileOffsets));
	memset(_animStorageChunkOffsets, 0, sizeof(_animStorageChunkOffsets));
	memset(_animStorageChunkSizes, 0, sizeof(_animStorageChunkSizes));
	g_engine = this;
}

ComfyEngine::~ComfyEngine() {
	gameShutdown();
	g_engine = nullptr;
}

Common::String ComfyEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error ComfyEngine::run() {
	setDebugger(new Console());
	return gameInit();
}

Common::Error ComfyEngine::gameInit() {
	if (_gameInitialized)
		return Common::kNoError;

	gameConfigInit();
	if (!iniReadGameConfig())
		return Common::kNoGameDataFoundError;

#ifdef USE_IMGUI
	ImGuiCallbacks imGuiCallbacks;
	imGuiCallbacks.init = onImGuiInit;
	imGuiCallbacks.render = onImGuiRender;
	imGuiCallbacks.cleanup = onImGuiCleanup;
	_system->setImGuiCallbacks(imGuiCallbacks);
#endif

	timerInit();
	_gameInitialized = true;
	_currentScene = _language;
	uint16 chooserScene = 2;
	if (_multiLanguage && (ConfMan.getBool("force_language_setup") ||
			!ConfMan.getBool("comfy_language_chosen"))) {
		if (!iniGetGameDataPath(99)) {
			gameShutdown();
			return Common::kNoGameDataFoundError;
		}

		uint16 selectedLanguage = sceneRun(_currentScene, true, false);
		if (selectedLanguage) {
			_currentScene = selectedLanguage;
			chooserScene = selectedLanguage;
			iniWriteLanguage(selectedLanguage);
		}

		ConfMan.setBool("comfy_language_chosen", true);
		ConfMan.flushToDisk();
	}

	if (iniGetGameDataPath(0)) {
		sceneRun(_currentScene, false, true);
		if (shouldQuit()) {
			gameShutdown();
			return Common::kNoError;
		}
	}

	while (_currentScene && !shouldQuit()) {
		if (!iniGetGameDataPath(_currentScene)) {
			gameShutdown();
			return Common::kNoGameDataFoundError;
		}

		if (_currentScene == 99) {
			_currentScene = sceneRun(chooserScene, _multiLanguage, false);
		} else {
			chooserScene = _currentScene;
			_currentScene = sceneRun(_currentScene, _multiLanguage, false);
		}

		if (_currentScene && _currentScene != 99)
			iniWriteLanguage(_currentScene);
	}

	gameShutdown();

#ifdef USE_IMGUI
	_system->setImGuiCallbacks(ImGuiCallbacks());
#endif

	return Common::kNoError;
}

void ComfyEngine::gameShutdown() {
	if (_lptKeyboardInitialized) {
		lptKeyboardShutdown();
		midiShutdown();
		assetsUnload(0);
	}

	midiPlyrStop();

	if (_timerInitialized)
		timerShutdown();

	if (_videoInitialized)
		videoShutdown(0);

	_sceneRunBuffer.clear();
	_sceneRunPtr = nullptr;
	_languageSessionRestartRequested = false;
	_currentScene = 0;
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

uint16 ComfyEngine::sceneRun(uint16 sceneId, bool checkNext, bool exitFlag) {
	byte restorePalette = 1;
	byte shouldStartNext = 0;
	uint16 nextScene = 0;

	_lptKeyboardInitialized = true;
	_keyboardUiInitialized = true;

	hostKeyboardLoadDatMap();

	if (_sceneRunBuffer.empty()) {
		_sceneRunBuffer.resize(0x10010);
		memset(&_sceneRunBuffer[0], 0, _sceneRunBuffer.size());
	}

	_sceneRunPtr = _sceneRunBuffer.data();

	if (!assetsLoad(0x4000, _sceneRunPtr)) {
		assetsUnload(0);
		if (_engineVersion != kEngineVersion3) {
			_sceneRunBuffer.clear();
			_sceneRunPtr = nullptr;
		}

		if (_videoInitialized)
			videoShutdown(0);

		_lptKeyboardInitialized = false;
		return 0;
	}

	gameMainLoop(sceneId);
	if (_engineVersion == kEngineVersion3 && _languageSessionRestartRequested) {
		nextScene = sceneId;
		_languageSessionRestartRequested = false;
	} else if (checkNext) {
		if (sceneGetActiveCount() > 1 && _midiHandles.size() > 1)
			nextScene = midiGetHandle(1);

		if (int16(nextScene) > 0 && int16(nextScene) < 0x65 && nextScene != sceneId)
			shouldStartNext = 1;

		if (_engineVersion == kEngineVersion3)
			shouldStartNext = 1;
	}

	if (exitFlag || shouldStartNext)
		restorePalette = 0;

	if (_engineVersion == kEngineVersion3)
		animFileShutdown();

	midiShutdown();
	assetsUnload(0);
	inputQueueReset();
	if (_engineVersion != kEngineVersion3) {
		_sceneRunBuffer.clear();
		_sceneRunPtr = nullptr;
	}

	videoShutdown(restorePalette);

	_lptKeyboardInitialized = false;
	_keyboardUiInitialized = false;

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
		case Common::EVENT_MOUSEMOVE:
			if (_engineVersion == kEngineVersion3) {
				_mouseX = CLIP<int16>(int32(event.mouse.x) * 320 / _logicalScreenWidth, 0, 319);
				_mouseY = CLIP<int16>(int32(event.mouse.y) * 200 / _logicalScreenHeight, 0, 199);
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (_engineVersion == kEngineVersion3) {
				_mouseLeftButton = true;
				_mouseLeftButtonEdge = false;
			}
			break;
		case Common::EVENT_LBUTTONUP:
			if (_engineVersion == kEngineVersion3)
				_mouseLeftButtonEdge = true;
			break;
		case Common::EVENT_RBUTTONDOWN:
			if (_engineVersion == kEngineVersion3) {
				_mouseRightButton = true;
				_mouseRightButtonEdge = false;
			}
			break;
		case Common::EVENT_RBUTTONUP:
			if (_engineVersion == kEngineVersion3)
				_mouseRightButtonEdge = true;
			break;
		default:
			break;
		}
	}
}

void ComfyEngine::mouseSetActor(Actor *actor) {
	_mouseActorSceneHandle = actor ? actorReadU16(*actor, kActorSceneHandle) : 0;
}

ComfyEngine::Actor *ComfyEngine::mouseGetActor() {
	if (!_mouseActorSceneHandle)
		return nullptr;

	uint16 actorIndex = sceneGetHandle(_mouseActorSceneHandle);
	if (!actorIndex) {
		_mouseActorSceneHandle = 0;
		return nullptr;
	}

	return actorGetPtr(actorIndex);
}

void ComfyEngine::mouseClearButtonEdges() {
	if (_mouseLeftButtonEdge) {
		_mouseLeftButtonEdge = false;
		_mouseLeftButton = false;
	}

	if (_mouseRightButtonEdge) {
		_mouseRightButtonEdge = false;
		_mouseRightButton = false;
	}
}

void ComfyEngine::mouseUpdateCursor() {
	_mouseFlags |= 0x10;
	_mouseActor = mouseGetActor();
	if (!_mouseActor) {
		_mouseCursorSprite = nullptr;
		_mouseFlags &= ~0x10;
		return;
	}

	uint32 selector = actorReadU32(*_mouseActor, kActorSpriteSelector);
	if (!selector) {
		_mouseCursorSprite = nullptr;
		_mouseFlags &= ~0x10;
		return;
	}

	_mouseCursorSprite = spriteGetPtr(int16(selector));
	if (_mouseCursorSprite)
		_mouseCursorSpriteId = uint16(_mouseCursorSprite->id);

	if (!actorReadU8(*_mouseActor, kActorVisible) && !(_mouseFlags & 8))
		_mouseFlags |= 8;

	_mouseFlags &= ~0x10;
}

void ComfyEngine::gameMainLoop(uint16 argument) {
	byte keepRunning = 1;
	uint16 lastKey = 0xFFFF;
	if (_engineVersion == kEngineVersion3)
		midiInitInstance();

	actorInit(1, 0, 1, 1, 0x14, 0, 0, 0, 1);
	if (_engineVersion == kEngineVersion3)
		mouseSetActor(nullptr);

	keyBitSet(2);
	inputQueueReset();
	lptKeyboardInit();

	uint16 rootIndex = sceneGetHandle(1);
	Actor *mainLoopActor = actorGetPtr(rootIndex);
	paletteLoadWithFade(0, 0);
	paletteVsyncFlip();
	paletteVsyncFlip();
	if (sceneGetActiveCount() && _midiHandles.size() > 1)
		midiHandleAddTo(1, argument);

	while (keepRunning && !shouldQuit()) {
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
		lptKeyToFlags(lastKey);
		actorTickTree(rootIndex);
		if (_engineVersion == kEngineVersion3) {
			mouseClearButtonEdges();
			mouseUpdateCursor();
		}

		uint16 frame = actorGetFrame();
		if (frame) {
			SpriteResource *background = spriteGetPtr(int16(frame));
			if (background && background->header.width == _logicalScreenWidth &&
					background->header.height == _logicalScreenHeight && !background->pixels.empty()) {
				spriteBlitRle(&background->pixels[0], background->pixels.size());
				videoSetResolution();
			} else {
				uint16 clear = background && background->pixels.size() > 3 ? background->pixels[3] : 0;
				framebufClear(clear);
				videoSetResolution();
			}
		}

		if (actorDraw(rootIndex, 0, 0)) {
			paletteVsyncFlip();
			animFrameWaitForVocCounter();
			animFrameRecordVocCounter(3);
			videoPresentFrame();
			animFrameRecordVocCounter(4);
		} else {
			renderSetDirty();
		}

		if (_engineVersion == kEngineVersion3)
			_waveOutputActive = true;

		lastKey = 0xFFFF;
		if (inputQueueHasItems())
			lastKey = lptReadKeyOrNext();

		_lastKey = lastKey;
		if (lastKey == 0x0101)
			keepRunning = 0;
		else if (mainLoopActor)
			keepRunning = actorReadU8(*mainLoopActor, kActorActive);

		processEvents();
		if (shouldQuit())
			keepRunning = 0;

		if (_musicEventMask) {
			if (_musicEventFlag)
				vocQueuePlayAll();

			if (vocQueueIsIdle() || _musicEnabled) {
				uint16 track = 1;
				for (;;) {
					_musicEventMask = uint16(int16(_musicEventMask) >> 1);
					if (!_musicEventMask)
						break;

					if (_musicEventMask & 1)
						envConvToXms(&_sceneMemoryBlock[_sceneMidiInstanceOffset], track);

					track++;
				}
			}
		}

		keyBitClear(0x42);
		if (_pendingScene) {
			sceneStartWithMusic(_pendingScene);
			if (!_musicEnabled)
				vocQueuePlayAll();

			keyBitSet(0x42);
		} else {
			actorSetAllVisible();
		}
	}

	if (sceneGetHandle(1))
		actorFreeSlot(1);

	lptKeyboardShutdown();
	sceneBlockPackRuntimeState();
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

void ComfyEngine::sceneTickEvent() {
	soundAdvanceTick();
	if (_soundEventIndex == _soundEventMaximum)
		return;

	if (_engineVersion == kEngineVersion3) {
		VocQueueEntry1999 &entry = _vocQueue1999[_soundEventIndex];
		bool started = false;
		if (entry.state == 0xFFFF && (entry.soundId != 0xFFFF || !animFrameIsReady())) {
			entry.state = 0;
			_soundEventSubIndex = 0xFFFF;
			soundPlayEntry(entry.soundId);
			keyBitSet(1);
			keyBitSet(4);
			keyBitClear(2);
			keyBitClear(3);
			started = true;
		}

		if (!started && _soundEventSubIndex != 0xFFFF &&
				_soundEventSubIndex != _soundEventPreviousSubIndex) {
			if (!_soundEventSubIndex) {
				if (entry.argumentCount)
					keyBitSet(entry.arguments[0]);

				_soundEventIndex++;
				if (_soundEventIndex == COMFY_VOC_QUEUE_CAPACITY)
					_soundEventIndex = 0;

				if (_soundEventIndex == _soundEventMaximum) {
					keyBitClear(1);
					keyBitClear(4);
					keyBitSet(3);
					keyBitSet(2);
				}
			} else if (_soundEventSubIndex == 1) {
				keyBitSet(3);
				keyBitClear(4);
			} else if (_soundEventSubIndex == 2) {
				keyBitSet(4);
				keyBitClear(3);
			} else if (_soundEventSubIndex - 2 < entry.argumentCount) {
				if (entry.clearArgumentKeys) {
					for (uint i = 0; i < entry.argumentCount; i++)
						keyBitClear(entry.arguments[i]);
				}

				keyBitSet(entry.arguments[_soundEventSubIndex - 2]);
			}
		}

		_soundEventPreviousSubIndex = _soundEventSubIndex;
		return;
	}

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

	if (_sceneEntryListActive) {
		midiPollSceneEntries();
		return;
	}

	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		MidiChannelState &state = _midiChannels[channel];
		if (!state.entryCount)
			continue;

		uint16 marker = _midiPlyrDriver->musicGetClearMarker(channel);
		if (marker)
			midiFireClearMarker(channel, marker);

		if (!_midiPlyrDriver->musicIsSongPlaying(channel))
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

bool ComfyEngine::vocQueuePush(uint16 soundId, uint16 argumentCount, uint32 pc) {
	uint16 next = (_soundEventMaximum + 1) % COMFY_VOC_QUEUE_CAPACITY;
	if (next == _soundEventIndex)
		return false;

	if (_engineVersion == kEngineVersion3) {
		VocQueueEntry1999 &entry = _vocQueue1999[_soundEventMaximum];
		entry.clearArgumentKeys = (argumentCount & 0x80) != 0;
		argumentCount &= 0x3F;
		entry.soundId = soundId;
		entry.argumentCount = MIN<uint16>(argumentCount, COMFY_VOC_ARG_CAPACITY_1999);
		entry.state = 0xFFFF;
		for (uint i = 0; i < COMFY_VOC_ARG_CAPACITY_1999; i++)
			entry.arguments[i] = 0;

		for (uint i = 0; i < entry.argumentCount; i++)
			entry.arguments[i] = scriptReadWord(pc + i * 2);

		_soundEventMaximum = next;
		keyBitSet(1);
		keyBitSet(4);
		keyBitClear(2);
		keyBitClear(3);
		return true;
	}

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
	return true;
}

void ComfyEngine::vocQueuePlayAll() {
	_mixer->stopHandle(_soundHandle);
	_soundCues.clear();
	if (_engineVersion == kEngineVersion3) {
		uint16 slot = _soundEventIndex;
		while (slot != _soundEventMaximum) {
			VocQueueEntry1999 &entry = _vocQueue1999[slot];
			for (uint i = 0; i < entry.argumentCount; i++)
				keyBitSet(entry.arguments[i]);

			slot++;
			if (slot == COMFY_VOC_QUEUE_CAPACITY)
				slot = 0;
		}

		_soundEventMaximum = _soundEventIndex;
		_soundPcm.clear();
		keyBitClear(1);
		keyBitClear(4);
		keyBitSet(3);
		keyBitSet(2);
		return;
	}

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
	keyBitSet(3);
	keyBitSet(2);
}

bool ComfyEngine::vocQueueIsIdle() {
	return _soundEventIndex == _soundEventMaximum;
}

} // End of namespace Comfy
