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

#include "engines/util.h"

#include "sludge/backdrop.h"
#include "sludge/event.h"
#include "sludge/freeze.h"
#include "sludge/graphics.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"
#include "sludge/zbuffer.h"

namespace Sludge {

GraphicsManager::GraphicsManager(SludgeEngine *vm) {
	_vm = vm;
	init();
}

GraphicsManager::~GraphicsManager() {
	kill();
}

void GraphicsManager::init() {
	// Init screen surface
	_winWidth = _sceneWidth = 640;
	_winHeight = _sceneHeight = 480;

	// LightMap
	_lightMapMode = LIGHTMAPMODE_PIXEL;
	_lightMapNumber = 0;

	// Parallax
	_parallaxStuff = new Parallax;

	// Camera
	_cameraZoom = 1.0;
	_cameraX = _cameraY = 0;

	// Freeze
	_frozenStuff = nullptr;

	// Back drop
	_backdropExists = false;

	// Sprites
	_spriteLayers = new SpriteLayers;
	_spriteLayers->numLayers = 0;

	// Sprite Bank
	_allLoadedBanks.clear();

	// ZBuffer
	_zBuffer = new ZBufferData;
	_zBuffer->originalNum = -1;
	_zBuffer->sprites = nullptr;

	// Colors
	_currentBlankColour = _renderSurface.format.ARGBToColor(255, 0, 0, 0);
	_currentBurnR = 0;
	_currentBurnG = 0;
	_currentBurnB = 0;

	// Thumbnail
	_thumbWidth = 0;
	_thumbHeight = 0;

	// Transition
	resetRandW();
	_brightnessLevel = 255;
	_fadeMode = 2;
}

void GraphicsManager::kill() {
	// kill parallax
	if (_parallaxStuff) {
		_parallaxStuff->kill();
		delete _parallaxStuff;
		_parallaxStuff = nullptr;
	}

	// kill frozen stuff
	FrozenStuffStruct *killMe = _frozenStuff;
	while (killMe) {
		_frozenStuff = _frozenStuff->next;
		if (killMe->backdropSurface.getPixels())
			killMe->backdropSurface.free();
		if (killMe->lightMapSurface.getPixels())
			killMe->lightMapSurface.free();
		delete killMe;
		killMe = nullptr;
		killMe = _frozenStuff;
	}

	// kill sprite layers
	if (_spriteLayers) {
		killSpriteLayers();
		delete _spriteLayers;
		_spriteLayers = nullptr;
	}

	// kill sprite banks
	LoadedSpriteBanks::iterator it;
	for (it = _allLoadedBanks.begin(); it != _allLoadedBanks.end(); ++it) {
		delete (*it);
		(*it) = nullptr;
	}
	_allLoadedBanks.clear();

	// kill zbuffer
	if (_zBuffer) {
		killZBuffer();
		delete _zBuffer;
		_zBuffer = nullptr;
	}

	// kill surfaces
	if (_renderSurface.getPixels())
		_renderSurface.free();

	if (_snapshotSurface.getPixels())
		_snapshotSurface.free();

	if (_backdropSurface.getPixels())
		_backdropSurface.free();

	if (_origBackdropSurface.getPixels())
		_origBackdropSurface.free();
}

bool GraphicsManager::initGfx() {
	initGraphics(_winWidth, _winHeight, _vm->getScreenPixelFormat());
	_renderSurface.create(_winWidth, _winHeight, *_vm->getScreenPixelFormat());

	if (!killResizeBackdrop(_winWidth, _winHeight))
		return fatal("Couldn't allocate memory for backdrop");

	blankAllScreen();

	return true;
}

void GraphicsManager::display() {
	g_system->copyRectToScreen((byte *)_renderSurface.getPixels(), _renderSurface.pitch, 0, 0, _renderSurface.w, _renderSurface.h);
	g_system->updateScreen();
	if (_brightnessLevel < 255)
		fixBrightness();
}

void GraphicsManager::clear() {
	_renderSurface.fillRect(Common::Rect(0, 0, _backdropSurface.w, _backdropSurface.h),
			_renderSurface.format.RGBToColor(0, 0, 0));
}

bool GraphicsManager::loadParallax(uint16 v, uint16 fracX, uint16 fracY) {
	if (!_parallaxStuff)
		_parallaxStuff = new Parallax;
	return _parallaxStuff->add(v, fracX, fracY);
}

void GraphicsManager::killParallax() {
	if (!_parallaxStuff)
		return;
	_parallaxStuff->kill();
}

void GraphicsManager::saveParallax(Common::WriteStream *fp) {
	if (_parallaxStuff)
		_parallaxStuff->save(fp);
}

void GraphicsManager::drawParallax() {
	if (_parallaxStuff)
		_parallaxStuff->draw();
}

void GraphicsManager::aimCamera(int cameraX, int cameraY) {
	_cameraX = cameraX;
	_cameraY = cameraY;
	_cameraX -= (float)(_winWidth >> 1) / _cameraZoom;
	_cameraY -= (float)(_winHeight >> 1) / _cameraZoom;

	if (_cameraX < 0)
		_cameraX = 0;
	else if (_cameraX > _sceneWidth - (float)_winWidth / _cameraZoom)
		_cameraX = _sceneWidth - (float)_winWidth / _cameraZoom;
	if (_cameraY < 0)
		_cameraY = 0;
	else if (_cameraY > _sceneHeight - (float)_winHeight / _cameraZoom)
		_cameraY = _sceneHeight - (float)_winHeight / _cameraZoom;
}

void GraphicsManager::zoomCamera(int z) {
	_vm->_evtMan->mouseX() = _vm->_evtMan->mouseX() * _cameraZoom;
	_vm->_evtMan->mouseY() = _vm->_evtMan->mouseY() * _cameraZoom;

	_cameraZoom = (float)z * 0.01;
	if ((float)_winWidth / _cameraZoom > _sceneWidth)
		_cameraZoom = (float)_winWidth / _sceneWidth;
	if ((float)_winHeight / _cameraZoom > _sceneHeight)
		_cameraZoom = (float)_winHeight / _sceneHeight;

	_vm->_evtMan->mouseX() = _vm->_evtMan->mouseX() / _cameraZoom;
	_vm->_evtMan->mouseY() = _vm->_evtMan->mouseY() / _cameraZoom;
}

void GraphicsManager::saveColors(Common::WriteStream *stream) {
	stream->writeUint16BE(_currentBlankColour);
	stream->writeByte(_currentBurnR);
	stream->writeByte(_currentBurnG);
	stream->writeByte(_currentBurnB);
}

void GraphicsManager::loadColors(Common::SeekableReadStream *stream) {
	_currentBlankColour = stream->readUint16BE();
	_currentBurnR = stream->readByte();
	_currentBurnG = stream->readByte();
	_currentBurnB = stream->readByte();
}

} // End of namespace Sludge
