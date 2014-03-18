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

#include "illusions/illusions.h"
#include "illusions/resourcesystem.h"
#include "illusions/backgroundresource.h"
#include "illusions/camera.h"
#include "illusions/graphics.h"
#include "illusions/input.h"
#include "illusions/updatefunctions.h"
#include "illusions/actor.h"
#include "illusions/actorresource.h"
#include "illusions/thread.h"
#include "illusions/screen.h"
#include "illusions/scriptresource.h"
#include "illusions/scriptman.h"
#include "illusions/time.h"
#include "illusions/dictionary.h"

#include "audio/audiostream.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Illusions {

IllusionsEngine::IllusionsEngine(OSystem *syst, const ADGameDescription *gd) :
	Engine(syst), _gameDescription(gd) {
	
	_random = new Common::RandomSource("illusions");
	
	Engine::syncSoundSettings();

}

IllusionsEngine::~IllusionsEngine() {

	delete _random;

}

Common::Error IllusionsEngine::run() {

	// Init search paths
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");
	SearchMan.addSubDirectoryMatching(gameDataDir, "resource");
	SearchMan.addSubDirectoryMatching(gameDataDir, "resrem");
	SearchMan.addSubDirectoryMatching(gameDataDir, "savegame");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx");
	SearchMan.addSubDirectoryMatching(gameDataDir, "video");
	SearchMan.addSubDirectoryMatching(gameDataDir, "voice");

	Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, true, &pixelFormat16);
	
	_dict = new Dictionary();

	_resSys = new ResourceSystem();
	_resSys->addResourceLoader(0x00060000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x000D0000, new ScriptResourceLoader(this));
	_resSys->addResourceLoader(0x00100000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00110000, new BackgroundResourceLoader(this));

	_screen = new Screen(this);
	_input = new Input();	
	_scriptMan = new ScriptMan(this);
	_actorItems = new ActorItems(this);
	_backgroundItems = new BackgroundItems(this);
	_camera = new Camera(this);
	
#if 0
	// ActorResource test
	_resSys->loadResource(0x00100006, 0, 0);
#endif

#if 1
	// BackgroundResource test
	_resSys->loadResource(0x00110007, 0, 0);
	BackgroundItem *backgroundItem = _backgroundItems->debugFirst();
	_system->copyRectToScreen((byte*)backgroundItem->_surfaces[0]->getBasePtr(0, 0), backgroundItem->_surfaces[0]->pitch, 0, 0, 640, 480);
	_system->updateScreen();
	_camera->panToPoint(Common::Point(800, 0), 500, 0);
	while (!shouldQuit()) {
		//debug("panPoints[0] = %d, %d", backgroundItem->_panPoints[0].x, backgroundItem->_panPoints[0].y);
		uint32 t = getCurrentTime();
		//debug("t = %d", t);
		_camera->update(t);
		_system->delayMillis(10);
		_system->copyRectToScreen((byte*)backgroundItem->_surfaces[0]->getBasePtr(backgroundItem->_panPoints[0].x, 0),
			backgroundItem->_surfaces[0]->pitch, 0, 0, 640, 480);
		_system->updateScreen();
		updateEvents();
	}
#endif
	
#if 0
	// ScriptResource test
	_resSys->loadResource(0x000D0001, 0, 0);
	
	_scriptMan->startScriptThread(0x00020004, 0, 0, 0, 0);

	while (!shouldQuit()) {
		updateEvents();
		_scriptMan->_threads->updateThreads();
	}
	
	
#endif

	delete _camera;
	delete _backgroundItems;
	delete _actorItems;
	delete _scriptMan;
	delete _input;
	delete _screen;
	delete _resSys;
	delete _dict;
	
	return Common::kNoError;
}

bool IllusionsEngine::hasFeature(EngineFeature f) const {
	return
		false;
		/*
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
		*/
}

void IllusionsEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			break;
		case Common::EVENT_KEYUP:
			break;
		case Common::EVENT_MOUSEMOVE:
  			break;
		case Common::EVENT_LBUTTONDOWN:
  			break;
		case Common::EVENT_LBUTTONUP:
  			break;
		case Common::EVENT_RBUTTONDOWN:
  			break;
		case Common::EVENT_RBUTTONUP:
  			break;
		case Common::EVENT_QUIT:
			quitGame();
			break;
		default:
			break;
		}
	}
}

Common::Point *IllusionsEngine::getObjectActorPositionPtr(uint32 objectId) {
	// TODO Dummy, to be replaced later
	return 0;
}

Control *IllusionsEngine::findControl(uint32 objectId) {
	// TODO Dummy, to be replaced later
	return 0;
}

void IllusionsEngine::notifyThreadId(uint32 &threadId) {
	if (threadId) {
		uint32 tempThreadId = threadId;
		threadId = 0;
		_scriptMan->_threads->notifyId(tempThreadId);
	}
}

FramesList *IllusionsEngine::findSequenceFrames(Sequence *sequence) {
	// TODO Dummy, to be replaced later
	return 0;
}

void IllusionsEngine::setCursorControl(Control *control) {
	// TODO Dummy, to be replaced later
}

void IllusionsEngine::placeCursor(Control *control, uint32 sequenceId) {
	// TODO Dummy, to be replaced later
}

bool IllusionsEngine::showCursor() {
	// TODO Dummy, to be replaced later
	// TODO ++cursor._visibleCtr;
	// TODO if (cursor._visibleCtr > 0)
	return false;
}

bool IllusionsEngine::hideCursor() {
	// TODO Dummy, to be replaced later
	// TODO --cursor._visibleCtr;
	// TODO if (cursor.visibleCtr <= 0) 
	return false;
}

} // End of namespace Illusions
