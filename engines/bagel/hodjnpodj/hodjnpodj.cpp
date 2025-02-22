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

#include "common/config-manager.h"
#include "engines/util.h"
#include "bagel/baglib/bagel.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/mazedoom/maze_doom.h"
#include "bagel/console.h"

namespace Bagel {
namespace HodjNPodj {

HodjNPodjEngine *g_engine;

static const BagelReg HODJNPODJ_REG = {
	"Hodj 'n' Podj",
	".",
	"hodjnpodj.ini",
	"hodjnpodj.sav",
	4000000,
	3,
	16,
	640,
	480
};

HodjNPodjEngine::HodjNPodjEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		BagelEngine(syst, gameDesc), CBagel(&HODJNPODJ_REG) {
	g_engine = this;
}

HodjNPodjEngine::~HodjNPodjEngine() {
	g_engine = nullptr;
}

ErrorCode HodjNPodjEngine::initialize() {
	CBagel::initialize();

	return ERR_NONE;
}

ErrorCode HodjNPodjEngine::shutdown() {
	CBagel::shutdown();

	// No more Sound System
	ShutDownSoundSystem();

	return _errCode;
}

ErrorCode HodjNPodjEngine::InitializeSoundSystem(uint16 nChannels, uint32 nFreq, uint16 nBitsPerSample) {
	// Nothing to do
	return ERR_NONE;
}

ErrorCode HodjNPodjEngine::ShutDownSoundSystem() {
	_mixer->stopAll();
	return ERR_NONE;
}

Common::Error HodjNPodjEngine::run() {
	initGraphics(640, 480);

	// Initialize systems
	_screen = new Graphics::Screen();
	_midi = new MusicPlayer();
	syncSoundSettings();

	// Set the engine's debugger console
	setDebugger(new Console());

	// Initialize
	preInit();
	initialize();

	Common::String minigame = ConfMan.get("minigame");
	if (minigame == "mazedoom")
		MazeDoom::run();
	else
		warning("TODO: entire game");

	// TODO: overall game

	// shutdown
	shutdown();
	postShutDown();

	return Common::kNoError;
}

} // namespace HodjNPodj
} // namespace Bagel
