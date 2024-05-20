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

#include "alcachofa/alcachofa.h"
#include "graphics/framelimiter.h"
#include "alcachofa/detection.h"
#include "alcachofa/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

#include "rooms.h"

namespace Alcachofa {

AlcachofaEngine *g_engine;

AlcachofaEngine::AlcachofaEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Alcachofa") {
	g_engine = this;
}

AlcachofaEngine::~AlcachofaEngine() {
	delete _screen;
}

uint32 AlcachofaEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String AlcachofaEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error AlcachofaEngine::run() {
	// Initialize 320x200 paletted graphics mode
	_renderer.reset(IRenderer::createOpenGLRenderer(Common::Point(1024, 768)));

	auto world = new World();
	delete world;
	Graphic graphic;
	graphic.setAnimation("MORTADELO_ACOSTANDOSE", AnimationFolder::Animations);
	graphic.loadResources();
	graphic.start(true);

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
	byte pal[256 * 3] = { 0 };
	Common::Event e;
	int offset = 0;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		_renderer->begin();

		graphic.update();

		graphic.testDraw();

		// Cycle through a simple palette
		++offset;
		for (int i = 0; i < 256; ++i)
			pal[i * 3 + 1] = (i + offset) % 256;
		g_system->getPaletteManager()->setPalette(pal, 0, 256);
		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		_renderer->end();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error AlcachofaEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Alcachofa
