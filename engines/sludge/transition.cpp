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

#include "common/textconsole.h"
#include "sludge/graphics.h"

namespace Sludge {

extern float snapTexW, snapTexH;

void GraphicsManager::setBrightnessLevel(int brightnessLevel) {
	if (brightnessLevel < 0)
		_brightnessLevel = 0;
	else if (brightnessLevel > 255)
		_brightnessLevel = 255;
	else
		_brightnessLevel = brightnessLevel;
}

//----------------------------------------------------
// PROPER BRIGHTNESS FADING
//----------------------------------------------------

unsigned lastFrom, lastTo;

void GraphicsManager::transitionFader() {
	uint br = MIN<uint>(255, _brightnessLevel);

	blendColor(&_renderSurface, TS_ARGB(br, 255, 255, 255), Graphics::BLEND_MULTIPLY);
}

void transitionCrossFader() {
#if 0
	if (! snapshotTextureName) return;

	glBindTexture(GL_TEXTURE_2D, snapshotTextureName);

	glEnable(GL_BLEND);

	const GLfloat vertices[] = {
		0.f, (GLfloat)winHeight, 0.f,
		(GLfloat)winWidth, (GLfloat)winHeight, 0.f,
		0.f, 0.f, 0.f,
		(GLfloat)winWidth, 0.f, 0.f
	};

	const GLfloat texCoords[] = {
		0.0f, snapTexH,
		snapTexW, snapTexH,
		0.0f, 0.0f,
		snapTexW, 0.0f
	};

	glUseProgram(shader.texture);

	setPMVMatrix(shader.texture);

	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 1);

	setPrimaryColor(1.0f, 1.0f, 1.0f, 1.0f - brightnessLevel / 255.f);

	drawQuad(shader.texture, vertices, 1, texCoords);

	glUniform1i(glGetUniformLocation(shader.texture, "modulateColor"), 0);
	glUseProgram(0);

	glDisable(GL_BLEND);
#endif
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
	_transitionTexture = new Graphics::TransparentSurface;

	_transitionTexture->create(256, 256, _transitionTexture->getSupportedPixelFormat());
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
	for (uint y = 0; y < _sceneHeight; y += _transitionTexture->h) {
		for (uint x = 0; x < _sceneWidth; x += _transitionTexture->w) {
			_transitionTexture->blit(_renderSurface, x, y);
		}
	}
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
	for (uint y = 0; y < _sceneHeight; y += _transitionTexture->h) {
		for (uint x = 0; x < _sceneWidth; x += _transitionTexture->w) {
			_transitionTexture->blit(_renderSurface, x, y);
		}
	}
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

		// The original stretched the texture, we just tile it
		for (uint y = 0; y < _sceneHeight; y += _transitionTexture->h) {
			for (uint x = 0; x < _sceneWidth; x += _transitionTexture->w) {
				_transitionTexture->blit(_renderSurface, x, y);
			}
		}
	}
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
