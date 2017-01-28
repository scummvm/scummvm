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

#include "mohawk/resource.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_sound.h"
#include "mohawk/riven_stack.h"

#include "common/system.h"
#include "engines/util.h"

namespace Mohawk {

RivenGraphics::RivenGraphics(MohawkEngine_Riven* vm) : GraphicsManager(), _vm(vm) {
	_bitmapDecoder = new MohawkBitmap();

	// Restrict ourselves to a single pixel format to simplify the effects implementation
	_pixelFormat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(608, 436, true, &_pixelFormat);

	// The actual game graphics only take up the first 392 rows. The inventory
	// occupies the rest of the screen and we don't use the buffer to hold that.
	_mainScreen = new Graphics::Surface();
	_mainScreen->create(608, 392, _pixelFormat);

	_effectScreen = new Graphics::Surface();
	_effectScreen->create(608, 392, _pixelFormat);

	_screenUpdateNesting = 0;
	_screenUpdateRunning = false;
	_scheduledTransition = -1;	// no transition
	_dirtyScreen = false;

	_creditsImage = 302;
	_creditsPos = 0;

	_transitionSpeed = 0;
	_fliesEffect = nullptr;
}

RivenGraphics::~RivenGraphics() {
	_effectScreen->free();
	delete _effectScreen;
	_mainScreen->free();
	delete _mainScreen;
	delete _bitmapDecoder;
	delete _fliesEffect;
}

MohawkSurface *RivenGraphics::decodeImage(uint16 id) {
	MohawkSurface *surface = _bitmapDecoder->decodeImage(_vm->getResource(ID_TBMP, id));
	surface->convertToTrueColor();
	return surface;
}

void RivenGraphics::copyImageToScreen(uint16 image, uint32 left, uint32 top, uint32 right, uint32 bottom) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	beginScreenUpdate();

	// Clip the width to fit on the screen. Fixes some images.
	if (left + surface->w > 608)
		surface->w = 608 - left;

	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(left, i + top), surface->getBasePtr(0, i), surface->w * surface->format.bytesPerPixel);

	_dirtyScreen = true;
	applyScreenUpdate();
}

void RivenGraphics::updateScreen(Common::Rect updateRect) {
	if (_dirtyScreen) {
		// Copy to screen if there's no transition. Otherwise transition. ;)
		if (_scheduledTransition < 0) {
			// mainScreen -> effectScreen -> systemScreen
			_effectScreen->copyRectToSurface(*_mainScreen, updateRect.left, updateRect.top, updateRect);
			_vm->_system->copyRectToScreen(_effectScreen->getBasePtr(updateRect.left, updateRect.top), _effectScreen->pitch, updateRect.left, updateRect.top, updateRect.width(), updateRect.height());
		} else {
			runScheduledTransition();
		}

		// Finally, update the screen.
		_vm->_system->updateScreen();
		_dirtyScreen = false;
	}
}

void RivenGraphics::scheduleWaterEffect(uint16 sfxeID) {
	Common::SeekableReadStream *sfxeStream = _vm->getResource(ID_SFXE, sfxeID);

	if (sfxeStream->readUint16BE() != 'SL')
		error ("Unknown sfxe tag");

	// Read in header info
	SFXERecord sfxeRecord;
	sfxeRecord.frameCount = sfxeStream->readUint16BE();
	uint32 offsetTablePosition = sfxeStream->readUint32BE();
	sfxeRecord.rect.left = sfxeStream->readUint16BE();
	sfxeRecord.rect.top = sfxeStream->readUint16BE();
	sfxeRecord.rect.right = sfxeStream->readUint16BE();
	sfxeRecord.rect.bottom = sfxeStream->readUint16BE();
	sfxeRecord.speed = sfxeStream->readUint16BE();
	// Skip the rest of the fields...

	// Read in offsets
	sfxeStream->seek(offsetTablePosition);
	uint32 *frameOffsets = new uint32[sfxeRecord.frameCount];
	for (uint16 i = 0; i < sfxeRecord.frameCount; i++)
		frameOffsets[i] = sfxeStream->readUint32BE();
	sfxeStream->seek(frameOffsets[0]);

	// Read in the scripts
	for (uint16 i = 0; i < sfxeRecord.frameCount; i++)
		sfxeRecord.frameScripts.push_back(sfxeStream->readStream((i == sfxeRecord.frameCount - 1) ? sfxeStream->size() - frameOffsets[i] : frameOffsets[i + 1] - frameOffsets[i]));

	// Set it to the first frame
	sfxeRecord.curFrame = 0;
	sfxeRecord.lastFrameTime = 0;

	delete[] frameOffsets;
	delete sfxeStream;
	_waterEffects.push_back(sfxeRecord);
}

void RivenGraphics::clearWaterEffects() {
	_waterEffects.clear();
}

bool RivenGraphics::runScheduledWaterEffects() {
	// Don't run the effect if it's disabled
	if (_vm->_vars["waterenabled"] == 0)
		return false;

	Graphics::Surface *screen = NULL;

	for (uint16 i = 0; i < _waterEffects.size(); i++) {
		if (_vm->_system->getMillis() > _waterEffects[i].lastFrameTime + 1000 / _waterEffects[i].speed) {
			// Lock the screen!
			if (!screen)
				screen = _vm->_system->lockScreen();

			// Make sure the script is at the starting point
			Common::SeekableReadStream *script = _waterEffects[i].frameScripts[_waterEffects[i].curFrame];
			if (script->pos() != 0)
				script->seek(0);

			// Run script
			uint16 curRow = 0;
			for (uint16 op = script->readUint16BE(); op != 4; op = script->readUint16BE()) {
				if (op == 1) {        // Increment Row
					curRow++;
				} else if (op == 3) { // Copy Pixels
					uint16 dstLeft = script->readUint16BE();
					uint16 srcLeft = script->readUint16BE();
					uint16 srcTop = script->readUint16BE();
					uint16 rowWidth = script->readUint16BE();
					memcpy ((byte *)screen->getBasePtr(dstLeft, curRow + _waterEffects[i].rect.top), (byte *)_mainScreen->getBasePtr(srcLeft, srcTop), rowWidth * _pixelFormat.bytesPerPixel);
				} else if (op != 4) { // End of Script
					error ("Unknown SFXE opcode %d", op);
				}
			}

			// Increment frame
			_waterEffects[i].curFrame++;
			if (_waterEffects[i].curFrame == _waterEffects[i].frameCount)
				_waterEffects[i].curFrame = 0;

			// Set the new time
			_waterEffects[i].lastFrameTime = _vm->_system->getMillis();
		}
	}

	// Unlock the screen if it has been locked and return true to update the screen
	if (screen) {
		_vm->_system->unlockScreen();
		return true;
	}

	return false;
}

void RivenGraphics::scheduleTransition(uint16 id, Common::Rect rect) {
	_scheduledTransition = id;
	_transitionRect = rect;
}

void RivenGraphics::runScheduledTransition() {
	if (_scheduledTransition < 0) // No transition is scheduled
		return;

	// TODO: There's a lot to be done here...

	// Note: Transitions 0-11 are actual transitions, but none are used in-game.
	// There's no point in implementing them if they're not used. These extra
	// transitions were found by hacking scripts.

	switch (_scheduledTransition) {
	case 0:  // Swipe Left
	case 1:  // Swipe Right
	case 2:  // Swipe Up
	case 3:  // Swipe Down
	case 12: // Pan Left
	case 13: // Pan Right
	case 14: // Pan Up
	case 15: // Pan Down
	case 16: // Dissolve
	case 17: // Dissolve (tspit CARD 155)
		break;
	default:
		if (_scheduledTransition >= 4 && _scheduledTransition <= 11)
			error("Found unused transition %d", _scheduledTransition);
		else
			error("Found unknown transition %d", _scheduledTransition);
	}

	// For now, just copy the image to screen without doing any transition.
	_effectScreen->copyRectToSurface(*_mainScreen, 0, 0, Common::Rect(_mainScreen->w, _mainScreen->h));
	_vm->_system->copyRectToScreen(_effectScreen->getBasePtr(0, 0), _effectScreen->pitch, 0, 0, _effectScreen->w, _effectScreen->h);
	_vm->_system->updateScreen();

	_scheduledTransition = -1; // Clear scheduled transition
}

void RivenGraphics::clearMainScreen() {
	_mainScreen->fillRect(Common::Rect(0, 0, 608, 392), _pixelFormat.RGBToColor(0, 0, 0));
}

void RivenGraphics::fadeToBlack() {
	// The transition speed is forced to best here
	setTransitionSpeed(kRivenTransitionSpeedBest);
	scheduleTransition(16);
	clearMainScreen();
	runScheduledTransition();
}

void RivenGraphics::drawExtrasImageToScreen(uint16 id, const Common::Rect &rect) {
	MohawkSurface *mhkSurface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	mhkSurface->convertToTrueColor();
	Graphics::Surface *surface = mhkSurface->getSurface();

	_vm->_system->copyRectToScreen(surface->getPixels(), surface->pitch, rect.left, rect.top, surface->w, surface->h);

	delete mhkSurface;
}

void RivenGraphics::drawRect(Common::Rect rect, bool active) {
	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	Graphics::Surface *screen = _vm->_system->lockScreen();

	if (active)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else
		screen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));

	_vm->_system->unlockScreen();
}

void RivenGraphics::drawImageRect(uint16 id, Common::Rect srcRect, Common::Rect dstRect) {
	// Draw tBMP id from srcRect to dstRect
	Graphics::Surface *surface = findImage(id)->getSurface();

	assert(srcRect.width() == dstRect.width() && srcRect.height() == dstRect.height());

	for (uint16 i = 0; i < srcRect.height(); i++)
		memcpy(_mainScreen->getBasePtr(dstRect.left, i + dstRect.top), surface->getBasePtr(srcRect.left, i + srcRect.top), srcRect.width() * surface->format.bytesPerPixel);

	_dirtyScreen = true;
}

void RivenGraphics::drawExtrasImage(uint16 id, Common::Rect dstRect) {
	MohawkSurface *mhkSurface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, id));
	mhkSurface->convertToTrueColor();
	Graphics::Surface *surface = mhkSurface->getSurface();

	assert(dstRect.width() == surface->w);

	for (uint16 i = 0; i < surface->h; i++)
		memcpy(_mainScreen->getBasePtr(dstRect.left, i + dstRect.top), surface->getBasePtr(0, i), surface->pitch);

	delete mhkSurface;
	_dirtyScreen = true;
}

void RivenGraphics::beginCredits() {
	// Clear the old cache
	clearCache();

	// Now cache all the credits images
	for (uint16 i = 302; i <= 320; i++) {
		MohawkSurface *surface = _bitmapDecoder->decodeImage(_vm->getExtrasResource(ID_TBMP, i));
		surface->convertToTrueColor();
		addImageToCache(i, surface);
	}

	// And clear our screen too
	clearMainScreen();
}

void RivenGraphics::updateCredits() {
	if ((_creditsImage == 303 || _creditsImage == 304) && _creditsPos == 0)
		fadeToBlack();

	if (_creditsImage < 304) {
		// For the first two credit images, they are faded from black to the image and then out again
		scheduleTransition(16);

		Graphics::Surface *frame = findImage(_creditsImage++)->getSurface();

		for (int y = 0; y < frame->h; y++)
			memcpy(_mainScreen->getBasePtr(124, y), frame->getBasePtr(0, y), frame->pitch);

		runScheduledTransition();
	} else {
		// Otheriwse, we're scrolling
		// Move the screen up one row
		memmove(_mainScreen->getPixels(), _mainScreen->getBasePtr(0, 1), _mainScreen->pitch * (_mainScreen->h - 1));

		// Only update as long as we're not before the last frame
		// Otherwise, we're just moving up a row (which we already did)
		if (_creditsImage <= 320) {
			// Copy the next row to the bottom of the screen
			Graphics::Surface *frame = findImage(_creditsImage)->getSurface();
			memcpy(_mainScreen->getBasePtr(124, _mainScreen->h - 1), frame->getBasePtr(0, _creditsPos), frame->pitch);
			_creditsPos++;

			if (_creditsPos == _mainScreen->h) {
				_creditsImage++;
				_creditsPos = 0;
			}
		}

		// Now flush the new screen
		_vm->_system->copyRectToScreen(_mainScreen->getPixels(), _mainScreen->pitch, 0, 0, _mainScreen->w, _mainScreen->h);
		_vm->_system->updateScreen();
	}
}

void RivenGraphics::beginScreenUpdate() {
	_screenUpdateNesting++;
}

void RivenGraphics::applyScreenUpdate(bool force) {
	if (force) {
		_screenUpdateNesting = 0;
	} else {
		_screenUpdateNesting--;
	}

	// The screen is only updated when the outermost screen update ends
	if (_screenUpdateNesting <= 0 && !_screenUpdateRunning) {
		_screenUpdateRunning = true;

		_vm->getCard()->runScript(kCardUpdateScript);
		_vm->_sound->triggerDrawSound();
		updateScreen();

		_screenUpdateNesting = 0;
		_screenUpdateRunning = false;
	}
}

void RivenGraphics::setFliesEffect(uint16 count, bool fireflies) {
	delete _fliesEffect;
	_fliesEffect = new FliesEffect(_vm, count, fireflies);
}

void RivenGraphics::clearFliesEffect() {
	delete _fliesEffect;
	_fliesEffect = nullptr;
}

void RivenGraphics::runFliesEffect() {
	if (_fliesEffect) {
		_fliesEffect->update();
	}
}

Graphics::Surface *RivenGraphics::getBackScreen() {
	return _mainScreen;
}

Graphics::Surface *RivenGraphics::getEffectScreen() {
	return _effectScreen;
}

const FliesEffect::FliesEffectData FliesEffect::_firefliesParameters = {
		true,
		true,
		true,
		true,
		3.0,
		0.7,
		40,
		2.0,
		1.0,
		8447718,
		30,
		10
};

const FliesEffect::FliesEffectData FliesEffect::_fliesParameters = {
		false,
		false,
		false,
		true,
		8.0,
		3.0,
		80,
		3.0,
		1.0,
		661528,
		30,
		10
};

FliesEffect::FliesEffect(MohawkEngine_Riven *vm, uint16 count, bool fireflies) :
		_vm(vm) {

	_effectSurface = _vm->_gfx->getEffectScreen();
	_backSurface = _vm->_gfx->getBackScreen();
	_gameRect = Common::Rect(608, 392);

	if (fireflies) {
		_parameters = &_firefliesParameters;
	} else {
		_parameters = &_fliesParameters;
	}

	_updatePeriodMs = 66;
	_nextUpdateTime = _vm->_system->getMillis();

	initFlies(count);
}

FliesEffect::~FliesEffect() {

}

void FliesEffect::initFlies(uint16 count) {
	_fly.resize(count);
	for (uint16 i = 0; i < _fly.size(); i++) {
		initFlyRandomPosition(i);
	}
}

void FliesEffect::initFlyRandomPosition(uint index) {
	int posX = _vm->_rnd->getRandomNumber(_gameRect.right - 3);
	int posY = _vm->_rnd->getRandomNumber(_gameRect.bottom - 3);

	if (posY < 100) {
		posY = 100;
	}

	initFlyAtPosition(index, posX, posY, 15);
}

int FliesEffect::randomBetween(int min, int max) {
	return _vm->_rnd->getRandomNumber(max - min) + min;
}

void FliesEffect::initFlyAtPosition(uint index, int posX, int posY, int posZ) {
	FliesEffectEntry &fly = _fly[index];

	fly.posX = posX;
	fly.posXFloat = posX;
	fly.posY = posY;
	fly.posYFloat = posY;
	fly.posZ = posZ;
	fly.light = true;

	fly.framesTillLightSwitch = randomBetween(_parameters->minFramesLit, _parameters->minFramesLit + _parameters->maxLightDuration);

	fly.hasBlur = false;
	fly.directionAngleRad = randomBetween(0, 300) / 100.0f;
	fly.directionAngleRadZ = randomBetween(0, 300) / 100.0f;
	fly.speed = randomBetween(0, 100) / 100.0f;
}

void FliesEffect::update() {
	if (_nextUpdateTime <= _vm->_system->getMillis()) {
		_nextUpdateTime = _updatePeriodMs + _vm->_system->getMillis();

		updateFlies();
		draw();
		updateScreen();
	}
}

void FliesEffect::updateFlies() {
	for (uint i = 0; i < _fly.size(); i++) {
		updateFlyPosition(i);

		if (_fly[i].posX < 1 || _fly[i].posX > _gameRect.right - 4 || _fly[i].posY > _gameRect.bottom - 4) {
			initFlyRandomPosition(i);
		}

		if (_parameters->lightable) {
			_fly[i].framesTillLightSwitch--;

			if (_fly[i].framesTillLightSwitch <= 0) {
				_fly[i].light = !_fly[i].light;
				_fly[i].framesTillLightSwitch = randomBetween(_parameters->minFramesLit, _parameters->minFramesLit + _parameters->maxLightDuration);
				_fly[i].hasBlur = false;
			}
		}
	}
}

void FliesEffect::updateFlyPosition(uint index) {
	FliesEffectEntry &fly = _fly[index];

	if (fly.directionAngleRad > 2.0f * M_PI) {
		fly.directionAngleRad = fly.directionAngleRad - 2.0f * M_PI;
	}
	if (fly.directionAngleRad < 0.0f) {
		fly.directionAngleRad = fly.directionAngleRad + 2.0f * M_PI;
	}
	if (fly.directionAngleRadZ > 2.0f * M_PI) {
		fly.directionAngleRadZ = fly.directionAngleRadZ - 2.0f * M_PI;
	}
	if (fly.directionAngleRadZ < 0.0f) {
		fly.directionAngleRadZ = fly.directionAngleRadZ + 2.0f * M_PI;
	}
	fly.posXFloat += cos(fly.directionAngleRad) * fly.speed;
	fly.posYFloat += sin(fly.directionAngleRad) * fly.speed;
	fly.posX = fly.posXFloat;
	fly.posY = fly.posYFloat;
	selectAlphaMap(
			fly.posXFloat - fly.posX >= 0.5,
			fly.posYFloat - fly.posY >= 0.5,
			&fly.alphaMap,
			&fly.width,
			&fly.height);
	fly.posZFloat += cos(fly.directionAngleRadZ) * (fly.speed / 2.0f);
	fly.posZ = fly.posZFloat;
	if (_parameters->canBlur && fly.speed > _parameters->blurSpeedTreshold) {
		fly.hasBlur = true;
		float blurPosXFloat = cos(fly.directionAngleRad + M_PI) * _parameters->blurDistance + fly.posXFloat;
		float blurPosYFloat = sin(fly.directionAngleRad + M_PI) * _parameters->blurDistance + fly.posYFloat;

		fly.blurPosX = blurPosXFloat;
		fly.blurPosY = blurPosYFloat;
		selectAlphaMap(
				blurPosXFloat - fly.blurPosX >= 0.5,
				blurPosYFloat - fly.blurPosY >= 0.5,
				&fly.blurAlphaMap,
				&fly.blurWidth,
				&fly.blurHeight);
	}
	if (fly.posY >= 100) {
		int maxAngularSpeed = _parameters->maxAcceleration;
		if (fly.posZ > 15) {
			maxAngularSpeed /= 2;
		}
		int angularSpeed = randomBetween(-maxAngularSpeed, maxAngularSpeed);
		fly.directionAngleRad += angularSpeed / 100.0f;
	} else {
		// Make the flies go down if they are too high in the screen
		int angularSpeed = randomBetween(0, 50);
		if (fly.directionAngleRad >= M_PI / 2.0f && fly.directionAngleRad <= 3.0f * M_PI / 2.0f) {
			// Going down
			fly.directionAngleRad -= angularSpeed / 100.0f;
		} else {
			// Going up
			fly.directionAngleRad += angularSpeed / 100.0f;
		}
		if (fly.posY < 1) {
			initFlyRandomPosition(index);
		}
	}
	if (fly.posZ >= 0) {
		int distanceToScreenEdge;
		if (fly.posX / 10 >= (_gameRect.right - fly.posX) / 10) {
			distanceToScreenEdge = (_gameRect.right - fly.posX) / 10;
		} else {
			distanceToScreenEdge = fly.posX / 10;
		}
		if (distanceToScreenEdge > (_gameRect.bottom - fly.posY) / 10) {
			distanceToScreenEdge = (_gameRect.bottom - fly.posY) / 10;
		}
		if (distanceToScreenEdge > 30) {
			distanceToScreenEdge = 30;
		}
		if (fly.posZ <= distanceToScreenEdge) {
			fly.directionAngleRadZ += randomBetween(-_parameters->maxAcceleration, _parameters->maxAcceleration) / 100.0f;
		} else {
			fly.posZ = distanceToScreenEdge;
			fly.directionAngleRadZ += M_PI;
		}
	} else {
		fly.posZ = 0;
		fly.directionAngleRadZ += M_PI;
	}
	float minSpeed = _parameters->minSpeed - fly.posZ / 40.0f;
	float maxSpeed = _parameters->maxSpeed - fly.posZ / 20.0f;
	fly.speed += randomBetween(-_parameters->maxAcceleration, _parameters->maxAcceleration) / 100.0f;
	if (fly.speed > maxSpeed) {
		fly.speed -= randomBetween(0, 50) / 100.0f;
	}
	if (fly.speed < minSpeed) {
		fly.speed += randomBetween(0, 50) / 100.0f;
	}
}

void FliesEffect::selectAlphaMap(bool horGridOffset, bool vertGridoffset, const uint16 **alphaMap, uint *width, uint *height) {
	static const uint16 alpha1[12] = {
			 8, 16,  8,
			16, 32, 16,
			 8, 16,  8,
			 0,  0,  0
	};

	static const uint16 alpha2[12] = {
			4, 12, 12, 4,
			8, 24, 24, 8,
			4, 12, 12, 4
	};

	static const uint16 alpha3[12] = {
			 4,  8,  4,
			12, 24, 12,
			12, 24, 12,
			 4,  8,  4
	};

	static const uint16 alpha4[16] = {
			2,  6,  6, 2,
			6, 18, 18, 6,
			6, 18, 18, 6,
			2,  6,  6, 2
	};

	static const uint16 alpha5[12] = {
			4,  8, 4,
			8, 32, 8,
			4,  8, 4,
			0,  0, 0
	};

	static const uint16 alpha6[12] = {
			2,  6,  6, 2,
			4, 24, 24, 4,
			2,  6,  6, 2
	};

	static const uint16 alpha7[12] = {
			2,  4, 2,
			6, 24, 6,
			6, 24, 6,
			2,  4, 2
	};

	static const uint16 alpha8[16] = {
			1,  3,  3, 1,
			3, 18, 18, 3,
			3, 18, 18, 3,
			1,  3,  3, 1
	};

	struct AlphaMap {
		bool horizontalGridOffset;
		bool verticalGridOffset;
		bool isLarge;
		uint16 width;
		uint16 height;
		const uint16 *pixels;
	};

	static const AlphaMap alphaSelector[] = {
			{ true,  true,  true,  4, 4, alpha4 },
			{ true,  true,  false, 4, 4, alpha8 },
			{ true,  false, true,  4, 3, alpha2 },
			{ true,  false, false, 4, 3, alpha6 },
			{ false, true,  true,  3, 4, alpha3 },
			{ false, true,  false, 3, 4, alpha7 },
			{ false, false, true,  3, 3, alpha1 },
			{ false, false, false, 3, 3, alpha5 }
	};

	for (uint i = 0; i < ARRAYSIZE(alphaSelector); i++) {
		if (alphaSelector[i].horizontalGridOffset == horGridOffset
		    && alphaSelector[i].verticalGridOffset == vertGridoffset
		    && alphaSelector[i].isLarge == _parameters->isLarge) {
			*alphaMap = alphaSelector[i].pixels;
			*width = alphaSelector[i].width;
			*height = alphaSelector[i].height;
			return;
		}
	}

	error("Unknown flies alpha map case");
}

void FliesEffect::draw() {
	const Graphics::PixelFormat format = _effectSurface->format;

	for (uint i = 0; i < _fly.size(); i++) {
		FliesEffectEntry &fly = _fly[i];
		uint32 color = _parameters->color32;
		if (!fly.light) {
			color = _fliesParameters.color32;
		}

		bool hoveringBrightBackground = false;
		for (uint y = 0; y < fly.height; y++) {
			uint16 *pixel = (uint16 *) _effectSurface->getBasePtr(fly.posX, fly.posY + y);

			for (uint x = 0; x < fly.width; x++) {
				byte r, g, b;
				format.colorToRGB(*pixel, r, g, b);

				if (_parameters->unlightIfTooBright) {
					if (r >= 192 || g >= 192 || b >= 192) {
						hoveringBrightBackground = true;
					}
				}
				colorBlending(color, r, g, b, fly.alphaMap[fly.width * y + x] - fly.posZ);

				*pixel = format.RGBToColor(r, g, b);
				++pixel;
			}
		}

		Common::Rect drawRect = Common::Rect(fly.width, fly.height);
		drawRect.translate(fly.posX, fly.posY);
		addToScreenDirtyRects(drawRect);
		addToEffectsDirtyRects(drawRect);

		if (fly.hasBlur) {
			for (uint y = 0; y < fly.blurHeight; y++) {
				uint16 *pixel = (uint16 *) _effectSurface->getBasePtr(fly.blurPosX, fly.blurPosY + y);
				for (uint x = 0; x < fly.blurWidth; x++) {
					byte r, g, b;
					format.colorToRGB(*pixel, r, g, b);

					colorBlending(color, r, g, b, fly.blurAlphaMap[fly.blurWidth * y + x] - fly.posZ);

					*pixel = format.RGBToColor(r, g, b);
					++pixel;
				}
			}

			Common::Rect drawRect2 = Common::Rect(fly.blurWidth, fly.blurHeight);
			drawRect2.translate(fly.blurPosX, fly.blurPosY);
			addToScreenDirtyRects(drawRect2);
			addToEffectsDirtyRects(drawRect2);

			fly.hasBlur = false;
		}

		if (hoveringBrightBackground) {
			fly.hasBlur = false;
			if (_parameters->lightable) {
				fly.light = false;
				fly.framesTillLightSwitch = randomBetween(_parameters->minFramesLit, _parameters->minFramesLit + _parameters->maxLightDuration);
			}

			if (_vm->_rnd->getRandomBit()) {
				fly.directionAngleRad += M_PI / 2.0;
			} else {
				fly.directionAngleRad -= M_PI / 2.0;
			}
		}
	}
}

void FliesEffect::colorBlending(uint32 flyColor, byte &r, byte &g, byte &b, int alpha) {
	alpha = CLIP(alpha, 0, 32);
	byte flyR = (flyColor & 0x000000FF) >> 0;
	byte flyG = (flyColor & 0x0000FF00) >> 8;
	byte flyB = (flyColor & 0x00FF0000) >> 16;

	r = (32 * r + alpha * (flyR - r)) / 32;
	g = (32 * g + alpha * (flyG - g)) / 32;
	b = (32 * b + alpha * (flyB - b)) / 32;
}

void FliesEffect::updateScreen() {
	for (uint i = 0; i < _screenSurfaceDirtyRects.size(); i++) {
		const Common::Rect &rect = _screenSurfaceDirtyRects[i];
		_vm->_system->copyRectToScreen(_effectSurface->getBasePtr(rect.left, rect.top),
		                               _effectSurface->pitch, rect.left, rect.top,
		                               rect.width(), rect.height()
		);
	}
	_screenSurfaceDirtyRects.clear();

	restoreEffectsSurface();
}

void FliesEffect::addToScreenDirtyRects(const Common::Rect &rect) {
	for (uint i = 0; i < _screenSurfaceDirtyRects.size(); i++) {
		if (rect.intersects(_screenSurfaceDirtyRects[i])) {
			_screenSurfaceDirtyRects[i].extend(rect);
			return;
		}
	}

	_screenSurfaceDirtyRects.push_back(rect);
}

void FliesEffect::addToEffectsDirtyRects(const Common::Rect &rect) {
	for (uint i = 0; i < _effectsSurfaceDirtyRects.size(); i++) {
		if (rect.intersects(_effectsSurfaceDirtyRects[i])) {
			_effectsSurfaceDirtyRects[i].extend(rect);
			return;
		}
	}

	_effectsSurfaceDirtyRects.push_back(rect);
}

void FliesEffect::restoreEffectsSurface() {
	for (uint i = 0; i < _effectsSurfaceDirtyRects.size(); i++) {
		const Common::Rect &rect = _effectsSurfaceDirtyRects[i];
		_effectSurface->copyRectToSurface(*_backSurface, rect.left, rect.top, rect);
		addToScreenDirtyRects(rect);
	}

	_effectsSurfaceDirtyRects.clear();
}

} // End of namespace Mohawk
