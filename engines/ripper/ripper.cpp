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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/ripper.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "ripper/detection.h"

namespace Ripper {

RipperEngine::RipperEngine(OSystem *system, const ADGameDescription *gameDescription) :
		Engine(system), _gameDescription(gameDescription) {
}

RipperEngine::~RipperEngine() {
}

bool RipperEngine::hasFeature(EngineFeature feature) const {
	return feature == kSupportsReturnToLauncher;
}

void RipperEngine::registerSearchPaths() {
	const Common::FSNode gamePath(ConfMan.getPath("path"));
	static const char *const directories[] = {
		"scene",
		"puzzle",
		"combat",
		"cyber"
	};

	for (uint i = 0; i < ARRAYSIZE(directories); ++i) {
		SearchMan.addSubDirectoryMatching(gamePath, directories[i]);
		debugC(2, kDebugResources, "Ripper: registered search directory '%s'", directories[i]);
	}
}

void RipperEngine::pumpEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
			debugC(1, kDebugGeneral, "Ripper: received quit event");
			quitGame();
		}
	}
}

Common::Error RipperEngine::run() {
	debugC(1, kDebugGeneral, "Ripper: starting skeletal engine runtime for '%s'", _gameDescription->gameId);

	registerSearchPaths();
	initGraphics(640, 480);

	while (!shouldQuit()) {
		pumpEvents();
		_system->updateScreen();
		_system->delayMillis(10);
	}

	debugC(1, kDebugGeneral, "Ripper: skeletal engine runtime stopped");
	return Common::kNoError;
}

} // End of namespace Ripper
