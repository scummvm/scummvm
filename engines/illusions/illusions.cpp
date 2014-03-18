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
	_controls = new Controls(this);
	
#if 0
	// ActorResource test
	_resSys->loadResource(0x00100006, 0, 0);
#endif

#if 0
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

#if 1
	// Actor/graphics test
	_resSys->loadResource(0x00110007, 0, 0);
	_resSys->loadResource(0x00100006, 0, 0);
	
	_controls->placeActor(0x00050008, Common::Point(200, 200), 0x00060136, 0x00040001, 0);
	
	Control *control = *_controls->_controls.begin();
	control->setActorFrameIndex(1);
	control->appearActor();

	while (!shouldQuit()) {
		updateGraphics();
		_screen->updateSprites();
		_system->updateScreen();
		updateEvents();
		
		//break;
	}
#endif

	delete _controls;
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

void IllusionsEngine::notifyThreadId(uint32 &threadId) {
	if (threadId) {
		uint32 tempThreadId = threadId;
		threadId = 0;
		_scriptMan->_threads->notifyId(tempThreadId);
	}
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

int IllusionsEngine::updateGraphics() {
	Common::Point panPoint(0, 0);

	uint32 currTime = getCurrentTime();
    _camera->update(currTime);
    
    // TODO Move to BackgroundItems class
    BackgroundItem *backgroundItem = _backgroundItems->findActiveBackground();
    if (backgroundItem) {
    	BackgroundResource *bgRes = backgroundItem->_bgRes;
    	for (uint i = 0; i < bgRes->_bgInfosCount; ++i) {
    		BgInfo *bgInfo = &bgRes->_bgInfos[i];
			// TODO int16 priority = artcntrlGetPriorityFromBase(bgInfos[v7].priorityBase);
			int16 priority = -1;
			_screen->_drawQueue->insertSurface(backgroundItem->_surfaces[i],
				bgInfo->_surfInfo._dimensions, backgroundItem->_panPoints[i], priority);
			if (bgInfo->_flags & 1)
				panPoint = backgroundItem->_panPoints[i];
		}
	}
	
	// TODO Move to Controls class
	for (Controls::ItemsIterator it = _controls->_controls.begin(); it != _controls->_controls.end(); ++it) {
		Control *control = *it;
		Actor *actor = control->_actor;
		
		debug("control->_pauseCtr: %d; actor->_flags: %04X", control->_pauseCtr, actor->_flags);
		
		if (control->_pauseCtr == 0 && actor && (actor->_flags & 1) && !(actor->_flags & 0x0200)) {
			// TODO Common::Point drawPosition = control->calcPosition(panPoint);
			Common::Point drawPosition(200, 200);//DEBUG
			if (actor->_flags & 0x2000) {
				Frame *frame = &(*actor->_frames)[actor->_frameIndex - 1];
				_screen->_decompressQueue->insert(&actor->_drawFlags, frame->_flags,
					frame->_surfInfo._pixelSize, frame->_surfInfo._dimensions,
					frame->_compressedPixels, actor->_surface);
				actor->_flags &= ~0x2000;
			}
			/* Unused
			if (actor->_flags & 0x4000) {
				nullsub_1(&actor->drawFlags);
				actor->flags &= ~0x4000;
			}
			*/
			if (actor->_surfInfo._dimensions._width && actor->_surfInfo._dimensions._height) {
				// TODO int16 priority = control->getPriority();
				int16 priority = 2;
				_screen->_drawQueue->insertSprite(&actor->_drawFlags, actor->_surface,
					actor->_surfInfo._dimensions, drawPosition, control->_position,
					priority, actor->_scale, actor->_spriteFlags);
			}
		}
	}

#if 0 // TODO
	if (_textInfo._surface) {
		int16 priority = getPriorityFromBase(99);
		_screen->_drawQueue->insertTextSurface(_textInfo._surface, _textInfo._dimensions,
			_textInfo._position, priority);
	}
#endif
    
	return 1;
}

} // End of namespace Illusions
