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
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/mazedoom/maze_doom.h"
#include "gui/debugger.h"

namespace Bagel {
namespace HodjNPodj {

HodjNPodjEngine *g_engine;

const Minigame HodjNPodjEngine::MINIGAMES[] = {
	{ "mazedoom", &MazeDoom::run },
	{ nullptr, nullptr }
};


HodjNPodjEngine::HodjNPodjEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		BagelEngine(syst, gameDesc) {
	g_engine = this;
}

HodjNPodjEngine::~HodjNPodjEngine() {
	g_engine = nullptr;
}

void HodjNPodjEngine::initialize() {
	// No implementation
}

void HodjNPodjEngine::shutdown() {
	// No more Sound System
	ShutDownSoundSystem();
}

void HodjNPodjEngine::InitializeSoundSystem(uint16 nChannels, uint32 nFreq, uint16 nBitsPerSample) {
	// Nothing to do
}

void HodjNPodjEngine::ShutDownSoundSystem() {
	_mixer->stopAll();
}

Common::Error HodjNPodjEngine::run() {
	initGraphics(640, 480);

	// Initialize systems
	_screen = new Graphics::Screen();
	syncSoundSettings();

	// Set the engine's debugger console
	setDebugger(new GUI::Debugger());

	// Initialize
	initialize();

	Common::String minigame = ConfMan.get("minigame");
	if (!minigame.empty())
		playMinigame(minigame);
	else
		warning("TODO: entire game");

	// TODO: overall game

	// shutdown
	shutdown();

	return Common::kNoError;
}

void HodjNPodjEngine::playMinigame(const Common::String &name) {
	for (const Minigame *game = MINIGAMES; game->_name; ++game) {
		if (name == game->_name) {
			// Add the minigame's folder to the search path
			Common::FSNode gamePath(ConfMan.getPath("path"));
			SearchMan.addDirectory("minigame", gamePath.getChild(game->_name), 0, 2);

			game->_run();

			SearchMan.remove("minigame");
			return;
		}
	}

	error("Unknown minigame specified - %s", name.c_str());
}

} // namespace HodjNPodj
} // namespace Bagel
