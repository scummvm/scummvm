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

#include "engines/util.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/algorithm.h"

#include "vcruise/runtime.h"
#include "vcruise/vcruise.h"

namespace VCruise {

VCruiseEngine::VCruiseEngine(OSystem *syst, const VCruiseGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
}

VCruiseEngine::~VCruiseEngine() {
}

void VCruiseEngine::handleEvents() {
	Common::Event evt;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(evt)) {
		switch (evt.type) {
		default:
			break;
		}
	}
}

Common::Error VCruiseEngine::run() {
	Common::List<Graphics::PixelFormat> pixelFormats = _system->getSupportedFormats();

	const Graphics::PixelFormat *fmt16 = nullptr;
	const Graphics::PixelFormat *fmt32 = nullptr;

	for (const Graphics::PixelFormat &fmt : pixelFormats) {
		if (fmt.rBits() == 8 && fmt.gBits() == 8 && fmt.bBits() == 8)
			fmt32 = &fmt;
		if ((fmt.rBits() + fmt.gBits() + fmt.bBits()) == 16)
			fmt16 = &fmt;
	}

	// Figure out screen layout
	Common::Point size;

	Common::Point videoSize;
	Common::Point traySize;
	Common::Point menuBarSize;
	const char *exeName = nullptr;

	if (_gameDescription->gameID == GID_REAH) {
		videoSize = Common::Point(608, 348);
		menuBarSize = Common::Point(640, 44);
		traySize = Common::Point(640, 88);
		exeName = "Reah.exe";
	} else if (_gameDescription->gameID == GID_SCHIZM) {
		videoSize = Common::Point(640, 360);
		menuBarSize = Common::Point(640, 32);
		traySize = Common::Point(640, 88);
		exeName = "Schizm.exe";
	} else {
		error("Unknown game");
	}

	size.x = videoSize.x;
	if (menuBarSize.x > size.x)
		size.x = menuBarSize.x;
	if (traySize.x > size.x)
		size.x = traySize.x;

	size.y = videoSize.y + menuBarSize.y + traySize.y;

	Common::Point menuTL = Common::Point((size.x - menuBarSize.x) / 2, 0);
	Common::Point videoTL = Common::Point((size.x - videoSize.x) / 2, menuTL.y + menuBarSize.y);
	Common::Point trayTL = Common::Point((size.x - traySize.x) / 2, videoTL.y + videoSize.y);

	_menuBarRect = Common::Rect(menuTL.x, menuTL.y, menuTL.x + menuBarSize.x, menuTL.y + menuBarSize.y);
	_videoRect = Common::Rect(videoTL.x, videoTL.y, videoTL.x + videoSize.x, videoTL.y + videoSize.y);
	_trayRect = Common::Rect(trayTL.x, trayTL.y, trayTL.x + traySize.x, trayTL.y + traySize.y);

	if (fmt32)
		initGraphics(size.x, size.y, fmt32);
	else if (fmt16)
		initGraphics(size.x, size.y, fmt16);
	else
		error("Unable to find a suitable graphics format");

	_system->fillScreen(0);

	_runtime.reset(new Runtime(_system, _mixer, _rootFSNode, _gameDescription->gameID));
	_runtime->initSections(_videoRect, _menuBarRect, _trayRect, _system->getScreenFormat());

	_runtime->loadCursors(exeName);

	// Run the game
	while (!shouldQuit()) {
		handleEvents();

		if (!_runtime->runFrame())
			break;

		_runtime->drawFrame();
		_system->delayMillis(10);
	}

	_runtime.reset();

	return Common::kNoError;
}

void VCruiseEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
}

bool VCruiseEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsReturnToLauncher:
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	};
}

Common::Error VCruiseEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	return Common::Error(Common::kUnknownError);
}

bool VCruiseEngine::canSaveAutosaveCurrently() {
	return false;
}

bool VCruiseEngine::canSaveGameStateCurrently() {
	return false;
}

void VCruiseEngine::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);

	_rootFSNode = gamePath;
}

} // End of namespace VCruise
