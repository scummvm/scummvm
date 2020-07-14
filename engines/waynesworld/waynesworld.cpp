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

#include "waynesworld/waynesworld.h"
#include "waynesworld/graphics.h"

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace WaynesWorld {

WaynesWorldEngine::WaynesWorldEngine(OSystem *syst, const ADGameDescription *gd) :
	Engine(syst), _gameDescription(gd) {

	_random = new Common::RandomSource("waynesworld");
	_isSaveAllowed = false;

	Engine::syncSoundSettings();

#ifdef USE_TRANSLATION
	_oldGUILanguage	= TransMan.getCurrentLanguage();

	if (gd->flags & GF_GUILANGSWITCH)
		TransMan.setLanguage(getLanguageLocale(gd->language));
#endif
}

WaynesWorldEngine::~WaynesWorldEngine() {
#ifdef USE_TRANSLATION
	if (TransMan.getCurrentLanguage() != _oldGUILanguage)
		TransMan.setLanguage(_oldGUILanguage);
#endif

	delete _random;

}

Common::Error WaynesWorldEngine::run() {

	_isSaveAllowed = false;

	initGraphics(320, 200);
	_screen = new Screen();
	_backgroundSurface = new WWSurface(320, 150);

#if 0
	while (!shouldQuit()) {
		updateEvents();
	}
#endif

#if 1
	loadPalette("m01/wstand0");
	g_system->getPaletteManager()->setPalette(_palette2, 0, 256);

	WWSurface *wayne = loadSurface("m01/wstand0");

	// Image::PCXDecoder *pcx = loadImage("m00/winter", false);
	// g_system->copyRectToScreen(pcx->getSurface()->getPixels(), pcx->getSurface()->pitch, 0, 0, pcx->getSurface()->w, pcx->getSurface()->h);
	// delete pcx;

	while (!shouldQuit()) {
		updateEvents();
		_screen->clear(0);
		_screen->drawSurface(wayne, _mouseX - 10, _mouseY - 10);
		g_system->updateScreen();
	}

	delete wayne;

#endif

	delete _backgroundSurface;
	delete _screen;

	return Common::kNoError;
}

bool WaynesWorldEngine::hasFeature(EngineFeature f) const {
	return false;
	/*
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
	*/
}

void WaynesWorldEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyCode = event.kbd.keycode;
			break;
		case Common::EVENT_KEYUP:
			_keyCode = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
  			_mouseX = event.mouse.x;
  			_mouseY = event.mouse.y;
  			break;
		case Common::EVENT_LBUTTONDOWN:
			//_mouseButtons |= kLeftButtonClicked;
			//_mouseButtons |= kLeftButtonDown;
  			break;
		case Common::EVENT_LBUTTONUP:
			//_mouseButtons &= ~kLeftButtonDown;
  			break;
		case Common::EVENT_RBUTTONDOWN:
			//_mouseButtons |= kRightButtonClicked;
			//_mouseButtons |= kRightButtonDown;
  			break;
		case Common::EVENT_RBUTTONUP:
			//_mouseButtons &= ~kRightButtonDown;
  			break;
		case Common::EVENT_QUIT:
			quitGame();
			break;
		default:
			break;
		}
	}
}

int WaynesWorldEngine::getRandom(int max) {
	return max == 0 ? 0 : _random->getRandomNumber(max - 1);
}

Image::PCXDecoder *WaynesWorldEngine::loadImage(const char *filename, bool appendRoomName) {
	Common::String tempFilename = appendRoomName
		? Common::String::format("%s/%s.pcx", _roomName.c_str(), filename)
		: Common::String::format("%s.pcx", filename);

	Common::File pcxFile;
	if (!pcxFile.open(tempFilename)) {
		warning("loadImage() Could not open '%s'", tempFilename.c_str());
		return nullptr;
	}

	Image::PCXDecoder *pcx = new Image::PCXDecoder();
	if (!pcx->loadStream(pcxFile)) {
		warning("loadImage() Could not process '%s'", tempFilename.c_str());
		delete pcx;
		return nullptr;
	}

	return pcx;
}

WWSurface *WaynesWorldEngine::loadSurfaceIntern(const char *filename, bool appendRoomName) {
    Image::PCXDecoder *imageDecoder = loadImage(filename, appendRoomName);
	WWSurface *surface = new WWSurface(imageDecoder->getSurface());
    delete imageDecoder;
    return surface;
}

WWSurface *WaynesWorldEngine::loadSurface(const char *filename) {
	return loadSurfaceIntern(filename, false);
}

WWSurface *WaynesWorldEngine::loadRoomSurface(const char *filename) {
	return loadSurfaceIntern(filename, true);
}

void WaynesWorldEngine::loadPalette(const char *filename) {
    Image::PCXDecoder *imageDecoder = loadImage(filename, false);
	if (imageDecoder->getPaletteColorCount() != 256) {
		warning("loadPalette() Could not load palette from '%s'", filename);
	} else {
		memcpy(_palette2, imageDecoder->getPalette(), imageDecoder->getPaletteColorCount() * 3);
	}
    delete imageDecoder;
}

} // End of namespace WaynesWorld
