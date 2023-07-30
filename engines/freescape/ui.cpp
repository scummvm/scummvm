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

#include "freescape/freescape.h"

namespace Freescape {

void FreescapeEngine::titleScreen() {
	if (!_title)
		return;

	int maxWait = 60 * 6;
	for (int i = 0; i < maxWait; i++ ) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				quitGame();
				return;

			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				_gfx->clear(0, 0, 0, true);
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_SPACE:
					i = maxWait;
					break;
				default:
					break;
				}
			break;
			default:
				break;
			}
		}

		drawTitle();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}
	_gfx->clear(0, 0, 0, true);
}

void FreescapeEngine::borderScreen() {
	if (!_border)
		return;

	int maxWait = 6 * 60;
	for (int i = 0; i < maxWait; i++ ) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				quitGame();
				return;

			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				_gfx->clear(0, 0, 0, true);
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_SPACE:
					i = maxWait;
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		drawBorder();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_gfx->clear(0, 0, 0, true);
}

void FreescapeEngine::drawUI() {
	Graphics::Surface *surface = nullptr;
	if (_border) { // This can be removed when all the borders are loaded
		uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);
		surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		surface->fillRect(_fullscreenViewArea, gray);
		drawCrossair(surface);
	} else
		return;

	if (isDOS())
		drawDOSUI(surface);
	else if (isC64())
		drawC64UI(surface);
	else if (isSpectrum())
		drawZXUI(surface);
	else if (isCPC())
		drawCPCUI(surface);
	else if (isAmiga() || isAtariST())
		drawAmigaAtariSTUI(surface);

	if (!_uiTexture)
		_uiTexture = _gfx->createTexture(surface);
	else
		_uiTexture->update(surface);

	_gfx->setViewport(_fullscreenViewArea);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
	_gfx->setViewport(_viewArea);

	surface->free();
	delete surface;
}

void FreescapeEngine::drawInfoMenu() {
	warning("Function \"%s\" not implemented", __FUNCTION__);
}

void FreescapeEngine::drawCrossair(Graphics::Surface *surface) {
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	surface->drawLine(_crossairPosition.x - 3, _crossairPosition.y, _crossairPosition.x - 2, _crossairPosition.y, white);
	surface->drawLine(_crossairPosition.x + 2, _crossairPosition.y, _crossairPosition.x + 3, _crossairPosition.y, white);

	surface->drawLine(_crossairPosition.x, _crossairPosition.y - 3, _crossairPosition.x, _crossairPosition.y - 2, white);
	surface->drawLine(_crossairPosition.x, _crossairPosition.y + 2, _crossairPosition.x, _crossairPosition.y + 3, white);
}

void FreescapeEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
}

void FreescapeEngine::drawDOSUI(Graphics::Surface *surface) {
}

void FreescapeEngine::drawZXUI(Graphics::Surface *surface) {
}

void FreescapeEngine::drawCPCUI(Graphics::Surface *surface) {
}

void FreescapeEngine::drawC64UI(Graphics::Surface *surface) {
}

} // End of namespace Freescape
