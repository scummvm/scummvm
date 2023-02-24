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
#include "common/translation.h"

#include "vcruise/runtime.h"
#include "vcruise/vcruise.h"

namespace VCruise {

VCruiseEngine::VCruiseEngine(OSystem *syst, const VCruiseGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addDirectory(gameDataDir.getPath(), gameDataDir, 0, 2);
}

VCruiseEngine::~VCruiseEngine() {
}

void VCruiseEngine::handleEvents() {
	Common::Event evt;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(evt)) {
		switch (evt.type) {
		case Common::EVENT_LBUTTONDOWN:
			_runtime->onLButtonDown(evt.mouse.x, evt.mouse.y);
			break;
		case Common::EVENT_LBUTTONUP:
			_runtime->onLButtonUp(evt.mouse.x, evt.mouse.y);
			break;
		case Common::EVENT_MOUSEMOVE:
			_runtime->onMouseMove(evt.mouse.x, evt.mouse.y);
			break;
		case Common::EVENT_KEYDOWN:
			_runtime->onKeyDown(evt.kbd.keycode);
			break;
		default:
			break;
		}
	}
}

Common::Error VCruiseEngine::run() {
	Common::List<Graphics::PixelFormat> pixelFormats = _system->getSupportedFormats();

#if !defined(USE_JPEG)
	if (_gameDescription->desc.flags & VCRUISE_GF_NEED_JPEG) {
		return Common::Error(Common::kUnknownError, _s("This game requires JPEG support, which was not compiled in."));
	}
#endif

#if !defined(USE_OGG) || !defined(USE_VORBIS)
	if (_gameDescription->desc.flags & VCRUISE_GF_WANT_OGG_VORBIS) {
		GUIErrorMessage(_("Music for this game requires Ogg Vorbis support, which was not compiled in.  The game will still play, but will not have any music."));
	}
#endif

#if !defined(USE_MAD)
	if (_gameDescription->desc.flags & VCRUISE_GF_WANT_MP3) {
		GUIErrorMessage(_("Music for this game requires MP3 support, which was not compiled in.  The game will still play, but will not have any music."));
	}
#endif

	const Graphics::PixelFormat *fmt16_565 = nullptr;
	const Graphics::PixelFormat *fmt16_555 = nullptr;
	const Graphics::PixelFormat *fmt32 = nullptr;

	for (const Graphics::PixelFormat &fmt : pixelFormats) {
		if (fmt32 == nullptr && fmt.bytesPerPixel == 4 && fmt.rBits() == 8 && fmt.gBits() == 8 && fmt.bBits() == 8)
			fmt32 = &fmt;
		if (fmt16_555 == nullptr && fmt.rBits() == 5 && fmt.gBits() == 5 && fmt.bBits() == 5)
			fmt16_555 = &fmt;
		if (fmt16_565 == nullptr && fmt.rBits() == 5 && fmt.gBits() == 6 && fmt.bBits() == 5)
			fmt16_565 = &fmt;
	}

	// Figure out screen layout
	Common::Point size;

	Common::Point videoSize;
	Common::Point traySize;
	Common::Point menuBarSize;

	if (_gameDescription->gameID == GID_REAH) {
		videoSize = Common::Point(608, 348);
		menuBarSize = Common::Point(640, 44);
		traySize = Common::Point(640, 88);
	} else if (_gameDescription->gameID == GID_SCHIZM) {
		videoSize = Common::Point(640, 360);
		menuBarSize = Common::Point(640, 32);
		traySize = Common::Point(640, 88);
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
	else if (fmt16_565)
		initGraphics(size.x, size.y, fmt16_565);
	else if (fmt16_555)
		initGraphics(size.x, size.y, fmt16_555);
	else
		error("Unable to find a suitable graphics format");

	_system->fillScreen(0);

	_runtime.reset(new Runtime(_system, _mixer, _rootFSNode, _gameDescription->gameID));
	_runtime->initSections(_videoRect, _menuBarRect, _trayRect, _system->getScreenFormat());

	const char *exeName = _gameDescription->desc.filesDescriptions[0].fileName;

	_runtime->loadCursors(exeName);

	if (ConfMan.getBool("vcruise_debug")) {
		_runtime->setDebugMode(true);
	}

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
	//case kSupportsSavingDuringRuntime:
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
