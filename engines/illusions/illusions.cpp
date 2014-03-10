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
	
	_resSys = new ResourceSystem();
	_resSys->addResourceLoader(0x00110000, new BackgroundResourceLoader(this));
	
	_resSys->loadResource(0x0011000B, 0, 0);

	BackgroundItem *backgroundItem = *(_backgroundItems.begin());
	_system->copyRectToScreen(backgroundItem->_surfaces[0]->getPixels(), backgroundItem->_surfaces[0]->pitch,
		0, 0, 640, 480);
	_system->updateScreen();

	while (!shouldQuit()) {
		updateEvents();
	}
	
	delete _resSys;
	
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

BackgroundItem *IllusionsEngine::allocBackgroundItem() {
	BackgroundItem *backgroundItem = new BackgroundItem(this);
	_backgroundItems.push_back(backgroundItem);
	return backgroundItem;
}

Graphics::Surface *IllusionsEngine::allocSurface(int16 width, int16 height) {
	// TODO Use screen pixel format?
	Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, pixelFormat16);
	return surface; 
}

Graphics::Surface *IllusionsEngine::allocSurface(SurfInfo &surfInfo) {
	return allocSurface(surfInfo._dimensions._width, surfInfo._dimensions._height);
}

} // End of namespace Illusions
