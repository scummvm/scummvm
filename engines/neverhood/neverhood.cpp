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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#include "audio/mixer.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "engines/util.h"

#include "neverhood/neverhood.h"
#include "neverhood/blbarchive.h"
#include "neverhood/console.h"
#include "neverhood/gamemodule.h"
#include "neverhood/gamevars.h"
#include "neverhood/graphics.h"
#include "neverhood/resourceman.h"
#include "neverhood/resource.h"
#include "neverhood/screen.h"
#include "neverhood/sound.h"
#include "neverhood/staticdata.h"

namespace Neverhood {

NeverhoodEngine::NeverhoodEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource("neverhood");

}

NeverhoodEngine::~NeverhoodEngine() {
	delete _rnd;
}

Common::Error NeverhoodEngine::run() {
	initGraphics(640, 480);

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "data");

	_isSaveAllowed = false;

	_mouseX = 0;
	_mouseY = 0;

	_gameState.sceneNum = 0;
	_gameState.which = 0;

	_staticData = new StaticData();
	_staticData->load("neverhood.dat");
	_gameVars = new GameVars();
	_screen = new Screen(this);
	_res = new ResourceMan();
	setDebugger(new Console(this));

	if (isDemo()) {
		_res->addArchive("a.blb");
		_res->addArchive("nevdemo.blb");
	} else {
		_res->addArchive("a.blb");
		_res->addArchive("c.blb");
		_res->addArchive("hd.blb");
		_res->addArchive("i.blb");
		_res->addArchive("m.blb");
		_res->addArchive("s.blb");
		_res->addArchive("t.blb");
	}

	CursorMan.showMouse(false);

	_soundMan = new SoundMan(this);
	_audioResourceMan = new AudioResourceMan(this);

	_gameModule = new GameModule(this);

	_isSaveAllowed = true;
	_updateSound = true;
	_enableMusic = !_mixer->isSoundTypeMuted(Audio::Mixer::kMusicSoundType);

	if (isDemo() && !isBigDemo()) {
		// Adjust this navigation list for the demo version
		NavigationList *navigationList = _staticData->getNavigationList(0x004B67E8);
		(*navigationList)[0].middleSmackerFileHash = 0;
		(*navigationList)[0].middleFlag = 1;
		(*navigationList)[2].middleSmackerFileHash = 0;
		(*navigationList)[2].middleFlag = 1;
		(*navigationList)[4].middleSmackerFileHash = 0;
		(*navigationList)[4].middleFlag = 1;
		(*navigationList)[5].middleSmackerFileHash = 0;
		(*navigationList)[5].middleFlag = 1;
	}

	if (ConfMan.hasKey("save_slot")) {
		if (loadGameState(ConfMan.getInt("save_slot")).getCode() != Common::kNoError)
			_gameModule->startup();
	} else
		_gameModule->startup();

	mainLoop();

	delete _gameModule;
	delete _soundMan;
	delete _audioResourceMan;

	delete _res;
	delete _screen;

	delete _gameVars;
	delete _staticData;

	return Common::kNoError;
}

void NeverhoodEngine::mainLoop() {
	uint32 nextFrameTime = 0;
	while (!shouldQuit()) {
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				_gameModule->handleKeyDown(event.kbd.keycode);
				_gameModule->handleAsciiKey(event.kbd.ascii);
				break;
			case Common::EVENT_KEYUP:
				break;
			case Common::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				_gameModule->handleMouseMove(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
				_gameModule->handleMouseDown(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_RBUTTONUP:
				_gameModule->handleMouseUp(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_WHEELUP:
				_gameModule->handleWheelUp();
				break;
			case Common::EVENT_WHEELDOWN:
				_gameModule->handleWheelDown();
				break;
			default:
				break;
			}
		}
		if (_system->getMillis() >= nextFrameTime) {
			_gameModule->checkRequests();
			_gameModule->handleUpdate();
			_gameModule->draw();
			_screen->update();
			if (_updateSound)
				_soundMan->update();
			nextFrameTime = _screen->getNextFrameTime();
		};

		_audioResourceMan->updateMusic();

		_system->updateScreen();
		_system->delayMillis(10);
	}
}

NPoint NeverhoodEngine::getMousePos() {
	NPoint pt;
	pt.x = _mouseX;
	pt.y = _mouseY;
	return pt;
}

} // End of namespace Neverhood
