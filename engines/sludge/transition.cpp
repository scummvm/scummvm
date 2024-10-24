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

#include "common/textconsole.h"
#include "sludge/graphics.h"

namespace Sludge {

extern float snapTexW, snapTexH;

void GraphicsManager::setBrightnessLevel(int brightnessLevel) {
	_brightnessLevel = CLIP(brightnessLevel, 0, 255);
}

//----------------------------------------------------
// PROPER BRIGHTNESS FADING
//----------------------------------------------------

unsigned lastFrom, lastTo;

void GraphicsManager::transitionFader() {
	blendColor(&_renderSurface, MS_ARGB(255 - _brightnessLevel, 0, 0, 0), Graphics::BLEND_NORMAL);
}

void GraphicsManager::transitionCrossFader() {
	if (!_snapshotSurface.getPixels())
		return;

	if (_brightnessLevel == 255)
		return;

	Graphics::ManagedSurface tmp;
	tmp.copyFrom(_snapshotSurface);
	tmp.blendBlitTo(_renderSurface, 0, 0, Graphics::FLIP_NONE, nullptr, MS_ARGB(255 - _brightnessLevel, 0xff, 0xff, 0xff));
}

void GraphicsManager::transitionSnapshotBox() {
	if (!_snapshotSurface.getPixels())
		return;

	if (_brightnessLevel == 255)
		return;

	uint32 xScale = (255 - _brightnessLevel) * _winWidth / 255;
	uint32 yScale = (255 - _brightnessLevel) * _winHeight / 255;

	Graphics::Surface *surf = _snapshotSurface.scale(xScale, yScale);

	_renderSurface.copyRectToSurface(surf->getPixels(), surf->pitch, (_winWidth - xScale) / 2, (_winHeight - yScale) / 2, xScale, yScale);

	delete surf;
}

//----------------------------------------------------
// FAST PSEUDO-RANDOM NUMBER STUFF FOR DISOLVE EFFECT
//----------------------------------------------------

void GraphicsManager::resetRandW() {
	int32 seed = 12345;

	for (int i = 0; i < RANDKK; i++) {
		for (int j = 0; j < 2; j++) {
			seed = seed * 2891336453u + 1;
			_randbuffer[i][j] = seed;
		}
	}

	_randp1 = 0;
	_randp2 = 10;
}

void GraphicsManager::reserveTransitionTexture() {
	_transitionTexture = new Graphics::ManagedSurface;

	_transitionTexture->create(256, 256, Graphics::BlendBlit::getSupportedPixelFormat());
}

void GraphicsManager::transitionDisolve() {
	if (!_transitionTexture)
		reserveTransitionTexture();

	if (!_brightnessLevel) {
		transitionFader();
		return;
	}

	byte *toScreen = (byte *)_transitionTexture->getPixels();
	byte *end = (byte *)_transitionTexture->getBasePtr(255, 255);

	do {
		// generate next number
		uint32 n = _randbuffer[_randp1][1];
		uint32 y = (n << 27) | ((n >> (32 - 27)) + _randbuffer[_randp2][1]);

		n = _randbuffer[_randp1][0];
		_randbuffer[_randp1][1] = (n << 19) | ((n >> (32 - 19)) + _randbuffer[_randp2][0]);
		_randbuffer[_randp1][0] = y;

		// rotate list pointers
		if (!_randp1--)
			_randp1 = RANDKK - 1;
		if (!_randp2--)
			_randp2 = RANDKK - 1;

		if ((y & 0xff) > _brightnessLevel) {
			toScreen[0] = 255;
			toScreen[1] = toScreen[2] = toScreen[3] = 0;
		} else {
			toScreen[0] = toScreen[1] = toScreen[2] = toScreen[3] = 0;
		}
		toScreen += 4;
	} while (toScreen < end);

	// The original stretched the texture, we just tile it
	for (uint y = 0; y < _sceneHeight; y += _transitionTexture->h)
		for (uint x = 0; x < _sceneWidth; x += _transitionTexture->w)
			_transitionTexture->blendBlitTo(_renderSurface, x, y);
}

void GraphicsManager::transitionTV() {
	if (!_transitionTexture)
		reserveTransitionTexture();

	byte *toScreen = (byte *)_transitionTexture->getPixels();
	byte *end = (byte *)_transitionTexture->getBasePtr(255, 255);

	do {
		// generate next number
		uint32 n = _randbuffer[_randp1][1];
		uint32 y = (n << 27) | ((n >> (32 - 27)) + _randbuffer[_randp2][1]);

		n = _randbuffer[_randp1][0];
		_randbuffer[_randp1][1] = (n << 19) | ((n >> (32 - 19)) + _randbuffer[_randp2][0]);
		_randbuffer[_randp1][0] = y;

		// rotate list pointers
		if (!_randp1--)
			_randp1 = RANDKK - 1;
		if (!_randp2--)
			_randp2 = RANDKK - 1;

		if ((y & 255u) > _brightnessLevel) {
			toScreen[0] = (n & 255);
			toScreen[1] = toScreen[2] = toScreen[3] = (n & 255);
		} else {
			toScreen[0] = toScreen[1] = toScreen[2] = toScreen[3] = 0;
		}
		toScreen += 4;
	} while (toScreen < end);

	// The original stretched the texture, we just tile it
	for (uint y = 0; y < _sceneHeight; y += _transitionTexture->h)
		for (uint x = 0; x < _sceneWidth; x += _transitionTexture->w)
			_transitionTexture->blendBlitTo(_renderSurface, x, y);
}

void GraphicsManager::transitionBlinds() {
	if (!_transitionTexture)
		reserveTransitionTexture();

	int level = _brightnessLevel / 16;

	for (int b = 0; b < 16; b++) {
		byte *toScreen = (byte *)_transitionTexture->getBasePtr(0, b * 16);

		if (level)
			memset(toScreen, 0, 256 * 4 * level);
		if (level < 32) {
			for (int y = 0; y < 16 - level; y++) {
				toScreen = (byte *)_transitionTexture->getBasePtr(0, b * 16 + y);
				for (int i = 0; i < 256; i++) {
					toScreen[0] = 0xff;
					toScreen[1] = toScreen[2] = toScreen[3] = 0;
					toScreen += 4;
				}
			}
		}
	}

	// The original stretched the texture, we just tile it
	for (uint y = 0; y < _sceneHeight; y += _transitionTexture->h)
		for (uint x = 0; x < _sceneWidth; x += _transitionTexture->w)
			_transitionTexture->blendBlitTo(_renderSurface, x, y);
}

//----------------------------------------------------

void GraphicsManager::fixBrightness() {
	switch (_fadeMode) {
	case 0:
		transitionFader();
		break;
	case 1:
		resetRandW();
		// Fall through!
	case 2:
		transitionDisolve();
		break;
	case 3:
		transitionTV();
		break;
	case 4:
		transitionBlinds();
		break;
	case 5:
		transitionCrossFader();
		break;
	case 6:
		transitionSnapshotBox();
		break;
	default:
		break;
	}
}

} // End of namespace Sludge
