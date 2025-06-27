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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/console.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/frame/app.h"

namespace Bagel {
namespace HodjNPodj {

HodjNPodjEngine *g_engine;
GAMESTRUCT *pGameParams;
CBfcMgr *lpMetaGame;

HodjNPodjEngine::HodjNPodjEngine(OSystem *syst, const ADGameDescription *gameDesc) :
	BagelEngine(syst, gameDesc) {
	g_engine = this;
	pGameParams = &_gameInfo;
	lpMetaGame = &_metaGame;
}

HodjNPodjEngine::~HodjNPodjEngine() {
}

Common::Error HodjNPodjEngine::run() {
	initGraphics(GAME_WIDTH, GAME_HEIGHT);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	_midi = new MusicPlayer();

	// Run the game

	Metagame::Frame::CTheApp app;

	if (getGameId() == "mazeodoom") {
		app.setStartupMinigame("mazedoom_demo");
	} else {
		Common::String minigame = ConfMan.get("minigame");
		if (!minigame.empty())
			app.setStartupMinigame(minigame);
	}

	app.Run();

	return Common::kNoError;
}

Common::Error HodjNPodjEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // namespace HodjNPodj
} // namespace Bagel
