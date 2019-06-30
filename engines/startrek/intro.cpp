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

#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::playIntro() {
	// TODO: .MT audio file

	initStarfieldPosition();
	initStarfield(10, 20, 309, 169, 128);

	SharedPtr<Bitmap> fakeStarfieldBitmap(new StubBitmap(0, 0));
	_starfieldSprite.bitmap = fakeStarfieldBitmap;
	initStarfieldSprite(&_starfieldSprite, fakeStarfieldBitmap, _starfieldRect);

	//delR3(&_enterpriseR3); // TODO: uncomment

	R3 planetR3 = R3();
	planetR3.matrix = initMatrix();
	planetR3.field1e = 3;
	planetR3.funcPtr1 = 0;
	planetR3.funcPtr2 = 0;
	planetR3.bitmapOffset = 0;

	_gfx->clearScreenAndPriBuffer();
	_gfx->fadeoutScreen();
	_gfx->loadPalette("gold");
	_gfx->setBackgroundImage(_gfx->loadBitmap("goldlogo"));
	_sound->playVoc("logo");
	_gfx->copyBackgroundScreen();
	_system->updateScreen();
	_system->delayMillis(10);
	_gfx->fadeinScreen();

	uint32 clockTicks = _clockTicks;

	Sprite subtitleSprite;
	_gfx->addSprite(&subtitleSprite);
	subtitleSprite.setXYAndPriority(0, 0, 12);
	subtitleSprite.bitmap = _gfx->loadBitmap("blank");
	subtitleSprite.drawPriority2 = 16;

	int index = 12;
	while (index >= 0) {
		Common::String file = Common::String::format("credit%02d.shp", index);
		// TODO: This loads the file, but does not do anything with the resulting data, so
		// this is just for caching it?
		// Remember to deal with similar commented function calls below, too.
		//loadFileWithParams(file, false, true, false);
		index -= 1;
	}

	//loadFileWithParams("legal.bmp", false, true, false);

	index = 6;
	while (index >= 0) {
		Common::String file = Common::String::format("tittxt%02d.bmp", index);
		//loadFileWithParams(file, false, true, false);
		index -= 1;
	}

	//loadFileWithParams("planet.shp", false, true, false);

	index = 6;
	while (index >= 0) {
		Common::String file = Common::String::format("ent%d3.r3s", index);
		//loadFileWithParams(file, false, true, false);
		index -= 1;
	}

	// TODO: kirkintr

	clockTicks += 540;

	while (_clockTicks < clockTicks && _sound->isMidiPlaying()) {
		waitForNextTick(true);
	}

	// TODO: MT audio file

	_gfx->fadeoutScreen();
	_gfx->loadPalette("bridge");
	_gfx->clearScreenAndPriBuffer();
	_sound->loadMusicFile("title");
	clockTicks = _clockTicks;

	int32 starfieldZoomSpeed = 0;
	int16 frame = 0;
	bool buttonPressed = false;

	while (frame != 0x180 || (_sound->isMidiPlaying() && !buttonPressed)) {
		if (!buttonPressed) {
			TrekEvent event;
			while (popNextEvent(&event, false)) {
				if (event.type == TREKEVENT_KEYDOWN) {
					_gfx->fadeoutScreen();
					buttonPressed = true;
				} else if (event.type == TREKEVENT_TICK)
					break;
			}
		}

		switch (frame) {
		case 0:
			starfieldZoomSpeed = 10;
			playMidiMusicTracks(MIDITRACK_0, -1);
			_byte_45b3c = 0;
			break;

		case 30:
			_sound->playVoc("kirkintr");
			loadSubtitleSprite(0, &subtitleSprite);
			break;

		case 36:
			loadSubtitleSprite(1, &subtitleSprite);
			break;

		case 42: // Enterprise moves toward camera
			loadSubtitleSprite(-1, &subtitleSprite);
			addR3(&_enterpriseR3);
			_enterpriseR3.field1e = 2;
			initIntroR3ObjectToMove(&_enterpriseR3, 330, 5000, 0, 0, 18);
			break;

		case 60: // Enterprise moves away from camera
			initIntroR3ObjectToMove(&_enterpriseR3, 0, 0, 30, 5000, 6);
			break;

		case 66: // Cut to scene with planet
			loadSubtitleSprite(2, &subtitleSprite);
			planetR3.field22 = 2000;
			planetR3.field24 = 10000 / _starfieldPointDivisor;
			planetR3.shpFile = SharedPtr<Common::MemoryReadStreamEndian>(loadFile("planet.shp"));
			initIntroR3ObjectToMove(&planetR3, 6, 10000, 6, 10000, 0);
			addR3(&planetR3);
			initIntroR3ObjectToMove(&_enterpriseR3, -15, 250, 15, 500, 18);
			starfieldZoomSpeed = 0;
			break;

		case 186:
			delR3(&_enterpriseR3);
			// TODO: the rest
			break;

		case 366:
			planetR3.shpFile.reset();
			delR3(&planetR3);
			break;

		case 378:
			_gfx->delSprite(&subtitleSprite);
			_byte_45b3c = 1;
			break;
		}

		if (!buttonPressed) {
			updateStarfieldAndShips(false);
			_gfx->drawAllSprites();
			_gfx->incPaletteFadeLevel();
			clockTicks += 3;

			while (_clockTicks < clockTicks)
				waitForNextTick();
		}

		_starfieldPosition.z += starfieldZoomSpeed;

		frame++;
		if (frame >= 0x186)
			frame = 0x186;
	}

	_gfx->fadeoutScreen();
	_gfx->delSprite(&_starfieldSprite);
	// TODO: the rest
}

void StarTrekEngine::initIntroR3ObjectToMove(R3 *r3, int16 srcAngle, int16 srcDepth, int16 destAngle, int16 destDepth, int16 ticks) {
	Fixed8 a1 = Fixed8::fromRaw((srcAngle << 8) / 90);
	Fixed8 a2 = Fixed8::fromRaw((destAngle << 8) / 90);

	r3->pos.x = sin(a1).multToInt(srcDepth) + _starfieldPosition.x;
	r3->pos.z = cos(a1).multToInt(srcDepth) + _starfieldPosition.z;
	r3->pos.y = 0;

	int32 deltaX = sin(a2).multToInt(destDepth) + _starfieldPosition.x - r3->pos.x;
	int32 deltaZ = cos(a2).multToInt(destDepth) + _starfieldPosition.z - r3->pos.z;
	debug("Z: %d, %d", r3->pos.z - _starfieldPosition.z, cos(a2).multToInt(destDepth));

	Angle angle = atan2(deltaX, deltaZ);
	r3->matrix = initSpeedMatrixForXZMovement(angle, initMatrix());

	debugCN(5, kDebugSpace, "initIntroR3ObjectToMove: pos %x,%x,%x; ", r3->pos.x, r3->pos.y, r3->pos.z);

	if (ticks != 0) {
		debugC(5, kDebugSpace, "speed %x,%x,%x\n", r3->speed.x, r3->speed.y, r3->speed.z);
		r3->speed.x = deltaX / ticks;
		r3->speed.z = deltaZ / ticks;
		r3->speed.y = 0;
	} else {
		debugC(5, kDebugSpace, "speed 0\n");
		r3->speed.x = 0;
		r3->speed.z = 0;
		r3->speed.y = 0;
	}
}

void StarTrekEngine::loadSubtitleSprite(int index, Sprite *sprite) {
	if (_showSubtitles) {
		if (index == -1)
			sprite->setBitmap(_gfx->loadBitmap("blank"));
		else {
			Common::String file = Common::String::format("tittxt%02d", index);
			sprite->setBitmap(_gfx->loadBitmap(file));
		}
	}
}

} // End of namespace StarTrek
