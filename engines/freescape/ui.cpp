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
		while (_eventManager->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				quitGame();
				return;

			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				_gfx->clear(0, 0, 0, true);
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				switch (event.customType) {
				case kActionChangeModeOrSkip:
					i = maxWait;
					break;
				default:
					break;
				}
			break;
			case Common::EVENT_RBUTTONDOWN:
				// fallthrough
			case Common::EVENT_LBUTTONDOWN:
				if (g_system->hasFeature(OSystem::kFeatureTouchscreen))
					i = maxWait;
				break;
			default:
				break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		drawTitle();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}
	_gfx->clear(0, 0, 0, true);
}

Graphics::Surface *FreescapeEngine::drawStringsInSurface(const Common::Array<Common::String> &lines) {
	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, color);

	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	switch (_renderMode) {
		case Common::kRenderCGA:
			color = 1;
			break;
		case Common::kRenderZX:
			color = isCastle() ? 7 : 6;
			break;
		case Common::kRenderCPC:
			color = _gfx->_underFireBackgroundColor;
			if (color == uint32(-1))
				color = 14;
			break;
		default:
			color = 14;
	}
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	if (isAmiga() || isAtariST()) {
		r = 0xFF;
		g = 0xFF;
		b = 0x55;
	}

	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int x = _viewArea.left;
	int y = _viewArea.top;

	for (int i = 0; i < int(lines.size()); i++) {
		drawStringInSurface(lines[i], x, y, front, back, surface);
		y = y + (isCastle() ? 12 : 9);
	}
	return surface;
}

void FreescapeEngine::borderScreen() {
	if (!_border)
		return;

	if (isDriller()) {
		if (isAmiga() || isAtariST())
			return; // TODO: add animation

		drawBorderScreenAndWait(nullptr, 6 * 60);
		// Modify and reload the border
		_border->fillRect(_viewArea, _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0));
		delete _borderTexture;
		_borderTexture = nullptr;
		loadBorder();

		if (isDemo())
			return;
	}

	if (isDOS() || isSpectrum()) {
		Common::Array<Common::String> lines;
		if (isDOS())
			lines.push_back("    CONFIGURATION MENU  ");
		else
			lines.push_back("    CONTROL  OPTIONS  ");
		lines.push_back("");
		lines.push_back("   1: KEYBOARD ONLY   ");
		lines.push_back("   2: IBM JOYSTICK    ");
		lines.push_back("   3: AMSTRAD JOYSTICK");
		lines.push_back("");
		if (isDOS())
			lines.push_back(" SPACEBAR:  BEGIN MISSION");
		else
			lines.push_back("   ENTER: BEGIN MISSION");
		lines.push_back("");
		if (isDOS())
			lines.push_back(" COPYRIGHT 1988 INCENTIVE");
		else
			lines.push_back("   (C) 1988 INCENTIVE");

		lines.push_back("");
		Graphics::Surface *surface = drawStringsInSurface(lines);
		drawBorderScreenAndWait(surface, 6 * 60);
		surface->free();
		delete surface;
	}
}

void FreescapeEngine::drawFullscreenMessageAndWait(Common::String message) {
	int letterPerLine = 0;
	int numberOfLines = 0;

	if (isDOS()) {
		letterPerLine = 28;
		numberOfLines = 10;
	} else if (isSpectrum() || isCPC()) {
		letterPerLine = 24;
		numberOfLines = 12;
	} else if (isAtariST()) {
		letterPerLine = 32;
		numberOfLines = 10;
	}

	Common::Array<Common::String> lines;
	for (int i = 0; i < numberOfLines; i++) {
		lines.push_back(message.substr(letterPerLine * i, letterPerLine));
	}
	Graphics::Surface *surface = drawStringsInSurface(lines);
	drawBorderScreenAndWait(surface);
	surface->free();
	delete surface;
}

void FreescapeEngine::drawBorderScreenAndWait(Graphics::Surface *surface, int maxWait) {
	for (int i = 0; i < maxWait; i++ ) {
		Common::Event event;
		while (_eventManager->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				quitGame();
				return;

			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				_gfx->clear(0, 0, 0, true);
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				switch (event.customType) {
				case kActionChangeModeOrSkip:
					maxWait = -1;
					break;
				default:
					break;
				}
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					_demoMode = true;
					maxWait = -1;
					break;
				default:
					break;
				}
				break;
			case Common::EVENT_RBUTTONDOWN:
				// fallthrough
			case Common::EVENT_LBUTTONDOWN:
				if (g_system->hasFeature(OSystem::kFeatureTouchscreen))
					i = maxWait;
				break;
			default:
				break;
			}
		}

		_gfx->clear(0, 0, 0, true);
		drawBorder();
		if (surface)
			drawFullscreenSurface(surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_gfx->clear(0, 0, 0, true);
}

void FreescapeEngine::drawFullscreenSurface(Graphics::Surface *surface) {
	if (!_uiTexture)
		_uiTexture = _gfx->createTexture(surface);
	else
		_uiTexture->update(surface);

	_gfx->setViewport(_fullscreenViewArea);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
	_gfx->setViewport(_viewArea);
}

void FreescapeEngine::drawUI() {
	Graphics::Surface *surface = nullptr;
	if (_border) { // This can be removed when all the borders are loaded
		uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);
		surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		surface->fillRect(_fullscreenViewArea, gray);
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

	drawFullscreenSurface(surface);

	_gfx->setViewport(_fullscreenViewArea);
	_gfx->renderCrossair(_crossairPosition);

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
