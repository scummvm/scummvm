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
	_gameInitialized(false), _videoInitialized(false),
	_timerInitialized(false), _lptKeyboardInitialized(false), _mainLoopRunning(false) {
	memset(_paletteFadeSource, 0, sizeof(_paletteFadeSource));
	memset(_paletteTarget, 0, sizeof(_paletteTarget));
	memset(_paletteDisplay, 0, sizeof(_paletteDisplay));
	memset(_logicalPalette, 0, sizeof(_logicalPalette));
	memset(&_inputQueue, 0, sizeof(_inputQueue));
	memset(_keyboardKeyToBit, 0xFF, sizeof(_keyboardKeyToBit));
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

	gameMainLoop();
	gameShutdown();
	return Common::kNoError;
}

Common::Error ComfyEngine::gameInit() {
	if (_gameInitialized)
		return Common::kNoError;

	gameConfigInit();
	if (!iniReadGameConfig())
		return Common::kNoGameDataFoundError;

	if (!iniGetGameDataPath(0) || !midiPlyrStart())
		return Common::kNoGameDataFoundError;

	videoInit();
	timerInit();
	lptKeyboardInit();
	_gameInitialized = true;
	return Common::kNoError;
}

void ComfyEngine::gameShutdown() {
	if (_lptKeyboardInitialized)
		lptKeyboardShutdown();

	midiPlyrStop();
	keyBitFree();

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
			debug("EVent %d", event.type);
			break;
		}
	}
}

void ComfyEngine::gameMainLoopTick() {
	uint16 ticks = timerTick();

	midiTrackTickAndRemove();
	animFileTickCommands();
	sceneTickEvent();
	midiPollChannels(ticks);
	paletteFadeStep(ticks);
	lptKeyboardScanAndProcess();
	actorTickTree();
	paletteVsyncFlip();
	renderFrame();
	processInput();
	processMusicEvents();
	processSceneTransition();
}

void ComfyEngine::midiTrackTickAndRemove() {
}

void ComfyEngine::animFileTickCommands() {
}

void ComfyEngine::sceneTickEvent() {
}

void ComfyEngine::midiPollChannels(uint16 ticks) {
	(void)ticks;
}

void ComfyEngine::actorTickTree() {
}

void ComfyEngine::renderFrame() {
	videoPresentFrame();
}

void ComfyEngine::processInput() {
}

void ComfyEngine::processMusicEvents() {
}

void ComfyEngine::processSceneTransition() {
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
