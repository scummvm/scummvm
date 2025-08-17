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

#include "common/debug.h"
#include "common/endian.h"

#include "tot/anims.h"
#include "tot/debug.h"
#include "tot/statics.h"
#include "tot/tot.h"
#include "tot/vars.h"

namespace Tot {

void TotEngine::drawText(uint number) {
	readTextFile();
	uint foo = 0;
	sayLine(number, 255, 0, foo, false);
	_verbFile.close();
}

void TotEngine::displayLoading() {
	const char *msg = (_lang == Common::ES_ESP) ? fullScreenMessages[0][58] : fullScreenMessages[1][58];

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
		if (gameTick) {
			if (secTrajIndex == secTrajLength)
				exitLoop = true;
			secTrajIndex += 1;
			if (_iframe2 >= _secondaryAnimationFrameCount - 1)
				_iframe2 = 0;
			else
				_iframe2++;
			_secondaryAnimation.posx = devilTrajectory[secTrajIndex][0] - 15;
			_secondaryAnimation.posy = devilTrajectory[secTrajIndex][1] - 42;
			if (secTrajIndex >= 0 && secTrajIndex <= 8) {
				_secondaryAnimation.dir = 2;
				_secondaryAnimation.depth = 1;
			} else if (secTrajIndex >= 9 && secTrajIndex <= 33) {
				_secondaryAnimation.dir = 2;
				_secondaryAnimation.depth = 14;
			} else if (secTrajIndex >= 34 && secTrajIndex <= 63) {
				_secondaryAnimation.dir = 1;
				_secondaryAnimation.depth = 14;
			} else {
				_secondaryAnimation.dir = 0;
				_secondaryAnimation.depth = 3;
			}

			gameTickHalfSpeed = true;
			sprites(false);
			gameTick = false;
			_graphics->advancePaletteAnim();
			_screen->update();
		}
	} while (!exitLoop && !shouldQuit());
	freeAnimation();
	_graphics->restoreBackground();
	assembleScreen();
	_graphics->drawScreen(sceneBackground);
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
		g_engine->oldGridX = 0;
		g_engine->oldGridY = 0;
	}
}

void sprites(bool drawCharacter);

static uint curDepth;
/**
 * Point of origin of the area surrounding the main character.
 * Calculated using the position of the character.
 */
static uint dirtyMainSpriteX, dirtyMainSpriteY;
/**
 * End point of origin of the area surrounding the main character.
 * Calculated using the position of the character + dimension
 */
static uint dirtyMainSpriteX2, dirtyMainSpriteY2;

static void assembleBackground() {
	uint posabs;


	// copies the entire clean background in backgroundCopy back into background
	g_engine->_graphics->restoreBackground();

	posabs = 4 + dirtyMainSpriteY * 320 + dirtyMainSpriteX;
	uint16 w, h;
	w = READ_LE_UINT16(g_engine->characterDirtyRect);
	h = READ_LE_UINT16(g_engine->characterDirtyRect + 2);
	w++;
	h++;

	dirtyMainSpriteX2 = dirtyMainSpriteX + w;
	dirtyMainSpriteY2 = dirtyMainSpriteY + h;

	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			int pos = posabs + j * 320 + i;
			int destPos = 4 + (j * w + i);
			g_engine->characterDirtyRect[destPos] = g_engine->sceneBackground[pos];
		}
	}
}

/**
 * calculates the overlapping area between the source image and the background,
 * then "blits" (copies) the pixels from the image to the overlapping area of the background, respecting transparency.
 */
static void assembleImage(byte *img, uint imgPosX, uint imgPosY) {

	uint x, // starting point of the overlap
		y;
	uint incx, // width and height of the overlapping area
		incy;

	uint16 wImg = READ_LE_UINT16(img) + 1;
	uint16 hImg = READ_LE_UINT16(img + 2) + 1;

	uint16 wBg = READ_LE_UINT16(g_engine->characterDirtyRect) + 1;
	uint16 hBg = READ_LE_UINT16(g_engine->characterDirtyRect + 2) + 1;

	// This region calculates the overlapping area of (x, incx, y, incy)
	{
		if (imgPosX < dirtyMainSpriteX)
			x = dirtyMainSpriteX;
		else
			x = imgPosX;

		if (imgPosX + wImg < dirtyMainSpriteX + wBg)
			incx = imgPosX + wImg - x;
		else
			incx = dirtyMainSpriteX + wBg - x;

		if (imgPosY < dirtyMainSpriteY)
			y = dirtyMainSpriteY;
		else
			y = imgPosY;

		if (imgPosY + hImg < dirtyMainSpriteY + hBg)
			incy = imgPosY + hImg - y;
		else
			incy = dirtyMainSpriteY + hBg - y;
	} // end of region calculating overlapping area

	for (int j = 0; j < incy; j++) {
		for (int i = 0; i < incx; i++) {
			int bgOffset = 4 + ((y - dirtyMainSpriteY) + j) * wBg + i + (x - dirtyMainSpriteX);
			int imgOffset = 4 + (y - imgPosY + j) * wImg + i + (x - imgPosX);
			if (img[imgOffset] != 0) {
				g_engine->characterDirtyRect[bgOffset] = img[imgOffset];
			}
		}
	}
}

static void overlayLayers() {
	if (g_engine->_screenLayers[curDepth] != NULL) {
		if (
			(g_engine->_depthMap[curDepth].posx <= dirtyMainSpriteX2) &&
			(g_engine->_depthMap[curDepth].posx2 > dirtyMainSpriteX) &&
			(g_engine->_depthMap[curDepth].posy < dirtyMainSpriteY2) &&
			(g_engine->_depthMap[curDepth].posy2 > dirtyMainSpriteY)) {
			assembleImage(g_engine->_screenLayers[curDepth], g_engine->_depthMap[curDepth].posx, g_engine->_depthMap[curDepth].posy);
		}
	}
}

void drawMainCharacter() {

	bool debug = false;
	if (debug) {
		g_engine->_graphics->sceneTransition(false, g_engine->sceneBackground, 13);
	}

	uint16 tempW;
	uint16 tempH;
	tempW = READ_LE_UINT16(g_engine->_curCharacterAnimationFrame);
	tempH = READ_LE_UINT16(g_engine->_curCharacterAnimationFrame + 2);
	tempW += 6;
	tempH += 6;

	g_engine->characterDirtyRect = (byte *)malloc((tempW + 1) * (tempH + 1) + 4);

	WRITE_LE_UINT16(g_engine->characterDirtyRect, tempW);
	WRITE_LE_UINT16(g_engine->characterDirtyRect + 2, tempH);

	assembleBackground();
	curDepth = 0;
	while (curDepth != kDepthLevelCount) {
		overlayLayers();
		if (g_engine->_mainCharAnimation.depth == curDepth)
			assembleImage(g_engine->_curCharacterAnimationFrame, g_engine->_characterPosX, g_engine->_characterPosY);
		curDepth += 1;
	}

	g_engine->_graphics->putImg(dirtyMainSpriteX, dirtyMainSpriteY, g_engine->characterDirtyRect);

	if (debug) {
		// draw background dirty area
		drawRect(2, dirtyMainSpriteX, dirtyMainSpriteY, dirtyMainSpriteX + tempW, dirtyMainSpriteY + tempH);
		drawPos(g_engine->_xframe2, g_engine->_yframe2, 218);
	}
	free(g_engine->characterDirtyRect);
}

void TotEngine::sprites(bool drawMainCharachter) {
	// grabs the current frame from the walk cycle
	_curCharacterAnimationFrame = _mainCharAnimation.bitmap[_charFacingDirection][_iframe];

	dirtyMainSpriteX = _characterPosX - 3;
	dirtyMainSpriteY = _characterPosY - 3;
	if (_isSecondaryAnimationEnabled) {
		if (_currentRoomData->secondaryTrajectoryLength > 1) {
			updateMovementGrids();
		}
		if (gameTickHalfSpeed) {
			if (_isPeterCoughing && !_sound->isVocPlaying()) {
				_iframe2 = 0;
			}
			_curSecondaryAnimationFrame = _secondaryAnimation.bitmap[_secondaryAnimation.dir][_iframe2];
		}
		uint16 curCharFrameW = READ_LE_UINT16(_curCharacterAnimationFrame);
		uint16 curCharFrameH = READ_LE_UINT16(_curCharacterAnimationFrame + 2);

		uint16 secAnimW = READ_LE_UINT16(_curSecondaryAnimationFrame);
		uint16 secAnimH = READ_LE_UINT16(_curSecondaryAnimationFrame + 2);

		if (
			((_secondaryAnimation.posx < (_characterPosX + curCharFrameW) + 4) &&
			 ((_secondaryAnimation.posx + secAnimW + 1) > dirtyMainSpriteX) &&
			 (_secondaryAnimation.posy < (_characterPosY + curCharFrameH + 4))) &&
			((_secondaryAnimation.posy + secAnimH + 1) > dirtyMainSpriteY)) { // Character is in the area of the animation

			if (_secondaryAnimation.posx < _characterPosX) {
				dirtyMainSpriteX = _secondaryAnimation.posx - 3;
			}
			if (_secondaryAnimation.posy < _characterPosY) {
				dirtyMainSpriteY = _secondaryAnimation.posy - 3;
			}

			uint16 patchW = secAnimW + curCharFrameW + 6;

			uint16 patchH;
			if ((curCharFrameH + _characterPosY) > (_secondaryAnimation.posy + secAnimH)) {
				patchH = curCharFrameH + 6 + abs(_characterPosY - _secondaryAnimation.posy);
			} else {
				patchH = secAnimH + 6 + abs(_characterPosY - _secondaryAnimation.posy);
			}

			if (dirtyMainSpriteY + patchH > 140) {
				patchH -= (dirtyMainSpriteY + patchH) - 140 + 1;
			}

			if (dirtyMainSpriteX + patchW > 320) {
				patchW -= (dirtyMainSpriteX + patchW) - 320 + 1;
			}

			characterDirtyRect = (byte *)malloc((patchW + 1) * (patchH + 1) + 4);

			WRITE_LE_UINT16(characterDirtyRect, patchW);
			WRITE_LE_UINT16(characterDirtyRect + 2, patchH);

			assembleBackground();
			curDepth = 0;
			while (curDepth != kDepthLevelCount) {
				overlayLayers();
				if (_secondaryAnimation.depth == curDepth)
					assembleImage(_curSecondaryAnimationFrame, _secondaryAnimation.posx, _secondaryAnimation.posy);
				if (_mainCharAnimation.depth == curDepth)
					assembleImage(_curCharacterAnimationFrame, _characterPosX, _characterPosY);
				curDepth += 1;
			}
			_graphics->putImg(dirtyMainSpriteX, dirtyMainSpriteY, characterDirtyRect);
		} else { // character and animation are in different parts of the screen

			if (drawMainCharachter) {
				drawMainCharacter();
			}

			dirtyMainSpriteX = _secondaryAnimation.posx - 3;
			dirtyMainSpriteY = _secondaryAnimation.posy - 3;

			secAnimW = READ_LE_UINT16(_curSecondaryAnimationFrame) + 6;
			secAnimH = READ_LE_UINT16(_curSecondaryAnimationFrame + 2) + 6;

			if (dirtyMainSpriteY + secAnimH > 140) {
				secAnimH -= (dirtyMainSpriteY + secAnimH) - 140 + 1;
			}

			if (dirtyMainSpriteX + secAnimW > 320) {
				secAnimW -= (dirtyMainSpriteX + secAnimW) - 320 + 1;
			}

			characterDirtyRect = (byte *)malloc((secAnimW + 1) * (secAnimH + 1) + 4);
			WRITE_LE_UINT16(characterDirtyRect, secAnimW);
			WRITE_LE_UINT16(characterDirtyRect + 2, secAnimH);

			assembleBackground();
			curDepth = 0;
			while (curDepth != kDepthLevelCount) {
				overlayLayers();
				if (_secondaryAnimation.depth == curDepth)
					assembleImage(_curSecondaryAnimationFrame, _secondaryAnimation.posx, _secondaryAnimation.posy);
				curDepth += 1;
			}
			_graphics->putImg(dirtyMainSpriteX, dirtyMainSpriteY, characterDirtyRect);
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
		gameTick = false;
		_graphics->advancePaletteAnim();
		sprites(true);
	} while (_currentTrajectoryIndex != _trajectoryLength);
	_charFacingDirection = 3;
	emptyLoop();
	gameTick = false;
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
		gameTick = false;
		_graphics->advancePaletteAnim();
		sprites(true);
	} while (_currentTrajectoryIndex != 0);
	emptyLoop();
	gameTick = false;
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
				gameTick = false;
				if (gameTickHalfSpeed) {
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
				gameTick = false;
				if (gameTickHalfSpeed) {
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
			gameTick = false;
			if (gameTickHalfSpeed) {
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
		gameTick = false;
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
			gameTick = false;
			_graphics->advancePaletteAnim();
			_graphics->putShape(animX, animY, animptr);
		}
		_screenLayers[_curObject.depth - 1] = NULL;
		_graphics->restoreBackground();
		animIndex = _mainCharAnimation.depth;
		_mainCharAnimation.depth = 30;
		_screenLayers[13] = animptr;
		_depthMap[13].posx = animX;
		_depthMap[13].posy = animY;
		assembleScreen();
		_graphics->drawScreen(sceneBackground);
		_screenLayers[13] = NULL;
		_mainCharAnimation.depth = animIndex;
		drawBackpack();
		for (animIndex = 32; animIndex <= secFrameCount; animIndex++) {
			animationFile.read(animptr, animFrameSize);
			emptyLoop();
			gameTick = false;
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
			gameTick = false;
			_graphics->putShape(animX, animY, animptr);
			if (shouldQuit()) {
				break;
			}
		}
		animIndex = _mainCharAnimation.depth;
		_mainCharAnimation.depth = 30;
		_screenLayers[12] = animptr;
		_depthMap[12].posx = animX;
		_depthMap[12].posy = animY;
		disableSecondAnimation();
		_screenLayers[12] = NULL;
		_mainCharAnimation.depth = animIndex;
		_graphics->drawScreen(sceneBackground);
		for (animIndex = 9; animIndex <= secFrameCount; animIndex++) {
			animationFile.read(animptr, animFrameSize);
			emptyLoop();
			gameTick = false;
			emptyLoop();
			gameTick = false;
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
		gameTick = false;
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
		_screenLayers[6] = (byte *)malloc(animFrameSize);
		_depthMap[6].posx = _secondaryAnimation.posx + 5;
		_depthMap[6].posy = _secondaryAnimation.posy - 6;
		animIndex = 0;
		do {
			emptyLoop();
			gameTick = false;
			_graphics->advancePaletteAnim();
			if (gameTickHalfSpeed) {
				animationFile.read(_screenLayers[6], animFrameSize);
				Common::copy(_screenLayers[6], _screenLayers[6] + animFrameSize, sceneBackground + 44900);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animIndex += 1;
				if (animIndex == 8)
					_sound->playVoc("PUFF", 191183, 18001);
			}
		} while (animIndex != secFrameCount && !shouldQuit());
		animationFile.close();
		_sound->stopVoc();
		_screenLayers[6] = NULL;
		_currentRoomData->animationFlag = true;
	} break;
	}
}

DoorRegistry readDoorMetadata(Common::SeekableReadStream *screenDataFile) {
	DoorRegistry doorMetadata;
	doorMetadata.nextScene = screenDataFile->readUint16LE();
	doorMetadata.exitPosX = screenDataFile->readUint16LE();
	doorMetadata.exitPosY = screenDataFile->readUint16LE();
	doorMetadata.openclosed = screenDataFile->readByte();
	doorMetadata.doorcode = screenDataFile->readByte();

	return doorMetadata;
}

Common::Point readPoint(Common::SeekableReadStream *screenDataFile) {
	Common::Point point;
	point.x = screenDataFile->readUint16LE();
	point.y = screenDataFile->readUint16LE();
	return point;
}

RoomBitmapRegister readAuxBitmaps(Common::SeekableReadStream *screenDataFile) {
	RoomBitmapRegister bitmapMetadata = RoomBitmapRegister();
	bitmapMetadata.bitmapPointer = screenDataFile->readSint32LE();
	bitmapMetadata.bitmapSize = screenDataFile->readUint16LE();
	bitmapMetadata.coordx = screenDataFile->readUint16LE();
	bitmapMetadata.coordy = screenDataFile->readUint16LE();
	bitmapMetadata.depth = screenDataFile->readUint16LE();
	return bitmapMetadata;
}

RoomObjectListEntry *readRoomObjects(Common::SeekableReadStream *screenDataFile) {
	RoomObjectListEntry *objectMetadata = new RoomObjectListEntry();
	objectMetadata->fileIndex = screenDataFile->readUint16LE();
	objectMetadata->objectName = screenDataFile->readPascalString();

	screenDataFile->skip(20 - objectMetadata->objectName.size());

	return objectMetadata;
}

RoomFileRegister *TotEngine::readScreenDataFile(Common::SeekableReadStream *screenDataFile) {
	RoomFileRegister *screenData = new RoomFileRegister();
	screenData->code = screenDataFile->readUint16LE();
	screenData->roomImagePointer = screenDataFile->readUint32LE();
	screenData->roomImageSize = screenDataFile->readUint16LE();
	screenDataFile->read(screenData->walkAreasGrid, 40 * 28);
	screenDataFile->read(screenData->mouseGrid, 40 * 28);
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 30; j++) {
			for (int k = 0; k < 5; k++) {
				screenData->trajectories[i][j][k] = readPoint(screenDataFile);
			}
		}
	}

	for (int i = 0; i < 5; i++) {
		screenData->doors[i] = readDoorMetadata(screenDataFile);
	}
	for (int i = 0; i < 15; i++) {
		screenData->screenLayers[i] = readAuxBitmaps(screenDataFile);
	}
	for (int i = 0; i < 51; i++) {
		screenData->screenObjectIndex[i] = readRoomObjects(screenDataFile);
	}

	screenData->animationFlag = screenDataFile->readByte();
	screenData->animationName = screenDataFile->readPascalString();
	screenDataFile->skip(8 - screenData->animationName.size());
	screenData->paletteAnimationFlag = screenDataFile->readByte();
	screenData->palettePointer = screenDataFile->readUint16LE();
	for (int i = 0; i < 300; i++) {
		screenData->secondaryAnimTrajectory[i] = readPoint(screenDataFile);
	}
	screenDataFile->read(screenData->secondaryAnimDirections, 600);
	screenData->secondaryTrajectoryLength = screenDataFile->readUint16LE();
	return screenData;
}

void TotEngine::loadScreenData(uint screenNumber) {
	debug("Opening screen %d", screenNumber);
	currentRoomNumber = screenNumber;

	_rooms->seek(screenNumber * kRoomRegSize, SEEK_SET);
	_currentRoomData = readScreenDataFile(_rooms);
	loadScreen();
	for (int i = 0; i < 15; i++) {
		RoomBitmapRegister &bitmap = _currentRoomData->screenLayers[i];
		if (bitmap.bitmapSize > 0) {
			loadItem(bitmap.coordx, bitmap.coordy, bitmap.bitmapSize, bitmap.bitmapPointer, bitmap.depth);
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
	palette secPalette;

	_cpCounter2 = _cpCounter;
	_mouse->hide();
	_graphics->copyPalette(g_engine->_graphics->_pal, secPalette);
	readItemRegister(_inventory[objectCode].code);
	_graphics->getImg(0, 0, 319, 139, sceneBackground);
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
			0                // color5
		);

	drawMenu(4);

	if (_curObject.used[0] != 9) {
		if (_curObject.beforeUseTextRef != 0) {
			readTextFile();
			textRef = readVerbRegister(_curObject.beforeUseTextRef);
			description = textRef.text;
			for (yaux = 0; yaux < textRef.text.size(); yaux++)
				description.setChar(_decryptionKey[yaux] ^ textRef.text[yaux], yaux);
			displayObjectDescription(description, 60, 15, 33, 255, 0);
			_verbFile.close();
		} else {
			description = _inventory[objectCode].objectName;
			displayObjectDescription(description, 60, 15, 33, 255, 0);
		}
	} else {

		if (_curObject.afterUseTextRef != 0) {
			readTextFile();
			textRef = readVerbRegister(_curObject.afterUseTextRef);
			description = textRef.text;
			for (yaux = 0; yaux < textRef.text.size(); yaux++)
				description.setChar(_decryptionKey[yaux] ^ textRef.text[yaux], yaux);
			displayObjectDescription(description, 60, 15, 33, 255, 0);
			_verbFile.close();
		} else {
			description = _inventory[objectCode].objectName;
			displayObjectDescription(description, 60, 15, 33, 255, 0);
		}
	}

	drawFlc(125, 70, _curObject.rotatingObjectAnimation, 60000, 9, 0, false, true, true, foobar);

	_graphics->sceneTransition(true, NULL, 3);
	_graphics->partialFadeOut(234);
	assembleScreen();
	_graphics->drawScreen(sceneBackground);
	_graphics->copyPalette(secPalette, g_engine->_graphics->_pal);
	_graphics->partialFadeIn(234);
	_mouse->show();
}

void TotEngine::useInventoryObjectWithInventoryObject(uint objectCode1, uint objectCode2) {
	byte invIndex, indobj1, indobj2;

	debug("Reading item register %d", objectCode1);
	readItemRegister(_invItemData, objectCode1, _curObject);
	// verifyCopyProtection2();
	if (_curObject.used[0] != 1 || _curObject.useWith != objectCode2) {
		drawText(Random(11) + 1022);
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
	uint textRef = _curObject.useTextRef;

	if (_curObject.replaceWith == 0) {
		readItemRegister(_invItemData, objectCode1, _curObject);
		_curObject.used[0] = 9;
		saveItemRegister(_curObject, _invItemData);

		readItemRegister(_invItemData, objectCode2, _curObject);
		_curObject.used[0] = 9;
		saveItemRegister(_curObject, _invItemData);

	} else {
		readItemRegister(_invItemData, _curObject.replaceWith, _curObject);
		_inventory[indobj1].bitmapIndex = _curObject.objectIconBitmap;
		_inventory[indobj1].code = _curObject.code;
		_inventory[indobj1].objectName = _curObject.name;
		for (indobj1 = indobj2; indobj1 < (kInventoryIconCount - 1); indobj1++) {
			_inventory[indobj1].bitmapIndex = _inventory[indobj1 + 1].bitmapIndex;
			_inventory[indobj1].code = _inventory[indobj1 + 1].code;
			_inventory[indobj1].objectName = _inventory[indobj1 + 1].objectName;
		}
		_mouse->hide();
		drawBackpack();
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
			zone2 == 24 || zone2 == 25) {

			_targetZone = 7;
			_mouse->mouseClickX = 232;
			_mouse->mouseClickY = 75;
			zone2 = 7;
		}
		if (zone2 == 24) {
			barredZone = false;
		}
	}
	if (zone1 < 10) {
		_xframe2 = _mouse->mouseClickX + 7;
		_yframe2 = _mouse->mouseClickY + 7;

		_mouse->hide();
		calculateRoute(zone1, zone2, true, barredZone);
		Common::Event e;
		do {
			_chrono->updateChrono();
			while (g_system->getEventManager()->pollEvent(e)) {
				changeGameSpeed(e);
			}
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
	if (animationPos >= 0 && animationPos <= 56) {
		_secondaryAnimation.depth = 0;
	} else if (animationPos >= 57 && animationPos <= 66) {
		_secondaryAnimation.depth = 1;
	} else if (animationPos >= 65 && animationPos <= 74) {
		_secondaryAnimation.depth = 2;
	} else if (animationPos >= 73 && animationPos <= 82) {
		_secondaryAnimation.depth = 3;
	} else if (animationPos >= 81 && animationPos <= 90) {
		_secondaryAnimation.depth = 4;
	} else if (animationPos >= 89 && animationPos <= 98) {
		_secondaryAnimation.depth = 5;
	} else if (animationPos >= 97 && animationPos <= 106) {
		_secondaryAnimation.depth = 6;
	} else if (animationPos >= 105 && animationPos <= 114) {
		_secondaryAnimation.depth = 7;
	} else if (animationPos >= 113 && animationPos <= 122) {
		_secondaryAnimation.depth = 8;
	} else if (animationPos >= 121 && animationPos <= 140) {
		_secondaryAnimation.depth = 9;
	}
}

void TotEngine::updateMainCharacterDepth() {
	if (_characterPosY >= 0 && _characterPosY <= 7) {
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
	if (gameTick) {

		if (_currentRoomData->animationFlag && gameTickHalfSpeed) {
			if (_isPeterCoughing && (Random(100) == 1) && !_sound->isVocPlaying() && _mintTopic[0] == false) {
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
		gameTick = false;
		_graphics->advancePaletteAnim();
	}
}

void TotEngine::actionLineText(Common::String actionLine) {
	euroText(160, 144, actionLine, 255, Graphics::kTextAlignCenter);
}

void TotEngine::animateGive(uint dir, uint height) {
	_charFacingDirection = dir;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		gameTick = false;
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
		gameTick = false;
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
		gameTick = false;

		_iframe = 15 + 5 + height * 10 + (i + 1);

		_graphics->advancePaletteAnim();
		sprites(true);
		_screen->update();
	}
	emptyLoop();
	gameTick = false;
	sprites(true);
	_iframe = 0;
}

void TotEngine::animateOpen2(uint dir, uint height) {
	_charFacingDirection = dir;
	_cpCounter = _cpCounter2;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		gameTick = false;
		_iframe = 15 + 6 + height * 10 - (i + 1);

		_graphics->advancePaletteAnim();
		sprites(true);
	}
	emptyLoop();
	gameTick = false;
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
	byte curAnimIFrame;
	uint currAnimWidth, curAnimHeight,
		curAnimIdx, curAnimLength, curAnimX, curAnimY, curAnimDepth, curAnimDir;

	bool loopBreak = false;
	if (_currentRoomData->animationFlag) {
		curAnimIFrame = _iframe2;
		curAnimX = _secondaryAnimation.posx;
		curAnimY = _secondaryAnimation.posy;
		currAnimWidth = _secondaryAnimWidth;
		curAnimHeight = _secondaryAnimHeight;
		curAnimDepth = _secondaryAnimation.depth;
		curAnimDir = _secondaryAnimation.dir;
		freeAnimation();
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
		if (gameTick) {
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
			gameTick = false;
			if (curAnimIdx % 24 == 0)
				_sound->playVoc();
			_graphics->advancePaletteAnim();
		}
		_screen->update();
		g_system->delayMillis(10);
	} while (!loopBreak && !shouldQuit());

	_sound->stopVoc();
	freeAnimation();
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
	_graphics->drawScreen(sceneBackground);
}

void TotEngine::updateVideo() {
	readBitmap(_curObject.dropOverlay, _screenLayers[_curObject.depth - 1], _curObject.dropOverlaySize, 319);
	_graphics->restoreBackground();
	assembleScreen();
	_graphics->drawScreen(sceneBackground);
}

void TotEngine::nicheAnimation(byte nicheDir, int32 bitmap) {
	uint bitmapOffset;
	int increment;

	// Room with Red
	if (_currentRoomData->code == 24) {
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
		Common::copy(_screenLayers[0], _screenLayers[0] + 892, sceneBackground + 44900);
		readBitmap(bitmap, _screenLayers[0], 892, 319);
		Common::copy(_screenLayers[0] + 4, _screenLayers[0] + 4 + 888, sceneBackground + 44900 + 892);
	} break;
	case 1: { // object slides to reveal empty stand
		bitmapOffset = 892 + 44900;
		increment = -1;
		// Reads the empty niche into a non-visible part of background
		readBitmap(bitmap, sceneBackground + 44900, 892, 319);
		// Copies whatever is currently on the niche in a non-visible part of background contiguous with the above
		Common::copy(_screenLayers[0] + 4, _screenLayers[0] + 4 + 888, sceneBackground + 44900 + 892);
		// We now have in consecutive pixels the empty stand and the object

	} break;
	}
	uint16 nicheWidth = READ_LE_UINT16(_screenLayers[0]);
	uint16 nicheHeight = READ_LE_UINT16(_screenLayers[0] + 2);

	// Set the height to double to animate 2 images of the same height moving up/down
	*(sceneBackground + 44900 + 2) = (nicheHeight * 2) + 1;

	_graphics->restoreBackground();

	for (uint i = 1; i <= nicheHeight; i++) {

		bitmapOffset = bitmapOffset + (increment * (nicheWidth + 1));
		Common::copy(sceneBackground + bitmapOffset, sceneBackground + bitmapOffset + 888, _screenLayers[0] + 4);
		assembleScreen();
		_graphics->drawScreen(sceneBackground);
		_screen->update();
	}
	readBitmap(bitmap, _screenLayers[0], 892, 319);

	_graphics->restoreBackground();
	assembleScreen();
	_graphics->drawScreen(sceneBackground);

	if (_currentRoomData->code == 24) {
		free(_screenLayers[1]);
		_screenLayers[1] = NULL;
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
	readItemRegister(screenObject);
	goToObject(
		_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount],
		_currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);
	verifyCopyProtection();
	if (_curObject.pickupable) {
		_mouse->hide();
		switch (_curObject.code) {
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
		switch (_curObject.height) {
		case 0: { // Pick up above
			switch (_curObject.code) {
			case 590: { // Ectoplasm
				animatePickup1(3, 0);
				animatePickup2(3, 0);
			} break;
			default: {
				animatePickup1(_charFacingDirection, 0);
				_screenLayers[_curObject.depth - 1] = NULL;
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animatePickup2(_charFacingDirection, 0);
			}
			}
		} break;
		case 1: { // Waist level
			switch (_curObject.code) {
			case 218: { // Necronomicon
				animatePickup1(0, 1);
				animatePickup2(0, 1);
			} break;
			case 223: { // table cloths
				animatePickup1(0, 1);
				_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject.replaceWith;
				updateVideo();
				animatePickup2(0, 1);
			} break;
			case 308: { // Mistletoe
				animatePickup1(0, 1);
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
			case 517: { // mints
				animatePickup1(0, 1);
				animatePickup2(0, 1);
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
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[2];

					with.bitmapPointer = 730743;
					with.bitmapSize = 64;
					with.coordx = 76;
					with.coordy = 62;
					with.depth = 1;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				_screenLayers[_curObject.depth - 1] = NULL;
				_graphics->restoreBackground();

				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animatePickup2(0, 1);
			} break;
			case 562: { // niche
				switch (_currentRoomData->code) {
				case 20: { // First scene with niche
					if (_niche[0][_niche[0][3]] > 0) {
						// Possibly
						if (_niche[0][3] == 2 || _niche[0][_niche[0][3]] == 563) {
							readItemRegister(_niche[0][_niche[0][3]]);
							_niche[0][_niche[0][3]] = 0;
							_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
							animatePickup1(3, 1);
							readBitmap(1190768, _screenLayers[_curObject.depth - 1], 892, 319);
							_currentRoomData->screenLayers[1].bitmapPointer = 1190768;
							_currentRoomData->screenLayers[1].bitmapSize = 892;
							_currentRoomData->screenLayers[1].coordx = 66;
							_currentRoomData->screenLayers[1].coordy = 35;
							_currentRoomData->screenLayers[1].depth = 1;
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(sceneBackground);
							animatePickup2(3, 1);
						} else {
							readItemRegister(_niche[0][_niche[0][3]]);
							_niche[0][_niche[0][3]] = 0;
							_niche[0][3] += 1;
							_niche[1][3] -= 1;
							_currentRoomData->screenObjectIndex[9]->objectName = "                    ";
							animatePickup1(3, 1);
							readBitmap(1190768, _screenLayers[_curObject.depth - 1],
									   892, 319);
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(sceneBackground);
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
						drawText(1049 + Random(10));
						_mouse->hide();
						return;
					}
				} break;
				case 24: { // Second scene with niche
					if (_niche[1][_niche[1][3]] > 0 && _niche[1][3] != 1) {
						if (_niche[1][3] == 2) {
							readItemRegister(_niche[1][2]);
							_niche[1][2] = 0;
							_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
							animatePickup1(0, 1);
							readBitmap(1399610, _screenLayers[_curObject.depth - 1], 892, 319);
							_currentRoomData->screenLayers[0].bitmapPointer = 1399610;
							_currentRoomData->screenLayers[0].bitmapSize = 892;
							_currentRoomData->screenLayers[0].coordx = 217;
							_currentRoomData->screenLayers[0].coordy = 48;
							_currentRoomData->screenLayers[0].depth = 1;
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(sceneBackground);
							animatePickup2(0, 1);
						} else {
							readItemRegister(_niche[1][_niche[1][3]]);
							_niche[1][_niche[1][3]] = 622;
							_niche[1][3] += 1;
							_niche[0][3] -= 1;
							_currentRoomData->screenObjectIndex[8]->objectName = "                    ";
							animatePickup1(0, 1);
							readBitmap(1399610, _screenLayers[0], 892, 319);
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(sceneBackground);
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
						drawText(1049 + Random(10));
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
				_screenLayers[3] = NULL;
				disableSecondAnimation();
				_graphics->drawScreen(sceneBackground);
				animatePickup2(2, 1);
				_isRedDevilCaptured = true;
				_isTrapSet = false;
			} break;
			default: {
				animatePickup1(_charFacingDirection, 1);
				_screenLayers[_curObject.depth - 1] = NULL;
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animatePickup2(_charFacingDirection, 1);
			}
			}
		} break;
		case 2: { // Pick up feet level
			switch (_curObject.code) {
			case 216: { // chisel
				animatePickup1(0, 2);
				_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject.replaceWith;
				updateVideo();
				animatePickup2(0, 2);
			} break;
			case 295: { // candles
				animatePickup1(3, 2);
				_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject.replaceWith;
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
				_screenLayers[_curObject.depth - 1] = NULL;
				{ // bird
					RoomBitmapRegister &with = _currentRoomData->screenLayers[2];

					with.bitmapPointer = 1545924;
					with.bitmapSize = 172;
					with.coordx = 38;
					with.coordy = 58;
					with.depth = 1;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{ // ring
					RoomBitmapRegister &with = _currentRoomData->screenLayers[1];

					with.bitmapPointer = 1591272;
					with.bitmapSize = 92;
					with.coordx = 50;
					with.coordy = 58;
					with.depth = 3;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animatePickup2(3, 2);
			} break;
			default: {
				animatePickup1(_charFacingDirection, 2);
				_screenLayers[_curObject.depth - 1] = NULL;
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animatePickup2(_charFacingDirection, 2);
			}
			}
		} break;
		case 9: { // bat
			uint textRef = _curObject.pickTextRef;
			readItemRegister(204);
			animatePickup1(0, 1);
			animateOpen2(0, 1);
			animateBat();
			_mouse->show();
			drawText(textRef);
			_currentRoomData->mouseGrid[34][8] = 24;
			_actionCode = 0;
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
			return;
		} break;
		}
		_mouse->show();

		if (_curObject.code != 624)
			for (int j = _curObject.ygrid1; j <= _curObject.ygrid2; j++)
				for (int i = _curObject.xgrid1; i <= _curObject.xgrid2; i++) {
					_currentRoomData->walkAreasGrid[i][j] = _curObject.walkAreasPatch[i - _curObject.xgrid1][j - _curObject.ygrid1];
					_currentRoomData->mouseGrid[i][j] = _curObject.mouseGridPatch[i - _curObject.xgrid1][j - _curObject.ygrid1];
				}
		switch (_curObject.code) {
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
				if (_currentRoomData->screenLayers[i].bitmapPointer ==
					_curObject.bitmapPointer) {
					_currentRoomData->screenLayers[i].bitmapPointer = 0;
					_currentRoomData->screenLayers[i].bitmapSize = 0;
					_currentRoomData->screenLayers[i].coordx = 0;
					_currentRoomData->screenLayers[i].coordy = 0;
					_currentRoomData->screenLayers[i].depth = 0;
				}
		}
		}
	} else {
		if (_curObject.pickTextRef > 0)
			drawText(_curObject.pickTextRef);
		_actionCode = 0;
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
		return;
	}
	inventoryIndex = 0;

	while (_inventory[inventoryIndex].code != 0) {
		inventoryIndex += 1;
	}

	_inventory[inventoryIndex].bitmapIndex = _curObject.objectIconBitmap;
	_inventory[inventoryIndex].code = _curObject.code;
	_inventory[inventoryIndex].objectName = _curObject.name;
	_mouse->hide();
	drawBackpack();
	_mouse->show();
	if (_curObject.pickTextRef > 0)
		drawText(_curObject.pickTextRef);
	_actionCode = 0;
	oldGridX = 0;
	oldGridY = 0;
	checkMouseGrid();
}

void TotEngine::replaceBackpack(byte obj1, uint obj2) {
	readItemRegister(obj2);
	_inventory[obj1].bitmapIndex = _curObject.objectIconBitmap;
	_inventory[obj1].code = obj2;
	_inventory[obj1].objectName = _curObject.name;
	_cpCounter = _cpCounter2;
}

void TotEngine::dropObjectInScreen(ScreenObject replacementObject) {
	byte objIndex;

	if (replacementObject.bitmapSize > 0) {
		objIndex = 0;
		while (!(_currentRoomData->screenLayers[objIndex].bitmapSize == 0) || objIndex == 15) {
			objIndex++;
		}
		if (_currentRoomData->screenLayers[objIndex].bitmapSize == 0) {
			{
				RoomBitmapRegister &with = _currentRoomData->screenLayers[objIndex];

				with.bitmapPointer = replacementObject.bitmapPointer;
				with.bitmapSize = replacementObject.bitmapSize;
				with.coordx = replacementObject.dropOverlayX;
				with.coordy = replacementObject.dropOverlayY;
				with.depth = replacementObject.depth;
				loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
			}
			for (int j = replacementObject.ygrid1; j <= replacementObject.ygrid2; j++)
				for (int i = replacementObject.xgrid1; i <= replacementObject.xgrid2; i++) {
					_currentRoomData->walkAreasGrid[i][j] = replacementObject.walkAreasPatch[i - replacementObject.xgrid1][j - replacementObject.ygrid1];
					_currentRoomData->mouseGrid[i][j] = replacementObject.mouseGridPatch[i - replacementObject.xgrid1][j - replacementObject.ygrid1];
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

		readItemRegister(_inventory[usedObjectIndex].code);

		goToObject(
			_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount],
			_currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);

		if (_curObject.useWith == sceneObject && sceneObject > 0 && _curObject.used[0] == 5) {
			switch (_curObject.useWith) {
			case 30: { // corn with rooster
				drawText(_curObject.useTextRef);
				_mouse->hide();

				animateGive(1, 2);
				animatePickup2(1, 2);

				updateItem(_curObject.code);
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
					gameTick = false;
					emptyLoop2();
					sprites(true);
					_screen->update();
				} while (_currentSecondaryTrajectoryIndex != 4);

				for (listIndex = 0; listIndex < _maxXGrid; listIndex++)
					for (invIndex = 0; invIndex < _maxYGrid; invIndex++) {
						_currentRoomData->walkAreasGrid[_oldposx + listIndex][_oldposy + invIndex] = _movementGridForSecondaryAnim[listIndex][invIndex];
						_currentRoomData->mouseGrid[_oldposx + listIndex][_oldposy + invIndex] = _mouseGridForSecondaryAnim[listIndex][invIndex];
					}

				freeAnimation();
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
				drawText(_curObject.useTextRef);
				_mouse->hide();
				_sound->playVoc("BLUP", 330921, 3858);
				animateGive(3, 1);
				animatePickup2(3, 1);
				_mouse->show();
				updateItem(_curObject.code);
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
							drawBackpack();
							_mouse->show();
						} else {
							readTextFile();
							sayLine(_curObject.useTextRef, 255, 0, foo, false);
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
							for (invIndex = 0; invIndex <= kInventoryIconCount; invIndex++) {
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
							drawBackpack();
							_mouse->show();
						} else {
							readTextFile();
							sayLine(_curObject.useTextRef, 255, 0, foo, false);
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
				switch (_curObject.code) {
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
					drawBackpack();
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
				drawBackpack();
				_mouse->show();
				for (foo = 0; foo < kCharacterCount; foo++) {
					_mintTopic[foo] = true;
					_firstTimeTopicC[foo] = true;
				}
			} break;
			case 164: {
				switch (_curObject.code) {
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
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
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
						drawBackpack();
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
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
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
						drawBackpack();
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
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
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
						drawBackpack();
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
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = _currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
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
						drawBackpack();
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
					gameTick = false;
					emptyLoop2();
					sprites(true);
				} while (!(_currentSecondaryTrajectoryIndex == (_currentRoomData->secondaryTrajectoryLength / 2)));

				animateGive(3, 2);
				updateInventory(usedObjectIndex);
				drawBackpack();
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
					gameTick = false;

					emptyLoop2();
					sprites(true);
				} while (_currentSecondaryTrajectoryIndex != _currentRoomData->secondaryTrajectoryLength);
				disableSecondAnimation();
				_graphics->drawScreen(sceneBackground);
				_mouse->show();
			} break;
			case 201: {
				drawText(_curObject.useTextRef);
				_sound->playVoc("GALLO", 94965, 46007);
				_mouse->hide();
				animatedSequence(5);
				replaceBackpack(usedObjectIndex, 423);
				drawBackpack();
				_mouse->show();
				_actionCode = 0;
				oldGridX = 0;
				oldGridY = 0;
				checkMouseGrid();
			} break;
			case 219: {
				drawText(_curObject.useTextRef);
				_mouse->hide();
				_sound->playVoc("TAZA", 223698, 29066);
				animateGive(3, 2);
				do {
					_chrono->updateChrono();
					if (gameTick) {
						_graphics->advancePaletteAnim();
						gameTick = false;
					}
					_screen->update();
					g_system->delayMillis(10);
				} while (_sound->isVocPlaying());
				animateOpen2(3, 2);
				updateItem(_curObject.code);
				disableSecondAnimation();
				_graphics->drawScreen(sceneBackground);
				_mouse->show();
				drawText(2652);
				_mouse->hide();
				readItemRegister(536);
				for (int i = 12; i <= 13; i++)
					for (int j = 7; j <= 14; j++)
						_currentRoomData->mouseGrid[i][j] = 14;
				for (int i = 8; i <= 12; i++)
					_currentRoomData->mouseGrid[14][i] = 14;
				_currentRoomData->mouseGrid[9][10] = 1;
				_currentRoomData->mouseGrid[10][10] = 1;
				for (int i = 0; i < 15; i++)
					if (_currentRoomData->screenLayers[i].bitmapPointer ==
						_curObject.bitmapPointer) {
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
				_inventory[usedObjectIndex].bitmapIndex = _curObject.objectIconBitmap;
				_inventory[usedObjectIndex].code = _curObject.code;
				_inventory[usedObjectIndex].objectName = _curObject.name;
				animatedSequence(4);
				_mouse->show();
				_actionCode = 0;
				oldGridX = 0;
				oldGridY = 0;
				checkMouseGrid();
			} break;
			case 221: {
				drawText(_curObject.useTextRef);
				_curObject.used[0] = 9;
				usedObjectIndex = 0;
				while (_inventory[usedObjectIndex].code != 0) {
					usedObjectIndex += 1;
				}

				_invItemData->seek(_curObject.code);

				saveItem(_curObject, _invItemData);
				readItemRegister(_invItemData, 221, _curObject);
				_inventory[usedObjectIndex].bitmapIndex = _curObject.objectIconBitmap;
				_inventory[usedObjectIndex].code = _curObject.code;
				_inventory[usedObjectIndex].objectName = _curObject.name;

				_mouse->hide();
				animatePickup1(2, 0);
				_sound->playVoc("TIJERAS", 252764, 5242);
				_sound->waitForSoundEnd();
				animatePickup2(2, 0);
				drawBackpack();
				_mouse->show();
			} break;
			case 227: {
				drawText(_curObject.useTextRef);
				_mouse->hide();
				_sound->playVoc("ALACENA", 319112, 11809);
				animatePickup1(0, 2);
				_sound->waitForSoundEnd();
				animateOpen2(0, 2);
				replaceBackpack(usedObjectIndex, 453);
				drawBackpack();
				_mouse->show();
				updateItem(_inventory[usedObjectIndex].code);
				_isCupboardOpen = true;
			} break;
			case 274: {
				drawText(_curObject.useTextRef);
				_mouse->hide();
				_sound->playVoc("CINCEL", 334779, 19490);
				animatePickup1(_charFacingDirection, 2);
				_sound->waitForSoundEnd();
				animateOpen2(_charFacingDirection, 2);
				_mouse->show();
				updateItem(_curObject.code);
				_isChestOpen = true;
			} break;
			case 416: {
				updateItem(_curObject.code);
				_mouse->hide();
				_sound->playVoc("PUERTA", 186429, 4754);
				animatePickup1(0, 1);
				animateOpen2(0, 1);
				_sound->loadVoc("GOTA", 140972, 1029);
				_mouse->show();
				drawText(_curObject.useTextRef);
				_currentRoomData->doors[2].openclosed = 0;
			} break;
			case 446: {
				drawText(_curObject.useTextRef);
				_mouse->hide();
				_sound->playVoc("TAZA", 223698, 29066);
				animateGive(0, 2);
				_sound->waitForSoundEnd();
				animatePickup2(0, 2);
				replaceBackpack(usedObjectIndex, 204);
				drawBackpack();
				_mouse->show();
			} break;
			case 507: {
				_mouse->hide();
				animatePickup1(0, 1);
				_sound->playVoc("MAQUINA", 153470, 7378);
				animateOpen2(0, 1);
				updateInventory(usedObjectIndex);
				drawBackpack();
				_mouse->show();
				_currentRoomData->mouseGrid[27][8] = 22;
			} break;
			case 549: {
				updateItem(_curObject.code);
				_mouse->hide();
				_sound->playVoc("PUERTA", 186429, 4754);
				animatePickup1(1, 1);
				animateOpen2(1, 1);
				_mouse->show();
				drawText(_curObject.useTextRef);
				_currentRoomData->doors[0].openclosed = 0;
			} break;
			case 562: { // put any object in the niches
				switch (_currentRoomData->code) {
				case 20: {
					if (_niche[0][_niche[0][3]] == 0) {

						if (_niche[0][3] == 0) {
							_niche[0][0] = _curObject.code;
							drawText(_curObject.useTextRef);
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
							_graphics->drawScreen(sceneBackground);
							animateOpen2(3, 1);
							updateInventory(usedObjectIndex);
							drawBackpack();
							_mouse->show();
						} else {

							_niche[0][_niche[0][3]] = _curObject.code;
							_niche[1][3] += 1;
							_niche[0][3] -= 1;
							drawText(_curObject.useTextRef);
							_mouse->hide();
							animateGive(3, 1);
							switch (_curObject.code) {
							case 561:
								readBitmap(1182652, _screenLayers[0], 892, 319);
								break;
							case 615:
								readBitmap(1181760, _screenLayers[0], 892, 319);
								break;
							}
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(sceneBackground);
							animateOpen2(3, 1);
							updateInventory(usedObjectIndex);
							drawBackpack();
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
						drawText(Random(11) + 1022);
					}
				} break;
				case 24: {
					if (_niche[1][_niche[1][3]] == 0) {

						if (_niche[1][3] == 0) {

							_niche[1][0] = _curObject.code;
							drawText(_curObject.useTextRef);
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
							_graphics->drawScreen(sceneBackground);
							animateOpen2(0, 1);
							updateInventory(usedObjectIndex);
							drawBackpack();
							_mouse->show();
						} else {

							_niche[1][_niche[1][3]] = _curObject.code;
							_niche[0][3] += 1;
							_niche[1][3] -= 1;
							drawText(_curObject.useTextRef);
							_mouse->hide();
							animateGive(0, 1);

							switch (_curObject.code) {
							case 561:
								readBitmap(1381982, _screenLayers[_curObject.depth - 1],
										   892, 319);
								break;
							case 615:
								readBitmap(1381090, _screenLayers[_curObject.depth - 1],
										   892, 319);
								break;
							}
							_graphics->restoreBackground();
							assembleScreen();
							_graphics->drawScreen(sceneBackground);
							animateOpen2(0, 1);
							updateInventory(usedObjectIndex);
							drawBackpack();
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
						drawText(Random(11) + 1022);
					}
				} break;
				}
			} break;
			case 583: {

				long offset = (_lang == Common::ES_ESP) ? flcOffsets[0][20] : flcOffsets[1][20];

				drawText(_curObject.useTextRef);
				_mouse->hide();
				drawFlc(140, 34, offset, 0, 9, 24, false, false, true, foobar);
				_mouse->show();
				updateItem(_curObject.code);
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
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[2];

					with.bitmapPointer = 1240818;
					with.bitmapSize = 116;
					with.coordx = 177;
					with.coordy = 82;
					with.depth = 1;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				readBitmap(1243652, _screenLayers[5], 2718, 319);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
			} break;
			case 594: {
				drawText(_curObject.useTextRef);
				_mouse->hide();
				animateGive(3, 2);
				animatePickup2(3, 2);
				replaceBackpack(usedObjectIndex, 607);
				drawBackpack();
				_mouse->show();
			} break;
			case 608: {
				drawText(_curObject.useTextRef);
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
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				updateInventory(usedObjectIndex);
				drawBackpack();
				_isTrapSet = true;
				_mouse->show();
			} break;
			case 632: {
				long offset = (_lang == Common::ES_ESP) ? flcOffsets[0][21] : flcOffsets[1][21];
				drawText(_curObject.useTextRef);
				_mouse->hide();
				animateGive(_charFacingDirection, 1);

				// Show feather on pedestal
				loadItem(187, 70, 104, 1545820, 8);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				_screen->update();

				animateOpen2(_charFacingDirection, 1);
				_mouse->show();
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 14);
				_mouse->hide();
				_sound->playVoc("PUFF", 191183, 18001);
				// Animate to scythe
				debug("Start anim!");
				drawFlc(180, 60, offset, 0, 9, 0, false, false, true, foobar);
				debug("End Anim!");
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
				loadItem(186, 63, 464, 1447508, 8);

				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
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
				_screenLayers[7] = NULL;
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animatePickup2(1, 1);
				drawBackpack();
				_mouse->show();
				for (listIndex = 35; listIndex <= 37; listIndex++)
					for (invIndex = 21; invIndex <= 25; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 11;
				_isScytheTaken = true;
				if (_isTridentTaken)
					_caves[3] = true;
			} break;
			case 633: { //Use ring!
				long offset = (_lang == Common::ES_ESP) ? flcOffsets[0][22] : flcOffsets[1][22];

				drawText(_curObject.useTextRef);
				_mouse->hide();
				animateGive(3, 1);
				loadItem(86, 55, 92, 1591272, 8);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
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
				loadItem(82, 53, 384, 1746554, 8);
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
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
				_screenLayers[7] = NULL;
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animatePickup2(3, 1);
				drawBackpack();
				_mouse->show();
				for (listIndex = 0; listIndex <= 2; listIndex++)
					for (invIndex = 10; invIndex <= 12; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 10;
				_isTridentTaken = true;
				if (_isScytheTaken)
					_caves[3] = true;
			} break;
			case 643: { // Urn with altar
				long offset = (_lang == Common::ES_ESP) ? flcOffsets[0][23] : flcOffsets[1][23];

				if (_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount] != 5)
					drawText(_curObject.useTextRef);
				_mouse->mouseClickX = 149 - 7;
				_mouse->mouseClickY = 126 - 7;
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 5);
				_mouse->hide();
				updateInventory(usedObjectIndex);
				drawBackpack();
				drawFlc(133, 0, offset, 0, 9, 22, false, false, true, foobar);
				{
					RoomBitmapRegister &with = _currentRoomData->screenLayers[2];

					with.bitmapPointer = 1744230;
					with.bitmapSize = 824;
					with.coordx = 147;
					with.coordy = 38;
					with.depth = 9;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
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
				long offset = (_lang == Common::ES_ESP) ? flcOffsets[0][24] : flcOffsets[1][24];

				drawText(_curObject.useTextRef);
				_mouse->mouseClickX = 178 - 7;
				_mouse->mouseClickY = 71 - 7;
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 3);
				_mouse->hide();
				_sound->playVoc("AFILAR", 0, 6433);
				drawFlc(160, 15, offset, 0, 9, 23, false, false, true, foobar);
				replaceBackpack(usedObjectIndex, 715);
				drawBackpack();
				_mouse->show();
			} break;
			case 686: {
				drawText(_curObject.useTextRef);
				_isVasePlaced = true;
				_caves[4] = false;
				_mouse->hide();
				animateGive(1, 1);
				updateInventory(usedObjectIndex);
				dropObjectInScreen(_curObject);
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				drawBackpack();
				animateOpen2(1, 1);
				for (listIndex = 19; listIndex <= 21; listIndex++)
					for (invIndex = 10; invIndex <= 13; invIndex++)
						_currentRoomData->mouseGrid[listIndex][invIndex] = 13;
				_mouse->show();
			} break;
			case 689: { // rope
				long offset = (_lang == Common::ES_ESP) ? flcOffsets[0][25] : flcOffsets[1][25];

				drawText(_curObject.useTextRef);
				_mouse->mouseClickX = 124 - 7;
				_mouse->mouseClickY = 133 - 7;
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], 9);
				_mouse->hide();
				drawFlc(110, 79, offset, 0, 9, 0, false, false, true, foobar);
				replaceBackpack(usedObjectIndex, 701);
				drawBackpack();
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				_mouse->show();
				for (listIndex = 18; listIndex <= 20; listIndex++)
					_currentRoomData->mouseGrid[listIndex][26] = 10;
				for (listIndex = 17; listIndex <= 21; listIndex++)
					_currentRoomData->mouseGrid[listIndex][27] = 10;
			} break;
			case 700: { // Trident
				long offset = (_lang == Common::ES_ESP) ? flcOffsets[0][26] : flcOffsets[1][26];
				drawText(_curObject.useTextRef);
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
				_screenLayers[2] = NULL;
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
				updateItem(_curObject.code);
			} break;
			case 709: { // rock with mural
				long offset = (_lang == Common::ES_ESP) ? flcOffsets[0][27] : flcOffsets[1][27];

				if (_isSealRemoved) {
					drawText(_curObject.useTextRef);
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
						loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
					}
					_graphics->restoreBackground();
					assembleScreen();
					_graphics->drawScreen(sceneBackground);

					_sound->waitForSoundEnd();
					_sound->playVoc("PUFF", 191183, 18001);
					animateOpen2(0, 1);
					drawFlc(180, 50, offset, 0, 9, 22, false, false, true, foobar);
					_shouldQuitGame = true;
				} else
					drawText(Random(11) + 1022);
			} break;
			}
		} else {
			goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], _currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);
			if (_curObject.code == 536 || _curObject.code == 220)
				drawText(Random(6) + 1033);
			else
				drawText(Random(11) + 1022);
		}
	} else { //use object with something on the scene
		if (sceneObject > 0) {
			readItemRegister(sceneObject);
			goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], _currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);
			switch (_curObject.used[0]) {
			case 0: {
				if (_curObject.useTextRef > 0)
					drawText(_curObject.useTextRef);
			} break;
			case 9: {
				if (_curObject.afterUseTextRef > 0)
					drawText(_curObject.afterUseTextRef);
			} break;
			case 100: {
				switch (_curObject.code) {
				case 153: {
					_mouse->hide();
					animatedSequence(2);
					_mouse->show();
				} break;
				case 154: {
					_curObject.used[0] = 9;
					if (_curObject.beforeUseTextRef > 0)
						drawText(_curObject.beforeUseTextRef);
					_mouse->hide();
					animatedSequence(1);
					_mouse->show();
					drawText(1425);
					_mouse->hide();
					animatedSequence(3);
					_mouse->show();
					updateItem(_curObject.code);
					readItemRegister(152);
					usedObjectIndex = 0;
					while (_inventory[usedObjectIndex].code != 0) {
						usedObjectIndex += 1;
					}
					_inventory[usedObjectIndex].bitmapIndex = _curObject.objectIconBitmap;
					_inventory[usedObjectIndex].code = _curObject.code;
					_inventory[usedObjectIndex].objectName = _curObject.name;
					_mouse->hide();
					drawBackpack();
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
					drawText(_curObject.useTextRef);
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
					drawText(_curObject.useTextRef);
					_sound->stopVoc();
					_sound->playVoc("CARBON", 21819, 5923);
					_mouse->hide();
					animatePickup1(0, 0);
					delay(100);
					animateOpen2(0, 0);
					_mouse->show();
					updateItem(_curObject.code);
					_currentRoomData->screenObjectIndex[16]->fileIndex = 362;
					_currentRoomData->screenObjectIndex[16]->objectName = getObjectName(2);
					_currentRoomData->screenObjectIndex[1]->fileIndex = 347;
					_currentRoomData->screenObjectIndex[1]->objectName = getObjectName(3);
					_sound->stopVoc();
					_sound->autoPlayVoc("CALDERA", 6433, 15386);
				} break;
				case 682: {
					long offsetWithJar = (_lang == Common::ES_ESP) ? flcOffsets[0][28] : flcOffsets[1][28];
					long offsetNoJar = (_lang == Common::ES_ESP) ? flcOffsets[0][29] : flcOffsets[1][29];

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
				if ((_curObject.beforeUseTextRef > 0) && (_curObject.code != 154))
					drawText(_curObject.beforeUseTextRef);
			} break;
			default:
				drawText(1022 + Random(11));
			}
		}
	}
	oldGridX = 0;
	oldGridY = 0;
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

	readItemRegister(screenObject);
	debug("Read screen object = %s, with code = %d, depth=%d", _curObject.name.c_str(), _curObject.code, _curObject.depth);
	goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount],
			   _currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);

	if (_curObject.openable == false) {
		drawText(Random(9) + 1059);
		return;
	} else {
		shouldSpeak = false;
		switch (_curObject.code) {
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
				_screenLayers[_curObject.depth - 1] = NULL;
				yIndex = 0;
				while (_currentRoomData->screenLayers[yIndex].depth != _curObject.depth && yIndex != 15) {
					yIndex++;
				}
				debug("changing bitmap at %d, with depth = %d", yIndex, _currentRoomData->screenLayers[yIndex].depth);
				_currentRoomData->screenLayers[yIndex].bitmapPointer = 0;
				_currentRoomData->screenLayers[yIndex].bitmapSize = 0;
				_currentRoomData->screenLayers[yIndex].coordx = 0;
				_currentRoomData->screenLayers[yIndex].coordy = 0;
				_currentRoomData->screenLayers[yIndex].depth = 0;
				_currentRoomData->doors[2].openclosed = 1;
				_graphics->restoreBackground();
				assembleScreen();
				_graphics->drawScreen(sceneBackground);
				animateOpen2(0, 1);
				_mouse->show();
				for (yIndex = 0; yIndex <= 12; yIndex++)
					for (xIndex = 33; xIndex <= 36; xIndex++)
						_currentRoomData->mouseGrid[xIndex][yIndex] = 43;
				for (xIndex = 33; xIndex <= 35; xIndex++)
					_currentRoomData->mouseGrid[xIndex][13] = 43;
				_actionCode = 0;
				oldGridX = 0;
				oldGridY = 0;
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
				_screenLayers[_curObject.depth - 1] = NULL;
				yIndex = 0;
				while (_currentRoomData->screenLayers[yIndex].depth != _curObject.depth && yIndex != 14) {
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
				_graphics->drawScreen(sceneBackground);
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
				oldGridX = 0;
				oldGridY = 0;
				checkMouseGrid();
				return;
			}
			break;
		}
		if (shouldSpeak) {
			drawText(Random(9) + 1059);
			return;
		}
		_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject.replaceWith;
		_mouse->hide();
		switch (_curObject.height) {
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
		for (yIndex = _curObject.ygrid1; yIndex <= _curObject.ygrid2; yIndex++)
			for (xIndex = _curObject.xgrid1; xIndex <= _curObject.xgrid2; xIndex++) {
				_currentRoomData->walkAreasGrid[xIndex][yIndex] = _curObject.walkAreasPatch[xIndex - _curObject.xgrid1][yIndex - _curObject.ygrid1];
				_currentRoomData->mouseGrid[xIndex][yIndex] = _curObject.mouseGridPatch[xIndex - _curObject.xgrid1][yIndex - _curObject.ygrid1];
			}
		for (xIndex = 0; xIndex < 15; xIndex++)
			if (_currentRoomData->screenLayers[xIndex].bitmapPointer == _curObject.bitmapPointer) {
				_currentRoomData->screenLayers[xIndex].bitmapPointer = _curObject.dropOverlay;
				_currentRoomData->screenLayers[xIndex].bitmapSize = _curObject.dropOverlaySize;
			}
		_actionCode = 0;
	}
	oldGridX = 0;
	oldGridY = 0;
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
	// verifyCopyProtection2();
	readItemRegister(sceneObject);
	goToObject(_currentRoomData->walkAreasGrid[((_characterPosX + kCharacterCorrectionX) / kXGridCount)][((_characterPosY + kCharacerCorrectionY) / kYGridCount)],
			   _currentRoomData->walkAreasGrid[correctedMouseX][correctedMouseY]);
	if (_curObject.closeable == false) {
		drawText((Random(10) + 1068));
		return;
	} else {
		shouldSpeak = false;
		switch (_curObject.code) {
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
			drawText(Random(10) + 1068);
			return;
		}
		_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[correctedMouseX][correctedMouseY]]->fileIndex = _curObject.replaceWith;
		_mouse->hide();
		switch (_curObject.height) {
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
		for (yIndex = _curObject.ygrid1; yIndex <= _curObject.ygrid2; yIndex++)
			for (xIndex = _curObject.xgrid1; xIndex <= _curObject.xgrid2; xIndex++) {
				_currentRoomData->walkAreasGrid[xIndex][yIndex] = _curObject.walkAreasPatch[xIndex - _curObject.xgrid1][yIndex - _curObject.ygrid1];
				_currentRoomData->mouseGrid[xIndex][yIndex] = _curObject.mouseGridPatch[xIndex - _curObject.xgrid1][yIndex - _curObject.ygrid1];
			}
		for (xIndex = 0; xIndex < 15; xIndex++)
			if (_currentRoomData->screenLayers[xIndex].bitmapPointer == _curObject.bitmapPointer) {
				_currentRoomData->screenLayers[xIndex].bitmapPointer = _curObject.dropOverlay;
				_currentRoomData->screenLayers[xIndex].bitmapSize = _curObject.dropOverlaySize;
			}
		_actionCode = 0;
	}
	oldGridX = 0;
	oldGridY = 0;
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
		drawText((Random(10) + 1039));
		_actionCode = 0;
		if (_cpCounter > 130)
			showError(274);
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 2: {
		_mouse->hide();
		actionLineText(getActionLineText(2) + _inventory[invPos].objectName);
		if (_cpCounter2 > 13)
			showError(274);
		_mouse->show();
		drawText((Random(10) + 1049));
		_actionCode = 0;
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 3: {
		_mouse->hide();
		actionLineText(getActionLineText(3) + _inventory[invPos].objectName);
		_mouse->show();
		_actionCode = 0;
		lookAtObject(invPos);
		oldGridX = 0;
		oldGridY = 0;
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
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		}
		break;
	case 5: {
		_mouse->hide();
		actionLineText(getActionLineText(5) + _inventory[invPos].objectName);
		_mouse->show();
		drawText(Random(9) + 1059);
		_actionCode = 0;
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 6: {
		_mouse->hide();
		actionLineText(getActionLineText(6) + _inventory[invPos].objectName);
		_mouse->show();
		drawText(Random(10) + 1068);
		_actionCode = 0;
		if (_cpCounter2 > 35)
			showError(274);
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	}
}

void TotEngine::loadObjects() {

	Common::File objectFile;
	switch (_gamePart) {
	case 1:
		objectFile.open("OBJMOCH.DAT");
		break;
	case 2:
		objectFile.open("OBJMOCH.TWO");
		break;
	}

	if (!objectFile.isOpen())
		showError(312);
	for (int i = 0; i < kInventoryIconCount; i++) {
		_inventoryIconBitmaps[i] = (byte *)malloc(kInventoryIconSize);
		objectFile.read(_inventoryIconBitmaps[i], kInventoryIconSize);
	}
	if (_cpCounter > 65)
		showError(274);
	const char *emptyName = (_lang == Common::ES_ESP) ? hardcodedObjects_ES[10] : hardcodedObjects_EN[10];
	for (int i = 0; i < kInventoryIconCount; i++) {
		_inventory[i].bitmapIndex = 34;
		_inventory[i].code = 0;
		_inventory[i].objectName = emptyName;
	}

	objectFile.close();
	debug("Successfully read objects!");
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

void loadScrollData(uint roomCode, bool rightScroll,
					uint horizontalPos, int scrollCorrection);

static byte *spriteBackground;

/**
 * Blits srcImage over dstImage on the zeroed pixels of dstImage
 */
static void blit(byte *srcImage, byte *dstImage) { // Near;
	uint16 w = READ_LE_UINT16(dstImage) + 1;
	uint16 h = READ_LE_UINT16(dstImage + 2) + 1;

	uint size = w * h;
	byte *dst = dstImage + 4;
	byte *src = srcImage + 4;
	for (int i = 0; i < size; i++) {
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

void TotEngine::scrollRight(uint &horizontalPos) {

	int characterPos = 25 + (320 - (_characterPosX + kCharacterCorrectionX * 2));
	// We scroll 4 by 4 pixels so we divide by 4 to find out the number of necessary steps
	uint stepCount = (320 - horizontalPos) >> 2;
	byte *assembledCharacterFrame = (byte *)malloc(_mainCharFrameSize);
	// Number of bytes to move
	size_t numBytes = 44796;
	for (int i = 0; i < stepCount; i++) {
		// move everything to the left
		memmove(sceneBackground + 4, sceneBackground + 8, numBytes);

		horizontalPos += 4;
		for (int k = 0; k < 140; k++) {
			for (int j = 0; j < 4; j++) {
				sceneBackground[320 + k * 320 + j] = backgroundCopy[horizontalPos + k * 320 + j];
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
			_graphics->putImageArea(_characterPosX - 2, _characterPosY, sceneBackground, spriteBackground);
			uint16 pasoframeW = READ_LE_UINT16(assembledCharacterFrame);
			uint16 pasoframeH = READ_LE_UINT16(assembledCharacterFrame + 2);
			// Grabs current area surrounding character (which might contain parts of A and B)
			_graphics->getImageArea(_characterPosX, _characterPosY, _characterPosX + pasoframeW, _characterPosY + pasoframeH, sceneBackground, spriteBackground);
			// blits over the character sprite, only on black pixels
			blit(spriteBackground, assembledCharacterFrame);
			// puts it back in the background (character + piece of background)
			_graphics->putImageArea(_characterPosX, _characterPosY, sceneBackground, assembledCharacterFrame);
		} else
			_characterPosX -= 4;
		_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		_screen->update();
		_graphics->drawScreen(sceneBackground);
	}
	free(assembledCharacterFrame);
}

void TotEngine::scrollLeft(uint &horizontalPos) {

	int characterPos = 25 + _characterPosX;
	horizontalPos = 320 - horizontalPos;
	// We scroll 4 by 4 pixels so we divide by 4 to find out the number of necessary steps
	uint stepCount = horizontalPos >> 2;

	byte *assembledCharacterFrame = (byte *)malloc(_mainCharFrameSize);
	size_t numBytes = 44796;
	for (int i = stepCount; i >= 1; i--) {
		for (int j = numBytes; j > 0; j--) {
			// move the previous background to the right
			sceneBackground[j + 4] = sceneBackground[j];
		}

		horizontalPos -= 4;
		for (int k = 0; k < 140; k++) {
			for (int j = 0; j < 4; j++) {
				sceneBackground[4 + k * 320 + j] = backgroundCopy[4 + horizontalPos + k * 320 + j];
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

			_graphics->putImageArea(_characterPosX + 2, _characterPosY, sceneBackground, spriteBackground);

			uint16 pasoframeW = READ_LE_UINT16(assembledCharacterFrame);
			uint16 pasoframeH = READ_LE_UINT16(assembledCharacterFrame + 2);

			_graphics->getImageArea(_characterPosX, _characterPosY, _characterPosX + pasoframeW, _characterPosY + pasoframeH, sceneBackground, spriteBackground);
			blit(spriteBackground, assembledCharacterFrame);
			_graphics->putImageArea(_characterPosX, _characterPosY, sceneBackground, assembledCharacterFrame);
		} else
			_characterPosX += 4;

		_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		_screen->update();
		_graphics->drawScreen(sceneBackground);
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
	debug("characterPos=%d,%d, size=%d,%d", _characterPosX, _characterPosY, characterFrameW, characterFrameH);
	/* Copy the area with the player from previous scren*/
	spriteBackground = (byte *)malloc(4 + (characterFrameW + 8) * (characterFrameH + 8));
	_graphics->getImageArea(_characterPosX, _characterPosY, _characterPosX + characterFrameW, _characterPosY + characterFrameH, sceneBackground, spriteBackground);

	// Start screen 2

	_rooms->seek(roomCode * kRoomRegSize, SEEK_SET);
	_currentRoomData = readScreenDataFile(_rooms);

	loadScreen();
	// Background now contains background B, backgroundCopy contains background B
	for (int i = 0; i < 15; i++) {
		{
			RoomBitmapRegister &with = _currentRoomData->screenLayers[i];
			if (with.bitmapSize > 0)
				loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
		}
	}
	// assembles the screen objects into background
	assembleScreen(true);
	// background contains background B + objects, backgroundCopy contains plain background B

	// Copies the contents of background into backgroundCopy
	Common::copy(sceneBackground, sceneBackground + 44804, backgroundCopy);
	// background contains background B + objects, backgroundCopy contains background B + objects

	g_engine->_graphics->_paletteAnimFrame = 0;
	getScreen(sceneBackground);
	// background now contains full background A again, backgroundCopy contains background B + objects

	_graphics->drawScreen(sceneBackground);
	if (rightScroll)
		scrollRight(horizontalPos);
	else
		scrollLeft(horizontalPos);

	// After scroll is done, backgroundCopy will now contain the resulting background (background B + objects)
	Common::copy(backgroundCopy, backgroundCopy + 44804, sceneBackground);

	_characterPosX += scrollCorrection;

	assembleScreen();
	_graphics->drawScreen(sceneBackground);
	free(spriteBackground);
	loadScreen();
	_trajectory[_currentTrajectoryIndex].x = _characterPosX;
	_trajectory[_currentTrajectoryIndex].y = _characterPosY;
}

void TotEngine::saveGameToRegister() {
	savedGame.roomCode = _currentRoomData->code;
	savedGame.trajectoryLength = _trajectoryLength;
	savedGame.currentTrajectoryIndex = _currentTrajectoryIndex;
	savedGame.backpackObjectCode = _backpackObjectCode;
	savedGame.rightSfxVol = _sound->_rightSfxVol;
	savedGame.leftSfxVol = _sound->_leftSfxVol;
	savedGame.musicVolRight = _sound->_musicVolRight;
	savedGame.musicVolLeft = _sound->_musicVolLeft;
	savedGame.oldGridX = oldGridX;
	savedGame.oldGridY = oldGridY;
	savedGame.secAnimDepth = _secondaryAnimation.depth;
	savedGame.secAnimDir = _secondaryAnimation.dir;
	savedGame.secAnimX = _secondaryAnimation.posx;
	savedGame.secAnimY = _secondaryAnimation.posy;
	savedGame.secAnimIFrame = _iframe2;

	savedGame.currentZone = _currentZone;
	savedGame.targetZone = _targetZone;
	savedGame.oldTargetZone = _oldTargetZone;
	savedGame.inventoryPosition = _inventoryPosition;
	savedGame.actionCode = _actionCode;
	savedGame.oldActionCode = _oldActionCode;
	savedGame.steps = _trajectorySteps;
	savedGame.doorIndex = _doorIndex;
	savedGame.characterFacingDir = _charFacingDirection;
	savedGame.iframe = _iframe;
	savedGame.gamePart = _gamePart;

	savedGame.isSealRemoved = _isSealRemoved;
	savedGame.obtainedList1 = _obtainedList1;
	savedGame.obtainedList2 = _obtainedList2;
	savedGame.list1Complete = _list1Complete;
	savedGame.list2Complete = _list2Complete;
	savedGame.isVasePlaced = _isVasePlaced;
	savedGame.isScytheTaken = _isScytheTaken;
	savedGame.isTridentTaken = _isTridentTaken;
	savedGame.isPottersWheelDelivered = _isPottersWheelDelivered;
	savedGame.isMudDelivered = _isMudDelivered;
	savedGame.isGreenDevilDelivered = _isGreenDevilDelivered;
	savedGame.isRedDevilCaptured = _isRedDevilCaptured;
	savedGame.isPottersManualDelivered = _isPottersManualDelivered;
	savedGame.isCupboardOpen = _isCupboardOpen;
	savedGame.isChestOpen = _isChestOpen;
	savedGame.isTVOn = _isTVOn;
	savedGame.isTrapSet = _isTrapSet;

	for (int i = 0; i < kInventoryIconCount; i++) {
		savedGame.mobj[i].bitmapIndex = _inventory[i].bitmapIndex;
		savedGame.mobj[i].code = _inventory[i].code;
		savedGame.mobj[i].objectName = _inventory[i].objectName;
	}

	savedGame.element1 = _element1;
	savedGame.element2 = _element2;
	savedGame.characterPosX = _characterPosX;
	savedGame.characterPosY = _characterPosY;
	savedGame.xframe2 = _xframe2;
	savedGame.yframe2 = _yframe2;

	savedGame.oldInventoryObjectName = _oldInventoryObjectName;
	savedGame.objetomoinventoryObjectNamehila = _inventoryObjectName;
	savedGame.characterName = _characterName;

	for (int i = 0; i < kRoutePointCount; i++) {
		savedGame.mainRoute[i].x = _mainRoute[i].x;
		savedGame.mainRoute[i].y = _mainRoute[i].y;
	}

	for (int i = 0; i < 300; i++) {
		savedGame.trajectory[i].x = _trajectory[i].x;
		savedGame.trajectory[i].y = _trajectory[i].y;
	}

	for (int i = 0; i < kCharacterCount; i++) {
		savedGame.firstTimeTopicA[i] = _firstTimeTopicA[i];
		savedGame.firstTimeTopicB[i] = _firstTimeTopicB[i];
		savedGame.firstTimeTopicC[i] = _firstTimeTopicC[i];
		savedGame.bookTopic[i] = _bookTopic[i];
		savedGame.mintTopic[i] = _mintTopic[i];
	}
	for (int i = 0; i < 5; i++) {
		savedGame.caves[i] = _caves[i];
		savedGame.firstList[i] = _firstList[i];
		savedGame.secondList[i] = _secondList[i];
	}
	for (int i = 0; i < 4; i++) {
		savedGame.niche[0][i] = _niche[0][i];
		savedGame.niche[1][i] = _niche[1][i];
	}
}

void TotEngine::loadGame(SavedGame game) {
	freeAnimation();
	freeScreenObjects();

	_trajectoryLength = game.trajectoryLength;
	_currentTrajectoryIndex = game.currentTrajectoryIndex;
	_backpackObjectCode = game.backpackObjectCode;
	_sound->_rightSfxVol = game.rightSfxVol;
	_sound->_leftSfxVol = game.leftSfxVol;
	_sound->_musicVolRight = game.musicVolRight;
	_sound->_musicVolLeft = game.musicVolLeft;
	oldGridX = game.oldGridX;
	oldGridY = game.oldGridY;
	_secondaryAnimation.depth = game.secAnimDepth;
	_secondaryAnimation.dir = game.secAnimDir;
	_secondaryAnimation.posx = game.secAnimX;
	_secondaryAnimation.posy = game.secAnimY;
	_iframe2 = game.secAnimIFrame;
	_currentZone = game.currentZone;
	_targetZone = game.targetZone;
	_oldTargetZone = game.oldTargetZone;
	_inventoryPosition = game.inventoryPosition;
	_actionCode = game.actionCode;
	_oldActionCode = game.oldActionCode;
	_trajectorySteps = game.steps;
	_doorIndex = game.doorIndex;
	_charFacingDirection = game.characterFacingDir;
	_iframe = game.iframe;
	if (game.gamePart != _gamePart) {
		_gamePart = game.gamePart;
		for (int i = 0; i < kInventoryIconCount; i++) {
			free(_inventoryIconBitmaps[i]);
		}
		loadObjects();
	}
	_isSealRemoved = game.isSealRemoved;
	_obtainedList1 = game.obtainedList1;
	_obtainedList2 = game.obtainedList2;
	_list1Complete = game.list1Complete;
	_list2Complete = game.list2Complete;
	_isVasePlaced = game.isVasePlaced;
	_isScytheTaken = game.isScytheTaken;
	if (_cpCounter > 24)
		showError(274);
	_isTridentTaken = game.isTridentTaken;
	_isPottersWheelDelivered = game.isPottersWheelDelivered;
	_isMudDelivered = game.isMudDelivered;
	_isGreenDevilDelivered = game.isGreenDevilDelivered;
	_isRedDevilCaptured = game.isRedDevilCaptured;
	_isPottersManualDelivered = game.isPottersManualDelivered;
	_isCupboardOpen = game.isCupboardOpen;
	_isChestOpen = game.isChestOpen;
	_isTVOn = game.isTVOn;
	_isTrapSet = game.isTrapSet;
	for (int i = 0; i < kInventoryIconCount; i++) {
		_inventory[i].bitmapIndex = game.mobj[i].bitmapIndex;
		_inventory[i].code = game.mobj[i].code;
		_inventory[i].objectName = game.mobj[i].objectName;
	}
	_element1 = game.element1;
	_element2 = game.element2;
	_characterPosX = game.characterPosX;
	_characterPosY = game.characterPosY;
	_xframe2 = game.xframe2;
	_yframe2 = game.yframe2;
	_oldInventoryObjectName = game.oldInventoryObjectName;
	_inventoryObjectName = game.objetomoinventoryObjectNamehila;
	_characterName = game.characterName;
	for (int i = 0; i < kRoutePointCount; i++) {
		_mainRoute[i].x = game.mainRoute[i].x;
		_mainRoute[i].y = game.mainRoute[i].y;
	}
	for (int indiaux = 0; indiaux < 300; indiaux++) {
		_trajectory[indiaux].x = game.trajectory[indiaux].x;
		_trajectory[indiaux].y = game.trajectory[indiaux].y;
	}
	for (int i = 0; i < kCharacterCount; i++) {
		_firstTimeTopicA[i] = game.firstTimeTopicA[i];
		_firstTimeTopicB[i] = game.firstTimeTopicB[i];
		_firstTimeTopicC[i] = game.firstTimeTopicC[i];
		_bookTopic[i] = game.bookTopic[i];
		_mintTopic[i] = game.mintTopic[i];
	}
	for (int i = 0; i < 5; i++) {
		_caves[i] = game.caves[i];
		_firstList[i] = game.firstList[i];
		_secondList[i] = game.secondList[i];
	}
	for (int i = 0; i < 4; i++) {
		_niche[0][i] = game.niche[0][i];
		_niche[1][i] = game.niche[1][i];
	}

	_graphics->totalFadeOut(0);
	_screen->clear();
	_graphics->loadPaletteFromFile("DEFAULT");
	loadScreenData(game.roomCode);

	switch (_currentRoomData->code) {
	case 2: {
		if (_isTVOn)
			_sound->autoPlayVoc("PARASITO", 355778, 20129);
		else
			loadTV();
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
	} break;
	case 4: {
		_sound->loadVoc("GOTA", 140972, 1029);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
	} break;
	case 5: {
		_sound->setSfxVolume(_sound->_leftSfxVol, 0);
		_sound->autoPlayVoc("CALDERA", 6433, 15386);
	} break;
	case 6: {
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
		_sound->autoPlayVoc("CALDERA", 6433, 15386);
	} break;
	case 17: {
		if (_bookTopic[0] == true && _currentRoomData->animationFlag)
			disableSecondAnimation();
	} break;
	case 20: {
		switch (_niche[0][_niche[0][3]]) {
		case 0:
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
			break;
		case 561:
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
			break;
		case 563:
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
			break;
		case 615:
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
			break;
		}
	} break;
	case 23: {
		_sound->autoPlayVoc("Fuente", 0, 0);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
	} break;
	case 24: {
		switch (_niche[1][_niche[1][3]]) {
		case 0:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
			break;
		case 561:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
			break;
		case 615:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
			break;
		case 622:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
			break;
		case 623:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
			break;
		}
		if (_isTrapSet) {
			_currentRoomData->animationFlag = true;
			loadAnimation(_currentRoomData->animationName);
			_iframe2 = 0;
			_currentSecondaryTrajectoryIndex = 1;
			_currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x = 214 - 15;
			_currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y = 115 - 42;
			_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
			_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
			_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
			_secondaryAnimation.depth = 14;

			for (int i = 0; i < _maxXGrid; i++)
				for (int j = 0; j < _maxYGrid; j++) {
					if (_maskGridSecondaryAnim[i][j] > 0)
						_currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j] = _maskGridSecondaryAnim[i][j];
					if (_maskMouseSecondaryAnim[i][j] > 0)
						_currentRoomData->mouseGrid[_oldposx + i][_oldposy + j] = _maskMouseSecondaryAnim[i][j];
				}
		}
		assembleScreen();
	} break;
	}

	mask();
	_inventoryPosition = 0;
	drawBackpack();
	if (_isRedDevilCaptured == false && _currentRoomData->code == 24 && _isTrapSet == false)
		runaroundRed();
	_graphics->sceneTransition(false, sceneBackground);
}

TextEntry TotEngine::readVerbRegister(uint numRegister) {
	_verbFile.seek(numRegister * kVerbRegSize);
	return readVerbRegister();
}

TextEntry TotEngine::readVerbRegister() {
	TextEntry regmht;
	// Since the text is encrypted it's safer to save the size as reported by
	// the pascal string.
	byte size = _verbFile.readByte();
	_verbFile.seek(-1, SEEK_CUR);
	regmht.text = _verbFile.readPascalString(false);
	_verbFile.skip(255 - size);
	regmht.continued = _verbFile.readByte();
	regmht.response = _verbFile.readUint16LE();
	regmht.pointer = _verbFile.readSint32LE();
	return regmht;
}

void TotEngine::sayLine(
	uint textRef,
	byte textColor,
	byte shadowColor,
	uint &responseNumber,
	/** Whether the text being said is part of a conversation or just descriptions */
	bool isWithinConversation) {
	TextEntry text;

	byte insertName, charCounter, lineBreakCount, width;
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
	case 8: { // patch
		posx = 10;
		posy = 100;
		width = 50;
	} break;
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
	case 21: { // p4
		posx = 10;
		posy = 100;
		width = 50;
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

		text = readVerbRegister();

		insertName = 0;

		for (int i = 0; i < text.text.size(); i++) {
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
			} while (charCounter + 1 <= text.text.size() - width);

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
		Common::Event e;
		// Plays talk cycle if needed
		do {
			_chrono->updateChrono();
			while (g_system->getEventManager()->pollEvent(e)) {
				if (isMouseEvent(e)) {
					if (e.type == Common::EVENT_LBUTTONUP || e.type == Common::EVENT_RBUTTONUP) {
						mouseClicked = true;
					}
				}
				changeGameSpeed(e);
			}
			if (gameTick) {
				gameTick = false;
				if (gameTickHalfSpeed) {
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

void TotEngine::wcScene() {
	palette wcPalette;
	_currentZone = _currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount];
	goToObject(_currentZone, _targetZone);

	_graphics->copyPalette(g_engine->_graphics->_pal, wcPalette);
	_mouse->hide();

	_graphics->partialFadeOut(234);

	const char *const *messages = (_lang == Common::ES_ESP) ? fullScreenMessages[0] : fullScreenMessages[1];

	littText(10, 20, messages[45], 253);
	delay(1000);

	bar(10, 20, 150, 30, 0);
	delay(2000);

	littText(100, 50, messages[46], 255);
	delay(1000);

	bar(100, 50, 250, 60, 0);
	delay(2000);

	littText(30, 110, messages[47], 253);
	delay(1000);

	bar(30, 110, 210, 120, 0);
	delay(3000);

	littText(50, 90, messages[48], 248);
	delay(1000);

	_sound->playVoc("WATER", 272050, 47062);
	bar(50, 90, 200, 100, 0);
	delay(4000);

	_characterPosX = 76 - kCharacterCorrectionX;
	_characterPosY = 78 - kCharacerCorrectionY;
	_graphics->copyPalette(wcPalette, g_engine->_graphics->_pal);
	_graphics->restoreBackground();
	assembleScreen();
	_graphics->drawScreen(sceneBackground);
	_graphics->partialFadeIn(234);
	_xframe2 = 0;
	_currentTrajectoryIndex = 0;
	_trajectoryLength = 1;
	_currentZone = 8;
	_targetZone = 8;
	_trajectory[0].x = _characterPosX;
	_trajectory[0].y = _characterPosY;

	_mouse->show();
}

void TotEngine::readConversationFile(Common::String f) {
	Common::File conversationFile;
	debug("Filename = %s", f.c_str());
	if (!conversationFile.open(Common::Path(f))) {
		showError(314);
	}
	int64 fileSize = conversationFile.size();
	byte *buf = (byte *)malloc(fileSize);
	conversationFile.read(buf, fileSize);

	_conversationData = new Common::MemorySeekableReadWriteStream(buf, fileSize, DisposeAfterUse::NO);
	conversationFile.close();
}

void saveDoorMetadata(DoorRegistry doors, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeUint16LE(doors.nextScene);
	screenDataStream->writeUint16LE(doors.exitPosX);
	screenDataStream->writeUint16LE(doors.exitPosY);
	screenDataStream->writeByte(doors.openclosed);
	screenDataStream->writeByte(doors.doorcode);
}

void savePoint(Common::Point point, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeUint16LE(point.x);
	screenDataStream->writeUint16LE(point.y);
}

void saveBitmapRegister(RoomBitmapRegister bitmap, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeSint32LE(bitmap.bitmapPointer);
	screenDataStream->writeUint16LE(bitmap.bitmapSize);
	screenDataStream->writeUint16LE(bitmap.coordx);
	screenDataStream->writeUint16LE(bitmap.coordy);
	screenDataStream->writeUint16LE(bitmap.depth);
}

void saveRoomObjectList(RoomObjectListEntry objectList, Common::SeekableWriteStream *screenDataStream) {

	screenDataStream->writeUint16LE(objectList.fileIndex);
	screenDataStream->writeByte(objectList.objectName.size());
	int paddingSize = 20 - objectList.objectName.size();
	if (paddingSize < 20) {
		screenDataStream->writeString(objectList.objectName);
	}
	if (paddingSize > 0) {
		char *padding = (char *)malloc(paddingSize);
		for (int i = 0; i < paddingSize; i++) {
			padding[i] = '\0';
		}
		screenDataStream->write(padding, paddingSize);

		free(padding);
	}
}

void saveRoom(RoomFileRegister *room, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeUint16LE(room->code);
	screenDataStream->writeUint32LE(room->roomImagePointer);
	screenDataStream->writeUint16LE(room->roomImageSize);
	screenDataStream->write(room->walkAreasGrid, 40 * 28);
	screenDataStream->write(room->mouseGrid, 40 * 28);

	// read puntos
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 30; j++) {
			for (int k = 0; k < 5; k++) {
				savePoint(room->trajectories[i][j][k], screenDataStream);
			}
		}
	}

	for (int i = 0; i < 5; i++) {
		saveDoorMetadata(room->doors[i], screenDataStream);
	}
	for (int i = 0; i < 15; i++) {
		saveBitmapRegister(room->screenLayers[i], screenDataStream);
	}
	for (int i = 0; i < 51; i++) {
		saveRoomObjectList(*room->screenObjectIndex[i], screenDataStream);
	}
	screenDataStream->writeByte(room->animationFlag);

	screenDataStream->writeByte(room->animationName.size());
	screenDataStream->writeString(room->animationName);
	int paddingSize = 8 - room->animationName.size();
	if (paddingSize > 0) {
		char *padding = (char *)malloc(paddingSize);
		for (int i = 0; i < paddingSize; i++) {
			padding[i] = '\0';
		}
		// 8 max char name
		screenDataStream->write(padding, paddingSize);

		free(padding);
	}
	screenDataStream->writeByte(room->paletteAnimationFlag);
	screenDataStream->writeUint16LE(room->palettePointer);
	for (int i = 0; i < 300; i++) {
		savePoint(room->secondaryAnimTrajectory[i], screenDataStream);
	}
	screenDataStream->write(room->secondaryAnimDirections, 600);
	screenDataStream->writeUint16LE(room->secondaryTrajectoryLength);
}

void TotEngine::saveRoomData(RoomFileRegister *room, Common::SeekableWriteStream *stream) {
	_rooms->seek(room->code * kRoomRegSize, SEEK_SET);
	saveRoom(room, stream);
}

/**
 * Object files contain a single register per object, with a set of 8 flags, to mark them as used in each save.
 */
void TotEngine::initializeObjectFile() {
	Common::File objFile;
	if (!objFile.open(Common::Path("OBJETOS.DAT"))) {
		showError(261);
	}
	delete (_invItemData);
	byte *objectData = (byte *)malloc(objFile.size());
	objFile.read(objectData, objFile.size());
	_invItemData = new Common::MemorySeekableReadWriteStream(objectData, objFile.size(), DisposeAfterUse::NO);
	objFile.close();
}

void TotEngine::saveItem(ScreenObject object, Common::SeekableWriteStream *objectDataStream) {
	objectDataStream->writeUint16LE(object.code);
	objectDataStream->writeByte(object.height);

	objectDataStream->writeByte(object.name.size());
	objectDataStream->writeString(object.name);
	int paddingSize = kObjectNameLength - object.name.size();
	if (paddingSize > 0) {
		char *padding = (char *)malloc(paddingSize);
		for (int i = 0; i < paddingSize; i++) {
			padding[i] = '\0';
		}
		// 8 max char name
		objectDataStream->write(padding, paddingSize);
		free(padding);
	}

	objectDataStream->writeUint16LE(object.lookAtTextRef);
	objectDataStream->writeUint16LE(object.beforeUseTextRef);
	objectDataStream->writeUint16LE(object.afterUseTextRef);
	objectDataStream->writeUint16LE(object.pickTextRef);
	objectDataStream->writeUint16LE(object.useTextRef);

	objectDataStream->writeByte(object.speaking);
	objectDataStream->writeByte(object.openable);
	objectDataStream->writeByte(object.closeable);

	objectDataStream->write(object.used, 8);

	objectDataStream->writeByte(object.pickupable);

	objectDataStream->writeUint16LE(object.useWith);
	objectDataStream->writeUint16LE(object.replaceWith);
	objectDataStream->writeByte(object.depth);
	objectDataStream->writeUint32LE(object.bitmapPointer);
	objectDataStream->writeUint16LE(object.bitmapSize);
	objectDataStream->writeUint16LE(object.rotatingObjectAnimation);
	objectDataStream->writeUint16LE(object.rotatingObjectPalette);
	objectDataStream->writeUint16LE(object.dropOverlayX);
	objectDataStream->writeUint16LE(object.dropOverlayY);
	objectDataStream->writeUint32LE(object.dropOverlay);
	objectDataStream->writeUint16LE(object.dropOverlaySize);
	objectDataStream->writeUint16LE(object.objectIconBitmap);

	objectDataStream->writeByte(object.xgrid1);
	objectDataStream->writeByte(object.ygrid1);
	objectDataStream->writeByte(object.xgrid2);
	objectDataStream->writeByte(object.ygrid2);

	objectDataStream->write(object.walkAreasPatch, 100);
	objectDataStream->write(object.mouseGridPatch, 100);
}

void TotEngine::saveItemRegister(ScreenObject object, Common::SeekableWriteStream *stream) {
	_invItemData->seek(object.code * kItemRegSize, SEEK_SET);
	saveItem(object, stream);
}

} // End of namespace Tot
