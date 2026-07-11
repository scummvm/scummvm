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
	_timerLastMillis(0), _pitAccumulator(0), _gameInitialized(false), _videoInitialized(false),
	_timerInitialized(false), _lptKeyboardInitialized(false), _mainLoopRunning(false) {
	memset(_paletteFadeSource, 0, sizeof(_paletteFadeSource));
	memset(_paletteTarget, 0, sizeof(_paletteTarget));
	memset(_paletteDisplay, 0, sizeof(_paletteDisplay));
	memset(_logicalPalette, 0, sizeof(_logicalPalette));
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

	videoInit();
	timerInit();
	lptKeyboardInit();
	_gameInitialized = true;
	return Common::kNoError;
}

void ComfyEngine::gameShutdown() {
	if (_lptKeyboardInitialized)
		lptKeyboardShutdown();

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

	_timerLastMillis = _system->getMillis();
	_pitAccumulator = 0;
	_timerInitialized = true;
}

void ComfyEngine::timerShutdown() {
	_timerLastMillis = 0;
	_pitAccumulator = 0;
	_timerInitialized = false;
}

void ComfyEngine::lptKeyboardInit() {
	_lptKeyboardInitialized = true;
}

void ComfyEngine::lptKeyboardShutdown() {
	_lptKeyboardInitialized = false;
}

void ComfyEngine::gameMainLoop() {
	_mainLoopRunning = true;

	while (_mainLoopRunning && !shouldQuit()) {
		processEvents();
		if (shouldQuit())
			break;

		uint32 currentMillis = _system->getMillis();
		_pitAccumulator += uint64(currentMillis - _timerLastMillis) * COMFY_PIT_INPUT_FREQUENCY;
		_timerLastMillis = currentMillis;

		while (_pitAccumulator >= uint64(COMFY_PIT_TIMER_DIVISOR) * 1000 && !shouldQuit()) {
			_pitAccumulator -= uint64(COMFY_PIT_TIMER_DIVISOR) * 1000;
			gameMainLoopTick();
		}

		if (!shouldQuit() && _pitAccumulator < uint64(COMFY_PIT_TIMER_DIVISOR) * 1000)
			waitForTimerTick();
	}

	_mainLoopRunning = false;
}

void ComfyEngine::waitForTimerTick() {
	uint64 pitThreshold = uint64(COMFY_PIT_TIMER_DIVISOR) * 1000;
	uint64 remaining = pitThreshold - _pitAccumulator;
	uint32 delay = (remaining + COMFY_PIT_INPUT_FREQUENCY - 1) / COMFY_PIT_INPUT_FREQUENCY;

	if (delay != 0)
		_system->delayMillis(delay);
}

void ComfyEngine::processEvents() {
	Common::Event event;

	while (_system->getEventManager()->pollEvent(event)) {
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

uint16 ComfyEngine::timerTick() {
	return 1;
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

void ComfyEngine::lptKeyboardScanAndProcess() {
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
