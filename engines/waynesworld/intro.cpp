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

#include "waynesworld/waynesworld.h"
#include "waynesworld/graphics.h"
#include "waynesworld/gxlarchive.h"

#include "audio/audiostream.h"
#include "graphics/paletteman.h"

namespace WaynesWorld {

void WaynesWorldEngine::runIntro() {
	if (!introPt1())
		return;
	if (!introPt2())
		return;
	if (!introPt3(false))
		return;
}

bool WaynesWorldEngine::introPt1() {
	static int16 array1[] = {179, 179, 181, 183, 186, 189, 192, 198, 208, 212, 217, 223, 234, 233, 236, 240, 241, 238, 238, 238, 225, 218, 218, 192, 164, 133, 103, 87};
	static int16 array2[] = {97, 97, 98, 98, 97, 96, 94, 91, 88, 85, 84, 82, 81, 78, 76, 76, 75, 74, 72, 72, 71, 69, 68, 68, 67, 67, 66, 68};
	static int16 array3[] = {80, 80, 69, 37, 0, 0, 0, 0};
	static int16 array4[] = {67, 67, 64, 54, 41, 27, 27, 27};

	GxlArchive *oa2Gxl = new GxlArchive("oa2");
	loadPalette(oa2Gxl, "paramnt.pcx");

	_musicIndex = 0;
	changeMusic();

	// "Paramount" background
	drawImageToScreen(oa2Gxl, "paramnt.pcx", 0, 0);
	waitSeconds(1);

	WWSurface *paramaSprite = new WWSurface(190, 112);
	// "And" animation
	drawImageToSurface(oa2Gxl, "parama.pcx", paramaSprite, 0, 0);
	drawRandomEffect(paramaSprite, 66, 30, 2, 2);
	delete paramaSprite;

	waitSeconds(2);

	paletteFadeOut(0, 256, 6);

	loadPalette(oa2Gxl, "backg.pcx");
	drawImageToScreen(oa2Gxl, "backg.pcx", 0, 0);

	// "Capstone logo" animation
	for (int i = 1; i < 29; ++i) {
		Common::String filename = Common::String::format("cap%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), array1[i - 1], array2[i - 1]);
		waitMillis(30);
	}
	waitSeconds(1);

	// "The pinnacle of entertainment software" animation
	for (int i = 1; i < 10; ++i) {
		Common::String filename = Common::String::format("txt%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 0, 138);
		waitMillis(30);
	}
	waitSeconds(1);

	// Shining "Capstone" animation
	for (int i = 1; i < 12; ++i) {
		Common::String filename = Common::String::format("captxt%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 113, 89);
		waitMillis(30);
	}
	waitSeconds(1);

	// Removing "The pinnacle of entertainment software" animation
	for (int i = 9; i > 0; --i) {
		Common::String filename = Common::String::format("txt%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 0, 138);
		waitMillis(30);
	}

	drawImageToScreen(oa2Gxl, "txtbkg.pcx", 0, 138);
	waitMillis(30);
	waitSeconds(1);

	for (int i = 1; i < 7; ++i) {
		Common::String filename = Common::String::format("capsp%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 76, 66);
		waitMillis(30);
	}

	for (int i = 1; i < 12; ++i) {
		Common::String filename = Common::String::format("presnt%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 117, 112);
		waitMillis(60);
	}

	for (int i = 1; i < 9; ++i) {
		Common::String filename = Common::String::format("capawy%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), array3[i - 1], array4[i - 1]);
		waitMillis(30);
	}

	delete oa2Gxl;

	// TODO add a check at each step to return false if ESC is pressed
	return true;
}

bool WaynesWorldEngine::introPt2() {
	paletteFadeOut(0, 256, 64);

	_oanGxl = new GxlArchive("oan");
	loadPalette(_oanGxl, "backg2.pcx");
	_demoPt2Surface = new WWSurface(320, 200);
	_midi->stopSong();

	// TODO add a check at each step to return false if ESC is pressed
	return true;
}

void WaynesWorldEngine::wwEffect(int arg0, int arg1, bool flag) {
	int xmult = 0;
	int ymult = 0;

	switch (arg0) {
	case 1:
		xmult = 10;
		ymult = 7;
		break;
	case 2:
		xmult = 0;
		ymult = 7;
		break;
	case 3:
		xmult = -10;
		ymult = 7;
		break;
	case 4:
		xmult = -10;
		ymult = 0;
		break;
	case 5:
		xmult = -10;
		ymult = -7;
		break;
	case 6:
		xmult = 0;
		ymult = -7;
		break;
	case 7:
		xmult = 10;
		ymult = -7;
		break;
	case 8:
		xmult = 10;
		ymult = 0;
		break;
	default:
		break;
	}

	_demoPt2Surface->clear(0);
	if (flag) {
		_demoPt2Surface->drawSurface(_backg2Surface, 0, 15);
	}

	for (int i = 4; i > 0; --i) {
		if (i <= arg1) {
			_demoPt2Surface->drawSurfaceTransparent(_outlineSurface, (i * xmult) + 47, (i * ymult) + 25);
		}
	}

	_demoPt2Surface->drawSurfaceTransparent(_logoSurface, 47, 25);
	_screen->drawSurface(_demoPt2Surface, 0, 0);

	// Added for better results
	waitMillis(200);
}

bool WaynesWorldEngine::introPt3(bool flag) {
	// sub1
	_backg2Surface = new WWSurface(320, 170);
	_logoSurface = new WWSurface(226, 134);
	_outlineSurface = new WWSurface(226, 134);

	drawImageToSurface(_oanGxl, "backg2.pcx", _backg2Surface, 0, 0);
	drawImageToSurface(_oanGxl, "logo.pcx", _logoSurface, 0, 0);
	drawImageToSurface(_oanGxl, "outline.pcx", _outlineSurface, 0, 0);
	// End of sub1

	wwEffect(1, 0, flag);

	if (flag)
		waitSeconds(1);

	_sound->playSound("sv42.snd", false);
	wwEffect(1, 1, flag);
	wwEffect(1, 2, flag);
	wwEffect(1, 3, flag);

	byte newColor[3] = {0, 0, 0};
	static byte rArr[] = { 9,  9,  9,  9, 43, 43, 53, 63, 63, 63, 63, 63, 63, 63, 45, 28,  9,  9,  9};
	static byte gArr[] = {33, 33, 40, 47, 47, 47, 47, 47, 35, 23,  0,  0,  0,  0,  0,  0,  0, 33, 33};
	static byte bArr[] = {29, 20, 20, 20, 20,  0,  0,  0,  0,  0,  0, 23, 37, 50, 50, 50, 50, 50, 40};

	for (int i = 0; i < 32; ++i) {
		const int index = (i % 19);
		newColor[0] = rArr[index] * 4;
		newColor[1] = gArr[index] * 4;
		newColor[2] = bArr[index] * 4;

		g_system->getPaletteManager()->setPalette((const byte *)&newColor, 236, 1);
		wwEffect((i % 8) + 1, 4, flag);
	}

	wwEffect(1, 3, flag);
	wwEffect(1, 2, flag);
	wwEffect(1, 1, flag);
	wwEffect(1, 0, flag);

	while (_sound->isSFXPlaying())
		waitMillis(30);

	_sound->playSound("sv16.snd", false);

	while (_sound->isSFXPlaying())
		waitMillis(30);

	waitSeconds(1);

	delete _outlineSurface;
	delete _logoSurface;
	delete _backg2Surface;

	// TODO add a check at each step to return false if ESC is pressed
	return true;
}


} // End of namespace WaynesWorld
