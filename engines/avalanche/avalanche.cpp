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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#include "avalanche/avalanche.h"

#include "engines/util.h"

namespace Avalanche {

	AvalancheEngine *AvalancheEngine::s_Engine = 0;

	AvalancheEngine::AvalancheEngine(OSystem *syst, const ADGameDescription *gd) : Engine(syst), _gameDescription(gd) {
		_system = syst;
		_console = new AvalancheConsole(this);
		_rnd = 0;
	}

	AvalancheEngine::~AvalancheEngine() {
		delete _console;
		delete _rnd;
	}

	GUI::Debugger *AvalancheEngine::getDebugger() {
		return _console;
	}

	Common::Platform AvalancheEngine::getPlatform() const {
		return _platform;
	}

	bool AvalancheEngine::hasFeature(EngineFeature f) const {
		return (f == kSupportsRTL) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
	}

	const char *AvalancheEngine::getCopyrightString() const {
		return "Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.";
	}

	Common::String AvalancheEngine::getSavegameFilename(int slot) {
		return _targetName + Common::String::format("-%02d.SAV", slot);
	}

	void AvalancheEngine::syncSoundSettings() {
		Engine::syncSoundSettings();

		//	_sound->syncVolume();
	}

	Common::Error AvalancheEngine::run() {
		s_Engine = this;
		initGraphics(320, 200, false);
		_console = new AvalancheConsole(this);

		//	_mouse = new MouseHandler(this);

		// Setup mixer
		syncSoundSettings();

		return Common::kNoError;
	}

	void AvalancheEngine::initialize() {
		//debugC(1, kDebugEngine, "initialize");

		_rnd = new Common::RandomSource("avalanche");
		_rnd->setSeed(42);                              // Kick random number generator
	}

} // End of namespace Avalanche
