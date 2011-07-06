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

#include "common/file.h"
#include "common/config-manager.h"
#include "base/plugins.h"
#include "base/version.h"
#include "graphics/cursorman.h"
#include "engines/util.h"
#include "neverhood/neverhood.h"
#include "neverhood/blbarchive.h"
#include "neverhood/collisionman.h"
#include "neverhood/gamemodule.h"
#include "neverhood/graphics.h"
#include "neverhood/resourceman.h"
#include "neverhood/resource.h"
#include "neverhood/screen.h"
#include "neverhood/staticdata.h"

namespace Neverhood {

NeverhoodEngine::NeverhoodEngine(OSystem *syst, const NeverhoodGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
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
	// Initialize backend
	_system->beginGFXTransaction();
		initCommonGFX(false);
	_system->initSize(640, 480);
	_system->endGFXTransaction();

	_isSaveAllowed = false;

	_staticData = new StaticData();
	_staticData->load("neverhood.dat");

	_screen = new Screen(this);

	_res = new ResourceMan();
	_res->addArchive("a.blb");
	_res->addArchive("c.blb");
	_res->addArchive("hd.blb");
	_res->addArchive("i.blb");
	_res->addArchive("m.blb");
	_res->addArchive("s.blb");
	_res->addArchive("t.blb");

#if 0
	BlbArchive *blb = new BlbArchive();
	blb->open("m.blb");
	delete blb;
#endif

#if 0
	ResourceFileEntry *r = _res->findEntry(0x50A80517);
#endif

#if 0
	int resourceHandle = _res->useResource(0x0CA04202);
	debug("resourceHandle = %d", resourceHandle);
	byte *data = _res->loadResource(resourceHandle);
	bool rle;
	NDimensions dimensions;
	NUnknown unknown;
	byte *palette, *pixels;
	parseBitmapResource(data, &rle, &dimensions, &unknown, &palette, &pixels);
	debug("%d, %d", dimensions.width, dimensions.height);
	byte *rawpixels = new byte[dimensions.width * dimensions.height];
	memset(rawpixels, 0, dimensions.width * dimensions.height);
	debug("rle = %d", rle);
	unpackSpriteRle(pixels, dimensions.width, dimensions.height, rawpixels, dimensions.width, false, false);
	Common::DumpFile d;
	d.open("dump.0");
	d.write(rawpixels, dimensions.width * dimensions.height);
	d.close();
	delete[] rawpixels;
	_res->unloadResource(resourceHandle);
	_res->unuseResource(resourceHandle);
#endif
	
#if 0
	{ // Create a new scope
		SpriteResource r(this);
		BaseSurface *surf = new BaseSurface(this, 0, 640, 480);
		r.load(0x0CA04202);
		debug("r: width = %d; height = %d", r.getDimensions().width, r.getDimensions().height);
		surf->drawSpriteResource(r);
		delete surf;
	}
#endif
	
#if 0
	{ // Create a new scope
		AnimResource r(this);
		r.load(0x000540B0);
	}
#endif

	_collisionMan = new CollisionMan(this);
	_gameModule = new GameModule(this);

	// Preliminary main loop, needs some more work but works for testing
	while (1) {
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
	
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				_keyState = event.kbd.keycode;
				break;
			case Common::EVENT_KEYUP:
				_keyState = Common::KEYCODE_INVALID;
				break;
			case Common::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;
			/*			
			case Common::EVENT_LBUTTONDOWN:
				_buttonState |= kLeftButton;
				break;
			case Common::EVENT_LBUTTONUP:
				_buttonState &= ~kLeftButton;
				break;
			case Common::EVENT_RBUTTONDOWN:
				_buttonState |= kRightButton;
				break;
			case Common::EVENT_RBUTTONUP:
				_buttonState &= ~kRightButton;
				break;
			case Common::EVENT_QUIT:
				_system->quit();
				break;
			*/			
			default:
				break;
			}
		}

		//debug("millis %d", _system->getMillis());		
		_gameModule->handleUpdate();
		_gameModule->draw();
		_screen->wait();
		_screen->update();
		
		//debug("---------------------------------------");
	
	}
	
	delete _gameModule;
	delete _collisionMan;
	
	delete _res;
	delete _screen;

	delete _staticData;
	
	debug("Ok.");

	return Common::kNoError;
}

uint32 NeverhoodEngine::getGlobalVar(uint32 nameHash) {
	// TODO
	return 0;
}

void NeverhoodEngine::setGlobalVar(uint32 nameHash, uint32 value) {
	// TODO
}

void NeverhoodEngine::incGlobalVar(uint32 nameHash, int incrValue) {
	setGlobalVar(nameHash, getGlobalVar(nameHash) - incrValue);
}

uint32 NeverhoodEngine::getSubVar(uint32 nameHash, uint32 subNameHash) {
	// TODO
	return 0;
}

void NeverhoodEngine::setSubVar(uint32 nameHash, uint32 subNameHash, uint32 value) {
	// TODO
}

} // End of namespace Neverhood
