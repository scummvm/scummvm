/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#include "nancy/nancy.h"
#include "nancy/resource.h"

#include "engines/util.h"

namespace Nancy {

NancyEngine *NancyEngine::s_Engine = 0;

NancyEngine::NancyEngine(OSystem *syst, const NancyGameDescription *gd) : Engine(syst), _gameDescription(gd)
{
	_system = syst;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "game");
	SearchMan.addSubDirectoryMatching(gameDataDir, "datafiles");

	DebugMan.addDebugChannel(kDebugSchedule, "Schedule", "Script Schedule debug level");
	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugDisplay, "Display", "Display debug level");
	DebugMan.addDebugChannel(kDebugMouse, "Mouse", "Mouse debug level");
	DebugMan.addDebugChannel(kDebugParser, "Parser", "Parser debug level");
	DebugMan.addDebugChannel(kDebugFile, "File", "File IO debug level");
	DebugMan.addDebugChannel(kDebugRoute, "Route", "Route debug level");
	DebugMan.addDebugChannel(kDebugInventory, "Inventory", "Inventory debug level");
	DebugMan.addDebugChannel(kDebugObject, "Object", "Object debug level");
	DebugMan.addDebugChannel(kDebugMusic, "Music", "Music debug level");

	_console = new NancyConsole(this);
	_rnd = 0;
}

NancyEngine::~NancyEngine() {

	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;
}

GUI::Debugger *NancyEngine::getDebugger() {
	return _console;
}

bool NancyEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *NancyEngine::getCopyrightString() const {
	return "Copyright 1989-1997 David P Gray, All Rights Reserved.";
}

GameType NancyEngine::getGameType() const {
	return _gameType;
}

Common::Platform NancyEngine::getPlatform() const {
	return _platform;
}

Common::Error NancyEngine::run() {
	s_Engine = this;
	Graphics::PixelFormat format(2, 5, 5, 5, 0, 10, 5, 0, 0);
	initGraphics(640, 480, true, &format);
	_console = new NancyConsole(this);

//	_mouse = new MouseHandler(this);
	_res = new ResourceManager(this);
	_res->initialize();

	// Setup mixer
	syncSoundSettings();

	Common::EventManager *ev = g_system->getEventManager();
	bool quit = false;

	while (!shouldQuit() && !quit) {
		Common::Event event;
		if (ev->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && (event.kbd.flags & Common::KBD_CTRL)) {
				switch(event.kbd.keycode) {
				case Common::KEYCODE_q:
					quit = true;
					break;
				case Common::KEYCODE_d:
					_console->attach();
				default:
					break;
				}
			}
		}
		_console->onFrame();
		_system->updateScreen();
		_system->delayMillis(16);
	}

	return Common::kNoError;
}

void NancyEngine::initialize() {
	debugC(1, kDebugEngine, "initialize");

	_rnd = new Common::RandomSource("nancy");
	_rnd->setSeed(42);                              // Kick random number generator
}

void NancyEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

//	_sound->syncVolume();
}

Common::String NancyEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%02d.SAV", slot);
}



} // End of namespace Nancy
