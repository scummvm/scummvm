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
#include "backends/keymapper/keymapper.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "image/png.h"

#include "tot/anims.h"
#include "tot/debug.h"
#include "tot/statics.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

void TotEngine::drawText(uint number) {
	readTextFile();
	uint foo = 0;
	sayLine(number, 255, 0, foo, false);
	_verbFile.close();
}

void TotEngine::displayLoading() {
	const char *msg = getFullScreenMessagesByCurrentLanguage()[58];

	setRGBPalette(255, 63, 63, 63);
	littText(121, 72, msg, 0);
	_screen->update();
	delay(kEnforcedTextAnimDelay);
	littText(120, 71, msg, 0);
	_screen->update();
	delay(kEnforcedTextAnimDelay);
	littText(119, 72, msg, 0);
	_screen->update();
	delay(kEnforcedTextAnimDelay);
	littText(120, 73, msg, 0);
	_screen->update();
	delay(kEnforcedTextAnimDelay);
	littText(120, 72, msg, 255);
	_screen->update();

	// enforce a delay for now so it's visible
	g_system->delayMillis(200);
}

void TotEngine::runaroundRed() {
	const uint devilTrajectory[91][2] = {
	 		 {204,  47}, {204,  49}, {203,  51}, {203,  53}, {201,  55}, {199, 57}, {197,  59}, {195,  61},
			 {193,  63}, {192,  65}, {192,  67}, {192,  69}, {192,  71}, {192, 73}, {191,  75}, {191,  77},
			 {191,  79}, {191,  81}, {191,  83}, {191,  85}, {191,  87}, {192, 89}, {192,  91}, {192,  93},
			 {192,  95}, {193,  97}, {194,  99}, {195, 101}, {196, 103}, {197, 105}, {198, 107}, {199, 109},
			 {200, 111}, {201, 111}, {203, 111}, {205, 111}, {207, 111}, {209, 111}, {211, 111}, {213, 111},
			 {215, 110}, {217, 110}, {219, 110}, {221, 110}, {223, 110}, {225, 110}, {227, 110}, {229, 110},
			 {231, 110}, {232, 109}, {234, 108}, {236, 107}, {238, 106}, {240, 105}, {242, 104}, {244, 103},
			 {246, 102}, {248, 101}, {250, 100}, {252,  99}, {254,  98}, {256,  97}, {258,  96}, {260, 95},
			 {261,  93}, {262,  91}, {263,  89}, {264,  87}, {265,  85}, {266,  83}, {267,  81}, {268, 79},
			 {269,  77}, {270,  75}, {271,  73}, {272,  71}, {273,  69}, {274,  67}, {275,  65}, {276, 63},
			 {277,  61}, {278,  59}, {279,  57}, {280,  55}, {281,  53}, {282,  52}, {282,  52}, {282, 52},
			 {282,  52}, {282,  52}, {282,  52}
	};


	bool exitLoop = false;
	loadDevil();
	uint secTrajIndex = 0;
	uint secTrajLength = 82;
	_iframe2 = 0;
	_isSecondaryAnimationEnabled = true;
	do {
		_chrono->updateChrono();
		if (_chrono->_gameTick) {
			if (secTrajIndex == secTrajLength)
				exitLoop = true;
			secTrajIndex += 1;
			if (_iframe2 >= _secondaryAnimationFrameCount - 1)
				_iframe2 = 0;
			else
				_iframe2++;
			_secondaryAnimation.posx = devilTrajectory[secTrajIndex][0] - 15;
			_secondaryAnimation.posy = devilTrajectory[secTrajIndex][1] - 42;
			if (secTrajIndex <= 8) {
				_secondaryAnimation.dir = 2;
				_secondaryAnimation.depth = 1;
			} else if (secTrajIndex <= 33) {
				_secondaryAnimation.dir = 2;
				_secondaryAnimation.depth = 14;
			} else if (secTrajIndex <= 63) {
				_secondaryAnimation.dir = 1;
				_secondaryAnimation.depth = 14;
			} else {
				_secondaryAnimation.dir = 0;
				_secondaryAnimation.depth = 3;
			}

			_chrono->_gameTickHalfSpeed = true;
			sprites(false);
			_chrono->_gameTick = false;
			_graphics->advancePaletteAnim();
			_screen->update();
		}
	} while (!exitLoop && !shouldQuit());
	clearAnimation();
	_graphics->restoreBackground();
	assembleScreen();
	_graphics->drawScreen(_sceneBackground);
}

void updateMovementGrids() {
	uint j1arm, j2arm;
	byte i1arm, i2arm;

	j1arm = (g_engine->_currentRoomData->secondaryAnimTrajectory[g_engine->_currentSecondaryTrajectoryIndex - 1].x / kXGridCount) + 1;
	j2arm = (g_engine->_currentRoomData->secondaryAnimTrajectory[g_engine->_currentSecondaryTrajectoryIndex - 1].y / kYGridCount) + 1;
	if ((g_engine->_oldposx != j1arm) || (g_engine->_oldposy != j2arm)) {

		for (i1arm = 0; i1arm < g_engine->_maxXGrid; i1arm++)
			for (i2arm = 0; i2arm < g_engine->_maxYGrid; i2arm++) {
				g_engine->_currentRoomData->walkAreasGrid[g_engine->_oldposx + i1arm][g_engine->_oldposy + i2arm] = g_engine->_movementGridForSecondaryAnim[i1arm][i2arm];
				g_engine->_currentRoomData->mouseGrid[g_engine->_oldposx + i1arm][g_engine->_oldposy + i2arm] = g_engine->_mouseGridForSecondaryAnim[i1arm][i2arm];
			}

		for (i1arm = 0; i1arm < g_engine->_maxXGrid; i1arm++)
			for (i2arm = 0; i2arm < g_engine->_maxYGrid; i2arm++) {
				g_engine->_movementGridForSecondaryAnim[i1arm][i2arm] = g_engine->_currentRoomData->walkAreasGrid[j1arm + i1arm][j2arm + i2arm];
				if (g_engine->_maskGridSecondaryAnim[i1arm][i2arm] > 0)
					g_engine->_currentRoomData->walkAreasGrid[j1arm + i1arm][j2arm + i2arm] = g_engine->_maskGridSecondaryAnim[i1arm][i2arm];

				g_engine->_mouseGridForSecondaryAnim[i1arm][i2arm] = g_engine->_currentRoomData->mouseGrid[j1arm + i1arm][j2arm + i2arm];
				if (g_engine->_maskMouseSecondaryAnim[i1arm][i2arm] > 0)
					g_engine->_currentRoomData->mouseGrid[j1arm + i1arm][j2arm + i2arm] = g_engine->_maskMouseSecondaryAnim[i1arm][i2arm];
			}

		g_engine->_oldposx = j1arm;
		g_engine->_oldposy = j2arm;
		g_engine->_oldGridX = 0;
		g_engine->_oldGridY = 0;
	}
}

void sprites(bool drawCharacter);

static void assembleBackground() {
	uint posabs;

	// copies the entire clean background in backgroundCopy back into background
	g_engine->_graphics->restoreBackground();

	posabs = 4 + g_engine->_dirtyMainSpriteY * 320 + g_engine->_dirtyMainSpriteX;
	uint16 w, h;
	w = READ_LE_UINT16(g_engine->_characterDirtyRect);
	h = READ_LE_UINT16(g_engine->_characterDirtyRect + 2);
	w++;
	h++;

	g_engine->_dirtyMainSpriteX2 = g_engine->_dirtyMainSpriteX + w;
	g_engine->_dirtyMainSpriteY2 = g_engine->_dirtyMainSpriteY + h;

	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			int pos = posabs + j * 320 + i;
			int destPos = 4 + (j * w + i);
			g_engine->_characterDirtyRect[destPos] = g_engine->_sceneBackground[pos];
		}
	}
}

/**
 * calculates the overlapping area between the source image and the background,
 * then "blits" (copies) the pixels from the image to the overlapping area of the background, respecting transparency.
 */
static void assembleImage(const byte *img, uint imgPosX, uint imgPosY) {

	uint x, // starting point of the overlap
		y;
	uint incx, // width and height of the overlapping area
		incy;

	uint16 wImg = READ_LE_UINT16(img) + 1;
	uint16 hImg = READ_LE_UINT16(img + 2) + 1;

	uint16 wBg = READ_LE_UINT16(g_engine->_characterDirtyRect) + 1;
	uint16 hBg = READ_LE_UINT16(g_engine->_characterDirtyRect + 2) + 1;

	// This region calculates the overlapping area of (x, incx, y, incy)
	{
		if (imgPosX < g_engine->_dirtyMainSpriteX)
			x = g_engine->_dirtyMainSpriteX;
		else
			x = imgPosX;

		if (imgPosX + wImg < g_engine->_dirtyMainSpriteX + wBg)
			incx = imgPosX + wImg - x;
		else
			incx = g_engine->_dirtyMainSpriteX + wBg - x;

		if (imgPosY < g_engine->_dirtyMainSpriteY)
			y = g_engine->_dirtyMainSpriteY;
		else
			y = imgPosY;

		if (imgPosY + hImg < g_engine->_dirtyMainSpriteY + hBg)
			incy = imgPosY + hImg - y;
		else
			incy = g_engine->_dirtyMainSpriteY + hBg - y;
	} // end of region calculating overlapping area

	for (uint j = 0; j < incy; j++) {
		for (uint i = 0; i < incx; i++) {
			int bgOffset = 4 + ((y - g_engine->_dirtyMainSpriteY) + j) * wBg + i + (x - g_engine->_dirtyMainSpriteX);
			int imgOffset = 4 + (y - imgPosY + j) * wImg + i + (x - imgPosX);
			if (img[imgOffset] != 0) {
				g_engine->_characterDirtyRect[bgOffset] = img[imgOffset];
			}
		}
	}
}

static void overlayLayers() {
	if (g_engine->_screenLayers[g_engine->_curDepth] != nullptr) {
		if (
			(g_engine->_depthMap[g_engine->_curDepth].posx <= g_engine->_dirtyMainSpriteX2) &&
			(g_engine->_depthMap[g_engine->_curDepth].posx2 > g_engine->_dirtyMainSpriteX) &&
			(g_engine->_depthMap[g_engine->_curDepth].posy < g_engine->_dirtyMainSpriteY2) &&
			(g_engine->_depthMap[g_engine->_curDepth].posy2 > g_engine->_dirtyMainSpriteY)) {
			assembleImage(g_engine->_screenLayers[g_engine->_curDepth], g_engine->_depthMap[g_engine->_curDepth].posx, g_engine->_depthMap[g_engine->_curDepth].posy);
		}
	}
}

void drawMainCharacter() {

	uint16 tempW;
	uint16 tempH;
	tempW = READ_LE_UINT16(g_engine->_curCharacterAnimationFrame);
	tempH = READ_LE_UINT16(g_engine->_curCharacterAnimationFrame + 2);
	tempW += 6;
	tempH += 6;

	if (g_engine->_dirtyMainSpriteY + tempH > 140) {
		tempH -= (g_engine->_dirtyMainSpriteY + tempH) - 140;
	}

	if (g_engine->_dirtyMainSpriteX + tempW > 320) {
		tempW -= (g_engine->_dirtyMainSpriteX + tempW) - 320;
	}

	g_engine->_characterDirtyRect = (byte *)malloc((tempW + 1) * (tempH + 1) + 4);

	WRITE_LE_UINT16(g_engine->_characterDirtyRect, tempW);
	WRITE_LE_UINT16(g_engine->_characterDirtyRect + 2, tempH);

	assembleBackground();
	g_engine->_curDepth = 0;
	while (g_engine->_curDepth != kDepthLevelCount) {
		overlayLayers();
		if (g_engine->_mainCharAnimation.depth == g_engine->_curDepth)
			assembleImage(g_engine->_curCharacterAnimationFrame, g_engine->_characterPosX, g_engine->_characterPosY);
		g_engine->_curDepth += 1;
	}

	g_engine->_graphics->putImg(g_engine->_dirtyMainSpriteX, g_engine->_dirtyMainSpriteY, g_engine->_characterDirtyRect);

	free(g_engine->_characterDirtyRect);
}

void TotEngine::sprites(bool drawMainCharachter) {
	// grabs the current frame from the walk cycle
	_curCharacterAnimationFrame = _mainCharAnimation.bitmap[_charFacingDirection][_iframe];

	_dirtyMainSpriteX = _characterPosX - 3;
	_dirtyMainSpriteY = _characterPosY - 3;
	if (_isSecondaryAnimationEnabled) {
		if (_currentRoomData->secondaryTrajectoryLength > 1) {
			updateMovementGrids();
		}
		if (_chrono->_gameTickHalfSpeed) {
			if (_isPeterCoughing && !_sound->isVocPlaying()) {
				_iframe2 = 0;
			}
			newSecondaryAnimationFrame();
			Common::copy(_secondaryAnimation.bitmap[_secondaryAnimation.dir][_iframe2], _secondaryAnimation.bitmap[_secondaryAnimation.dir][_iframe2] + _secondaryAnimFrameSize,  _curSecondaryAnimationFrame);
		}
		uint16 curCharFrameW = READ_LE_UINT16(_curCharacterAnimationFrame);
		uint16 curCharFrameH = READ_LE_UINT16(_curCharacterAnimationFrame + 2);

		uint16 secAnimW = READ_LE_UINT16(_curSecondaryAnimationFrame);
		uint16 secAnimH = READ_LE_UINT16(_curSecondaryAnimationFrame + 2);

		if (
			((_secondaryAnimation.posx < (_characterPosX + curCharFrameW) + 4) &&
			 ((_secondaryAnimation.posx + secAnimW + 1) > _dirtyMainSpriteX) &&
			 (_secondaryAnimation.posy < (_characterPosY + curCharFrameH + 4))) &&
			((_secondaryAnimation.posy + secAnimH + 1) > _dirtyMainSpriteY)) { // Character is in the area of the animation

			if (_secondaryAnimation.posx < _characterPosX) {
				_dirtyMainSpriteX = _secondaryAnimation.posx - 3;
			}
			if (_secondaryAnimation.posy < _characterPosY) {
				_dirtyMainSpriteY = _secondaryAnimation.posy - 3;
			}

			uint16 patchW = secAnimW + curCharFrameW + 6;

			uint16 patchH;
			if ((curCharFrameH + _characterPosY) > (_secondaryAnimation.posy + secAnimH)) {
				patchH = curCharFrameH + 6 + abs(_characterPosY - _secondaryAnimation.posy);
			} else {
				patchH = secAnimH + 6 + abs(_characterPosY - _secondaryAnimation.posy);
			}

			if (_dirtyMainSpriteY + patchH > 140) {
				patchH -= (_dirtyMainSpriteY + patchH) - 140 + 1;
			}

			if (_dirtyMainSpriteX + patchW > 320) {
				patchW -= (_dirtyMainSpriteX + patchW) - 320 + 1;
			}

			_characterDirtyRect = (byte *)malloc((patchW + 1) * (patchH + 1) + 4);

			WRITE_LE_UINT16(_characterDirtyRect, patchW);
			WRITE_LE_UINT16(_characterDirtyRect + 2, patchH);

			assembleBackground();
			_curDepth = 0;
			while (_curDepth != kDepthLevelCount) {
				overlayLayers();
				if (_secondaryAnimation.depth == _curDepth)
					assembleImage(_curSecondaryAnimationFrame, _secondaryAnimation.posx, _secondaryAnimation.posy);
				if (_mainCharAnimation.depth == _curDepth)
					assembleImage(_curCharacterAnimationFrame, _characterPosX, _characterPosY);
				_curDepth += 1;
			}
			_graphics->putImg(_dirtyMainSpriteX, _dirtyMainSpriteY, _characterDirtyRect);
			free(_characterDirtyRect);
		} else { // character and animation are in different parts of the screen

			if (drawMainCharachter) {
				drawMainCharacter();
			}

			_dirtyMainSpriteX = _secondaryAnimation.posx - 3;
			_dirtyMainSpriteY = _secondaryAnimation.posy - 3;

			secAnimW = READ_LE_UINT16(_curSecondaryAnimationFrame) + 6;
			secAnimH = READ_LE_UINT16(_curSecondaryAnimationFrame + 2) + 6;

			if (_dirtyMainSpriteY + secAnimH > 140) {
				secAnimH -= (_dirtyMainSpriteY + secAnimH) - 140 + 1;
			}

			if (_dirtyMainSpriteX + secAnimW > 320) {
				secAnimW -= (_dirtyMainSpriteX + secAnimW) - 320 + 1;
			}

			_characterDirtyRect = (byte *)malloc((secAnimW + 1) * (secAnimH + 1) + 4);
			WRITE_LE_UINT16(_characterDirtyRect, secAnimW);
			WRITE_LE_UINT16(_characterDirtyRect + 2, secAnimH);

			assembleBackground();
			_curDepth = 0;
			while (_curDepth != kDepthLevelCount) {
				overlayLayers();
				if (_secondaryAnimation.depth == _curDepth)
					assembleImage(_curSecondaryAnimationFrame, _secondaryAnimation.posx, _secondaryAnimation.posy);
				_curDepth += 1;
			}
			_graphics->putImg(_dirtyMainSpriteX, _dirtyMainSpriteY, _characterDirtyRect);
			free(_characterDirtyRect);
		}
	} else if (drawMainCharachter) {
		drawMainCharacter();
	}
}

void TotEngine::adjustKey() {
	_iframe = 0;
	calculateTrajectory(129, 13);
	_charFacingDirection = 0;
	do {
		_characterPosX = _trajectory[_currentTrajectoryIndex].x;
		_characterPosY = _trajectory[_currentTrajectoryIndex].y;
		_iframe++;
		_currentTrajectoryIndex += 1;
		emptyLoop();
		_chrono->_gameTick = false;
		_graphics->advancePaletteAnim();
		sprites(true);
	} while (_currentTrajectoryIndex != _trajectoryLength);
	_charFacingDirection = 3;
	emptyLoop();
	_chrono->_gameTick = false;
	sprites(true);
}

void TotEngine::adjustKey2() {
	_iframe = 0;
	_charFacingDirection = 2;
	_trajectoryLength = 1;
	do {
		_characterPosX = _trajectory[_currentTrajectoryIndex].x;
		_characterPosY = _trajectory[_currentTrajectoryIndex].y;
		_iframe++;
		_currentTrajectoryIndex -= 1;
		emptyLoop();
		_chrono->_gameTick = false;
		_graphics->advancePaletteAnim();
		sprites(true);
	} while (_currentTrajectoryIndex != 0);
	emptyLoop();
	_chrono->_gameTick = false;
	sprites(true);
}

void TotEngine::animatedSequence(uint numSequence) {
	Common::File animationFile;
	uint repIndex, animIndex;
	uint16 animFrameSize;
	uint animX, animY;
	int tmpCharacterPosX;
	byte secFrameCount;
	byte *animptr;

	switch (numSequence) {
	case 1: {
		tmpCharacterPosX = _characterPosX;
		_characterPosX = 3;
		if (!animationFile.open("POZO01.DAT")) {
			showError(277);
		}
		animX = 127;
		animY = 70;
		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(animFrameSize);
		for (uint loopIdx = 1; loopIdx <= 3; loopIdx++) {
			_sound->playVoc("POZO", 180395, 6034);
			animIndex = 0;
			do {
				emptyLoop();
				_chrono->_gameTick = false;
				if (_chrono->_gameTickHalfSpeed) {
					if (_currentSecondaryTrajectoryIndex >= _currentRoomData->secondaryTrajectoryLength)
						_currentSecondaryTrajectoryIndex = 1;
					else
						_currentSecondaryTrajectoryIndex += 1;
					_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
					_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
					_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
					if (_iframe2 >= _secondaryAnimationFrameCount - 1)
						_iframe2 = 0;
					else
						_iframe2++;
					sprites(false);
					animationFile.read(animptr, animFrameSize);
					_graphics->putImg(animX, animY, animptr);
					animIndex += 1;
				}
			} while (animIndex != secFrameCount && !shouldQuit());
			animationFile.seek(4);
		}
		free(animptr);
		animationFile.close();
		_characterPosX = tmpCharacterPosX;
	} break;
	case 2: {
		if (!animationFile.open("POZOATR.DAT")) {
			showError(277);
		}
		animX = 127;
		tmpCharacterPosX = _characterPosX;
		_characterPosX = 3;
		animY = 70;

		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(animFrameSize);
		for (repIndex = 1; repIndex <= 3; repIndex++) {
			_sound->playVoc("POZO", 180395, 6034);
			animIndex = 0;
			do {
				emptyLoop();
				_chrono->_gameTick = false;
				if (_chrono->_gameTickHalfSpeed) {
					if (_currentSecondaryTrajectoryIndex >= _currentRoomData->secondaryTrajectoryLength)
						_currentSecondaryTrajectoryIndex = 1;
					else
						_currentSecondaryTrajectoryIndex += 1;
					_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
					_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
					_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
					if (_iframe2 >= _secondaryAnimationFrameCount - 1)
						_iframe2 = 0;
					else
						_iframe2++;
					sprites(false);
					animationFile.read(animptr, animFrameSize);
					_graphics->putImg(animX, animY, animptr);
					animIndex += 1;
				}
			} while (animIndex != secFrameCount && !shouldQuit());
			animationFile.seek(4);
		}
		_sound->stopVoc();
		free(animptr);
		animationFile.close();
		_characterPosX = tmpCharacterPosX;
	} break;
	case 3: {
		tmpCharacterPosX = _characterPosX;
		_characterPosX = 3;
		if (!animationFile.open("POZO02.DAT")) {
			showError(277);
		}
		animX = 127;
		animY = 70;

		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(animFrameSize);
		animIndex = 0;
		do {
			emptyLoop();
			_chrono->_gameTick = false;
			if (_chrono->_gameTickHalfSpeed) {
				if (_currentSecondaryTrajectoryIndex >= _currentRoomData->secondaryTrajectoryLength)
					_currentSecondaryTrajectoryIndex = 1;
				else
					_currentSecondaryTrajectoryIndex += 1;
				_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
				_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
				_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
				if (_iframe2 >= _secondaryAnimationFrameCount - 1)
					_iframe2 = 0;
				else
					_iframe2++;
				sprites(false);
				animationFile.read(animptr, animFrameSize);
				_graphics->putImg(animX, animY, animptr);
				animIndex += 1;
			}
		} while (animIndex != secFrameCount && !shouldQuit());
		free(animptr);
		animationFile.close();
		_iframe = 0;
		_charFacingDirection = 2;
		emptyLoop();
		_chrono->_gameTick = false;
		_characterPosX = tmpCharacterPosX;
		sprites(true);
	} break;
	case 4: {
		adjustKey();
		if (!animationFile.open("TIOLLAVE.DAT")) {
			showError(277);
		}
		animX = 85;
		animY = 15;

		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(animFrameSize);

		for (animIndex = 1; animIndex <= 31; animIndex++) {
			animationFile.read(animptr, animFrameSize);
			emptyLoop();
			_chrono->_gameTick = false;
			_graphics->advancePaletteAnim();
			_graphics->putShape(animX, animY, animptr);
		}

		clearScreenLayer(_curObject->depth - 1);
		_graphics->restoreBackground();
		animIndex = _mainCharAnimation.depth;
		_mainCharAnimation.depth = 30;
		clearScreenLayer(12);
		_screenLayers[12] = animptr;
		_depthMap[12].posx = animX;
		_depthMap[12].posy = animY;
		assembleScreen();
		_graphics->drawScreen(_sceneBackground);
		// do not clear layer yet, needed for next frames
		_screenLayers[12] = nullptr;

		_mainCharAnimation.depth = animIndex;
		drawInventory();
		for (animIndex = 32; animIndex <= secFrameCount; animIndex++) {
			animationFile.read(animptr, animFrameSize);
			emptyLoop();
			_chrono->_gameTick = false;
			_graphics->advancePaletteAnim();
			_graphics->putImg(animX, animY, animptr);
			if (shouldQuit()) {
				break;
			}
		}
		adjustKey2();
		free(animptr);
		animationFile.close();
	} break;
	case 5: {
		if (!animationFile.open("TIOSACO.DAT")) {
			showError(277);
		}
		animX = 204;
		animY = 44;
		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(animFrameSize);
		for (animIndex = 1; animIndex <= 8; animIndex++) {
			animationFile.read(animptr, animFrameSize);
			emptyLoop();
			_chrono->_gameTick = false;
			_graphics->putShape(animX, animY, animptr);
			if (shouldQuit()) {
				break;
			}
		}
		animIndex = _mainCharAnimation.depth;
		_mainCharAnimation.depth = 30;

		clearScreenLayer(12);
		_screenLayers[12] = animptr;
		_depthMap[12].posx = animX;
		_depthMap[12].posy = animY;
		disableSecondAnimation();
		_mainCharAnimation.depth = animIndex;
		_graphics->drawScreen(_sceneBackground);
		// do not clear layer yet, needed for next frames
		_screenLayers[12] = nullptr;
		for (animIndex = 9; animIndex <= secFrameCount; animIndex++) {
			animationFile.read(animptr, animFrameSize);
			emptyLoop();
			_chrono->_gameTick = false;
			emptyLoop();
			_chrono->_gameTick = false;
			_graphics->putShape(animX, animY, animptr);
			if (shouldQuit()) {
				break;
			}
		}
		free(animptr);
		animationFile.close();
		_iframe = 0;
		_charFacingDirection = 2;
		emptyLoop();
		_chrono->_gameTick = false;
		sprites(true);
	} break;
	case 6: {
		_currentRoomData->animationFlag = false;
		if (!animationFile.open("AZCCOG.DAT")) {
			showError(277);
		}
		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		if (g_engine->_screenLayers[6]) {
			free(g_engine->_screenLayers[6]);
		}
		_screenLayers[6] = (byte *)malloc(animFrameSize);
		_depthMap[6].posx = _secondaryAnimation.posx + 5;
		_depthMap[6].posy = _secondaryAnimation.posy - 6;
		animIndex = 0;
		do {
			emptyLoop();
			_chrono->_gameTick = false;
			_graphics->advancePaletteAnim();
			if (_chrono->_gameTickHalfSpeed) {
				animationFile.read(_screenLayers[6], animFrameSize);
				Common::copy(_screenLayers[6], _screenLayers[6] + animFrameSize, _sceneBackground + 44900);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animIndex += 1;
				if (animIndex == 8)
					_sound->playVoc("PUFF", 191183, 18001);
			}
		} while (animIndex != secFrameCount && !shouldQuit());
		animationFile.close();
		_sound->stopVoc();
		clearScreenLayer(6);
		_currentRoomData->animationFlag = true;
	} break;
	}
}

void TotEngine::loadScreenData(uint screenNumber) {
	_currentRoomNumber = screenNumber;

	_rooms->seek(screenNumber * kRoomRegSize, SEEK_SET);
	if (_currentRoomData) {
		delete _currentRoomData;
	}
	_currentRoomData = readScreenDataFile(_rooms);
	loadScreen();
	for (int i = 0; i < 15; i++) {
		RoomBitmapRegister &bitmap = _currentRoomData->screenLayers[i];
		if (bitmap.bitmapSize > 0) {
			loadScreenLayer(bitmap.coordx, bitmap.coordy, bitmap.bitmapSize, bitmap.bitmapPointer, bitmap.depth);
		}
	}
	if (_currentRoomData->animationFlag && _currentRoomData->code != 24) {
		loadAnimation(_currentRoomData->animationName);
		_iframe2 = 0;
		_currentSecondaryTrajectoryIndex = 1;
		_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
		_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
		_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
		if (_currentRoomData->animationName == "FUENTE01")
			_secondaryAnimation.depth = 0;
		else {
			updateSecondaryAnimationDepth();
		}
		for (int i = 0; i < _maxXGrid; i++)
			for (int j = 0; j < _maxYGrid; j++) {
				if (_maskGridSecondaryAnim[i][j] > 0)
					_currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j] = _maskGridSecondaryAnim[i][j];
				if (_maskMouseSecondaryAnim[i][j] > 0)
					_currentRoomData->mouseGrid[_oldposx + i][_oldposy + j] = _maskMouseSecondaryAnim[i][j];
			}
	} else
		_currentRoomData->animationFlag = false;

	updateMainCharacterDepth();
	assembleScreen();
	g_engine->_graphics->_paletteAnimFrame = 0;
}

/**
 * Calculates a trajectory between the current position (xframe, yframe)
 * and the target position (finalx, finaly) using bresenham's algorithm
 */
void TotEngine::calculateTrajectory(uint finalX, uint finalY) {

	int deltaX = finalX - _characterPosX;
	int deltaY = finalY - _characterPosY;

	int incrXDiag = (deltaX >= 0) ? 1 : -1;
	int incrYDiag = (deltaY >= 0) ? 1 : -1;

	deltaX = abs(deltaX);
	deltaY = abs(deltaY);

	bool horizontalDominant = (deltaX >= deltaY);
	if (!horizontalDominant) {
		int tmp = deltaX;
		deltaX = deltaY;
		deltaY = tmp;
	}

	// step increments for straight moves
	int straightXIncrease = horizontalDominant ? incrXDiag : 0;
	int straightYIncrease = horizontalDominant ? 0 : incrYDiag;

	int straightAmount = 2 * deltaY;
	int slope = straightAmount - (2 * deltaX);
	int dec = straightAmount - deltaX; // decision variable

	// start at initial position
	uint x = _characterPosX, y = _characterPosY;
	int trajectorySteps = deltaX + 1; // number of steps

	_currentTrajectoryIndex = 0;
	_trajectoryLength = 0;
	_trajectory[_trajectoryLength].x = x;
	_trajectory[_trajectoryLength].y = y;
	_trajectoryLength++;
	while (--trajectorySteps) {
		if (dec >= 0) {
			x += incrXDiag;
			y += incrYDiag;
			dec += slope;
		} else {
			x += straightXIncrease;
			y += straightYIncrease;
			dec += straightAmount;
		}
		_trajectory[_trajectoryLength].x = x;
		_trajectory[_trajectoryLength].y = y;
		_trajectoryLength++;
	}

	// Ensure last value is also final position to account for overflow of the route
	_trajectory[_trajectoryLength].x = finalX;
	_trajectory[_trajectoryLength].y = finalY;
	_trajectory[_trajectoryLength + 1].x = finalX;
	_trajectory[_trajectoryLength + 1].y = finalY;
}

void TotEngine::lookAtObject(byte objectCode) {
	byte yaux;

	Common::String description;
	bool foobar = false;
	TextEntry textRef;
	Palette secPalette;

	_cpCounter2 = _cpCounter;
	_mouse->hide();
	_graphics->copyPalette(g_engine->_graphics->_pal, secPalette);
	readObject(_inventory[objectCode].code);
	_graphics->getImg(0, 0, 319, 139, _sceneBackground);
	_graphics->partialFadeOut(234);
	bar(0, 0, 319, 139, 0);
	for (yaux = 1; yaux <= 12; yaux++)
		buttonBorder(
			120 - (yaux * 5), // x1
			80 - (yaux * 5),  // y1
			200 + (yaux * 5), // x2
			70 + (yaux * 5),  // y2
			251,              // color1
			251,              // color2
			251,              // color3
			251,              // color4
			0                 // color5
		);

	drawMenu(4);

	if (_curObject->used[0] != 9) {
		if (_curObject->beforeUseTextRef != 0) {
			readTextFile();
			textRef = readTextRegister(_curObject->beforeUseTextRef);
			description = textRef.text;
			for (yaux = 0; yaux < textRef.text.size(); yaux++)
				description.setChar(_decryptionKey[yaux] ^ textRef.text[yaux], yaux);
			displayObjectDescription(description);
			_verbFile.close();
		} else {
			description = _inventory[objectCode].objectName;
			displayObjectDescription(description);
		}
	} else {

		if (_curObject->afterUseTextRef != 0) {
			readTextFile();
			textRef = readTextRegister(_curObject->afterUseTextRef);
			description = textRef.text;
			for (yaux = 0; yaux < textRef.text.size(); yaux++)
				description.setChar(_decryptionKey[yaux] ^ textRef.text[yaux], yaux);
			displayObjectDescription(description);
			_verbFile.close();
		} else {
			description = _inventory[objectCode].objectName;
			displayObjectDescription(description);
		}
	}

	drawFlc(125, 70, _curObject->rotatingObjectAnimation, 60000, 9, 0, false, true, true, foobar);

	_graphics->sceneTransition(true, nullptr, 3);
	_graphics->partialFadeOut(234);
	assembleScreen();
	_graphics->drawScreen(_sceneBackground);
	_graphics->copyPalette(secPalette, g_engine->_graphics->_pal);
	_graphics->partialFadeIn(234);
	_mouse->show();
}

void TotEngine::useInventoryObjectWithInventoryObject(uint objectCode1, uint objectCode2) {
	byte invIndex, indobj1, indobj2;

	readObject(_sceneObjectsData, objectCode1, _curObject);
	if (_curObject->used[0] != 1 || _curObject->useWith != objectCode2) {
		drawText(getRandom(11) + 1022);
		return;
	}

	invIndex = 0;
	while (_inventory[invIndex].code != objectCode1) {
		invIndex += 1;
	}
	indobj1 = invIndex;

	invIndex = 0;
	while (_inventory[invIndex].code != objectCode2) {
		invIndex += 1;
	}

	indobj2 = invIndex;
	uint textRef = _curObject->useTextRef;

	if (_curObject->replaceWith == 0) {
		readObject(_sceneObjectsData, objectCode1, _curObject);
		_curObject->used[0] = 9;
		saveObject(_curObject, _sceneObjectsData);

		readObject(_sceneObjectsData, objectCode2, _curObject);
		_curObject->used[0] = 9;
		saveObject(_curObject, _sceneObjectsData);

	} else {
		readObject(_sceneObjectsData, _curObject->replaceWith, _curObject);
		_inventory[indobj1].bitmapIndex = _curObject->objectIconBitmap;
		_inventory[indobj1].code = _curObject->code;
		_inventory[indobj1].objectName = _curObject->name;
		for (indobj1 = indobj2; indobj1 < (kInventoryIconCount - 1); indobj1++) {
			_inventory[indobj1].bitmapIndex = _inventory[indobj1 + 1].bitmapIndex;
			_inventory[indobj1].code = _inventory[indobj1 + 1].code;
			_inventory[indobj1].objectName = _inventory[indobj1 + 1].objectName;
		}
		_mouse->hide();
		drawInventory();
		_mouse->show();
	}
	if (textRef > 0)
		drawText(textRef);
}

void TotEngine::calculateRoute(byte zone1, byte zone2, bool extraCorrection, bool barredZone) {
	// Resets the entire route
	for (_trajectorySteps = 0; _trajectorySteps < 7; _trajectorySteps++) {
		_mainRoute[_trajectorySteps].x = 0;
		_mainRoute[_trajectorySteps].y = 0;
	}
	_trajectorySteps = 0;

	// Starts first element of route with current position
	_mainRoute[0].x = _characterPosX;
	_mainRoute[0].y = _characterPosY;
	Common::Point point;
	do {
		_trajectorySteps += 1;
		point = _currentRoomData->trajectories[zone1 - 1][zone2 - 1][_trajectorySteps - 1];

		if (point.x < (kCharacterCorrectionX + 3))
			_mainRoute[_trajectorySteps].x = 3;
		else
			_mainRoute[_trajectorySteps].x = point.x - kCharacterCorrectionX;
		if (point.y < (kCharacerCorrectionY + 3))
			_mainRoute[_trajectorySteps].y = 3;
		else
			_mainRoute[_trajectorySteps].y = point.y - kCharacerCorrectionY;

	} while (point.y != 9999 && _trajectorySteps != 5);

	if (zone2 < 10) {
		if (point.y == 9999) {
			_mainRoute[_trajectorySteps].x = _xframe2 - kCharacterCorrectionX;
			_mainRoute[_trajectorySteps].y = _yframe2 - kCharacerCorrectionY;
		} else {
			_mainRoute[6].x = _xframe2 - kCharacterCorrectionX;
			_mainRoute[6].y = _yframe2 - kCharacerCorrectionY;
			_trajectorySteps = 6;
		}
	} else {
		if ((_mainRoute[_trajectorySteps].y + kCharacerCorrectionY) == 9999) {
			_trajectorySteps -= 1;
		}
		if (extraCorrection) {
			switch (_currentRoomData->code) {
			case 5:
				if (zone2 == 27)
					_trajectorySteps += 1;
				break;
			case 6:
				if (zone2 == 21)
					_trajectorySteps += 1;
				break;
			}
		}
	}
	if (extraCorrection && barredZone) {
		_trajectorySteps -= 1;
	}

	// Sets xframe2 now to be the number of steps in the route
	_xframe2 = _trajectorySteps;
	// Sets yframe2 to now be current step
	_yframe2 = 1;
	_trajectoryLength = 10;
	_currentTrajectoryIndex = 30;
}

void TotEngine::goToObject(byte zone1, byte zone2) {
	bool barredZone = false;

	_cpCounter2 = _cpCounter;

	for (int i = 0; i < 5; i++) {
		if (_currentRoomData->doors[i].doorcode == zone2) {
			barredZone = true;
			break;
		}
	}

	if (_currentRoomData->code == 21 && _currentRoomData->animationFlag) {
		if ((zone2 >= 1 && zone2 <= 5) ||
			(zone2 >= 9 && zone2 <= 13) ||
			(zone2 >= 18 && zone2 <= 21) ||
			zone2 == 25) {

			_targetZone = 7;
			_mouse->mouseClickX = 232;
			_mouse->mouseClickY = 75;
			zone2 = 7;
		}
		if (zone2 == 24) {
			barredZone = false;
			_targetZone = 7;
			_mouse->mouseClickX = 232;
			_mouse->mouseClickY = 75;
			zone2 = 7;
		}
	}
	if (zone1 < 10) {
		_xframe2 = _mouse->mouseClickX + 7;
		_yframe2 = _mouse->mouseClickY + 7;

		_mouse->hide();
		calculateRoute(zone1, zone2, true, barredZone);

		do {
			_chrono->updateChrono();
			g_engine->_events->pollEvent();
			advanceAnimations(barredZone, false);
			_screen->update();
			g_system->delayMillis(10);
		} while (_xframe2 != 0);

		_iframe = 0;
		sprites(true);
		_mouse->show();
	}
}

void TotEngine::updateSecondaryAnimationDepth() {
	uint animationPos = _secondaryAnimation.posy + _secondaryAnimHeight - 1;
	if (animationPos && animationPos <= 56) {
		_secondaryAnimation.depth = 0;
	} else if (animationPos >= 57 && animationPos <= 64) {
		_secondaryAnimation.depth = 1;
	} else if (animationPos >= 65 && animationPos <= 72) {
		_secondaryAnimation.depth = 2;
	} else if (animationPos >= 73 && animationPos <= 80) {
		_secondaryAnimation.depth = 3;
	} else if (animationPos >= 81 && animationPos <= 88) {
		_secondaryAnimation.depth = 4;
	} else if (animationPos >= 89 && animationPos <= 96) {
		_secondaryAnimation.depth = 5;
	} else if (animationPos >= 97 && animationPos <= 104) {
		_secondaryAnimation.depth = 6;
	} else if (animationPos >= 105 && animationPos <= 112) {
		_secondaryAnimation.depth = 7;
	} else if (animationPos >= 113 && animationPos <= 120) {
		_secondaryAnimation.depth = 8;
	} else if (animationPos >= 121 && animationPos <= 140) {
		_secondaryAnimation.depth = 9;
	}
}

void TotEngine::updateMainCharacterDepth() {
	if (_characterPosY && _characterPosY <= 7) {
		_mainCharAnimation.depth = 0;
	} else if (_characterPosY >= 8 && _characterPosY <= 17) {
		_mainCharAnimation.depth = 1;
	} else if (_characterPosY >= 18 && _characterPosY <= 25) {
		_mainCharAnimation.depth = 2;
	} else if (_characterPosY >= 26 && _characterPosY <= 33) {
		_mainCharAnimation.depth = 3;
	} else if (_characterPosY >= 34 && _characterPosY <= 41) {
		_mainCharAnimation.depth = 4;
	} else if (_characterPosY >= 42 && _characterPosY <= 49) {
		_mainCharAnimation.depth = 5;
	} else if (_characterPosY >= 50 && _characterPosY <= 57) {
		_mainCharAnimation.depth = 6;
	} else if (_characterPosY >= 58 && _characterPosY <= 65) {
		_mainCharAnimation.depth = 7;
	} else if (_characterPosY >= 66 && _characterPosY <= 73) {
		_mainCharAnimation.depth = 8;
	} else if (_characterPosY >= 74 && _characterPosY <= 139) {
		_mainCharAnimation.depth = 9;
	}
}

void TotEngine::advanceAnimations(bool barredZone, bool animateMouse) {
	if (_chrono->_gameTick) {

		if (_currentRoomData->animationFlag && _chrono->_gameTickHalfSpeed) {
			if (_isPeterCoughing && (getRandom(100) == 1) && !_sound->isVocPlaying() && _mintTopic[0] == false) {
				debug("Playing tos");
				_sound->playVoc("TOS", 258006, 14044);
			}
			if (_currentSecondaryTrajectoryIndex >= _currentRoomData->secondaryTrajectoryLength)
				_currentSecondaryTrajectoryIndex = 1;
			else
				_currentSecondaryTrajectoryIndex += 1;
			if (_iframe2 >= _secondaryAnimationFrameCount - 1)
				_iframe2 = 0;
			else
				_iframe2++;
			_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
			_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
			_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
			switch (_currentRoomData->code) {
			case 23:
				_secondaryAnimation.depth = 0;
				break;
			case 24:
				_secondaryAnimation.depth = 14;
				break;
			default:
				updateSecondaryAnimationDepth();
			}
		}

		// This means character needs to walk
		if (_xframe2 > 0) {
			if (_charFacingDirection == 1 || _charFacingDirection == 3) {
				_currentTrajectoryIndex += 1;
			}
			_currentTrajectoryIndex += 1;
			// yframe2, xframe2 now represent max steps and current step
			if (_yframe2 <= _xframe2) {
				if (_currentTrajectoryIndex > _trajectoryLength) {
					_element1 = _mainRoute[_yframe2].x - _mainRoute[_yframe2 - 1].x;
					_element2 = _mainRoute[_yframe2].y - _mainRoute[_yframe2 - 1].y;

					if ((abs(_element1) > 2) || (abs(_element2) > 2)) {
						if (abs(_element2) > (abs(_element1) + 5)) {
							if (_element2 < 0)
								_charFacingDirection = 0;
							else
								_charFacingDirection = 2;
						} else {
							if (_element1 > 0)
								_charFacingDirection = 1;
							else
								_charFacingDirection = 3;
						}
					}
					_yframe2 += 1;
					calculateTrajectory(_mainRoute[_yframe2 - 1].x, _mainRoute[_yframe2 - 1].y);
				}
			} else if (_currentTrajectoryIndex >= _trajectoryLength) {
				_xframe2 = 0;
				if (!_roomChange) {
					_element1 = (_mouse->mouseX + 7) - (_characterPosX + kCharacterCorrectionX);
					_element2 = (_mouse->mouseY + 7) - (_characterPosY + kCharacerCorrectionY);
					if (abs(_element2) > (abs(_element1) + 20)) {
						if (_element2 < 0)
							_charFacingDirection = 0;
						else
							_charFacingDirection = 2;
					} else {
						if (_element1 > 0)
							_charFacingDirection = 1;
						else
							_charFacingDirection = 3;
					}
				}
			}
			_iframe++;
			if (_iframe > kWalkFrameCount)
				_iframe = 0;
		} else {
			_iframe = 0;
			if (barredZone == false && !_roomChange) {
				_element1 = (_mouse->mouseX + 7) - (_characterPosX + kCharacterCorrectionX);
				_element2 = (_mouse->mouseY + 7) - (_characterPosY + kCharacerCorrectionY);
				if (abs(_element2) > (abs(_element1) + 20)) {
					if (_element2 < 0)
						_charFacingDirection = 0;
					else
						_charFacingDirection = 2;
				} else {
					if (_element1 > 0)
						_charFacingDirection = 1;
					else
						_charFacingDirection = 3;
				}
			}
		}
		_characterPosX = _trajectory[_currentTrajectoryIndex].x;
		_characterPosY = _trajectory[_currentTrajectoryIndex].y;

		updateMainCharacterDepth();

		if (_isDrawingEnabled) {
			sprites(true);
		}
		_chrono->_gameTick = false;
		_graphics->advancePaletteAnim();
	}
}

void TotEngine::actionLineText(const Common::String &actionLine) {
	euroText(160, 144, actionLine, 255, Graphics::kTextAlignCenter);
}

void TotEngine::animateGive(uint dir, uint height) {
	_charFacingDirection = dir;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		_chrono->_gameTick = false;
		// Must add 1 to i because the original game uses 1-based indices
		_iframe = 15 + 6 + 5 + height * 10 - (i + 1);

		_graphics->advancePaletteAnim();
		sprites(true);
	}
	_iframe = 0;
}

// Lean over to pick
void TotEngine::animatePickup1(uint dir, uint height) {
	_charFacingDirection = dir;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		_chrono->_gameTick = false;
		_iframe = 15 + height * 10 + (i + 1);

		_graphics->advancePaletteAnim();
		sprites(true);
		_screen->update();
	}
}

// Lean back after pick
void TotEngine::animatePickup2(uint dir, uint height) {
	_charFacingDirection = dir;

	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		_chrono->_gameTick = false;

		_iframe = 15 + 5 + height * 10 + (i + 1);

		_graphics->advancePaletteAnim();
		sprites(true);
		_screen->update();
	}
	emptyLoop();
	_chrono->_gameTick = false;
	sprites(true);
	_iframe = 0;
}

void TotEngine::animateOpen2(uint dir, uint height) {
	_charFacingDirection = dir;
	_cpCounter = _cpCounter2;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		_chrono->_gameTick = false;
		_iframe = 15 + 6 + height * 10 - (i + 1);

		_graphics->advancePaletteAnim();
		sprites(true);
	}
	emptyLoop();
	_chrono->_gameTick = false;
	sprites(true);
	_iframe = 0;
}

void TotEngine::animateBat() {
	const uint batTrajectory[91][2] = {
			{288,  40},  {289,  38},  {289,  36},  {289,  34},  {290,  32},  {290,  30},  {289,  31},  {288,  32},
			{288,  34},  {286,  36},  {284,  38},  {282,  40},  {280,  42},  {278,  44},  {276,  46},  {274,  48},
			{272,  50},  {270,  48},  {268,  46},  {266,  48},  {265,  50},  {264,  52},  {263,  54},  {263,  56},
			{262,  58},  {261,  60},  {261,  62},  {261,  64},  {260,  66},  {260,  68},  {260,  70},  {260,  72},
			{259,  74},  {259,  76},  {260,  78},  {262,  76},  {264,  74},  {266,  72},  {266,  74},  {266,  76},
			{268,  78},  {270,  80},  {272,  82},  {274,  84},  {276,  86},  {278,  88},  {279,  90},  {280,  88},
			{281,  86},  {282,  84},  {283,  82},  {283,  84},  {283,  86},  {283,  88},  {284,  90},  {284,  92},
			{284,  94},  {284,  96},  {285,  98},  {285, 100},  {286,  98},  {288,  96},  {290,  94},  {290,  92},
			{290,  90},  {289,  92},  {289,  94},  {289,  96},  {289,  98},  {289, 100},  {289, 102},  {288, 104},
			{287, 106},  {287, 108},  {288, 110},  {288, 112},  {288, 114},  {289, 116},  {289, 114},  {289, 112},
			{290, 110},  {290, 112},  {290, 114},  {289, 116},  {289, 118},  {289, 120},  {289, 122},  {289, 124},
			{290, 122},  {290, 120},  {290, 118}
	};
	byte curAnimIFrame = 0;
	uint currAnimWidth = 0, curAnimHeight = 0,
		 curAnimIdx = 0, curAnimLength = 0,
		 curAnimX = 0, curAnimY = 0,
		 curAnimDepth = 0, curAnimDir = 0;

	bool loopBreak = false;
	if (_currentRoomData->animationFlag) {
		curAnimIFrame = _iframe2;
		curAnimX = _secondaryAnimation.posx;
		curAnimY = _secondaryAnimation.posy;
		currAnimWidth = _secondaryAnimWidth;
		curAnimHeight = _secondaryAnimHeight;
		curAnimDepth = _secondaryAnimation.depth;
		curAnimDir = _secondaryAnimation.dir;
		clearAnimation();
	}
	loadBat();
	_sound->stopVoc();
	_sound->playVoc("MURCIEL", 160848, 4474);
	curAnimIdx = 0;
	curAnimLength = 87;
	_iframe2 = 0;
	_secondaryAnimation.depth = 14;
	do {
		_chrono->updateChrono();
		if (_chrono->_gameTick) {
			if (curAnimIdx == curAnimLength)
				loopBreak = true;
			curAnimIdx += 1;
			if (_iframe2 >= _secondaryAnimationFrameCount - 1)
				_iframe2 = 0;
			else
				_iframe2++;

			_secondaryAnimation.posx = batTrajectory[curAnimIdx][0] - 20;
			_secondaryAnimation.posy = batTrajectory[curAnimIdx][1];
			_secondaryAnimation.dir = 0;
			sprites(true);
			_chrono->_gameTick = false;
			if (curAnimIdx % 24 == 0)
				_sound->playVoc();
			_graphics->advancePaletteAnim();
		}
		_screen->update();
		g_system->delayMillis(10);
	} while (!loopBreak && !shouldQuit());

	_sound->stopVoc();
	clearAnimation();
	if (_currentRoomData->animationFlag) {
		_secondaryAnimWidth = currAnimWidth;
		_secondaryAnimHeight = curAnimHeight;
		setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, RESTORE, false);
		loadAnimation(_currentRoomData->animationName);
		_iframe2 = curAnimIFrame;
		_secondaryAnimation.posx = curAnimX;
		_secondaryAnimation.posy = curAnimY;
		_secondaryAnimation.depth = curAnimDepth;
		_secondaryAnimation.dir = curAnimDir;
	}
	_graphics->restoreBackground();
	assembleScreen();
	_graphics->drawScreen(_sceneBackground);
}

void TotEngine::updateVideo() {
	readBitmap(_curObject->dropOverlay, _screenLayers[_curObject->depth - 1], _curObject->dropOverlaySize, 319);
	_graphics->restoreBackground();
	assembleScreen();
	_graphics->drawScreen(_sceneBackground);
}

void TotEngine::nicheAnimation(byte nicheDir, int32 bitmap) {
	uint bitmapOffset = 0;
	int increment = 0;

	// Room with Red
	if (_currentRoomData->code == 24) {
		if (g_engine->_screenLayers[1]) {
			free(g_engine->_screenLayers[1]);
		}
		_screenLayers[1] = (byte *)malloc(3660);
		readBitmap(1382874, _screenLayers[1], 3652, 319);
		uint16 object1Width = READ_LE_UINT16(_screenLayers[1]);
		uint16 object1Height = READ_LE_UINT16(_screenLayers[1] + 2);
		_depthMap[1].posx = 211;
		_depthMap[1].posy = 16;
		_depthMap[1].posx2 = 211 + object1Width + 1;
		_depthMap[1].posy2 = 16 + object1Height + 1;
	}

	switch (nicheDir) {
	case 0: {
		bitmapOffset = 44904;
		increment = 1;
		Common::copy(_screenLayers[0], _screenLayers[0] + 892, _sceneBackground + 44900);
		readBitmap(bitmap, _screenLayers[0], 892, 319);
		Common::copy(_screenLayers[0] + 4, _screenLayers[0] + 4 + 888, _sceneBackground + 44900 + 892);
	} break;
	case 1: { // object slides to reveal empty stand
		bitmapOffset = 892 + 44900;
		increment = -1;
		// Reads the empty niche into a non-visible part of background
		readBitmap(bitmap, _sceneBackground + 44900, 892, 319);
		// Copies whatever is currently on the niche in a non-visible part of background contiguous with the above
		Common::copy(_screenLayers[0] + 4, _screenLayers[0] + 4 + 888, _sceneBackground + 44900 + 892);
		// We now have in consecutive pixels the empty stand and the object

	} break;
	}
	uint16 nicheWidth = READ_LE_UINT16(_screenLayers[0]);
	uint16 nicheHeight = READ_LE_UINT16(_screenLayers[0] + 2);

	// Set the height to double to animate 2 images of the same height moving up/down
	*(_sceneBackground + 44900 + 2) = (nicheHeight * 2) + 1;

	_graphics->restoreBackground();

	for (uint i = 1; i <= nicheHeight; i++) {

		bitmapOffset = bitmapOffset + (increment * (nicheWidth + 1));
		Common::copy(_sceneBackground + bitmapOffset, _sceneBackground + bitmapOffset + 888, _screenLayers[0] + 4);
		assembleScreen();
		_graphics->drawScreen(_sceneBackground);
		_screen->update();
	}
	readBitmap(bitmap, _screenLayers[0], 892, 319);

	_graphics->restoreBackground();
	assembleScreen();
	_graphics->drawScreen(_sceneBackground);

	if (_currentRoomData->code == 24) {
		clearScreenLayer(1);
	}
}

void TotEngine::pickupScreenObject() {
	byte inventoryIndex;
	uint screenObject;
	Common::Point p = _mouse->getClickCoordsWithinGrid();
	uint correctedMouseX = p.x;
	uint correctedMouseY = p.y;
	screenObject = _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex;
	if (screenObject == 0)
		return;
	readObject(screenObject);
	goToObject(
		_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount],
		_currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);
	if (_curObject->pickupable) {
		_mouse->hide();
		switch (_curObject->code) {
		case 521: { // Corridor lamp
			_currentRoomData->mouseGrid[10][11] = 19;
			_currentRoomData->mouseGrid[9][12] = 18;
			_currentRoomData->mouseGrid[10][12] = 18;
		} break;
		case 567: { // Pickup rubble
			if (_currentRoomData->animationFlag) {
				_mouse->show();
				drawText(3226);
				return;
			}
		} break;
		case 590: // Ectoplasm
			_caves[1] = true;
			break;
		case 665: // Bird
			_caves[0] = true;
			break;
		case 676:
		case 688: {
			_caves[4] = true;
			_isVasePlaced = false;
		} break;
		}
		switch (_curObject->height) {
		case 0: { // Pick up above
			switch (_curObject->code) {
			case 590: { // Ectoplasm
				animatePickup1(3, 0);
				animatePickup2(3, 0);
			} break;
			default: {
				animatePickup1(_charFacingDirection, 0);
				clearScreenLayer(_curObject->depth - 1);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animatePickup2(_charFacingDirection, 0);
			}
			}
		} break;
		case 1: { // Waist level
			switch (_curObject->code) {
			case 218: // Necronomicon
			case 308: // Mistletoe
			case 517: // mints
				animatePickup1(0, 1);
				animatePickup2(0, 1);
			 break;
			case 223: { // table cloths
				animatePickup1(0, 1);
				_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject->replaceWith;
				updateVideo();
				animatePickup2(0, 1);
			} break;
			case 402: { // Kitchen table
				animatePickup1(0, 1);
				updateVideo();
				animatePickup2(0, 1);
			} break;
			case 479: { // Scissors
				animatePickup1(3, 1);
				animatePickup2(3, 1);
			} break;
			case 521: { // Puts plaster and key on the floor
				animatePickup1(0, 1);
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[1];

					with.bitmapPointer = 775611;
					with.bitmapSize = 36;
					with.coordx = 80;
					with.coordy = 56;
					with.depth = 2;
					loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[2];

					with.bitmapPointer = 730743;
					with.bitmapSize = 64;
					with.coordx = 76;
					with.coordy = 62;
					with.depth = 1;
					loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}

				clearScreenLayer(_curObject->depth -1);
				_graphics->restoreBackground();

				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animatePickup2(0, 1);
			} break;
			case 562: { // niche
				switch (_currentRoomData->code) {
				case 20: { // First scene with niche
					if (_niche[0][_niche[0][3]] > 0) {
						// Possibly
						if (_niche[0][3] == 2 || _niche[0][_niche[0][3]] == 563) {
							readObject(_niche[0][_niche[0][3]]);
							_niche[0][_niche[0][3]] = 0;
							_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
							animatePickup1(3, 1);
							readBitmap(1190768, _screenLayers[_curObject->depth - 1], 892, 319);
							_currentRoomData->screenLayers[1].bitmapPointer = 1190768;
							_currentRoomData->screenLayers[1].bitmapSize = 892;
							_currentRoomData->screenLayers[1].coordx = 66;
							_currentRoomData->screenLayers[1].coordy = 35;
							_currentRoomData->screenLayers[1].depth = 1;
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(_sceneBackground);
							animatePickup2(3, 1);
						} else {
							readObject(_niche[0][_niche[0][3]]);
							_niche[0][_niche[0][3]] = 0;
							_niche[0][3] += 1;
							_niche[1][3] -= 1;
							_currentRoomData->screenObjectIndex[9]->objectName = "                    ";
							animatePickup1(3, 1);
							readBitmap(1190768, _screenLayers[_curObject->depth - 1],
									   892, 319);
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(_sceneBackground);
							animatePickup2(3, 1);
							_sound->playVoc("PLATAF", 375907, 14724);
							_currentRoomData->screenLayers[1].bitmapSize = 892;
							_currentRoomData->screenLayers[1].coordx = 66;
							_currentRoomData->screenLayers[1].coordy = 35;
							_currentRoomData->screenLayers[1].depth = 1;
							switch (_niche[0][_niche[0][3]]) {
							case 0: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
								nicheAnimation(0, 1190768);
								_currentRoomData->screenLayers[1].bitmapPointer = 1190768;
							} break;
							case 561: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
								nicheAnimation(0, 1182652);
								_currentRoomData->screenLayers[1].bitmapPointer = 1182652;
							} break;
							case 563: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
								nicheAnimation(0, 1186044);
								_currentRoomData->screenLayers[1].bitmapPointer = 1186044;
							} break;
							case 615: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
								nicheAnimation(0, 1181760);
								_currentRoomData->screenLayers[1].bitmapPointer = 1181760;
							} break;
							}
							updateAltScreen(24);
						}
					} else {

						_mouse->show();
						drawText(1049 + getRandom(10));
						_mouse->hide();
						return;
					}
				} break;
				case 24: { // Second scene with niche
					if (_niche[1][_niche[1][3]] > 0 && _niche[1][3] != 1) {
						if (_niche[1][3] == 2) {
							readObject(_niche[1][2]);
							_niche[1][2] = 0;
							_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
							animatePickup1(0, 1);
							readBitmap(1399610, _screenLayers[_curObject->depth - 1], 892, 319);
							_currentRoomData->screenLayers[0].bitmapPointer = 1399610;
							_currentRoomData->screenLayers[0].bitmapSize = 892;
							_currentRoomData->screenLayers[0].coordx = 217;
							_currentRoomData->screenLayers[0].coordy = 48;
							_currentRoomData->screenLayers[0].depth = 1;
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(_sceneBackground);
							animatePickup2(0, 1);
						} else {
							readObject(_niche[1][_niche[1][3]]);
							_niche[1][_niche[1][3]] = 622;
							_niche[1][3] += 1;
							_niche[0][3] -= 1;
							_currentRoomData->screenObjectIndex[8]->objectName = "                    ";
							animatePickup1(0, 1);
							readBitmap(1399610, _screenLayers[0], 892, 319);
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(_sceneBackground);
							animatePickup2(0, 1);
							_sound->playVoc("PLATAF", 375907, 14724);
							_currentRoomData->screenLayers[0].bitmapSize = 892;
							_currentRoomData->screenLayers[0].coordx = 217;
							_currentRoomData->screenLayers[0].coordy = 48;
							_currentRoomData->screenLayers[0].depth = 1;
							switch (_niche[1][_niche[1][3]]) {
							case 0: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
								nicheAnimation(0, 1399610);
								_currentRoomData->screenLayers[0].bitmapPointer = 1399610;
							} break;
							case 561: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
								nicheAnimation(0, 1381982);
								_currentRoomData->screenLayers[0].bitmapPointer = 1381982;
							} break;
							case 615: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
								nicheAnimation(0, 1381090);
								_currentRoomData->screenLayers[0].bitmapPointer = 1381090;
							} break;
							case 622: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
								nicheAnimation(0, 1400502);
								_currentRoomData->screenLayers[0].bitmapPointer = 1400502;
							} break;
							case 623: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
								nicheAnimation(0, 1398718);
								_currentRoomData->screenLayers[0].bitmapPointer = 1398718;
							} break;
							}
							updateAltScreen(20);
						}
					} else {
						_mouse->show();
						drawText(1049 + getRandom(10));
						_mouse->hide();
						return;
					}
				} break;
				}
			} break;
			case 624: { // red devil
				animatePickup1(2, 1);
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[3];

					with.bitmapPointer = 0;
					with.bitmapSize = 0;
					with.coordx = 0;
					with.coordy = 0;
					with.depth = 0;
				}
				clearScreenLayer(3);
				disableSecondAnimation();
				_graphics->drawScreen(_sceneBackground);
				animatePickup2(2, 1);
				_isRedDevilCaptured = true;
				_isTrapSet = false;
			} break;
			default: {
				animatePickup1(_charFacingDirection, 1);
				clearScreenLayer(_curObject->depth - 1);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animatePickup2(_charFacingDirection, 1);
			}
			}
		} break;
		case 2: { // Pick up feet level
			switch (_curObject->code) {
			case 216: { // chisel
				animatePickup1(0, 2);
				_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject->replaceWith;
				updateVideo();
				animatePickup2(0, 2);
			} break;
			case 295: { // candles
				animatePickup1(3, 2);
				_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject->replaceWith;
				updateVideo();
				animatePickup2(3, 2);
			} break;
			case 362: { // charcoal
				animatePickup1(0, 2);
				animatePickup2(0, 2);
			} break;
			case 598: { // mud
				animatePickup1(3, 2);
				animatePickup2(3, 2);
			} break;
			case 659: { // spider web, puts bird and ring on the floor
				animatePickup1(3, 2);
				clearScreenLayer(_curObject->depth - 1);
				{ // bird
					RoomBitmapRegister &with = _currentRoomData->screenLayers[2];

					with.bitmapPointer = 1545924;
					with.bitmapSize = 172;
					with.coordx = 38;
					with.coordy = 58;
					with.depth = 1;
					loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{ // ring
					RoomBitmapRegister &with = _currentRoomData->screenLayers[1];

					with.bitmapPointer = 1591272;
					with.bitmapSize = 92;
					with.coordx = 50;
					with.coordy = 58;
					with.depth = 3;
					loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animatePickup2(3, 2);
			} break;
			default: {
				animatePickup1(_charFacingDirection, 2);
				clearScreenLayer(_curObject->depth - 1);
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animatePickup2(_charFacingDirection, 2);
			}
			}
		} break;
		case 9: { // bat
			uint textRef = _curObject->pickTextRef;
			readObject(204);
			animatePickup1(0, 1);
			animateOpen2(0, 1);
			animateBat();
			_mouse->show();
			drawText(textRef);
			_currentRoomData->mouseGrid[34][8] = 24;
			_actionCode = 0;
			_oldGridX = 0;
			_oldGridY = 0;
			checkMouseGrid();
			return;
		} break;
		}
		_mouse->show();

		if (_curObject->code != 624)
			for (int j = _curObject->ygrid1; j <= _curObject->ygrid2; j++)
				for (int i = _curObject->xgrid1; i <= _curObject->xgrid2; i++) {
					_currentRoomData->walkAreasGrid[i][j] = _curObject->walkAreasPatch[i - _curObject->xgrid1][j - _curObject->ygrid1];
					_currentRoomData->mouseGrid[i][j] = _curObject->mouseGridPatch[i - _curObject->xgrid1][j - _curObject->ygrid1];
				}
		switch (_curObject->code) {
		case 216: { // chisel
			_currentRoomData->screenLayers[5].bitmapPointer = 517485;
			_currentRoomData->screenLayers[5].bitmapSize = 964;
			_currentRoomData->screenLayers[5].coordx = 223;
			_currentRoomData->screenLayers[5].coordy = 34;
			_currentRoomData->screenLayers[5].depth = 1;
		} break;
		case 218:; // necronomicon
			break;
		case 223: { // table cloth
			_currentRoomData->screenLayers[6].bitmapPointer = 436752;
			_currentRoomData->screenLayers[6].bitmapSize = 1372;
			_currentRoomData->screenLayers[6].coordx = 174;
			_currentRoomData->screenLayers[6].coordy = 32;
			_currentRoomData->screenLayers[6].depth = 1;
		} break;
		case 295: { // candles
			_currentRoomData->screenLayers[3].bitmapPointer = 1130756;
			_currentRoomData->screenLayers[3].bitmapSize = 1764;
			_currentRoomData->screenLayers[3].coordx = 100;
			_currentRoomData->screenLayers[3].coordy = 28;
			_currentRoomData->screenLayers[3].depth = 1;
		} break;
		case 308:; // mistletoe
			break;
		case 362:; // charcoal
			break;
		case 402: {
			_currentRoomData->screenLayers[5].bitmapPointer = 68130;
			_currentRoomData->screenLayers[5].bitmapSize = 2564;
			_currentRoomData->screenLayers[5].coordx = 148;
			_currentRoomData->screenLayers[5].coordy = 49;
			_currentRoomData->screenLayers[5].depth = 7;
		} break;
		case 479:; // scissors
			break;
		case 517:; // mints
			break;
		case 590:; // ectoplasm
			break;
		case 598:; // mud
			break;
		case 624:; // red devil
			break;
		default: {
			for (int i = 0; i < 15; i++)
				if (_currentRoomData->screenLayers[i].bitmapPointer == _curObject->bitmapPointer) {
					_currentRoomData->screenLayers[i].bitmapPointer = 0;
					_currentRoomData->screenLayers[i].bitmapSize = 0;
					_currentRoomData->screenLayers[i].coordx = 0;
					_currentRoomData->screenLayers[i].coordy = 0;
					_currentRoomData->screenLayers[i].depth = 0;
				}
		}
		}
	} else {
		if (_curObject->pickTextRef > 0)
			drawText(_curObject->pickTextRef);
		_actionCode = 0;
		_oldGridX = 0;
		_oldGridY = 0;
		checkMouseGrid();
		return;
	}
	inventoryIndex = 0;

	while (_inventory[inventoryIndex].code != 0) {
		inventoryIndex += 1;
	}

	_inventory[inventoryIndex].bitmapIndex = _curObject->objectIconBitmap;
	_inventory[inventoryIndex].code = _curObject->code;
	_inventory[inventoryIndex].objectName = _curObject->name;
	_mouse->hide();
	drawInventory();
	_mouse->show();
	if (_curObject->pickTextRef > 0)
		drawText(_curObject->pickTextRef);
	_actionCode = 0;
	_oldGridX = 0;
	_oldGridY = 0;
	checkMouseGrid();
}

void TotEngine::replaceBackpack(byte obj1, uint obj2) {
	readObject(obj2);
	_inventory[obj1].bitmapIndex = _curObject->objectIconBitmap;
	_inventory[obj1].code = obj2;
	_inventory[obj1].objectName = _curObject->name;
	_cpCounter = _cpCounter2;
}

void TotEngine::dropObjectInScreen(ScreenObject *replacementObject) {
	byte objIndex;

	if (replacementObject->bitmapSize > 0) {
		objIndex = 0;
		while (!(_currentRoomData->screenLayers[objIndex].bitmapSize == 0) || objIndex == 15) {
			objIndex++;
		}
		if (_currentRoomData->screenLayers[objIndex].bitmapSize == 0) {
			{
				RoomBitmapRegister &with = _currentRoomData->screenLayers[objIndex];

				with.bitmapPointer = replacementObject->bitmapPointer;
				with.bitmapSize = replacementObject->bitmapSize;
				with.coordx = replacementObject->dropOverlayX;
				with.coordy = replacementObject->dropOverlayY;
				with.depth = replacementObject->depth;
				loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
			}
			for (int j = replacementObject->ygrid1; j <= replacementObject->ygrid2; j++)
				for (int i = replacementObject->xgrid1; i <= replacementObject->xgrid2; i++) {
					_currentRoomData->walkAreasGrid[i][j] = replacementObject->walkAreasPatch[i - replacementObject->xgrid1][j - replacementObject->ygrid1];
					_currentRoomData->mouseGrid[i][j] = replacementObject->mouseGridPatch[i - replacementObject->xgrid1][j - replacementObject->ygrid1];
				}
		} else
			showError(264);
	}
}

void TotEngine::useScreenObject() {
	byte usedObjectIndex,
		listIndex, invIndex;
	bool verifyList, foobar;
	uint foo;
	Common::Point p = _mouse->getClickCoordsWithinGrid();
	uint correctedMouseX = p.x;
	uint correctedMouseY = p.y;
	uint sceneObject = _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex;

	if (_inventoryObjectName != "") { //Use inv object with something on the scene
		usedObjectIndex = 0;
		while (_inventory[usedObjectIndex].objectName != _inventoryObjectName) {
			usedObjectIndex += 1;
		}

		readObject(_inventory[usedObjectIndex].code);

		goToObject(
			_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount],
			_currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);

		if (_curObject->useWith == sceneObject && sceneObject > 0 && _curObject->used[0] == 5) {
			switch (_curObject->useWith) {
			case 30: { // corn with rooster
				drawText(_curObject->useTextRef);
				_mouse->hide();

				animateGive(1, 2);
				animatePickup2(1, 2);

				updateObject(_curObject->code);
				_currentRoomData->screenObjectIndex[27]->fileIndex = 201;
				do {
					_chrono->updateChrono();
					if (_iframe2 >= _secondaryAnimationFrameCount - 1)
						_iframe2 = 0;
					else
						_iframe2++;
					if (_currentSecondaryTrajectoryIndex >= _currentRoomData->secondaryTrajectoryLength)
						_currentSecondaryTrajectoryIndex = 1;
					else
						_currentSecondaryTrajectoryIndex += 1;
					_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
					_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
					_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
					emptyLoop();
					_chrono->_gameTick = false;
					emptyLoop2();
					sprites(true);
					_screen->update();
				} while (_currentSecondaryTrajectoryIndex != 4);

				for (listIndex = 0; listIndex < _maxXGrid; listIndex++)
					for (invIndex = 0; invIndex < _maxYGrid; invIndex++) {
						_currentRoomData->walkAreasGrid[_oldposx + listIndex][_oldposy + invIndex] = _movementGridForSecondaryAnim[listIndex][invIndex];
						_currentRoomData->mouseGrid[_oldposx + listIndex][_oldposy + invIndex] = _mouseGridForSecondaryAnim[listIndex][invIndex];
					}

				clearAnimation();
				_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x + 8;
				_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
				_currentRoomData->animationName = "GALLOPIC";
				_currentRoomData->secondaryAnimDirections[299] = 201;
				loadAnimation("GALLOPIC");
				_currentRoomData->secondaryAnimDirections[0] = 0;
				_currentRoomData->secondaryAnimTrajectory[0].x = _secondaryAnimation.posx;
				_currentRoomData->secondaryAnimTrajectory[0].y = _secondaryAnimation.posy;
				_currentSecondaryTrajectoryIndex = 1;
				_currentRoomData->secondaryTrajectoryLength = 1;

				for (listIndex = 0; listIndex < _maxXGrid; listIndex++)
					for (invIndex = 0; invIndex < _maxYGrid; invIndex++) {
						if (_maskGridSecondaryAnim[listIndex][invIndex] > 0)
							_currentRoomData->walkAreasGrid[_oldposx + listIndex][_oldposy + invIndex] = _maskGridSecondaryAnim[listIndex][invIndex];
						if (_maskMouseSecondaryAnim[listIndex][invIndex] > 0)
							_currentRoomData->mouseGrid[_oldposx + listIndex][_oldposy + invIndex] = _maskMouseSecondaryAnim[listIndex][invIndex];
					}
				_mouse->show();
			} break;
			case 153: { // oil with well
				drawText(_curObject->useTextRef);
				_mouse->hide();
				_sound->playVoc("BLUP", 330921, 3858);
				animateGive(3, 1);
				animatePickup2(3, 1);
				_mouse->show();
				updateObject(_curObject->code);
				_currentRoomData->screenObjectIndex[21]->fileIndex = 154;
			} break;
			case 157: { // giving something to john
				verifyList = false;
				debug("used object = %d", _inventory[usedObjectIndex].code);
				if (_obtainedList1) {
					for (listIndex = 0; listIndex < 5; listIndex++) {
						if (_inventory[usedObjectIndex].code == _firstList[listIndex])
							verifyList = true;
					}

					if (verifyList) {
						int completedListItems = 0;
						for (listIndex = 0; listIndex < 5; listIndex++) {
							for (invIndex = 0; invIndex < kInventoryIconCount; invIndex++) {
								if (_inventory[invIndex].code == _firstList[listIndex]) {
									completedListItems += 1;
									break;
								}
							}
						}
						if (completedListItems == 5) {
							_obtainedList1 = false;
							_list1Complete = true;
							readTextFile();
							sayLine(1018, 255, 0, foo, false);
							sayLine(foo, 253, 249, foo, true);
							_verbFile.close();

							_mouse->hide();
							animateGive(_charFacingDirection, 1);
							animateOpen2(_charFacingDirection, 1);
							for (listIndex = 0; listIndex < 5; listIndex++) {
								invIndex = 0;
								while (_inventory[invIndex].code != _firstList[listIndex]) {
									invIndex += 1;
								}
								updateInventory(invIndex);
							}
							listIndex = 0;
							while (_inventory[listIndex].code != 149) {
								listIndex += 1;
							}
							updateInventory(listIndex);
							drawInventory();
							_mouse->show();
						} else {
							readTextFile();
							sayLine(_curObject->useTextRef, 255, 0, foo, false);
							_mouse->hide();
							animateGive(_charFacingDirection, 1);
							animateOpen2(_charFacingDirection, 1);
							_mouse->show();
							sayLine(foo, 253, 249, foo, true);
							_verbFile.close();
						}
					}
				}
				verifyList = false;
				if (_obtainedList2) {
					for (listIndex = 0; listIndex < 5; listIndex++)
						if (_inventory[usedObjectIndex].code == _secondList[listIndex])
							verifyList = true;
					if (verifyList) {
						int completedListItems = 0;
						for (listIndex = 0; listIndex < 5; listIndex++) {
							for (invIndex = 0; invIndex < kInventoryIconCount; invIndex++) {
								if (_inventory[invIndex].code == _secondList[listIndex]) {
									completedListItems += 1;
									break;
								}
							}
						}
						if (completedListItems == 5) {
							_obtainedList2 = false;
							_list2Complete = true;
							readTextFile();
							sayLine(1020, 255, 0, foo, false);
							sayLine(foo, 253, 249, foo, true);
							_verbFile.close();
							_mouse->hide();
							animateGive(_charFacingDirection, 1);
							animateOpen2(_charFacingDirection, 1);
							for (listIndex = 0; listIndex < 5; listIndex++) {
								invIndex = 0;
								while (_inventory[invIndex].code != _secondList[listIndex]) {
									invIndex += 1;
								}
								updateInventory(invIndex);
							}
							listIndex = 0;
							while (_inventory[listIndex].code != 150) {
								listIndex += 1;
							}
							updateInventory(listIndex);
							drawInventory();
							_mouse->show();
						} else {
							readTextFile();
							sayLine(_curObject->useTextRef, 255, 0, foo, false);
							_mouse->hide();
							animateGive(_charFacingDirection, 1);
							animateOpen2(_charFacingDirection, 1);
							_mouse->show();
							sayLine(foo, 253, 249, foo, true);
							_verbFile.close();
						}
					}
				}
			} break;
			case 159: {
				switch (_curObject->code) {
				case 173: {
					readTextFile();
					sayLine(1118, 255, 0, foo, false);
					_mouse->hide();
					animateGive(_charFacingDirection, 1);
					animateOpen2(_charFacingDirection, 1);
					_mouse->show();
					sayLine(foo, 253, 249, foo, true);
					_verbFile.close();
				} break;
				case 218: {
					readTextFile();
					sayLine(687, 255, 0, foo, false);
					_mouse->hide();
					animateGive(_charFacingDirection, 1);
					animateOpen2(_charFacingDirection, 1);
					_mouse->show();
					sayLine(foo, 253, 249, foo, true);
					_verbFile.close();
					_mouse->hide();
					updateInventory(usedObjectIndex);
					drawInventory();
					_mouse->show();
					for (foo = 0; foo < kCharacterCount; foo++)
						_bookTopic[foo] = true;
					_firstTimeTopicB[0] = true;
				} break;
				}
			} break;
			case 160: {
				readTextFile();
				sayLine(2466, 255, 0, foo, false);
				_mouse->hide();
				animateGive(_charFacingDirection, 1);
				animateOpen2(_charFacingDirection, 1);
				_mouse->show();
				sayLine(foo, 253, 249, foo, true);
				_verbFile.close();
				_mouse->hide();
				updateInventory(usedObjectIndex);
				drawInventory();
				_mouse->show();
				for (foo = 0; foo < kCharacterCount; foo++) {
					_mintTopic[foo] = true;
					_firstTimeTopicC[foo] = true;
				}
			} break;
			case 164: {
				switch (_curObject->code) {
				case 563: {
					_isPottersManualDelivered = true;
					if (_isPottersWheelDelivered && _isGreenDevilDelivered && _isMudDelivered) {
						readTextFile();
						sayLine(2696, 255, 0, foo, false);
						_mouse->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						_mouse->show();
						sayLine(foo, 253, 249, foo, true);
						_caves[4] = true;
						sayLine(3247, 253, 249, foo, true);
						_verbFile.close();
						_mouse->hide();
						replaceBackpack(usedObjectIndex, 676);
						drawInventory();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
						}
						_currentRoomData->mouseGrid[15][12] = 7;
						_mouse->show();
					} else {
						readTextFile();
						sayLine(2696, 255, 0, foo, false);
						_mouse->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						_mouse->show();
						sayLine(foo, 253, 249, foo, true);
						sayLine(3246, 253, 249, foo, true);
						_verbFile.close();
						_mouse->hide();
						updateInventory(usedObjectIndex);
						drawInventory();
						_mouse->show();
					}
				} break;
				case 598: {
					_isMudDelivered = true;
					if (_isPottersWheelDelivered && _isGreenDevilDelivered && _isPottersManualDelivered) {
						readTextFile();
						sayLine(2821, 255, 0, foo, false);
						_mouse->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						_mouse->show();
						sayLine(foo, 253, 249, foo, true);
						_caves[4] = true;
						sayLine(3247, 253, 249, foo, true);
						_verbFile.close();
						_mouse->hide();
						replaceBackpack(usedObjectIndex, 676);
						drawInventory();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
						}
						_currentRoomData->mouseGrid[15][12] = 7;
						_mouse->show();
					} else {
						readTextFile();
						sayLine(2821, 255, 0, foo, false);
						_mouse->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						_mouse->show();
						sayLine(foo, 253, 249, foo, true);
						sayLine(3246, 253, 249, foo, true);
						_verbFile.close();
						_mouse->hide();
						updateInventory(usedObjectIndex);
						drawInventory();
						_mouse->show();
					}
				} break;
				case 623: {
					_isPottersWheelDelivered = true;
					if (_isMudDelivered && _isGreenDevilDelivered && _isPottersManualDelivered) {
						readTextFile();
						sayLine(2906, 255, 0, foo, false);
						_mouse->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						_mouse->show();
						sayLine(foo, 253, 249, foo, true);
						_caves[4] = true;
						sayLine(3247, 253, 249, foo, true);
						_verbFile.close();
						_mouse->hide();
						replaceBackpack(usedObjectIndex, 676);
						drawInventory();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
						}
						_currentRoomData->mouseGrid[15][12] = 7;
						_mouse->show();
					} else {
						readTextFile();
						sayLine(2906, 255, 0, foo, false);
						_mouse->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						_mouse->show();
						sayLine(foo, 253, 249, foo, true);
						sayLine(3246, 253, 249, foo, true);
						_verbFile.close();
						_mouse->hide();
						updateInventory(usedObjectIndex);
						drawInventory();
						_mouse->show();
					}
				} break;
				case 701: {
					_isGreenDevilDelivered = true;
					if (_isMudDelivered && _isPottersWheelDelivered && _isPottersManualDelivered) {
						readTextFile();
						sayLine(3188, 255, 0, foo, false);
						_mouse->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						_mouse->show();
						sayLine(foo, 253, 249, foo, true);
						_caves[4] = true;
						sayLine(3247, 253, 249, foo, true);
						_verbFile.close();
						_mouse->hide();
						replaceBackpack(usedObjectIndex, 676);
						drawInventory();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
						}
						_currentRoomData->mouseGrid[15][12] = 7;
						_mouse->show();
					} else {
						readTextFile();
						sayLine(3188, 255, 0, foo, false);
						_mouse->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						_mouse->show();
						sayLine(foo, 253, 249, foo, true);
						sayLine(3246, 253, 249, foo, true);
						_verbFile.close();
						_mouse->hide();
						updateInventory(usedObjectIndex);
						drawInventory();
						_mouse->show();
					}
				} break;
				}
			} break;
			case 165: {
				drawText(1098);
				_mouse->hide();
				do {
					if (_iframe2 >= _secondaryAnimationFrameCount - 1)
						_iframe2 = 0;
					else
						_iframe2++;
					if (_currentSecondaryTrajectoryIndex >= _currentRoomData->secondaryTrajectoryLength)
						_currentSecondaryTrajectoryIndex = 1;
					else
						_currentSecondaryTrajectoryIndex += 1;
					_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
					_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
					_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
					emptyLoop();
					_chrono->_gameTick = false;
					emptyLoop2();
					sprites(true);
				} while (!(_currentSecondaryTrajectoryIndex == (_currentRoomData->secondaryTrajectoryLength / 2)));

				animateGive(3, 2);
				updateInventory(usedObjectIndex);
				drawInventory();
				animateOpen2(3, 2);
				animatedSequence(6);

				do {
					if (_iframe2 >= _secondaryAnimationFrameCount - 1)
						_iframe2 = 0;
					else
						_iframe2++;
					if (_currentSecondaryTrajectoryIndex >= _currentRoomData->secondaryTrajectoryLength)
						_currentSecondaryTrajectoryIndex = 1;
					else
						_currentSecondaryTrajectoryIndex += 1;
					_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
					_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
					_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
					emptyLoop();
					_chrono->_gameTick = false;

					emptyLoop2();
					sprites(true);
				} while (_currentSecondaryTrajectoryIndex != _currentRoomData->secondaryTrajectoryLength);
				disableSecondAnimation();
				_graphics->drawScreen(_sceneBackground);
				_mouse->show();
			} break;
			case 201: {
				drawText(_curObject->useTextRef);
				_sound->playVoc("GALLO", 94965, 46007);
				_mouse->hide();
				animatedSequence(5);
				replaceBackpack(usedObjectIndex, 423);
				drawInventory();
				_mouse->show();
				_actionCode = 0;
				_oldGridX = 0;
				_oldGridY = 0;
				checkMouseGrid();
			} break;
			case 219: {
				drawText(_curObject->useTextRef);
				_mouse->hide();
				_sound->playVoc("TAZA", 223698, 29066);
				animateGive(3, 2);
				do {
					_chrono->updateChrono();
					if (_chrono->_gameTick) {
						_graphics->advancePaletteAnim();
						_chrono->_gameTick = false;
					}
					_screen->update();
					g_system->delayMillis(10);
				} while (_sound->isVocPlaying());
				animateOpen2(3, 2);
				updateObject(_curObject->code);
				disableSecondAnimation();
				_graphics->drawScreen(_sceneBackground);
				_mouse->show();
				drawText(2652);
				_mouse->hide();
				readObject(536);
				for (int i = 12; i <= 13; i++)
					for (int j = 7; j <= 14; j++)
						_currentRoomData->mouseGrid[i][j] = 14;
				for (int i = 8; i <= 12; i++)
					_currentRoomData->mouseGrid[14][i] = 14;
				_currentRoomData->mouseGrid[9][10] = 1;
				_currentRoomData->mouseGrid[10][10] = 1;
				for (int i = 0; i < 15; i++)
					if (_currentRoomData->screenLayers[i].bitmapPointer == _curObject->bitmapPointer) {
						_currentRoomData->screenLayers[i].bitmapPointer = 0;
						_currentRoomData->screenLayers[i].bitmapSize = 0;
						_currentRoomData->screenLayers[i].coordx = 0;
						_currentRoomData->screenLayers[i].coordy = 0;
						_currentRoomData->screenLayers[i].depth = 0;
					}
				usedObjectIndex = 0;
				while (_inventory[usedObjectIndex].code != 0) {
					usedObjectIndex += 1;
				}
				_inventory[usedObjectIndex].bitmapIndex = _curObject->objectIconBitmap;
				_inventory[usedObjectIndex].code = _curObject->code;
				_inventory[usedObjectIndex].objectName = _curObject->name;
				animatedSequence(4);
				_mouse->show();
				_actionCode = 0;
				_oldGridX = 0;
				_oldGridY = 0;
				checkMouseGrid();
			} break;
			case 221: {
				drawText(_curObject->useTextRef);
				_curObject->used[0] = 9;
				usedObjectIndex = 0;
				while (_inventory[usedObjectIndex].code != 0) {
					usedObjectIndex += 1;
				}

				_sceneObjectsData->seek(_curObject->code);

				saveObjectsData(_curObject, _sceneObjectsData);
				readObject(_sceneObjectsData, 221, _curObject);
				_inventory[usedObjectIndex].bitmapIndex = _curObject->objectIconBitmap;
				_inventory[usedObjectIndex].code = _curObject->code;
				_inventory[usedObjectIndex].objectName = _curObject->name;

				_mouse->hide();
				animatePickup1(2, 0);
				_sound->playVoc("TIJERAS", 252764, 5242);
				_sound->waitForSoundEnd();
				animatePickup2(2, 0);
				drawInventory();
				_mouse->show();
			} break;
			case 227: {
				drawText(_curObject->useTextRef);
				_mouse->hide();
				_sound->playVoc("ALACENA", 319112, 11809);
				animatePickup1(0, 2);
				_sound->waitForSoundEnd();
				animateOpen2(0, 2);
				replaceBackpack(usedObjectIndex, 453);
				drawInventory();
				_mouse->show();
				updateObject(_inventory[usedObjectIndex].code);
				_isCupboardOpen = true;
			} break;
			case 274: {
				drawText(_curObject->useTextRef);
				_mouse->hide();
				_sound->playVoc("CINCEL", 334779, 19490);
				animatePickup1(_charFacingDirection, 2);
				_sound->waitForSoundEnd();
				animateOpen2(_charFacingDirection, 2);
				_mouse->show();
				updateObject(_curObject->code);
				_isChestOpen = true;
			} break;
			case 416: {
				updateObject(_curObject->code);
				_mouse->hide();
				_sound->playVoc("PUERTA", 186429, 4754);
				animatePickup1(0, 1);
				animateOpen2(0, 1);
				_sound->waitForSoundEnd();
				_sound->loadVoc("GOTA", 140972, 1029);
				_mouse->show();
				drawText(_curObject->useTextRef);
				_currentRoomData->doors[2].openclosed = 0;
			} break;
			case 446: {
				drawText(_curObject->useTextRef);
				_mouse->hide();
				_sound->playVoc("TAZA", 223698, 29066);
				animateGive(0, 2);
				_sound->waitForSoundEnd();
				animatePickup2(0, 2);
				replaceBackpack(usedObjectIndex, 204);
				drawInventory();
				_mouse->show();
			} break;
			case 507: {
				_mouse->hide();
				animatePickup1(0, 1);
				_sound->playVoc("MAQUINA", 153470, 7378);
				animateOpen2(0, 1);
				updateInventory(usedObjectIndex);
				drawInventory();
				_mouse->show();
				_currentRoomData->mouseGrid[27][8] = 22;
			} break;
			case 549: {
				updateObject(_curObject->code);
				_mouse->hide();
				_sound->playVoc("PUERTA", 186429, 4754);
				animatePickup1(1, 1);
				animateOpen2(1, 1);
				_mouse->show();
				drawText(_curObject->useTextRef);
				_currentRoomData->doors[0].openclosed = 0;
			} break;
			case 562: { // put any object in the niches
				switch (_currentRoomData->code) {
				case 20: {
					if (_niche[0][_niche[0][3]] == 0) {

						if (_niche[0][3] == 0) {
							_niche[0][0] = _curObject->code;
							drawText(_curObject->useTextRef);
							_mouse->hide();
							_currentRoomData->screenObjectIndex[9]->objectName = "                    ";
							animateGive(3, 1);
							switch (_niche[0][0]) {
							case 561: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
								readBitmap(1182652, _screenLayers[0], 892, 319);
								_currentRoomData->screenLayers[1].bitmapPointer = 1182652;
							} break;
							case 615: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
								readBitmap(1181760, _screenLayers[0], 892, 319);
								_currentRoomData->screenLayers[1].bitmapPointer = 1181760;
							} break;
							}
							_currentRoomData->screenLayers[1].bitmapSize = 892;
							_currentRoomData->screenLayers[1].coordx = 66;
							_currentRoomData->screenLayers[1].coordy = 35;
							_currentRoomData->screenLayers[1].depth = 1;
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(_sceneBackground);
							animateOpen2(3, 1);
							updateInventory(usedObjectIndex);
							drawInventory();
							_mouse->show();
						} else {

							_niche[0][_niche[0][3]] = _curObject->code;
							_niche[1][3] += 1;
							_niche[0][3] -= 1;
							drawText(_curObject->useTextRef);
							_mouse->hide();
							animateGive(3, 1);
							switch (_curObject->code) {
							case 561:
								readBitmap(1182652, _screenLayers[0], 892, 319);
								break;
							case 615:
								readBitmap(1181760, _screenLayers[0], 892, 319);
								break;
							}
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(_sceneBackground);
							animateOpen2(3, 1);
							updateInventory(usedObjectIndex);
							drawInventory();
							_currentRoomData->screenObjectIndex[9]->objectName = "                    ";
							_sound->playVoc("PLATAF", 375907, 14724);
							switch (_niche[0][_niche[0][3]]) {
							case 0: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
								nicheAnimation(1, 1190768);
								_currentRoomData->screenLayers[1].bitmapPointer = 1190768;
							} break;
							case 561: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
								nicheAnimation(1, 1182652);
								_currentRoomData->screenLayers[1].bitmapPointer = 1182652;
							} break;
							case 563: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
								nicheAnimation(1, 1186044);
								_currentRoomData->screenLayers[1].bitmapPointer = 1186044;
							} break;
							case 615: {
								_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
								nicheAnimation(1, 1181760);
								_currentRoomData->screenLayers[1].bitmapPointer = 1181760;
							} break;
							}
							_currentRoomData->screenLayers[1].bitmapSize = 892;
							_currentRoomData->screenLayers[1].coordx = 66;
							_currentRoomData->screenLayers[1].coordy = 35;
							_currentRoomData->screenLayers[1].depth = 1;
							_mouse->show();
							updateAltScreen(24);
						}
					} else {
						drawText(getRandom(11) + 1022);
					}
				} break;
				case 24: {
					if (_niche[1][_niche[1][3]] == 0) {

						if (_niche[1][3] == 0) {

							_niche[1][0] = _curObject->code;
							drawText(_curObject->useTextRef);
							_mouse->hide();
							_currentRoomData->screenObjectIndex[8]->objectName = "                    ";
							animateGive(0, 1);
							switch (_niche[1][0]) {
							case 561: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
								readBitmap(1381982, _screenLayers[0], 892, 319);
								_currentRoomData->screenLayers[0].bitmapPointer = 1381982;
							} break;
							case 615: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
								readBitmap(1381090, _screenLayers[0], 892, 319);
								_currentRoomData->screenLayers[0].bitmapPointer = 1381090;
							} break;
							}
							_currentRoomData->screenLayers[0].bitmapSize = 892;
							_currentRoomData->screenLayers[0].coordx = 217;
							_currentRoomData->screenLayers[0].coordy = 48;
							_currentRoomData->screenLayers[0].depth = 1;
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(_sceneBackground);
							animateOpen2(0, 1);
							updateInventory(usedObjectIndex);
							drawInventory();
							_mouse->show();
						} else {

							_niche[1][_niche[1][3]] = _curObject->code;
							_niche[0][3] += 1;
							_niche[1][3] -= 1;
							drawText(_curObject->useTextRef);
							_mouse->hide();
							animateGive(0, 1);

							switch (_curObject->code) {
							case 561:
								readBitmap(1381982, _screenLayers[_curObject->depth - 1],
										   892, 319);
								break;
							case 615:
								readBitmap(1381090, _screenLayers[_curObject->depth - 1],
										   892, 319);
								break;
							}
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(_sceneBackground);
							animateOpen2(0, 1);
							updateInventory(usedObjectIndex);
							drawInventory();
							_currentRoomData->screenObjectIndex[8]->objectName = "                    ";
							_sound->playVoc("PLATAF", 375907, 14724);
							switch (_niche[1][_niche[1][3]]) {
							case 0: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
								nicheAnimation(1, 1399610);
								_currentRoomData->screenLayers[0].bitmapPointer = 1399610;
							} break;
							case 561: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
								nicheAnimation(1, 1381982);
								_currentRoomData->screenLayers[0].bitmapPointer = 1381982;
							} break;
							case 615: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
								nicheAnimation(1, 1381090);
								_currentRoomData->screenLayers[0].bitmapPointer = 1381090;
							} break;
							case 622: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
								nicheAnimation(1, 1400502);
								_currentRoomData->screenLayers[0].bitmapPointer = 1400502;
							} break;
							case 623: {
								_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
								nicheAnimation(1, 1398718);
								_currentRoomData->screenLayers[0].bitmapPointer = 1398718;
							} break;
							}
							_currentRoomData->screenLayers[0].bitmapSize = 892;
							_currentRoomData->screenLayers[0].coordx = 217;
							_currentRoomData->screenLayers[0].coordy = 48;
							_currentRoomData->screenLayers[0].depth = 1;
							_mouse->show();
							updateAltScreen(20);
						}
					} else {
						drawText(getRandom(11) + 1022);
					}
				} break;
				}
			} break;
			case 583: {

				int32 offset = getOffsetsByCurrentLanguage()[20];

				drawText(_curObject->useTextRef);
				_mouse->hide();
				drawFlc(140, 34, offset, 0, 9, 24, false, false, true, foobar);
				_mouse->show();
				updateObject(_curObject->code);
				_currentRoomData->screenObjectIndex[7]->fileIndex = 716;
				_currentRoomData->mouseGrid[19][9] = 14;
				_currentRoomData->mouseGrid[22][16] = 15;
				for (listIndex = 21; listIndex <= 22; listIndex++)
					for (invIndex = 17; invIndex <= 20; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 17;
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

					with.bitmapPointer = 1243652;
					with.bitmapSize = 2718;
					with.coordx = 127;
					with.coordy = 36;
					with.depth = 6;
				}
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[1];

					with.bitmapPointer = 1240474;
					with.bitmapSize = 344;
					with.coordx = 168;
					with.coordy = 83;
					with.depth = 12;
					loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[2];

					with.bitmapPointer = 1240818;
					with.bitmapSize = 116;
					with.coordx = 177;
					with.coordy = 82;
					with.depth = 1;
					loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				readBitmap(1243652, _screenLayers[5], 2718, 319);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
			} break;
			case 594: {
				drawText(_curObject->useTextRef);
				_mouse->hide();
				animateGive(3, 2);
				animatePickup2(3, 2);
				replaceBackpack(usedObjectIndex, 607);
				drawInventory();
				_mouse->show();
			} break;
			case 608: {
				drawText(_curObject->useTextRef);
				goToObject(_currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY], 26);
				_mouse->hide();
				animateGive(2, 2);
				animateOpen2(2, 2);
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[3];

					with.bitmapPointer = 1546096;
					with.bitmapSize = 372;
					with.coordx = 208;
					with.coordy = 105;
					with.depth = 4;
					loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				updateInventory(usedObjectIndex);
				drawInventory();
				_isTrapSet = true;
				_mouse->show();
			} break;
			case 632: {
				int32 offset = getOffsetsByCurrentLanguage()[21];
				drawText(_curObject->useTextRef);
				_mouse->hide();
				animateGive(_charFacingDirection, 1);

				// Show feather on pedestal
				loadScreenLayer(187, 70, 104, 1545820, 8);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				_screen->update();

				animateOpen2(_charFacingDirection, 1);
				_mouse->show();
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 14);
				_mouse->hide();
				_sound->playVoc("PUFF", 191183, 18001);
				// Animate to scythe
				drawFlc(180, 60, offset, 0, 9, 0, false, false, true, foobar);
				// load Scythe
				_currentRoomData->screenLayers[2].bitmapPointer = 1545820;
				_currentRoomData->screenLayers[2].bitmapSize = 104;
				_currentRoomData->screenLayers[2].coordx = 277;
				_currentRoomData->screenLayers[2].coordy = 104;
				_currentRoomData->screenLayers[2].depth = 1;
				_depthMap[0].posy = 104;
				readBitmap(1545820, _screenLayers[0], 104, 319);

				_currentRoomData->screenLayers[4].bitmapPointer = 1447508;
				_currentRoomData->screenLayers[4].bitmapSize = 464;
				_currentRoomData->screenLayers[4].coordx = 186;
				_currentRoomData->screenLayers[4].coordy = 64;
				_currentRoomData->screenLayers[4].depth = 8;
				loadScreenLayer(186, 63, 464, 1447508, 8);

				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				_mouse->show();
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 18);
				_mouse->hide();
				animatePickup1(1, 1);
				replaceBackpack(usedObjectIndex, 638);
				_currentRoomData->screenLayers[4].bitmapPointer = 0;
				_currentRoomData->screenLayers[4].bitmapSize = 0;
				_currentRoomData->screenLayers[4].coordx = 0;
				_currentRoomData->screenLayers[4].coordy = 0;
				_currentRoomData->screenLayers[4].depth = 0;
				clearScreenLayer(7);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animatePickup2(1, 1);
				drawInventory();
				_mouse->show();
				for (listIndex = 35; listIndex <= 37; listIndex++)
					for (invIndex = 21; invIndex <= 25; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 11;
				_isScytheTaken = true;
				if (_isTridentTaken)
					_caves[3] = true;
			} break;
			case 633: { //Use ring!
				int32 offset = getOffsetsByCurrentLanguage()[22];

				drawText(_curObject->useTextRef);
				_mouse->hide();
				animateGive(3, 1);
				loadScreenLayer(86, 55, 92, 1591272, 8);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animateOpen2(3, 1);
				_mouse->show();
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 10);
				_mouse->hide();
				_sound->playVoc("PUFF", 191183, 18001);
				drawFlc(0, 47, offset, 0, 9, 0, false, false, true, foobar);

				_currentRoomData->screenLayers[3].bitmapPointer = 1591272;
				_currentRoomData->screenLayers[3].bitmapSize = 92;
				_currentRoomData->screenLayers[3].coordx = 18;
				_currentRoomData->screenLayers[3].coordy = 60;
				_currentRoomData->screenLayers[3].depth = 3;
				_depthMap[2].posx = 18;
				_depthMap[2].posy = 60;
				readBitmap(1591272, _screenLayers[2], 92, 319);

				_currentRoomData->screenLayers[4].bitmapPointer = 1746554;
				_currentRoomData->screenLayers[4].bitmapSize = 384;
				_currentRoomData->screenLayers[4].coordx = 82;
				_currentRoomData->screenLayers[4].coordy = 53;
				_currentRoomData->screenLayers[4].depth = 8;
				loadScreenLayer(82, 53, 384, 1746554, 8);
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				_mouse->show();
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 15);
				_mouse->hide();
				animatePickup1(3, 1);
				replaceBackpack(usedObjectIndex, 637);
				_currentRoomData->screenLayers[4].bitmapPointer = 0;
				_currentRoomData->screenLayers[4].bitmapSize = 0;
				_currentRoomData->screenLayers[4].coordx = 0;
				_currentRoomData->screenLayers[4].coordy = 0;
				_currentRoomData->screenLayers[4].depth = 0;
				clearScreenLayer(7);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animatePickup2(3, 1);
				drawInventory();
				_mouse->show();
				for (listIndex = 0; listIndex <= 2; listIndex++)
					for (invIndex = 10; invIndex <= 12; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 10;
				_isTridentTaken = true;
				if (_isScytheTaken)
					_caves[3] = true;
			} break;
			case 643: { // Urn with altar
				int32 offset = getOffsetsByCurrentLanguage()[23];

				if (_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount] != 5)
					drawText(_curObject->useTextRef);
				_mouse->mouseClickX = 149 - 7;
				_mouse->mouseClickY = 126 - 7;
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 5);
				_mouse->hide();
				updateInventory(usedObjectIndex);
				drawInventory();
				drawFlc(133, 0, offset, 0, 9, 22, false, false, true, foobar);
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[2];

					with.bitmapPointer = 1744230;
					with.bitmapSize = 824;
					with.coordx = 147;
					with.coordy = 38;
					with.depth = 9;
					loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				updateAltScreen(31);
				for (listIndex = 18; listIndex <= 20; listIndex++)
					for (invIndex = 8; invIndex <= 14; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 12;
				_isSealRemoved = true;
				_caves[1] = false;
				_caves[0] = false;
				_caves[4] = false;
				_mouse->show();
			} break;
			case 657: { // sharpen scythe
				int32 offset = getOffsetsByCurrentLanguage()[24];

				drawText(_curObject->useTextRef);
				_mouse->mouseClickX = 178 - 7;
				_mouse->mouseClickY = 71 - 7;
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 3);
				_mouse->hide();
				_sound->playVoc("AFILAR", 0, 6433);
				drawFlc(160, 15, offset, 0, 9, 23, false, false, true, foobar);
				replaceBackpack(usedObjectIndex, 715);
				drawInventory();
				_mouse->show();
			} break;
			case 686: {
				drawText(_curObject->useTextRef);
				_isVasePlaced = true;
				_caves[4] = false;
				_mouse->hide();
				animateGive(1, 1);
				updateInventory(usedObjectIndex);
				dropObjectInScreen(_curObject);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				drawInventory();
				animateOpen2(1, 1);
				for (listIndex = 19; listIndex <= 21; listIndex++)
					for (invIndex = 10; invIndex <= 13; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 13;
				_mouse->show();
			} break;
			case 689: { // rope
				int32 offset = getOffsetsByCurrentLanguage()[25];

				drawText(_curObject->useTextRef);
				_mouse->mouseClickX = 124 - 7;
				_mouse->mouseClickY = 133 - 7;
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 9);
				_mouse->hide();
				drawFlc(110, 79, offset, 0, 9, 0, false, false, true, foobar);
				replaceBackpack(usedObjectIndex, 701);
				drawInventory();
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				_mouse->show();
				for (listIndex = 18; listIndex <= 20; listIndex++)
					_currentRoomData->mouseGrid[listIndex][26] = 10;
				for (listIndex = 17; listIndex <= 21; listIndex++)
					_currentRoomData->mouseGrid[listIndex][27] = 10;
			} break;
			case 700: { // Trident
				int32 offset = getOffsetsByCurrentLanguage()[26];
				drawText(_curObject->useTextRef);
				_mouse->mouseClickX = 224 - 7;
				_mouse->mouseClickY = 91 - 7;
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 7);
				_mouse->hide();
				drawFlc(208, 0, offset, 0, 9, 21, false, false, true, foobar);
				_currentRoomData->screenLayers[0].bitmapPointer = 0;
				_currentRoomData->screenLayers[0].bitmapSize = 0;
				_currentRoomData->screenLayers[0].coordx = 0;
				_currentRoomData->screenLayers[0].coordy = 0;
				_currentRoomData->screenLayers[0].depth = 0;
				clearScreenLayer(2);
				for (invIndex = 6; invIndex <= 9; invIndex++)
					_currentRoomData->mouseGrid[26][invIndex] = 3;
				for (invIndex = 3; invIndex <= 5; invIndex++)
					_currentRoomData->mouseGrid[27][invIndex] = 3;
				for (invIndex = 6; invIndex <= 10; invIndex++)
					_currentRoomData->mouseGrid[27][invIndex] = 4;
				for (invIndex = 11; invIndex <= 12; invIndex++)
					_currentRoomData->mouseGrid[27][invIndex] = 7;
				for (invIndex = 2; invIndex <= 10; invIndex++)
					_currentRoomData->mouseGrid[28][invIndex] = 4;
				for (invIndex = 11; invIndex <= 12; invIndex++)
					_currentRoomData->mouseGrid[28][invIndex] = 7;
				_currentRoomData->mouseGrid[28][13] = 4;
				for (invIndex = 1; invIndex <= 14; invIndex++)
					_currentRoomData->mouseGrid[29][invIndex] = 4;
				for (listIndex = 30; listIndex <= 32; listIndex++)
					for (invIndex = 0; invIndex <= 15; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 4;
				for (invIndex = 1; invIndex <= 14; invIndex++)
					_currentRoomData->mouseGrid[33][invIndex] = 4;
				for (invIndex = 2; invIndex <= 14; invIndex++)
					_currentRoomData->mouseGrid[34][invIndex] = 4;
				for (invIndex = 3; invIndex <= 8; invIndex++)
					_currentRoomData->mouseGrid[35][invIndex] = 4;
				for (invIndex = 9; invIndex <= 11; invIndex++)
					_currentRoomData->mouseGrid[35][invIndex] = 7;
				_currentRoomData->doors[1].openclosed = 1;
				_mouse->show();
				updateObject(_curObject->code);
			} break;
			case 709: { // rock with mural
				int32 offset = getOffsetsByCurrentLanguage()[27];

				if (_isSealRemoved) {
					drawText(_curObject->useTextRef);
					_mouse->hide();
					animatePickup1(0, 1);
					_sound->playVoc("TIZA", 390631, 18774);
					{
						RoomBitmapRegister &with = _currentRoomData->screenLayers[1];

						with.bitmapPointer = 1745054;
						with.bitmapSize = 1500;
						with.coordx = 39;
						with.coordy = 16;
						with.depth = 1;
						loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
					}
					_graphics->restoreBackground();
					assembleScreen();
					_graphics->drawScreen(_sceneBackground);

					_sound->waitForSoundEnd();
					_sound->playVoc("PUFF", 191183, 18001);
					animateOpen2(0, 1);
					drawFlc(180, 50, offset, 0, 9, 22, false, false, true, foobar);
					_shouldQuitGame = true;
				} else
					drawText(getRandom(11) + 1022);
			} break;
			}
		} else {
			goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], _currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);
			if (_curObject->code == 536 || _curObject->code == 220)
				drawText(getRandom(6) + 1033);
			else
				drawText(getRandom(11) + 1022);
		}
	} else { //use object with something on the scene
		if (sceneObject > 0) {
			readObject(sceneObject);
			goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], _currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);
			switch (_curObject->used[0]) {
			case 0: {
				if (_curObject->useTextRef > 0)
					drawText(_curObject->useTextRef);
			} break;
			case 9: {
				if (_curObject->afterUseTextRef > 0)
					drawText(_curObject->afterUseTextRef);
			} break;
			case 100: {
				switch (_curObject->code) {
				case 153: {
					_mouse->hide();
					animatedSequence(2);
					_mouse->show();
				} break;
				case 154: {
					_curObject->used[0] = 9;
					if (_curObject->beforeUseTextRef > 0)
						drawText(_curObject->beforeUseTextRef);
					_mouse->hide();
					animatedSequence(1);
					_mouse->show();
					drawText(1425);
					_mouse->hide();
					animatedSequence(3);
					_mouse->show();
					updateObject(_curObject->code);
					readObject(152);
					usedObjectIndex = 0;
					while (_inventory[usedObjectIndex].code != 0) {
						usedObjectIndex += 1;
					}
					_inventory[usedObjectIndex].bitmapIndex = _curObject->objectIconBitmap;
					_inventory[usedObjectIndex].code = _curObject->code;
					_inventory[usedObjectIndex].objectName = _curObject->name;
					_mouse->hide();
					drawInventory();
					_mouse->show();
				} break;
				case 169: {
					_mouse->hide();
					animatePickup1(0, 1);
					animateOpen2(0, 1);
					_isTVOn = !(_isTVOn);
					if (_isTVOn) {
						_sound->playVoc("CLICK", 27742, 2458);
						_sound->waitForSoundEnd();
						_currentRoomData->paletteAnimationFlag = true;
						_sound->autoPlayVoc("PARASITO", 355778, 20129);
					} else {
						_sound->stopVoc();
						_sound->playVoc("CLICK", 27742, 2458);
						_currentRoomData->paletteAnimationFlag = false;
						for (int i = 195; i <= 200; i++) {
							g_engine->_graphics->_pal[i * 3 + 0] = 2 << 2;
							g_engine->_graphics->_pal[i * 3 + 1] = 2 << 2;
							g_engine->_graphics->_pal[i * 3 + 2] = 2 << 2;
							setRGBPalette(i, 2, 2, 2);
						}
					}
					_mouse->show();
				} break;
				case 347: {
					drawText(_curObject->useTextRef);
					_sound->stopVoc();
					_sound->playVoc("CLICK", 27742, 2458);
					_mouse->hide();
					animatePickup1(0, 0);
					delay(100);
					animateOpen2(0, 0);
					_sound->stopVoc();
					_sound->autoPlayVoc("CALDERA", 6433, 15386);
					_graphics->turnLightOn();
					_mouse->show();
					_currentRoomData->palettePointer = 1536;
					_currentRoomData->screenObjectIndex[1]->fileIndex = 424;
					_currentRoomData->doors[1].openclosed = 1;
				} break;
				case 359: {
					drawText(_curObject->useTextRef);
					_sound->stopVoc();
					_sound->playVoc("CARBON", 21819, 5923);
					_mouse->hide();
					animatePickup1(0, 0);
					delay(100);
					animateOpen2(0, 0);
					_mouse->show();
					updateObject(_curObject->code);
					_currentRoomData->screenObjectIndex[16]->fileIndex = 362;
					_currentRoomData->screenObjectIndex[16]->objectName = getObjectName(2);
					_currentRoomData->screenObjectIndex[1]->fileIndex = 347;
					_currentRoomData->screenObjectIndex[1]->objectName = getObjectName(3);
					_sound->stopVoc();
					_sound->autoPlayVoc("CALDERA", 6433, 15386);
				} break;
				case 682: {
					int32 offsetWithJar = getOffsetsByCurrentLanguage()[28];
					int32 offsetNoJar = getOffsetsByCurrentLanguage()[29];

					_mouse->hide();
					_sound->playVoc("CLICK", 27742, 2458);
					animatePickup1(0, 1);

					_sound->waitForSoundEnd();
					animateOpen2(0, 1);
					_sound->playVoc("FUEGO", 72598, 9789);
					if (_isVasePlaced) {
						drawFlc(108, 0, offsetWithJar, 0, 9, 0, false, false, true, foobar);
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1636796;
							with.bitmapSize = 628;
							with.coordx = 153;
							with.coordy = 48;
							with.depth = 1;
						}
						for (listIndex = 19; listIndex <= 21; listIndex++)
							for (invIndex = 10; invIndex <= 13; invIndex++)
								_currentRoomData->mouseGrid[listIndex][invIndex] = 12;
					} else
						drawFlc(108, 0, offsetNoJar, 0, 9, 0, false, false, true, foobar);
					_mouse->show();
				} break;
				}
				if ((_curObject->beforeUseTextRef > 0) && (_curObject->code != 154))
					drawText(_curObject->beforeUseTextRef);
			} break;
			default:
				drawText(1022 + getRandom(11));
			}
		}
	}
	_oldGridX = 0;
	_oldGridY = 0;
	_actionCode = 0;
	checkMouseGrid();
}

void TotEngine::openScreenObject() {
	byte xIndex, yIndex;
	bool shouldSpeak; //Whether the character should speak to reject the action or not

	Common::Point p = _mouse->getClickCoordsWithinGrid();
	uint correctedMouseX = p.x;
	uint correctedMouseY = p.y;
	uint screenObject = _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex;
	if (screenObject == 0)
		return;

	readObject(screenObject);
	debug("Read screen object = %s, with code = %d, depth=%d", _curObject->name.c_str(), _curObject->code, _curObject->depth);
	goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount],
			   _currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);

	if (_curObject->openable == false) {
		drawText(getRandom(9) + 1059);
		return;
	} else {
		shouldSpeak = false;
		switch (_curObject->code) {
		case 227:
			if (_isCupboardOpen == false)
				shouldSpeak = true;
			break;
		case 274:
			if (_isChestOpen == false)
				shouldSpeak = true;
			break;
		case 415:
			if (_currentRoomData->doors[2].openclosed == 2)
				shouldSpeak = true;
			else {
				_mouse->hide();
				animatePickup1(0, 1);
				clearScreenLayer(_curObject->depth - 1);
				yIndex = 0;
				while (yIndex < 14 && _currentRoomData->screenLayers[yIndex].depth != _curObject->depth) {
					yIndex++;
				}
				_currentRoomData->screenLayers[yIndex].bitmapPointer = 0;
				_currentRoomData->screenLayers[yIndex].bitmapSize = 0;
				_currentRoomData->screenLayers[yIndex].coordx = 0;
				_currentRoomData->screenLayers[yIndex].coordy = 0;
				_currentRoomData->screenLayers[yIndex].depth = 0;
				_currentRoomData->doors[2].openclosed = 1;
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animateOpen2(0, 1);
				_mouse->show();
				for (yIndex = 0; yIndex <= 12; yIndex++)
					for (xIndex = 33; xIndex <= 36; xIndex++)
						_currentRoomData->mouseGrid[xIndex][yIndex] = 43;
				for (xIndex = 33; xIndex <= 35; xIndex++)
					_currentRoomData->mouseGrid[xIndex][13] = 43;
				_actionCode = 0;
				_oldGridX = 0;
				_oldGridY = 0;
				_oldTargetZone = 0;
				checkMouseGrid();
				return;
			}
			break;
		case 548:
			if (_currentRoomData->doors[0].openclosed == 2)
				shouldSpeak = true;
			else {
				_mouse->hide();
				animatePickup1(1, 1);
				clearScreenLayer(_curObject->depth - 1);
				yIndex = 0;
				while (_currentRoomData->screenLayers[yIndex].depth != _curObject->depth && yIndex != 14) {
					yIndex++;
				}
				_currentRoomData->screenLayers[yIndex].bitmapPointer = 0;
				_currentRoomData->screenLayers[yIndex].bitmapSize = 0;
				_currentRoomData->screenLayers[yIndex].coordx = 0;
				_currentRoomData->screenLayers[yIndex].coordy = 0;
				_currentRoomData->screenLayers[yIndex].depth = 0;
				_currentRoomData->doors[0].openclosed = 1;
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(_sceneBackground);
				animateOpen2(1, 1);
				_mouse->show();
				xIndex = 30;
				for (yIndex = 17; yIndex <= 18; yIndex++)
					_currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				xIndex += 1;
				for (yIndex = 4; yIndex <= 20; yIndex++)
					_currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				xIndex += 1;
				for (yIndex = 0; yIndex <= 20; yIndex++)
					_currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				xIndex += 1;
				for (yIndex = 0; yIndex <= 17; yIndex++)
					_currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				xIndex += 1;
				for (yIndex = 0; yIndex <= 12; yIndex++)
					_currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				for (xIndex = 35; xIndex <= 39; xIndex++)
					for (yIndex = 0; yIndex <= 10; yIndex++)
						_currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				_actionCode = 0;
				_oldGridX = 0;
				_oldGridY = 0;
				checkMouseGrid();
				return;
			}
			break;
		}
		if (shouldSpeak) {
			drawText(getRandom(9) + 1059);
			return;
		}
		_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject->replaceWith;
		_mouse->hide();
		switch (_curObject->height) {
		case 0: {
			animatePickup1(_charFacingDirection, 0);
			updateVideo();
			animateOpen2(_charFacingDirection, 0);
		} break;
		case 1: {
			animatePickup1(_charFacingDirection, 1);
			updateVideo();
			animateOpen2(_charFacingDirection, 1);
		} break;
		case 2: {
			animatePickup1(_charFacingDirection, 2);
			updateVideo();
			animateOpen2(_charFacingDirection, 2);
		} break;
		}
		_mouse->show();
		for (yIndex = _curObject->ygrid1; yIndex <= _curObject->ygrid2; yIndex++)
			for (xIndex = _curObject->xgrid1; xIndex <= _curObject->xgrid2; xIndex++) {
				_currentRoomData->walkAreasGrid[xIndex][yIndex] = _curObject->walkAreasPatch[xIndex - _curObject->xgrid1][yIndex - _curObject->ygrid1];
				_currentRoomData->mouseGrid[xIndex][yIndex] = _curObject->mouseGridPatch[xIndex - _curObject->xgrid1][yIndex - _curObject->ygrid1];
			}
		for (xIndex = 0; xIndex < 15; xIndex++)
			if (_currentRoomData->screenLayers[xIndex].bitmapPointer == _curObject->bitmapPointer) {
				_currentRoomData->screenLayers[xIndex].bitmapPointer = _curObject->dropOverlay;
				_currentRoomData->screenLayers[xIndex].bitmapSize = _curObject->dropOverlaySize;
			}
		_actionCode = 0;
	}
	_oldGridX = 0;
	_oldGridY = 0;
	checkMouseGrid();
}

void TotEngine::closeScreenObject() {
	byte xIndex, yIndex;
	bool shouldSpeak;
	uint sceneObject;
	Common::Point p = _mouse->getClickCoordsWithinGrid();
	uint correctedMouseX = p.x;
	uint correctedMouseY = p.y;
	sceneObject = _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex;
	if (sceneObject == 0)
		return;
	readObject(sceneObject);
	goToObject(_currentRoomData->walkAreasGrid[((_characterPosX + kCharacterCorrectionX) / kXGridCount)][((_characterPosY + kCharacerCorrectionY) / kYGridCount)],
			   _currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);
	if (_curObject->closeable == false) {
		drawText((getRandom(10) + 1068));
		return;
	} else {
		shouldSpeak = false;
		switch (_curObject->code) {
		case 224:
		case 226:
			if (_isCupboardOpen == false)
				shouldSpeak = true;
			break;
		case 275:
		case 277:
			if (_isChestOpen == false)
				shouldSpeak = true;
			break;
		}
		if (shouldSpeak) {
			drawText(getRandom(10) + 1068);
			return;
		}
		_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject->replaceWith;
		_mouse->hide();
		switch (_curObject->height) {
		case 0: {
			animatePickup1(_charFacingDirection, 0);
			updateVideo();
			animateOpen2(_charFacingDirection, 0);
		} break;
		case 1: {
			animatePickup1(_charFacingDirection, 1);
			updateVideo();
			animateOpen2(_charFacingDirection, 1);
		} break;
		case 2: {
			animatePickup1(_charFacingDirection, 2);
			updateVideo();
			animateOpen2(_charFacingDirection, 2);
		} break;
		}
		_mouse->show();
		for (yIndex = _curObject->ygrid1; yIndex <= _curObject->ygrid2; yIndex++)
			for (xIndex = _curObject->xgrid1; xIndex <= _curObject->xgrid2; xIndex++) {
				_currentRoomData->walkAreasGrid[xIndex][yIndex] = _curObject->walkAreasPatch[xIndex - _curObject->xgrid1][yIndex - _curObject->ygrid1];
				_currentRoomData->mouseGrid[xIndex][yIndex] = _curObject->mouseGridPatch[xIndex - _curObject->xgrid1][yIndex - _curObject->ygrid1];
			}
		for (xIndex = 0; xIndex < 15; xIndex++)
			if (_currentRoomData->screenLayers[xIndex].bitmapPointer == _curObject->bitmapPointer) {
				_currentRoomData->screenLayers[xIndex].bitmapPointer = _curObject->dropOverlay;
				_currentRoomData->screenLayers[xIndex].bitmapSize = _curObject->dropOverlaySize;
			}
		_actionCode = 0;
	}
	_oldGridX = 0;
	_oldGridY = 0;
	checkMouseGrid();
}

void TotEngine::action() {
	bar(0, 140, 319, 149, 0);
	Common::String actionLine;
	switch (_actionCode) {
	case 0:
		actionLine = getActionLineText(0);
		break;
	case 1:
		actionLine = getActionLineText(1);
		break;
	case 2:
		actionLine = getActionLineText(2);
		break;
	case 3:
		actionLine = getActionLineText(3);
		break;
	case 4: {
		actionLine = getActionLineText(4);
		_inventoryObjectName = "";
	} break;
	case 5:
		actionLine = getActionLineText(5);
		break;
	case 6:
		actionLine = getActionLineText(6);
		break;
	}
	actionLineText(actionLine);
}

void TotEngine::handleAction(byte invPos) {

	bar(0, 140, 319, 149, 0);
	switch (_actionCode) {
	case 1: {
		_mouse->hide();
		actionLineText(getActionLineText(1) + _inventory[invPos].objectName);
		_mouse->show();
		drawText((getRandom(10) + 1039));
		_actionCode = 0;
		if (_cpCounter > 130)
			showError(274);
		_oldGridX = 0;
		_oldGridY = 0;
		checkMouseGrid();
	} break;
	case 2: {
		_mouse->hide();
		actionLineText(getActionLineText(2) + _inventory[invPos].objectName);
		if (_cpCounter2 > 13)
			showError(274);
		_mouse->show();
		drawText((getRandom(10) + 1049));
		_actionCode = 0;
		_oldGridX = 0;
		_oldGridY = 0;
		checkMouseGrid();
	} break;
	case 3: {
		_mouse->hide();
		actionLineText(getActionLineText(3) + _inventory[invPos].objectName);
		_mouse->show();
		_actionCode = 0;
		lookAtObject(invPos);
		_oldGridX = 0;
		_oldGridY = 0;
		checkMouseGrid();
	} break;
	case 4:
		if (_inventoryObjectName == "") {
			_mouse->hide();
			actionLineText(getActionLineText(4) + _inventory[invPos].objectName + getActionLineText(7));
			_mouse->show();
			_inventoryObjectName = _inventory[invPos].objectName;
			_backpackObjectCode = _inventory[invPos].code;
		} else {

			_actionCode = 0;
			if (_cpCounter > 25)
				showError(274);
			useInventoryObjectWithInventoryObject(_backpackObjectCode, _inventory[invPos].code);
			_oldGridX = 0;
			_oldGridY = 0;
			checkMouseGrid();
		}
		break;
	case 5: {
		_mouse->hide();
		actionLineText(getActionLineText(5) + _inventory[invPos].objectName);
		_mouse->show();
		drawText(getRandom(9) + 1059);
		_actionCode = 0;
		_oldGridX = 0;
		_oldGridY = 0;
		checkMouseGrid();
	} break;
	case 6: {
		_mouse->hide();
		actionLineText(getActionLineText(6) + _inventory[invPos].objectName);
		_mouse->show();
		drawText(getRandom(10) + 1068);
		_actionCode = 0;
		if (_cpCounter2 > 35)
			showError(274);
		_oldGridX = 0;
		_oldGridY = 0;
		checkMouseGrid();
	} break;
	}
}

void TotEngine::loadInventory() {

	Common::File objectFile;
	switch (_gamePart) {
	case 1:
		objectFile.open("OBJMOCH.DAT");
		break;
	case 2:
		objectFile.open("OBJMOCH.TWO");
		break;
	}

	if (!objectFile.isOpen()) {
		debug("Object file exists? %d, open? %d, gamepart? %d", objectFile.exists("OBJMOCH.DAT"), objectFile.isOpen(), _gamePart);
		showError(312);
	}
	for (int i = 0; i < kInventoryIconCount; i++) {
		_inventoryIconBitmaps[i] = (byte *)malloc(kInventoryIconSize);
		objectFile.read(_inventoryIconBitmaps[i], kInventoryIconSize);
	}
	if (_cpCounter > 65)
		showError(274);
	const char *emptyName = getHardcodedTextsByCurrentLanguage()[10];
	for (int i = 0; i < kInventoryIconCount; i++) {
		_inventory[i].bitmapIndex = 34;
		_inventory[i].code = 0;
		_inventory[i].objectName = emptyName;
	}

	objectFile.close();
}

void TotEngine::obtainName(Common::String &playerName) {
	uint16 namePromptBGSize;
	byte *namePromptBG;

	namePromptBGSize = imagesize(84, 34, 235, 80);
	namePromptBG = (byte *)malloc(namePromptBGSize);
	_graphics->getImg(84, 34, 235, 80, namePromptBG);
	drawMenu(8);
	_screen->update();
	Common::String prompt;
	readAlphaGraph(prompt, 8, 125, 62, 252);
	playerName = prompt.c_str();
	_graphics->putImg(84, 34, namePromptBG);
	_screen->update();
	free(namePromptBG);
}

/**
 * Blits srcImage over dstImage on the zeroed pixels of dstImage
 */
static void blit(byte *srcImage, byte *dstImage) { // Near;
	uint16 w = READ_LE_UINT16(dstImage) + 1;
	uint16 h = READ_LE_UINT16(dstImage + 2) + 1;

	uint size = w * h;
	byte *dst = dstImage + 4;
	byte *src = srcImage + 4;
	for (uint i = 0; i < size; i++) {
		if (dst[i] == 0) {
			dst[i] = src[i];
		}
	}
}

/**
 * Grabs the action area of the screen into a pointer
 */
static void getScreen(byte *bg) {
	byte *screenBuf = (byte *)g_engine->_screen->getPixels();
	Common::copy(screenBuf, screenBuf + (22400 * 2), bg + 4);
}

void TotEngine::scrollRight(uint horizontalPos) {

	int characterPos = 25 + (320 - (_characterPosX + kCharacterCorrectionX * 2));
	// We scroll 4 by 4 pixels so we divide by 4 to find out the number of necessary steps
	uint stepCount = (320 - horizontalPos) >> 2;
	byte *assembledCharacterFrame = (byte *)malloc(_mainCharFrameSize);
	// Number of bytes to move
	size_t numBytes = 44796;
	for (uint i = 0; i < stepCount; i++) {
		// move everything to the left
		memmove(_sceneBackground + 4, _sceneBackground + 8, numBytes);

		horizontalPos += 4;
		for (int k = 0; k < 140; k++) {
			for (int j = 0; j < 4; j++) {
				_sceneBackground[320 + k * 320 + j] = _backgroundCopy[horizontalPos + k * 320 + j];
			}
		}
		if (characterPos > 0) {
			characterPos -= 2;
			if (characterPos > 0 && _iframe < 15) {
				_iframe++;
			} else
				_iframe = 0;

			_characterPosX -= 2;

			_curCharacterAnimationFrame = _mainCharAnimation.bitmap[1][_iframe];
			// We need to copy the original frame as to not replace its black background for after
			// the scroll ends. Original code would copy from XMS memory.
			Common::copy(_curCharacterAnimationFrame, _curCharacterAnimationFrame + _mainCharFrameSize, assembledCharacterFrame);

			// puts the original captured background back in the background for next iteration
			_graphics->putImageArea(_characterPosX - 2, _characterPosY, _sceneBackground, _spriteBackground);
			uint16 pasoframeW = READ_LE_UINT16(assembledCharacterFrame);
			uint16 pasoframeH = READ_LE_UINT16(assembledCharacterFrame + 2);
			// Grabs current area surrounding character (which might contain parts of A and B)
			_graphics->getImageArea(_characterPosX, _characterPosY, _characterPosX + pasoframeW, _characterPosY + pasoframeH, _sceneBackground, _spriteBackground);
			// blits over the character sprite, only on black pixels
			blit(_spriteBackground, assembledCharacterFrame);
			// puts it back in the background (character + piece of background)
			_graphics->putImageArea(_characterPosX, _characterPosY, _sceneBackground, assembledCharacterFrame);
		} else
			_characterPosX -= 4;
		_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		_screen->update();
		_graphics->drawScreen(_sceneBackground);
	}
	free(assembledCharacterFrame);
}

void TotEngine::scrollLeft(uint horizontalPos) {

	int characterPos = 25 + _characterPosX;
	horizontalPos = 320 - horizontalPos;
	// We scroll 4 by 4 pixels so we divide by 4 to find out the number of necessary steps
	uint stepCount = horizontalPos >> 2;

	byte *assembledCharacterFrame = (byte *)malloc(_mainCharFrameSize);
	size_t numBytes = 44796;
	for (int i = stepCount; i >= 1; i--) {
		for (int j = numBytes; j > 0; j--) {
			// move the previous background to the right
			_sceneBackground[j + 4] = _sceneBackground[j];
		}

		horizontalPos -= 4;
		for (int k = 0; k < 140; k++) {
			for (int j = 0; j < 4; j++) {
				_sceneBackground[4 + k * 320 + j] = _backgroundCopy[4 + horizontalPos + k * 320 + j];
			}
		}

		if (characterPos > 0) {
			characterPos -= 2;
			if (characterPos > 0 && _iframe < 15)
				_iframe++;
			else
				_iframe = 0;

			_characterPosX += 2;

			_curCharacterAnimationFrame = _mainCharAnimation.bitmap[3][_iframe];
			Common::copy(_curCharacterAnimationFrame, _curCharacterAnimationFrame + _mainCharFrameSize, assembledCharacterFrame);

			_graphics->putImageArea(_characterPosX + 2, _characterPosY, _sceneBackground, _spriteBackground);

			uint16 pasoframeW = READ_LE_UINT16(assembledCharacterFrame);
			uint16 pasoframeH = READ_LE_UINT16(assembledCharacterFrame + 2);

			_graphics->getImageArea(_characterPosX, _characterPosY, _characterPosX + pasoframeW, _characterPosY + pasoframeH, _sceneBackground, _spriteBackground);
			blit(_spriteBackground, assembledCharacterFrame);
			_graphics->putImageArea(_characterPosX, _characterPosY, _sceneBackground, assembledCharacterFrame);
		} else
			_characterPosX += 4;

		_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		_screen->update();
		_graphics->drawScreen(_sceneBackground);
	}
	free(assembledCharacterFrame);
}

/**
 * Scrolling happens between two screens. We grab the area surroudning the player from screen A,
 * then transition to screen B.
 */
void TotEngine::loadScrollData(uint roomCode, bool rightScroll, uint horizontalPos, int scrollCorrection) {

	_graphics->restoreBackground();

	// background now contains background A, backgroundCopy contains background A
	uint characterFrameW = READ_LE_UINT16(_curCharacterAnimationFrame);
	uint characterFrameH = READ_LE_UINT16(_curCharacterAnimationFrame + 2);
	/* Copy the area with the player from previous scren*/
	_spriteBackground = (byte *)malloc(4 + (characterFrameW + 8) * (characterFrameH + 8));
	_graphics->getImageArea(_characterPosX, _characterPosY, _characterPosX + characterFrameW, _characterPosY + characterFrameH, _sceneBackground, _spriteBackground);

	// Start screen 2

	_rooms->seek(roomCode * kRoomRegSize, SEEK_SET);
	if (_currentRoomData) {
		delete _currentRoomData;
	}
	_currentRoomData = readScreenDataFile(_rooms);

	loadScreen();
	// Background now contains background B, backgroundCopy contains background B
	for (int i = 0; i < 15; i++) {
		{
			RoomBitmapRegister &with = _currentRoomData->screenLayers[i];
			if (with.bitmapSize > 0)
				loadScreenLayer(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
		}
	}
	// assembles the screen objects into background
	assembleScreen(true);
	// background contains background B + objects, backgroundCopy contains plain background B

	// Copies the contents of background into backgroundCopy
	Common::copy(_sceneBackground, _sceneBackground + 44804, _backgroundCopy);
	// background contains background B + objects, backgroundCopy contains background B + objects

	g_engine->_graphics->_paletteAnimFrame = 0;
	getScreen(_sceneBackground);
	// background now contains full background A again, backgroundCopy contains background B + objects

	_graphics->drawScreen(_sceneBackground);
	if (rightScroll)
		scrollRight(horizontalPos);
	else
		scrollLeft(horizontalPos);

	// After scroll is done, backgroundCopy will now contain the resulting background (background B + objects)
	Common::copy(_backgroundCopy, _backgroundCopy + 44804, _sceneBackground);

	_characterPosX += scrollCorrection;

	assembleScreen();
	_graphics->drawScreen(_sceneBackground);
	free(_spriteBackground);
	loadScreen();
	_trajectory[_currentTrajectoryIndex].x = _characterPosX;
	_trajectory[_currentTrajectoryIndex].y = _characterPosY;
}

void TotEngine::sayLine(
	uint textRef,
	byte textColor,
	byte shadowColor,
	uint &responseNumber,
	/** Whether the text being said is part of a conversation or just descriptions */
	bool isWithinConversation) {
	TextEntry text;

	uint charCounter;
	byte insertName, lineBreakCount, width;
	byte characterFacingDir;

	uint talkAnimIndex, bgSize, posx, posy;

	byte *backgroundtextht;
	byte lineBreaks[15];
	_mouse->hide();
	switch (_currentRoomData->code) {
	case 2: { // Leisure room
		posx = 10;
		posy = 2;
		width = 28;
	} break;
	case 3: { // dining room
		posx = 130;
		posy = 2;
		width = 30;
	} break;
	case 8:  // patch
	case 21: // p4
		posx = 10;
		posy = 100;
		width = 50;
		break;
	case 10: { // well
		posx = 10;
		posy = 2;
		width = 40;
	} break;
	case 11: { // pond
		posx = 172;
		posy = 2;
		width = 26;
	} break;
	case 16: { // dorm. 1
		posx = 140;
		posy = 2;
		width = 30;
	} break;
	case 17: { // dorm. 2
		posx = 10;
		posy = 2;
		width = 30;
	} break;
	case 23: { // fountain
		posx = 10;
		posy = 2;
		width = 19;
	} break;
	case 25: { // catacombs
		posx = 10;
		posy = 2;
		width = 22;
	} break;
	case 28: { // storage room
		posx = 180;
		posy = 60;
		width = 24;
	} break;
	case 31: { // prison
		posx = 10;
		posy = 2;
		width = 25;
	} break;
	default: { // any other room
		posx = 10;
		posy = 2;
		width = 50;
	}
	}

	_verbFile.seek(textRef * kVerbRegSize);

	do {

		text = readTextRegister();

		insertName = 0;

		for (uint i = 0; i < text.text.size(); i++) {
			text.text.setChar(_decryptionKey[i] ^ text.text[i], i);
			if (text.text[i] == '@')
				insertName = i;
		}

		if (insertName > 0) {
			text.text.deleteChar(insertName);
			text.text.insertString(_characterName, insertName);
		}

		if (text.text.size() < width) {
			bgSize = imagesize(posx - 1, posy - 1, posx + (text.text.size() * 8) + 2, posy + 13);
			backgroundtextht = (byte *)malloc(bgSize);

			_graphics->getImg(posx - 1, posy - 1, posx + (text.text.size() * 8) + 2, posy + 13, backgroundtextht);

			littText(posx - 1, posy, text.text, shadowColor);
			_screen->update();
			delay(kEnforcedTextAnimDelay);
			littText(posx + 1, posy, text.text, shadowColor);
			_screen->update();
			delay(kEnforcedTextAnimDelay);
			littText(posx, posy - 1, text.text, shadowColor);
			_screen->update();
			delay(kEnforcedTextAnimDelay);
			littText(posx, posy + 1, text.text, shadowColor);
			_screen->update();
			delay(kEnforcedTextAnimDelay);

			littText(posx, posy, text.text, textColor);
			_screen->update();
			delay(kEnforcedTextAnimDelay);
		} else {

			charCounter = 0;
			lineBreakCount = 0;
			lineBreaks[0] = 0;

			// Breaks text lines on the last space when reaching the [anchoht]
			do {
				charCounter += width;
				lineBreakCount += 1;
				do {
					charCounter -= 1;
				} while (text.text[charCounter] != ' ');
				lineBreaks[lineBreakCount] = charCounter + 1;
			} while (((int)charCounter + 1) <= ((int)text.text.size() - (int)width));

			lineBreakCount += 1;
			lineBreaks[lineBreakCount] = text.text.size();

			// Grab patch of background behind where the text will be, to paste it back later
			bgSize = imagesize(posx - 1, posy - 1, posx + (width * 8) + 2, posy + lineBreakCount * 13);
			backgroundtextht = (byte *)malloc(bgSize);
			_graphics->getImg(posx - 1, posy - 1, posx + (width * 8) + 2, posy + lineBreakCount * 13, backgroundtextht);

			for (int i = 1; i <= lineBreakCount; i++) {

				Common::String lineString = Common::String(text.text.c_str() + lineBreaks[i - 1], text.text.c_str() + lineBreaks[i]);

				littText(posx + 1, posy + ((i - 1) * 11), lineString, shadowColor);
				_screen->update();
				delay(kEnforcedTextAnimDelay);
				littText(posx - 1, posy + ((i - 1) * 11), lineString, shadowColor);
				_screen->update();
				delay(kEnforcedTextAnimDelay);
				littText(posx, posy + ((i - 1) * 11) + 1, lineString, shadowColor);
				_screen->update();
				delay(kEnforcedTextAnimDelay);
				littText(posx, posy + ((i - 1) * 11) - 1, lineString, shadowColor);
				_screen->update();
				delay(kEnforcedTextAnimDelay);
				littText(posx, posy + ((i - 1) * 11), lineString, textColor);
				_screen->update();
				delay(kEnforcedTextAnimDelay);
			}
		}

		talkAnimIndex = 0;
		bool mouseClicked = false;

		// Plays talk cycle if needed
		do {
			_chrono->updateChrono();
			g_engine->_events->pollEvent();
			if (g_engine->_events->_leftMouseButton || g_engine->_events->_rightMouseButton) {
				mouseClicked = true;
			}
			if (_chrono->_gameTick) {
				_chrono->_gameTick = false;
				if (_chrono->_gameTickHalfSpeed) {
					if (isWithinConversation) {
						talkAnimIndex += 1;
						if (textColor == 255) {
							_iframe2 = 0;
							if (_iframe >= 15)
								_iframe = 0;
							else
								_iframe++;
						} else {

							_iframe = 0;
							if (_iframe2 >= _secondaryAnimationFrameCount - 1)
								_iframe2 = 0;
							else
								_iframe2++;
						}
						// Talk sprites are always put in facing direction 0
						characterFacingDir = _charFacingDirection;
						_charFacingDirection = 0;
						sprites(true);
						_charFacingDirection = characterFacingDir;
					} else {
						if (_currentSecondaryTrajectoryIndex >= _currentRoomData->secondaryTrajectoryLength)
							_currentSecondaryTrajectoryIndex = 1;
						else
							_currentSecondaryTrajectoryIndex += 1;
						_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
						_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
						_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
						if (_iframe2 >= _secondaryAnimationFrameCount - 1)
							_iframe2 = 0;
						else
							_iframe2++;
						sprites(false);
					}
				}
				_graphics->advancePaletteAnim();
			}
			_screen->update();
			g_system->delayMillis(10);
		} while (talkAnimIndex <= (text.text.size() * 4) && !mouseClicked && !shouldQuit());

		_graphics->putImg(posx - 1, posy - 1, backgroundtextht);
		free(backgroundtextht);

		g_system->delayMillis(10);
	} while (text.continued && !shouldQuit());
	responseNumber = text.response;
	_mouse->show();
}


void TotEngine::initScreenPointers() {
	_screenSize = 65520;
	_sceneBackground = (byte *)malloc(_screenSize);
	_backgroundCopy = (byte *)malloc(_screenSize);
}

void TotEngine::loadAnimationForDirection(Common::SeekableReadStream *stream, int direction) {
	uint size = _secondaryAnimFrameSize;
	for (int j = 0; j < _secondaryAnimationFrameCount; j++) {
		if (_secondaryAnimation.bitmap[direction][j]) {
			free(_secondaryAnimation.bitmap[direction][j]);
		}
		_secondaryAnimation.bitmap[direction][j] = (byte *)malloc(size);
		stream->read(_secondaryAnimation.bitmap[direction][j], size);
		Common::copy(_secondaryAnimation.bitmap[direction][j], _secondaryAnimation.bitmap[direction][j] + size, g_engine->_curSecondaryAnimationFrame);
	}
}

void TotEngine::loadAnimation(const Common::String &animationName) {
	Common::File animFile;

	if (animationName == "PETER")
		_isPeterCoughing = true;
	else
		_isPeterCoughing = false;

	_isSecondaryAnimationEnabled = true;
	if (!animFile.open(Common::Path(animationName + ".DAT"))) {
		showError(265);
	}

	_secondaryAnimFrameSize = animFile.readUint16LE();
	_secondaryAnimationFrameCount = animFile.readByte();
	_secondaryAnimDirCount = animFile.readByte();
	newSecondaryAnimationFrame();
	if (_secondaryAnimDirCount != 0) {

		_secondaryAnimationFrameCount = _secondaryAnimationFrameCount / 4;
		for (int i = 0; i < 4; i++) {
			loadAnimationForDirection(&animFile, i);
		}
	} else {
		loadAnimationForDirection(&animFile, 0);
	}

	animFile.close();
	_secondaryAnimWidth = READ_LE_UINT16(_secondaryAnimation.bitmap[0][1]) + 1;
	_secondaryAnimHeight = READ_LE_UINT16(_secondaryAnimation.bitmap[0][1] + 2) + 1;

	setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, SET_WITH_ANIM, false);

	readObject(_currentRoomData->secondaryAnimDirections[299]);
	_maxXGrid = (_curObject->xgrid2 - _curObject->xgrid1 + 1);
	_maxYGrid = (_curObject->ygrid2 - _curObject->ygrid1 + 1);
	_oldposx = _curObject->xgrid1 + 1;
	_oldposy = _curObject->ygrid1 + 1;

	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++) {
			_maskGridSecondaryAnim[i][j] = 0;
			_maskMouseSecondaryAnim[i][j] = 0;
			_movementGridForSecondaryAnim[i][j] = 0;
			_mouseGridForSecondaryAnim[i][j] = 0;
		}

	for (int i = 0; i < _maxXGrid; i++)
		for (int j = 0; j < _maxYGrid; j++) {
			_maskGridSecondaryAnim[i][j] = _curObject->walkAreasPatch[i][j];
			_maskMouseSecondaryAnim[i][j] = _curObject->mouseGridPatch[i][j];
			_movementGridForSecondaryAnim[i][j] = _currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j];
			_mouseGridForSecondaryAnim[i][j] = _currentRoomData->mouseGrid[_oldposx + i][_oldposy + j];
		}
	_iframe2 = 0;
}

void TotEngine::updateAltScreen(byte otherScreenNumber) {
	uint i22;
	uint i11;

	if (_currentRoomData) {
		byte currentScreen = _currentRoomData->code;

		setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, RESTORE);

		// Save current room
		saveRoomData(_currentRoomData, _rooms);

		// Load other screen
		_rooms->seek(otherScreenNumber * kRoomRegSize, SEEK_SET);
		delete _currentRoomData;
		_currentRoomData = readScreenDataFile(_rooms);

		switch (otherScreenNumber) {
		case 20: {
			switch (_niche[0][_niche[0][3]]) {
			case 0: {
				_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
				_currentRoomData->screenLayers[1].bitmapPointer = 1190768;
			} break;
			case 561: {
				_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
				_currentRoomData->screenLayers[1].bitmapPointer = 1182652;
			} break;
			case 563: {
				_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
				_currentRoomData->screenLayers[1].bitmapPointer = 1186044;
			} break;
			case 615: {
				_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
				_currentRoomData->screenLayers[1].bitmapPointer = 1181760;
			} break;
			}
			_currentRoomData->screenLayers[1].bitmapSize = 892;
			_currentRoomData->screenLayers[1].coordx = 66;
			_currentRoomData->screenLayers[1].coordy = 35;
			_currentRoomData->screenLayers[1].depth = 1;
		} break;
		case 24: {
			switch (_niche[1][_niche[1][3]]) {
			case 0: {
				_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
				_currentRoomData->screenLayers[0].bitmapPointer = 1399610;
			} break;
			case 561: {
				_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
				_currentRoomData->screenLayers[0].bitmapPointer = 1381982;
			} break;
			case 615: {
				_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
				_currentRoomData->screenLayers[0].bitmapPointer = 1381090;
			} break;
			case 622: {
				_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
				_currentRoomData->screenLayers[0].bitmapPointer = 1400502;
			} break;
			case 623: {
				_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
				_currentRoomData->screenLayers[0].bitmapPointer = 1398718;
			} break;
			}
			_currentRoomData->screenLayers[0].bitmapSize = 892;
			_currentRoomData->screenLayers[0].coordx = 217;
			_currentRoomData->screenLayers[0].coordy = 48;
			_currentRoomData->screenLayers[0].depth = 1;
		} break;
		case 31: {
			for (i11 = 23; i11 <= 25; i11++)
				for (i22 = 4; i22 <= 9; i22++)
					_currentRoomData->mouseGrid[i11][i22] = 4;
			for (i11 = 23; i11 <= 25; i11++)
				for (i22 = 10; i22 <= 11; i22++)
					_currentRoomData->mouseGrid[i11][i22] = 3;

			_currentRoomData->screenLayers[0].bitmapSize = 0;
			_currentRoomData->screenLayers[0].bitmapPointer = 0;
			_currentRoomData->screenLayers[0].coordx = 0;
			_currentRoomData->screenLayers[0].coordy = 0;
			_currentRoomData->screenLayers[0].depth = 0;
		} break;
		}

		// Save other screen
		saveRoomData(_currentRoomData, _rooms);

		// Restore current room again
		_rooms->seek(currentScreen * kRoomRegSize, SEEK_SET);
		if (_currentRoomData) {
			delete _currentRoomData;
		}
		_currentRoomData = readScreenDataFile(_rooms);

		setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, SET_WITH_ANIM);
	}
}

void TotEngine::loadTV() {

	Common::File fichct;
	if (!fichct.open("PALETAS.DAT")) {
		showError(310);
	}
	fichct.seek(_currentRoomData->palettePointer + 603);
	fichct.read(g_engine->_graphics->_palAnimSlice, 144);
	for (int i = 0; i <= 48; i++) {
		g_engine->_graphics->_palAnimSlice[i * 3 + 0] <<= 2;
		g_engine->_graphics->_palAnimSlice[i * 3 + 1] <<= 2;
		g_engine->_graphics->_palAnimSlice[i * 3 + 2] <<= 2;
	}

	fichct.close();
	for (int ix = 195; ix <= 200; ix++) {
		g_engine->_graphics->_pal[ix * 3 + 0] = 2 << 2;
		g_engine->_graphics->_pal[ix * 3 + 1] = 2 << 2;
		g_engine->_graphics->_pal[ix * 3 + 2] = 2 << 2;
		setRGBPalette(ix, 2, 2, 2);
	}
}

void TotEngine::loadScreen() {
	Common::File paletteFile;
	Palette palcp;

	_screenSize = _currentRoomData->roomImageSize;
	readBitmap(_currentRoomData->roomImagePointer, _sceneBackground, _screenSize, 316);
	Common::copy(_sceneBackground, _sceneBackground + _screenSize, _backgroundCopy);
	switch (_gamePart) {
	case 1: {
		if (!paletteFile.open("PALETAS.DAT")) {
			showError(310);
		}
		paletteFile.seek(_currentRoomData->palettePointer);
		paletteFile.read(palcp, 603);
		if (_currentRoomData->paletteAnimationFlag) {
			paletteFile.read(g_engine->_graphics->_palAnimSlice, 144);
			for (int i = 0; i <= 48; i++) {
				g_engine->_graphics->_palAnimSlice[i * 3 + 0] <<= 2;
				g_engine->_graphics->_palAnimSlice[i * 3 + 1] <<= 2;
				g_engine->_graphics->_palAnimSlice[i * 3 + 2] <<= 2;
			}
		}
		paletteFile.close();
		for (int i = 1; i <= 200; i++) {
			g_engine->_graphics->_pal[i * 3 + 0] = palcp[i * 3 + 0] << 2;
			g_engine->_graphics->_pal[i * 3 + 1] = palcp[i * 3 + 1] << 2;
			g_engine->_graphics->_pal[i * 3 + 2] = palcp[i * 3 + 2] << 2;
		}
		_graphics->setPalette(g_engine->_graphics->_pal, 0, 201);
	} break;
	case 2: {
		_graphics->loadPaletteFromFile("SEGUNDA");
		_currentRoomData->paletteAnimationFlag = true;
	} break;
	}
}

void TotEngine::clearScreenLayers() {
	for (int i = 0; i < kNumScreenOverlays; i++) {
		clearScreenLayer(i);
	}
}

void TotEngine::clearAnimation() {
	if (_isSecondaryAnimationEnabled) {
		_isSecondaryAnimationEnabled = false;
		if (_curSecondaryAnimationFrame != nullptr) {
			free(_curSecondaryAnimationFrame);
		}
		_curSecondaryAnimationFrame = nullptr;
		for (int j = 0; j < _secondaryAnimDirCount; j++) {
			for (int i = 0; i < _secondaryAnimationFrameCount; i++) {
				if (_secondaryAnimation.bitmap[j][i] != nullptr) {
					free(_secondaryAnimation.bitmap[j][i]);
				}
				_secondaryAnimation.bitmap[j][i] = nullptr;
			}
		}
	}
}

void TotEngine::freeInventory() {
	for (int i = 0; i < kInventoryIconCount; i++) {
		free(_inventoryIconBitmaps[i]);
	}
}

void TotEngine::loadScreenLayerWithDepth(uint coordx, uint coordy, uint bitmapSize, int32 bitmapIndex, uint depth) {
	if (g_engine->_screenLayers[depth]) {
		free(g_engine->_screenLayers[depth]);
	}
	_screenLayers[depth] = (byte *)malloc(bitmapSize);
	readBitmap(bitmapIndex, _screenLayers[depth], bitmapSize, 319);

	uint16 w, h;
	w = READ_LE_UINT16(_screenLayers[depth]);
	h = READ_LE_UINT16(_screenLayers[depth] + 2);
	_depthMap[depth].posx = coordx;
	_depthMap[depth].posy = coordy;
	_depthMap[depth].posx2 = coordx + w + 1;
	_depthMap[depth].posy2 = coordy + h + 1;
}

void TotEngine::loadScreenLayer(uint coordx, uint coordy, uint bitmapSize, int32 bitmapIndex, uint depth) {
	loadScreenLayerWithDepth(coordx, coordy, bitmapSize, bitmapIndex, depth - 1);
}

void TotEngine::updateInventory(byte index) {
	for (int i = index; i < (kInventoryIconCount - 1); i++) {
		_inventory[i].bitmapIndex = _inventory[i + 1].bitmapIndex;
		_inventory[i].code = _inventory[i + 1].code;
		_inventory[i].objectName = _inventory[i + 1].objectName;
	}
}

void TotEngine::drawLookAtItem(RoomObjectListEntry obj) {
	_mouse->hide();
	bar(0, 140, 319, 149, 0);
	actionLineText(getActionLineText(3) + obj.objectName);
	_mouse->show();
}

void TotEngine::putIcon(uint iconPosX, uint iconPosY, uint iconNum) {
	// substract 1 to account for 1-based indices
	_graphics->putImg(iconPosX, iconPosY, _inventoryIconBitmaps[_inventory[iconNum].bitmapIndex - 1]);
}

void TotEngine::drawInventory() {
	putIcon(34, 169, _inventoryPosition);
	putIcon(77, 169, _inventoryPosition + 1);
	putIcon(120, 169, _inventoryPosition + 2);
	putIcon(163, 169, _inventoryPosition + 3);
	putIcon(206, 169, _inventoryPosition + 4);
	putIcon(249, 169, _inventoryPosition + 5);
}

void lightUpLeft() {
	line(10, 173, 29, 173, 255);
	line(10, 173, 10, 189, 255);
	line(30, 174, 30, 190, 249);
	line(30, 190, 11, 190, 249);
}

void turnOffLeft() {
	line(10, 173, 29, 173, 249);
	line(10, 173, 10, 189, 249);
	line(30, 174, 30, 190, 255);
	line(30, 190, 11, 190, 255);
}

void lightUpRight() {
	line(291, 173, 310, 173, 255);
	line(291, 173, 291, 189, 255);
	line(311, 174, 311, 190, 249);
	line(311, 190, 292, 190, 249);
}

void turnOffRight() {
	line(291, 173, 310, 173, 249);
	line(291, 173, 291, 189, 249);
	line(311, 174, 311, 190, 255);
	line(311, 190, 292, 190, 255);
}

void TotEngine::drawInventory(byte dir, byte max) {
	switch (dir) {
	case 0:
		if (_inventoryPosition > 0) {
			_inventoryPosition -= 1;
			drawInventory();
		}
		break;
	case 1:
		if (_inventoryPosition < (max - 6)) {
			_inventoryPosition += 1;
			drawInventory();
		}
		break;
	}
	_mouse->hide();
	if (_inventoryPosition > 0)
		lightUpLeft();
	else
		turnOffLeft();

	if (_inventory[_inventoryPosition + 6].code > 0)
		lightUpRight();
	else
		turnOffRight();
	_mouse->show();
	if (_cpCounter > 145)
		showError(274);
}

void TotEngine::drawInventoryMask() {

	buttonBorder(0, 140, 319, 149, 0, 0, 0, 0, 0);
	for (int i = 1; i <= 25; i++)
		buttonBorder(0, (175 - i), 319, (174 + i), 251, 251, 251, 251, 0);
	drawMenu(1);
	if (_inventoryPosition > 1)
		lightUpLeft();
	else
		turnOffLeft();
	if (_inventory[_inventoryPosition + 6].code > 0)
		lightUpRight();
	else
		turnOffRight();
}

byte *getArrow(uint x, uint y) {
	Common::File menuFile;
	if (!menuFile.open("MENUS.DAT")) {
		showError(258);
	}

	int32 menuOffset = isLanguageSpanish() ? menuOffsets_ES[0][0] : menuOffsets_EN[0][0];
	uint w = 20;
	uint h = 18;
	uint size = 4 + w * h;
	byte *bitmap = (byte *)malloc(size);
	WRITE_LE_UINT16(bitmap, w - 1);
	WRITE_LE_UINT16(bitmap + 2, h - 1);
	menuFile.seek(menuOffset);


	uint originalW = menuFile.readUint16LE() + 1;
	for(uint i = 0; i < h; i++) {
		menuFile.seek(menuOffset + 4 + + (y + i) * originalW + x);
		menuFile.read(bitmap + 4 + i * w, w);
	}
	menuFile.close();
	return bitmap;
}

void TotEngine::drawLeftArrow(uint x, uint y) {
	byte *bitmap = getArrow(10, 23);
	_graphics->putImg(x, y, bitmap);
	free(bitmap);
}

void TotEngine::drawRightArrow(uint x, uint y) {
	byte *bitmap = getArrow(291, 23);
	_graphics->putImg(x, y, bitmap);
	free(bitmap);
}


void TotEngine::drawMenu(byte menuNumber) {
	byte *bitmap;
	uint menuSize;
	byte xmenu = 0, ymenu = 0;
	int32 menuOffset;

	Common::File menuFile;
	if (!menuFile.open("MENUS.DAT")) {
		showError(258);
	}

	menuOffset = isLanguageSpanish() ? menuOffsets_ES[menuNumber - 1][0] : menuOffsets_EN[menuNumber - 1][0];
	menuSize = isLanguageSpanish() ? menuOffsets_ES[menuNumber - 1][1] : menuOffsets_EN[menuNumber - 1][1];

	switch (menuNumber) {
	case 1: {
		xmenu = 0;
		ymenu = 150;
	} break;
	case 2:
	case 3:
	case 6:
		xmenu = 50;
		ymenu = 10;
		break;
	case 4: {
		if (_cpCounter2 > 20)
			showError(274);
		xmenu = 50;
		ymenu = 10;
	} break;
	case 5: {
		if (_cpCounter > 23)
			showError(274);
		xmenu = 0;
		ymenu = 150;
	} break;
	case 7: {
		xmenu = 58;
		ymenu = 48;
	} break;
	case 8: {
		xmenu = 84;
		ymenu = 34;
	} break;
	}

	bitmap = (byte *)malloc(menuSize);
	menuFile.seek(menuOffset);
	menuFile.read(bitmap, menuSize);
	_graphics->putImg(xmenu, ymenu, bitmap);
	free(bitmap);
	menuFile.close();
}

static void loadDiploma(Common::String &photoName, Common::String &key) {
	Palette auxPal;
	byte *screen;
	uint size;
	byte *stamp;

	Common::File dipFile;
	if (!dipFile.open("DIPLOMA.PAN")) {
		showError(318);
	}

	dipFile.read(auxPal, 768);

	screen = (byte *)malloc(64000);
	dipFile.read(screen, 64000);
	dipFile.close();

	if (!dipFile.open(Common::Path("DIPLOMA/SELLO.BMP")))
		showError(271);
	stamp = (byte *)malloc(2054);
	dipFile.read(stamp, 2054);
	dipFile.close();
	g_engine->_graphics->drawFullScreen(screen);

	free(screen);

	if (dipFile.open(Common::Path("DIPLOMA/" + photoName + ".FOT"))) {
		size = dipFile.size() - 768;
		screen = (byte *)malloc(size);
		dipFile.read(g_engine->_graphics->_pal, 768);
		dipFile.read(screen, size);
		dipFile.close();
		g_engine->_graphics->putShape(10, 20, screen);
		free(screen);
	}
	for (int i = 16; i <= 255; i++) {
		auxPal[i * 3 + 0] = g_engine->_graphics->_pal[i * 3 + 0];
		auxPal[i * 3 + 1] = g_engine->_graphics->_pal[i * 3 + 1];
		auxPal[i * 3 + 2] = g_engine->_graphics->_pal[i * 3 + 2];
	}

	g_engine->_graphics->copyPalette(auxPal, g_engine->_graphics->_pal);
	g_engine->_graphics->fixPalette(g_engine->_graphics->_pal, 768);
	g_engine->_graphics->setPalette(g_engine->_graphics->_pal);
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();

	char *passArray =  new char[10];
	for (int i = 0; i < 10; i++)
		passArray[i] = (char)(getRandom(10) + 48);

	key.append(passArray, passArray + 10);

	const char *const *messages = getFullScreenMessagesByCurrentLanguage();
	biosText(91, 16, messages[49] + key, 255);
	biosText(90, 15,  messages[49] + key, 13);

	biosText(81, 61,  messages[50], 0);
	biosText(61, 81,  messages[51], 0);
	if(isLanguageSpanish()) {
		biosText(31, 101, messages[52] + g_engine->_characterName, 0);
	} else {
		biosText(31, 101, messages[52], 0);
		biosText(151, 101, g_engine->_characterName, 0);
	}
	biosText(31, 121, messages[53], 0);
	biosText(31, 141, messages[54], 0);
	biosText(31, 161, messages[55], 0);

	biosText(80, 60,  messages[50], 15);
	biosText(60, 80,  messages[51], 15);
	biosText(30, 100, messages[52], 15);

	biosText(150, 100, g_engine->_characterName, 13);

	biosText(30, 120, messages[53], 15);
	biosText(30, 140, messages[54], 15);
	biosText(30, 160, messages[55], 15);

	delay(1500);
	g_engine->_sound->playVoc("PORTAZO", 434988, 932);
	g_engine->_graphics->putShape(270, 159, stamp);
	delete[] passArray;
	free(stamp);
}

static void saveDiploma(Common::String &photoName, Common::String &key) {
	Common::String name;
	if (photoName != "")
		name = "tot-diploma-" + photoName + ".png";
	else
		name = "tot-diploma-default.png";
	Common::OutSaveFile *thumbnail = g_engine->getSaveFileManager()->openForSaving(name);
	Graphics::Surface *surface = g_system->lockScreen();
	assert(surface);
	Image::writePNG(*thumbnail, *surface, g_engine->_graphics->getPalette());
	g_system->unlockScreen();
	thumbnail->finalize();
	delete thumbnail;
}

void TotEngine::generateDiploma(Common::String &photoName) {
	Common::String key;
	_mouse->hide();
	photoName.toUppercase();
	_graphics->totalFadeOut(0);
	loadDiploma(photoName, key);

	do {
		_screen->update();
		g_engine->_events->pollEvent();
		g_system->delayMillis(10);
	} while (!g_engine->_events->_keyPressed && !shouldQuit());
	saveDiploma(photoName, key);
	_mouse->show();
}

void TotEngine::checkMouseGrid() {
	uint xGrid, yGrid;
	Common::String invObject;
	if (_cpCounter2 > 120)
		showError(274);
	if (_mouse->mouseY <= 131) {
		xGrid = _mouse->getMouseCoordsWithinGrid().x;
		yGrid = _mouse->getMouseCoordsWithinGrid().y;
		if (_currentRoomData->mouseGrid[xGrid][yGrid] != _currentRoomData->mouseGrid[_oldGridX][_oldGridY] || _oldInventoryObjectName != "") {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (_actionCode) {
			case 0:
				actionLine = getActionLineText(0) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 1:
				actionLine = getActionLineText(1) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 2:
				actionLine = getActionLineText(2) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 3:
				actionLine = getActionLineText(3) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 4:
				if (_inventoryObjectName != "")
					actionLine = getActionLineText(4) + _inventoryObjectName + getActionLineText(7) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				else
					actionLine = getActionLineText(4) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 5:
				actionLine = getActionLineText(5) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			case 6:
				actionLine = getActionLineText(6) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
				break;
			default:
				actionLine = getActionLineText(0) + _currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[xGrid][yGrid]]->objectName;
			}
			actionLineText(actionLine);
			_mouse->show();
			_oldGridX = xGrid;
			_oldGridY = yGrid;
		}
		_oldActionCode = 253;
		_oldInventoryObjectName = "";
	} else if (_mouse->mouseY <= 165) {
		if (_actionCode != _oldActionCode) {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (_actionCode) {
			case 0:
				actionLine = getActionLineText(0);
				break;
			case 1:
				actionLine = getActionLineText(1);
				break;
			case 2:
				actionLine = getActionLineText(2);
				break;
			case 3:
				actionLine = getActionLineText(3);
				break;
			case 4:
				if (_inventoryObjectName != "")
					actionLine = getActionLineText(4) + _inventoryObjectName + getActionLineText(7);
				else
					actionLine = getActionLineText(4);
				break;
			case 5:
				actionLine = getActionLineText(5);
				break;
			case 6:
				actionLine = getActionLineText(6);
				break;
			}
			actionLineText(actionLine);
			_mouse->show();
			_oldActionCode = _actionCode;
			_oldInventoryObjectName = "";
			_oldGridX = 0;
			_oldGridY = 0;
		}
	} else if (_mouse->mouseY <= 199) {
		if (_mouse->mouseX >= 26 && _mouse->mouseX <= 65) {
			invObject = _inventory[_inventoryPosition].objectName;
		} else if (_mouse->mouseX >= 70 && _mouse->mouseX <= 108) {
			invObject = _inventory[_inventoryPosition + 1].objectName;
		} else if (_mouse->mouseX >= 113 && _mouse->mouseX <= 151) {
			invObject = _inventory[_inventoryPosition + 2].objectName;
		} else if (_mouse->mouseX >= 156 && _mouse->mouseX <= 194) {
			invObject = _inventory[_inventoryPosition + 3].objectName;
		} else if (_mouse->mouseX >= 199 && _mouse->mouseX <= 237) {
			invObject = _inventory[_inventoryPosition + 4].objectName;
		} else if (_mouse->mouseX >= 242 && _mouse->mouseX <= 280) {
			invObject = _inventory[_inventoryPosition + 5].objectName;
		} else {
			invObject = ' ';
		}

		if (invObject != _oldInventoryObjectName || _oldGridX != 0) {
			bar(0, 140, 319, 149, 0);
			Common::String actionLine;
			switch (_actionCode) {
			case 1:
				actionLine = getActionLineText(1) + invObject;
				break;
			case 2:
				actionLine = getActionLineText(2) + invObject;
				break;
			case 3:
				actionLine = getActionLineText(3) + invObject;
				break;
			case 4:
				if (_inventoryObjectName == "")
					actionLine = getActionLineText(4) + invObject;
				else
					actionLine = getActionLineText(4) + _inventoryObjectName + getActionLineText(7) + invObject;
				break;
			case 5:
				actionLine = getActionLineText(5) + invObject;
				break;
			case 6:
				actionLine = getActionLineText(6) + invObject;
				break;
			default:
				euroText(160, 144, invObject, 255, Graphics::kTextAlignCenter);
			}
			actionLineText(actionLine);
			_mouse->show();
			_oldInventoryObjectName = invObject;
		}
		_oldActionCode = 255;
		_oldGridX = 0;
		_oldGridY = 0;
	}
}

void TotEngine::readAlphaGraph(Common::String &output, int length, int posx, int posy, byte barColor) {
	int pun = 1;
	bar(posx, posy - 2, posx + length * 8, posy + 8, barColor);

	biosText(posx, posy, "_", 0);

	Common::Event e;
	bool done = false;

	g_system->getEventManager()->getKeymapper()->setEnabled(false);
	while (!done && !shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {

			if (e.type == Common::EVENT_KEYDOWN) {
				int keycode = e.kbd.keycode;
				int asciiCode = e.kbd.ascii;
				// ENTER key
				if (keycode == Common::KEYCODE_RETURN || keycode == Common::KEYCODE_KP_ENTER) {
					if (output.size() > 0) {
						done = true;
						continue;
					}
				}
				// Max 8 chars
				if (pun > length && asciiCode != 8) {
					_sound->beep(750, 60);
					bar((posx + (output.size()) * 8), (posy - 2), (posx + (output.size() + 1) * 8), (posy + 8), barColor);
				} else if (asciiCode == 8 && pun > 1) { // delete
					output = output.substr(0, output.size() - 1);
					bar(posx, (posy - 2), (posx + length * 8), (posy + 8), barColor);
					biosText(posx, posy, output.c_str(), 0);
					biosText((posx + (output.size()) * 8), posy, "_", 0);
					pun -= 1;
				} else if (
					(asciiCode < 97 || asciiCode > 122) &&
					(asciiCode < 65 || asciiCode > 90) &&
					(asciiCode < 32 || asciiCode > 57) &&
					(asciiCode < 164 || asciiCode > 165)) {
					_sound->beep(1200, 60);
				} else {
					pun += 1;
					output = output + (char)e.kbd.ascii;
					bar(posx, (posy - 2), (posx + length * 8), (posy + 8), barColor);
					biosText(posx, posy, output.c_str(), 0);
					biosText((posx + (output.size()) * 8), posy, "_", 0);
				}
			}
		}
		g_system->delayMillis(10);
		_screen->update();
	}

	g_system->getEventManager()->getKeymapper()->setEnabled(true);
}

bool getEvent(Common::Event &e, Common::Event &firstEvent) {
	if (firstEvent.type != Common::EVENT_INVALID) {
		e = firstEvent;
		firstEvent.type = Common::EVENT_INVALID;
		return true;
	}
	return g_system->getEventManager()->pollEvent(e);
}

void TotEngine::readAlphaGraphSmall(
	Common::String &output,
	int length,
	int posx,
	int posy,
	byte barColor,
	byte textColor,
	Common::Event event) {
	int pun = 1;
	bool removeCaret = false;
	g_system->getEventManager()->getKeymapper()->setEnabled(false);
	output = "";
	bar(posx, posy + 2, posx + length * 6, posy + 9, barColor);
	euroText(posx, posy, "-", textColor);
	Common::Event e;
	bool done = false;
	while (!done && !shouldQuit()) {
		while (getEvent(e, event)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				int keycode = e.kbd.keycode;
				int asciiCode = e.kbd.ascii;

				// ENTER key
				if (keycode == Common::KEYCODE_RETURN || keycode == Common::KEYCODE_KP_ENTER) {
					if (output.size() > 0) {
						done = true;
						removeCaret = true;
						continue;
					}
				}
				if (pun > length && asciiCode != 8) {
					_sound->beep(750, 60);
					bar((posx + (output.size()) * 6), (posy + 2), (posx + (output.size() + 1) * 6), (posy + 10), barColor);
				} else if (asciiCode == 8 && pun > 1) {
					output = output.substr(0, output.size() - 1);

					bar(posx, (posy + 2), (posx + length * 6), (posy + 10), barColor);
					euroText(posx, posy, output, textColor);
					euroText(posx + _graphics->euroTextWidth(output), posy, "-", textColor);
					pun -= 1;
					removeCaret = true;
				} else if (
					(asciiCode < 97 || asciiCode > 122) &&
					(asciiCode < 65 || asciiCode > 90) &&
					(asciiCode < 32 || asciiCode > 57) &&
					(asciiCode < 164 || asciiCode > 165)) {
					_sound->beep(1200, 60);
					removeCaret = false;
				} else {
					pun += 1;
					output = output + (char)e.kbd.ascii;

					bar(posx, (posy + 2), (posx + length * 6), (posy + 9), barColor);
					euroText(posx, posy, output, textColor);

					euroText(posx + _graphics->euroTextWidth(output), posy, "-", textColor);
					removeCaret = true;
				}
			}
		}

		g_system->delayMillis(10);
		_screen->update();
	}

	if (removeCaret) {
		bar(posx + _graphics->euroTextWidth(output), posy + 2, (posx + _graphics->euroTextWidth(output)) + 6, posy + 9, barColor);
	}
	g_system->getEventManager()->getKeymapper()->setEnabled(true);
}

void TotEngine::displayObjectDescription(const Common::String &textString) {
	uint xpos = 60;
	uint ypos = 15;
	byte maxWidth = 33;
	byte textColor = 255;
	byte shadowColor = 0;
	uint ihc;
	byte lineCounter;
	byte newLineMatrix[10];

	if (textString.size() < maxWidth) {
		euroText((xpos + 1), (ypos + 1), textString, shadowColor);
		_screen->update();
		delay(kEnforcedTextAnimDelay);
		euroText(xpos, ypos, textString, textColor);
		_screen->update();
		delay(kEnforcedTextAnimDelay);
	} else {
		ihc = 0;
		lineCounter = 0;
		newLineMatrix[0] = 0;
		do {
			ihc += maxWidth + 1;
			lineCounter += 1;
			do {
				ihc -= 1;
			} while (textString[ihc] != ' ');
			newLineMatrix[lineCounter] = ihc + 1;
		} while (ihc + 1 <= textString.size() - maxWidth);

		lineCounter += 1;
		newLineMatrix[lineCounter] = textString.size();

		for (byte line = 1; line <= lineCounter; line++) {
			Common::String lineString = textString.substr(newLineMatrix[line - 1], newLineMatrix[line] - newLineMatrix[line - 1]);
			euroText((xpos + 1), (ypos + ((line - 1) * 11) + 1), lineString, shadowColor);
			_screen->update();
			delay(kEnforcedTextAnimDelay);
			euroText(xpos, (ypos + ((line - 1) * 11)), lineString, textColor);
			_screen->update();
			delay(kEnforcedTextAnimDelay);
		}
	}
}

void TotEngine::buttonBorder(uint x1, uint y1, uint x2, uint y2,
				  byte color1, byte color2, byte color3, byte color4, byte color5) {

	bar(x1, y1, x2, y2, color4);
	line(x1, y1, x1, y2, color1);
	line(x1, y1, x2, y1, color1);

	line(x2, y1, x2, y2, color2);
	line(x2, y2, x1, y2, color2);

	putpixel(x2, y1, color3);
	putpixel(x1, y2, color3);

	_screen->addDirtyRect(Common::Rect(x1, y1, x2, y2));
	_screen->update();
}

static void buttonPress(uint xx1, uint yy1, uint xx2, uint yy2, bool pressed) {
	g_engine->_mouse->hide();

	uint color = 0;
	if (pressed)
		color = 249;
	else
		color = 255;
	line(xx1, yy1, (xx2 - 1), yy1, color);
	line(xx1, yy1, xx1, (yy2 - 1), color);
	if (pressed)
		color = 255;
	else
		color = 249;
	line((xx1 + 1), yy2, xx2, yy2, color);
	line(xx2, (yy1 + 1), xx2, yy2, color);
	g_engine->_screen->addDirtyRect(Common::Rect(xx1, yy1, xx2, yy2));
	g_engine->_screen->update();
	g_engine->_mouse->show();
	delay(100);
}

void TotEngine::copyProtection() {
	if (!ConfMan.getBool("copy_protection") || isDemo()) {
		return;
	}
	const char *message = isLanguageSpanish() ? hardcodedTexts_ES[12] : hardcodedTexts_EN[12];
	int beepDelay = 100;
	int chunkSize = 4;

	uint inputPassword = 0;
	byte cursorPos = 0, attempts = 0;
	int32 diskPass = 0, protXor1 = 0, protXor2 = 0;

	Common::File protectionFile;
	if (!protectionFile.open("MCGA.DRV")) {
		showError(260);
	}
	Common::String inputString = "      ";
	byte numRow = getRandom(95) + 1;
	byte numColumn = getRandom(38) + 1;

	protectionFile.seek(1 * chunkSize);
	protXor1 = protectionFile.readUint32LE();
	protectionFile.seek((((numRow - 1) * 38) + numColumn + 1) * chunkSize);
	protXor2 = protectionFile.readUint32LE();
	inputPassword = 0;
	diskPass = protectionFile.readUint32LE();
	uint correctPassword = ((diskPass ^ protXor1) - protXor2);
	bool exitDialog = false;
	protectionFile.close();

	uint oldMouseX = _mouse->mouseX;
	uint oldMouseY = _mouse->mouseY;
	uint oldMouseMaskIndex = _mouse->mouseMaskIndex;
	_mouse->hide();

	uint bgSize = imagesize(50, 10, 270, 120);
	byte *bgPointer = (byte *)malloc(bgSize);
	_graphics->getImg(50, 10, 270, 120, bgPointer);
	_mouse->mouseX = 150;
	_mouse->mouseY = 60;
	_mouse->mouseMaskIndex = 1;
	_mouse->setMouseArea(Common::Rect(55, 13, 250, 105));
	for (int i = 1; i <= 6; i++)
		buttonBorder((120 - (i * 10)), (80 - (i * 10)), (200 + (i * 10)), (60 + (i * 10)), 251, 251, 251, 251, 0);

	drawMenu(6);
	euroText(65, 15, Common::String::format(message, numRow, numColumn), 255);
	cursorPos = 0;
	attempts = 0;
	_mouse->warpMouse(1, _mouse->mouseX, _mouse->mouseY);
	_screen->markAllDirty();
	_screen->update();
	Common::Event e;
	do {
		bool mouseClicked = false;
		do {
			_chrono->updateChrono();
			if (_chrono->_gameTick) {
				_mouse->animateMouseIfNeeded();
				_chrono->_gameTick = false;
			}
			while (g_system->getEventManager()->pollEvent(e)) {
				if (isMouseEvent(e)) {
					_mouse->warpMouse(e.mouse);
					_mouse->mouseX = e.mouse.x;
					_mouse->mouseY = e.mouse.y;
				}
				if (e.type == Common::EVENT_LBUTTONUP) {
					_mouse->mouseClickX = e.mouse.x;
					_mouse->mouseClickY = e.mouse.y;
					mouseClicked = true;
				}
			}
			_screen->markAllDirty();
			_screen->update();
			g_system->delayMillis(10);
		} while (!mouseClicked && !shouldQuit());

		int correctedMouseX = _mouse->mouseClickX + 7;
		int correctedMouseY = _mouse->mouseClickY + 7;

		if ((correctedMouseX > 59 && correctedMouseX < 180) && (correctedMouseY > 28 && correctedMouseY < 119)) {
			if (correctedMouseY <= 58) {
				if (correctedMouseX <= 89) {
					if (cursorPos < 50) {
						_sound->beep(200, beepDelay);
						buttonPress(60, 29, 89, 58, true);
						euroText(205 + cursorPos, 44, "0", 255);
						inputString = inputString + '0';
						buttonPress(60, 29, 89, 58, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				} else if (correctedMouseX <= 119) {
					if (cursorPos < 50) {
						_sound->beep(250, beepDelay);
						buttonPress(90, 29, 119, 58, true);
						euroText((205 + cursorPos), 44, "1", 255);
						inputString = inputString + '1';
						buttonPress(90, 29, 119, 58, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				} else if (correctedMouseX <= 149) {
					if (cursorPos < 50) {
						_sound->beep(300, beepDelay);
						buttonPress(120, 29, 149, 58, true);
						euroText((205 + cursorPos), 44, "2", 255);
						inputString = inputString + '2';
						buttonPress(120, 29, 149, 58, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				} else {
					if (cursorPos < 50) {
						_sound->beep(350, beepDelay);
						buttonPress(150, 29, 179, 58, true);
						euroText((205 + cursorPos), 44, "3", 255);
						inputString = inputString + '3';
						buttonPress(150, 29, 179, 58, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				}
			} else if (correctedMouseY <= 88) {
				if (correctedMouseX <= 89) {
					if (cursorPos < 50) {
						_sound->beep(400, beepDelay);
						buttonPress(60, 59, 89, 88, true);
						euroText((205 + cursorPos), 44, "4", 255);
						inputString = inputString + '4';
						buttonPress(60, 59, 89, 88, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				} else if (correctedMouseX <= 119) {
					if (cursorPos < 50) {
						_sound->beep(450, beepDelay);
						buttonPress(90, 59, 119, 88, true);
						euroText((205 + cursorPos), 44, "5", 255);
						inputString = inputString + '5';
						buttonPress(90, 59, 119, 88, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				} else if (correctedMouseX <= 149) {
					if (cursorPos < 50) {
						_sound->beep(500, beepDelay);
						buttonPress(120, 59, 149, 88, true);
						euroText((205 + cursorPos), 44, "6", 255);
						inputString = inputString + '6';
						buttonPress(120, 59, 149, 88, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				} else {
					if (cursorPos < 50) {
						_sound->beep(550, beepDelay);
						buttonPress(150, 59, 179, 88, true);
						euroText((205 + cursorPos), 44, "7", 255);
						inputString = inputString + '7';
						buttonPress(150, 59, 179, 88, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				}
			} else {
				if (correctedMouseX <= 89) {
					if (cursorPos < 50) {
						_sound->beep(600, beepDelay);
						buttonPress(60, 89, 89, 118, true);
						euroText((205 + cursorPos), 44, "8", 255);
						inputString = inputString + '8';
						buttonPress(60, 89, 89, 118, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				} else if (correctedMouseX <= 119) {
					if (cursorPos < 50) {
						_sound->beep(650, beepDelay);
						buttonPress(90, 89, 119, 118, true);
						euroText((205 + cursorPos), 44, "9", 255);
						inputString = inputString + '9';
						buttonPress(90, 89, 119, 118, false);
						cursorPos += 10;
					} else {
						_sound->beep(70, 250);
					}
				} else if (correctedMouseX <= 149) {
					if (cursorPos > 0) {
						_sound->beep(700, beepDelay);
						buttonPress(120, 89, 149, 118, true);
						cursorPos -= 10;
						euroText((205 + cursorPos), 44, "\xDB", 250);
						inputString = inputString.substr(0, inputString.size() - 1);
						buttonPress(120, 89, 149, 118, false);
					} else {
						_sound->beep(70, 250);
					}
				} else {
					if (cursorPos > 39) {
						_sound->beep(750, beepDelay);
						buttonPress(150, 89, 179, 118, true);
						inputPassword = atoi(inputString.c_str());
						buttonPress(150, 89, 179, 118, false);
						if (attempts < 3) {
							if (inputPassword == correctPassword)
								exitDialog = true;
							else {
								attempts += 1;
								_sound->beep(60, 250);
							}
						}
						if (attempts >= 3)
							showError(259);
					} else {
						_sound->beep(70, 250);
					}
				}
			}
		}

	} while (!exitDialog && !shouldQuit());

	_graphics->putImg(50, 10, bgPointer);
	_mouse->mouseX = oldMouseX;
	_mouse->mouseY = oldMouseY;
	_mouse->mouseMaskIndex = oldMouseMaskIndex;
	_mouse->warpMouse(_mouse->mouseMaskIndex, _mouse->mouseX, _mouse->mouseY);
	if (_cpCounter > 8)
		showError(274);
	free(bgPointer);
	_mouse->setMouseArea(Common::Rect(0, 0, 305, 185));
}

void TotEngine::soundControls() {
	uint oldMouseX,
		oldMouseY,
		soundControlsSize,
		sliderSize,
		sliderBgSize,
		sfxVol,
		musicVol,
		xfade,
		oldxfade;

	byte ytext, oldMouseMask;
	bool exitSoundControls;

	exitSoundControls = false;
	oldMouseX = _mouse->mouseX;
	oldMouseY = _mouse->mouseY;
	oldMouseMask = _mouse->mouseMaskIndex;
	_mouse->hide();

	soundControlsSize = imagesize(50, 10, 270, 120);
	// What was on the screen before blitting sound controls
	byte *soundControlsBackground = (byte *)malloc(soundControlsSize);
	_graphics->getImg(50, 10, 270, 120, soundControlsBackground);

	_mouse->mouseX = 150;
	_mouse->mouseY = 60;
	_mouse->mouseMaskIndex = 1;

	_mouse->setMouseArea(Common::Rect(55, 13, 250, 105));

	for (ytext = 1; ytext <= 6; ytext++)
		buttonBorder(120 - (ytext * 10), 80 - (ytext * 10), 200 + (ytext * 10), 60 + (ytext * 10), 251, 251, 251, 251, 0);

	buttonBorder(86, 31, 94, 44, 0, 0, 0, 0, 0);

	line(90, 31, 90, 44, 255);

	sliderSize = imagesize(86, 31, 94, 44);
	byte *slider = (byte *)malloc(sliderSize);
	_graphics->getImg(86, 31, 94, 44, slider);

	drawMenu(3);
	sliderBgSize = imagesize(86, 31, 234, 44);

	byte *sliderBackground1 = (byte *)malloc(sliderBgSize);
	byte *sliderBackground2 = (byte *)malloc(sliderBgSize);
	_graphics->getImg(86, 31, 234, 44, sliderBackground1);
	_graphics->getImg(86, 76, 234, 89, sliderBackground2);

	sfxVol = MIN(256, ConfMan.getInt("sfx_volume")) * 140 / 256;
	musicVol = MIN(256, ConfMan.getInt("music_volume")) * 140 / 256;
	_graphics->putImg(sfxVol + 86, 31, slider);
	_graphics->putImg(musicVol + 86, 76, slider);

	_mouse->warpMouse(1, _mouse->mouseX, _mouse->mouseY);
	bool keyPressed = false;
	bool mouseClicked = false;
	do {
		_chrono->updateChrono();
		do {
			_chrono->updateChrono();
			_mouse->animateMouseIfNeeded();
			g_engine->_events->pollEvent(true);

			if (g_engine->_events->_keyPressed) {
				keyPressed = true;
			}
			if (g_engine->_events->_leftMouseButton) {
				debug("Mouse clicked!");
				mouseClicked = true;
				_mouse->mouseClickX = g_engine->_events->_mouseX;
				_mouse->mouseClickY = g_engine->_events->_mouseY;
			}

			_screen->update();
		} while ((!keyPressed && !mouseClicked) && !shouldQuit());

		_chrono->updateChrono();
		_mouse->animateMouseIfNeeded();
		if (keyPressed) {
			exitSoundControls = true;
		}
		if (mouseClicked) {

			if (_mouse->mouseClickY >= 22 && _mouse->mouseClickY <= 37) {
				_mouse->hide();
				xfade = 86 + sfxVol;
				bool mouseReleased = false;
				do {
					oldxfade = xfade;
					g_engine->_events->pollEvent(true);
					if (g_engine->_events->_leftMouseButton == 0) {
						mouseReleased = true;
					} else {
						xfade = g_engine->_events->_mouseX;
					}

					if (xfade < 86) {
						xfade = 86;
					} else if (xfade > 226) {
						xfade = 226;
					}

					if (oldxfade != xfade) {
						_graphics->putImg(86, 31, sliderBackground1);
						_graphics->putImg(xfade, 31, slider);
						// This yields a volume between 0 and 140
						sfxVol = xfade - 86;

						ConfMan.setInt("sfx_volume", sfxVol * 256 / 140);
						g_engine->syncSoundSettings();
					}
					_screen->update();
				} while (!mouseReleased);

				_mouse->show();
			} else if (_mouse->mouseClickY >= 67 && _mouse->mouseClickY <= 82) {
				_mouse->hide();
				xfade = 86 + musicVol;
				bool mouseReleased = false;
				do {

					oldxfade = xfade;
					g_engine->_events->pollEvent(true);

					if (g_engine->_events->_leftMouseButton == 0) {
						mouseReleased = true;
					} else {
						xfade = g_engine->_events->_mouseX;
					}
					if (xfade < 86) {
						xfade = 86;
					} else if (xfade > 226) {
						xfade = 226;
					}

					if (oldxfade != xfade) {
						_graphics->putImg(86, 76, sliderBackground2);
						_graphics->putImg(xfade, 76, slider);
						musicVol = xfade - 86;
						ConfMan.setInt("music_volume", musicVol * 256 / 140);
						g_engine->syncSoundSettings();
					}
					_screen->update();
				} while (!mouseReleased);

				_mouse->show();
			} else if (_mouse->mouseClickY >= 97 && _mouse->mouseClickY <= 107) {
				exitSoundControls = true;
			}
			mouseClicked = false;
		}
		g_system->delayMillis(10);
		_screen->update();
	} while (!exitSoundControls && !shouldQuit());

	_graphics->putImg(50, 10, soundControlsBackground);
	_mouse->mouseX = oldMouseX;
	_mouse->mouseY = oldMouseY;
	_mouse->mouseMaskIndex = oldMouseMask;
	_mouse->warpMouse(_mouse->mouseMaskIndex, _mouse->mouseX, _mouse->mouseY);
	free(soundControlsBackground);
	free(slider);
	free(sliderBackground1);
	free(sliderBackground2);

	if (_cpCounter > 7)
		showError(274);
	_mouse->setMouseArea(Common::Rect(0, 0, 305, 185));
}

void TotEngine::loadBat() {
	Common::File animFile;

	_isSecondaryAnimationEnabled = true;
	if (!animFile.open("MURCIE.DAT")) {
		showError(265);
	}
	_secondaryAnimFrameSize = animFile.readUint16LE();
	_secondaryAnimationFrameCount = animFile.readByte();
	_secondaryAnimDirCount = animFile.readByte();
	newSecondaryAnimationFrame();
	loadAnimationForDirection(&animFile, 0);
	animFile.close();
}

void TotEngine::loadDevil() {
	Common::File animFile;

	_isSecondaryAnimationEnabled = true;
	if (!animFile.open("ROJOMOV.DAT")) {
		showError(265);
	}
	_secondaryAnimFrameSize = animFile.readUint16LE();
	_secondaryAnimationFrameCount = animFile.readByte();
	_secondaryAnimDirCount = animFile.readByte();
	newSecondaryAnimationFrame();
	if (_secondaryAnimDirCount != 0) {
		_secondaryAnimationFrameCount = _secondaryAnimationFrameCount / 4;
		for (int i = 0; i <= 3; i++) {
			loadAnimationForDirection(&animFile, i);
		}
	}
	animFile.close();
}

void TotEngine::assembleCompleteBackground(byte *image, uint coordx, uint coordy) {
	uint16 w, h;
	uint16 wFondo;
	w = READ_LE_UINT16(image);
	h = READ_LE_UINT16(image + 2);

	wFondo = READ_LE_UINT16(_sceneBackground);

	wFondo++;
	w++;
	h++;
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int color = image[4 + j * w + i];
			if (color != 0) {
				_sceneBackground[4 + (coordy + j) * wFondo + (coordx + i)] = color;
			}
		}
	}
}

/**
 * Assemble the screen for scroll assembles only the objects because scrolling screens
 * never have secondary animations and character animation is assembled elsewhere.
 */
void TotEngine::assembleScreen(bool scroll) {

	for (int indice = 0; indice < kDepthLevelCount; indice++) {
		if (_screenLayers[indice] != nullptr) {
			assembleCompleteBackground(_screenLayers[indice], _depthMap[indice].posx, _depthMap[indice].posy);
		}
		if (!scroll && _mainCharAnimation.depth == indice) {
			assembleCompleteBackground(_mainCharAnimation.bitmap[_charFacingDirection][_iframe], _characterPosX, _characterPosY);
		}
		if (!scroll && _currentRoomData->animationFlag && _secondaryAnimation.depth == indice) {
			assembleCompleteBackground(_curSecondaryAnimationFrame, _secondaryAnimation.posx, _secondaryAnimation.posy);
		}
	}
}

void TotEngine::disableSecondAnimation() {
	setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, RESTORE);
	_currentRoomData->animationFlag = false;
	clearAnimation();
	_graphics->restoreBackground();
	assembleScreen();
}

void TotEngine::setRoomTrajectories(int animationHeight, int animationWidth, TRAJECTORIES_OP op, bool fixGrids) {
	// add to restore the room, subtract to adjust before loading the screen

	if (_currentRoomData->animationFlag && _currentRoomData->animationName != "QQQQQQQQ") {
		for (int i = 0; i < _currentRoomData->secondaryTrajectoryLength; i++) {
			if (op == RESTORE) {
				_currentRoomData->secondaryAnimTrajectory[i].x = _currentRoomData->secondaryAnimTrajectory[i].x + (animationWidth >> 1);
				_currentRoomData->secondaryAnimTrajectory[i].y = _currentRoomData->secondaryAnimTrajectory[i].y + animationHeight;
			} else {
				_currentRoomData->secondaryAnimTrajectory[i].x = _currentRoomData->secondaryAnimTrajectory[i].x - (animationWidth >> 1);
				_currentRoomData->secondaryAnimTrajectory[i].y = _currentRoomData->secondaryAnimTrajectory[i].y - animationHeight;
			}
		}
		if (fixGrids) {
			for (int i = 0; i < _maxXGrid; i++) {
				for (int j = 0; j < _maxYGrid; j++) {
					if (op == RESTORE) {
						_currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j] = _movementGridForSecondaryAnim[i][j];
						_currentRoomData->mouseGrid[_oldposx + i][_oldposy + j] = _mouseGridForSecondaryAnim[i][j];
					} else {
						if (_maskGridSecondaryAnim[i][j] > 0)
							_currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j] = _maskGridSecondaryAnim[i][j];
						if (_maskMouseSecondaryAnim[i][j] > 0)
							_currentRoomData->mouseGrid[_oldposx + i][_oldposy + j] = _maskMouseSecondaryAnim[i][j];
					}
				}
			}
		}
	}
}

} // End of namespace Tot
