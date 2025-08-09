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

#include "tot/offsets.h"
#include "tot/routines.h"
#include "tot/texts.h"
#include "tot/tot.h"

namespace Tot {

void drawText(uint number) {
	assignText();
	hypertext(number, 255, 0, foo, false);
	verb.close();
}

void displayLoading() {
	const char *msg = (g_engine->_lang == Common::ES_ESP) ? fullScreenMessages[0][58] : fullScreenMessages[1][58];

	setRGBPalette(255, 63, 63, 63);
	littText(121, 72, msg, 0);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	littText(120, 71, msg, 0);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	littText(119, 72, msg, 0);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	littText(120, 73, msg, 0);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	littText(120, 72, msg, 255);
	g_engine->_screen->update();

	// enforce a delay for now so it's visible
	g_system->delayMillis(200);
}


void runaroundRed() {
	const uint trajectory[91][2] = {
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
	iframe2 = 0;
	isSecondaryAnimationEnabled = true;
	do {
		g_engine->_chrono->updateChrono();
		if (timeToDraw) {
			if (secTrajIndex == secTrajLength)
				exitLoop = true;
			secTrajIndex += 1;
			if (iframe2 >= secondaryAnimationFrameCount - 1)
				iframe2 = 0;
			else
				iframe2++;
			secondaryAnimation.posx = trajectory[secTrajIndex][0] - 15;
			secondaryAnimation.posy = trajectory[secTrajIndex][1] - 42;
			if (secTrajIndex >= 0 && secTrajIndex <= 8) {
				secondaryAnimation.dir = 2;
				secondaryAnimation.depth = 1;
			} else if (secTrajIndex >= 9 && secTrajIndex <= 33) {
				secondaryAnimation.dir = 2;
				secondaryAnimation.depth = 14;
			} else if (secTrajIndex >= 34 && secTrajIndex <= 63) {
				secondaryAnimation.dir = 1;
				secondaryAnimation.depth = 14;
			} else {
				secondaryAnimation.dir = 0;
				secondaryAnimation.depth = 3;
			}

			timeToDraw2 = true;
			sprites(false);
			timeToDraw = false;
			if (palAnimStep >= 4) {
				palAnimStep = 0;
				if (isPaletteAnimEnabled > 6)
					isPaletteAnimEnabled = 0;
				else
					isPaletteAnimEnabled += 1;
				updatePalette(isPaletteAnimEnabled);
			} else
				palAnimStep += 1;
			g_engine->_screen->update();
		}
	} while (!exitLoop && !g_engine->shouldQuit());
	freeAnimation();
	restoreBackground();
	assembleScreen();
	drawScreen(sceneBackground);
}

void updateMovementGrids() {
	uint j1arm, j2arm;
	byte i1arm, i2arm;

	j1arm = (currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x / xGridCount) + 1;
	j2arm = (currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y / yGridCount) + 1;
	if ((oldposx != j1arm) || (oldposy != j2arm)) {

		for (i1arm = 0; i1arm < maxXGrid; i1arm++)
			for (i2arm = 0; i2arm < maxYGrid; i2arm++) {
				currentRoomData->walkAreasGrid[oldposx + i1arm][oldposy + i2arm] = movementGridForSecondaryAnim[i1arm][i2arm];
				currentRoomData->mouseGrid[oldposx + i1arm][oldposy + i2arm] = mouseGridForSecondaryAnim[i1arm][i2arm];
			}

		for (i1arm = 0; i1arm < maxXGrid; i1arm++)
			for (i2arm = 0; i2arm < maxYGrid; i2arm++) {
				movementGridForSecondaryAnim[i1arm][i2arm] = currentRoomData->walkAreasGrid[j1arm + i1arm][j2arm + i2arm];
				if (maskGridSecondaryAnim[i1arm][i2arm] > 0)
					currentRoomData->walkAreasGrid[j1arm + i1arm][j2arm + i2arm] = maskGridSecondaryAnim[i1arm][i2arm];

				mouseGridForSecondaryAnim[i1arm][i2arm] = currentRoomData->mouseGrid[j1arm + i1arm][j2arm + i2arm];
				if (maskMouseSecondaryAnim[i1arm][i2arm] > 0)
					currentRoomData->mouseGrid[j1arm + i1arm][j2arm + i2arm] = maskMouseSecondaryAnim[i1arm][i2arm];
			}

		oldposx = j1arm;
		oldposy = j2arm;
		oldGridX = 0;
		oldGridY = 0;
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
	restoreBackground();

	posabs = 4 + dirtyMainSpriteY * 320 + dirtyMainSpriteX;
	uint16 w, h;
	w = READ_LE_UINT16(characterDirtyRect);
	h = READ_LE_UINT16(characterDirtyRect + 2);
	w++;
	h++;

	dirtyMainSpriteX2 = dirtyMainSpriteX + w;
	dirtyMainSpriteY2 = dirtyMainSpriteY + h;

	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			int pos = posabs + j * 320 + i;
			int destPos = 4 + (j * w + i);
			characterDirtyRect[destPos] = sceneBackground[pos];
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

	uint16 wBg = READ_LE_UINT16(characterDirtyRect) + 1;
	uint16 hBg = READ_LE_UINT16(characterDirtyRect + 2) + 1;

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
				characterDirtyRect[bgOffset] = img[imgOffset];
			}
		}
	}
}

static void overlayLayers() {
	if (screenLayers[curDepth] != NULL) {
		if (
			(depthMap[curDepth].posx <= dirtyMainSpriteX2) &&
			(depthMap[curDepth].posx2 > dirtyMainSpriteX) &&
			(depthMap[curDepth].posy < dirtyMainSpriteY2) &&
			(depthMap[curDepth].posy2 > dirtyMainSpriteY)) {
			assembleImage(screenLayers[curDepth], depthMap[curDepth].posx, depthMap[curDepth].posy);
		}
	}
}

void drawMainCharacter() {

	bool debug = false;
	if (debug) {
		screenTransition(13, false, sceneBackground);
	}

	uint16 tempW;
	uint16 tempH;
	tempW = READ_LE_UINT16(curCharacterAnimationFrame);
	tempH = READ_LE_UINT16(curCharacterAnimationFrame + 2);
	tempW += 6;
	tempH += 6;

	characterDirtyRect = (byte *)malloc((tempW + 1) * (tempH + 1) + 4);

	WRITE_LE_UINT16(characterDirtyRect, tempW);
	WRITE_LE_UINT16(characterDirtyRect + 2, tempH);

	assembleBackground();
	curDepth = 0;
	while (curDepth != depthLevelCount) {
		overlayLayers();
		if (mainCharAnimation.depth == curDepth)
			assembleImage(curCharacterAnimationFrame, characterPosX, characterPosY);
		curDepth += 1;
	}

	g_engine->_graphics->putImg(dirtyMainSpriteX, dirtyMainSpriteY, characterDirtyRect);

	if (debug) {
		// draw background dirty area
		drawRect(2, dirtyMainSpriteX, dirtyMainSpriteY, dirtyMainSpriteX + tempW, dirtyMainSpriteY + tempH);
		drawPos(xframe2, yframe2, 218);
	}
	free(characterDirtyRect);
}

void sprites(bool drawMainCharachter) {
	// grabs the current frame from the walk cycle
	curCharacterAnimationFrame = mainCharAnimation.bitmap[charFacingDirection][iframe];

	dirtyMainSpriteX = characterPosX - 3;
	dirtyMainSpriteY = characterPosY - 3;
	if (isSecondaryAnimationEnabled) {
		if (currentRoomData->secondaryTrajectoryLength > 1) {
			updateMovementGrids();
		}
		if (timeToDraw2) {
			if (isPeterCoughing && !g_engine->_sound->isVocPlaying()) {
				iframe2 = 0;
			}
			curSecondaryAnimationFrame = secondaryAnimation.bitmap[secondaryAnimation.dir][iframe2];
		}
		uint16 curCharFrameW = READ_LE_UINT16(curCharacterAnimationFrame);
		uint16 curCharFrameH = READ_LE_UINT16(curCharacterAnimationFrame + 2);

		uint16 secAnimW = READ_LE_UINT16(curSecondaryAnimationFrame);
		uint16 secAnimH = READ_LE_UINT16(curSecondaryAnimationFrame + 2);

		if (
			((secondaryAnimation.posx < (characterPosX + curCharFrameW) + 4) &&
			 ((secondaryAnimation.posx + secAnimW + 1) > dirtyMainSpriteX) &&
			 (secondaryAnimation.posy < (characterPosY + curCharFrameH + 4))) &&
			((secondaryAnimation.posy + secAnimH + 1) > dirtyMainSpriteY)) { // Character is in the area of the animation

			if (secondaryAnimation.posx < characterPosX) {
				dirtyMainSpriteX = secondaryAnimation.posx - 3;
			}
			if (secondaryAnimation.posy < characterPosY) {
				dirtyMainSpriteY = secondaryAnimation.posy - 3;
			}

			uint16 patchW = secAnimW + curCharFrameW + 6;

			uint16 patchH;
			if ((curCharFrameH + characterPosY) > (secondaryAnimation.posy + secAnimH)) {
				patchH = curCharFrameH + 6 + abs(characterPosY - secondaryAnimation.posy);
			} else {
				patchH = secAnimH + 6 + abs(characterPosY - secondaryAnimation.posy);
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

			assembleBackground(); // {Montar el Sprite Total}
			curDepth = 0;
			while (curDepth != depthLevelCount) {
				overlayLayers();
				if (secondaryAnimation.depth == curDepth)
					assembleImage(curSecondaryAnimationFrame, secondaryAnimation.posx, secondaryAnimation.posy);
				if (mainCharAnimation.depth == curDepth)
					assembleImage(curCharacterAnimationFrame, characterPosX, characterPosY);
				curDepth += 1;
			}
			g_engine->_graphics->putImg(dirtyMainSpriteX, dirtyMainSpriteY, characterDirtyRect);
		} else { // character and animation are in different parts of the screen

			if (drawMainCharachter) {
				drawMainCharacter();
			}

			dirtyMainSpriteX = secondaryAnimation.posx - 3;
			dirtyMainSpriteY = secondaryAnimation.posy - 3;

			secAnimW = READ_LE_UINT16(curSecondaryAnimationFrame) + 6;
			secAnimH = READ_LE_UINT16(curSecondaryAnimationFrame + 2) + 6;

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
			while (curDepth != depthLevelCount) {
				overlayLayers();
				if (secondaryAnimation.depth == curDepth)
					assembleImage(curSecondaryAnimationFrame, secondaryAnimation.posx, secondaryAnimation.posy);
				curDepth += 1;
			}
			g_engine->_graphics->putImg(dirtyMainSpriteX, dirtyMainSpriteY, characterDirtyRect);
		}
	} else if (drawMainCharachter) {
		drawMainCharacter();
	}
}

void adjustKey() {
	iframe = 0;
	calculateTrajectory(129, 13);
	charFacingDirection = 0;
	do {
		characterPosX = trajectory[currentTrajectoryIndex].x;
		characterPosY = trajectory[currentTrajectoryIndex].y;
		iframe++;
		currentTrajectoryIndex += 1;
		emptyLoop();
		timeToDraw = false;
		if (palAnimStep >= 4) {
			palAnimStep = 0;
			if (isPaletteAnimEnabled > 6)
				isPaletteAnimEnabled = 0;
			else
				isPaletteAnimEnabled += 1;
			updatePalette(isPaletteAnimEnabled);
		} else
			palAnimStep += 1;
		sprites(true);
	} while (currentTrajectoryIndex != trajectoryLength);
	charFacingDirection = 3;
	emptyLoop();
	timeToDraw = false;
	sprites(true);
}

void adjustKey2() {
	iframe = 0;
	charFacingDirection = 2;
	trajectoryLength = 1;
	do {
		characterPosX = trajectory[currentTrajectoryIndex].x;
		characterPosY = trajectory[currentTrajectoryIndex].y;
		iframe++;
		currentTrajectoryIndex -= 1;
		emptyLoop();
		timeToDraw = false;
		if (palAnimStep >= 4) {
			palAnimStep = 0;
			if (isPaletteAnimEnabled > 6)
				isPaletteAnimEnabled = 0;
			else
				isPaletteAnimEnabled += 1;
			updatePalette(isPaletteAnimEnabled);
		} else
			palAnimStep += 1;
		sprites(true);
	} while (currentTrajectoryIndex != 0);
	emptyLoop();
	timeToDraw = false;
	sprites(true);
}

void animatedSequence(uint numSequence) {
	Common::File animationFile;
	uint repIndex, animIndex;
	uint16 animFrameSize;
	uint animX, animY;
	int tmpCharacterPosX;
	byte secFrameCount;
	byte *animptr;

	switch (numSequence) {
	case 1: {
		tmpCharacterPosX = characterPosX;
		characterPosX = 3;
		if (!animationFile.open("POZO01.DAT")) {
			showError(277);
		}
		animX = 127;
		animY = 70;
		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(animFrameSize);
		for (uint loopIdx = 1; loopIdx <= 3; repIndex++) {
			g_engine->_sound->playVoc("POZO", 180395, 6034);
			animIndex = 0;
			do {
				emptyLoop();
				timeToDraw = false;
				if (timeToDraw2) {
					if (currentSecondaryTrajectoryIndex >= currentRoomData->secondaryTrajectoryLength)
						currentSecondaryTrajectoryIndex = 1;
					else
						currentSecondaryTrajectoryIndex += 1;
					secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
					secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
					secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					sprites(false);
					animationFile.read(animptr, animFrameSize);
					g_engine->_graphics->putImg(animX, animY, animptr);
					animIndex += 1;
				}
			} while (animIndex != secFrameCount && !g_engine->shouldQuit());
			animationFile.seek(4);
		}
		free(animptr);
		animationFile.close();
		characterPosX = tmpCharacterPosX;
	} break;
	case 2: {
		if (!animationFile.open("POZOATR.DAT")) {
			showError(277);
		}
		animX = 127;
		tmpCharacterPosX = characterPosX;
		characterPosX = 3;
		animY = 70;

		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(animFrameSize);
		for (repIndex = 1; repIndex <= 3; repIndex++) {
			g_engine->_sound->playVoc("POZO", 180395, 6034);
			animIndex = 0;
			do {
				emptyLoop();
				timeToDraw = false;
				if (timeToDraw2) {
					if (currentSecondaryTrajectoryIndex >= currentRoomData->secondaryTrajectoryLength)
						currentSecondaryTrajectoryIndex = 1;
					else
						currentSecondaryTrajectoryIndex += 1;
					secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
					secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
					secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					sprites(false);
					animationFile.read(animptr, animFrameSize);
					g_engine->_graphics->putImg(animX, animY, animptr);
					animIndex += 1;
				}
			} while (animIndex != secFrameCount && !g_engine->shouldQuit());
			animationFile.seek(4);
		}
		g_engine->_sound->stopVoc();
		free(animptr);
		animationFile.close();
		characterPosX = tmpCharacterPosX;
	} break;
	case 3: {
		tmpCharacterPosX = characterPosX;
		characterPosX = 3;
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
			timeToDraw = false;
			if (timeToDraw2) {
				if (currentSecondaryTrajectoryIndex >= currentRoomData->secondaryTrajectoryLength)
					currentSecondaryTrajectoryIndex = 1;
				else
					currentSecondaryTrajectoryIndex += 1;
				secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
				secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
				secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
				if (iframe2 >= secondaryAnimationFrameCount - 1)
					iframe2 = 0;
				else
					iframe2++;
				sprites(false);
				animationFile.read(animptr, animFrameSize);
				g_engine->_graphics->putImg(animX, animY, animptr);
				animIndex += 1;
			}
		} while (animIndex != secFrameCount && !g_engine->shouldQuit());
		free(animptr);
		animationFile.close();
		iframe = 0;
		charFacingDirection = 2;
		emptyLoop();
		timeToDraw = false;
		characterPosX = tmpCharacterPosX;
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
			timeToDraw = false;
			if (palAnimStep >= 4) {
				palAnimStep = 0;
				if (isPaletteAnimEnabled > 6)
					isPaletteAnimEnabled = 0;
				else
					isPaletteAnimEnabled += 1;
				updatePalette(isPaletteAnimEnabled);
			} else
				palAnimStep += 1;
			g_engine->_graphics->putShape(animX, animY, animptr);
		}
		screenLayers[regobj.depth - 1] = NULL;
		restoreBackground();
		animIndex = mainCharAnimation.depth;
		mainCharAnimation.depth = 30;
		screenLayers[13] = animptr;
		depthMap[13].posx = animX;
		depthMap[13].posy = animY;
		assembleScreen();
		drawScreen(sceneBackground);
		screenLayers[13] = NULL;
		mainCharAnimation.depth = animIndex;
		drawBackpack();
		for (animIndex = 32; animIndex <= secFrameCount; animIndex++) {
			animationFile.read(animptr, animFrameSize);
			emptyLoop();
			timeToDraw = false;
			if (palAnimStep >= 4) {
				palAnimStep = 0;
				if (isPaletteAnimEnabled > 6)
					isPaletteAnimEnabled = 0;
				else
					isPaletteAnimEnabled += 1;
				updatePalette(isPaletteAnimEnabled);
			} else
				palAnimStep += 1;
			g_engine->_graphics->putImg(animX, animY, animptr);
			if (g_engine->shouldQuit()) {
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
			timeToDraw = false;
			g_engine->_graphics->putShape(animX, animY, animptr);
			if (g_engine->shouldQuit()) {
				break;
			}
		}
		animIndex = mainCharAnimation.depth;
		mainCharAnimation.depth = 30;
		screenLayers[12] = animptr;
		depthMap[12].posx = animX;
		depthMap[12].posy = animY;
		disableSecondAnimation();
		screenLayers[12] = NULL;
		mainCharAnimation.depth = animIndex;
		drawScreen(sceneBackground);
		for (animIndex = 9; animIndex <= secFrameCount; animIndex++) {
			animationFile.read(animptr, animFrameSize);
			emptyLoop();
			timeToDraw = false;
			emptyLoop();
			timeToDraw = false;
			g_engine->_graphics->putShape(animX, animY, animptr);
			if (g_engine->shouldQuit()) {
				break;
			}
		}
		free(animptr);
		animationFile.close();
		iframe = 0;
		charFacingDirection = 2;
		emptyLoop();
		timeToDraw = false;
		sprites(true);
	} break;
	case 6: {
		currentRoomData->animationFlag = false;
		if (!animationFile.open("AZCCOG.DAT")) {
			showError(277);
		}
		animFrameSize = animationFile.readUint16LE();
		secFrameCount = animationFile.readByte();
		animationFile.readByte();
		screenLayers[6] = (byte *)malloc(animFrameSize);
		depthMap[6].posx = secondaryAnimation.posx + 5;
		depthMap[6].posy = secondaryAnimation.posy - 6;
		animIndex = 0;
		do {
			emptyLoop();
			timeToDraw = false;
			if (palAnimStep >= 4) {
				palAnimStep = 0;
				if (isPaletteAnimEnabled > 6)
					isPaletteAnimEnabled = 0;
				else
					isPaletteAnimEnabled += 1;
				updatePalette(isPaletteAnimEnabled);
			} else
				palAnimStep += 1;
			if (timeToDraw2) {
				animationFile.read(screenLayers[6], animFrameSize);
				Common::copy(screenLayers[6], screenLayers[6] + animFrameSize, sceneBackground + 44900);
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animIndex += 1;
				if (animIndex == 8)
					g_engine->_sound->playVoc("PUFF", 191183, 18001);
			}
		} while (animIndex != secFrameCount && !g_engine->shouldQuit());
		animationFile.close();
		g_engine->_sound->stopVoc();
		screenLayers[6] = NULL;
		currentRoomData->animationFlag = true;
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

RoomFileRegister *readScreenDataFile(Common::SeekableReadStream *screenDataFile) {
	RoomFileRegister *screenData = new RoomFileRegister();
	screenData->code = screenDataFile->readUint16LE();
	screenData->roomImagePointer = screenDataFile->readUint32LE();
	screenData->roomImageSize = screenDataFile->readUint16LE();
	screenDataFile->read(screenData->walkAreasGrid, 40 * 28);
	screenDataFile->read(screenData->mouseGrid, 40 * 28);
	// read puntos
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

void loadScreenData(uint screenNumber) {
	debug("Opening screen %d", screenNumber);
	currentRoomNumber = screenNumber;

	rooms->seek(screenNumber * roomRegSize, SEEK_SET);
	clearScreenData();
	currentRoomData = readScreenDataFile(rooms);
	loadScreen();
	for (int i = 0; i < 15; i++) {
		RoomBitmapRegister &bitmap = currentRoomData->screenLayers[i];
		if (bitmap.bitmapSize > 0) {
			loadItem(bitmap.coordx, bitmap.coordy, bitmap.bitmapSize, bitmap.bitmapPointer, bitmap.depth);
		}
	}
	if (currentRoomData->animationFlag && currentRoomData->code != 24) {
		loadAnimation(currentRoomData->animationName);
		iframe2 = 0;
		currentSecondaryTrajectoryIndex = 1;
		secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
		secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
		secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
		if (currentRoomData->animationName == "FUENTE01")
			secondaryAnimation.depth = 0;
		else {
			updateSecondaryAnimationDepth();
		}
		for (int i = 0; i < maxXGrid; i++)
			for (int j = 0; j < maxYGrid; j++) {
				if (maskGridSecondaryAnim[i][j] > 0)
					currentRoomData->walkAreasGrid[oldposx + i][oldposy + j] = maskGridSecondaryAnim[i][j];
				if (maskMouseSecondaryAnim[i][j] > 0)
					currentRoomData->mouseGrid[oldposx + i][oldposy + j] = maskMouseSecondaryAnim[i][j];
			}
	} else
		currentRoomData->animationFlag = false;

	updateMainCharacterDepth();
	assembleScreen();
	isPaletteAnimEnabled = 0;
}

/**
 * Calculates a trajectory between the current position (xframe, yframe)
 * and the target position (finalx, finaly) using bresenham's algorithm
 */
void calculateTrajectory(uint finalX, uint finalY) {

	int deltaX = finalX - characterPosX;
	int deltaY = finalY - characterPosY;

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
	uint x = characterPosX, y = characterPosY;
	int steps = deltaX + 1; // number of steps

	currentTrajectoryIndex = 0;
	trajectoryLength = 0;
	trajectory[trajectoryLength].x = x;
	trajectory[trajectoryLength].y = y;
	trajectoryLength++;
	while (--steps) {
		if (dec >= 0) {
			x += incrXDiag;
			y += incrYDiag;
			dec += slope;
		} else {
			x += straightXIncrease;
			y += straightYIncrease;
			dec += straightAmount;
		}
		trajectory[trajectoryLength].x = x;
		trajectory[trajectoryLength].y = y;
		trajectoryLength++;
	}

	// Ensure last value is also final position to account for overflow of the route
	trajectory[trajectoryLength].x = finalX;
	trajectory[trajectoryLength].y = finalY;
	trajectory[trajectoryLength + 1].x = finalX;
	trajectory[trajectoryLength + 1].y = finalY;

	// longtray--; // decrement because of 1-starting arrays
}

void lookAtObject(byte objectCode) {
	byte yaux;

	Common::String description;
	bool foobar = false;
	TextEntry textRef;
	palette secPalette;

	cpCounter2 = cpCounter;
	g_engine->_mouseManager->hide();
	copyPalette(pal, secPalette);
	readItemRegister(mobj[objectCode].code);
	g_engine->_graphics->getImg(0, 0, 319, 139, sceneBackground);
	partialFadeOut(234);
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

	if (regobj.used[0] != 9) {
		if (regobj.beforeUseTextRef != 0) {
			assignText();
			textRef = readVerbRegister(regobj.beforeUseTextRef);
			description = textRef.text;
			for (yaux = 0; yaux < textRef.text.size(); yaux++)
				description.setChar(decryptionKey[yaux] ^ textRef.text[yaux], yaux);
			hyperText(description, 60, 15, 33, 255, 0);
			verb.close();
		} else {
			description = mobj[objectCode].objectName;
			hyperText(description, 60, 15, 33, 255, 0);
		}
	} else {

		if (regobj.afterUseTextRef != 0) {
			assignText();
			textRef = readVerbRegister(regobj.afterUseTextRef);
			description = textRef.text;
			for (yaux = 0; yaux < textRef.text.size(); yaux++)
				description.setChar(decryptionKey[yaux] ^ textRef.text[yaux], yaux);
			hyperText(description, 60, 15, 33, 255, 0);
			verb.close();
		} else {
			description = mobj[objectCode].objectName;
			hyperText(description, 60, 15, 33, 255, 0);
		}
	}

	drawFlc(125, 70, regobj.rotatingObjectAnimation, 60000, 9, 0, false, true, true, foobar);

	screenTransition(3, true, NULL);
	partialFadeOut(234);
	assembleScreen();
	drawScreen(sceneBackground);
	copyPalette(secPalette, pal);
	partialFadeIn(234);
	g_engine->_mouseManager->show();
}

void useInventoryObjectWithInventoryObject(uint objectCode1, uint objectCode2) {
	byte invIndex, indobj1, indobj2;

	debug("Reading item register %d", objectCode1);
	readItemRegister(invItemData, objectCode1, regobj);
	// verifyCopyProtection2();
	if (regobj.used[0] != 1 || regobj.useWith != objectCode2) {
		drawText(Random(11) + 1022);
		return;
	}

	invIndex = 0;
	while (mobj[invIndex].code != objectCode1) {
		invIndex += 1;
	}
	indobj1 = invIndex;

	invIndex = 0;
	while (mobj[invIndex].code != objectCode2) {
		invIndex += 1;
	}

	indobj2 = invIndex;
	foo = regobj.useTextRef;

	if (regobj.replaceWith == 0) {
		readItemRegister(invItemData, objectCode1, regobj);
		regobj.used[0] = 9;
		saveItemRegister(regobj, invItemData);

		readItemRegister(invItemData, objectCode2, regobj);
		regobj.used[0] = 9;
		saveItemRegister(regobj, invItemData);

	} else {
		readItemRegister(invItemData, regobj.replaceWith, regobj);
		mobj[indobj1].bitmapIndex = regobj.objectIconBitmap;
		mobj[indobj1].code = regobj.code;
		mobj[indobj1].objectName = regobj.name;
		for (indobj1 = indobj2; indobj1 < (inventoryIconCount - 1); indobj1++) {
			mobj[indobj1].bitmapIndex = mobj[indobj1 + 1].bitmapIndex;
			mobj[indobj1].code = mobj[indobj1 + 1].code;
			mobj[indobj1].objectName = mobj[indobj1 + 1].objectName;
		}
		g_engine->_mouseManager->hide();
		drawBackpack();
		g_engine->_mouseManager->show();
	}
	if (foo > 0)
		drawText(foo);
}

void calculateRoute(byte zone1, byte zone2, bool extraCorrection, bool barredZone) {
	// Resets the entire route
	for (steps = 0; steps < 7; steps++) {
		mainRoute[steps].x = 0;
		mainRoute[steps].y = 0;
	}
	steps = 0;

	// Starts first element of route with current position
	mainRoute[0].x = characterPosX;
	mainRoute[0].y = characterPosY;
	Common::Point point;
	do {
		steps += 1;
		point = currentRoomData->trajectories[zone1 - 1][zone2 - 1][steps - 1];

		if (point.x < (characterCorrectionX + 3))
			mainRoute[steps].x = 3;
		else
			mainRoute[steps].x = point.x - characterCorrectionX;
		if (point.y < (characerCorrectionY + 3))
			mainRoute[steps].y = 3;
		else
			mainRoute[steps].y = point.y - characerCorrectionY;

	} while (point.y != 9999 && steps != 5);

	if (zone2 < 10) {
		if (point.y == 9999) {
			mainRoute[steps].x = xframe2 - characterCorrectionX;
			mainRoute[steps].y = yframe2 - characerCorrectionY;
		} else {
			mainRoute[6].x = xframe2 - characterCorrectionX;
			mainRoute[6].y = yframe2 - characerCorrectionY;
			steps = 6;
		}
	} else {
		if ((mainRoute[steps].y + characerCorrectionY) == 9999) {
			steps -= 1;
		}
		if (extraCorrection) {
			switch (currentRoomData->code) {
			case 5:
				if (zone2 == 27)
					steps += 1;
				break;
			case 6:
				if (zone2 == 21)
					steps += 1;
				break;
			}
		}
	}
	if (extraCorrection && barredZone) {
		steps -= 1;
	}

	// Sets xframe2 now to be the number of steps in the route
	xframe2 = steps;
	// Sets yframe2 to now be current step
	yframe2 = 1;
	trajectoryLength = 10;
	currentTrajectoryIndex = 30;
}

void goToObject(byte zone1, byte zone2) {
	bool barredZone = false;

	cpCounter2 = cpCounter;

	for (int i = 0; i < 5; i++) {
		if (currentRoomData->doors[i].doorcode == zone2) {
			barredZone = true;
			break;
		}
	}

	if (currentRoomData->code == 21 && currentRoomData->animationFlag) {
		if ((zone2 >= 1 && zone2 <= 5) ||
			(zone2 >= 9 && zone2 <= 13) ||
			(zone2 >= 18 && zone2 <= 21) ||
			zone2 == 24 || zone2 == 25) {

			targetZone = 7;
			mouseClickX = 232;
			mouseClickY = 75;
			zone2 = 7;
		}
		if (zone2 == 24) {
			barredZone = false;
		}
	}
	if (zone1 < 10) {
		xframe2 = mouseClickX + 7;
		yframe2 = mouseClickY + 7;

		g_engine->_mouseManager->hide();
		calculateRoute(zone1, zone2, true, barredZone);
		Common::Event e;
		do {
			g_engine->_chrono->updateChrono();
			while (g_system->getEventManager()->pollEvent(e)) {
				changeGameSpeed(e);
			}
			advanceAnimations(barredZone, false);
			g_engine->_screen->update();
			g_system->delayMillis(10);
		} while (xframe2 != 0);

		iframe = 0;
		sprites(true);
		g_engine->_mouseManager->show();
	}
}

void updateSecondaryAnimationDepth() {
	uint animationPos = secondaryAnimation.posy + secondaryAnimHeight - 1;
	if (animationPos >= 0 && animationPos <= 56) {
		secondaryAnimation.depth = 0;
	} else if (animationPos >= 57 && animationPos <= 66) {
		secondaryAnimation.depth = 1;
	} else if (animationPos >= 65 && animationPos <= 74) {
		secondaryAnimation.depth = 2;
	} else if (animationPos >= 73 && animationPos <= 82) {
		secondaryAnimation.depth = 3;
	} else if (animationPos >= 81 && animationPos <= 90) {
		secondaryAnimation.depth = 4;
	} else if (animationPos >= 89 && animationPos <= 98) {
		secondaryAnimation.depth = 5;
	} else if (animationPos >= 97 && animationPos <= 106) {
		secondaryAnimation.depth = 6;
	} else if (animationPos >= 105 && animationPos <= 114) {
		secondaryAnimation.depth = 7;
	} else if (animationPos >= 113 && animationPos <= 122) {
		secondaryAnimation.depth = 8;
	} else if (animationPos >= 121 && animationPos <= 140) {
		secondaryAnimation.depth = 9;
	}
}

void updateMainCharacterDepth() {
	if (characterPosY >= 0 && characterPosY <= 7) {
		mainCharAnimation.depth = 0;
	} else if (characterPosY >= 8 && characterPosY <= 17) {
		mainCharAnimation.depth = 1;
	} else if (characterPosY >= 18 && characterPosY <= 25) {
		mainCharAnimation.depth = 2;
	} else if (characterPosY >= 26 && characterPosY <= 33) {
		mainCharAnimation.depth = 3;
	} else if (characterPosY >= 34 && characterPosY <= 41) {
		mainCharAnimation.depth = 4;
	} else if (characterPosY >= 42 && characterPosY <= 49) {
		mainCharAnimation.depth = 5;
	} else if (characterPosY >= 50 && characterPosY <= 57) {
		mainCharAnimation.depth = 6;
	} else if (characterPosY >= 58 && characterPosY <= 65) {
		mainCharAnimation.depth = 7;
	} else if (characterPosY >= 66 && characterPosY <= 73) {
		mainCharAnimation.depth = 8;
	} else if (characterPosY >= 74 && characterPosY <= 139) {
		mainCharAnimation.depth = 9;
	}
}

void advanceAnimations(bool barredZone, bool animateMouse) {
	if (timeToDraw) {

		if (currentRoomData->animationFlag && timeToDraw2) {
			if (isPeterCoughing && (Random(100) == 1) && !g_engine->_sound->isVocPlaying() && mintTopic[0] == false) {
				debug("Playing tos");
				g_engine->_sound->playVoc("TOS", 258006, 14044);
			}
			if (currentSecondaryTrajectoryIndex >= currentRoomData->secondaryTrajectoryLength)
				currentSecondaryTrajectoryIndex = 1;
			else
				currentSecondaryTrajectoryIndex += 1;
			if (iframe2 >= secondaryAnimationFrameCount - 1)
				iframe2 = 0;
			else
				iframe2++;
			secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
			secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
			secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
			switch (currentRoomData->code) {
			case 23:
				secondaryAnimation.depth = 0;
				break;
			case 24:
				secondaryAnimation.depth = 14;
				break;
			default:
				updateSecondaryAnimationDepth();
			}
		}

		// This means character needs to walk
		if (xframe2 > 0) {
			if (charFacingDirection == 1 || charFacingDirection == 3) {
				currentTrajectoryIndex += 1;
			}
			currentTrajectoryIndex += 1;
			// yframe2, xframe2 now represent max steps and current step
			if (yframe2 <= xframe2) {
				if (currentTrajectoryIndex > trajectoryLength) {
					element1 = mainRoute[yframe2].x - mainRoute[yframe2 - 1].x;
					element2 = mainRoute[yframe2].y - mainRoute[yframe2 - 1].y;

					if ((abs(element1) > 2) || (abs(element2) > 2)) {
						if (abs(element2) > (abs(element1) + 5)) {
							if (element2 < 0)
								charFacingDirection = 0;
							else
								charFacingDirection = 2;
						} else {
							if (element1 > 0)
								charFacingDirection = 1;
							else
								charFacingDirection = 3;
						}
					}
					yframe2 += 1;
					calculateTrajectory(mainRoute[yframe2 - 1].x, mainRoute[yframe2 - 1].y);
				}
			} else if (currentTrajectoryIndex >= trajectoryLength) {
				xframe2 = 0;
				if (!roomChange) {
					element1 = (mouseX + 7) - (characterPosX + characterCorrectionX);
					element2 = (mouseY + 7) - (characterPosY + characerCorrectionY);
					if (abs(element2) > (abs(element1) + 20)) {
						if (element2 < 0)
							charFacingDirection = 0;
						else
							charFacingDirection = 2;
					} else {
						if (element1 > 0)
							charFacingDirection = 1;
						else
							charFacingDirection = 3;
					}
				}
			}
			iframe++;
			if (iframe > walkFrameCount)
				iframe = 0;
		} else {
			iframe = 0;
			if (barredZone == false && !roomChange) {
				element1 = (mouseX + 7) - (characterPosX + characterCorrectionX);
				element2 = (mouseY + 7) - (characterPosY + characerCorrectionY);
				if (abs(element2) > (abs(element1) + 20)) {
					if (element2 < 0)
						charFacingDirection = 0;
					else
						charFacingDirection = 2;
				} else {
					if (element1 > 0)
						charFacingDirection = 1;
					else
						charFacingDirection = 3;
				}
			}
		}
		characterPosX = trajectory[currentTrajectoryIndex].x;
		characterPosY = trajectory[currentTrajectoryIndex].y;

		updateMainCharacterDepth();

		if (isDrawingEnabled) {
			sprites(true);
		}
		timeToDraw = false;
		if (currentRoomData->paletteAnimationFlag && palAnimStep >= 4) {
			palAnimStep = 0;
			if (isPaletteAnimEnabled > 6)
				isPaletteAnimEnabled = 0;
			else
				isPaletteAnimEnabled += 1;
			if (currentRoomData->code == 4 && isPaletteAnimEnabled == 4)
				g_engine->_sound->playVoc();
			updatePalette(isPaletteAnimEnabled);
		} else {
			palAnimStep += 1;
		}
	}
}

void actionLineText(Common::String actionLine) {
	euroText(160, 144, actionLine, 255, Graphics::kTextAlignCenter);
}

void animateGive(uint dir, uint height) {
	charFacingDirection = dir;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		timeToDraw = false;
		// Must add 1 to i because the original game uses 1-based indices
		iframe = 15 + 6 + 5 + height * 10 - (i + 1);

		if (currentRoomData->paletteAnimationFlag && palAnimStep >= 4) {
			palAnimStep = 0;
			if (isPaletteAnimEnabled > 6)
				isPaletteAnimEnabled = 0;
			else
				isPaletteAnimEnabled += 1;
			updatePalette(isPaletteAnimEnabled);
		} else
			palAnimStep += 1;
		sprites(true);
	}
	iframe = 0;
}

// Lean over to pick
void animatePickup1(uint dir, uint height) {
	charFacingDirection = dir;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		timeToDraw = false;
		iframe = 15 + height * 10 + (i + 1);

		if (currentRoomData->paletteAnimationFlag && palAnimStep >= 4) {
			palAnimStep = 0;
			if (isPaletteAnimEnabled > 6)
				isPaletteAnimEnabled = 0;
			else
				isPaletteAnimEnabled += 1;
			updatePalette(isPaletteAnimEnabled);
		} else
			palAnimStep += 1;
		sprites(true);
		g_engine->_screen->update();
	}
}

// Lean back after pick
void animatePickup2(uint dir, uint height) {
	charFacingDirection = dir;

	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		timeToDraw = false;

		iframe = 15 + 5 + height * 10 + (i + 1);

		if (currentRoomData->paletteAnimationFlag && palAnimStep >= 4) {
			palAnimStep = 0;
			if (isPaletteAnimEnabled > 6)
				isPaletteAnimEnabled = 0;
			else
				isPaletteAnimEnabled += 1;
			updatePalette(isPaletteAnimEnabled);
		} else
			palAnimStep += 1;
		sprites(true);
		g_engine->_screen->update();
	}
	emptyLoop();
	timeToDraw = false;
	sprites(true);
	iframe = 0;
}

void animateOpen2(uint dir, uint height) {
	charFacingDirection = dir;
	cpCounter = cpCounter2;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		timeToDraw = false;
		iframe = 15 + 6 + height * 10 - (i + 1);

		if (currentRoomData->paletteAnimationFlag && palAnimStep >= 4) {
			palAnimStep = 0;
			if (isPaletteAnimEnabled > 6)
				isPaletteAnimEnabled = 0;
			else
				isPaletteAnimEnabled += 1;
			updatePalette(isPaletteAnimEnabled);
		} else
			palAnimStep += 1;
		sprites(true);
	}
	emptyLoop();
	timeToDraw = false;
	sprites(true);
	iframe = 0;
}

void animateBat() {
	const uint trajectory[91][2] = {
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
	if (currentRoomData->animationFlag) {
		curAnimIFrame = iframe2;
		curAnimX = secondaryAnimation.posx;
		curAnimY = secondaryAnimation.posy;
		currAnimWidth = secondaryAnimWidth;
		curAnimHeight = secondaryAnimHeight;
		curAnimDepth = secondaryAnimation.depth;
		curAnimDir = secondaryAnimation.dir;
		freeAnimation();
	}
	loadBat();
	g_engine->_sound->stopVoc();
	g_engine->_sound->playVoc("MURCIEL", 160848, 4474);
	curAnimIdx = 0;
	curAnimLength = 87;
	iframe2 = 0;
	secondaryAnimation.depth = 14;
	do {
		g_engine->_chrono->updateChrono();
		if (timeToDraw) {
			if (curAnimIdx == curAnimLength)
				loopBreak = true;
			curAnimIdx += 1;
			if (iframe2 >= secondaryAnimationFrameCount - 1)
				iframe2 = 0;
			else
				iframe2++;

			secondaryAnimation.posx = trajectory[curAnimIdx][0] - 20;
			secondaryAnimation.posy = trajectory[curAnimIdx][1];
			secondaryAnimation.dir = 0;
			sprites(true);
			timeToDraw = false;
			if (curAnimIdx % 24 == 0)
				g_engine->_sound->playVoc();
			if (palAnimStep >= 4) {
				palAnimStep = 0;
				if (isPaletteAnimEnabled > 6)
					isPaletteAnimEnabled = 0;
				else
					isPaletteAnimEnabled += 1;
				updatePalette(isPaletteAnimEnabled);
			} else
				palAnimStep += 1;
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
	} while (!loopBreak && !g_engine->shouldQuit());

	g_engine->_sound->stopVoc();
	freeAnimation();
	if (currentRoomData->animationFlag) {
		secondaryAnimWidth = currAnimWidth;
		secondaryAnimHeight = curAnimHeight;
		setRoomTrajectories(secondaryAnimHeight, secondaryAnimWidth, RESTORE, false);
		loadAnimation(currentRoomData->animationName);
		iframe2 = curAnimIFrame;
		secondaryAnimation.posx = curAnimX;
		secondaryAnimation.posy = curAnimY;
		secondaryAnimation.depth = curAnimDepth;
		secondaryAnimation.dir = curAnimDir;
	}
	restoreBackground();
	assembleScreen();
	drawScreen(sceneBackground);
}

void updateVideo() {
	readBitmap(regobj.dropOverlay, screenLayers[regobj.depth - 1], regobj.dropOverlaySize, 319);
	restoreBackground();
	assembleScreen();
	drawScreen(sceneBackground);
}

void nicheAnimation(byte nicheDir, int32 bitmap) {
	uint bitmapOffset;
	int increment;

	// Room with Red
	if (currentRoomData->code == 24) {
		screenLayers[1] = (byte *)malloc(3660);
		readBitmap(1382874, screenLayers[1], 3652, 319);
		uint16 object1Width = READ_LE_UINT16(screenLayers[1]);
		uint16 object1Height = READ_LE_UINT16(screenLayers[1] + 2);
		depthMap[1].posx = 211;
		depthMap[1].posy = 16;
		depthMap[1].posx2 = 211 + object1Width + 1;
		depthMap[1].posy2 = 16 + object1Height + 1;
	}

	switch (nicheDir) {
	case 0: {
		bitmapOffset = 44904;
		increment = 1;
		Common::copy(screenLayers[0], screenLayers[0] + 892, sceneBackground + 44900);
		readBitmap(bitmap, screenLayers[0], 892, 319);
		Common::copy(screenLayers[0] + 4, screenLayers[0] + 4 + 888, sceneBackground + 44900 + 892);
	} break;
	case 1: { // object slides to reveal empty stand
		bitmapOffset = 892 + 44900;
		increment = -1;
		// Reads the empty niche into a non-visible part of background
		readBitmap(bitmap, sceneBackground + 44900, 892, 319);
		// Copies whatever is currently on the niche in a non-visible part of background contiguous with the above
		Common::copy(screenLayers[0] + 4, screenLayers[0] + 4 + 888, sceneBackground + 44900 + 892);
		// We now have in consecutive pixels the empty stand and the object

	} break;
	}
	uint16 nicheWidth = READ_LE_UINT16(screenLayers[0]);
	uint16 nicheHeight = READ_LE_UINT16(screenLayers[0] + 2);

	// Set the height to double to animate 2 images of the same height moving up/down
	*(sceneBackground + 44900 + 2) = (nicheHeight * 2) + 1;

	restoreBackground();

	for (uint i = 1; i <= nicheHeight; i++) {

		bitmapOffset = bitmapOffset + (increment * (nicheWidth + 1));
		Common::copy(sceneBackground + bitmapOffset, sceneBackground + bitmapOffset + 888, screenLayers[0] + 4);
		assembleScreen();
		drawScreen(sceneBackground);
		g_engine->_screen->update();
	}
	readBitmap(bitmap, screenLayers[0], 892, 319);

	restoreBackground();
	assembleScreen();
	drawScreen(sceneBackground);

	if (currentRoomData->code == 24) {
		free(screenLayers[1]);
		screenLayers[1] = NULL;
	}
}

void pickupScreenObject() {
	byte inventoryIndex;
	uint screenObject;

	uint mouseX = (mouseClickX + 7) / xGridCount;
	uint mouseY = (mouseClickY + 7) / yGridCount;
	screenObject = currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex;
	if (screenObject == 0)
		return;
	readItemRegister(screenObject);
	goToObject(
		currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount],
		currentRoomData->walkAreasGrid[mouseX][mouseY]);
	verifyCopyProtection();
	if (regobj.pickupable) {
		g_engine->_mouseManager->hide();
		switch (regobj.code) {
		case 521: { // Corridor lamp
			currentRoomData->mouseGrid[10][11] = 19;
			currentRoomData->mouseGrid[9][12] = 18;
			currentRoomData->mouseGrid[10][12] = 18;
		} break;
		case 567: { // Pickup rubble
			if (currentRoomData->animationFlag) {
				g_engine->_mouseManager->show();
				drawText(3226);
				return;
			}
		} break;
		case 590: // Ectoplasm
			caves[1] = true;
			break;
		case 665: // Bird
			caves[0] = true;
			break;
		case 676:
		case 688: {
			caves[4] = true;
			isVasePlaced = false;
		} break;
		}
		switch (regobj.height) {
		case 0: { // Pick up above
			switch (regobj.code) {
			case 590: { // Ectoplasm
				animatePickup1(3, 0);
				animatePickup2(3, 0);
			} break;
			default: {
				animatePickup1(charFacingDirection, 0);
				screenLayers[regobj.depth - 1] = NULL;
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animatePickup2(charFacingDirection, 0);
			}
			}
		} break;
		case 1: { // Waist level
			switch (regobj.code) {
			case 218: { // Necronomicon
				animatePickup1(0, 1);
				animatePickup2(0, 1);
			} break;
			case 223: { // table cloths
				animatePickup1(0, 1);
				currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex = regobj.replaceWith;
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
					RoomBitmapRegister &with = currentRoomData->screenLayers[1];

					with.bitmapPointer = 775611;
					with.bitmapSize = 36;
					with.coordx = 80;
					with.coordy = 56;
					with.depth = 2;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{
					RoomBitmapRegister &with = currentRoomData->screenLayers[2];

					with.bitmapPointer = 730743;
					with.bitmapSize = 64;
					with.coordx = 76;
					with.coordy = 62;
					with.depth = 1;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				screenLayers[regobj.depth - 1] = NULL;
				restoreBackground();

				assembleScreen();
				drawScreen(sceneBackground);
				animatePickup2(0, 1);
			} break;
			case 562: { // niche
				switch (currentRoomData->code) {
				case 20: { // First scene with niche
					if (niche[0][niche[0][3]] > 0) {
						// Possibly
						if (niche[0][3] == 2 || niche[0][niche[0][3]] == 563) {
							readItemRegister(niche[0][niche[0][3]]);
							niche[0][niche[0][3]] = 0;
							currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
							animatePickup1(3, 1);
							readBitmap(1190768, screenLayers[regobj.depth - 1], 892, 319);
							currentRoomData->screenLayers[1].bitmapPointer = 1190768;
							currentRoomData->screenLayers[1].bitmapSize = 892;
							currentRoomData->screenLayers[1].coordx = 66;
							currentRoomData->screenLayers[1].coordy = 35;
							currentRoomData->screenLayers[1].depth = 1;
							restoreBackground();
							assembleScreen();
							drawScreen(sceneBackground);
							animatePickup2(3, 1);
						} else {
							readItemRegister(niche[0][niche[0][3]]);
							niche[0][niche[0][3]] = 0;
							niche[0][3] += 1;
							niche[1][3] -= 1;
							currentRoomData->screenObjectIndex[9]->objectName = "                    ";
							animatePickup1(3, 1);
							readBitmap(1190768, screenLayers[regobj.depth - 1],
									   892, 319);
							restoreBackground();
							assembleScreen();
							drawScreen(sceneBackground);
							animatePickup2(3, 1);
							g_engine->_sound->playVoc("PLATAF", 375907, 14724);
							currentRoomData->screenLayers[1].bitmapSize = 892;
							currentRoomData->screenLayers[1].coordx = 66;
							currentRoomData->screenLayers[1].coordy = 35;
							currentRoomData->screenLayers[1].depth = 1;
							switch (niche[0][niche[0][3]]) {
							case 0: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
								nicheAnimation(0, 1190768);
								currentRoomData->screenLayers[1].bitmapPointer = 1190768;
							} break;
							case 561: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
								nicheAnimation(0, 1182652);
								currentRoomData->screenLayers[1].bitmapPointer = 1182652;
							} break;
							case 563: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
								nicheAnimation(0, 1186044);
								currentRoomData->screenLayers[1].bitmapPointer = 1186044;
							} break;
							case 615: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
								nicheAnimation(0, 1181760);
								currentRoomData->screenLayers[1].bitmapPointer = 1181760;
							} break;
							}
							updateAltScreen(24);
						}
					} else {

						g_engine->_mouseManager->show();
						drawText(1049 + Random(10));
						g_engine->_mouseManager->hide();
						return;
					}
				} break;
				case 24: { // Second scene with niche
					if (niche[1][niche[1][3]] > 0 && niche[1][3] != 1) {
						if (niche[1][3] == 2) {
							readItemRegister(niche[1][2]);
							niche[1][2] = 0;
							currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
							animatePickup1(0, 1);
							readBitmap(1399610, screenLayers[regobj.depth - 1], 892, 319);
							currentRoomData->screenLayers[0].bitmapPointer = 1399610;
							currentRoomData->screenLayers[0].bitmapSize = 892;
							currentRoomData->screenLayers[0].coordx = 217;
							currentRoomData->screenLayers[0].coordy = 48;
							currentRoomData->screenLayers[0].depth = 1;
							restoreBackground();
							assembleScreen();
							drawScreen(sceneBackground);
							animatePickup2(0, 1);
						} else {
							readItemRegister(niche[1][niche[1][3]]);
							niche[1][niche[1][3]] = 622;
							niche[1][3] += 1;
							niche[0][3] -= 1;
							currentRoomData->screenObjectIndex[8]->objectName = "                    ";
							animatePickup1(0, 1);
							readBitmap(1399610, screenLayers[0], 892, 319);
							restoreBackground();
							assembleScreen();
							drawScreen(sceneBackground);
							animatePickup2(0, 1);
							g_engine->_sound->playVoc("PLATAF", 375907, 14724);
							currentRoomData->screenLayers[0].bitmapSize = 892;
							currentRoomData->screenLayers[0].coordx = 217;
							currentRoomData->screenLayers[0].coordy = 48;
							currentRoomData->screenLayers[0].depth = 1;
							switch (niche[1][niche[1][3]]) {
							case 0: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
								nicheAnimation(0, 1399610);
								currentRoomData->screenLayers[0].bitmapPointer = 1399610;
							} break;
							case 561: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
								nicheAnimation(0, 1381982);
								currentRoomData->screenLayers[0].bitmapPointer = 1381982;
							} break;
							case 615: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
								nicheAnimation(0, 1381090);
								currentRoomData->screenLayers[0].bitmapPointer = 1381090;
							} break;
							case 622: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
								nicheAnimation(0, 1400502);
								currentRoomData->screenLayers[0].bitmapPointer = 1400502;
							} break;
							case 623: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
								nicheAnimation(0, 1398718);
								currentRoomData->screenLayers[0].bitmapPointer = 1398718;
							} break;
							}
							updateAltScreen(20);
						}
					} else {
						g_engine->_mouseManager->show();
						drawText(1049 + Random(10));
						g_engine->_mouseManager->hide();
						return;
					}
				} break;
				}
			} break;
			case 624: { // red devil
				animatePickup1(2, 1);
				{
					RoomBitmapRegister &with = currentRoomData->screenLayers[3];

					with.bitmapPointer = 0;
					with.bitmapSize = 0;
					with.coordx = 0;
					with.coordy = 0;
					with.depth = 0;
				}
				screenLayers[3] = NULL;
				disableSecondAnimation();
				drawScreen(sceneBackground);
				animatePickup2(2, 1);
				isRedDevilCaptured = true;
				isTrapSet = false;
			} break;
			default: {
				animatePickup1(charFacingDirection, 1);
				screenLayers[regobj.depth - 1] = NULL;
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animatePickup2(charFacingDirection, 1);
			}
			}
		} break;
		case 2: { // Pick up feet level
			switch (regobj.code) {
			case 216: { // chisel
				animatePickup1(0, 2);
				currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex = regobj.replaceWith;
				updateVideo();
				animatePickup2(0, 2);
			} break;
			case 295: { // candles
				animatePickup1(3, 2);
				currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex = regobj.replaceWith;
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
				screenLayers[regobj.depth - 1] = NULL;
				{ // bird
					RoomBitmapRegister &with = currentRoomData->screenLayers[2];

					with.bitmapPointer = 1545924;
					with.bitmapSize = 172;
					with.coordx = 38;
					with.coordy = 58;
					with.depth = 1;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{ // ring
					RoomBitmapRegister &with = currentRoomData->screenLayers[1];

					with.bitmapPointer = 1591272;
					with.bitmapSize = 92;
					with.coordx = 50;
					with.coordy = 58;
					with.depth = 3;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animatePickup2(3, 2);
			} break;
			default: {
				animatePickup1(charFacingDirection, 2);
				screenLayers[regobj.depth - 1] = NULL;
				assembleScreen();
				drawScreen(sceneBackground);
				animatePickup2(charFacingDirection, 2);
			}
			}
		} break;
		case 9: { // bat
			foo = regobj.pickTextRef;
			readItemRegister(204);
			animatePickup1(0, 1);
			animateOpen2(0, 1);
			animateBat();
			g_engine->_mouseManager->show();
			drawText(foo);
			currentRoomData->mouseGrid[34][8] = 24;
			actionCode = 0;
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
			return;
		} break;
		}
		g_engine->_mouseManager->show();

		if (regobj.code != 624)
			for (int j = regobj.yrej1; j <= regobj.yrej2; j++)
				for (int i = regobj.xrej1; i <= regobj.xrej2; i++) {
					currentRoomData->walkAreasGrid[i][j] = regobj.walkAreasPatch[i - regobj.xrej1][j - regobj.yrej1];
					currentRoomData->mouseGrid[i][j] = regobj.mouseGridPatch[i - regobj.xrej1][j - regobj.yrej1];
				}
		switch (regobj.code) {
		case 216: { // chisel
			currentRoomData->screenLayers[5].bitmapPointer = 517485;
			currentRoomData->screenLayers[5].bitmapSize = 964;
			currentRoomData->screenLayers[5].coordx = 223;
			currentRoomData->screenLayers[5].coordy = 34;
			currentRoomData->screenLayers[5].depth = 1;
		} break;
		case 218:; // necronomicon
			break;
		case 223: { // table cloth
			currentRoomData->screenLayers[6].bitmapPointer = 436752;
			currentRoomData->screenLayers[6].bitmapSize = 1372;
			currentRoomData->screenLayers[6].coordx = 174;
			currentRoomData->screenLayers[6].coordy = 32;
			currentRoomData->screenLayers[6].depth = 1;
		} break;
		case 295: { // candles
			currentRoomData->screenLayers[3].bitmapPointer = 1130756;
			currentRoomData->screenLayers[3].bitmapSize = 1764;
			currentRoomData->screenLayers[3].coordx = 100;
			currentRoomData->screenLayers[3].coordy = 28;
			currentRoomData->screenLayers[3].depth = 1;
		} break;
		case 308:; // mistletoe
			break;
		case 362:; // charcoal
			break;
		case 402: {
			currentRoomData->screenLayers[5].bitmapPointer = 68130;
			currentRoomData->screenLayers[5].bitmapSize = 2564;
			currentRoomData->screenLayers[5].coordx = 148;
			currentRoomData->screenLayers[5].coordy = 49;
			currentRoomData->screenLayers[5].depth = 7;
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
				if (currentRoomData->screenLayers[i].bitmapPointer ==
					regobj.bitmapPointer) {
					currentRoomData->screenLayers[i].bitmapPointer = 0;
					currentRoomData->screenLayers[i].bitmapSize = 0;
					currentRoomData->screenLayers[i].coordx = 0;
					currentRoomData->screenLayers[i].coordy = 0;
					currentRoomData->screenLayers[i].depth = 0;
				}
		}
		}
	} else {
		if (regobj.pickTextRef > 0)
			drawText(regobj.pickTextRef);
		actionCode = 0;
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
		return;
	}
	inventoryIndex = 0;

	while (mobj[inventoryIndex].code != 0) {
		inventoryIndex += 1;
	}

	mobj[inventoryIndex].bitmapIndex = regobj.objectIconBitmap;
	mobj[inventoryIndex].code = regobj.code;
	mobj[inventoryIndex].objectName = regobj.name;
	g_engine->_mouseManager->hide();
	drawBackpack();
	g_engine->_mouseManager->show();
	if (regobj.pickTextRef > 0)
		drawText(regobj.pickTextRef);
	actionCode = 0;
	oldGridX = 0;
	oldGridY = 0;
	checkMouseGrid();
}

void replaceBackpack(byte obj1, uint obj2) {
	readItemRegister(obj2);
	mobj[obj1].bitmapIndex = regobj.objectIconBitmap;
	mobj[obj1].code = obj2;
	mobj[obj1].objectName = regobj.name;
	cpCounter = cpCounter2;
}

void dropObjectInScreen(ScreenObject replacementObject) {
	byte objIndex;

	if (replacementObject.bitmapSize > 0) {
		objIndex = 0;
		while (!(currentRoomData->screenLayers[objIndex].bitmapSize == 0) || objIndex == 15) {
			objIndex++;
		}
		if (currentRoomData->screenLayers[objIndex].bitmapSize == 0) {
			{
				RoomBitmapRegister &with = currentRoomData->screenLayers[objIndex];

				with.bitmapPointer = replacementObject.bitmapPointer;
				with.bitmapSize = replacementObject.bitmapSize;
				with.coordx = replacementObject.dropOverlayX;
				with.coordy = replacementObject.dropOverlayY;
				with.depth = replacementObject.depth;
				loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
			}
			for (int j = replacementObject.yrej1; j <= replacementObject.yrej2; j++)
				for (int i = replacementObject.xrej1; i <= replacementObject.xrej2; i++) {
					currentRoomData->walkAreasGrid[i][j] = replacementObject.walkAreasPatch[i - replacementObject.xrej1][j - replacementObject.yrej1];
					currentRoomData->mouseGrid[i][j] = replacementObject.mouseGridPatch[i - replacementObject.xrej1][j - replacementObject.yrej1];
				}
		} else
			showError(264);
	}
}

void useScreenObject() {
	byte usedObjectIndex,
		listIndex, invIndex;
	bool verifyList, foobar;

	uint mouseX = (mouseClickX + 7) / xGridCount;
	uint mouseY = (mouseClickY + 7) / yGridCount;
	uint sceneObject = currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex;

	if (inventoryObjectName != "") { //Use inv object with something on the scene
		usedObjectIndex = 0;
		while (mobj[usedObjectIndex].objectName != inventoryObjectName) {
			usedObjectIndex += 1;
		}

		readItemRegister(mobj[usedObjectIndex].code);

		goToObject(
			currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount],
			currentRoomData->walkAreasGrid[mouseX][mouseY]);

		if (regobj.useWith == sceneObject && sceneObject > 0 && regobj.used[0] == 5) {
			switch (regobj.useWith) {
			case 30: { // corn with rooster
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();

				animateGive(1, 2);
				animatePickup2(1, 2);

				updateItem(regobj.code);
				currentRoomData->screenObjectIndex[27]->fileIndex = 201;
				do {
					g_engine->_chrono->updateChrono();
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					if (currentSecondaryTrajectoryIndex >= currentRoomData->secondaryTrajectoryLength)
						currentSecondaryTrajectoryIndex = 1;
					else
						currentSecondaryTrajectoryIndex += 1;
					secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
					secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
					secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
					emptyLoop();
					timeToDraw = false;
					emptyLoop2();
					sprites(true);
					g_engine->_screen->update();
				} while (currentSecondaryTrajectoryIndex != 4);

				for (listIndex = 0; listIndex < maxXGrid; listIndex++)
					for (invIndex = 0; invIndex < maxYGrid; invIndex++) {
						currentRoomData->walkAreasGrid[oldposx + listIndex][oldposy + invIndex] = movementGridForSecondaryAnim[listIndex][invIndex];
						currentRoomData->mouseGrid[oldposx + listIndex][oldposy + invIndex] = mouseGridForSecondaryAnim[listIndex][invIndex];
					}

				freeAnimation();
				secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x + 8;
				secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
				currentRoomData->animationName = "GALLOPIC";
				currentRoomData->secondaryAnimDirections[299] = 201;
				loadAnimation("GALLOPIC");
				currentRoomData->secondaryAnimDirections[0] = 0;
				currentRoomData->secondaryAnimTrajectory[0].x = secondaryAnimation.posx;
				currentRoomData->secondaryAnimTrajectory[0].y = secondaryAnimation.posy;
				currentSecondaryTrajectoryIndex = 1;
				currentRoomData->secondaryTrajectoryLength = 1;

				for (listIndex = 0; listIndex < maxXGrid; listIndex++)
					for (invIndex = 0; invIndex < maxYGrid; invIndex++) {
						if (maskGridSecondaryAnim[listIndex][invIndex] > 0)
							currentRoomData->walkAreasGrid[oldposx + listIndex][oldposy + invIndex] = maskGridSecondaryAnim[listIndex][invIndex];
						if (maskMouseSecondaryAnim[listIndex][invIndex] > 0)
							currentRoomData->mouseGrid[oldposx + listIndex][oldposy + invIndex] = maskMouseSecondaryAnim[listIndex][invIndex];
					}
				g_engine->_mouseManager->show();
			} break;
			case 153: { // oil with well
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("BLUP", 330921, 3858);
				animateGive(3, 1);
				animatePickup2(3, 1);
				g_engine->_mouseManager->show();
				updateItem(regobj.code);
				currentRoomData->screenObjectIndex[21]->fileIndex = 154;
			} break;
			case 157: { // giving something to john
				verifyList = false;
				debug("used object = %d", mobj[usedObjectIndex].code);
				if (obtainedList1) {
					for (listIndex = 0; listIndex < 5; listIndex++) {
						if (mobj[usedObjectIndex].code == firstList[listIndex])
							verifyList = true;
					}

					if (verifyList) {
						int completedListItems = 0;
						for (listIndex = 0; listIndex < 5; listIndex++) {
							for (invIndex = 0; invIndex < inventoryIconCount; invIndex++) {
								if (mobj[invIndex].code == firstList[listIndex]) {
									completedListItems += 1;
									break;
								}
							}
						}
						if (completedListItems == 5) {
							obtainedList1 = false;
							list1Complete = true;
							assignText();
							hypertext(1018, 255, 0, foo, false);
							hypertext(foo, 253, 249, foo, true);
							verb.close();

							g_engine->_mouseManager->hide();
							animateGive(charFacingDirection, 1);
							animateOpen2(charFacingDirection, 1);
							for (listIndex = 0; listIndex < 5; listIndex++) {
								invIndex = 0;
								while (mobj[invIndex].code != firstList[listIndex]) {
									invIndex += 1;
								}
								updateInventory(invIndex);
							}
							listIndex = 0;
							while (mobj[listIndex].code != 149) {
								listIndex += 1;
							}
							updateInventory(listIndex);
							drawBackpack();
							g_engine->_mouseManager->show();
						} else {
							assignText();
							hypertext(regobj.useTextRef, 255, 0, foo, false);
							g_engine->_mouseManager->hide();
							animateGive(charFacingDirection, 1);
							animateOpen2(charFacingDirection, 1);
							g_engine->_mouseManager->show();
							hypertext(foo, 253, 249, foo, true);
							verb.close();
						}
					}
				}
				verifyList = false;
				if (obtainedList2) {
					for (listIndex = 0; listIndex < 5; listIndex++)
						if (mobj[usedObjectIndex].code == secondList[listIndex])
							verifyList = true;
					if (verifyList) {
						int completedListItems = 0;
						for (listIndex = 0; listIndex < 5; listIndex++) {
							for (invIndex = 0; invIndex <= inventoryIconCount; invIndex++) {
								if (mobj[invIndex].code == secondList[listIndex]) {
									completedListItems += 1;
									break;
								}
							}
						}
						if (completedListItems == 5) {
							obtainedList2 = false;
							list2Complete = true;
							assignText();
							hypertext(1020, 255, 0, foo, false);
							hypertext(foo, 253, 249, foo, true);
							verb.close();
							g_engine->_mouseManager->hide();
							animateGive(charFacingDirection, 1);
							animateOpen2(charFacingDirection, 1);
							for (listIndex = 0; listIndex < 5; listIndex++) {
								invIndex = 0;
								while (mobj[invIndex].code != secondList[listIndex]) {
									invIndex += 1;
								}
								updateInventory(invIndex);
							}
							listIndex = 0;
							while (mobj[listIndex].code != 150) {
								listIndex += 1;
							}
							updateInventory(listIndex);
							drawBackpack();
							g_engine->_mouseManager->show();
						} else {
							assignText();
							hypertext(regobj.useTextRef, 255, 0, foo, false);
							g_engine->_mouseManager->hide();
							animateGive(charFacingDirection, 1);
							animateOpen2(charFacingDirection, 1);
							g_engine->_mouseManager->show();
							hypertext(foo, 253, 249, foo, true);
							verb.close();
						}
					}
				}
			} break;
			case 159: {
				switch (regobj.code) {
				case 173: {
					assignText();
					hypertext(1118, 255, 0, foo, false);
					g_engine->_mouseManager->hide();
					animateGive(charFacingDirection, 1);
					animateOpen2(charFacingDirection, 1);
					g_engine->_mouseManager->show();
					hypertext(foo, 253, 249, foo, true);
					verb.close();
				} break;
				case 218: {
					assignText();
					hypertext(687, 255, 0, foo, false);
					g_engine->_mouseManager->hide();
					animateGive(charFacingDirection, 1);
					animateOpen2(charFacingDirection, 1);
					g_engine->_mouseManager->show();
					hypertext(foo, 253, 249, foo, true);
					verb.close();
					g_engine->_mouseManager->hide();
					updateInventory(usedObjectIndex);
					drawBackpack();
					g_engine->_mouseManager->show();
					for (foo = 0; foo < characterCount; foo++)
						bookTopic[foo] = true;
					firstTimeTopicB[0] = true;
				} break;
				}
			} break;
			case 160: {
				assignText();
				hypertext(2466, 255, 0, foo, false);
				g_engine->_mouseManager->hide();
				animateGive(charFacingDirection, 1);
				animateOpen2(charFacingDirection, 1);
				g_engine->_mouseManager->show();
				hypertext(foo, 253, 249, foo, true);
				verb.close();
				g_engine->_mouseManager->hide();
				updateInventory(usedObjectIndex);
				drawBackpack();
				g_engine->_mouseManager->show();
				for (foo = 0; foo < characterCount; foo++) {
					mintTopic[foo] = true;
					firstTimeTopicC[foo] = true;
				}
			} break;
			case 164: {
				switch (regobj.code) {
				case 563: {
					isPottersManualDelivered = true;
					if (isPottersWheelDelivered && isGreenDevilDelivered && isMudDelivered) {
						assignText();
						hypertext(2696, 255, 0, foo, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(foo, 253, 249, foo, true);
						caves[4] = true;
						hypertext(3247, 253, 249, foo, true);
						verb.close();
						g_engine->_mouseManager->hide();
						replaceBackpack(usedObjectIndex, 676);
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
						}
						currentRoomData->mouseGrid[15][12] = 7;
						g_engine->_mouseManager->show();
					} else {
						assignText();
						hypertext(2696, 255, 0, foo, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(foo, 253, 249, foo, true);
						hypertext(3246, 253, 249, foo, true);
						verb.close();
						g_engine->_mouseManager->hide();
						updateInventory(usedObjectIndex);
						drawBackpack();
						g_engine->_mouseManager->show();
					}
				} break;
				case 598: {
					isMudDelivered = true;
					if (isPottersWheelDelivered && isGreenDevilDelivered && isPottersManualDelivered) {
						assignText();
						hypertext(2821, 255, 0, foo, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(foo, 253, 249, foo, true);
						caves[4] = true;
						hypertext(3247, 253, 249, foo, true);
						verb.close();
						g_engine->_mouseManager->hide();
						replaceBackpack(usedObjectIndex, 676);
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
						}
						currentRoomData->mouseGrid[15][12] = 7;
						g_engine->_mouseManager->show();
					} else {
						assignText();
						hypertext(2821, 255, 0, foo, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(foo, 253, 249, foo, true);
						hypertext(3246, 253, 249, foo, true);
						verb.close();
						g_engine->_mouseManager->hide();
						updateInventory(usedObjectIndex);
						drawBackpack();
						g_engine->_mouseManager->show();
					}
				} break;
				case 623: {
					isPottersWheelDelivered = true;
					if (isMudDelivered && isGreenDevilDelivered && isPottersManualDelivered) {
						assignText();
						hypertext(2906, 255, 0, foo, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(foo, 253, 249, foo, true);
						caves[4] = true;
						hypertext(3247, 253, 249, foo, true);
						verb.close();
						g_engine->_mouseManager->hide();
						replaceBackpack(usedObjectIndex, 676);
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
						}
						currentRoomData->mouseGrid[15][12] = 7;
						g_engine->_mouseManager->show();
					} else {
						assignText();
						hypertext(2906, 255, 0, foo, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(foo, 253, 249, foo, true);
						hypertext(3246, 253, 249, foo, true);
						verb.close();
						g_engine->_mouseManager->hide();
						updateInventory(usedObjectIndex);
						drawBackpack();
						g_engine->_mouseManager->show();
					}
				} break;
				case 701: {
					isGreenDevilDelivered = true;
					if (isMudDelivered && isPottersWheelDelivered && isPottersManualDelivered) {
						assignText();
						hypertext(3188, 255, 0, foo, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(foo, 253, 249, foo, true);
						caves[4] = true;
						hypertext(3247, 253, 249, foo, true);
						verb.close();
						g_engine->_mouseManager->hide();
						replaceBackpack(usedObjectIndex, 676);
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = currentRoomData->screenLayers[0];

							with.bitmapPointer = 1545820;
							with.bitmapSize = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.depth = 1;
							loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
						}
						currentRoomData->mouseGrid[15][12] = 7;
						g_engine->_mouseManager->show();
					} else {
						assignText();
						hypertext(3188, 255, 0, foo, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(foo, 253, 249, foo, true);
						hypertext(3246, 253, 249, foo, true);
						verb.close();
						g_engine->_mouseManager->hide();
						updateInventory(usedObjectIndex);
						drawBackpack();
						g_engine->_mouseManager->show();
					}
				} break;
				}
			} break;
			case 165: {
				drawText(1098);
				g_engine->_mouseManager->hide();
				do {
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					if (currentSecondaryTrajectoryIndex >= currentRoomData->secondaryTrajectoryLength)
						currentSecondaryTrajectoryIndex = 1;
					else
						currentSecondaryTrajectoryIndex += 1;
					secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
					secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
					secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
					emptyLoop();
					timeToDraw = false;
					emptyLoop2();
					sprites(true);
				} while (!(currentSecondaryTrajectoryIndex == (currentRoomData->secondaryTrajectoryLength / 2)));

				animateGive(3, 2);
				updateInventory(usedObjectIndex);
				drawBackpack();
				animateOpen2(3, 2);
				animatedSequence(6);

				do {
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					if (currentSecondaryTrajectoryIndex >= currentRoomData->secondaryTrajectoryLength)
						currentSecondaryTrajectoryIndex = 1;
					else
						currentSecondaryTrajectoryIndex += 1;
					secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
					secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
					secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
					emptyLoop();
					timeToDraw = false;

					emptyLoop2();
					sprites(true);
				} while (currentSecondaryTrajectoryIndex != currentRoomData->secondaryTrajectoryLength);
				disableSecondAnimation();
				drawScreen(sceneBackground);
				g_engine->_mouseManager->show();
			} break;
			case 201: {
				drawText(regobj.useTextRef);
				g_engine->_sound->playVoc("GALLO", 94965, 46007);
				g_engine->_mouseManager->hide();
				animatedSequence(5);
				replaceBackpack(usedObjectIndex, 423);
				drawBackpack();
				g_engine->_mouseManager->show();
				actionCode = 0;
				oldGridX = 0;
				oldGridY = 0;
				checkMouseGrid();
			} break;
			case 219: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("TAZA", 223698, 29066);
				animateGive(3, 2);
				do {
					g_engine->_chrono->updateChrono();
					if (timeToDraw) {
						if (palAnimStep >= 4) {
							palAnimStep = 0;
							if (isPaletteAnimEnabled > 6)
								isPaletteAnimEnabled = 0;
							else
								isPaletteAnimEnabled += 1;
							updatePalette(isPaletteAnimEnabled);
						} else
							palAnimStep += 1;
						timeToDraw = false;
					}
					g_engine->_screen->update();
					g_system->delayMillis(10);
				} while (g_engine->_sound->isVocPlaying());
				animateOpen2(3, 2);
				updateItem(regobj.code);
				disableSecondAnimation();
				drawScreen(sceneBackground);
				g_engine->_mouseManager->show();
				drawText(2652);
				g_engine->_mouseManager->hide();
				readItemRegister(536);
				for (int i = 12; i <= 13; i++)
					for (int j = 7; j <= 14; j++)
						currentRoomData->mouseGrid[i][j] = 14;
				for (int i = 8; i <= 12; i++)
					currentRoomData->mouseGrid[14][i] = 14;
				currentRoomData->mouseGrid[9][10] = 1;
				currentRoomData->mouseGrid[10][10] = 1;
				for (int i = 0; i < 15; i++)
					if (currentRoomData->screenLayers[i].bitmapPointer ==
						regobj.bitmapPointer) {
						currentRoomData->screenLayers[i].bitmapPointer = 0;
						currentRoomData->screenLayers[i].bitmapSize = 0;
						currentRoomData->screenLayers[i].coordx = 0;
						currentRoomData->screenLayers[i].coordy = 0;
						currentRoomData->screenLayers[i].depth = 0;
					}
				usedObjectIndex = 0;
				while (mobj[usedObjectIndex].code != 0) {
					usedObjectIndex += 1;
				}
				mobj[usedObjectIndex].bitmapIndex = regobj.objectIconBitmap;
				mobj[usedObjectIndex].code = regobj.code;
				mobj[usedObjectIndex].objectName = regobj.name;
				animatedSequence(4);
				g_engine->_mouseManager->show();
				actionCode = 0;
				oldGridX = 0;
				oldGridY = 0;
				checkMouseGrid();
			} break;
			case 221: {
				drawText(regobj.useTextRef);
				regobj.used[0] = 9;
				usedObjectIndex = 0;
				while (mobj[usedObjectIndex].code != 0) {
					usedObjectIndex += 1;
				}

				invItemData->seek(regobj.code);

				saveItem(regobj, invItemData);
				readItemRegister(invItemData, 221, regobj);
				mobj[usedObjectIndex].bitmapIndex = regobj.objectIconBitmap;
				mobj[usedObjectIndex].code = regobj.code;
				mobj[usedObjectIndex].objectName = regobj.name;

				g_engine->_mouseManager->hide();
				animatePickup1(2, 0);
				g_engine->_sound->playVoc("TIJERAS", 252764, 5242);
				g_engine->_sound->waitForSoundEnd();
				animatePickup2(2, 0);
				drawBackpack();
				g_engine->_mouseManager->show();
			} break;
			case 227: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("ALACENA", 319112, 11809);
				animatePickup1(0, 2);
				g_engine->_sound->waitForSoundEnd();
				animateOpen2(0, 2);
				replaceBackpack(usedObjectIndex, 453);
				drawBackpack();
				g_engine->_mouseManager->show();
				updateItem(mobj[usedObjectIndex].code);
				isCupboardOpen = true;
			} break;
			case 274: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("CINCEL", 334779, 19490);
				animatePickup1(charFacingDirection, 2);
				g_engine->_sound->waitForSoundEnd();
				animateOpen2(charFacingDirection, 2);
				g_engine->_mouseManager->show();
				updateItem(regobj.code);
				isChestOpen = true;
			} break;
			case 416: {
				updateItem(regobj.code);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("PUERTA", 186429, 4754);
				animatePickup1(0, 1);
				animateOpen2(0, 1);
				g_engine->_sound->loadVoc("GOTA", 140972, 1029);
				g_engine->_mouseManager->show();
				drawText(regobj.useTextRef);
				currentRoomData->doors[2].openclosed = 0;
			} break;
			case 446: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("TAZA", 223698, 29066);
				animateGive(0, 2);
				g_engine->_sound->waitForSoundEnd();
				animatePickup2(0, 2);
				replaceBackpack(usedObjectIndex, 204);
				drawBackpack();
				g_engine->_mouseManager->show();
			} break;
			case 507: {
				g_engine->_mouseManager->hide();
				animatePickup1(0, 1);
				g_engine->_sound->playVoc("MAQUINA", 153470, 7378);
				animateOpen2(0, 1);
				updateInventory(usedObjectIndex);
				drawBackpack();
				g_engine->_mouseManager->show();
				currentRoomData->mouseGrid[27][8] = 22;
			} break;
			case 549: {
				updateItem(regobj.code);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("PUERTA", 186429, 4754);
				animatePickup1(1, 1);
				animateOpen2(1, 1);
				g_engine->_mouseManager->show();
				drawText(regobj.useTextRef);
				currentRoomData->doors[0].openclosed = 0;
			} break;
			case 562: { // put any object in the niches
				switch (currentRoomData->code) {
				case 20: {
					if (niche[0][niche[0][3]] == 0) {

						if (niche[0][3] == 0) {
							niche[0][0] = regobj.code;
							drawText(regobj.useTextRef);
							g_engine->_mouseManager->hide();
							currentRoomData->screenObjectIndex[9]->objectName = "                    ";
							animateGive(3, 1);
							switch (niche[0][0]) {
							case 561: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
								readBitmap(1182652, screenLayers[0], 892, 319);
								currentRoomData->screenLayers[1].bitmapPointer = 1182652;
							} break;
							case 615: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
								readBitmap(1181760, screenLayers[0], 892, 319);
								currentRoomData->screenLayers[1].bitmapPointer = 1181760;
							} break;
							}
							currentRoomData->screenLayers[1].bitmapSize = 892;
							currentRoomData->screenLayers[1].coordx = 66;
							currentRoomData->screenLayers[1].coordy = 35;
							currentRoomData->screenLayers[1].depth = 1;
							restoreBackground();
							assembleScreen();
							drawScreen(sceneBackground);
							animateOpen2(3, 1);
							updateInventory(usedObjectIndex);
							drawBackpack();
							g_engine->_mouseManager->show();
						} else {

							niche[0][niche[0][3]] = regobj.code;
							niche[1][3] += 1;
							niche[0][3] -= 1;
							drawText(regobj.useTextRef);
							g_engine->_mouseManager->hide();
							animateGive(3, 1);
							switch (regobj.code) {
							case 561:
								readBitmap(1182652, screenLayers[0], 892, 319);
								break;
							case 615:
								readBitmap(1181760, screenLayers[0], 892, 319);
								break;
							}
							restoreBackground();
							assembleScreen();
							drawScreen(sceneBackground);
							animateOpen2(3, 1);
							updateInventory(usedObjectIndex);
							drawBackpack();
							currentRoomData->screenObjectIndex[9]->objectName = "                    ";
							g_engine->_sound->playVoc("PLATAF", 375907, 14724);
							switch (niche[0][niche[0][3]]) {
							case 0: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
								nicheAnimation(1, 1190768);
								currentRoomData->screenLayers[1].bitmapPointer = 1190768;
							} break;
							case 561: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
								nicheAnimation(1, 1182652);
								currentRoomData->screenLayers[1].bitmapPointer = 1182652;
							} break;
							case 563: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
								nicheAnimation(1, 1186044);
								currentRoomData->screenLayers[1].bitmapPointer = 1186044;
							} break;
							case 615: {
								currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
								nicheAnimation(1, 1181760);
								currentRoomData->screenLayers[1].bitmapPointer = 1181760;
							} break;
							}
							currentRoomData->screenLayers[1].bitmapSize = 892;
							currentRoomData->screenLayers[1].coordx = 66;
							currentRoomData->screenLayers[1].coordy = 35;
							currentRoomData->screenLayers[1].depth = 1;
							g_engine->_mouseManager->show();
							updateAltScreen(24);
						}
					} else {
						drawText(Random(11) + 1022);
					}
				} break;
				case 24: {
					if (niche[1][niche[1][3]] == 0) {

						if (niche[1][3] == 0) {

							niche[1][0] = regobj.code;
							drawText(regobj.useTextRef);
							g_engine->_mouseManager->hide();
							currentRoomData->screenObjectIndex[8]->objectName = "                    ";
							animateGive(0, 1);
							switch (niche[1][0]) {
							case 561: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
								readBitmap(1381982, screenLayers[0], 892, 319);
								currentRoomData->screenLayers[0].bitmapPointer = 1381982;
							} break;
							case 615: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
								readBitmap(1381090, screenLayers[0], 892, 319);
								currentRoomData->screenLayers[0].bitmapPointer = 1381090;
							} break;
							}
							currentRoomData->screenLayers[0].bitmapSize = 892;
							currentRoomData->screenLayers[0].coordx = 217;
							currentRoomData->screenLayers[0].coordy = 48;
							currentRoomData->screenLayers[0].depth = 1;
							restoreBackground();
							assembleScreen();
							drawScreen(sceneBackground);
							animateOpen2(0, 1);
							updateInventory(usedObjectIndex);
							drawBackpack();
							g_engine->_mouseManager->show();
						} else {

							niche[1][niche[1][3]] = regobj.code;
							niche[0][3] += 1;
							niche[1][3] -= 1;
							drawText(regobj.useTextRef);
							g_engine->_mouseManager->hide();
							animateGive(0, 1);

							switch (regobj.code) {
							case 561:
								readBitmap(1381982, screenLayers[regobj.depth - 1],
										   892, 319);
								break;
							case 615:
								readBitmap(1381090, screenLayers[regobj.depth - 1],
										   892, 319);
								break;
							}
							restoreBackground();
							assembleScreen();
							drawScreen(sceneBackground);
							animateOpen2(0, 1);
							updateInventory(usedObjectIndex);
							drawBackpack();
							currentRoomData->screenObjectIndex[8]->objectName = "                    ";
							g_engine->_sound->playVoc("PLATAF", 375907, 14724);
							switch (niche[1][niche[1][3]]) {
							case 0: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
								nicheAnimation(1, 1399610);
								currentRoomData->screenLayers[0].bitmapPointer = 1399610;
							} break;
							case 561: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
								nicheAnimation(1, 1381982);
								currentRoomData->screenLayers[0].bitmapPointer = 1381982;
							} break;
							case 615: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
								nicheAnimation(1, 1381090);
								currentRoomData->screenLayers[0].bitmapPointer = 1381090;
							} break;
							case 622: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
								nicheAnimation(1, 1400502);
								currentRoomData->screenLayers[0].bitmapPointer = 1400502;
							} break;
							case 623: {
								currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
								nicheAnimation(1, 1398718);
								currentRoomData->screenLayers[0].bitmapPointer = 1398718;
							} break;
							}
							currentRoomData->screenLayers[0].bitmapSize = 892;
							currentRoomData->screenLayers[0].coordx = 217;
							currentRoomData->screenLayers[0].coordy = 48;
							currentRoomData->screenLayers[0].depth = 1;
							g_engine->_mouseManager->show();
							updateAltScreen(20);
						}
					} else {
						drawText(Random(11) + 1022);
					}
				} break;
				}
			} break;
			case 583: {

				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][20] : flcOffsets[1][20];

				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				drawFlc(140, 34, offset, 0, 9, 24, false, false, true, foobar);
				g_engine->_mouseManager->show();
				updateItem(regobj.code);
				currentRoomData->screenObjectIndex[7]->fileIndex = 716;
				currentRoomData->mouseGrid[19][9] = 14;
				currentRoomData->mouseGrid[22][16] = 15;
				for (listIndex = 21; listIndex <= 22; listIndex++)
					for (invIndex = 17; invIndex <= 20; invIndex++)
						currentRoomData->mouseGrid[listIndex][invIndex] = 17;
				{
					RoomBitmapRegister &with = currentRoomData->screenLayers[0];

					with.bitmapPointer = 1243652;
					with.bitmapSize = 2718;
					with.coordx = 127;
					with.coordy = 36;
					with.depth = 6;
				}
				{
					RoomBitmapRegister &with = currentRoomData->screenLayers[1];

					with.bitmapPointer = 1240474;
					with.bitmapSize = 344;
					with.coordx = 168;
					with.coordy = 83;
					with.depth = 12;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				{
					RoomBitmapRegister &with = currentRoomData->screenLayers[2];

					with.bitmapPointer = 1240818;
					with.bitmapSize = 116;
					with.coordx = 177;
					with.coordy = 82;
					with.depth = 1;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				readBitmap(1243652, screenLayers[5], 2718, 319);
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
			} break;
			case 594: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				animateGive(3, 2);
				animatePickup2(3, 2);
				replaceBackpack(usedObjectIndex, 607);
				drawBackpack();
				g_engine->_mouseManager->show();
			} break;
			case 608: {
				drawText(regobj.useTextRef);
				goToObject(currentRoomData->walkAreasGrid[mouseX][mouseY], 26);
				g_engine->_mouseManager->hide();
				animateGive(2, 2);
				animateOpen2(2, 2);
				{
					RoomBitmapRegister &with = currentRoomData->screenLayers[3];

					with.bitmapPointer = 1546096;
					with.bitmapSize = 372;
					with.coordx = 208;
					with.coordy = 105;
					with.depth = 4;
					loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
				}
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				updateInventory(usedObjectIndex);
				drawBackpack();
				isTrapSet = true;
				g_engine->_mouseManager->show();
			} break;
			case 632: {
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][21] : flcOffsets[1][21];
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				animateGive(charFacingDirection, 1);

				// Show feather on pedestal
				loadItem(187, 70, 104, 1545820, 8);
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				g_engine->_screen->update();

				animateOpen2(charFacingDirection, 1);
				g_engine->_mouseManager->show();
				goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], 14);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("PUFF", 191183, 18001);
				// Animate to scythe
				debug("Start anim!");
				drawFlc(180, 60, offset, 0, 9, 0, false, false, true, foobar);
				debug("End Anim!");
				// load Scythe
				currentRoomData->screenLayers[2].bitmapPointer = 1545820;
				currentRoomData->screenLayers[2].bitmapSize = 104;
				currentRoomData->screenLayers[2].coordx = 277;
				currentRoomData->screenLayers[2].coordy = 104;
				currentRoomData->screenLayers[2].depth = 1;
				depthMap[0].posy = 104;
				readBitmap(1545820, screenLayers[0], 104, 319);

				currentRoomData->screenLayers[4].bitmapPointer = 1447508;
				currentRoomData->screenLayers[4].bitmapSize = 464;
				currentRoomData->screenLayers[4].coordx = 186;
				currentRoomData->screenLayers[4].coordy = 64;
				currentRoomData->screenLayers[4].depth = 8;
				loadItem(186, 63, 464, 1447508, 8);

				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				g_engine->_mouseManager->show();
				goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], 18);
				g_engine->_mouseManager->hide();
				animatePickup1(1, 1);
				replaceBackpack(usedObjectIndex, 638);
				currentRoomData->screenLayers[4].bitmapPointer = 0;
				currentRoomData->screenLayers[4].bitmapSize = 0;
				currentRoomData->screenLayers[4].coordx = 0;
				currentRoomData->screenLayers[4].coordy = 0;
				currentRoomData->screenLayers[4].depth = 0;
				screenLayers[7] = NULL;
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animatePickup2(1, 1);
				drawBackpack();
				g_engine->_mouseManager->show();
				for (listIndex = 35; listIndex <= 37; listIndex++)
					for (invIndex = 21; invIndex <= 25; invIndex++)
						currentRoomData->mouseGrid[listIndex][invIndex] = 11;
				isScytheTaken = true;
				if (isTridentTaken)
					caves[3] = true;
			} break;
			case 633: { //Use ring!
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][22] : flcOffsets[1][22];

				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				animateGive(3, 1);
				loadItem(86, 55, 92, 1591272, 8);
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animateOpen2(3, 1);
				g_engine->_mouseManager->show();
				goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], 10);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("PUFF", 191183, 18001);
				drawFlc(0, 47, offset, 0, 9, 0, false, false, true, foobar);

				currentRoomData->screenLayers[3].bitmapPointer = 1591272;
				currentRoomData->screenLayers[3].bitmapSize = 92;
				currentRoomData->screenLayers[3].coordx = 18;
				currentRoomData->screenLayers[3].coordy = 60;
				currentRoomData->screenLayers[3].depth = 3;
				depthMap[2].posx = 18;
				depthMap[2].posy = 60;
				readBitmap(1591272, screenLayers[2], 92, 319);

				currentRoomData->screenLayers[4].bitmapPointer = 1746554;
				currentRoomData->screenLayers[4].bitmapSize = 384;
				currentRoomData->screenLayers[4].coordx = 82;
				currentRoomData->screenLayers[4].coordy = 53;
				currentRoomData->screenLayers[4].depth = 8;
				loadItem(82, 53, 384, 1746554, 8);
				assembleScreen();
				drawScreen(sceneBackground);
				g_engine->_mouseManager->show();
				goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], 15);
				g_engine->_mouseManager->hide();
				animatePickup1(3, 1);
				replaceBackpack(usedObjectIndex, 637);
				currentRoomData->screenLayers[4].bitmapPointer = 0;
				currentRoomData->screenLayers[4].bitmapSize = 0;
				currentRoomData->screenLayers[4].coordx = 0;
				currentRoomData->screenLayers[4].coordy = 0;
				currentRoomData->screenLayers[4].depth = 0;
				screenLayers[7] = NULL;
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animatePickup2(3, 1);
				drawBackpack();
				g_engine->_mouseManager->show();
				for (listIndex = 0; listIndex <= 2; listIndex++)
					for (invIndex = 10; invIndex <= 12; invIndex++)
						currentRoomData->mouseGrid[listIndex][invIndex] = 10;
				isTridentTaken = true;
				if (isScytheTaken)
					caves[3] = true;
			} break;
			case 643: { // Urn with altar
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][23] : flcOffsets[1][23];

				if (currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount] != 5)
					drawText(regobj.useTextRef);
				mouseClickX = 149 - 7;
				mouseClickY = 126 - 7;
				goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], 5);
				g_engine->_mouseManager->hide();
				updateInventory(usedObjectIndex);
				drawBackpack();
				drawFlc(133, 0, offset, 0, 9, 22, false, false, true, foobar);
				{
					RoomBitmapRegister &with = currentRoomData->screenLayers[2];

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
						currentRoomData->mouseGrid[listIndex][invIndex] = 12;
				isSealRemoved = true;
				caves[1] = false;
				caves[0] = false;
				caves[4] = false;
				g_engine->_mouseManager->show();
			} break;
			case 657: { // sharpen scythe
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][24] : flcOffsets[1][24];

				drawText(regobj.useTextRef);
				mouseClickX = 178 - 7;
				mouseClickY = 71 - 7;
				goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], 3);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("AFILAR", 0, 6433);
				drawFlc(160, 15, offset, 0, 9, 23, false, false, true, foobar);
				replaceBackpack(usedObjectIndex, 715);
				drawBackpack();
				g_engine->_mouseManager->show();
			} break;
			case 686: {
				drawText(regobj.useTextRef);
				isVasePlaced = true;
				caves[4] = false;
				g_engine->_mouseManager->hide();
				animateGive(1, 1);
				updateInventory(usedObjectIndex);
				dropObjectInScreen(regobj);
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				drawBackpack();
				animateOpen2(1, 1);
				for (listIndex = 19; listIndex <= 21; listIndex++)
					for (invIndex = 10; invIndex <= 13; invIndex++)
						currentRoomData->mouseGrid[listIndex][invIndex] = 13;
				g_engine->_mouseManager->show();
			} break;
			case 689: { // rope
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][25] : flcOffsets[1][25];

				drawText(regobj.useTextRef);
				mouseClickX = 124 - 7;
				mouseClickY = 133 - 7;
				goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], 9);
				g_engine->_mouseManager->hide();
				drawFlc(110, 79, offset, 0, 9, 0, false, false, true, foobar);
				replaceBackpack(usedObjectIndex, 701);
				drawBackpack();
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				g_engine->_mouseManager->show();
				for (listIndex = 18; listIndex <= 20; listIndex++)
					currentRoomData->mouseGrid[listIndex][26] = 10;
				for (listIndex = 17; listIndex <= 21; listIndex++)
					currentRoomData->mouseGrid[listIndex][27] = 10;
			} break;
			case 700: { // Trident
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][26] : flcOffsets[1][26];
				drawText(regobj.useTextRef);
				mouseClickX = 224 - 7;
				mouseClickY = 91 - 7;
				goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], 7);
				g_engine->_mouseManager->hide();
				drawFlc(208, 0, offset, 0, 9, 21, false, false, true, foobar);
				currentRoomData->screenLayers[0].bitmapPointer = 0;
				currentRoomData->screenLayers[0].bitmapSize = 0;
				currentRoomData->screenLayers[0].coordx = 0;
				currentRoomData->screenLayers[0].coordy = 0;
				currentRoomData->screenLayers[0].depth = 0;
				screenLayers[2] = NULL;
				for (invIndex = 6; invIndex <= 9; invIndex++)
					currentRoomData->mouseGrid[26][invIndex] = 3;
				for (invIndex = 3; invIndex <= 5; invIndex++)
					currentRoomData->mouseGrid[27][invIndex] = 3;
				for (invIndex = 6; invIndex <= 10; invIndex++)
					currentRoomData->mouseGrid[27][invIndex] = 4;
				for (invIndex = 11; invIndex <= 12; invIndex++)
					currentRoomData->mouseGrid[27][invIndex] = 7;
				for (invIndex = 2; invIndex <= 10; invIndex++)
					currentRoomData->mouseGrid[28][invIndex] = 4;
				for (invIndex = 11; invIndex <= 12; invIndex++)
					currentRoomData->mouseGrid[28][invIndex] = 7;
				currentRoomData->mouseGrid[28][13] = 4;
				for (invIndex = 1; invIndex <= 14; invIndex++)
					currentRoomData->mouseGrid[29][invIndex] = 4;
				for (listIndex = 30; listIndex <= 32; listIndex++)
					for (invIndex = 0; invIndex <= 15; invIndex++)
						currentRoomData->mouseGrid[listIndex][invIndex] = 4;
				for (invIndex = 1; invIndex <= 14; invIndex++)
					currentRoomData->mouseGrid[33][invIndex] = 4;
				for (invIndex = 2; invIndex <= 14; invIndex++)
					currentRoomData->mouseGrid[34][invIndex] = 4;
				for (invIndex = 3; invIndex <= 8; invIndex++)
					currentRoomData->mouseGrid[35][invIndex] = 4;
				for (invIndex = 9; invIndex <= 11; invIndex++)
					currentRoomData->mouseGrid[35][invIndex] = 7;
				currentRoomData->doors[1].openclosed = 1;
				g_engine->_mouseManager->show();
				updateItem(regobj.code);
			} break;
			case 709: { // rock with mural
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][27] : flcOffsets[1][27];

				if (isSealRemoved) {
					drawText(regobj.useTextRef);
					g_engine->_mouseManager->hide();
					animatePickup1(0, 1);
					g_engine->_sound->playVoc("TIZA", 390631, 18774);
					{
						RoomBitmapRegister &with = currentRoomData->screenLayers[1];

						with.bitmapPointer = 1745054;
						with.bitmapSize = 1500;
						with.coordx = 39;
						with.coordy = 16;
						with.depth = 1;
						loadItem(with.coordx, with.coordy, with.bitmapSize, with.bitmapPointer, with.depth);
					}
					restoreBackground();
					assembleScreen();
					drawScreen(sceneBackground);

					g_engine->_sound->waitForSoundEnd();
					g_engine->_sound->playVoc("PUFF", 191183, 18001);
					animateOpen2(0, 1);
					drawFlc(180, 50, offset, 0, 9, 22, false, false, true, foobar);
					shouldQuitGame = true;
				} else
					drawText(Random(11) + 1022);
			} break;
			}
		} else {
			goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], currentRoomData->walkAreasGrid[mouseX][mouseY]);
			if (regobj.code == 536 || regobj.code == 220)
				drawText(Random(6) + 1033);
			else
				drawText(Random(11) + 1022);
		}
	} else { //use object with something on the scene
		if (sceneObject > 0) {
			readItemRegister(sceneObject);
			goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], currentRoomData->walkAreasGrid[mouseX][mouseY]);
			switch (regobj.used[0]) {
			case 0: {
				if (regobj.useTextRef > 0)
					drawText(regobj.useTextRef);
			} break;
			case 9: {
				if (regobj.afterUseTextRef > 0)
					drawText(regobj.afterUseTextRef);
			} break;
			case 100: {
				switch (regobj.code) {
				case 153: {
					g_engine->_mouseManager->hide();
					animatedSequence(2);
					g_engine->_mouseManager->show();
				} break;
				case 154: {
					regobj.used[0] = 9;
					if (regobj.beforeUseTextRef > 0)
						drawText(regobj.beforeUseTextRef);
					g_engine->_mouseManager->hide();
					animatedSequence(1);
					g_engine->_mouseManager->show();
					drawText(1425);
					g_engine->_mouseManager->hide();
					animatedSequence(3);
					g_engine->_mouseManager->show();
					updateItem(regobj.code);
					readItemRegister(152);
					usedObjectIndex = 0;
					while (mobj[usedObjectIndex].code != 0) {
						usedObjectIndex += 1;
					}
					mobj[usedObjectIndex].bitmapIndex = regobj.objectIconBitmap;
					mobj[usedObjectIndex].code = regobj.code;
					mobj[usedObjectIndex].objectName = regobj.name;
					g_engine->_mouseManager->hide();
					drawBackpack();
					g_engine->_mouseManager->show();
				} break;
				case 169: {
					g_engine->_mouseManager->hide();
					animatePickup1(0, 1);
					animateOpen2(0, 1);
					isTVOn = !(isTVOn);
					if (isTVOn) {
						g_engine->_sound->playVoc("CLICK", 27742, 2458);
						g_engine->_sound->waitForSoundEnd();
						currentRoomData->paletteAnimationFlag = true;
						g_engine->_sound->autoPlayVoc("PARASITO", 355778, 20129);
					} else {
						g_engine->_sound->stopVoc();
						g_engine->_sound->playVoc("CLICK", 27742, 2458);
						currentRoomData->paletteAnimationFlag = false;
						for (int i = 195; i <= 200; i++) {
							pal[i * 3 + 0] = 2 << 2;
							pal[i * 3 + 1] = 2 << 2;
							pal[i * 3 + 2] = 2 << 2;
							setRGBPalette(i, 2, 2, 2);
						}
					}
					g_engine->_mouseManager->show();
				} break;
				case 347: {
					drawText(regobj.useTextRef);
					g_engine->_sound->stopVoc();
					g_engine->_sound->playVoc("CLICK", 27742, 2458);
					g_engine->_mouseManager->hide();
					animatePickup1(0, 0);
					delay(100);
					animateOpen2(0, 0);
					g_engine->_sound->stopVoc();
					g_engine->_sound->autoPlayVoc("CALDERA", 6433, 15386);
					turnLightOn();
					g_engine->_mouseManager->show();
					currentRoomData->palettePointer = 1536;
					currentRoomData->screenObjectIndex[1]->fileIndex = 424;
					currentRoomData->doors[1].openclosed = 1;
				} break;
				case 359: {
					drawText(regobj.useTextRef);
					g_engine->_sound->stopVoc();
					g_engine->_sound->playVoc("CARBON", 21819, 5923);
					g_engine->_mouseManager->hide();
					animatePickup1(0, 0);
					delay(100);
					animateOpen2(0, 0);
					g_engine->_mouseManager->show();
					updateItem(regobj.code);
					currentRoomData->screenObjectIndex[16]->fileIndex = 362;
					currentRoomData->screenObjectIndex[16]->objectName = getObjectName(2);
					currentRoomData->screenObjectIndex[1]->fileIndex = 347;
					currentRoomData->screenObjectIndex[1]->objectName = getObjectName(3);
					g_engine->_sound->stopVoc();
					g_engine->_sound->autoPlayVoc("CALDERA", 6433, 15386);
				} break;
				case 682: {
					long offsetWithJar = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][28] : flcOffsets[1][28];
					long offsetNoJar = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][29] : flcOffsets[1][29];

					g_engine->_mouseManager->hide();
					g_engine->_sound->playVoc("CLICK", 27742, 2458);
					animatePickup1(0, 1);

					g_engine->_sound->waitForSoundEnd();
					animateOpen2(0, 1);
					g_engine->_sound->playVoc("FUEGO", 72598, 9789);
					if (isVasePlaced) {
						drawFlc(108, 0, offsetWithJar, 0, 9, 0, false, false, true, foobar);
						{
							RoomBitmapRegister &with = currentRoomData->screenLayers[0];

							with.bitmapPointer = 1636796;
							with.bitmapSize = 628;
							with.coordx = 153;
							with.coordy = 48;
							with.depth = 1;
						}
						for (listIndex = 19; listIndex <= 21; listIndex++)
							for (invIndex = 10; invIndex <= 13; invIndex++)
								currentRoomData->mouseGrid[listIndex][invIndex] = 12;
					} else
						drawFlc(108, 0, offsetNoJar, 0, 9, 0, false, false, true, foobar);
					g_engine->_mouseManager->show();
				} break;
				}
				if ((regobj.beforeUseTextRef > 0) && (regobj.code != 154))
					drawText(regobj.beforeUseTextRef);
			} break;
			default:
				drawText(1022 + Random(11));
			}
		}
	}
	oldGridX = 0;
	oldGridY = 0;
	actionCode = 0;
	checkMouseGrid();
}

void openScreenObject() {
	byte xIndex, yIndex;
	bool shouldSpeak; //Whether the character should speak to reject the action or not

	uint mouseX = (mouseClickX + 7) / xGridCount;
	uint mouseY = (mouseClickY + 7) / yGridCount;
	uint screenObject = currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex;
	if (screenObject == 0)
		return;

	readItemRegister(screenObject);
	debug("Read screen object = %s, with code = %d, depth=%d", regobj.name.c_str(), regobj.code, regobj.depth);
	goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount],
			   currentRoomData->walkAreasGrid[mouseX][mouseY]);

	if (regobj.openable == false) {
		drawText(Random(9) + 1059);
		return;
	} else {
		shouldSpeak = false;
		switch (regobj.code) {
		case 227:
			if (isCupboardOpen == false)
				shouldSpeak = true;
			break;
		case 274:
			if (isChestOpen == false)
				shouldSpeak = true;
			break;
		case 415:
			if (currentRoomData->doors[2].openclosed == 2)
				shouldSpeak = true;
			else {
				g_engine->_mouseManager->hide();
				animatePickup1(0, 1);
				screenLayers[regobj.depth - 1] = NULL;
				yIndex = 0;
				while (currentRoomData->screenLayers[yIndex].depth != regobj.depth && yIndex != 15) {
					yIndex++;
				}
				debug("changing bitmap at %d, with depth = %d", yIndex, currentRoomData->screenLayers[yIndex].depth);
				currentRoomData->screenLayers[yIndex].bitmapPointer = 0;
				currentRoomData->screenLayers[yIndex].bitmapSize = 0;
				currentRoomData->screenLayers[yIndex].coordx = 0;
				currentRoomData->screenLayers[yIndex].coordy = 0;
				currentRoomData->screenLayers[yIndex].depth = 0;
				currentRoomData->doors[2].openclosed = 1;
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animateOpen2(0, 1);
				g_engine->_mouseManager->show();
				for (yIndex = 0; yIndex <= 12; yIndex++)
					for (xIndex = 33; xIndex <= 36; xIndex++)
						currentRoomData->mouseGrid[xIndex][yIndex] = 43;
				for (xIndex = 33; xIndex <= 35; xIndex++)
					currentRoomData->mouseGrid[xIndex][13] = 43;
				actionCode = 0;
				oldGridX = 0;
				oldGridY = 0;
				oldTargetZone = 0;
				checkMouseGrid();
				return;
			}
			break;
		case 548:
			if (currentRoomData->doors[0].openclosed == 2)
				shouldSpeak = true;
			else {
				g_engine->_mouseManager->hide();
				animatePickup1(1, 1);
				screenLayers[regobj.depth - 1] = NULL;
				yIndex = 0;
				while (currentRoomData->screenLayers[yIndex].depth != regobj.depth && yIndex != 14) {
					yIndex++;
				}
				currentRoomData->screenLayers[yIndex].bitmapPointer = 0;
				currentRoomData->screenLayers[yIndex].bitmapSize = 0;
				currentRoomData->screenLayers[yIndex].coordx = 0;
				currentRoomData->screenLayers[yIndex].coordy = 0;
				currentRoomData->screenLayers[yIndex].depth = 0;
				currentRoomData->doors[0].openclosed = 1;
				restoreBackground();
				assembleScreen();
				drawScreen(sceneBackground);
				animateOpen2(1, 1);
				g_engine->_mouseManager->show();
				xIndex = 30;
				for (yIndex = 17; yIndex <= 18; yIndex++)
					currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				xIndex += 1;
				for (yIndex = 4; yIndex <= 20; yIndex++)
					currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				xIndex += 1;
				for (yIndex = 0; yIndex <= 20; yIndex++)
					currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				xIndex += 1;
				for (yIndex = 0; yIndex <= 17; yIndex++)
					currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				xIndex += 1;
				for (yIndex = 0; yIndex <= 12; yIndex++)
					currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				for (xIndex = 35; xIndex <= 39; xIndex++)
					for (yIndex = 0; yIndex <= 10; yIndex++)
						currentRoomData->mouseGrid[xIndex][yIndex] = 8;
				actionCode = 0;
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
		currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex = regobj.replaceWith;
		g_engine->_mouseManager->hide();
		switch (regobj.height) {
		case 0: {
			animatePickup1(charFacingDirection, 0);
			updateVideo();
			animateOpen2(charFacingDirection, 0);
		} break;
		case 1: {
			animatePickup1(charFacingDirection, 1);
			updateVideo();
			animateOpen2(charFacingDirection, 1);
		} break;
		case 2: {
			animatePickup1(charFacingDirection, 2);
			updateVideo();
			animateOpen2(charFacingDirection, 2);
		} break;
		}
		g_engine->_mouseManager->show();
		for (yIndex = regobj.yrej1; yIndex <= regobj.yrej2; yIndex++)
			for (xIndex = regobj.xrej1; xIndex <= regobj.xrej2; xIndex++) {
				currentRoomData->walkAreasGrid[xIndex][yIndex] = regobj.walkAreasPatch[xIndex - regobj.xrej1][yIndex - regobj.yrej1];
				currentRoomData->mouseGrid[xIndex][yIndex] = regobj.mouseGridPatch[xIndex - regobj.xrej1][yIndex - regobj.yrej1];
			}
		for (xIndex = 0; xIndex < 15; xIndex++)
			if (currentRoomData->screenLayers[xIndex].bitmapPointer == regobj.bitmapPointer) {
				currentRoomData->screenLayers[xIndex].bitmapPointer = regobj.dropOverlay;
				currentRoomData->screenLayers[xIndex].bitmapSize = regobj.dropOverlaySize;
			}
		actionCode = 0;
	}
	oldGridX = 0;
	oldGridY = 0;
	checkMouseGrid();
}

void closeScreenObject() {
	byte xIndex, yIndex;
	bool shouldSpeak;
	uint sceneObject;

	byte mouseX = (mouseClickX + 7) / xGridCount;
	byte mouseY = (mouseClickY + 7) / yGridCount;
	sceneObject = currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex;
	if (sceneObject == 0)
		return;
	// verifyCopyProtection2();
	readItemRegister(sceneObject);
	goToObject(currentRoomData->walkAreasGrid[((characterPosX + characterCorrectionX) / xGridCount)][((characterPosY + characerCorrectionY) / yGridCount)],
			   currentRoomData->walkAreasGrid[mouseX][mouseY]);
	if (regobj.closeable == false) {
		drawText((Random(10) + 1068));
		return;
	} else {
		shouldSpeak = false;
		switch (regobj.code) {
		case 224:
		case 226:
			if (isCupboardOpen == false)
				shouldSpeak = true;
			break;
		case 275:
		case 277:
			if (isChestOpen == false)
				shouldSpeak = true;
			break;
		}
		if (shouldSpeak) {
			drawText(Random(10) + 1068);
			return;
		}
		currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[mouseX][mouseY]]->fileIndex = regobj.replaceWith;
		g_engine->_mouseManager->hide();
		switch (regobj.height) {
		case 0: {
			animatePickup1(charFacingDirection, 0);
			updateVideo();
			animateOpen2(charFacingDirection, 0);
		} break;
		case 1: {
			animatePickup1(charFacingDirection, 1);
			updateVideo();
			animateOpen2(charFacingDirection, 1);
		} break;
		case 2: {
			animatePickup1(charFacingDirection, 2);
			updateVideo();
			animateOpen2(charFacingDirection, 2);
		} break;
		}
		g_engine->_mouseManager->show();
		for (yIndex = regobj.yrej1; yIndex <= regobj.yrej2; yIndex++)
			for (xIndex = regobj.xrej1; xIndex <= regobj.xrej2; xIndex++) {
				currentRoomData->walkAreasGrid[xIndex][yIndex] = regobj.walkAreasPatch[xIndex - regobj.xrej1][yIndex - regobj.yrej1];
				currentRoomData->mouseGrid[xIndex][yIndex] = regobj.mouseGridPatch[xIndex - regobj.xrej1][yIndex - regobj.yrej1];
			}
		for (xIndex = 0; xIndex < 15; xIndex++)
			if (currentRoomData->screenLayers[xIndex].bitmapPointer == regobj.bitmapPointer) {
				currentRoomData->screenLayers[xIndex].bitmapPointer = regobj.dropOverlay;
				currentRoomData->screenLayers[xIndex].bitmapSize = regobj.dropOverlaySize;
			}
		actionCode = 0;
	}
	oldGridX = 0;
	oldGridY = 0;
	checkMouseGrid();
}

void action() {
	bar(0, 140, 319, 149, 0);
	Common::String actionLine;
	switch (actionCode) {
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
		inventoryObjectName = "";
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

void handleAction(byte posinv) {

	bar(0, 140, 319, 149, 0);
	switch (actionCode) {
	case 1: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(1) + mobj[posinv].objectName);
		g_engine->_mouseManager->show();
		drawText((Random(10) + 1039));
		actionCode = 0;
		if (cpCounter > 130)
			showError(274);
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 2: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(2) + mobj[posinv].objectName);
		if (cpCounter2 > 13)
			showError(274);
		g_engine->_mouseManager->show();
		drawText((Random(10) + 1049));
		actionCode = 0;
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 3: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(3) + mobj[posinv].objectName);
		g_engine->_mouseManager->show();
		actionCode = 0;
		lookAtObject(posinv);
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 4:
		if (inventoryObjectName == "") {
			g_engine->_mouseManager->hide();
			actionLineText(getActionLineText(4) + mobj[posinv].objectName + getActionLineText(7));
			g_engine->_mouseManager->show();
			inventoryObjectName = mobj[posinv].objectName;
			backpackObjectCode = mobj[posinv].code;
		} else {

			actionCode = 0;
			if (cpCounter > 25)
				showError(274);
			useInventoryObjectWithInventoryObject(backpackObjectCode, mobj[posinv].code);
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		}
		break;
	case 5: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(5) + mobj[posinv].objectName);
		g_engine->_mouseManager->show();
		drawText(Random(9) + 1059);
		actionCode = 0;
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 6: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(6) + mobj[posinv].objectName);
		g_engine->_mouseManager->show();
		drawText(Random(10) + 1068);
		actionCode = 0;
		if (cpCounter2 > 35)
			showError(274);
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	}
}

void loadObjects() {

	Common::File objectFile;
	switch (gamePart) {
	case 1:
		objectFile.open("OBJMOCH.DAT");
		break;
	case 2:
		objectFile.open("OBJMOCH.TWO");
		break;
	}

	if (!objectFile.isOpen())
		showError(312);
	for (int i = 0; i < inventoryIconCount; i++) {
		inventoryIconBitmaps[i] = (byte *)malloc(inventoryIconSize);
		objectFile.read(inventoryIconBitmaps[i], inventoryIconSize);
	}
	if (cpCounter > 65)
		showError(274);
	const char *emptyName = (g_engine->_lang == Common::ES_ESP) ? hardcodedObjects_ES[10] : hardcodedObjects_EN[10];
	for (int i = 0; i < inventoryIconCount; i++) {
		mobj[i].bitmapIndex = 34;
		mobj[i].code = 0;
		mobj[i].objectName = emptyName;
	}

	objectFile.close();
	debug("Successfully read objects!");
}

void obtainName(Common::String &playerName) {
	uint16 namePromptBGSize;
	byte *namePromptBG;

	namePromptBGSize = imagesize(84, 34, 235, 80);
	namePromptBG = (byte *)malloc(namePromptBGSize);
	g_engine->_graphics->getImg(84, 34, 235, 80, namePromptBG);
	drawMenu(8);
	g_engine->_screen->update();
	Common::String prompt;
	readAlphaGraph(prompt, 8, 125, 62, 252);
	playerName = prompt.c_str();
	g_engine->_graphics->putImg(84, 34, namePromptBG);
	g_engine->_screen->update();
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

static void scrollRight(uint &horizontalPos) {

	int characterPos = 25 + (320 - (characterPosX + characterCorrectionX * 2));
	// We scroll 4 by 4 pixels so we divide by 4 to find out the number of necessary steps
	uint stepCount = (320 - horizontalPos) >> 2;
	byte *assembledCharacterFrame = (byte *)malloc(mainCharFrameSize);
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
			if (characterPos > 0 && iframe < 15) {
				iframe++;
			} else
				iframe = 0;

			characterPosX -= 2;

			curCharacterAnimationFrame = mainCharAnimation.bitmap[1][iframe];
			// We need to copy the original frame as to not replace its black background for after
			// the scroll ends. Original code would copy from XMS memory.
			Common::copy(curCharacterAnimationFrame, curCharacterAnimationFrame + mainCharFrameSize, assembledCharacterFrame);

			// puts the original captured background back in the background for next iteration
			g_engine->_graphics->putImageArea(characterPosX - 2, characterPosY, sceneBackground, spriteBackground);
			uint16 pasoframeW = READ_LE_UINT16(assembledCharacterFrame);
			uint16 pasoframeH = READ_LE_UINT16(assembledCharacterFrame + 2);
			// Grabs current area surrounding character (which might contain parts of A and B)
			g_engine->_graphics->getImageArea(characterPosX, characterPosY, characterPosX + pasoframeW, characterPosY + pasoframeH, sceneBackground, spriteBackground);
			// blits over the character sprite, only on black pixels
			blit(spriteBackground, assembledCharacterFrame);
			// puts it back in the background (character + piece of background)
			g_engine->_graphics->putImageArea(characterPosX, characterPosY, sceneBackground, assembledCharacterFrame);
		} else
			characterPosX -= 4;
		g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		g_engine->_screen->update();
		drawScreen(sceneBackground);
	}
	free(assembledCharacterFrame);
}

static void scrollLeft(uint &horizontalPos) {

	int characterPos = 25 + characterPosX;
	horizontalPos = 320 - horizontalPos;
	// We scroll 4 by 4 pixels so we divide by 4 to find out the number of necessary steps
	uint stepCount = horizontalPos >> 2;

	byte *assembledCharacterFrame = (byte *)malloc(mainCharFrameSize);
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
			if (characterPos > 0 && iframe < 15)
				iframe++;
			else
				iframe = 0;

			characterPosX += 2;

			curCharacterAnimationFrame = mainCharAnimation.bitmap[3][iframe];
			Common::copy(curCharacterAnimationFrame, curCharacterAnimationFrame + mainCharFrameSize, assembledCharacterFrame);

			g_engine->_graphics->putImageArea(characterPosX + 2, characterPosY, sceneBackground, spriteBackground);

			uint16 pasoframeW = READ_LE_UINT16(assembledCharacterFrame);
			uint16 pasoframeH = READ_LE_UINT16(assembledCharacterFrame + 2);

			g_engine->_graphics->getImageArea(characterPosX, characterPosY, characterPosX + pasoframeW, characterPosY + pasoframeH, sceneBackground, spriteBackground);
			blit(spriteBackground, assembledCharacterFrame);
			g_engine->_graphics->putImageArea(characterPosX, characterPosY, sceneBackground, assembledCharacterFrame);
		} else
			characterPosX += 4;

		g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		g_engine->_screen->update();
		drawScreen(sceneBackground);
	}
	free(assembledCharacterFrame);
}

/**
 * Scrolling happens between two screens. We grab the area surroudning the player from screen A,
 * then transition to screen B.
 */
void loadScrollData(uint roomCode, bool rightScroll, uint horizontalPos, int scrollCorrection) {

	restoreBackground();

	// background now contains background A, backgroundCopy contains background A
	uint characterFrameW = READ_LE_UINT16(curCharacterAnimationFrame);
	uint characterFrameH = READ_LE_UINT16(curCharacterAnimationFrame + 2);
	debug("characterPos=%d,%d, size=%d,%d", characterPosX, characterPosY, characterFrameW, characterFrameH);
	/* Copy the area with the player from previous scren*/
	spriteBackground = (byte *)malloc(4 + (characterFrameW + 8) * (characterFrameH + 8));
	g_engine->_graphics->getImageArea(characterPosX, characterPosY, characterPosX + characterFrameW, characterPosY + characterFrameH, sceneBackground, spriteBackground);

	// Start screen 2

	rooms->seek(roomCode * roomRegSize, SEEK_SET);
	currentRoomData = readScreenDataFile(rooms);

	loadScreen();
	// Background now contains background B, backgroundCopy contains background B
	for (int i = 0; i < 15; i++) {
		{
			RoomBitmapRegister &with = currentRoomData->screenLayers[i];
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

	isPaletteAnimEnabled = 0;
	getScreen(sceneBackground);
	// background now contains full background A again, backgroundCopy contains background B + objects

	drawScreen(sceneBackground);
	if (rightScroll)
		scrollRight(horizontalPos);
	else
		scrollLeft(horizontalPos);

	// After scroll is done, backgroundCopy will now contain the resulting background (background B + objects)
	Common::copy(backgroundCopy, backgroundCopy + 44804, sceneBackground);

	characterPosX += scrollCorrection;

	assembleScreen();
	drawScreen(sceneBackground);
	free(spriteBackground);
	loadScreen();
	trajectory[currentTrajectoryIndex].x = characterPosX;
	trajectory[currentTrajectoryIndex].y = characterPosY;
}

void saveGameToRegister() {
	savedGame.roomCode = currentRoomData->code;
	savedGame.trajectoryLength = trajectoryLength;
	savedGame.currentTrajectoryIndex = currentTrajectoryIndex;
	savedGame.backpackObjectCode = backpackObjectCode;
	savedGame.rightSfxVol = rightSfxVol;
	savedGame.leftSfxVol = leftSfxVol;
	savedGame.musicVolRight = musicVolRight;
	savedGame.musicVolLeft = musicVolLeft;
	savedGame.oldGridX = oldGridX;
	savedGame.oldGridY = oldGridY;
	savedGame.secAnimDepth = secondaryAnimation.depth;
	savedGame.secAnimDir = secondaryAnimation.dir;
	savedGame.secAnimX = secondaryAnimation.posx;
	savedGame.secAnimY = secondaryAnimation.posy;
	savedGame.secAnimIFrame = iframe2;

	savedGame.currentZone = currentZone;
	savedGame.targetZone = targetZone;
	savedGame.oldTargetZone = oldTargetZone;
	savedGame.inventoryPosition = inventoryPosition;
	savedGame.actionCode = actionCode;
	savedGame.oldActionCode = oldActionCode;
	savedGame.steps = steps;
	savedGame.doorIndex = doorIndex;
	savedGame.characterFacingDir = charFacingDirection;
	savedGame.iframe = iframe;
	savedGame.gamePart = gamePart;

	savedGame.isSealRemoved = isSealRemoved;
	savedGame.obtainedList1 = obtainedList1;
	savedGame.obtainedList2 = obtainedList2;
	savedGame.list1Complete = list1Complete;
	savedGame.list2Complete = list2Complete;
	savedGame.isVasePlaced = isVasePlaced;
	savedGame.isScytheTaken = isScytheTaken;
	savedGame.isTridentTaken = isTridentTaken;
	savedGame.isPottersWheelDelivered = isPottersWheelDelivered;
	savedGame.isMudDelivered = isMudDelivered;
	savedGame.isGreenDevilDelivered = isGreenDevilDelivered;
	savedGame.isRedDevilCaptured = isRedDevilCaptured;
	savedGame.isPottersManualDelivered = isPottersManualDelivered;
	savedGame.isCupboardOpen = isCupboardOpen;
	savedGame.isChestOpen = isChestOpen;
	savedGame.isTVOn = isTVOn;
	savedGame.isTrapSet = isTrapSet;

	for (int i = 0; i < inventoryIconCount; i++) {
		savedGame.mobj[i].bitmapIndex = mobj[i].bitmapIndex;
		savedGame.mobj[i].code = mobj[i].code;
		savedGame.mobj[i].objectName = mobj[i].objectName;
	}

	savedGame.element1 = element1;
	savedGame.element2 = element2;
	savedGame.characterPosX = characterPosX;
	savedGame.characterPosY = characterPosY;
	savedGame.xframe2 = xframe2;
	savedGame.yframe2 = yframe2;

	savedGame.oldInventoryObjectName = oldInventoryObjectName;
	savedGame.objetomoinventoryObjectNamehila = inventoryObjectName;
	savedGame.characterName = characterName;

	for (int i = 0; i < routePointCount; i++) {
		savedGame.mainRoute[i].x = mainRoute[i].x;
		savedGame.mainRoute[i].y = mainRoute[i].y;
	}

	for (int i = 0; i < 300; i++) {
		savedGame.trajectory[i].x = trajectory[i].x;
		savedGame.trajectory[i].y = trajectory[i].y;
	}

	for (int i = 0; i < characterCount; i++) {
		savedGame.firstTimeTopicA[i] = firstTimeTopicA[i];
		savedGame.firstTimeTopicB[i] = firstTimeTopicB[i];
		savedGame.firstTimeTopicC[i] = firstTimeTopicC[i];
		savedGame.bookTopic[i] = bookTopic[i];
		savedGame.mintTopic[i] = mintTopic[i];
	}
	for (int i = 0; i < 5; i++) {
		savedGame.caves[i] = caves[i];
		savedGame.firstList[i] = firstList[i];
		savedGame.secondList[i] = secondList[i];
	}
	for (int i = 0; i < 4; i++) {
		savedGame.niche[0][i] = niche[0][i];
		savedGame.niche[1][i] = niche[1][i];
	}
}

void loadGame(SavedGame game) {
	freeAnimation();
	freeScreenObjects();
	transitionEffect = Random(15) + 1;

	trajectoryLength = game.trajectoryLength;
	currentTrajectoryIndex = game.currentTrajectoryIndex;
	backpackObjectCode = game.backpackObjectCode;
	rightSfxVol = game.rightSfxVol;
	leftSfxVol = game.leftSfxVol;
	musicVolRight = game.musicVolRight;
	musicVolLeft = game.musicVolLeft;
	oldGridX = game.oldGridX;
	oldGridY = game.oldGridY;
	secondaryAnimation.depth = game.secAnimDepth;
	secondaryAnimation.dir = game.secAnimDir;
	secondaryAnimation.posx = game.secAnimX;
	secondaryAnimation.posy = game.secAnimY;
	iframe2 = game.secAnimIFrame;
	currentZone = game.currentZone;
	targetZone = game.targetZone;
	oldTargetZone = game.oldTargetZone;
	inventoryPosition = game.inventoryPosition;
	actionCode = game.actionCode;
	oldActionCode = game.oldActionCode;
	steps = game.steps;
	doorIndex = game.doorIndex;
	charFacingDirection = game.characterFacingDir;
	iframe = game.iframe;
	if (game.gamePart != gamePart) {
		gamePart = game.gamePart;
		for (int i = 0; i < inventoryIconCount; i++) {
			free(inventoryIconBitmaps[i]);
		}
		loadObjects();
	}
	isSealRemoved = game.isSealRemoved;
	obtainedList1 = game.obtainedList1;
	obtainedList2 = game.obtainedList2;
	list1Complete = game.list1Complete;
	list2Complete = game.list2Complete;
	isVasePlaced = game.isVasePlaced;
	isScytheTaken = game.isScytheTaken;
	if (cpCounter > 24)
		showError(274);
	isTridentTaken = game.isTridentTaken;
	isPottersWheelDelivered = game.isPottersWheelDelivered;
	isMudDelivered = game.isMudDelivered;
	isGreenDevilDelivered = game.isGreenDevilDelivered;
	isRedDevilCaptured = game.isRedDevilCaptured;
	isPottersManualDelivered = game.isPottersManualDelivered;
	isCupboardOpen = game.isCupboardOpen;
	isChestOpen = game.isChestOpen;
	isTVOn = game.isTVOn;
	isTrapSet = game.isTrapSet;
	for (int i = 0; i < inventoryIconCount; i++) {
		mobj[i].bitmapIndex = game.mobj[i].bitmapIndex;
		mobj[i].code = game.mobj[i].code;
		mobj[i].objectName = game.mobj[i].objectName;
	}
	element1 = game.element1;
	element2 = game.element2;
	characterPosX = game.characterPosX;
	characterPosY = game.characterPosY;
	xframe2 = game.xframe2;
	yframe2 = game.yframe2;
	oldInventoryObjectName = game.oldInventoryObjectName;
	inventoryObjectName = game.objetomoinventoryObjectNamehila;
	characterName = game.characterName;
	for (int i = 0; i < routePointCount; i++) {
		mainRoute[i].x = game.mainRoute[i].x;
		mainRoute[i].y = game.mainRoute[i].y;
	}
	for (int indiaux = 0; indiaux < 300; indiaux++) {
		trajectory[indiaux].x = game.trajectory[indiaux].x;
		trajectory[indiaux].y = game.trajectory[indiaux].y;
	}
	for (int i = 0; i < characterCount; i++) {
		firstTimeTopicA[i] = game.firstTimeTopicA[i];
		firstTimeTopicB[i] = game.firstTimeTopicB[i];
		firstTimeTopicC[i] = game.firstTimeTopicC[i];
		bookTopic[i] = game.bookTopic[i];
		mintTopic[i] = game.mintTopic[i];
	}
	for (int i = 0; i < 5; i++) {
		caves[i] = game.caves[i];
		firstList[i] = game.firstList[i];
		secondList[i] = game.secondList[i];
	}
	for (int i = 0; i < 4; i++) {
		niche[0][i] = game.niche[0][i];
		niche[1][i] = game.niche[1][i];
	}

	totalFadeOut(0);
	g_engine->_screen->clear();
	loadPalette("DEFAULT");
	loadScreenData(game.roomCode);

	switch (currentRoomData->code) {
	case 2: {
		if (isTVOn)
			g_engine->_sound->autoPlayVoc("PARASITO", 355778, 20129);
		else
			loadTV();
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
	} break;
	case 4: {
		g_engine->_sound->loadVoc("GOTA", 140972, 1029);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
	} break;
	case 5: {
		g_engine->_sound->setSfxVolume(leftSfxVol, 0);
		g_engine->_sound->autoPlayVoc("CALDERA", 6433, 15386);
	} break;
	case 6: {
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		g_engine->_sound->autoPlayVoc("CALDERA", 6433, 15386);
	} break;
	case 17: {
		if (bookTopic[0] == true && currentRoomData->animationFlag)
			disableSecondAnimation();
	} break;
	case 20: {
		switch (niche[0][niche[0][3]]) {
		case 0:
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
			break;
		case 561:
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
			break;
		case 563:
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
			break;
		case 615:
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
			break;
		}
	} break;
	case 23: {
		g_engine->_sound->autoPlayVoc("Fuente", 0, 0);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
	} break;
	case 24: {
		switch (niche[1][niche[1][3]]) {
		case 0:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
			break;
		case 561:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
			break;
		case 615:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
			break;
		case 622:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
			break;
		case 623:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
			break;
		}
		if (isTrapSet) {
			currentRoomData->animationFlag = true;
			loadAnimation(currentRoomData->animationName);
			iframe2 = 0;
			currentSecondaryTrajectoryIndex = 1;
			currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x = 214 - 15;
			currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y = 115 - 42;
			secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
			secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
			secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
			secondaryAnimation.depth = 14;

			for (int i = 0; i < maxXGrid; i++)
				for (int j = 0; j < maxYGrid; j++) {
					if (maskGridSecondaryAnim[i][j] > 0)
						currentRoomData->walkAreasGrid[oldposx + i][oldposy + j] = maskGridSecondaryAnim[i][j];
					if (maskMouseSecondaryAnim[i][j] > 0)
						currentRoomData->mouseGrid[oldposx + i][oldposy + j] = maskMouseSecondaryAnim[i][j];
				}
		}
		assembleScreen();
	} break;
	}

	mask();
	inventoryPosition = 0;
	drawBackpack();
	if (isRedDevilCaptured == false && currentRoomData->code == 24 && isTrapSet == false)
		runaroundRed();
	screenTransition(transitionEffect, false, sceneBackground);
}

/**
 * Loads talking animation of main adn secondary character
 */
void loadTalkAnimations() {
	Common::File animFile;

	if (!animFile.open("TIOHABLA.SEC")) {
		showError(265);
	}
	mainCharFrameSize = animFile.readUint16LE();

	int32 offset = mainCharFrameSize * 16;
	offset = (offset * charFacingDirection) + 2;
	animFile.seek(offset);
	debug("LoadTalk charFacingDirection=%d", charFacingDirection);
	//Will load talking anim always in the upwards direction of the walk cycle array
	for (int i = 0; i < 16; i++) {
		mainCharAnimation.bitmap[0][i] = (byte *)malloc(mainCharFrameSize);
		animFile.read(mainCharAnimation.bitmap[0][i], mainCharFrameSize);
	}
	animFile.close();

	if ((currentRoomData->animationName != "PETER") && (currentRoomData->animationName != "ARZCAEL")) {
		iframe2 = 0;
		free(curSecondaryAnimationFrame);
		bool result;
		switch (regobj.speaking) {
		case 1:
			result = animFile.open("JOHN.SEC");
			break;
		case 5:
			result = animFile.open("ALFRED.SEC");
			break;
		default:
			result = animFile.open(Common::Path(currentRoomData->animationName + Common::String(".SEC")));
		}

		if (!result)
			showError(265);
		secondaryAnimFrameSize = animFile.readUint16LE();
		secondaryAnimationFrameCount = animFile.readByte();
		secondaryAnimDirCount = animFile.readByte();

		curSecondaryAnimationFrame = (byte *)malloc(secondaryAnimFrameSize);
		if (secondaryAnimDirCount != 0) {
			secondaryAnimationFrameCount = secondaryAnimationFrameCount / 4;
			for (int i = 0; i <= 3; i++) {
				loadAnimationForDirection(&animFile, i);
			}
		} else {
			loadAnimationForDirection(&animFile, 0);
		}
		animFile.close();
	}
}

void unloadTalkAnimations() {

	Common::File animFile;
	if (!animFile.open("PERSONAJ.SPT")) {
		showError(265);
	}
	mainCharFrameSize = animFile.readUint16LE();

	for (int i = 0; i < walkFrameCount; i++) {
		mainCharAnimation.bitmap[0][i] = (byte *)malloc(mainCharFrameSize);
		animFile.read(mainCharAnimation.bitmap[0][i], mainCharFrameSize);
	}
	animFile.close();

	if ((currentRoomData->animationName != "PETER") && (currentRoomData->animationName != "ARZCAEL")) {
		if (!animFile.open(Common::Path(currentRoomData->animationName + ".DAT"))) {
			showError(265);
		}
		secondaryAnimFrameSize = animFile.readUint16LE();
		secondaryAnimationFrameCount = animFile.readByte();
		secondaryAnimDirCount = animFile.readByte();
		curSecondaryAnimationFrame = (byte *)malloc(secondaryAnimFrameSize);
		if (secondaryAnimDirCount != 0) {

			secondaryAnimationFrameCount = secondaryAnimationFrameCount / 4;
			for (int i = 0; i <= 3; i++) {
				loadAnimationForDirection(&animFile, i);
			}
		} else {
			loadAnimationForDirection(&animFile, 0);
		}
		animFile.close();
	}
}

TextEntry readVerbRegister(uint numRegister) {
	verb.seek(numRegister * verbRegSize);
	return readVerbRegister();
}

TextEntry readVerbRegister() {
	TextEntry regmht;
	// Since the text is encrypted it's safer to save the size as reported by
	// the pascal string.
	byte size = verb.readByte();
	verb.seek(-1, SEEK_CUR);
	regmht.text = verb.readPascalString(false);
	verb.skip(255 - size);
	regmht.continued = verb.readByte();
	regmht.response = verb.readUint16LE();
	regmht.pointer = verb.readSint32LE();
	return regmht;
}

void hypertext(
	uint textRef,
	byte textColor,
	byte shadowColor,
	uint &responseNumber,
	/** Whether the text being said is part of a conversation or just descriptions */
	bool isWithinConversation) {
	TextEntry text;

	byte insertName, iht, iteracionesht, lineaht, anchoht;
	byte direccionmovimientopaso;

	uint indiceaniconversa, tambackgroundht, xht, yht;

	byte *backgroundtextht;
	byte matrizsaltosht[15];
	g_engine->_mouseManager->hide();
	switch (currentRoomData->code) {
	case 2: { // Leisure room
		xht = 10;
		yht = 2;
		anchoht = 28;
	} break;
	case 3: { // dining room
		xht = 130;
		yht = 2;
		anchoht = 30;
	} break;
	case 8: { // patch
		xht = 10;
		yht = 100;
		anchoht = 50;
	} break;
	case 10: { // well
		xht = 10;
		yht = 2;
		anchoht = 40;
	} break;
	case 11: { // pond
		xht = 172;
		yht = 2;
		anchoht = 26;
	} break;
	case 16: { // dorm. 1
		xht = 140;
		yht = 2;
		anchoht = 30;
	} break;
	case 17: { // dorm. 2
		xht = 10;
		yht = 2;
		anchoht = 30;
	} break;
	case 21: { // p4
		xht = 10;
		yht = 100;
		anchoht = 50;
	} break;
	case 23: { // fountain
		xht = 10;
		yht = 2;
		anchoht = 19;
	} break;
	case 25: { // catacombs
		xht = 10;
		yht = 2;
		anchoht = 22;
	} break;
	case 28: { // storage room
		xht = 180;
		yht = 60;
		anchoht = 24;
	} break;
	case 31: { // prison
		xht = 10;
		yht = 2;
		anchoht = 25;
	} break;
	default: { // any other room
		xht = 10;
		yht = 2;
		anchoht = 50;
	}
	}

	verb.seek(textRef * verbRegSize);

	do {

		text = readVerbRegister();

		insertName = 0;

		for (int i = 0; i < text.text.size(); i++) {
			text.text.setChar(decryptionKey[i] ^ text.text[i], i);
			if (text.text[i] == '@')
				insertName = i;
		}

		if (insertName > 0) {
			text.text.deleteChar(insertName);
			text.text.insertString(characterName, insertName);
		}

		if (text.text.size() < anchoht) {
			tambackgroundht = imagesize(xht - 1, yht - 1, xht + (text.text.size() * 8) + 2, yht + 13);
			backgroundtextht = (byte *)malloc(tambackgroundht);

			g_engine->_graphics->getImg(xht - 1, yht - 1, xht + (text.text.size() * 8) + 2, yht + 13, backgroundtextht);

			littText(xht - 1, yht, text.text, shadowColor);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
			littText(xht + 1, yht, text.text, shadowColor);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
			littText(xht, yht - 1, text.text, shadowColor);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
			littText(xht, yht + 1, text.text, shadowColor);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);

			littText(xht, yht, text.text, textColor);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
		} else {

			iht = 0;
			iteracionesht = 0;
			matrizsaltosht[0] = 0;

			// Breaks text lines on the last space when reaching the [anchoht]
			do {
				iht += anchoht;
				iteracionesht += 1;
				do {
					iht -= 1;
				} while (text.text[iht] != ' ');
				matrizsaltosht[iteracionesht] = iht + 1;
			} while (iht + 1 <= text.text.size() - anchoht);

			iteracionesht += 1;
			matrizsaltosht[iteracionesht] = text.text.size();

			// Grab patch of background behind where the text will be, to paste it back later
			tambackgroundht = imagesize(xht - 1, yht - 1, xht + (anchoht * 8) + 2, yht + iteracionesht * 13);
			backgroundtextht = (byte *)malloc(tambackgroundht);
			g_engine->_graphics->getImg(xht - 1, yht - 1, xht + (anchoht * 8) + 2, yht + iteracionesht * 13, backgroundtextht);

			for (lineaht = 1; lineaht <= iteracionesht; lineaht++) {

				Common::String lineString = Common::String(text.text.c_str() + matrizsaltosht[lineaht - 1], text.text.c_str() + matrizsaltosht[lineaht]);

				littText(xht + 1, yht + ((lineaht - 1) * 11), lineString, shadowColor);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
				littText(xht - 1, yht + ((lineaht - 1) * 11), lineString, shadowColor);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
				littText(xht, yht + ((lineaht - 1) * 11) + 1, lineString, shadowColor);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
				littText(xht, yht + ((lineaht - 1) * 11) - 1, lineString, shadowColor);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
				littText(xht, yht + ((lineaht - 1) * 11), lineString, textColor);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
			}
		}

		indiceaniconversa = 0;
		bool mouseClicked = false;
		Common::Event e;
		// Plays talk cycle if needed
		do {
			g_engine->_chrono->updateChrono();
			while (g_system->getEventManager()->pollEvent(e)) {
				if (isMouseEvent(e)) {
					if (e.type == Common::EVENT_LBUTTONUP || e.type == Common::EVENT_RBUTTONUP) {
						mouseClicked = true;
					}
				}
				changeGameSpeed(e);
			}
			if (timeToDraw) {
				timeToDraw = false;
				if (timeToDraw2) {
					if (isWithinConversation) {
						indiceaniconversa += 1;
						if (textColor == 255) {
							iframe2 = 0;
							if (iframe >= 15)
								iframe = 0;
							else
								iframe++;
						} else {

							iframe = 0;
							if (iframe2 >= secondaryAnimationFrameCount - 1)
								iframe2 = 0;
							else
								iframe2++;
						}
						// Talk sprites are always put in facing direction 0
						direccionmovimientopaso = charFacingDirection;
						charFacingDirection = 0;
						sprites(true);
						charFacingDirection = direccionmovimientopaso;
					} else {
						if (currentSecondaryTrajectoryIndex >= currentRoomData->secondaryTrajectoryLength)
							currentSecondaryTrajectoryIndex = 1;
						else
							currentSecondaryTrajectoryIndex += 1;
						secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
						secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
						secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
						if (iframe2 >= secondaryAnimationFrameCount - 1)
							iframe2 = 0;
						else
							iframe2++;
						sprites(false);
					}
				}
				if (currentRoomData->paletteAnimationFlag && palAnimStep >= 4) {
					palAnimStep = 0;
					if (isPaletteAnimEnabled > 6)
						isPaletteAnimEnabled = 0;
					else
						isPaletteAnimEnabled += 1;
					if (currentRoomData->code == 4 && isPaletteAnimEnabled == 4)
						g_engine->_sound->playVoc();
					updatePalette(isPaletteAnimEnabled);
				} else
					palAnimStep += 1;
			}
			g_engine->_screen->update();
			g_system->delayMillis(10);
		} while (indiceaniconversa <= (text.text.size() * 4) && !mouseClicked && !g_engine->shouldQuit());

		g_engine->_graphics->putImg(xht - 1, yht - 1, backgroundtextht);
		free(backgroundtextht);

		g_system->delayMillis(10);
	} while (text.continued && !g_engine->shouldQuit());
	responseNumber = text.response;
	g_engine->_mouseManager->show();
}

void wcScene() {
	palette wcPalette;
	currentZone = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
	goToObject(currentZone, targetZone);

	copyPalette(pal, wcPalette);
	g_engine->_mouseManager->hide();

	partialFadeOut(234);

	const char *const *messages = (g_engine->_lang == Common::ES_ESP) ? fullScreenMessages[0] : fullScreenMessages[1];

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

	g_engine->_sound->playVoc("WATER", 272050, 47062);
	bar(50, 90, 200, 100, 0);
	delay(4000);

	characterPosX = 76 - characterCorrectionX;
	characterPosY = 78 - characerCorrectionY;
	copyPalette(wcPalette, pal);
	restoreBackground();
	assembleScreen();
	drawScreen(sceneBackground);
	partialFadeIn(234);
	xframe2 = 0;
	currentTrajectoryIndex = 0;
	trajectoryLength = 1;
	currentZone = 8;
	targetZone = 8;
	trajectory[0].x = characterPosX;
	trajectory[0].y = characterPosY;

	g_engine->_mouseManager->show();
}

void readConversationFile(Common::String f) {
	Common::File conversationFile;
	debug("Filename = %s", f.c_str());
	if (!conversationFile.open(Common::Path(f))) {
		showError(314);
	}
	int64 fileSize = conversationFile.size();
	byte *buf = (byte *)malloc(fileSize);
	conversationFile.read(buf, fileSize);

	conversationData = new Common::MemorySeekableReadWriteStream(buf, fileSize, DisposeAfterUse::NO);
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

void saveRoomData(RoomFileRegister *room, Common::SeekableWriteStream *stream) {
	rooms->seek(room->code * roomRegSize, SEEK_SET);
	saveRoom(room, stream);
}

/**
 * Object files contain a single register per object, with a set of 8 flags, to mark them as used in each save.
 */
void initializeObjectFile() {
	Common::File objFile;
	if (!objFile.open(Common::Path("OBJETOS.DAT"))) {
		showError(261);
	}
	delete (invItemData);
	byte *objectData = (byte *)malloc(objFile.size());
	objFile.read(objectData, objFile.size());
	invItemData = new Common::MemorySeekableReadWriteStream(objectData, objFile.size(), DisposeAfterUse::NO);
	objFile.close();
}

void saveItem(ScreenObject object, Common::SeekableWriteStream *objectDataStream) {
	objectDataStream->writeUint16LE(object.code);
	objectDataStream->writeByte(object.height);

	objectDataStream->writeByte(object.name.size());
	objectDataStream->writeString(object.name);
	int paddingSize = objectNameLength - object.name.size();
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

	objectDataStream->writeByte(object.xrej1);
	objectDataStream->writeByte(object.yrej1);
	objectDataStream->writeByte(object.xrej2);
	objectDataStream->writeByte(object.yrej2);

	objectDataStream->write(object.walkAreasPatch, 100);
	objectDataStream->write(object.mouseGridPatch, 100);
}

void saveItemRegister(ScreenObject object, Common::SeekableWriteStream *stream) {
	invItemData->seek(object.code * itemRegSize, SEEK_SET);
	saveItem(object, stream);
}

void printNiches() {
	debug("|   | %03d | %03d | %03d | %03d |", 0, 1, 2, 3);
	debug("| 0 | %03d | %03d | %03d | %03d |", niche[0][0], niche[0][1], niche[0][2], niche[0][3]);
	debug("| 1 | %03d | %03d | %03d | %03d |", niche[1][0], niche[1][1], niche[1][2], niche[1][3]);
	debug("niche[0][niche[0][3]] = %d", niche[0][niche[0][3]]);
	debug("niche[1][niche[1][3]] = %d", niche[1][niche[1][3]]);
}

} // End of namespace Tot
