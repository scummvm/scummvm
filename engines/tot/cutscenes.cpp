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

#include "common/config-manager.h"

#include "tot/anims.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

byte *drawCreditsScreen(uint &sizeAuxBG, byte *&auxBG) {
	Palette intermediatePalette, darkPalette;

	Common::File ppFile;

	if (!ppFile.open("DIPLOMA.PAN")) {
		showError(315);
	}
	byte *backgroundPointer = (byte *)malloc(320 * 200);
	ppFile.read(intermediatePalette, 256 * 3);
	ppFile.read(backgroundPointer, 320 * 200);
	ppFile.close();

	g_engine->_graphics->drawFullScreen(backgroundPointer);

	sizeAuxBG = imagesize(0, 0, 319, 59);

	// Screen is now backgroundPointer so auxBG contains a 320x60 crop of backgroundPointer
	auxBG = (byte *)malloc(sizeAuxBG);
	g_engine->_graphics->getImg(0, 0, 319, 59, auxBG);

	for (int i = 0; i < 256; i++) {
		darkPalette[i * 3 + 0] = 0;
		darkPalette[i * 3 + 1] = 0;
		darkPalette[i * 3 + 2] = 0;
		// Adjust for 6-bit DAC color
		intermediatePalette[i * 3 + 0] <<= 2;
		intermediatePalette[i * 3 + 1] <<= 2;
		intermediatePalette[i * 3 + 2] <<= 2;
	}

	g_engine->_graphics->fadePalettes(darkPalette, intermediatePalette);
	g_engine->_graphics->copyPalette(intermediatePalette, g_engine->_graphics->_pal);
	if (g_engine->_cpCounter2 > 9)
		showError(274);
	return backgroundPointer;
}

void putCreditsImg(uint x, uint y, const byte *img1, const byte *img2, bool direct) {

	uint16 wImg1, hImg1;
	uint horizontalAux;
	byte *step;

	wImg1 = READ_LE_UINT16(img1);
	hImg1 = READ_LE_UINT16(img1 + 2);

	step = (byte *)malloc((wImg1 + 1) * (hImg1 + 1) + 4);

	horizontalAux = wImg1 + 1;
	uint yPos = hImg1 + y;

	// makes sure that if the image is at the bottom of the screen we chop the bottom part
	for (int i = yPos; i >= 200; i--)
		hImg1 -= 1;

	hImg1++;

	// Copies the crop in the background corresponding to the current credit window in img1
	for (int i = 0; i < hImg1; i++) {
		const byte *src = img2 + (320 * (y + i)) + x;
		byte *dst = step + 4 + (horizontalAux * i);
		Common::copy(src, src + horizontalAux, dst);
	}

	for (int kk = 0; kk < hImg1; kk++) {
		uint inc2 = (kk * wImg1) + 4;
		yPos = kk + y;
		for (int jj = 0; jj <= wImg1; jj++) {
			uint inc = inc2 + jj;
			if ((direct && img1[inc] > 0) || (img1[inc] > 16 && yPos >= 66 && yPos <= 192)) {
				step[inc] = img1[inc];
			} else if (img1[inc] > 16) {
				switch (yPos) {
				case 59:
				case 199:
					step[inc] = img1[inc] + 210;
					break;
				case 60:
				case 198:
					step[inc] = img1[inc] + 180;
					break;
				case 61:
				case 197:
					step[inc] = img1[inc] + 150;
					break;
				case 62:
				case 196:
					step[inc] = img1[inc] + 120;
					break;
				case 63:
				case 195:
					step[inc] = img1[inc] + 90;
					break;
				case 64:
				case 194:
					step[inc] = img1[inc] + 60;
					break;
				case 65:
				case 193:
					step[inc] = img1[inc] + 30;
					break;
				}
			}
		}
	}
	// Wait until render tick
	do {
		g_engine->_chrono->updateChrono();
		g_system->delayMillis(10);
	} while (!g_engine->_chrono->_gameTick && !g_engine->shouldQuit());
	g_engine->_chrono->_gameTick = false;

	// Copies the credit window directly to the screen
	for (int i = 0; i < hImg1; i++) {
		const byte *src = step + 4 + (horizontalAux * i);
		byte *dst = ((byte *)g_engine->_screen->getPixels()) + (320 * (y + i)) + x;
		Common::copy(src, src + horizontalAux, dst);
	}
	g_engine->_screen->addDirtyRect(Common::Rect(x, y, x + wImg1 + 1, y + hImg1 + 1));
	free(step);
}

void scrollCredit(
	int32 position,
	uint size,
	Palette &pal2,
	byte *&background,
	bool &exit,
	int minHeight,
	bool withFade,
	bool refresh) {
	Common::File creditFile;
	if (!creditFile.open("CREDITOS.DAT")) {
		showError(270);
	}
	creditFile.seek(position);
	creditFile.read(g_engine->_sceneBackground, size);
	creditFile.read(pal2, 768);
	creditFile.close();

	for (int i = 16; i < 256; i++) {
		// Adjust for 6-bit DAC
		pal2[i * 3 + 0] <<= 2;
		pal2[i * 3 + 1] <<= 2;
		pal2[i * 3 + 2] <<= 2;

		g_engine->_graphics->_pal[i * 3 + 0] = pal2[i * 3 + 0];
		g_engine->_graphics->_pal[i * 3 + 1] = pal2[i * 3 + 1];
		g_engine->_graphics->_pal[i * 3 + 2] = pal2[i * 3 + 2];
	}

	g_engine->_graphics->setPalette(&g_engine->_graphics->_pal[16 * 3 + 0], 16, 240);
	// Loops an image from the bottom of the screen to the top
	for (int i = 199; i >= minHeight; i--) {
		g_engine->_events->pollEvent();

		putCreditsImg(85, i, g_engine->_sceneBackground, background, !withFade);
		if (g_engine->_events->_keyPressed) {
			exit = true;
			break;
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
		if (g_engine->shouldQuit())
			break;
	}
	if (refresh) {
		g_engine->_graphics->copyFromScreen(background);
	}
}

void scrollSingleCredit(
	int32 pos,
	uint size,
	Palette &pal2,
	byte *&background,
	bool &exit) {
	scrollCredit(
		pos,
		size,
		pal2,
		background,
		exit,
		8,
		true,
		false);
}

void removeTitle(byte *&background2) {
	uint i2, j2;
	for (int i1 = 1; i1 <= 15000; i1++) {
		g_engine->_events->pollEvent();
		i2 = g_engine->getRandomNumber(318);
		j2 = getRandom(58);
		byte *src = background2 + 4 + (j2 * 320) + i2;
		byte *dest = ((byte *)g_engine->_screen->getPixels()) + (j2 * 320) + i2;
		Common::copy(src, src + 2, dest);

		byte *src2 = background2 + 4 + ((j2 + 1) * 320) + i2;
		byte *dest2 = ((byte *)g_engine->_screen->getPixels()) + ((j2 + 1) * 320) + i2;

		Common::copy(src2, src2 + 2, dest2);

		i2 = getRandom(320);
		j2 = getRandom(60);

		byte *src3 = background2 + 4 + (j2 * 320) + i2;
		byte *dest3 = ((byte *)g_engine->_screen->getPixels()) + (j2 * 320) + i2;
		Common::copy(src3, src3 + 1, dest3);
		if (i1 % 200 == 0) {
			g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 60));
			g_engine->_screen->update();
		}
		if (g_engine->shouldQuit()) {
			break;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(0, 0, 320, 60));
	g_engine->_screen->update();
}

inline bool keyPressed() {

	g_engine->_events->pollEvent();
	return g_engine->_events->_keyPressed;
}

void TotEngine::credits() {
	_saveAllowed = true;
	Palette pal2;
	byte *background2;
	uint sizeBg2;
	bool shouldExit = false;

	_mouse->hide();
	_graphics->totalFadeOut(0);
	_sound->fadeOutMusic();
	_screen->clear();
	_sound->playMidi("CREDITOS", true);
	_sound->fadeInMusic();
	byte *background = drawCreditsScreen(sizeBg2, background2);

	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollCredit(0, 8004, pal2, background, shouldExit, 10, false, true);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(8772, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(17544, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(26316, 7504, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(34588, 7504, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(42860, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(51632, 7504, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	removeTitle(background2);
	if (keyPressed() || shouldExit)
		goto Lexit;
	_graphics->putImg(0, 0, background2);
	if (keyPressed() || shouldExit)
		goto Lexit;
	_graphics->copyFromScreen(background);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollCredit(59904, 8004, pal2, background, shouldExit, 10, false, true);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(68676, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(77448, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(86220, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(94992, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(103764, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollSingleCredit(112536, 8004, pal2, background, shouldExit);
	if (keyPressed() || shouldExit)
		goto Lexit;
	removeTitle(background2);
	if (keyPressed() || shouldExit)
		goto Lexit;
	_graphics->putImg(0, 0, background2);
	if (keyPressed() || shouldExit)
		goto Lexit;
	_graphics->copyFromScreen(background);
	if (keyPressed() || shouldExit)
		goto Lexit;
	scrollCredit(121308, 8004, pal2, background, shouldExit, 80, false, true);
Lexit:
	delay(1000);
	_graphics->totalFadeOut(0);
	_sound->fadeOutMusic();
	_screen->clear();
	_sound->playMidi("INTRODUC", true);
	_sound->fadeInMusic();
	_mouse->show();
	free(background);
	free(background2);
}

void TotEngine::introduction() {
	_saveAllowed = false;
	_mouse->hide();
	bool exitPressed;
	uint loopCount;
	bool isSpanish = isLanguageSpanish();
	const char *const *messages = getFullScreenMessagesByCurrentLanguage();
	const int32 *offsets = getOffsetsByCurrentLanguage();
	exitPressed = false;
	_graphics->totalFadeOut(0);

	if (_cpCounter > 6)
		showError(270);
	_screen->clear();
	drawFlc(136, 53, offsets[2], 136, 9, 1, true, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(135, 54, offsets[3], 0, 9, 2, true, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	_graphics->totalFadeOut(0);
	_screen->clear();
	if (isSpanish) {
		littText(25, 20, messages[0], 253);
		littText(25, 35, messages[1], 253);
		littText(25, 50, messages[2], 253);
		littText(25, 65, messages[3], 253);
		littText(25, 80, messages[4], 253);
		littText(25, 95, messages[5], 253);
		littText(25, 120, messages[6], 253);
		littText(25, 140, messages[7], 253);
		littText(25, 155, messages[8], 253);
	} else {
		littText(25, 35, messages[0], 253);
		littText(25, 55, messages[1], 253);
		littText(25, 75, messages[2], 253);
		littText(25, 95, messages[3], 253);
		littText(25, 115, messages[4], 253);
		littText(25, 135, messages[5], 253);
	}
	if (shouldQuit()) {
		return;
	}
	_graphics->totalFadeIn(0, "DEFAULT");
	_screen->markAllDirty();
	_screen->update();
	loopCount = 0;

	do {
		_chrono->updateChrono();
		g_engine->_events->pollEvent();
		if (g_engine->_events->_keyPressed || g_engine->_events->_leftMouseButton) {
			goto LexitIntro;
		}

		if (g_engine->_chrono->_gameTick) {
			loopCount += 1;
		}
		g_system->delayMillis(10);
	} while (loopCount < 180 && !shouldQuit());

	_graphics->totalFadeOut(0);
	_screen->clear();
	drawFlc(0, 0, offsets[4], 0, 9, 3, true, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(110, 30, offsets[5], 2, 9, 4, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(110, 30, offsets[6], 3, 9, 5, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(110, 30, offsets[7], 0, 9, 0, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(110, 30, offsets[8], isSpanish ? 4 : 8, 9, 6, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(110, 30, offsets[9], 3, 9, 7, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(110, 30, offsets[8], isSpanish ? 3 : 8, 9, 8, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(110, 30, offsets[9], 2, 9, 9, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(0, 0, offsets[10], 0, 9, 0, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(235, 100, offsets[11], 3, 9, 10, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(150, 40, offsets[12], 0, 9, 11, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(235, 100, offsets[11], 3, 9, 12, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(150, 40, offsets[12], isSpanish ? 0 : 2, 9, 13, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(235, 100, offsets[11], isSpanish ? 3 : 8, 9, 14, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(150, 40, offsets[12], 0, 9, 15, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(173, 98, offsets[13], 0, 9, 0, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(224, 100, offsets[14], 2, 9, 16, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(0, 0, offsets[15], 0, 18, 17, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	drawFlc(150, 40, offsets[16], 0, 9, 18, false, true, false, exitPressed);
	if (exitPressed)
		goto LexitIntro;
	delay(1000);
LexitIntro:
	_graphics->totalFadeOut(0);
	_screen->clear();
	_mouse->show();
}

void TotEngine::firstIntroduction() {
	if (!_firstTimeDone && !_isIntroSeen) {
		introduction();
		_firstTimeDone = true;
		ConfMan.setBool("introSeen", true);
		ConfMan.flushToDisk();
	}
}

void TotEngine::initialLogo() {
	bool foobar = false;
	const int32 *offsets = getOffsetsByCurrentLanguage();
	drawFlc(0, 0, offsets[0], 0, 18, 25, false, false, false, foobar);
	delay(1000);
}

void TotEngine::sacrificeScene() {
	_saveAllowed = false;
	Palette palaux;

	Common::File file;
	bool isSpanish = isLanguageSpanish();
	const char *const *messages = getFullScreenMessagesByCurrentLanguage();

	const int32 *offsets = getOffsetsByCurrentLanguage();

	_sound->stopVoc();
	bool exitPressed = _currentRoomData->paletteAnimationFlag;
	_currentRoomData->paletteAnimationFlag = false;

	bar(0, 139, 319, 149, 0);
	bar(10, 10, 310, 120, 0);
	if (isSpanish) {
		littText(10, 10, messages[9], 200);
		littText(10, 30, messages[10], 200);
		littText(10, 50, messages[11], 200);
		littText(10, 70, messages[12], 200);
		littText(10, 90, messages[13], 200);
	} else {
		littText(10, 20, messages[9], 200);
		littText(10, 40, messages[10], 200);
		littText(10, 60, messages[11], 200);
		littText(10, 80, messages[12], 200);
		littText(10, 100, messages[13], 200);
	}

	for (int i = 0; i <= 28; i++)
		setRGBPalette(200, i * 2, i * 2, i * 2);

	delay(10000);
	if (shouldQuit())
		return;

	for (int i = 28; i >= 0; i--)
		setRGBPalette(200, i * 2, i * 2, i * 2);

	bar(10, 10, 310, 120, 0);
	if (isSpanish) {
		littText(10, 10, messages[14], 200);
		littText(10, 30, messages[15], 200);
		littText(10, 50, messages[16], 200);
		littText(10, 70, messages[17], 200);
		littText(10, 90, messages[18], 200);
	} else {
		littText(10, 20, messages[14], 200);
		littText(10, 40, messages[15], 200);
		littText(10, 60, messages[16], 200);
		littText(10, 80, messages[17], 200);
	}

	for (int i = 0; i <= 28; i++)
		setRGBPalette(200, i * 2, i * 2, i * 2);
	delay(10000);
	if (shouldQuit())
		return;

	for (int i = 28; i >= 0; i--)
		setRGBPalette(200, i * 2, i * 2, i * 2);

	bar(10, 10, 310, 120, 0);
	if (isSpanish) {
		littText(10, 10, messages[19], 200);
		littText(10, 50, messages[20], 200);
		littText(10, 70, messages[21], 200);
		littText(10, 90, messages[22], 200);
	} else {
		littText(10, 30, messages[19], 200);
		littText(10, 60, messages[20], 200);
		littText(10, 80, messages[21], 200);
	}

	for (int i = 0; i <= 28; i++)
		setRGBPalette(200, i * 2, i * 2, i * 2);
	delay(10000);
	if (shouldQuit())
		return;

	for (int i = 28; i >= 0; i--)
		setRGBPalette(200, i * 2, i * 2, i * 2);
	bar(10, 10, 310, 120, 0);

	if (!file.open("SALONREC.PAN")) {
		showError(318);
	}
	file.read(palaux, 768);
	file.read(_sceneBackground, 44800);
	file.close();

	g_engine->_graphics->_pal[0] = 0;
	g_engine->_graphics->_pal[1] = 0;
	g_engine->_graphics->_pal[2] = 0;
	for (int i = 1; i <= 234; i++) {
		g_engine->_graphics->_pal[i * 3 + 1] = palaux[i * 3 + 1] << 2;
		g_engine->_graphics->_pal[i * 3 + 2] = palaux[i * 3 + 2] << 2;
		g_engine->_graphics->_pal[i * 3 + 3] = palaux[i * 3 + 3] << 2;
	}

	// We dont have the width and height here in the byte buffer
	_graphics->drawScreen(_sceneBackground, false);
	_graphics->partialFadeIn(234);
	_sound->stopVoc();

	if (shouldQuit())
		return;

	drawFlc(0, 0, offsets[17], 0, 9, 19, false, false, true, exitPressed);
	_graphics->totalFadeOut(128);
	_sound->stopVoc();
	delay(1000);
	if (shouldQuit())
		return;

	_sound->fadeOutMusic();
	_sound->playMidi("SACRIFIC", true);
	_sound->fadeInMusic();
	_graphics->clear();

	littText(10, 31, messages[23], 254);
	littText(10, 29, messages[23], 254);
	littText(11, 30, messages[23], 254);
	littText(9, 30, messages[23], 254);

	littText(10, 51, messages[24], 254);
	littText(10, 49, messages[24], 254);
	littText(11, 50, messages[24], 254);
	littText(9, 50, messages[24], 254);

	littText(10, 71, messages[25], 254);
	littText(10, 69, messages[25], 254);
	littText(11, 70, messages[25], 254);
	littText(9, 70, messages[25], 254);

	littText(10, 30, messages[23], 255);
	littText(10, 50, messages[24], 255);
	littText(10, 70, messages[25], 255);

	for (int i = 0; i <= 31; i++) {
		setRGBPalette(255, 32 + i, i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}

	delay(10000);
	if (shouldQuit())
		return;

	for (int i = 31; i >= 0; i--) {
		setRGBPalette(255, 63 - (32 - i), i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(2000);

	if (!file.open("SACRIFIC.PAN")) {
		showError(318);
	}
	file.read(palaux, 768);

	for (int i = 0; i < 256; i++) {
		palaux[i * 3 + 0] <<= 2;
		palaux[i * 3 + 1] <<= 2;
		palaux[i * 3 + 2] <<= 2;
	}

	file.read(_sceneBackground, 64000);
	file.close();
	_graphics->drawFullScreen(_sceneBackground);

	palaux[0] = 0;
	palaux[1] = 0;
	palaux[2] = 0;

	_graphics->redFadeIn(palaux);

	drawFlc(112, 57, offsets[18], 33, 9, 20, true, false, true, exitPressed);
	_sound->autoPlayVoc("REZOS", 0, 0);
	if (shouldQuit())
		return;

	drawFlc(42, 30, offsets[19], 0, 9, 27, false, false, false, exitPressed);

	if (shouldQuit())
		return;

	_graphics->totalFadeOut(128);
	_sound->stopVoc();
	_graphics->clear();

	littText(10, 21, messages[26], 254);
	littText(10, 19, messages[26], 254);
	littText(11, 20, messages[26], 254);
	littText(9, 20, messages[26], 254);

	littText(10, 41, messages[27], 254);
	littText(10, 39, messages[27], 254);
	littText(11, 40, messages[27], 254);
	littText(9, 40, messages[27], 254);

	littText(10, 61, messages[28], 254);
	littText(10, 59, messages[28], 254);
	littText(11, 60, messages[28], 254);
	littText(9, 60, messages[28], 254);

	littText(10, 81, messages[29], 254);
	littText(10, 79, messages[29], 254);
	littText(11, 80, messages[29], 254);
	littText(9, 80, messages[29], 254);

	littText(10, 101, messages[30], 254);
	littText(10, 99, messages[30], 254);
	littText(11, 100, messages[30], 254);
	littText(9, 100, messages[30], 254);

	littText(10, 121, messages[31], 254);
	littText(10, 119, messages[31], 254);
	littText(11, 120, messages[31], 254);
	littText(9, 120, messages[31], 254);

	littText(10, 141, messages[32], 254);
	littText(10, 139, messages[32], 254);
	littText(11, 140, messages[32], 254);
	littText(9, 140, messages[32], 254);

	if (!isSpanish) {
		littText(10, 161, messages[56], 254);
		littText(10, 159, messages[56], 254);
		littText(11, 160, messages[56], 254);
		littText(9, 160, messages[56], 254);

		littText(10, 181, messages[57], 254);
		littText(10, 179, messages[57], 254);
		littText(11, 180, messages[57], 254);
		littText(9, 180, messages[57], 254);
	}

	littText(10, 20, messages[26], 255);
	littText(10, 40, messages[27], 255);
	littText(10, 60, messages[28], 255);
	littText(10, 80, messages[29], 255);
	littText(10, 100, messages[30], 255);
	littText(10, 120, messages[31], 255);
	littText(10, 140, messages[32], 255);

	if (!isSpanish) {
		littText(10, 160, messages[56], 255);
		littText(10, 180, messages[57], 255);
	}

	for (int i = 0; i <= 31; i++) {
		setRGBPalette(255, 32 + i, i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(16000);
	if (shouldQuit())
		return;

	for (int i = 31; i >= 0; i--) {
		setRGBPalette(255, 63 - (32 - i), i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(2000);
	if (shouldQuit())
		return;

	_graphics->clear();

	littText(10, 21, messages[33], 254);
	littText(10, 19, messages[33], 254);
	littText(11, 20, messages[33], 254);
	littText(9, 20, messages[33], 254);

	littText(10, 41, messages[34], 254);
	littText(10, 39, messages[34], 254);
	littText(11, 40, messages[34], 254);
	littText(9, 40, messages[34], 254);

	littText(10, 61, messages[35], 254);
	littText(10, 59, messages[35], 254);
	littText(11, 60, messages[35], 254);
	littText(9, 60, messages[35], 254);
	if (isSpanish) {
		littText(10, 81, messages[36], 254);
		littText(10, 79, messages[36], 254);
		littText(11, 80, messages[36], 254);
		littText(9, 80, messages[36], 254);

		littText(10, 101, messages[37], 254);
		littText(10, 99, messages[37], 254);
		littText(11, 100, messages[37], 254);
		littText(9, 100, messages[37], 254);

		littText(10, 121, messages[38], 254);
		littText(10, 119, messages[38], 254);
		littText(11, 120, messages[38], 254);
		littText(9, 120, messages[38], 254);

		littText(10, 141, messages[39], 254);
		littText(10, 139, messages[39], 254);
		littText(11, 140, messages[39], 254);
		littText(9, 140, messages[39], 254);
	}

	littText(10, 20, messages[33], 255);
	littText(10, 40, messages[34], 255);
	littText(10, 60, messages[35], 255);
	if (isSpanish) {
		littText(10, 80, messages[36], 255);
		littText(10, 100, messages[37], 255);
		littText(10, 120, messages[38], 255);
		littText(10, 140, messages[39], 255);
	}

	for (int i = 0; i <= 31; i++) {
		setRGBPalette(255, 32 + i, i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(13000);
	if (shouldQuit())
		return;

	for (int i = 31; i >= 0; i--) {
		setRGBPalette(255, 63 - (32 - i), i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(2000);
	if (shouldQuit())
		return;
	_graphics->clear();

	littText(10, 61, messages[40], 254);
	littText(10, 59, messages[40], 254);
	littText(11, 60, messages[40], 254);
	littText(9, 60, messages[40], 254);

	littText(10, 81, messages[41], 254);
	littText(10, 79, messages[41], 254);
	littText(11, 80, messages[41], 254);
	littText(9, 80, messages[41], 254);

	littText(10, 101, messages[42], 254);
	littText(10, 99, messages[42], 254);
	littText(11, 100, messages[42], 254);
	littText(9, 100, messages[42], 254);

	littText(10, 60, messages[40], 255);
	littText(10, 80, messages[41], 255);
	littText(10, 100, messages[42], 255);

	for (int i = 0; i <= 31; i++) {
		setRGBPalette(255, 32 + i, i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(7000);
	if (shouldQuit())
		return;
	for (int i = 31; i >= 0; i--) {
		setRGBPalette(255, 63 - (32 - i), i * 2, i * 2);
		setRGBPalette(254, 32 - i, 0, 0);
	}
	delay(2000);
	_graphics->totalFadeOut(0);
	_currentRoomData->paletteAnimationFlag = exitPressed;
	_saveAllowed = true;
}

void TotEngine::ending() {
	_saveAllowed = false;
	bool exitRequested;

	const char *const *messages = getFullScreenMessagesByCurrentLanguage();
	const int32 *offsets = getOffsetsByCurrentLanguage();

	littText(10, 41, messages[43], 249);
	littText(10, 39, messages[43], 249);
	littText(11, 40, messages[43], 249);
	littText(9, 40, messages[43], 249);

	littText(10, 61, messages[44], 249);
	littText(10, 59, messages[44], 249);
	littText(11, 60, messages[44], 249);
	littText(9, 60, messages[44], 249);

	littText(10, 40, messages[43], 253);
	littText(10, 60, messages[44], 253);
	if (shouldQuit()) {
		return;
	}
	delay(4000);
	_graphics->totalFadeOut(0);
	_graphics->clear();
	if (shouldQuit()) {
		return;
	}
	_sound->fadeOutMusic();
	_sound->playMidi("SACRIFIC", true);
	_sound->fadeInMusic();
	drawFlc(0, 0, offsets[30], 12, 9, 26, true, false, false, exitRequested);
	if (exitRequested) {
		return;
	}
	drawFlc(0, 0, offsets[31], 0, 9, 0, false, false, false, exitRequested);
	if (exitRequested) {
		return;
	}
	delay(1000);
	_sound->playVoc("NOOO", 0, 0);
	delay(3000);
	_saveAllowed = true;
}

void TotEngine::wcScene() {
	Palette wcPalette;
	_currentZone = _currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount];
	goToObject(_currentZone, _targetZone);

	_graphics->copyPalette(g_engine->_graphics->_pal, wcPalette);
	_mouse->hide();

	_graphics->partialFadeOut(234);

	const char *const *messages = getFullScreenMessagesByCurrentLanguage();

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
	_graphics->drawScreen(_sceneBackground);
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

} // End of namespace Tot
