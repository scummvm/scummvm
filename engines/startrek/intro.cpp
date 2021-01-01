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

#include "startrek/resource.h"
#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::playIntro() {
	// TODO: .MT audio file

	_frameIndex = 0;

	initStarfieldPosition();
	initStarfield(10, 20, 309, 169, 128);

	initStarfieldSprite(&_starfieldSprite, new StubBitmap(0, 0), _starfieldRect);

	//delR3(&_enterpriseR3); // TODO: uncomment

	R3 planetR3 = R3();
	planetR3.matrix = initMatrix();
	planetR3.field1e = 3;
	planetR3.funcPtr1 = 0;
	planetR3.funcPtr2 = 0;

	_gfx->clearScreenAndPriBuffer();
	_gfx->fadeoutScreen();
	_gfx->loadPalette("gold");
	_gfx->setBackgroundImage("goldlogo");
	_sound->playVoc("logo");
	_gfx->copyBackgroundScreen();
	_system->updateScreen();
	_system->delayMillis(10);
	_gfx->fadeinScreen();

	uint32 clockTicks = _clockTicks;

	Sprite subtitleSprite;
	_gfx->addSprite(&subtitleSprite);
	subtitleSprite.setXYAndPriority(0, 0, 12);
	subtitleSprite.setBitmap(_resource->loadBitmapFile("blank"));
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
		case 0: // Starfield is shown
			starfieldZoomSpeed = 10;
			_sound->playMidiMusicTracks(MIDITRACK_0, -1);
			_byte_45b3c = 0;
			break;

		case 30: // Kirk starts narrating
			_sound->playVoc("kirkintr");
			loadSubtitleSprite(0, &subtitleSprite); // "Space..."
			break;

		case 36:
			loadSubtitleSprite(1, &subtitleSprite); // "The final frontier"
			break;

		case 42: // Enterprise moves toward camera
			loadSubtitleSprite(-1, &subtitleSprite);
			addR3(&_enterpriseR3);
			_enterpriseR3.bitmap = nullptr;
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
			planetR3.bitmap = new Bitmap(_resource->loadFile("planet.shp"));
			initIntroR3ObjectToMove(&planetR3, 6, 10000, 6, 10000, 0);
			addR3(&planetR3);
			initIntroR3ObjectToMove(&_enterpriseR3, -15, 250, 15, 500, 18);
			starfieldZoomSpeed = 0;
			break;

		case 90:
			loadSubtitleSprite(3, &subtitleSprite);
			initIntroR3ObjectToMove(&planetR3, 0, 5000, 0, 5000, 0);
			planetR3.pos.y = -2500;
			planetR3.pos.z = -1;
			initIntroR3ObjectToMove(&_enterpriseR3, -30, 450, 45, 540, 54);
			break;

		case 108:
			loadSubtitleSprite(4, &subtitleSprite);
			break;

		case 126:
			loadSubtitleSprite(5, &subtitleSprite);
			break;

		case 144:
			loadSubtitleSprite(6, &subtitleSprite);
			initIntroR3ObjectToMove(&planetR3, 30, 5600, 30, 5600, 0);
			planetR3.pos.y = -2500;
			planetR3.pos.z = -1;
			initIntroR3ObjectToMove(&_enterpriseR3, -10, 800, 155, 600, 18);
			break;

		case 150:
			loadSubtitleSprite(6, &subtitleSprite);
			break;

		case 162:
			delR3(&planetR3);
			break;

		case 168:
			loadSubtitleSprite(-1, &subtitleSprite);
			initIntroR3ObjectToMove(&_enterpriseR3, 340, 5000, 0, 0, 18);
			break;

		case 186:
			delR3(&_enterpriseR3);
			showCreditsScreen(&planetR3, 0, false);
			initIntroR3ObjectToMove(&planetR3, 0, 0, 0, _starfieldPointDivisor, 18);
			// TODO: the rest
			break;

		case 204:
			// TODO
			break;

		case 216:
			// TODO
			break;

		case 222:
			delR3(&planetR3);
			break;

		case 228:
			showCreditsScreen(&planetR3, 1, false);
			break;

		case 240:
			showCreditsScreen(&planetR3, 2);
			break;

		case 252:
			showCreditsScreen(&planetR3, 3);
			break;

		case 264:
			showCreditsScreen(&planetR3, 4);
			break;

		case 276:
			showCreditsScreen(&planetR3, 5);
			break;

		case 288:
			showCreditsScreen(&planetR3, 6);
			break;

		case 300:
			showCreditsScreen(&planetR3, 7);
			break;

		case 312:
			showCreditsScreen(&planetR3, 8);
			break;

		case 324:
			showCreditsScreen(&planetR3, 9);
			break;

		case 330:
			showCreditsScreen(&planetR3, 10);
			break;

		case 336:
			showCreditsScreen(&planetR3, 11);
			break;

		case 342:
			showCreditsScreen(&planetR3, 12);
			break;

		case 348:
			showCreditsScreen(&planetR3, 13);
			break;

		case 354:
			showCreditsScreen(&planetR3, 14);
			break;

		case 360:
			showCreditsScreen(&planetR3, 15);
			break;

		case 366:
			if (getFeatures() & GF_CDROM)
				delR3(&planetR3);
			break;

		case 378:
			_gfx->delSprite(&subtitleSprite);
			_byte_45b3c = 1;
			break;

		default:
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

void StarTrekEngine::showCreditsScreen(R3 *creditsBuffer, int index, bool deletePrevious) {
	if (!(getFeatures() & GF_CDROM) && index > 0)
		return;

	if (deletePrevious) {
		delR3(creditsBuffer);
	}

	creditsBuffer->bitmap = new Bitmap(_resource->loadFile(Common::String::format("credit%02d.shp", index)));
	creditsBuffer->field1e = 3;
	creditsBuffer->field22 = 1;
	creditsBuffer->field24 = 1;
	addR3(creditsBuffer);
}

void StarTrekEngine::initIntroR3ObjectToMove(R3 *r3, int16 srcAngle, int16 srcDepth, int16 destAngle, int16 destDepth, int16 ticks) {
	int32 srcAngleX = (int32)floor((double)_sineTable.at(srcAngle / 90) * srcDepth);
	int32 srcAngleZ = (int32)floor((double)_cosineTable.at(srcAngle / 90) * srcDepth);
	r3->pos = Point3(srcAngleX + _starfieldPosition.x, 0, srcAngleZ + _starfieldPosition.z);

	int32 destAngleX = (int32)floor((double)_sineTable.at(destAngle / 90) * destDepth);
	int32 destAngleZ = (int32)floor((double)_cosineTable.at(destAngle / 90) * destDepth);
	int32 deltaX = destAngleX + _starfieldPosition.x - r3->pos.x;
	int32 deltaZ = destAngleZ + _starfieldPosition.z - r3->pos.z;
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
			sprite->setBitmap(_resource->loadBitmapFile("blank"));
		else {
			Common::String file = Common::String::format("tittxt%02d", index);
			sprite->setBitmap(_resource->loadBitmapFile(file));
		}
	}
}

} // End of namespace StarTrek
