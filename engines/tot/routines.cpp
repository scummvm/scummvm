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

#include "engines/tot/offsets.h"
#include "engines/tot/routines.h"
#include "engines/tot/texts.h"
#include "engines/tot/tot.h"

namespace Tot {

void drawText(uint numero) {
	assignText();
	hypertext(numero, 255, 0, kaka, false);
	verb.close();
}

void runaroundRed() {
	const uint trayseg[91][2] = {
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

	uint itrayseg, longtrayseg;

	bool exitLoop = false;
	loadDevil();
	itrayseg = 0;
	longtrayseg = 82;
	iframe2 = 0;
	animacion2 = true;
	do {
		g_engine->_chrono->updateChrono();
		if (tocapintar) {
			if (itrayseg == longtrayseg)
				exitLoop = true;
			itrayseg += 1;
			if (iframe2 >= secondaryAnimationFrameCount - 1)
				iframe2 = 0;
			else
				iframe2++;
			animado.posx = trayseg[itrayseg][0] - 15;
			animado.posy = trayseg[itrayseg][1] - 42;
			if (itrayseg >= 0 && itrayseg <= 8) {
				animado.dir = 2;
				animado.profundidad = 1;
			} else if (itrayseg >= 9 && itrayseg <= 33) {
				animado.dir = 2;
				animado.profundidad = 14;
			} else if (itrayseg >= 34 && itrayseg <= 63) {
				animado.dir = 1;
				animado.profundidad = 14;
			} else {
				animado.dir = 0;
				animado.profundidad = 3;
			}

			tocapintar2 = true;
			sprites(false);
			tocapintar = false;
			if (saltospal >= 4) {
				saltospal = 0;
				if (movidapaleta > 6)
					movidapaleta = 0;
				else
					movidapaleta += 1;
				updatePalette(movidapaleta);
			} else
				saltospal += 1;
			g_engine->_screen->update();
		}
	} while (!exitLoop && !g_engine->shouldQuit());
	freeAnimation();
	screenHandleToBackground();
	assembleScreen();
	drawScreen(background);
}

void updateMovementCells() {
	uint j1arm, j2arm;
	byte i1arm, i2arm;

	j1arm = (currentRoomData->tray2[indicetray2 - 1].x / factorx) + 1;
	j2arm = (currentRoomData->tray2[indicetray2 - 1].y / factory) + 1;
	if ((oldposx != j1arm) || (oldposy != j2arm)) {

		for (i1arm = 0; i1arm < maxrejax; i1arm++)
			for (i2arm = 0; i2arm < maxrejay; i2arm++) {
				currentRoomData->rejapantalla[oldposx + i1arm][oldposy + i2arm] = rejafondomovto[i1arm][i2arm];
				currentRoomData->mouseGrid[oldposx + i1arm][oldposy + i2arm] = rejafondoraton[i1arm][i2arm];
			}

		for (i1arm = 0; i1arm < maxrejax; i1arm++)
			for (i2arm = 0; i2arm < maxrejay; i2arm++) {
				rejafondomovto[i1arm][i2arm] = currentRoomData->rejapantalla[j1arm + i1arm][j2arm + i2arm];
				if (rejamascaramovto[i1arm][i2arm] > 0)
					currentRoomData->rejapantalla[j1arm + i1arm][j2arm + i2arm] = rejamascaramovto[i1arm][i2arm];

				rejafondoraton[i1arm][i2arm] = currentRoomData->mouseGrid[j1arm + i1arm][j2arm + i2arm];
				if (rejamascararaton[i1arm][i2arm] > 0)
					currentRoomData->mouseGrid[j1arm + i1arm][j2arm + i2arm] = rejamascararaton[i1arm][i2arm];
			}

		oldposx = j1arm;
		oldposy = j2arm;
		oldxrejilla = 0;
		oldyrejilla = 0;
	}
}

void sprites(bool pintapersonaje);

static uint indice;
/**
 * Calculated using the position of the character
 */
static uint posfondox,
	posfondoy;
/**
 * Calculated using the position of the character + dimension
 */
static uint posfondox2, posfondoy2;

static void assembleBackground() {
	uint posabs;

	// XMStoPointer(ptr(segfondo, (offfondo + 4)), handpantalla, 4, (sizepantalla - longint(4)));

	// copies the entire clean background in handpantalla back into fondo
	screenHandleToBackground();

	posabs = 4 + posfondoy * 320 + posfondox;
	// debug("posabs = %d, posfondox = %d, posfondoy=%d", posabs, posfondox, posfondoy);
	uint16 w, h;
	w = READ_LE_UINT16(characterDirtyRect);
	h = READ_LE_UINT16(characterDirtyRect + 2);
	w++;
	h++;

	posfondox2 = posfondox + w;
	posfondoy2 = posfondoy + h;

	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			int pos = posabs + j * 320 + i;
			int destPos = 4 + (j * w + i);
			characterDirtyRect[destPos] = background[pos];
		}
	}
}

/**
 * calculates the overlapping area between the source image and the background,
 * then "blits" (copies) the pixels from the image to the overlapping area of the background, respecting transparency.
 */
static void assembleImage(byte *imagen, uint posimagenx, uint posimageny) { // Near;

	uint x, // starting point of the overlap
		y;
	uint incx, // width and height of the overlapping area
		incy;

	uint16 wImg = READ_LE_UINT16(imagen) + 1;
	uint16 hImg = READ_LE_UINT16(imagen + 2) + 1;

	uint16 wFondo = READ_LE_UINT16(characterDirtyRect) + 1;
	uint16 hFondo = READ_LE_UINT16(characterDirtyRect + 2) + 1;

	// This region calculates the overlapping area of (x, incx, y, incy)
	{
		if (posimagenx < posfondox)
			x = posfondox;
		else
			x = posimagenx;

		if (posimagenx + wImg < posfondox + wFondo)
			incx = posimagenx + wImg - x;
		else
			incx = posfondox + wFondo - x;

		if (posimageny < posfondoy)
			y = posfondoy;
		else
			y = posimageny;

		if (posimageny + hImg < posfondoy + hFondo)
			incy = posimageny + hImg - y;
		else
			incy = posfondoy + hFondo - y;
	} // end of region calculating overlapping area

	for (int j = 0; j < incy; j++) {
		for (int i = 0; i < incx; i++) {
			int orfondo = 4 + ((y - posfondoy) + j) * wFondo + i + (x - posfondox);
			int orimg = 4 + (y - posimageny + j) * wImg + i + (x - posimagenx);
			if (imagen[orimg] != 0) {
				characterDirtyRect[orfondo] = imagen[orimg];
			}
		}
	}
}

static void overlayObject() {
	if (screenObjects[indice] != NULL) {
		if (
			(depthMap[indice].posx <= posfondox2) &&
			(depthMap[indice].posx2 > posfondox) &&
			(depthMap[indice].posy < posfondoy2) &&
			(depthMap[indice].posy2 > posfondoy)) {
			assembleImage(screenObjects[indice], depthMap[indice].posx, depthMap[indice].posy);
		}
	}
}

void drawMainCharacter() {

	bool debug = false;
	if (debug) {
		screenTransition(13, false, background);
	}

	uint16 tempW;
	uint16 tempH;
	tempW = READ_LE_UINT16(pasoframe);
	tempH = READ_LE_UINT16(pasoframe + 2);
	tempW += 6;
	tempH += 6;

	characterDirtyRect = (byte *)malloc((tempW + 1) * (tempH + 1) + 4);

	WRITE_LE_UINT16(characterDirtyRect, tempW);
	WRITE_LE_UINT16(characterDirtyRect + 2, tempH);

	assembleBackground();
	indice = 0;
	while (indice != nivelesdeprof) {

		overlayObject();
		if (secuencia.profundidad == indice)
			assembleImage(pasoframe, characterPosX, characterPosY);
		indice += 1;
	}

	putImg(posfondox, posfondoy, characterDirtyRect);

	if (debug) {
		// draw background dirty area
		drawRect(2, posfondox, posfondoy, posfondox + tempW, posfondoy + tempH);

		// drawRect(225, posfondox, posfondoy, posfondox2, posfondoy2);

		// uint16 wImg = READ_LE_UINT16(pasoframe);
		// uint16 hImg = READ_LE_UINT16(pasoframe + 2);

		drawPos(xframe2, yframe2, 218);
		// draw patch of characters sprite
		// drawRect(223, xframe, yframe, xframe + wImg, yframe + hImg);
	}
	free(characterDirtyRect);
}

void sprites(bool pintapersonaje) {
	// grabs the current frame from the walk cycle
	pasoframe = secuencia.bitmap[direccionmovimiento][iframe];

	posfondox = characterPosX - 3;
	posfondoy = characterPosY - 3;
	if (animacion2) {
		if (currentRoomData->longtray2 > 1) {
			updateMovementCells();
		}
		if (tocapintar2) {
			if (peteractivo && !g_engine->_sound->isVocPlaying()) {
				iframe2 = 0;
			}
			pasoanimado = animado.dib[animado.dir][iframe2];
		}
		uint16 pasoframeW = READ_LE_UINT16(pasoframe);
		uint16 pasoframeH = READ_LE_UINT16(pasoframe + 2);

		uint16 pasoanimadoW = READ_LE_UINT16(pasoanimado);
		uint16 pasoanimadoH = READ_LE_UINT16(pasoanimado + 2);

		if (
			((animado.posx < (characterPosX + pasoframeW) + 4) &&
			 ((animado.posx + pasoanimadoW + 1) > posfondox) &&
			 (animado.posy < (characterPosY + pasoframeH + 4))) &&
			((animado.posy + pasoanimadoH + 1) > posfondoy)) { // Character is in the area of the animation

			if (animado.posx < characterPosX) {
				posfondox = animado.posx - 3;
			}
			if (animado.posy < characterPosY) {
				posfondoy = animado.posy - 3;
			}

			uint16 patchW = pasoanimadoW + pasoframeW + 6;

			uint16 patchH;
			if ((pasoframeH + characterPosY) > (animado.posy + pasoanimadoH)) {
				patchH = pasoframeH + 6 + abs(characterPosY - animado.posy);
			} else {
				patchH = pasoanimadoH + 6 + abs(characterPosY - animado.posy);
			}

			if (posfondoy + patchH > 140) {
				patchH -= (posfondoy + patchH) - 140 + 1;
			}

			if (posfondox + patchW > 320) {
				patchW -= (posfondox + patchW) - 320 + 1;
			}

			characterDirtyRect = (byte *)malloc((patchW + 1) * (patchH + 1) + 4);

			WRITE_LE_UINT16(characterDirtyRect, patchW);
			WRITE_LE_UINT16(characterDirtyRect + 2, patchH);

			assembleBackground(); // {Montar el Sprite Total}
			indice = 0;
			while (indice != nivelesdeprof) {
				overlayObject();
				if (animado.profundidad == indice)
					assembleImage(pasoanimado, animado.posx, animado.posy);
				if (secuencia.profundidad == indice)
					assembleImage(pasoframe, characterPosX, characterPosY);
				indice += 1;
			}
			putImg(posfondox, posfondoy, characterDirtyRect);
		} else { // character and animation are in different parts of the screen

			if (pintapersonaje) {
				drawMainCharacter();
			}

			posfondox = animado.posx - 3;
			posfondoy = animado.posy - 3;

			pasoanimadoW = READ_LE_UINT16(pasoanimado) + 6;
			pasoanimadoH = READ_LE_UINT16(pasoanimado + 2) + 6;

			if (posfondoy + pasoanimadoH > 140) {
				pasoanimadoH -= (posfondoy + pasoanimadoH) - 140 + 1;
			}

			if (posfondox + pasoanimadoW > 320) {
				pasoanimadoW -= (posfondox + pasoanimadoW) - 320 + 1;
			}

			characterDirtyRect = (byte *)malloc((pasoanimadoW + 1) * (pasoanimadoH + 1) + 4);
			WRITE_LE_UINT16(characterDirtyRect, pasoanimadoW);
			WRITE_LE_UINT16(characterDirtyRect + 2, pasoanimadoH);

			assembleBackground();
			indice = 0;
			while (indice != nivelesdeprof) {
				overlayObject();
				if (animado.profundidad == indice)
					assembleImage(pasoanimado, animado.posx, animado.posy);
				indice += 1;
			}
			putImg(posfondox, posfondoy, characterDirtyRect);
		}
	} else if (pintapersonaje) {
		drawMainCharacter();
	}
}

void adjustKey() {
	iframe = 0;
	calculateTrajectory(129, 13);
	direccionmovimiento = 0;
	do {
		characterPosX = trayec[indicetray].x;
		characterPosY = trayec[indicetray].y;
		iframe++;
		indicetray += 1;
		emptyLoop();
		tocapintar = false;
		if (saltospal >= 4) {
			saltospal = 0;
			if (movidapaleta > 6)
				movidapaleta = 0;
			else
				movidapaleta += 1;
			updatePalette(movidapaleta);
		} else
			saltospal += 1;
		sprites(true);
	} while (indicetray != longtray);
	direccionmovimiento = 3;
	emptyLoop();
	tocapintar = false;
	sprites(true);
}

void adjustKey2() {
	iframe = 0;
	direccionmovimiento = 2;
	longtray = 1;
	do {
		characterPosX = trayec[indicetray].x;
		characterPosY = trayec[indicetray].y;
		iframe++;
		indicetray -= 1;
		emptyLoop();
		tocapintar = false;
		if (saltospal >= 4) {
			saltospal = 0;
			if (movidapaleta > 6)
				movidapaleta = 0;
			else
				movidapaleta += 1;
			updatePalette(movidapaleta);
		} else
			saltospal += 1;
		sprites(true);
	} while (indicetray != 0);
	emptyLoop();
	tocapintar = false;
	sprites(true);
}

void animatedSequence(uint numSequence) {
	Common::File animationFile;
	uint indicerep, indiceani;
	uint16 tamsecani;
	uint animx, animy;
	int guadaxframe;
	byte numframessec;
	byte *animptr;

	switch (numSequence) {
	case 1: {
		guadaxframe = characterPosX;
		characterPosX = 3;
		if (!animationFile.open("POZO01.DAT")) {
			showError(277);
		}
		animx = 127;
		animy = 70;
		tamsecani = animationFile.readUint16LE();
		numframessec = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(tamsecani);
		for (indicerep = 1; indicerep <= 3; indicerep++) {
			g_engine->_sound->playVoc("POZO", 180395, 6034);
			indiceani = 0;
			do {
				emptyLoop();
				tocapintar = false;
				if (tocapintar2) {
					if (indicetray2 >= currentRoomData->longtray2)
						indicetray2 = 1;
					else
						indicetray2 += 1;
					animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
					animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
					animado.dir = currentRoomData->dir2[indicetray2 - 1];
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					sprites(false);
					animationFile.read(animptr, tamsecani);
					putImg(animx, animy, animptr);
					indiceani += 1;
				}
			} while (indiceani != numframessec && !g_engine->shouldQuit());
			animationFile.seek(4);
		}
		free(animptr);
		animationFile.close();
		characterPosX = guadaxframe;
	} break;
	case 2: {
		if (!animationFile.open("POZOATR.DAT")) {
			showError(277);
		}
		animx = 127;
		guadaxframe = characterPosX;
		characterPosX = 3;
		animy = 70;

		tamsecani = animationFile.readUint16LE();
		numframessec = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(tamsecani);
		for (indicerep = 1; indicerep <= 3; indicerep++) {
			g_engine->_sound->playVoc("POZO", 180395, 6034);
			indiceani = 0;
			do {
				emptyLoop();
				tocapintar = false;
				if (tocapintar2) {
					if (indicetray2 >= currentRoomData->longtray2)
						indicetray2 = 1;
					else
						indicetray2 += 1;
					animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
					animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
					animado.dir = currentRoomData->dir2[indicetray2 - 1];
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					sprites(false);
					animationFile.read(animptr, tamsecani);
					putImg(animx, animy, animptr);
					indiceani += 1;
				}
			} while (indiceani != numframessec && !g_engine->shouldQuit());
			animationFile.seek(4);
			// seek(ficherosecuenciaanimada, 4);
		}
		g_engine->_sound->stopVoc();
		free(animptr);
		animationFile.close();
		characterPosX = guadaxframe;
	} break;
	case 3: {
		guadaxframe = characterPosX;
		characterPosX = 3;
		if (!animationFile.open("POZO02.DAT")) {
			showError(277);
		}
		animx = 127;
		animy = 70;

		tamsecani = animationFile.readUint16LE();
		numframessec = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(tamsecani);
		indiceani = 0;
		do {
			emptyLoop();
			tocapintar = false;
			if (tocapintar2) {
				if (indicetray2 >= currentRoomData->longtray2)
					indicetray2 = 1;
				else
					indicetray2 += 1;
				animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
				animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
				animado.dir = currentRoomData->dir2[indicetray2 - 1];
				if (iframe2 >= secondaryAnimationFrameCount - 1)
					iframe2 = 0;
				else
					iframe2++;
				sprites(false);
				animationFile.read(animptr, tamsecani);
				putImg(animx, animy, animptr);
				indiceani += 1;
			}
		} while (indiceani != numframessec && !g_engine->shouldQuit());
		free(animptr);
		animationFile.close();
		iframe = 0;
		direccionmovimiento = 2;
		emptyLoop();
		tocapintar = false;
		characterPosX = guadaxframe;
		sprites(true);
	} break;
	case 4: {
		adjustKey();
		if (!animationFile.open("TIOLLAVE.DAT")) {
			showError(277);
		}
		animx = 85;
		animy = 15;

		tamsecani = animationFile.readUint16LE();
		numframessec = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(tamsecani);

		for (indiceani = 1; indiceani <= 31; indiceani++) {
			animationFile.read(animptr, tamsecani);
			emptyLoop();
			tocapintar = false;
			if (saltospal >= 4) {
				saltospal = 0;
				if (movidapaleta > 6)
					movidapaleta = 0;
				else
					movidapaleta += 1;
				updatePalette(movidapaleta);
			} else
				saltospal += 1;
			putShape(animx, animy, animptr);
		}
		screenObjects[regobj.profundidad - 1] = NULL;
		screenHandleToBackground();
		indiceani = secuencia.profundidad;
		secuencia.profundidad = 30;
		screenObjects[13] = animptr;
		depthMap[13].posx = animx;
		depthMap[13].posy = animy;
		assembleScreen();
		drawScreen(background);
		screenObjects[13] = NULL;
		secuencia.profundidad = indiceani;
		drawBackpack();
		for (indiceani = 32; indiceani <= numframessec; indiceani++) {
			animationFile.read(animptr, tamsecani);
			emptyLoop();
			tocapintar = false;
			if (saltospal >= 4) {
				saltospal = 0;
				if (movidapaleta > 6)
					movidapaleta = 0;
				else
					movidapaleta += 1;
				updatePalette(movidapaleta);
			} else
				saltospal += 1;
			putImg(animx, animy, animptr);
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
		animx = 204;
		animy = 44;
		tamsecani = animationFile.readUint16LE();
		numframessec = animationFile.readByte();
		animationFile.readByte();
		animptr = (byte *)malloc(tamsecani);
		for (indiceani = 1; indiceani <= 8; indiceani++) {
			animationFile.read(animptr, tamsecani);
			emptyLoop();
			tocapintar = false;
			putShape(animx, animy, animptr);
			if (g_engine->shouldQuit()) {
				break;
			}
		}
		indiceani = secuencia.profundidad;
		secuencia.profundidad = 30;
		screenObjects[12] = animptr;
		depthMap[12].posx = animx;
		depthMap[12].posy = animy;
		disableSecondAnimation();
		screenObjects[12] = NULL;
		secuencia.profundidad = indiceani;
		drawScreen(background);
		for (indiceani = 9; indiceani <= numframessec; indiceani++) {
			animationFile.read(animptr, tamsecani);
			emptyLoop();
			tocapintar = false;
			emptyLoop();
			tocapintar = false;
			putShape(animx, animy, animptr);
			if (g_engine->shouldQuit()) {
				break;
			}
		}
		free(animptr);
		animationFile.close();
		iframe = 0;
		direccionmovimiento = 2;
		emptyLoop();
		tocapintar = false;
		sprites(true);
	} break;
	case 6: {
		currentRoomData->animationFlag = false;
		if (!animationFile.open("AZCCOG.DAT")) {
			showError(277);
		}
		tamsecani = animationFile.readUint16LE();
		numframessec = animationFile.readByte();
		animationFile.readByte();
		screenObjects[6] = (byte *)malloc(tamsecani);
		// objetos[7] = ptr(segfondo, (offfondo + 44900));
		depthMap[6].posx = animado.posx + 5;
		depthMap[6].posy = animado.posy - 6;
		indiceani = 0;
		do {
			emptyLoop();
			tocapintar = false;
			if (saltospal >= 4) {
				saltospal = 0;
				if (movidapaleta > 6)
					movidapaleta = 0;
				else
					movidapaleta += 1;
				updatePalette(movidapaleta);
			} else
				saltospal += 1;
			if (tocapintar2) {
				animationFile.read(screenObjects[6], tamsecani);
				Common::copy(screenObjects[6], screenObjects[6] + tamsecani, background + 44900);
				// blockread(ficherosecuenciaanimada, objetos[7], tamsecani);
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				indiceani += 1;
				if (indiceani == 8)
					g_engine->_sound->playVoc("PUFF", 191183, 18001);
			}
		} while (indiceani != numframessec && !g_engine->shouldQuit());
		animationFile.close();
		g_engine->_sound->stopVoc();
		screenObjects[6] = NULL;
		currentRoomData->animationFlag = true;
	} break;
	}
}

DoorRegistry readRegPuertas(Common::SeekableReadStream *screenDataFile) {
	DoorRegistry doorMetadata;
	doorMetadata.pantallaquecarga = screenDataFile->readUint16LE();
	doorMetadata.posxsalida = screenDataFile->readUint16LE();
	doorMetadata.posysalida = screenDataFile->readUint16LE();
	doorMetadata.abiertacerrada = screenDataFile->readByte();
	doorMetadata.codigopuerta = screenDataFile->readByte();

	return doorMetadata;
}

Common::Point readPunto(Common::SeekableReadStream *screenDataFile) {
	Common::Point point;
	point.x = screenDataFile->readUint16LE();
	point.y = screenDataFile->readUint16LE();
	return point;
}

RoomBitmapRegister readBitmapAux(Common::SeekableReadStream *screenDataFile) {
	RoomBitmapRegister bitmapMetadata = RoomBitmapRegister();
	bitmapMetadata.puntbitmap = screenDataFile->readSint32LE();
	bitmapMetadata.tambitmap = screenDataFile->readUint16LE();
	bitmapMetadata.coordx = screenDataFile->readUint16LE();
	bitmapMetadata.coordy = screenDataFile->readUint16LE();
	bitmapMetadata.profund = screenDataFile->readUint16LE();
	return bitmapMetadata;
}

RoomObjectListEntry *readRegIndexadoObjetos(Common::SeekableReadStream *screenDataFile) {
	RoomObjectListEntry *objectMetadata = new RoomObjectListEntry();
	objectMetadata->indicefichero = screenDataFile->readUint16LE();
	objectMetadata->objectName = screenDataFile->readPascalString();

	screenDataFile->skip(20 - objectMetadata->objectName.size());

	return objectMetadata;
}

RoomFileRegister *readScreenDataFile(Common::SeekableReadStream *screenDataFile) {
	RoomFileRegister *screenData = new RoomFileRegister();
	screenData->codigo = screenDataFile->readUint16LE();
	screenData->puntimagenpantalla = screenDataFile->readUint32LE();
	screenData->tamimagenpantalla = screenDataFile->readUint16LE();
	screenDataFile->read(screenData->rejapantalla, 40 * 28);
	screenDataFile->read(screenData->mouseGrid, 40 * 28);
	// read puntos
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 30; j++) {
			for (int k = 0; k < 5; k++) {
				screenData->trajectories[i][j][k] = readPunto(screenDataFile);
			}
		}
	}

	for (int i = 0; i < 5; i++) {
		screenData->doors[i] = readRegPuertas(screenDataFile);
	}
	for (int i = 0; i < 15; i++) {
		screenData->bitmapasociados[i] = readBitmapAux(screenDataFile);
	}
	for (int i = 0; i < 51; i++) {
		screenData->indexadoobjetos[i] = readRegIndexadoObjetos(screenDataFile);
	}

	screenData->animationFlag = screenDataFile->readByte();
	screenData->nombremovto = screenDataFile->readPascalString();
	screenDataFile->skip(8 - screenData->nombremovto.size());
	screenData->paletteAnimationFlag = screenDataFile->readByte();
	screenData->puntpaleta = screenDataFile->readUint16LE();
	for (int i = 0; i < 300; i++) {
		screenData->tray2[i] = readPunto(screenDataFile);
	}
	screenDataFile->read(screenData->dir2, 600);
	screenData->longtray2 = screenDataFile->readUint16LE();
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
		RoomBitmapRegister &bitmap = currentRoomData->bitmapasociados[i];
		if (bitmap.tambitmap > 0) {
			loadItem(bitmap.coordx, bitmap.coordy, bitmap.tambitmap, bitmap.puntbitmap, bitmap.profund);
		}
	}
	if (currentRoomData->animationFlag && currentRoomData->codigo != 24) {
		loadAnimation(currentRoomData->nombremovto);
		iframe2 = 0;
		indicetray2 = 1;
		animado.dir = currentRoomData->dir2[indicetray2 - 1];
		animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
		animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
		if (currentRoomData->nombremovto == "FUENTE01")
			animado.profundidad = 0;
		else {
			updateSecondaryAnimationDepth();
		}
		for (int i = 0; i < maxrejax; i++)
			for (int j = 0; j < maxrejay; j++) {
				if (rejamascaramovto[i][j] > 0)
					currentRoomData->rejapantalla[oldposx + i][oldposy + j] = rejamascaramovto[i][j];
				if (rejamascararaton[i][j] > 0)
					currentRoomData->mouseGrid[oldposx + i][oldposy + j] = rejamascararaton[i][j];
			}
	} else
		currentRoomData->animationFlag = false;

	updateMainCharacterDepth();
	assembleScreen();
	movidapaleta = 0;
}

/**
 * Calculates a trajectory between the current position (xframe, yframe)
 * and the target position (finalx, finaly) using bresenham's algorithm
 */
void calculateTrajectory(uint finalx, uint finaly) {

	int deltaX = finalx - characterPosX;
	int deltaY = finaly - characterPosY;

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
	int incrXRecto = horizontalDominant ? incrXDiag : 0;
	int incrYRecto = horizontalDominant ? 0 : incrYDiag;

	int contRecto = 2 * deltaY;
	int contDiag = contRecto - (2 * deltaX);
	int BX = contRecto - deltaX; // decision variable

	// start at initial position
	uint x = characterPosX, y = characterPosY;
	int steps = deltaX + 1; // number of steps

	indicetray = 0;
	longtray = 0;
	trayec[longtray].x = x;
	trayec[longtray].y = y;
	longtray++;
	while (--steps) {
		if (BX >= 0) {
			x += incrXDiag;
			y += incrYDiag;
			BX += contDiag;
		} else {
			x += incrXRecto;
			y += incrYRecto;
			BX += contRecto;
		}
		trayec[longtray].x = x;
		trayec[longtray].y = y;
		longtray++;
	}

	// Ensure last value is also final position to account for overflow of the route
	trayec[longtray].x = finalx;
	trayec[longtray].y = finaly;
	trayec[longtray + 1].x = finalx;
	trayec[longtray + 1].y = finaly;

	// longtray--; // decrement because of 1-starting arrays
}

void lookInventoryObject(byte numeroobjeto) {
	byte yaux;

	Common::String cadenadescripcion;
	bool kkaux;
	regismht reghpt;
	palette paletaseg;

	kkaux = false;
	contadorpc2 = contadorpc;
	g_engine->_mouseManager->hide();
	copyPalette(pal, paletaseg);
	readItemRegister(mobj[numeroobjeto].code);
	getImg(0, 0, 319, 139, background);
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
			0,                // color5
			0,                // xtext
			""                // text
		);

	drawMenu(4);

	if (regobj.usar[0] != 9) {
		if (regobj.beforeUseTextRef != 0) {
			assignText();
			reghpt = readVerbRegister(regobj.beforeUseTextRef);
			cadenadescripcion = reghpt.cadenatext;
			for (yaux = 0; yaux < reghpt.cadenatext.size(); yaux++)
				cadenadescripcion.setChar(decryptionKey[yaux] ^ reghpt.cadenatext[yaux], yaux);
			hipercadena(cadenadescripcion, 60, 15, 33, 255, 0);
			verb.close();
		} else {
			cadenadescripcion = mobj[numeroobjeto].objectName;
			hipercadena(cadenadescripcion, 60, 15, 33, 255, 0);
		}
	} else {

		if (regobj.afterUseTextRef != 0) {
			assignText();
			reghpt = readVerbRegister(regobj.afterUseTextRef);
			cadenadescripcion = reghpt.cadenatext;
			for (yaux = 0; yaux < reghpt.cadenatext.size(); yaux++)
				cadenadescripcion.setChar(decryptionKey[yaux] ^ reghpt.cadenatext[yaux], yaux);
			hipercadena(cadenadescripcion, 60, 15, 33, 255, 0);
			verb.close();
		} else {
			cadenadescripcion = mobj[numeroobjeto].objectName;
			hipercadena(cadenadescripcion, 60, 15, 33, 255, 0);
		}
	}

	drawFlc(125, 70, regobj.punteroframesgiro, 60000, 9, 0, false, true, true, kkaux);

	screenTransition(3, true, NULL);
	partialFadeOut(234);
	assembleScreen();
	drawScreen(background);
	copyPalette(paletaseg, pal);
	partialFadeIn(234);
	g_engine->_mouseManager->show();
}

void useInventoryObjectWithInventoryObject(uint numobj1, uint numobj2) {
	byte indicemochila, indobj1, indobj2;

	debug("Reading item register %d", numobj1);
	readItemRegister(invItemData, numobj1, regobj);
	// verifyCopyProtection2();
	if (regobj.usar[0] != 1 || regobj.usarcon != numobj2) {
		drawText(Random(11) + 1022);
		return;
	}

	indicemochila = 0;
	while (mobj[indicemochila].code != numobj1) {
		indicemochila += 1;
	}
	indobj1 = indicemochila;

	indicemochila = 0;
	while (mobj[indicemochila].code != numobj2) {
		indicemochila += 1;
	}

	indobj2 = indicemochila;
	kaka = regobj.useTextRef;

	if (regobj.reemplazarpor == 0) {
		readItemRegister(invItemData, numobj1, regobj);
		regobj.usar[0] = 9;
		saveItemRegister(regobj, invItemData);

		readItemRegister(invItemData, numobj2, regobj);
		regobj.usar[0] = 9;
		saveItemRegister(regobj, invItemData);

	} else {
		readItemRegister(invItemData, regobj.reemplazarpor, regobj);
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
	if (kaka > 0)
		drawText(kaka);
}

void calculateRoute(byte zona1, byte zona2, bool extraCorrection, bool zonavedada) {
	// Resets the entire route
	for (pasos = 0; pasos < 7; pasos++) {
		mainRoute[pasos].x = 0;
		mainRoute[pasos].y = 0;
	}
	pasos = 0;

	// Starts first element of route with current position
	mainRoute[0].x = characterPosX;
	mainRoute[0].y = characterPosY;
	Common::Point point;
	do {
		pasos += 1;
		point = currentRoomData->trajectories[zona1 - 1][zona2 - 1][pasos - 1];

		if (point.x < (rectificacionx + 3))
			mainRoute[pasos].x = 3;
		else
			mainRoute[pasos].x = point.x - rectificacionx;
		if (point.y < (rectificaciony + 3))
			mainRoute[pasos].y = 3;
		else
			mainRoute[pasos].y = point.y - rectificaciony;

	} while (point.y != 9999 && pasos != 5);

	if (zona2 < 10) {
		if (point.y == 9999) {
			mainRoute[pasos].x = xframe2 - rectificacionx;
			mainRoute[pasos].y = yframe2 - rectificaciony;
		} else {
			mainRoute[6].x = xframe2 - rectificacionx;
			mainRoute[6].y = yframe2 - rectificaciony;
			pasos = 6;
		}
	} else {
		if ((mainRoute[pasos].y + rectificaciony) == 9999) {
			pasos -= 1;
		}
		if (extraCorrection) {
			switch (currentRoomData->codigo) {
			case 5:
				if (zona2 == 27)
					pasos += 1;
				break;
			case 6:
				if (zona2 == 21)
					pasos += 1;
				break;
			}
		}
	}
	if (extraCorrection && zonavedada) {
		pasos -= 1;
	}

	// Sets xframe2 now to be the number of steps in the route
	xframe2 = pasos;
	// Sets yframe2 to now be current step
	yframe2 = 1;
	longtray = 10;
	indicetray = 30;
}

void goToObject(byte zona1, byte zona2) {
	bool zonavedada = false;

	contadorpc2 = contadorpc;

	for (int indicepaso = 0; indicepaso < 5; indicepaso++) {
		if (currentRoomData->doors[indicepaso].codigopuerta == zona2) {
			zonavedada = true;
			break;
		}
	}

	if (currentRoomData->codigo == 21 && currentRoomData->animationFlag) {
		if ((zona2 >= 1 && zona2 <= 5) ||
			(zona2 >= 9 && zona2 <= 13) ||
			(zona2 >= 18 && zona2 <= 21) ||
			zona2 == 24 || zona2 == 25) {

			zonadestino = 7;
			pulsax = 232;
			pulsay = 75;
			zona2 = 7;
		}
		if (zona2 == 24) {
			zonavedada = false;
		}
	}
	if (zona1 < 10) {
		xframe2 = pulsax + 7;
		yframe2 = pulsay + 7;

		g_engine->_mouseManager->hide();
		calculateRoute(zona1, zona2, true, zonavedada);
		Common::Event e;
		do {
			g_engine->_chrono->updateChrono();
			while (g_system->getEventManager()->pollEvent(e)) {
				changeGameSpeed(e);
			}
			advanceAnimations(zonavedada, false);
			g_engine->_screen->update();
			g_system->delayMillis(10);
		} while (xframe2 != 0);

		iframe = 0;
		sprites(true);
		g_engine->_mouseManager->show();
	}
}

void updateSecondaryAnimationDepth() {
	uint animadoposition = animado.posy + altoanimado - 1;
	if (animadoposition >= 0 && animadoposition <= 56) {
		animado.profundidad = 0;
	} else if (animadoposition >= 57 && animadoposition <= 66) {
		animado.profundidad = 1;
	} else if (animadoposition >= 65 && animadoposition <= 74) {
		animado.profundidad = 2;
	} else if (animadoposition >= 73 && animadoposition <= 82) {
		animado.profundidad = 3;
	} else if (animadoposition >= 81 && animadoposition <= 90) {
		animado.profundidad = 4;
	} else if (animadoposition >= 89 && animadoposition <= 98) {
		animado.profundidad = 5;
	} else if (animadoposition >= 97 && animadoposition <= 106) {
		animado.profundidad = 6;
	} else if (animadoposition >= 105 && animadoposition <= 114) {
		animado.profundidad = 7;
	} else if (animadoposition >= 113 && animadoposition <= 122) {
		animado.profundidad = 8;
	} else if (animadoposition >= 121 && animadoposition <= 140) {
		animado.profundidad = 9;
	}
}

void updateMainCharacterDepth() {
	if (characterPosY >= 0 && characterPosY <= 7) {
		secuencia.profundidad = 0;
	} else if (characterPosY >= 8 && characterPosY <= 17) {
		secuencia.profundidad = 1;
	} else if (characterPosY >= 18 && characterPosY <= 25) {
		secuencia.profundidad = 2;
	} else if (characterPosY >= 26 && characterPosY <= 33) {
		secuencia.profundidad = 3;
	} else if (characterPosY >= 34 && characterPosY <= 41) {
		secuencia.profundidad = 4;
	} else if (characterPosY >= 42 && characterPosY <= 49) {
		secuencia.profundidad = 5;
	} else if (characterPosY >= 50 && characterPosY <= 57) {
		secuencia.profundidad = 6;
	} else if (characterPosY >= 58 && characterPosY <= 65) {
		secuencia.profundidad = 7;
	} else if (characterPosY >= 66 && characterPosY <= 73) {
		secuencia.profundidad = 8;
	} else if (characterPosY >= 74 && characterPosY <= 139) {
		secuencia.profundidad = 9;
	}
}

void advanceAnimations(bool zonavedada, bool animateMouse) {
	if (tocapintar) {

		if (currentRoomData->animationFlag && tocapintar2) {
			if (peteractivo && (Random(100) == 1) && !g_engine->_sound->isVocPlaying() && caramelos[0] == false) {
				debug("Playing tos");
				g_engine->_sound->playVoc("TOS", 258006, 14044);
			}
			if (indicetray2 >= currentRoomData->longtray2)
				indicetray2 = 1;
			else
				indicetray2 += 1;
			if (iframe2 >= secondaryAnimationFrameCount - 1)
				iframe2 = 0;
			else
				iframe2++;
			animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
			animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
			animado.dir = currentRoomData->dir2[indicetray2 - 1];
			switch (currentRoomData->codigo) {
			case 23:
				animado.profundidad = 0;
				break;
			case 24:
				animado.profundidad = 14;
				break;
			default:
				updateSecondaryAnimationDepth();
			}
		}

		// This means character needs to walk
		if (xframe2 > 0) {
			if (direccionmovimiento == 1 || direccionmovimiento == 3) {
				indicetray += 1;
			}
			indicetray += 1;
			// yframe2, xframe2 now represent max steps and current step
			if (yframe2 <= xframe2) {
				if (indicetray > longtray) {
					elemento1 = mainRoute[yframe2].x - mainRoute[yframe2 - 1].x;
					elemento2 = mainRoute[yframe2].y - mainRoute[yframe2 - 1].y;

					if ((abs(elemento1) > 2) || (abs(elemento2) > 2)) {
						if (abs(elemento2) > (abs(elemento1) + 5)) {
							if (elemento2 < 0)
								direccionmovimiento = 0;
							else
								direccionmovimiento = 2;
						} else {
							if (elemento1 > 0)
								direccionmovimiento = 1;
							else
								direccionmovimiento = 3;
						}
					}
					yframe2 += 1;
					calculateTrajectory(mainRoute[yframe2 - 1].x, mainRoute[yframe2 - 1].y);
				}
			} else if (indicetray >= longtray) {
				xframe2 = 0;
				if (!cambiopantalla) {
					elemento1 = (xraton + 7) - (characterPosX + rectificacionx);
					elemento2 = (yraton + 7) - (characterPosY + rectificaciony);
					if (abs(elemento2) > (abs(elemento1) + 20)) {
						if (elemento2 < 0)
							direccionmovimiento = 0;
						else
							direccionmovimiento = 2;
					} else {
						if (elemento1 > 0)
							direccionmovimiento = 1;
						else
							direccionmovimiento = 3;
					}
				}
			}
			iframe++;
			if (iframe > walkFrameCount)
				iframe = 0;
		} else {
			iframe = 0;
			if (zonavedada == false && !cambiopantalla) {
				elemento1 = (xraton + 7) - (characterPosX + rectificacionx);
				elemento2 = (yraton + 7) - (characterPosY + rectificaciony);
				if (abs(elemento2) > (abs(elemento1) + 20)) {
					if (elemento2 < 0)
						direccionmovimiento = 0;
					else
						direccionmovimiento = 2;
				} else {
					if (elemento1 > 0)
						direccionmovimiento = 1;
					else
						direccionmovimiento = 3;
				}
			}
		}
		characterPosX = trayec[indicetray].x;
		characterPosY = trayec[indicetray].y;

		updateMainCharacterDepth();

		if (pintaractivo) {
			sprites(true);
		}
		tocapintar = false;
		if (currentRoomData->paletteAnimationFlag && saltospal >= 4) {
			saltospal = 0;
			if (movidapaleta > 6)
				movidapaleta = 0;
			else
				movidapaleta += 1;
			if (currentRoomData->codigo == 4 && movidapaleta == 4)
				g_engine->_sound->playVoc();
			updatePalette(movidapaleta);
		} else {
			saltospal += 1;
		}
	}
}

void actionLineText(Common::String actionLine) {
	outtextxy(160, 144, actionLine, 255, true, Graphics::kTextAlignCenter);
}

void animateGive(uint cogedir, uint cogealt) {
	direccionmovimiento = cogedir;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		tocapintar = false;
		// Must add 1 to i because the original game uses 1-based indices
		iframe = 15 + 6 + 5 + cogealt * 10 - (i + 1);

		if (currentRoomData->paletteAnimationFlag && saltospal >= 4) {
			saltospal = 0;
			if (movidapaleta > 6)
				movidapaleta = 0;
			else
				movidapaleta += 1;
			updatePalette(movidapaleta);
		} else
			saltospal += 1;
		sprites(true);
	}
	iframe = 0;
}

// Lean over to pick
void animatePickup1(uint cogedir, uint cogealt) {
	direccionmovimiento = cogedir;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		tocapintar = false;
		iframe = 15 + cogealt * 10 + (i + 1);

		if (currentRoomData->paletteAnimationFlag && saltospal >= 4) {
			saltospal = 0;
			if (movidapaleta > 6)
				movidapaleta = 0;
			else
				movidapaleta += 1;
			updatePalette(movidapaleta);
		} else
			saltospal += 1;
		sprites(true);
		g_engine->_screen->update();
	}
}

// Lean back after pick
void animatePickup2(uint cogedir, uint cogealt) {
	direccionmovimiento = cogedir;

	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		tocapintar = false;

		iframe = 15 + 5 + cogealt * 10 + (i + 1);

		if (currentRoomData->paletteAnimationFlag && saltospal >= 4) {
			saltospal = 0;
			if (movidapaleta > 6)
				movidapaleta = 0;
			else
				movidapaleta += 1;
			updatePalette(movidapaleta);
		} else
			saltospal += 1;
		sprites(true);
		g_engine->_screen->update();
	}
	emptyLoop();
	tocapintar = false;
	sprites(true);
	iframe = 0;
}

void animateOpen2(uint cogedir, uint cogealt) {
	direccionmovimiento = cogedir;
	contadorpc = contadorpc2;
	for (uint i = 0; i < 5; i++) {
		emptyLoop();
		tocapintar = false;
		iframe = 15 + 6 + cogealt * 10 - (i + 1);

		if (currentRoomData->paletteAnimationFlag && saltospal >= 4) {
			saltospal = 0;
			if (movidapaleta > 6)
				movidapaleta = 0;
			else
				movidapaleta += 1;
			updatePalette(movidapaleta);
		} else
			saltospal += 1;
		sprites(true);
	}
	emptyLoop();
	tocapintar = false;
	sprites(true);
	iframe = 0;
}

void animateBat() {
	const uint trayseg[91][2] = {
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
	byte iframe2seg;
	uint anchoaniseg, altoaniseg,
		itrayseg, longtrayseg, xseg, yseg, profseg, dirseg;

	bool salir_del_bucle = false;
	if (currentRoomData->animationFlag) {
		iframe2seg = iframe2;
		xseg = animado.posx;
		yseg = animado.posy;
		anchoaniseg = anchoanimado;
		altoaniseg = altoanimado;
		profseg = animado.profundidad;
		dirseg = animado.dir;
		freeAnimation();
	}
	loadBat();
	g_engine->_sound->stopVoc();
	g_engine->_sound->playVoc("MURCIEL", 160848, 4474);
	itrayseg = 0;
	longtrayseg = 87;
	iframe2 = 0;
	animado.profundidad = 14;
	do {
		g_engine->_chrono->updateChrono();
		if (tocapintar) {
			if (itrayseg == longtrayseg)
				salir_del_bucle = true;
			itrayseg += 1;
			if (iframe2 >= secondaryAnimationFrameCount - 1)
				iframe2 = 0;
			else
				iframe2++;

			animado.posx = trayseg[itrayseg][0] - 20;
			animado.posy = trayseg[itrayseg][1];
			animado.dir = 0;
			sprites(true);
			tocapintar = false;
			if (itrayseg % 24 == 0)
				g_engine->_sound->playVoc();
			if (saltospal >= 4) {
				saltospal = 0;
				if (movidapaleta > 6)
					movidapaleta = 0;
				else
					movidapaleta += 1;
				updatePalette(movidapaleta);
			} else
				saltospal += 1;
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
	} while (!salir_del_bucle && !g_engine->shouldQuit());

	g_engine->_sound->stopVoc();
	freeAnimation();
	if (currentRoomData->animationFlag) {
		anchoanimado = anchoaniseg;
		altoanimado = altoaniseg;
		setRoomTrajectories(altoanimado, anchoanimado, RESTORE, false);
		loadAnimation(currentRoomData->nombremovto);
		iframe2 = iframe2seg;
		animado.posx = xseg;
		animado.posy = yseg;
		animado.profundidad = profseg;
		animado.dir = dirseg;
	}
	screenHandleToBackground();
	assembleScreen();
	drawScreen(background);
}

void updateVideo() {
	readBitmap(regobj.puntparche, screenObjects[regobj.profundidad - 1], regobj.tamparche, 319);
	screenHandleToBackground();
	assembleScreen();
	drawScreen(background);
}

void nicheAnimation(byte direccionhn, int32 bitmap) {
	uint posdibhn, indicehn;
	int incrementohn;

	// Room with Red
	if (currentRoomData->codigo == 24) {
		screenObjects[1] = (byte *)malloc(3660);
		readBitmap(1382874, screenObjects[1], 3652, 319);
		uint16 object1Width = READ_LE_UINT16(screenObjects[1]);
		uint16 object1Height = READ_LE_UINT16(screenObjects[1] + 2);
		depthMap[1].posx = 211;
		depthMap[1].posy = 16;
		depthMap[1].posx2 = 211 + object1Width + 1;
		depthMap[1].posy2 = 16 + object1Height + 1;
	}

	switch (direccionhn) {
	case 0: {
		posdibhn = 44904;
		incrementohn = 1;
		Common::copy(screenObjects[0], screenObjects[0] + 892, background + 44900);
		readBitmap(bitmap, screenObjects[0], 892, 319);
		Common::copy(screenObjects[0] + 4, screenObjects[0] + 4 + 888, background + 44900 + 892);
	} break;
	case 1: { // object slides to reveal empty stand
		posdibhn = 892 + 44900;
		incrementohn = -1;
		// Reads the empty alcove into a non-visible part of fondo
		readBitmap(bitmap, background + 44900, 892, 319);
		// Copies whatever is currently on the alcove in a non-visible part of fondo contiguous with the above
		Common::copy(screenObjects[0] + 4, screenObjects[0] + 4 + 888, background + 44900 + 892);
		// We now have in consecutive pixels the empty stand and the object

	} break;
	}
	uint16 alcoveWidth = READ_LE_UINT16(screenObjects[0]);
	uint16 alcoveHeight = READ_LE_UINT16(screenObjects[0] + 2);

	// Set the height to double to animate 2 images of the same height moving up/down
	*(background + 44900 + 2) = (alcoveHeight * 2) + 1;

	screenHandleToBackground();

	for (indicehn = 1; indicehn <= alcoveHeight; indicehn++) {

		posdibhn = posdibhn + (incrementohn * (alcoveWidth + 1));
		Common::copy(background + posdibhn, background + posdibhn + 888, screenObjects[0] + 4);
		assembleScreen();
		drawScreen(background);
		g_engine->_screen->update();
	}
	readBitmap(bitmap, screenObjects[0], 892, 319);

	screenHandleToBackground();
	assembleScreen();
	drawScreen(background);

	if (currentRoomData->codigo == 24) {
		free(screenObjects[1]);
		screenObjects[1] = NULL;
	}
}

void pickupScreenObject() {
	byte indicemochila, indicex, indicey;
	uint screenObject;

	uint mouseX = (pulsax + 7) / factorx;
	uint mouseY = (pulsay + 7) / factory;
	screenObject = currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[mouseX][mouseY]]->indicefichero;
	if (screenObject == 0)
		return;
	readItemRegister(screenObject);
	goToObject(
		currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory],
		currentRoomData->rejapantalla[mouseX][mouseY]);
	verifyCopyProtection();
	if (regobj.coger) {
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
			cavernas[1] = true;
			break;
		case 665: // Bird
			cavernas[0] = true;
			break;
		case 676:
		case 688: {
			cavernas[4] = true;
			vasijapuesta = false;
		} break;
		}
		switch (regobj.altura) {
		case 0: { // Pick up above
			switch (regobj.code) {
			case 590: { // Ectoplasm
				animatePickup1(3, 0);
				animatePickup2(3, 0);
			} break;
			default: {
				animatePickup1(direccionmovimiento, 0);
				screenObjects[regobj.profundidad - 1] = NULL;
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				animatePickup2(direccionmovimiento, 0);
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
				currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[mouseX][mouseY]]->indicefichero = regobj.reemplazarpor;
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
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[1];

					with.puntbitmap = 775611;
					with.tambitmap = 36;
					with.coordx = 80;
					with.coordy = 56;
					with.profund = 2;
					loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
				}
				{
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[2];

					with.puntbitmap = 730743;
					with.tambitmap = 64;
					with.coordx = 76;
					with.coordy = 62;
					with.profund = 1;
					loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
				}
				screenObjects[regobj.profundidad - 1] = NULL;
				screenHandleToBackground();

				assembleScreen();
				drawScreen(background);
				animatePickup2(0, 1);
			} break;
			case 562: { // alcove
				switch (currentRoomData->codigo) {
				case 20: { // First scene with alcove
					if (hornacina[0][hornacina[0][3]] > 0) {
						// Possibly
						if (hornacina[0][3] == 2 || hornacina[0][hornacina[0][3]] == 563) {
							readItemRegister(hornacina[0][hornacina[0][3]]);
							hornacina[0][hornacina[0][3]] = 0;
							currentRoomData->indexadoobjetos[9]->objectName = getObjectName(4);
							animatePickup1(3, 1);
							readBitmap(1190768, screenObjects[regobj.profundidad - 1], 892, 319);
							currentRoomData->bitmapasociados[1].puntbitmap = 1190768;
							currentRoomData->bitmapasociados[1].tambitmap = 892;
							currentRoomData->bitmapasociados[1].coordx = 66;
							currentRoomData->bitmapasociados[1].coordy = 35;
							currentRoomData->bitmapasociados[1].profund = 1;
							screenHandleToBackground();
							assembleScreen();
							drawScreen(background);
							animatePickup2(3, 1);
						} else {
							readItemRegister(hornacina[0][hornacina[0][3]]);
							hornacina[0][hornacina[0][3]] = 0;
							hornacina[0][3] += 1;
							hornacina[1][3] -= 1;
							currentRoomData->indexadoobjetos[9]->objectName = "                    ";
							animatePickup1(3, 1);
							readBitmap(1190768, screenObjects[regobj.profundidad - 1],
									   892, 319);
							screenHandleToBackground();
							assembleScreen();
							drawScreen(background);
							animatePickup2(3, 1);
							g_engine->_sound->playVoc("PLATAF", 375907, 14724);
							currentRoomData->bitmapasociados[1].tambitmap = 892;
							currentRoomData->bitmapasociados[1].coordx = 66;
							currentRoomData->bitmapasociados[1].coordy = 35;
							currentRoomData->bitmapasociados[1].profund = 1;
							switch (hornacina[0][hornacina[0][3]]) {
							case 0: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(4);
								nicheAnimation(0, 1190768);
								currentRoomData->bitmapasociados[1].puntbitmap = 1190768;
							} break;
							case 561: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(5);
								nicheAnimation(0, 1182652);
								currentRoomData->bitmapasociados[1].puntbitmap = 1182652;
							} break;
							case 563: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(6);
								nicheAnimation(0, 1186044);
								currentRoomData->bitmapasociados[1].puntbitmap = 1186044;
							} break;
							case 615: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(7);
								nicheAnimation(0, 1181760);
								currentRoomData->bitmapasociados[1].puntbitmap = 1181760;
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
				case 24: { // Second scene with alcove
					if (hornacina[1][hornacina[1][3]] > 0 && hornacina[1][3] != 1) {
						if (hornacina[1][3] == 2) {
							readItemRegister(hornacina[1][2]);
							hornacina[1][2] = 0;
							currentRoomData->indexadoobjetos[8]->objectName = getObjectName(4);
							animatePickup1(0, 1);
							readBitmap(1399610, screenObjects[regobj.profundidad - 1], 892, 319);
							currentRoomData->bitmapasociados[0].puntbitmap = 1399610;
							currentRoomData->bitmapasociados[0].tambitmap = 892;
							currentRoomData->bitmapasociados[0].coordx = 217;
							currentRoomData->bitmapasociados[0].coordy = 48;
							currentRoomData->bitmapasociados[0].profund = 1;
							screenHandleToBackground();
							assembleScreen();
							drawScreen(background);
							animatePickup2(0, 1);
						} else {
							readItemRegister(hornacina[1][hornacina[1][3]]);
							hornacina[1][hornacina[1][3]] = 622;
							hornacina[1][3] += 1;
							hornacina[0][3] -= 1;
							currentRoomData->indexadoobjetos[8]->objectName = "                    ";
							animatePickup1(0, 1);
							readBitmap(1399610, screenObjects[0], 892, 319);
							screenHandleToBackground();
							assembleScreen();
							drawScreen(background);
							animatePickup2(0, 1);
							g_engine->_sound->playVoc("PLATAF", 375907, 14724);
							currentRoomData->bitmapasociados[0].tambitmap = 892;
							currentRoomData->bitmapasociados[0].coordx = 217;
							currentRoomData->bitmapasociados[0].coordy = 48;
							currentRoomData->bitmapasociados[0].profund = 1;
							switch (hornacina[1][hornacina[1][3]]) {
							case 0: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(4);
								nicheAnimation(0, 1399610);
								currentRoomData->bitmapasociados[0].puntbitmap = 1399610;
							} break;
							case 561: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(5);
								nicheAnimation(0, 1381982);
								currentRoomData->bitmapasociados[0].puntbitmap = 1381982;
							} break;
							case 615: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(7);
								nicheAnimation(0, 1381090);
								currentRoomData->bitmapasociados[0].puntbitmap = 1381090;
							} break;
							case 622: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(8);
								nicheAnimation(0, 1400502);
								currentRoomData->bitmapasociados[0].puntbitmap = 1400502;
							} break;
							case 623: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(9);
								nicheAnimation(0, 1398718);
								currentRoomData->bitmapasociados[0].puntbitmap = 1398718;
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
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[3];

					with.puntbitmap = 0;
					with.tambitmap = 0;
					with.coordx = 0;
					with.coordy = 0;
					with.profund = 0;
				}
				screenObjects[3] = NULL;
				disableSecondAnimation();
				drawScreen(background);
				animatePickup2(2, 1);
				rojo_capturado = true;
				trampa_puesta = false;
			} break;
			default: {
				animatePickup1(direccionmovimiento, 1);
				screenObjects[regobj.profundidad - 1] = NULL;
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				animatePickup2(direccionmovimiento, 1);
			}
			}
		} break;
		case 2: { // Pick up feet level
			switch (regobj.code) {
			case 216: { // chisel
				animatePickup1(0, 2);
				currentRoomData->indexadoobjetos[currentRoomData->mouseGrid
													 [mouseX][mouseY]]
					->indicefichero = regobj.reemplazarpor;
				updateVideo();
				animatePickup2(0, 2);
			} break;
			case 295: { // candles
				animatePickup1(3, 2);
				currentRoomData->indexadoobjetos[currentRoomData->mouseGrid
													 [mouseX][mouseY]]
					->indicefichero = regobj.reemplazarpor;
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
				screenObjects[regobj.profundidad - 1] = NULL;
				{ // bird
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[2];

					with.puntbitmap = 1545924;
					with.tambitmap = 172;
					with.coordx = 38;
					with.coordy = 58;
					with.profund = 1;
					loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
				}
				{ // ring
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[1];

					with.puntbitmap = 1591272;
					with.tambitmap = 92;
					with.coordx = 50;
					with.coordy = 58;
					with.profund = 3;
					loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
				}
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				animatePickup2(3, 2);
			} break;
			default: {
				animatePickup1(direccionmovimiento, 2);
				screenObjects[regobj.profundidad - 1] = NULL;
				assembleScreen();
				drawScreen(background);
				animatePickup2(direccionmovimiento, 2);
			}
			}
		} break;
		case 9: { // bat
			kaka = regobj.pickTextRef;
			readItemRegister(204);
			animatePickup1(0, 1);
			animateOpen2(0, 1);
			animateBat();
			g_engine->_mouseManager->show();
			drawText(kaka);
			currentRoomData->mouseGrid[34][8] = 24;
			numeroaccion = 0;
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
			return;
		} break;
		}
		g_engine->_mouseManager->show();

		if (regobj.code != 624)
			for (indicey = regobj.yrej1; indicey <= regobj.yrej2; indicey++)
				for (indicex = regobj.xrej1; indicex <= regobj.xrej2; indicex++) {
					currentRoomData->rejapantalla[indicex][indicey] = regobj.parcherejapantalla[indicex - regobj.xrej1][indicey - regobj.yrej1];
					currentRoomData->mouseGrid[indicex][indicey] = regobj.parcherejaraton[indicex - regobj.xrej1][indicey - regobj.yrej1];
				}
		switch (regobj.code) {
		case 216: { // chisel
			currentRoomData->bitmapasociados[5].puntbitmap = 517485;
			currentRoomData->bitmapasociados[5].tambitmap = 964;
			currentRoomData->bitmapasociados[5].coordx = 223;
			currentRoomData->bitmapasociados[5].coordy = 34;
			currentRoomData->bitmapasociados[5].profund = 1;
		} break;
		case 218:; // necronomicon
			break;
		case 223: { // table cloth
			currentRoomData->bitmapasociados[6].puntbitmap = 436752;
			currentRoomData->bitmapasociados[6].tambitmap = 1372;
			currentRoomData->bitmapasociados[6].coordx = 174;
			currentRoomData->bitmapasociados[6].coordy = 32;
			currentRoomData->bitmapasociados[6].profund = 1;
		} break;
		case 295: { // candles
			currentRoomData->bitmapasociados[3].puntbitmap = 1130756;
			currentRoomData->bitmapasociados[3].tambitmap = 1764;
			currentRoomData->bitmapasociados[3].coordx = 100;
			currentRoomData->bitmapasociados[3].coordy = 28;
			currentRoomData->bitmapasociados[3].profund = 1;
		} break;
		case 308:; // mistletoe
			break;
		case 362:; // charcoal
			break;
		case 402: {
			currentRoomData->bitmapasociados[5].puntbitmap = 68130;
			currentRoomData->bitmapasociados[5].tambitmap = 2564;
			currentRoomData->bitmapasociados[5].coordx = 148;
			currentRoomData->bitmapasociados[5].coordy = 49;
			currentRoomData->bitmapasociados[5].profund = 7;
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
			for (indicex = 0; indicex < 15; indicex++)
				if (currentRoomData->bitmapasociados[indicex].puntbitmap ==
					regobj.punterobitmap) {
					currentRoomData->bitmapasociados[indicex].puntbitmap = 0;
					currentRoomData->bitmapasociados[indicex].tambitmap = 0;
					currentRoomData->bitmapasociados[indicex].coordx = 0;
					currentRoomData->bitmapasociados[indicex].coordy = 0;
					currentRoomData->bitmapasociados[indicex].profund = 0;
				}
		}
		}
	} else {
		if (regobj.pickTextRef > 0)
			drawText(regobj.pickTextRef);
		numeroaccion = 0;
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
		return;
	}
	indicemochila = 0;

	while (mobj[indicemochila].code != 0) {
		indicemochila += 1;
	}

	mobj[indicemochila].bitmapIndex = regobj.objectIconBitmap;
	mobj[indicemochila].code = regobj.code;
	mobj[indicemochila].objectName = regobj.name;
	g_engine->_mouseManager->hide();
	drawBackpack();
	g_engine->_mouseManager->show();
	if (regobj.pickTextRef > 0)
		drawText(regobj.pickTextRef);
	numeroaccion = 0;
	oldxrejilla = 0;
	oldyrejilla = 0;
	checkMouseGrid();
}

void replaceBackpack(byte indicador1, uint indicador2) {
	readItemRegister(indicador2);
	mobj[indicador1].bitmapIndex = regobj.objectIconBitmap;
	mobj[indicador1].code = indicador2;
	mobj[indicador1].objectName = regobj.name;
	contadorpc = contadorpc2;
}

void dropObjectInScreen(InvItemRegister regobjsustituto) {
	byte indicepaso, indicex, indicey;

	if (regobjsustituto.tambitmap > 0) {
		indicepaso = 0;
		while (!(currentRoomData->bitmapasociados[indicepaso].tambitmap == 0) || indicepaso == 15) {
			indicepaso++;
		}
		if (currentRoomData->bitmapasociados[indicepaso].tambitmap == 0) {
			{
				RoomBitmapRegister &with = currentRoomData->bitmapasociados[indicepaso];

				with.puntbitmap = regobjsustituto.punterobitmap;
				with.tambitmap = regobjsustituto.tambitmap;
				with.coordx = regobjsustituto.xparche;
				with.coordy = regobjsustituto.yparche;
				with.profund = regobjsustituto.profundidad;
				loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
			}
			for (indicey = regobjsustituto.yrej1; indicey <= regobjsustituto.yrej2; indicey++)
				for (indicex = regobjsustituto.xrej1; indicex <= regobjsustituto.xrej2; indicex++) {
					currentRoomData->rejapantalla[indicex][indicey] = regobjsustituto.parcherejapantalla[indicex - regobjsustituto.xrej1][indicey - regobjsustituto.yrej1];
					currentRoomData->mouseGrid[indicex][indicey] = regobjsustituto.parcherejaraton[indicex - regobjsustituto.xrej1][indicey - regobjsustituto.yrej1];
				}
		} else
			showError(264);
	}
}

void useScreenObject() {
	byte indicemochila, indlista, aciertoslista, indmoch,
		indicex, indicey;
	bool controlarlista, basurillalog;

	uint mouseX = (pulsax + 7) / factorx;
	uint mouseY = (pulsay + 7) / factory;
	uint screenObject = currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[mouseX][mouseY]]->indicefichero;

	if (objetomochila != "") {
		indicemochila = 0;
		while (mobj[indicemochila].objectName != objetomochila) {
			indicemochila += 1;
		}

		readItemRegister(mobj[indicemochila].code);

		goToObject(
			currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory],
			currentRoomData->rejapantalla[mouseX][mouseY]);

		if (regobj.usarcon == screenObject && screenObject > 0 && regobj.usar[0] == 5) {
			switch (regobj.usarcon) {
			case 30: { // corn with rooster
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();

				animateGive(1, 2);
				animatePickup2(1, 2);

				updateItem(regobj.code);
				currentRoomData->indexadoobjetos[27]->indicefichero = 201;
				do {
					g_engine->_chrono->updateChrono();
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					if (indicetray2 >= currentRoomData->longtray2)
						indicetray2 = 1;
					else
						indicetray2 += 1;
					animado.dir = currentRoomData->dir2[indicetray2 - 1];
					animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
					animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
					emptyLoop();
					tocapintar = false;
					emptyLoop2();
					sprites(true);
					g_engine->_screen->update();
				} while (indicetray2 != 4);

				for (indlista = 0; indlista < maxrejax; indlista++)
					for (indmoch = 0; indmoch < maxrejay; indmoch++) {
						currentRoomData->rejapantalla[oldposx + indlista][oldposy + indmoch] = rejafondomovto[indlista][indmoch];
						currentRoomData->mouseGrid[oldposx + indlista][oldposy + indmoch] = rejafondoraton[indlista][indmoch];
					}

				freeAnimation();
				animado.posx = currentRoomData->tray2[indicetray2 - 1].x + 8;
				animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
				currentRoomData->nombremovto = "GALLOPIC";
				currentRoomData->dir2[299] = 201;
				loadAnimation("GALLOPIC");
				currentRoomData->dir2[0] = 0;
				currentRoomData->tray2[0].x = animado.posx;
				currentRoomData->tray2[0].y = animado.posy;
				indicetray2 = 1;
				currentRoomData->longtray2 = 1;

				for (indlista = 0; indlista < maxrejax; indlista++)
					for (indmoch = 0; indmoch < maxrejay; indmoch++) {
						if (rejamascaramovto[indlista][indmoch] > 0)
							currentRoomData->rejapantalla[oldposx + indlista][oldposy + indmoch] = rejamascaramovto[indlista][indmoch];
						if (rejamascararaton[indlista][indmoch] > 0)
							currentRoomData->mouseGrid[oldposx + indlista][oldposy + indmoch] = rejamascararaton[indlista][indmoch];
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
				currentRoomData->indexadoobjetos[21]->indicefichero = 154;
			} break;
			case 157: { // giving something to john
				controlarlista = false;
				debug("used object = %d", mobj[indicemochila].code);
				if (lista1) {
					for (indlista = 0; indlista < 5; indlista++) {
						if (mobj[indicemochila].code == firstList[indlista])
							controlarlista = true;
					}
					if (controlarlista) {
						aciertoslista = 0;
						for (indlista = 0; indlista < 5; indlista++) {
							for (indmoch = 0; indmoch < inventoryIconCount; indmoch++) {
								if (mobj[indmoch].code == firstList[indlista]) {
									aciertoslista += 1;
									break;
								}
							}
						}
						if (aciertoslista == 5) {
							lista1 = false;
							list1Complete = true;
							assignText();
							hypertext(1018, 255, 0, kaka, false);
							hypertext(kaka, 253, 249, kaka, true);
							verb.close();

							g_engine->_mouseManager->hide();
							animateGive(direccionmovimiento, 1);
							animateOpen2(direccionmovimiento, 1);
							for (indlista = 0; indlista < 5; indlista++) {
								indmoch = 0;
								while (mobj[indmoch].code != firstList[indlista]) {
									indmoch += 1;
								}
								updateInventory(indmoch);
							}
							indlista = 0;
							while (mobj[indlista].code != 149) {
								indlista += 1;
							}
							updateInventory(indlista);
							drawBackpack();
							g_engine->_mouseManager->show();
						} else {
							assignText();
							hypertext(regobj.useTextRef, 255, 0, kaka, false);
							g_engine->_mouseManager->hide();
							animateGive(direccionmovimiento, 1);
							animateOpen2(direccionmovimiento, 1);
							g_engine->_mouseManager->show();
							hypertext(kaka, 253, 249, kaka, true);
							verb.close();
						}
					}
				}
				controlarlista = false;
				if (lista2) {
					for (indlista = 0; indlista < 5; indlista++)
						if (mobj[indicemochila].code == secondList[indlista])
							controlarlista = true;
					if (controlarlista) {
						aciertoslista = 0;
						for (indlista = 0; indlista < 5; indlista++) {
							for (indmoch = 0; indmoch <= inventoryIconCount; indmoch++) {
								if (mobj[indmoch].code == secondList[indlista]) {
									aciertoslista += 1;
									break;
								}
							}
						}
						if (aciertoslista == 5) {
							lista2 = false;
							list2Complete = true;
							assignText();
							hypertext(1020, 255, 0, kaka, false);
							hypertext(kaka, 253, 249, kaka, true);
							verb.close();
							g_engine->_mouseManager->hide();
							animateGive(direccionmovimiento, 1);
							animateOpen2(direccionmovimiento, 1);
							for (indlista = 0; indlista < 5; indlista++) {
								indmoch = 0;
								while (mobj[indmoch].code != secondList[indlista]) {
									indmoch += 1;
								}
								updateInventory(indmoch);
							}
							indlista = 0;
							while (mobj[indlista].code != 150) {
								indlista += 1;
							}
							updateInventory(indlista);
							drawBackpack();
							g_engine->_mouseManager->show();
						} else {
							assignText();
							hypertext(regobj.useTextRef, 255, 0, kaka, false);
							g_engine->_mouseManager->hide();
							animateGive(direccionmovimiento, 1);
							animateOpen2(direccionmovimiento, 1);
							g_engine->_mouseManager->show();
							hypertext(kaka, 253, 249, kaka, true);
							verb.close();
						}
					}
				}
			} break;
			case 159: {
				switch (regobj.code) {
				case 173: {
					assignText();
					hypertext(1118, 255, 0, kaka, false);
					g_engine->_mouseManager->hide();
					animateGive(direccionmovimiento, 1);
					animateOpen2(direccionmovimiento, 1);
					g_engine->_mouseManager->show();
					hypertext(kaka, 253, 249, kaka, true);
					verb.close();
				} break;
				case 218: {
					assignText();
					hypertext(687, 255, 0, kaka, false);
					g_engine->_mouseManager->hide();
					animateGive(direccionmovimiento, 1);
					animateOpen2(direccionmovimiento, 1);
					g_engine->_mouseManager->show();
					hypertext(kaka, 253, 249, kaka, true);
					verb.close();
					g_engine->_mouseManager->hide();
					updateInventory(indicemochila);
					drawBackpack();
					g_engine->_mouseManager->show();
					for (kaka = 0; kaka < maxpersonajes; kaka++)
						libro[kaka] = true;
					lprimera[0] = true;
				} break;
				}
			} break;
			case 160: {
				assignText();
				hypertext(2466, 255, 0, kaka, false);
				g_engine->_mouseManager->hide();
				animateGive(direccionmovimiento, 1);
				animateOpen2(direccionmovimiento, 1);
				g_engine->_mouseManager->show();
				hypertext(kaka, 253, 249, kaka, true);
				verb.close();
				g_engine->_mouseManager->hide();
				updateInventory(indicemochila);
				drawBackpack();
				g_engine->_mouseManager->show();
				for (kaka = 0; kaka < maxpersonajes; kaka++) {
					caramelos[kaka] = true;
					cprimera[kaka] = true;
				}
			} break;
			case 164: {
				switch (regobj.code) {
				case 563: {
					manual_torno = true;
					if (torno && diablillo_verde && barro) {
						assignText();
						hypertext(2696, 255, 0, kaka, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(kaka, 253, 249, kaka, true);
						cavernas[4] = true;
						hypertext(3247, 253, 249, kaka, true);
						verb.close();
						g_engine->_mouseManager->hide();
						replaceBackpack(indicemochila, 676);
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = currentRoomData->bitmapasociados[0];

							with.puntbitmap = 1545820;
							with.tambitmap = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.profund = 1;
							loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
						}
						currentRoomData->mouseGrid[15][12] = 7;
						g_engine->_mouseManager->show();
					} else {
						assignText();
						hypertext(2696, 255, 0, kaka, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(kaka, 253, 249, kaka, true);
						hypertext(3246, 253, 249, kaka, true);
						verb.close();
						g_engine->_mouseManager->hide();
						updateInventory(indicemochila);
						drawBackpack();
						g_engine->_mouseManager->show();
					}
				} break;
				case 598: {
					barro = true;
					if (torno && diablillo_verde && manual_torno) {
						assignText();
						hypertext(2821, 255, 0, kaka, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(kaka, 253, 249, kaka, true);
						cavernas[4] = true;
						hypertext(3247, 253, 249, kaka, true);
						verb.close();
						g_engine->_mouseManager->hide();
						replaceBackpack(indicemochila, 676);
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = currentRoomData->bitmapasociados[0];

							with.puntbitmap = 1545820;
							with.tambitmap = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.profund = 1;
							loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
						}
						currentRoomData->mouseGrid[15][12] = 7;
						g_engine->_mouseManager->show();
					} else {
						assignText();
						hypertext(2821, 255, 0, kaka, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(kaka, 253, 249, kaka, true);
						hypertext(3246, 253, 249, kaka, true);
						verb.close();
						g_engine->_mouseManager->hide();
						updateInventory(indicemochila);
						drawBackpack();
						g_engine->_mouseManager->show();
					}
				} break;
				case 623: {
					torno = true;
					if (barro && diablillo_verde && manual_torno) {
						assignText();
						hypertext(2906, 255, 0, kaka, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(kaka, 253, 249, kaka, true);
						cavernas[4] = true;
						hypertext(3247, 253, 249, kaka, true);
						verb.close();
						g_engine->_mouseManager->hide();
						replaceBackpack(indicemochila, 676);
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = currentRoomData->bitmapasociados[0];

							with.puntbitmap = 1545820;
							with.tambitmap = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.profund = 1;
							loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
						}
						currentRoomData->mouseGrid[15][12] = 7;
						g_engine->_mouseManager->show();
					} else {
						assignText();
						hypertext(2906, 255, 0, kaka, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(kaka, 253, 249, kaka, true);
						hypertext(3246, 253, 249, kaka, true);
						verb.close();
						g_engine->_mouseManager->hide();
						updateInventory(indicemochila);
						drawBackpack();
						g_engine->_mouseManager->show();
					}
				} break;
				case 701: {
					diablillo_verde = true;
					if (barro && torno && manual_torno) {
						assignText();
						hypertext(3188, 255, 0, kaka, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animatePickup2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(kaka, 253, 249, kaka, true);
						cavernas[4] = true;
						hypertext(3247, 253, 249, kaka, true);
						verb.close();
						g_engine->_mouseManager->hide();
						replaceBackpack(indicemochila, 676);
						drawBackpack();
						disableSecondAnimation();
						{
							RoomBitmapRegister &with = currentRoomData->bitmapasociados[0];

							with.puntbitmap = 1545820;
							with.tambitmap = 104;
							with.coordx = 120;
							with.coordy = 55;
							with.profund = 1;
							loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
						}
						currentRoomData->mouseGrid[15][12] = 7;
						g_engine->_mouseManager->show();
					} else {
						assignText();
						hypertext(3188, 255, 0, kaka, false);
						g_engine->_mouseManager->hide();
						animateGive(0, 1);
						animateOpen2(0, 1);
						g_engine->_mouseManager->show();
						hypertext(kaka, 253, 249, kaka, true);
						hypertext(3246, 253, 249, kaka, true);
						verb.close();
						g_engine->_mouseManager->hide();
						updateInventory(indicemochila);
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
					if (indicetray2 >= currentRoomData->longtray2)
						indicetray2 = 1;
					else
						indicetray2 += 1;
					animado.dir = currentRoomData->dir2[indicetray2 - 1];
					animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
					animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
					emptyLoop();
					tocapintar = false;
					emptyLoop2();
					sprites(true);
				} while (!(indicetray2 == (currentRoomData->longtray2 / 2)));

				animateGive(3, 2);
				updateInventory(indicemochila);
				drawBackpack();
				animateOpen2(3, 2);
				animatedSequence(6);

				do {
					if (iframe2 >= secondaryAnimationFrameCount - 1)
						iframe2 = 0;
					else
						iframe2++;
					if (indicetray2 >= currentRoomData->longtray2)
						indicetray2 = 1;
					else
						indicetray2 += 1;
					animado.dir = currentRoomData->dir2[indicetray2 - 1];
					animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
					animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
					emptyLoop();
					tocapintar = false;

					emptyLoop2();
					sprites(true);
				} while (indicetray2 != currentRoomData->longtray2);
				disableSecondAnimation();
				drawScreen(background);
				g_engine->_mouseManager->show();
			} break;
			case 201: {
				drawText(regobj.useTextRef);
				g_engine->_sound->playVoc("GALLO", 94965, 46007);
				g_engine->_mouseManager->hide();
				animatedSequence(5);
				replaceBackpack(indicemochila, 423);
				drawBackpack();
				g_engine->_mouseManager->show();
				numeroaccion = 0;
				oldxrejilla = 0;
				oldyrejilla = 0;
				checkMouseGrid();
			} break;
			case 219: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("TAZA", 223698, 29066);
				animateGive(3, 2);
				do {
					g_engine->_chrono->updateChrono();
					if (tocapintar) {
						if (saltospal >= 4) {
							saltospal = 0;
							if (movidapaleta > 6)
								movidapaleta = 0;
							else
								movidapaleta += 1;
							updatePalette(movidapaleta);
						} else
							saltospal += 1;
						tocapintar = false;
					}
					g_engine->_screen->update();
					g_system->delayMillis(10);
				} while (g_engine->_sound->isVocPlaying());
				animateOpen2(3, 2);
				updateItem(regobj.code);
				disableSecondAnimation();
				drawScreen(background);
				g_engine->_mouseManager->show();
				drawText(2652);
				g_engine->_mouseManager->hide();
				readItemRegister(536);
				for (indicex = 12; indicex <= 13; indicex++)
					for (indicey = 7; indicey <= 14; indicey++)
						currentRoomData->mouseGrid[indicex][indicey] = 14;
				for (indicey = 8; indicey <= 12; indicey++)
					currentRoomData->mouseGrid[14][indicey] = 14;
				currentRoomData->mouseGrid[9][10] = 1;
				currentRoomData->mouseGrid[10][10] = 1;
				for (indicex = 0; indicex < 15; indicex++)
					if (currentRoomData->bitmapasociados[indicex].puntbitmap ==
						regobj.punterobitmap) {
						currentRoomData->bitmapasociados[indicex].puntbitmap = 0;
						currentRoomData->bitmapasociados[indicex].tambitmap = 0;
						currentRoomData->bitmapasociados[indicex].coordx = 0;
						currentRoomData->bitmapasociados[indicex].coordy = 0;
						currentRoomData->bitmapasociados[indicex].profund = 0;
					}
				indicemochila = 0;
				while (mobj[indicemochila].code != 0) {
					indicemochila += 1;
				}
				mobj[indicemochila].bitmapIndex = regobj.objectIconBitmap;
				mobj[indicemochila].code = regobj.code;
				mobj[indicemochila].objectName = regobj.name;
				animatedSequence(4);
				g_engine->_mouseManager->show();
				numeroaccion = 0;
				oldxrejilla = 0;
				oldyrejilla = 0;
				checkMouseGrid();
			} break;
			case 221: {
				drawText(regobj.useTextRef);
				regobj.usar[0] = 9;
				indicemochila = 0;
				while (mobj[indicemochila].code != 0) {
					indicemochila += 1;
				}

				invItemData->seek(regobj.code);

				saveItem(regobj, invItemData);
				readItemRegister(invItemData, 221, regobj);
				mobj[indicemochila].bitmapIndex = regobj.objectIconBitmap;
				mobj[indicemochila].code = regobj.code;
				mobj[indicemochila].objectName = regobj.name;

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
				replaceBackpack(indicemochila, 453);
				drawBackpack();
				g_engine->_mouseManager->show();
				updateItem(mobj[indicemochila].code);
				alacena_abierta = true;
			} break;
			case 274: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("CINCEL", 334779, 19490);
				animatePickup1(direccionmovimiento, 2);
				g_engine->_sound->waitForSoundEnd();
				animateOpen2(direccionmovimiento, 2);
				g_engine->_mouseManager->show();
				updateItem(regobj.code);
				baul_abierto = true;
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
				currentRoomData->doors[2].abiertacerrada = 0;
			} break;
			case 446: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("TAZA", 223698, 29066);
				animateGive(0, 2);
				g_engine->_sound->waitForSoundEnd();
				animatePickup2(0, 2);
				replaceBackpack(indicemochila, 204);
				drawBackpack();
				g_engine->_mouseManager->show();
			} break;
			case 507: {
				g_engine->_mouseManager->hide();
				animatePickup1(0, 1);
				g_engine->_sound->playVoc("MAQUINA", 153470, 7378);
				animateOpen2(0, 1);
				updateInventory(indicemochila);
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
				currentRoomData->doors[0].abiertacerrada = 0;
			} break;
			case 562: { // put any object in the alcoves
				switch (currentRoomData->codigo) {
				case 20: {
					if (hornacina[0][hornacina[0][3]] == 0) {

						if (hornacina[0][3] == 0) {
							hornacina[0][0] = regobj.code;
							drawText(regobj.useTextRef);
							g_engine->_mouseManager->hide();
							currentRoomData->indexadoobjetos[9]->objectName = "                    ";
							animateGive(3, 1);
							switch (hornacina[0][0]) {
							case 561: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(5);
								readBitmap(1182652, screenObjects[0], 892, 319);
								currentRoomData->bitmapasociados[1].puntbitmap = 1182652;
							} break;
							case 615: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(7);
								readBitmap(1181760, screenObjects[0], 892, 319);
								currentRoomData->bitmapasociados[1].puntbitmap = 1181760;
							} break;
							}
							currentRoomData->bitmapasociados[1].tambitmap = 892;
							currentRoomData->bitmapasociados[1].coordx = 66;
							currentRoomData->bitmapasociados[1].coordy = 35;
							currentRoomData->bitmapasociados[1].profund = 1;
							screenHandleToBackground();
							// XMStoPointer(ptr(segfondo, (offfondo + 4)), _handpantalla, 4, (sizepantalla - int32(4)));
							assembleScreen();
							drawScreen(background);
							animateOpen2(3, 1);
							updateInventory(indicemochila);
							drawBackpack();
							g_engine->_mouseManager->show();
						} else {

							hornacina[0][hornacina[0][3]] = regobj.code;
							hornacina[1][3] += 1;
							hornacina[0][3] -= 1;
							drawText(regobj.useTextRef);
							g_engine->_mouseManager->hide();
							animateGive(3, 1);
							switch (regobj.code) {
							case 561:
								readBitmap(1182652, screenObjects[0], 892, 319);
								break;
							case 615:
								readBitmap(1181760, screenObjects[0], 892, 319);
								break;
							}
							screenHandleToBackground();
							assembleScreen();
							drawScreen(background);
							animateOpen2(3, 1);
							updateInventory(indicemochila);
							drawBackpack();
							currentRoomData->indexadoobjetos[9]->objectName = "                    ";
							g_engine->_sound->playVoc("PLATAF", 375907, 14724);
							switch (hornacina[0][hornacina[0][3]]) {
							case 0: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(4);
								nicheAnimation(1, 1190768);
								currentRoomData->bitmapasociados[1].puntbitmap = 1190768;
							} break;
							case 561: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(5);
								nicheAnimation(1, 1182652);
								currentRoomData->bitmapasociados[1].puntbitmap = 1182652;
							} break;
							case 563: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(6);
								nicheAnimation(1, 1186044);
								currentRoomData->bitmapasociados[1].puntbitmap = 1186044;
							} break;
							case 615: {
								currentRoomData->indexadoobjetos[9]->objectName = getObjectName(7);
								nicheAnimation(1, 1181760);
								currentRoomData->bitmapasociados[1].puntbitmap = 1181760;
							} break;
							}
							currentRoomData->bitmapasociados[1].tambitmap = 892;
							currentRoomData->bitmapasociados[1].coordx = 66;
							currentRoomData->bitmapasociados[1].coordy = 35;
							currentRoomData->bitmapasociados[1].profund = 1;
							g_engine->_mouseManager->show();
							updateAltScreen(24);
						}
					} else {
						drawText(Random(11) + 1022);
					}
				} break;
				case 24: {
					if (hornacina[1][hornacina[1][3]] == 0) {

						if (hornacina[1][3] == 0) {

							hornacina[1][0] = regobj.code;
							drawText(regobj.useTextRef);
							g_engine->_mouseManager->hide();
							currentRoomData->indexadoobjetos[8]->objectName = "                    ";
							animateGive(0, 1);
							switch (hornacina[1][0]) {
							case 561: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(5);
								readBitmap(1381982, screenObjects[0], 892, 319);
								currentRoomData->bitmapasociados[0].puntbitmap = 1381982;
							} break;
							case 615: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(7);
								readBitmap(1381090, screenObjects[0], 892, 319);
								currentRoomData->bitmapasociados[0].puntbitmap = 1381090;
							} break;
							}
							currentRoomData->bitmapasociados[0].tambitmap = 892;
							currentRoomData->bitmapasociados[0].coordx = 217;
							currentRoomData->bitmapasociados[0].coordy = 48;
							currentRoomData->bitmapasociados[0].profund = 1;
							screenHandleToBackground();
							// XMStoPointer(ptr(segfondo, (offfondo + 4)), _handpantalla, 4, (sizepantalla - int32(4)));
							assembleScreen();
							drawScreen(background);
							animateOpen2(0, 1);
							updateInventory(indicemochila);
							drawBackpack();
							g_engine->_mouseManager->show();
						} else {

							hornacina[1][hornacina[1][3]] = regobj.code;
							hornacina[0][3] += 1;
							hornacina[1][3] -= 1;
							drawText(regobj.useTextRef);
							g_engine->_mouseManager->hide();
							animateGive(0, 1);

							switch (regobj.code) {
							case 561:
								readBitmap(1381982, screenObjects[regobj.profundidad - 1],
										   892, 319);
								break;
							case 615:
								readBitmap(1381090, screenObjects[regobj.profundidad - 1],
										   892, 319);
								break;
							}
							screenHandleToBackground();
							// XMStoPointer(ptr(segfondo, (offfondo + 4)), _handpantalla, 4, (sizepantalla - int32(4)));
							assembleScreen();
							drawScreen(background);
							animateOpen2(0, 1);
							updateInventory(indicemochila);
							drawBackpack();
							currentRoomData->indexadoobjetos[8]->objectName = "                    ";
							g_engine->_sound->playVoc("PLATAF", 375907, 14724);
							switch (hornacina[1][hornacina[1][3]]) {
							case 0: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(4);
								nicheAnimation(1, 1399610);
								currentRoomData->bitmapasociados[0].puntbitmap = 1399610;
							} break;
							case 561: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(5);
								nicheAnimation(1, 1381982);
								currentRoomData->bitmapasociados[0].puntbitmap = 1381982;
							} break;
							case 615: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(7);
								nicheAnimation(1, 1381090);
								currentRoomData->bitmapasociados[0].puntbitmap = 1381090;
							} break;
							case 622: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(8);
								nicheAnimation(1, 1400502);
								currentRoomData->bitmapasociados[0].puntbitmap = 1400502;
							} break;
							case 623: {
								currentRoomData->indexadoobjetos[8]->objectName = getObjectName(9);
								nicheAnimation(1, 1398718);
								currentRoomData->bitmapasociados[0].puntbitmap = 1398718;
							} break;
							}
							currentRoomData->bitmapasociados[0].tambitmap = 892;
							currentRoomData->bitmapasociados[0].coordx = 217;
							currentRoomData->bitmapasociados[0].coordy = 48;
							currentRoomData->bitmapasociados[0].profund = 1;
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
				drawFlc(140, 34, offset, 0, 9, 24, false, false, true, basurillalog);
				g_engine->_mouseManager->show();
				updateItem(regobj.code);
				currentRoomData->indexadoobjetos[7]->indicefichero = 716;
				currentRoomData->mouseGrid[19][9] = 14;
				currentRoomData->mouseGrid[22][16] = 15;
				for (indlista = 21; indlista <= 22; indlista++)
					for (indmoch = 17; indmoch <= 20; indmoch++)
						currentRoomData->mouseGrid[indlista][indmoch] = 17;
				{
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[0];

					with.puntbitmap = 1243652;
					with.tambitmap = 2718;
					with.coordx = 127;
					with.coordy = 36;
					with.profund = 6;
				}
				{
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[1];

					with.puntbitmap = 1240474;
					with.tambitmap = 344;
					with.coordx = 168;
					with.coordy = 83;
					with.profund = 12;
					loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
				}
				{
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[2];

					with.puntbitmap = 1240818;
					with.tambitmap = 116;
					with.coordx = 177;
					with.coordy = 82;
					with.profund = 1;
					loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
				}
				readBitmap(1243652, screenObjects[5], 2718, 319);
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
			} break;
			case 594: {
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				animateGive(3, 2);
				animatePickup2(3, 2);
				replaceBackpack(indicemochila, 607);
				drawBackpack();
				g_engine->_mouseManager->show();
			} break;
			case 608: {
				drawText(regobj.useTextRef);
				goToObject(currentRoomData->rejapantalla[mouseX][mouseY], 26);
				g_engine->_mouseManager->hide();
				animateGive(2, 2);
				animateOpen2(2, 2);
				{
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[3];

					with.puntbitmap = 1546096;
					with.tambitmap = 372;
					with.coordx = 208;
					with.coordy = 105;
					with.profund = 4;
					loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
				}
				screenHandleToBackground();
				// XMStoPointer(ptr(segfondo, (offfondo + 4)), _handpantalla, 4,(sizepantalla - int32(4)));
				assembleScreen();
				drawScreen(background);
				updateInventory(indicemochila);
				drawBackpack();
				trampa_puesta = true;
				g_engine->_mouseManager->show();
			} break;
			case 632: {
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][21] : flcOffsets[1][21];
				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				animateGive(direccionmovimiento, 1);

				// Show feather on pedestal
				loadItem(187, 70, 104, 1545820, 8);
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				g_engine->_screen->update();

				animateOpen2(direccionmovimiento, 1);
				g_engine->_mouseManager->show();
				goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], 14);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("PUFF", 191183, 18001);
				// Animate to scythe
				debug("Start anim!");
				drawFlc(180, 60, offset, 0, 9, 0, false, false, true, basurillalog);
				debug("End Anim!");
				// load Scythe
				currentRoomData->bitmapasociados[2].puntbitmap = 1545820;
				currentRoomData->bitmapasociados[2].tambitmap = 104;
				currentRoomData->bitmapasociados[2].coordx = 277;
				currentRoomData->bitmapasociados[2].coordy = 104;
				currentRoomData->bitmapasociados[2].profund = 1;
				depthMap[0].posy = 104;
				readBitmap(1545820, screenObjects[0], 104, 319);

				currentRoomData->bitmapasociados[4].puntbitmap = 1447508;
				currentRoomData->bitmapasociados[4].tambitmap = 464;
				currentRoomData->bitmapasociados[4].coordx = 186;
				currentRoomData->bitmapasociados[4].coordy = 64;
				currentRoomData->bitmapasociados[4].profund = 8;
				loadItem(186, 63, 464, 1447508, 8);

				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				g_engine->_mouseManager->show();
				goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], 18);
				g_engine->_mouseManager->hide();
				animatePickup1(1, 1);
				replaceBackpack(indicemochila, 638);
				currentRoomData->bitmapasociados[4].puntbitmap = 0;
				currentRoomData->bitmapasociados[4].tambitmap = 0;
				currentRoomData->bitmapasociados[4].coordx = 0;
				currentRoomData->bitmapasociados[4].coordy = 0;
				currentRoomData->bitmapasociados[4].profund = 0;
				screenObjects[7] = NULL;
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				animatePickup2(1, 1);
				drawBackpack();
				g_engine->_mouseManager->show();
				for (indlista = 35; indlista <= 37; indlista++)
					for (indmoch = 21; indmoch <= 25; indmoch++)
						currentRoomData->mouseGrid[indlista][indmoch] = 11;
				guadagna = true;
				if (tridente)
					cavernas[3] = true;
			} break;
			case 633: { //Use ring!
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][22] : flcOffsets[1][22];

				drawText(regobj.useTextRef);
				g_engine->_mouseManager->hide();
				animateGive(3, 1);
				loadItem(86, 55, 92, 1591272, 8);
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				animateOpen2(3, 1);
				g_engine->_mouseManager->show();
				goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], 10);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("PUFF", 191183, 18001);
				drawFlc(0, 47, offset, 0, 9, 0, false, false, true, basurillalog);

				currentRoomData->bitmapasociados[3].puntbitmap = 1591272;
				currentRoomData->bitmapasociados[3].tambitmap = 92;
				currentRoomData->bitmapasociados[3].coordx = 18;
				currentRoomData->bitmapasociados[3].coordy = 60;
				currentRoomData->bitmapasociados[3].profund = 3;
				depthMap[2].posx = 18;
				depthMap[2].posy = 60;
				readBitmap(1591272, screenObjects[2], 92, 319);

				currentRoomData->bitmapasociados[4].puntbitmap = 1746554;
				currentRoomData->bitmapasociados[4].tambitmap = 384;
				currentRoomData->bitmapasociados[4].coordx = 82;
				currentRoomData->bitmapasociados[4].coordy = 53;
				currentRoomData->bitmapasociados[4].profund = 8;
				loadItem(82, 53, 384, 1746554, 8);
				assembleScreen();
				drawScreen(background);
				g_engine->_mouseManager->show();
				goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], 15);
				g_engine->_mouseManager->hide();
				animatePickup1(3, 1);
				replaceBackpack(indicemochila, 637);
				currentRoomData->bitmapasociados[4].puntbitmap = 0;
				currentRoomData->bitmapasociados[4].tambitmap = 0;
				currentRoomData->bitmapasociados[4].coordx = 0;
				currentRoomData->bitmapasociados[4].coordy = 0;
				currentRoomData->bitmapasociados[4].profund = 0;
				screenObjects[7] = NULL;
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				animatePickup2(3, 1);
				drawBackpack();
				g_engine->_mouseManager->show();
				for (indlista = 0; indlista <= 2; indlista++)
					for (indmoch = 10; indmoch <= 12; indmoch++)
						currentRoomData->mouseGrid[indlista][indmoch] = 10;
				tridente = true;
				if (guadagna)
					cavernas[3] = true;
			} break;
			case 643: { // Urn with altar
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][23] : flcOffsets[1][23];

				if (currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory] != 5)
					drawText(regobj.useTextRef);
				pulsax = 149 - 7;
				pulsay = 126 - 7;
				goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], 5);
				g_engine->_mouseManager->hide();
				updateInventory(indicemochila);
				drawBackpack();
				drawFlc(133, 0, offset, 0, 9, 22, false, false, true, basurillalog);
				{
					RoomBitmapRegister &with = currentRoomData->bitmapasociados[2];

					with.puntbitmap = 1744230;
					with.tambitmap = 824;
					with.coordx = 147;
					with.coordy = 38;
					with.profund = 9;
					loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
				}
				updateAltScreen(31);
				for (indlista = 18; indlista <= 20; indlista++)
					for (indmoch = 8; indmoch <= 14; indmoch++)
						currentRoomData->mouseGrid[indlista][indmoch] = 12;
				sello_quitado = true;
				cavernas[1] = false;
				cavernas[0] = false;
				cavernas[4] = false;
				g_engine->_mouseManager->show();
			} break;
			case 657: { // sharpen scythe
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][24] : flcOffsets[1][24];

				drawText(regobj.useTextRef);
				pulsax = 178 - 7;
				pulsay = 71 - 7;
				goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], 3);
				g_engine->_mouseManager->hide();
				g_engine->_sound->playVoc("AFILAR", 0, 6433);
				drawFlc(160, 15, offset, 0, 9, 23, false, false, true, basurillalog);
				replaceBackpack(indicemochila, 715);
				drawBackpack();
				g_engine->_mouseManager->show();
			} break;
			case 686: {
				drawText(regobj.useTextRef);
				vasijapuesta = true;
				cavernas[4] = false;
				g_engine->_mouseManager->hide();
				animateGive(1, 1);
				updateInventory(indicemochila);
				dropObjectInScreen(regobj);
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				drawBackpack();
				animateOpen2(1, 1);
				for (indlista = 19; indlista <= 21; indlista++)
					for (indmoch = 10; indmoch <= 13; indmoch++)
						currentRoomData->mouseGrid[indlista][indmoch] = 13;
				g_engine->_mouseManager->show();
			} break;
			case 689: { // rope
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][25] : flcOffsets[1][25];

				drawText(regobj.useTextRef);
				pulsax = 124 - 7;
				pulsay = 133 - 7;
				goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], 9);
				g_engine->_mouseManager->hide();
				drawFlc(110, 79, offset, 0, 9, 0, false, false, true, basurillalog);
				replaceBackpack(indicemochila, 701);
				drawBackpack();
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				g_engine->_mouseManager->show();
				for (indlista = 18; indlista <= 20; indlista++)
					currentRoomData->mouseGrid[indlista][26] = 10;
				for (indlista = 17; indlista <= 21; indlista++)
					currentRoomData->mouseGrid[indlista][27] = 10;
			} break;
			case 700: { // Trident
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][26] : flcOffsets[1][26];
				drawText(regobj.useTextRef);
				pulsax = 224 - 7;
				pulsay = 91 - 7;
				goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], 7);
				g_engine->_mouseManager->hide();
				drawFlc(208, 0, offset, 0, 9, 21, false, false, true, basurillalog);
				currentRoomData->bitmapasociados[0].puntbitmap = 0;
				currentRoomData->bitmapasociados[0].tambitmap = 0;
				currentRoomData->bitmapasociados[0].coordx = 0;
				currentRoomData->bitmapasociados[0].coordy = 0;
				currentRoomData->bitmapasociados[0].profund = 0;
				screenObjects[2] = NULL;
				for (indmoch = 6; indmoch <= 9; indmoch++)
					currentRoomData->mouseGrid[26][indmoch] = 3;
				for (indmoch = 3; indmoch <= 5; indmoch++)
					currentRoomData->mouseGrid[27][indmoch] = 3;
				for (indmoch = 6; indmoch <= 10; indmoch++)
					currentRoomData->mouseGrid[27][indmoch] = 4;
				for (indmoch = 11; indmoch <= 12; indmoch++)
					currentRoomData->mouseGrid[27][indmoch] = 7;
				for (indmoch = 2; indmoch <= 10; indmoch++)
					currentRoomData->mouseGrid[28][indmoch] = 4;
				for (indmoch = 11; indmoch <= 12; indmoch++)
					currentRoomData->mouseGrid[28][indmoch] = 7;
				currentRoomData->mouseGrid[28][13] = 4;
				for (indmoch = 1; indmoch <= 14; indmoch++)
					currentRoomData->mouseGrid[29][indmoch] = 4;
				for (indlista = 30; indlista <= 32; indlista++)
					for (indmoch = 0; indmoch <= 15; indmoch++)
						currentRoomData->mouseGrid[indlista][indmoch] = 4;
				for (indmoch = 1; indmoch <= 14; indmoch++)
					currentRoomData->mouseGrid[33][indmoch] = 4;
				for (indmoch = 2; indmoch <= 14; indmoch++)
					currentRoomData->mouseGrid[34][indmoch] = 4;
				for (indmoch = 3; indmoch <= 8; indmoch++)
					currentRoomData->mouseGrid[35][indmoch] = 4;
				for (indmoch = 9; indmoch <= 11; indmoch++)
					currentRoomData->mouseGrid[35][indmoch] = 7;
				currentRoomData->doors[1].abiertacerrada = 1;
				g_engine->_mouseManager->show();
				updateItem(regobj.code);
			} break;
			case 709: { // rock with mural
				long offset = (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0][27] : flcOffsets[1][27];

				if (sello_quitado) {
					drawText(regobj.useTextRef);
					g_engine->_mouseManager->hide();
					animatePickup1(0, 1);
					g_engine->_sound->playVoc("TIZA", 390631, 18774);
					{
						RoomBitmapRegister &with = currentRoomData->bitmapasociados[1];

						with.puntbitmap = 1745054;
						with.tambitmap = 1500;
						with.coordx = 39;
						with.coordy = 16;
						with.profund = 1;
						loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
					}
					screenHandleToBackground();
					assembleScreen();
					drawScreen(background);

					g_engine->_sound->waitForSoundEnd();
					g_engine->_sound->playVoc("PUFF", 191183, 18001);
					animateOpen2(0, 1);
					drawFlc(180, 50, offset, 0, 9, 22, false, false, true, basurillalog);
					salirdeljuego = true;
				} else
					drawText(Random(11) + 1022);
			} break;
			}
		} else {
			goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], currentRoomData->rejapantalla[mouseX][mouseY]);
			if (regobj.code == 536 || regobj.code == 220)
				drawText(Random(6) + 1033);
			else
				drawText(Random(11) + 1022);
		}
	} else {
		if (screenObject > 0) {
			readItemRegister(screenObject);
			goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], currentRoomData->rejapantalla[mouseX][mouseY]);
			switch (regobj.usar[0]) {
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
					regobj.usar[0] = 9;
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
					indicemochila = 0;
					while (mobj[indicemochila].code != 0) {
						indicemochila += 1;
					}
					mobj[indicemochila].bitmapIndex = regobj.objectIconBitmap;
					mobj[indicemochila].code = regobj.code;
					mobj[indicemochila].objectName = regobj.name;
					g_engine->_mouseManager->hide();
					drawBackpack();
					g_engine->_mouseManager->show();
				} break;
				case 169: {
					g_engine->_mouseManager->hide();
					animatePickup1(0, 1);
					animateOpen2(0, 1);
					teleencendida = !(teleencendida);
					if (teleencendida) {
						g_engine->_sound->playVoc("CLICK", 27742, 2458);
						g_engine->_sound->waitForSoundEnd();
						currentRoomData->paletteAnimationFlag = true;
						g_engine->_sound->autoPlayVoc("PARASITO", 355778, 20129);
					} else {
						g_engine->_sound->stopVoc();
						g_engine->_sound->playVoc("CLICK", 27742, 2458);
						currentRoomData->paletteAnimationFlag = false;
						for (indicex = 195; indicex <= 200; indicex++) {
							pal[indicex * 3 + 0] = 2 << 2;
							pal[indicex * 3 + 1] = 2 << 2;
							pal[indicex * 3 + 2] = 2 << 2;
							setRGBPalette(indicex, 2, 2, 2);
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
					currentRoomData->puntpaleta = 1536;
					currentRoomData->indexadoobjetos[1]->indicefichero = 424;
					currentRoomData->doors[1].abiertacerrada = 1;
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
					currentRoomData->indexadoobjetos[16]->indicefichero = 362;
					currentRoomData->indexadoobjetos[16]->objectName = getObjectName(2);
					currentRoomData->indexadoobjetos[1]->indicefichero = 347;
					currentRoomData->indexadoobjetos[1]->objectName = getObjectName(3);
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
					if (vasijapuesta) {
						drawFlc(108, 0, offsetWithJar, 0, 9, 0, false, false, true, basurillalog);
						{
							RoomBitmapRegister &with = currentRoomData->bitmapasociados[0];

							with.puntbitmap = 1636796;
							with.tambitmap = 628;
							with.coordx = 153;
							with.coordy = 48;
							with.profund = 1;
						}
						for (indlista = 19; indlista <= 21; indlista++)
							for (indmoch = 10; indmoch <= 13; indmoch++)
								currentRoomData->mouseGrid[indlista][indmoch] = 12;
					} else
						drawFlc(108, 0, offsetNoJar, 0, 9, 0, false, false, true, basurillalog);
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
	oldxrejilla = 0;
	oldyrejilla = 0;
	numeroaccion = 0;
	checkMouseGrid();
}

void openScreenObject() {
	byte indicex, indicey;
	bool sueltapegote;

	uint mouseX = (pulsax + 7) / factorx;
	uint mouseY = (pulsay + 7) / factory;
	uint screenObject = currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[mouseX][mouseY]]->indicefichero;
	if (screenObject == 0)
		return;

	readItemRegister(screenObject);
	debug("Read screen object = %s, with code = %d, depth=%d", regobj.name.c_str(), regobj.code, regobj.profundidad);
	goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory],
			   currentRoomData->rejapantalla[mouseX][mouseY]);

	if (regobj.abrir == false) {
		drawText(Random(9) + 1059);
		return;
	} else {
		sueltapegote = false;
		switch (regobj.code) {
		case 227:
			if (alacena_abierta == false)
				sueltapegote = true;
			break;
		case 274:
			if (baul_abierto == false)
				sueltapegote = true;
			break;
		case 415:
			if (currentRoomData->doors[2].abiertacerrada == 2)
				sueltapegote = true;
			else {
				g_engine->_mouseManager->hide();
				animatePickup1(0, 1);
				screenObjects[regobj.profundidad - 1] = NULL;
				indicey = 0;
				while (currentRoomData->bitmapasociados[indicey].profund != regobj.profundidad && indicey != 15) {
					indicey++;
				}
				debug("changing bitmap at %d, with depth = %d", indicey, currentRoomData->bitmapasociados[indicey].profund);
				currentRoomData->bitmapasociados[indicey].puntbitmap = 0;
				currentRoomData->bitmapasociados[indicey].tambitmap = 0;
				currentRoomData->bitmapasociados[indicey].coordx = 0;
				currentRoomData->bitmapasociados[indicey].coordy = 0;
				currentRoomData->bitmapasociados[indicey].profund = 0;
				currentRoomData->doors[2].abiertacerrada = 1;
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				animateOpen2(0, 1);
				g_engine->_mouseManager->show();
				for (indicey = 0; indicey <= 12; indicey++)
					for (indicex = 33; indicex <= 36; indicex++)
						currentRoomData->mouseGrid[indicex][indicey] = 43;
				for (indicex = 33; indicex <= 35; indicex++)
					currentRoomData->mouseGrid[indicex][13] = 43;
				numeroaccion = 0;
				oldxrejilla = 0;
				oldyrejilla = 0;
				oldzonadestino = 0;
				checkMouseGrid();
				return;
			}
			break;
		case 548:
			if (currentRoomData->doors[0].abiertacerrada == 2)
				sueltapegote = true;
			else {
				g_engine->_mouseManager->hide();
				animatePickup1(1, 1);
				screenObjects[regobj.profundidad - 1] = NULL;
				indicey = 0;
				while (currentRoomData->bitmapasociados[indicey].profund != regobj.profundidad && indicey != 14) {
					indicey++;
				}
				currentRoomData->bitmapasociados[indicey].puntbitmap = 0;
				currentRoomData->bitmapasociados[indicey].tambitmap = 0;
				currentRoomData->bitmapasociados[indicey].coordx = 0;
				currentRoomData->bitmapasociados[indicey].coordy = 0;
				currentRoomData->bitmapasociados[indicey].profund = 0;
				currentRoomData->doors[0].abiertacerrada = 1;
				screenHandleToBackground();
				assembleScreen();
				drawScreen(background);
				animateOpen2(1, 1);
				g_engine->_mouseManager->show();
				indicex = 30;
				for (indicey = 17; indicey <= 18; indicey++)
					currentRoomData->mouseGrid[indicex][indicey] = 8;
				indicex += 1;
				for (indicey = 4; indicey <= 20; indicey++)
					currentRoomData->mouseGrid[indicex][indicey] = 8;
				indicex += 1;
				for (indicey = 0; indicey <= 20; indicey++)
					currentRoomData->mouseGrid[indicex][indicey] = 8;
				indicex += 1;
				for (indicey = 0; indicey <= 17; indicey++)
					currentRoomData->mouseGrid[indicex][indicey] = 8;
				indicex += 1;
				for (indicey = 0; indicey <= 12; indicey++)
					currentRoomData->mouseGrid[indicex][indicey] = 8;
				for (indicex = 35; indicex <= 39; indicex++)
					for (indicey = 0; indicey <= 10; indicey++)
						currentRoomData->mouseGrid[indicex][indicey] = 8;
				numeroaccion = 0;
				oldxrejilla = 0;
				oldyrejilla = 0;
				checkMouseGrid();
				return;
			}
			break;
		}
		if (sueltapegote) {
			drawText(Random(9) + 1059);
			return;
		}
		currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[mouseX][mouseY]]->indicefichero = regobj.reemplazarpor;
		g_engine->_mouseManager->hide();
		switch (regobj.altura) {
		case 0: {
			animatePickup1(direccionmovimiento, 0);
			updateVideo();
			animateOpen2(direccionmovimiento, 0);
		} break;
		case 1: {
			animatePickup1(direccionmovimiento, 1);
			updateVideo();
			animateOpen2(direccionmovimiento, 1);
		} break;
		case 2: {
			animatePickup1(direccionmovimiento, 2);
			updateVideo();
			animateOpen2(direccionmovimiento, 2);
		} break;
		}
		g_engine->_mouseManager->show();
		for (indicey = regobj.yrej1; indicey <= regobj.yrej2; indicey++)
			for (indicex = regobj.xrej1; indicex <= regobj.xrej2; indicex++) {
				currentRoomData->rejapantalla[indicex][indicey] = regobj.parcherejapantalla[indicex - regobj.xrej1][indicey - regobj.yrej1];
				currentRoomData->mouseGrid[indicex][indicey] = regobj.parcherejaraton[indicex - regobj.xrej1][indicey - regobj.yrej1];
			}
		for (indicex = 0; indicex < 15; indicex++)
			if (currentRoomData->bitmapasociados[indicex].puntbitmap == regobj.punterobitmap) {
				currentRoomData->bitmapasociados[indicex].puntbitmap = regobj.puntparche;
				currentRoomData->bitmapasociados[indicex].tambitmap = regobj.tamparche;
			}
		numeroaccion = 0;
	}
	oldxrejilla = 0;
	oldyrejilla = 0;
	checkMouseGrid();
}

void closeScreenObject() {
	byte indicex, indicey, x_del_raton, y_del_raton;
	bool sueltapegote;
	uint objeto_de_la_pantalla;

	x_del_raton = ((pulsax + 7) / factorx);
	y_del_raton = ((pulsay + 7) / factory);
	objeto_de_la_pantalla = currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[x_del_raton][y_del_raton]]->indicefichero;
	if (objeto_de_la_pantalla == 0)
		return;
	// verifyCopyProtection2();
	readItemRegister(objeto_de_la_pantalla);
	goToObject(currentRoomData->rejapantalla[((characterPosX + rectificacionx) / factorx)][((characterPosY + rectificaciony) / factory)],
			   currentRoomData->rejapantalla[x_del_raton][y_del_raton]);
	if (regobj.cerrar == false) {
		drawText((Random(10) + 1068));
		return;
	} else {
		sueltapegote = false;
		switch (regobj.code) {
		case 224:
		case 226:
			if (alacena_abierta == false)
				sueltapegote = true;
			break;
		case 275:
		case 277:
			if (baul_abierto == false)
				sueltapegote = true;
			break;
		}
		if (sueltapegote) {
			drawText(Random(10) + 1068);
			return;
		}
		currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[x_del_raton][y_del_raton]]->indicefichero = regobj.reemplazarpor;
		g_engine->_mouseManager->hide();
		switch (regobj.altura) {
		case 0: {
			animatePickup1(direccionmovimiento, 0);
			updateVideo();
			animateOpen2(direccionmovimiento, 0);
		} break;
		case 1: {
			animatePickup1(direccionmovimiento, 1);
			updateVideo();
			animateOpen2(direccionmovimiento, 1);
		} break;
		case 2: {
			animatePickup1(direccionmovimiento, 2);
			updateVideo();
			animateOpen2(direccionmovimiento, 2);
		} break;
		}
		g_engine->_mouseManager->show();
		for (indicey = regobj.yrej1; indicey <= regobj.yrej2; indicey++)
			for (indicex = regobj.xrej1; indicex <= regobj.xrej2; indicex++) {
				currentRoomData->rejapantalla[indicex][indicey] = regobj.parcherejapantalla[indicex - regobj.xrej1][indicey - regobj.yrej1];
				currentRoomData->mouseGrid[indicex][indicey] = regobj.parcherejaraton[indicex - regobj.xrej1][indicey - regobj.yrej1];
			}
		for (indicex = 0; indicex < 15; indicex++)
			if (currentRoomData->bitmapasociados[indicex].puntbitmap == regobj.punterobitmap) {
				currentRoomData->bitmapasociados[indicex].puntbitmap = regobj.puntparche;
				currentRoomData->bitmapasociados[indicex].tambitmap = regobj.tamparche;
			}
		numeroaccion = 0;
	}
	oldxrejilla = 0;
	oldyrejilla = 0;
	checkMouseGrid();
}

void action() {
	bar(0, 140, 319, 149, 0);
	Common::String actionLine;
	switch (numeroaccion) {
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
		objetomochila = "";
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
	switch (numeroaccion) {
	case 1: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(1) + mobj[posinv].objectName);
		g_engine->_mouseManager->show();
		drawText((Random(10) + 1039));
		numeroaccion = 0;
		if (contadorpc > 130)
			showError(274);
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 2: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(2) + mobj[posinv].objectName);
		if (contadorpc2 > 13)
			showError(274);
		g_engine->_mouseManager->show();
		drawText((Random(10) + 1049));
		numeroaccion = 0;
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 3: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(3) + mobj[posinv].objectName);
		g_engine->_mouseManager->show();
		numeroaccion = 0;
		lookInventoryObject(posinv);
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 4:
		if (objetomochila == "") {
			g_engine->_mouseManager->hide();
			actionLineText(getActionLineText(4) + mobj[posinv].objectName + getActionLineText(7));
			g_engine->_mouseManager->show();
			objetomochila = mobj[posinv].objectName;
			codigoobjmochila = mobj[posinv].code;
		} else {

			numeroaccion = 0;
			if (contadorpc > 25)
				showError(274);
			useInventoryObjectWithInventoryObject(codigoobjmochila, mobj[posinv].code);
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
		break;
	case 5: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(5) + mobj[posinv].objectName);
		g_engine->_mouseManager->show();
		drawText(Random(9) + 1059);
		numeroaccion = 0;
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 6: {
		g_engine->_mouseManager->hide();
		actionLineText(getActionLineText(6) + mobj[posinv].objectName);
		g_engine->_mouseManager->show();
		drawText(Random(10) + 1068);
		numeroaccion = 0;
		if (contadorpc2 > 35)
			showError(274);
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	}
}

void loadObjects() {

	Common::File ficheroobj;
	switch (gamePart) {
	case 1:
		ficheroobj.open("OBJMOCH.DAT");
		break;
	case 2:
		ficheroobj.open("OBJMOCH.TWO");
		break;
	}

	if (!ficheroobj.isOpen())
		showError(312);
	for (int i = 0; i < inventoryIconCount; i++) {
		mochilaxms[i] = (byte *)malloc(sizeicono);
		ficheroobj.read(mochilaxms[i], sizeicono);
	}
	if (contadorpc > 65)
		showError(274);
	for (int i = 0; i < inventoryIconCount; i++) {
		mobj[i].bitmapIndex = 34;
		mobj[i].code = 0;
		mobj[i].objectName = "VACIO";
	}

	ficheroobj.close();
	debug("Successfully read objects!");
}

void obtainName(Common::String &nombrejugador) {
	uint16 tamfondonom;
	byte *puntfondonom;

	tamfondonom = imagesize(84, 34, 235, 80);
	puntfondonom = (byte *)malloc(tamfondonom);
	getImg(84, 34, 235, 80, puntfondonom);
	drawMenu(8);
	g_engine->_screen->update();
	Common::String paso;
	readAlphaGraph(paso, 8, 125, 62, 252);
	nombrejugador = paso.c_str();
	putImg(84, 34, puntfondonom);
	g_engine->_screen->update();
	free(puntfondonom);
}

void loadScrollData(uint numpantalla, bool scrollder,
					uint poshor, int correccionscroll);

static byte *fondsprite;

/**
 * Blits image1 over image2 on the 0 pixels of image2
 */
static void montaimagenvir(byte *image1, byte *image2) { // Near;
	uint16 w = READ_LE_UINT16(image2) + 1;
	uint16 h = READ_LE_UINT16(image2 + 2) + 1;

	uint size = w * h;
	byte *dst = image2 + 4;
	byte *src = image1 + 4;
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

	int characterPos = 25 + (320 - (characterPosX + rectificacionx * 2));
	// We scroll 4 by 4 pixels so we divide by 4 to find out the number of necessary steps
	uint stepCount = (320 - horizontalPos) >> 2;
	byte *assembledCharacterFrame = (byte *)malloc(sizeframe);
	// Number of bytes to move
	size_t numBytes = 44796;
	for (int i = 0; i < stepCount; i++) {
		// move everything to the left
		memmove(background + 4, background + 8, numBytes);

		horizontalPos += 4;
		for (int k = 0; k < 140; k++) {
			for (int j = 0; j < 4; j++) {
				background[320 + k * 320 + j] = screenHandle[horizontalPos + k * 320 + j];
			}
		}
		if (characterPos > 0) {
			characterPos -= 2;
			if (characterPos > 0 && iframe < 15) {
				iframe++;
			} else
				iframe = 0;

			characterPosX -= 2;

			pasoframe = secuencia.bitmap[1][iframe];
			// We need to copy the original frame as to not replace its black background for after
			// the scroll ends. Original code would copy from XMS memory.
			Common::copy(pasoframe, pasoframe + sizeframe, assembledCharacterFrame);

			// puts the original captured background back in the background for next iteration
			putVirtualImg(characterPosX - 2, characterPosY, background, fondsprite);
			uint16 pasoframeW = READ_LE_UINT16(assembledCharacterFrame);
			uint16 pasoframeH = READ_LE_UINT16(assembledCharacterFrame + 2);
			// Grabs current area surrounding character (which might contain parts of A and B)
			getVirtualImg(characterPosX, characterPosY, characterPosX + pasoframeW, characterPosY + pasoframeH, background, fondsprite);
			// blits over the character sprite, only on black pixels
			montaimagenvir(fondsprite, assembledCharacterFrame);
			// puts it back in the background (character + piece of background)
			putVirtualImg(characterPosX, characterPosY, background, assembledCharacterFrame);
		} else
			characterPosX -= 4;
		g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		g_engine->_screen->update();
		drawScreen(background);
	}
	free(assembledCharacterFrame);
}

static void scrollLeft(uint &poshor) {

	int characterPos = 25 + characterPosX;
	poshor = 320 - poshor;
	// We scroll 4 by 4 pixels so we divide by 4 to find out the number of necessary steps
	uint numpasos = poshor >> 2;

	byte *assembledCharacterFrame = (byte *)malloc(sizeframe);
	size_t numBytes = 44796;
	for (int i = numpasos; i >= 1; i--) {
		for (int j = numBytes; j > 0; j--) {
			// move the previous background to the right
			background[j + 4] = background[j];
		}

		poshor -= 4;
		for (int k = 0; k < 140; k++) {
			for (int j = 0; j < 4; j++) {
				background[4 + k * 320 + j] = screenHandle[4 + poshor + k * 320 + j];
			}
		}

		if (characterPos > 0) {
			characterPos -= 2;
			if (characterPos > 0 && iframe < 15)
				iframe++;
			else
				iframe = 0;

			characterPosX += 2;

			pasoframe = secuencia.bitmap[3][iframe];
			Common::copy(pasoframe, pasoframe + sizeframe, assembledCharacterFrame);

			putVirtualImg(characterPosX + 2, characterPosY, background, fondsprite);

			uint16 pasoframeW = READ_LE_UINT16(assembledCharacterFrame);
			uint16 pasoframeH = READ_LE_UINT16(assembledCharacterFrame + 2);

			getVirtualImg(characterPosX, characterPosY, characterPosX + pasoframeW, characterPosY + pasoframeH, background, fondsprite);
			montaimagenvir(fondsprite, assembledCharacterFrame);
			putVirtualImg(characterPosX, characterPosY, background, assembledCharacterFrame);
		} else
			characterPosX += 4;

		g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 140));
		g_engine->_screen->update();
		drawScreen(background);
	}
	free(assembledCharacterFrame);
}

/**
 * Scrolling happens between two screens. We grab the area surroudning the player from screen A,
 * then transition to screen B.
 */
void loadScrollData(uint numpantalla, bool scrollder, uint poshor, int correccionscroll) {
	uint indicecarga;

	screenHandleToBackground();
	// Fondo now contains background A, handpantalla contains background A

	uint pasoframeW = READ_LE_UINT16(pasoframe);
	uint pasoframeH = READ_LE_UINT16(pasoframe + 2);
	debug("characterPos=%d,%d, size=%d,%d", characterPosX, characterPosY, pasoframeW, pasoframeH);
	/* Copy the area with the player from previous scren*/
	fondsprite = (byte *)malloc(4 + (pasoframeW + 8) * (pasoframeH + 8));
	getVirtualImg(characterPosX, characterPosY, characterPosX + pasoframeW, characterPosY + pasoframeH, background, fondsprite);

	// Start screen 2
	Common::File fichpanta;

	rooms->seek(numpantalla * roomRegSize, SEEK_SET);
	currentRoomData = readScreenDataFile(rooms);

	loadScreen();
	// Fondo now contains background B, handpantalla contains background B
	for (indicecarga = 0; indicecarga < 15; indicecarga++) {
		{
			RoomBitmapRegister &with = currentRoomData->bitmapasociados[indicecarga];
			if (with.tambitmap > 0)
				loadItem(with.coordx, with.coordy, with.tambitmap, with.puntbitmap, with.profund);
		}
	}
	// assembles the screen objects into fondo
	assembleScreen(true);
	// Fondo contains background B + objects, handpantalla contains plain background B

	// Copies the contents of fondo into handpantalla
	Common::copy(background, background + 44804, screenHandle);
	// Fondo contains background B + objects, handpantalla contains background B + objects

	movidapaleta = 0;
	getScreen(background);
	// Fondo now contains full background A again, handpantalla contains background B + objects

	drawScreen(background);
	if (scrollder)
		scrollRight(poshor);
	else
		scrollLeft(poshor);

	// After scroll is done, handpantalla will now contain the resulting fondo (background B + objects)
	Common::copy(screenHandle, screenHandle + 44804, background);

	characterPosX += correccionscroll;

	assembleScreen();
	drawScreen(background);
	free(fondsprite);
	loadScreen();
	trayec[indicetray].x = characterPosX;
	trayec[indicetray].y = characterPosY;
}

void saveGameToRegister() {
	uint indiaux;
	regpartida.numeropantalla = currentRoomData->codigo;
	regpartida.longtray = longtray;
	regpartida.indicetray = indicetray;
	regpartida.codigoobjmochila = codigoobjmochila;
	regpartida.volumenfxderecho = rightSfxVol;
	regpartida.volumenfxizquierdo = leftSfxVol;
	regpartida.volumenmelodiaderecho = musicVolRight;
	regpartida.volumenmelodiaizquierdo = musicVolLeft;
	regpartida.oldxrejilla = oldxrejilla;
	regpartida.oldyrejilla = oldyrejilla;
	regpartida.animadoprofundidad = animado.profundidad;
	regpartida.animadodir = animado.dir;
	regpartida.animadoposx = animado.posx;
	regpartida.animadoposy = animado.posy;
	regpartida.animadoiframe2 = iframe2;

	regpartida.zonaactual = zonaactual;
	regpartida.zonadestino = zonadestino;
	regpartida.oldzonadestino = oldzonadestino;
	regpartida.posicioninv = posicioninv;
	regpartida.numeroaccion = numeroaccion;
	regpartida.oldnumeroacc = oldnumeroacc;
	regpartida.pasos = pasos;
	regpartida.indicepuertas = indicepuertas;
	regpartida.direccionmovimiento = direccionmovimiento;
	regpartida.iframe = iframe;
	regpartida.parte_del_juego = gamePart;

	regpartida.sello_quitado = sello_quitado;
	regpartida.lista1 = lista1;
	regpartida.lista2 = lista2;
	regpartida.completadalista1 = list1Complete;
	regpartida.completadalista2 = list2Complete;
	regpartida.vasijapuesta = vasijapuesta;
	regpartida.guadagna = guadagna;
	regpartida.tridente = tridente;
	regpartida.torno = torno;
	regpartida.barro = barro;
	regpartida.diablillo_verde = diablillo_verde;
	regpartida.rojo_capturado = rojo_capturado;
	regpartida.manual_torno = manual_torno;
	regpartida.alacena_abierta = alacena_abierta;
	regpartida.baul_abierto = baul_abierto;
	regpartida.teleencendida = teleencendida;
	regpartida.trampa_puesta = trampa_puesta;

	for (indiaux = 0; indiaux < inventoryIconCount; indiaux++) {
		regpartida.mobj[indiaux].bitmapIndex = mobj[indiaux].bitmapIndex;
		regpartida.mobj[indiaux].code = mobj[indiaux].code;
		regpartida.mobj[indiaux].objectName = mobj[indiaux].objectName;
	}

	regpartida.elemento1 = elemento1;
	regpartida.elemento2 = elemento2;
	regpartida.xframe = characterPosX;
	regpartida.yframe = characterPosY;
	regpartida.xframe2 = xframe2;
	regpartida.yframe2 = yframe2;

	regpartida.oldobjmochila = oldobjmochila;
	regpartida.objetomochila = objetomochila;
	regpartida.nombrepersonaje = nombrepersonaje;

	for (int i = 0; i < routePointCount; i++) {
		regpartida.mainRoute[i].x = mainRoute[i].x;
		regpartida.mainRoute[i].y = mainRoute[i].y;
	}

	for (indiaux = 0; indiaux < 300; indiaux++) {
		regpartida.trayec[indiaux].x = trayec[indiaux].x;
		regpartida.trayec[indiaux].y = trayec[indiaux].y;
	}

	for (indiaux = 0; indiaux < maxpersonajes; indiaux++) {
		regpartida.primera[indiaux] = primera[indiaux];
		regpartida.lprimera[indiaux] = lprimera[indiaux];
		regpartida.cprimera[indiaux] = cprimera[indiaux];
		regpartida.libro[indiaux] = libro[indiaux];
		regpartida.caramelos[indiaux] = caramelos[indiaux];
	}
	for (indiaux = 0; indiaux < 5; indiaux++) {
		regpartida.cavernas[indiaux] = cavernas[indiaux];
		regpartida.firstList[indiaux] = firstList[indiaux];
		regpartida.secondList[indiaux] = secondList[indiaux];
	}
	for (indiaux = 0; indiaux < 4; indiaux++) {
		regpartida.hornacina[0][indiaux] = hornacina[0][indiaux];
		regpartida.hornacina[1][indiaux] = hornacina[1][indiaux];
	}
}

void loadGame(regispartida game) {
	freeAnimation();
	freeScreenObjects();

	uint indiaux, indiaux2;
	tipoefectofundido = Random(15) + 1;

	longtray = game.longtray;
	indicetray = game.indicetray;
	codigoobjmochila = game.codigoobjmochila;
	rightSfxVol = game.volumenfxderecho;
	leftSfxVol = game.volumenfxizquierdo;
	musicVolRight = game.volumenmelodiaderecho;
	musicVolLeft = game.volumenmelodiaizquierdo;
	oldxrejilla = game.oldxrejilla;
	oldyrejilla = game.oldyrejilla;
	animado.profundidad = game.animadoprofundidad;
	animado.dir = game.animadodir;
	animado.posx = game.animadoposx;
	animado.posy = game.animadoposy;
	iframe2 = game.animadoiframe2;
	zonaactual = game.zonaactual;
	zonadestino = game.zonadestino;
	oldzonadestino = game.oldzonadestino;
	posicioninv = game.posicioninv;
	numeroaccion = game.numeroaccion;
	oldnumeroacc = game.oldnumeroacc;
	pasos = game.pasos;
	indicepuertas = game.indicepuertas;
	direccionmovimiento = game.direccionmovimiento;
	iframe = game.iframe;
	if (game.parte_del_juego != gamePart) {
		gamePart = game.parte_del_juego;
		for (int i = 0; i < inventoryIconCount; i++) {
			free(mochilaxms[i]);
		}
		loadObjects();
	}
	sello_quitado = game.sello_quitado;
	lista1 = game.lista1;
	lista2 = game.lista2;
	list1Complete = game.completadalista1;
	list2Complete = game.completadalista2;
	vasijapuesta = game.vasijapuesta;
	guadagna = game.guadagna;
	if (contadorpc > 24)
		showError(274);
	tridente = game.tridente;
	torno = game.torno;
	barro = game.barro;
	diablillo_verde = game.diablillo_verde;
	rojo_capturado = game.rojo_capturado;
	manual_torno = game.manual_torno;
	alacena_abierta = game.alacena_abierta;
	baul_abierto = game.baul_abierto;
	teleencendida = game.teleencendida;
	trampa_puesta = game.trampa_puesta;
	for (indiaux = 0; indiaux < inventoryIconCount; indiaux++) {
		mobj[indiaux].bitmapIndex = game.mobj[indiaux].bitmapIndex;
		mobj[indiaux].code = game.mobj[indiaux].code;
		mobj[indiaux].objectName = game.mobj[indiaux].objectName;
	}
	elemento1 = game.elemento1;
	elemento2 = game.elemento2;
	characterPosX = game.xframe;
	characterPosY = game.yframe;
	xframe2 = game.xframe2;
	yframe2 = game.yframe2;
	oldobjmochila = game.oldobjmochila;
	objetomochila = game.objetomochila;
	nombrepersonaje = game.nombrepersonaje;
	for (int i = 0; i < routePointCount; i++) {
		mainRoute[i].x = game.mainRoute[i].x;
		mainRoute[i].y = game.mainRoute[i].y;
	}
	for (indiaux = 0; indiaux < 300; indiaux++) {
		trayec[indiaux].x = game.trayec[indiaux].x;
		trayec[indiaux].y = game.trayec[indiaux].y;
	}
	for (indiaux = 0; indiaux < maxpersonajes; indiaux++) {
		primera[indiaux] = game.primera[indiaux];
		lprimera[indiaux] = game.lprimera[indiaux];
		cprimera[indiaux] = game.cprimera[indiaux];
		libro[indiaux] = game.libro[indiaux];
		caramelos[indiaux] = game.caramelos[indiaux];
	}
	for (indiaux = 0; indiaux < 5; indiaux++) {
		cavernas[indiaux] = game.cavernas[indiaux];
		firstList[indiaux] = game.firstList[indiaux];
		secondList[indiaux] = game.secondList[indiaux];
	}
	for (indiaux = 0; indiaux < 4; indiaux++) {
		hornacina[0][indiaux] = game.hornacina[0][indiaux];
		hornacina[1][indiaux] = game.hornacina[1][indiaux];
	}

	totalFadeOut(0);
	cleardevice();
	loadPalette("DEFAULT");
	loadScreenData(game.numeropantalla);

	switch (currentRoomData->codigo) {
	case 2: {
		if (teleencendida)
			g_engine->_sound->autoPlayVoc("PARASITO", 355778, 20129);
		else
			cargatele();
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
		if (libro[0] == true && currentRoomData->animationFlag)
			disableSecondAnimation();
	} break;
	case 20: {
		switch (hornacina[0][hornacina[0][3]]) {
		case 0:
			currentRoomData->indexadoobjetos[9]->objectName = getObjectName(4);
			break;
		case 561:
			currentRoomData->indexadoobjetos[9]->objectName = getObjectName(5);
			break;
		case 563:
			currentRoomData->indexadoobjetos[9]->objectName = getObjectName(6);
			break;
		case 615:
			currentRoomData->indexadoobjetos[9]->objectName = getObjectName(7);
			break;
		}
	} break;
	case 23: {
		g_engine->_sound->autoPlayVoc("Fuente", 0, 0);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
	} break;
	case 24: {
		switch (hornacina[1][hornacina[1][3]]) {
		case 0:
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(4);
			break;
		case 561:
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(5);
			break;
		case 615:
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(7);
			break;
		case 622:
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(8);
			break;
		case 623:
			currentRoomData->indexadoobjetos[8]->objectName = getObjectName(9);
			break;
		}
		if (trampa_puesta) {
			currentRoomData->animationFlag = true;
			loadAnimation(currentRoomData->nombremovto);
			iframe2 = 0;
			indicetray2 = 1;
			currentRoomData->tray2[indicetray2 - 1].x = 214 - 15;
			currentRoomData->tray2[indicetray2 - 1].y = 115 - 42;
			animado.dir = currentRoomData->dir2[indicetray2 - 1];
			animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
			animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
			animado.profundidad = 14;

			for (indiaux = 0; indiaux < maxrejax; indiaux++)
				for (indiaux2 = 0; indiaux2 < maxrejay; indiaux2++) {
					if (rejamascaramovto[indiaux][indiaux2] > 0)
						currentRoomData->rejapantalla[oldposx + indiaux][oldposy + indiaux2] = rejamascaramovto[indiaux][indiaux2];
					if (rejamascararaton[indiaux][indiaux2] > 0)
						currentRoomData->mouseGrid[oldposx + indiaux][oldposy + indiaux2] = rejamascararaton[indiaux][indiaux2];
				}
		}
		assembleScreen();
	} break;
	}

	mask();
	posicioninv = 0;
	drawBackpack();
	if (rojo_capturado == false && currentRoomData->codigo == 24 && trampa_puesta == false)
		runaroundRed();
	screenTransition(tipoefectofundido, false, background);
}

struct indicepart {
	byte ultimapartida;
	Common::String listapartidas[6];
};

void seleccionaPartida(indicepart regindfich, int numSeleccion) {
	g_engine->_mouseManager->hide();
	for (int i = 0; i < 6; i++) {
		int color = i == numSeleccion ? 255 : 253;
		outtextxy(65, 29, regindfich.listapartidas[i], color);
	}
	g_engine->_mouseManager->show();
}

void saveLoad() {
	uint tamfondmenu;
	uint oldxraton, oldyraton;
	byte *puntfondmenu;
	byte ytext;
	byte partidaselecc;
	bool modificada;
	bool salirmenufunciones;
	Common::String nombrepartida;
	indicepart regindfich;

	Common::File fichindice;
	if (fichindice.open("PARTIDAS.INX")) {
		regindfich.ultimapartida = fichindice.readByte();
		for (int i = 0; i < 6; i++) {
			int nameSize = fichindice.readByte();
			char *name = (char *)malloc(nameSize);
			fichindice.read(name, nameSize);
			fichindice.skip(255 - nameSize);
			regindfich.listapartidas[i] = Common::String(name, name + nameSize);
		}
	} else {
		regindfich.ultimapartida = 1;
		for (int i = 0; i < 6; i++) {
			regindfich.listapartidas[i] = Common::String().format("DISPONIBLE %d", i);
		}
	}
	salirmenufunciones = false;
	oldxraton = xraton;
	oldyraton = yraton;
	g_engine->_mouseManager->hide();

	tamfondmenu = imagesize(50, 10, 270, 120);
	puntfondmenu = (byte *)malloc(tamfondmenu);
	getImg(50, 10, 270, 120, puntfondmenu);

	for (int i = 0; i < 6; i++) {
		uint textY = i + 1;
		buttonBorder((120 - (textY * 10)), (80 - (textY * 10)), (200 + (textY * 10)), (60 + (textY * 10)), 251, 251, 251, 251, 0, 0, "");
	}
	drawMenu(2);
	if (desactivagrabar) {
		bar(61, 15, 122, 23, 253);
		bar(201, 15, 259, 23, 253);
	}
	// setcolor(253);
	outtextxy(65, 29, regindfich.listapartidas[0], 253);
	outtextxy(65, 44, regindfich.listapartidas[1], 253);
	outtextxy(65, 59, regindfich.listapartidas[2], 253);
	outtextxy(65, 74, regindfich.listapartidas[3], 253);
	outtextxy(65, 89, regindfich.listapartidas[4], 253);
	outtextxy(65, 104, regindfich.listapartidas[5], 253);
	if (contadorpc2 > 17)
		showError(274);
	xraton = 150;
	yraton = 60;
	// iraton = 1;
	partidaselecc = 0;
	modificada = false;
	nombrepartida = "";
	g_engine->_mouseManager->setMouseArea(Common::Rect(55, 13, 250, 105));
	g_engine->_mouseManager->setMousePos(1, xraton, yraton);
	do {
		Common::Event e;
		bool mouseClicked = false;
		bool keyPressed = false;
		do {
			if (tocapintar) {
				g_engine->_chrono->updateChrono();
				g_engine->_mouseManager->animateMouseIfNeeded();
			}
			while (g_system->getEventManager()->pollEvent(e)) {
				if (isMouseEvent(e)) {
					g_engine->_mouseManager->setMousePos(e.mouse);
					xraton = e.mouse.x;
					yraton = e.mouse.y;
				}

				if (e.type == Common::EVENT_LBUTTONUP || e.type == Common::EVENT_RBUTTONUP) {
					mouseClicked = true;
					pulsax = e.mouse.x;
					pulsay = e.mouse.y;
				} else if (e.type == Common::EVENT_KEYUP) {
					keyPressed = true;
				}
			}

			g_engine->_screen->update();
			g_system->delayMillis(10);
		} while (!keyPressed && !mouseClicked && !g_engine->shouldQuit());

		if (mouseClicked) {
			if (pulsay >= 13 && pulsay <= 16) {
				if (pulsax >= 54 && pulsax <= 124) {
					if (partidaselecc > 0 && !desactivagrabar && (nombrepartida != Common::String("DISPONIBLE ") + (char)(partidaselecc + 48)) && (nombrepartida != "")) {
						// saveGame(partidaselecc);
						fichindice.close();
						putImg(50, 10, puntfondmenu);
						salirmenufunciones = true;
						partidaselecc = 0;
					} else {
						sound(100, 300);
					}
				} else if (pulsax >= 130 && pulsax <= 194) {
					if ((partidaselecc > 0) && !((modificada))) {

						if (regindfich.listapartidas[partidaselecc] != (Common::String("DISPONIBLE ") + (char)(partidaselecc + 48))) {
							g_engine->_mouseManager->hide();
							putImg(50, 10, puntfondmenu);
							free(puntfondmenu);
							if (!desactivagrabar) {
								freeAnimation();
								freeScreenObjects();
							}
							// loadGame(partidaselecc);
							xraton = oldxraton;
							yraton = oldyraton;

							g_engine->_mouseManager->show();
							g_engine->_mouseManager->setMouseArea(Common::Rect(0, 0, 305, 185));
							salirmenufunciones = true;
							partidaselecc = 0;
							return;
						} else {
							sound(100, 300);
						}
					} else {
						sound(100, 300);
						g_engine->_mouseManager->hide();
						bar(61, 31, 259, 39, 251);
						outtextxy(65, 29, regindfich.listapartidas[1], 253);
						bar(61, 46, 259, 54, 251);
						outtextxy(65, 44, regindfich.listapartidas[2], 253);
						bar(61, 61, 259, 69, 251);
						outtextxy(65, 59, regindfich.listapartidas[3], 253);
						bar(61, 76, 259, 84, 251);
						outtextxy(65, 74, regindfich.listapartidas[4], 253);
						bar(61, 91, 259, 99, 251);
						outtextxy(65, 89, regindfich.listapartidas[5], 253);
						bar(61, 106, 259, 114, 251);
						outtextxy(65, 104, regindfich.listapartidas[6], 253);
						g_engine->_mouseManager->show();
					}
				} else if (pulsax >= 200 && pulsax <= 250) {
					if (inGame && !desactivagrabar) {
						putImg(50, 10, puntfondmenu);
						salirmenufunciones = true;
						partidaselecc = 0;
					} else {
						sound(100, 300);
					}
				}
			} else if (pulsay >= 24 && pulsay <= 32) {
				partidaselecc = 0;
				modificada = false;
				ytext = 29;
				seleccionaPartida(regindfich, 0);
				nombrepartida = regindfich.listapartidas[0];
			} else if (pulsay >= 39 && pulsay <= 47) {
				partidaselecc = 1;
				modificada = false;
				ytext = 44;
				seleccionaPartida(regindfich, 1);
				nombrepartida = regindfich.listapartidas[1];
			} else if (pulsay >= 54 && pulsay <= 62) {
				partidaselecc = 2;
				modificada = false;
				ytext = 59;
				seleccionaPartida(regindfich, 2);
				nombrepartida = regindfich.listapartidas[2];
			} else if (pulsay >= 69 && pulsay <= 77) {
				partidaselecc = 3;
				modificada = false;
				ytext = 74;
				seleccionaPartida(regindfich, 3);
				nombrepartida = regindfich.listapartidas[3];
			} else if (pulsay >= 84 && pulsay <= 92) {
				partidaselecc = 4;
				modificada = false;
				ytext = 89;
				seleccionaPartida(regindfich, 4);
				nombrepartida = regindfich.listapartidas[4];
			} else if (pulsay >= 99 && pulsay <= 107) {
				partidaselecc = 5;
				modificada = false;
				ytext = 0;
				seleccionaPartida(regindfich, 5);
				nombrepartida = regindfich.listapartidas[5];
			}
		}

		if (partidaselecc > 0 && keypressed()) {
			g_engine->_mouseManager->hide();
			readAlphaGraphSmall(nombrepartida, 30, 65, ytext, 251, 254);
			modificada = true;
			g_engine->_mouseManager->show();
		}
	} while (!salirmenufunciones && !g_engine->shouldQuit());
	xraton = oldxraton;
	yraton = oldyraton;
	g_engine->_mouseManager->setMousePos(iraton, xraton, yraton);
	free(puntfondmenu);
	g_engine->_mouseManager->setMouseArea(Common::Rect(0, 0, 305, 185));
}

/**
 * Loads talking animation of main adn secondary character
 */
void loadTalkAnimations() {
	Common::File fichcani;

	if (!fichcani.open("TIOHABLA.SEC")) {
		showError(265);
	}
	int32 posfilehabla;

	sizeframe = fichcani.readUint16LE();

	posfilehabla = sizeframe * 16;
	posfilehabla = (posfilehabla * direccionmovimiento) + 2;
	fichcani.seek(posfilehabla);
	debug("LoadTalk direccionmovimiento=%d", direccionmovimiento);
	for (int i = 0; i < 16; i++) {
		secuencia.bitmap[0][i] = (byte *)malloc(sizeframe);
		fichcani.read(secuencia.bitmap[0][i], sizeframe);
	}
	fichcani.close();

	if ((currentRoomData->nombremovto != "PETER") && (currentRoomData->nombremovto != "ARZCAEL")) {
		iframe2 = 0;
		free(pasoanimado);
		bool result;
		switch (regobj.habla) {
		case 1:
			result = fichcani.open("JOHN.SEC");
			break;
		case 5:
			result = fichcani.open("ALFRED.SEC");
			break;
		default:
			result = fichcani.open(Common::Path(currentRoomData->nombremovto + Common::String(".SEC")));
		}

		if (!result)
			showError(265);
		sizeanimado = fichcani.readUint16LE();
		secondaryAnimationFrameCount = fichcani.readByte();
		numerodir = fichcani.readByte();

		pasoanimado = (byte *)malloc(sizeanimado);
		if (numerodir != 0) {
			secondaryAnimationFrameCount = secondaryAnimationFrameCount / 4;
			for (int i = 0; i <= 3; i++) {
				loadAnimationForDirection(&fichcani, i);
			}
		} else {
			loadAnimationForDirection(&fichcani, 0);
		}
		fichcani.close();
	}
}

void unloadTalkAnimations() {

	Common::File fichcani;
	if (!fichcani.open("PERSONAJ.SPT")) {
		showError(265);
	}
	sizeframe = fichcani.readUint16LE();

	for (int i = 0; i < walkFrameCount; i++) {
		secuencia.bitmap[0][i] = (byte *)malloc(sizeframe);
		fichcani.read(secuencia.bitmap[0][i], sizeframe);
	}
	fichcani.close();

	if ((currentRoomData->nombremovto != "PETER") && (currentRoomData->nombremovto != "ARZCAEL")) {
		if (!fichcani.open(Common::Path(currentRoomData->nombremovto + ".DAT"))) {
			showError(265);
		}
		sizeanimado = fichcani.readUint16LE();
		secondaryAnimationFrameCount = fichcani.readByte();
		numerodir = fichcani.readByte();
		pasoanimado = (byte *)malloc(sizeanimado);
		if (numerodir != 0) {

			secondaryAnimationFrameCount = secondaryAnimationFrameCount / 4;
			for (int i = 0; i <= 3; i++) {
				loadAnimationForDirection(&fichcani, i);
			}
		} else {
			loadAnimationForDirection(&fichcani, 0);
		}
		fichcani.close();
	}
}

regismht readVerbRegister(uint numRegister) {
	verb.seek(numRegister * verbRegSize);
	return readVerbRegister();
}

regismht readVerbRegister() {
	regismht regmht;
	// Since the text is encrypted it's safer to save the size as reported by
	// the pascal string.
	byte size = verb.readByte();
	verb.seek(-1, SEEK_CUR);
	regmht.cadenatext = verb.readPascalString(false);
	verb.skip(255 - size);
	regmht.encadenado = verb.readByte();
	regmht.respuesta = verb.readUint16LE();
	regmht.punteronil = verb.readSint32LE();
	return regmht;
}

void hypertext(
	uint numreght,
	byte colortextoht,
	byte colorsombraht,
	uint &numresp,
	/** Whether the text being said is part of a conversation or just descriptions */
	bool banderaconversa) {
	regismht regmht;

	byte insertarnombre, iht, iteracionesht, lineaht, anchoht;
	byte direccionmovimientopaso;

	uint indiceaniconversa, tamfondoht, xht, yht;

	byte *fondotextht;
	byte matrizsaltosht[15];
	g_engine->_mouseManager->hide();
	switch (currentRoomData->codigo) {
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

	verb.seek(numreght * verbRegSize);

	do {

		regmht = readVerbRegister();

		insertarnombre = 0;

		for (int i = 0; i < regmht.cadenatext.size(); i++) {
			regmht.cadenatext.setChar(decryptionKey[i] ^ regmht.cadenatext[i], i);
			if (regmht.cadenatext[i] == '@')
				insertarnombre = i;
		}

		if (insertarnombre > 0) {
			regmht.cadenatext.deleteChar(insertarnombre);
			regmht.cadenatext.insertString(nombrepersonaje, insertarnombre);
		}

		if (regmht.cadenatext.size() < anchoht) {
			tamfondoht = imagesize(xht - 1, yht - 1, xht + (regmht.cadenatext.size() * 8) + 2, yht + 13);
			fondotextht = (byte *)malloc(tamfondoht);

			getImg(xht - 1, yht - 1, xht + (regmht.cadenatext.size() * 8) + 2, yht + 13, fondotextht);

			outtextxy(xht - 1, yht, regmht.cadenatext, colorsombraht);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
			outtextxy(xht + 1, yht, regmht.cadenatext, colorsombraht);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
			outtextxy(xht, yht - 1, regmht.cadenatext, colorsombraht);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);
			outtextxy(xht, yht + 1, regmht.cadenatext, colorsombraht);
			g_engine->_screen->update();
			delay(enforcedTextAnimDelay);

			outtextxy(xht, yht, regmht.cadenatext, colortextoht);
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
				} while (regmht.cadenatext[iht] != ' ');
				matrizsaltosht[iteracionesht] = iht + 1;
			} while (iht + 1 <= regmht.cadenatext.size() - anchoht);

			iteracionesht += 1;
			matrizsaltosht[iteracionesht] = regmht.cadenatext.size();

			// Grab patch of background behind where the text will be, to paste it back later
			tamfondoht = imagesize(xht - 1, yht - 1, xht + (anchoht * 8) + 2, yht + iteracionesht * 13);
			fondotextht = (byte *)malloc(tamfondoht);
			getImg(xht - 1, yht - 1, xht + (anchoht * 8) + 2, yht + iteracionesht * 13, fondotextht);

			for (lineaht = 1; lineaht <= iteracionesht; lineaht++) {

				Common::String lineString = Common::String(regmht.cadenatext.c_str() + matrizsaltosht[lineaht - 1], regmht.cadenatext.c_str() + matrizsaltosht[lineaht]);

				outtextxy(xht + 1, yht + ((lineaht - 1) * 11), lineString, colorsombraht);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
				outtextxy(xht - 1, yht + ((lineaht - 1) * 11), lineString, colorsombraht);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
				outtextxy(xht, yht + ((lineaht - 1) * 11) + 1, lineString, colorsombraht);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
				outtextxy(xht, yht + ((lineaht - 1) * 11) - 1, lineString, colorsombraht);
				g_engine->_screen->update();
				delay(enforcedTextAnimDelay);
				outtextxy(xht, yht + ((lineaht - 1) * 11), lineString, colortextoht);
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
			if (tocapintar) {
				tocapintar = false;
				if (tocapintar2) {
					if (banderaconversa) {
						indiceaniconversa += 1;
						if (colortextoht == 255) {
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
						direccionmovimientopaso = direccionmovimiento;
						direccionmovimiento = 0;
						sprites(true);
						direccionmovimiento = direccionmovimientopaso;
					} else {
						if (indicetray2 >= currentRoomData->longtray2)
							indicetray2 = 1;
						else
							indicetray2 += 1;
						animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
						animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
						animado.dir = currentRoomData->dir2[indicetray2 - 1];
						if (iframe2 >= secondaryAnimationFrameCount - 1)
							iframe2 = 0;
						else
							iframe2++;
						sprites(false);
					}
				}
				if (currentRoomData->paletteAnimationFlag && saltospal >= 4) {
					saltospal = 0;
					if (movidapaleta > 6)
						movidapaleta = 0;
					else
						movidapaleta += 1;
					if (currentRoomData->codigo == 4 && movidapaleta == 4)
						g_engine->_sound->playVoc();
					updatePalette(movidapaleta);
				} else
					saltospal += 1;
			}
			g_engine->_screen->update();
			g_system->delayMillis(10);
		} while (indiceaniconversa <= (regmht.cadenatext.size() * 4) && !mouseClicked && !g_engine->shouldQuit());

		putImg(xht - 1, yht - 1, fondotextht);
		free(fondotextht);

		g_system->delayMillis(10);
	} while (regmht.encadenado && !g_engine->shouldQuit());
	numresp = regmht.respuesta;
	g_engine->_mouseManager->show();
}

void wcScene() {
	palette palwater;
	zonaactual = currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
	goToObject(zonaactual, zonadestino);

	copyPalette(pal, palwater);
	g_engine->_mouseManager->hide();

	partialFadeOut(234);

	const char *const *messages = (g_engine->_lang == Common::ES_ESP) ? fullScreenMessages[0] : fullScreenMessages[1];

	outtextxy(10, 20, messages[45], 253);
	delay(1000);

	bar(10, 20, 150, 30, 0);
	delay(2000);

	outtextxy(100, 50, messages[46], 255);
	delay(1000);

	bar(100, 50, 250, 60, 0);
	delay(2000);

	outtextxy(30, 110, messages[47], 253);
	delay(1000);

	bar(30, 110, 210, 120, 0);
	delay(3000);

	outtextxy(50, 90, messages[48], 248);
	delay(1000);

	g_engine->_sound->playVoc("WATER", 272050, 47062);
	bar(50, 90, 200, 100, 0);
	delay(4000);

	characterPosX = 76 - rectificacionx;
	characterPosY = 78 - rectificaciony;
	copyPalette(palwater, pal);
	screenHandleToBackground();
	assembleScreen();
	drawScreen(background);
	partialFadeIn(234);
	xframe2 = 0;
	indicetray = 0;
	longtray = 1;
	zonaactual = 8;
	zonadestino = 8;
	trayec[0].x = characterPosX;
	trayec[0].y = characterPosY;

	g_engine->_mouseManager->show();
}

void readConversationFile(Common::String f) {
	Common::File conversationFile;
	debug("Filename = %s", f.c_str());
	if (!conversationFile.open(Common::Path(f))) {
		showError(314);
	}
	int64 fileSize = conversationFile.size();
	// TODO: Will this be freed automatically by the `MemorySeekableReadWriteStream`?
	byte *buf = (byte *)malloc(fileSize);
	conversationFile.read(buf, fileSize);

	conversationData = new Common::MemorySeekableReadWriteStream(buf, fileSize, DisposeAfterUse::NO);
	conversationFile.close();
}

void saveDoorMetadata(DoorRegistry doors, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeUint16LE(doors.pantallaquecarga);
	screenDataStream->writeUint16LE(doors.posxsalida);
	screenDataStream->writeUint16LE(doors.posysalida);
	screenDataStream->writeByte(doors.abiertacerrada);
	screenDataStream->writeByte(doors.codigopuerta);
}

void savePoint(Common::Point point, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeUint16LE(point.x);
	screenDataStream->writeUint16LE(point.y);
}

void saveBitmapRegister(RoomBitmapRegister bitmap, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeSint32LE(bitmap.puntbitmap);
	screenDataStream->writeUint16LE(bitmap.tambitmap);
	screenDataStream->writeUint16LE(bitmap.coordx);
	screenDataStream->writeUint16LE(bitmap.coordy);
	screenDataStream->writeUint16LE(bitmap.profund);
}

void saveRoomObjectList(RoomObjectListEntry objectList, Common::SeekableWriteStream *screenDataStream) {

	screenDataStream->writeUint16LE(objectList.indicefichero);
	screenDataStream->writeByte(objectList.objectName.size());
	int paddingSize = 20 - objectList.objectName.size();
	if (paddingSize < 20) {
		screenDataStream->writeString(objectList.objectName);
		// screenDataStream->write(objectList->objectName, objectList.dummy);
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
	screenDataStream->writeUint16LE(room->codigo);
	screenDataStream->writeUint32LE(room->puntimagenpantalla);
	screenDataStream->writeUint16LE(room->tamimagenpantalla);
	screenDataStream->write(room->rejapantalla, 40 * 28);
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
		saveBitmapRegister(room->bitmapasociados[i], screenDataStream);
	}
	for (int i = 0; i < 51; i++) {
		saveRoomObjectList(*room->indexadoobjetos[i], screenDataStream);
	}
	screenDataStream->writeByte(room->animationFlag);

	screenDataStream->writeByte(room->nombremovto.size());
	screenDataStream->writeString(room->nombremovto);
	int paddingSize = 8 - room->nombremovto.size();
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
	screenDataStream->writeUint16LE(room->puntpaleta);
	for (int i = 0; i < 300; i++) {
		savePoint(room->tray2[i], screenDataStream);
	}
	screenDataStream->write(room->dir2, 600);
	screenDataStream->writeUint16LE(room->longtray2);
}

void saveRoomData(RoomFileRegister *room, Common::SeekableWriteStream *stream) {
	rooms->seek(room->codigo * roomRegSize, SEEK_SET);
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

void saveItem(InvItemRegister object, Common::SeekableWriteStream *objectDataStream) {
	objectDataStream->writeUint16LE(object.code);
	objectDataStream->writeByte(object.altura);

	objectDataStream->writeByte(object.name.size());
	objectDataStream->writeString(object.name);
	int paddingSize = longitudnombreobjeto - object.name.size();
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

	objectDataStream->writeByte(object.habla);
	objectDataStream->writeByte(object.abrir);
	objectDataStream->writeByte(object.cerrar);

	objectDataStream->write(object.usar, 8);

	objectDataStream->writeByte(object.coger);

	objectDataStream->writeUint16LE(object.usarcon);
	objectDataStream->writeUint16LE(object.reemplazarpor);
	objectDataStream->writeByte(object.profundidad);
	objectDataStream->writeUint32LE(object.punterobitmap);
	objectDataStream->writeUint16LE(object.tambitmap);
	objectDataStream->writeUint16LE(object.punteroframesgiro);
	objectDataStream->writeUint16LE(object.punteropaletagiro);
	objectDataStream->writeUint16LE(object.xparche);
	objectDataStream->writeUint16LE(object.yparche);
	objectDataStream->writeUint32LE(object.puntparche);
	objectDataStream->writeUint16LE(object.tamparche);
	objectDataStream->writeUint16LE(object.objectIconBitmap);

	objectDataStream->writeByte(object.xrej1);
	objectDataStream->writeByte(object.yrej1);
	objectDataStream->writeByte(object.xrej2);
	objectDataStream->writeByte(object.yrej2);

	objectDataStream->write(object.parcherejapantalla, 100);
	objectDataStream->write(object.parcherejaraton, 100);
}

void saveItemRegister(InvItemRegister object, Common::SeekableWriteStream *stream) {
	invItemData->seek(object.code * itemRegSize, SEEK_SET);
	saveItem(object, stream);
}

void printHornacinas() {
	debug("|   | %03d | %03d | %03d | %03d |", 0, 1, 2, 3);
	debug("| 0 | %03d | %03d | %03d | %03d |", hornacina[0][0], hornacina[0][1], hornacina[0][2], hornacina[0][3]);
	debug("| 1 | %03d | %03d | %03d | %03d |", hornacina[1][0], hornacina[1][1], hornacina[1][2], hornacina[1][3]);
	debug("hornacina[0][hornacina[0][3]] = %d", hornacina[0][hornacina[0][3]]);
	debug("hornacina[1][hornacina[1][3]] = %d", hornacina[1][hornacina[1][3]]);
}

} // End of namespace Tot
