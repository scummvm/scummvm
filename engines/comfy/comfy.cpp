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

ComfyEngine::ComfyEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Comfy") {
	g_engine = this;
}

ComfyEngine::~ComfyEngine() {
	delete _screen;
}

uint32 ComfyEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String ComfyEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error ComfyEngine::run() {
	initGraphics(COMFY_SCREEN_WIDTH, COMFY_SCREEN_HEIGHT);
	_screen = new Graphics::Screen(COMFY_SCREEN_WIDTH, COMFY_SCREEN_HEIGHT);
	_screen->clear(0);
	_screen->update();

	setDebugger(new Console());

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	gameMainLoop();
	return Common::kNoError;
}

void ComfyEngine::gameMainLoop() {
	uint32 previousMillis = _system->getMillis();
	uint64 pitAccumulator = 0;
	uint64 pitThreshold = uint64(COMFY_PIT_TIMER_DIVISOR) * 1000;

	while (!shouldQuit()) {
		processEvents();
		if (shouldQuit())
			break;

		uint32 currentMillis = _system->getMillis();
		pitAccumulator += uint64(currentMillis - previousMillis) * COMFY_PIT_INPUT_FREQUENCY;
		previousMillis = currentMillis;

		while (pitAccumulator >= pitThreshold && !shouldQuit()) {
			pitAccumulator -= pitThreshold;
			gameMainLoopTick();
		}

		if (!shouldQuit() && pitAccumulator < pitThreshold)
			waitForTimerTick(pitAccumulator);
	}
}

void ComfyEngine::waitForTimerTick(uint64 pitAccumulator) {
	uint64 pitThreshold = uint64(COMFY_PIT_TIMER_DIVISOR) * 1000;
	uint64 remaining = pitThreshold - pitAccumulator;
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

void ComfyEngine::paletteFadeStep(uint16 ticks) {
	(void)ticks;
}

void ComfyEngine::lptKeyboardScanAndProcess() {
}

void ComfyEngine::actorTickTree() {
}

void ComfyEngine::renderFrame() {
	_screen->makeAllDirty();
	_screen->update();
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
