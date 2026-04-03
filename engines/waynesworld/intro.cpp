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

#include "waynesworld/waynesworld.h"
#include "waynesworld/graphics.h"
#include "waynesworld/gxlarchive.h"

#include "audio/audiostream.h"
#include "graphics/paletteman.h"

namespace WaynesWorld {

void WaynesWorldEngine::runIntro() {
	bool continueFl = introPt1();

	if (continueFl)
		continueFl = introPt2();

	if (continueFl)
		continueFl = introPt3(false);

	if (continueFl)
		continueFl = introPt4();

	if (continueFl)
		continueFl = introPt3(false);

	introPt5();

	if (continueFl)
		introPt6();

	introPt7();
}

bool WaynesWorldEngine::introPt1() {
	static const int16 array1[] = {179, 179, 181, 183, 186, 189, 192, 198, 208, 212, 217, 223, 234, 233, 236, 240, 241, 238, 238, 238, 225, 218, 218, 192, 164, 133, 103, 87};
	static const int16 array2[] = {97, 97, 98, 98, 97, 96, 94, 91, 88, 85, 84, 82, 81, 78, 76, 76, 75, 74, 72, 72, 71, 69, 68, 68, 67, 67, 66, 68};
	static const int16 array3[] = {80, 80, 69, 37, 0, 0, 0, 0};
	static const int16 array4[] = {67, 67, 64, 54, 41, 27, 27, 27};

	GxlArchive *oa2Gxl = new GxlArchive("oa2");
	loadPalette(oa2Gxl, "paramnt.pcx");

	_musicIndex = 0;
	changeMusic();

	// "Paramount" background
	drawImageToScreen(oa2Gxl, "paramnt.pcx", 0, 0);
	waitSeconds(1);
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	WWSurface *paramaSprite = new WWSurface(190, 112);
	// "And" animation
	drawImageToSurface(oa2Gxl, "parama.pcx", paramaSprite, 0, 0);
	drawRandomEffect(paramaSprite, 66, 30, 2, 2);
	delete paramaSprite;

	waitSeconds(2);
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	paletteFadeOut(0, 256, 6);

	loadPalette(oa2Gxl, "backg.pcx");
	drawImageToScreen(oa2Gxl, "backg.pcx", 0, 0);

	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	// "Capstone logo" animation
	for (int i = 1; i < 29; ++i) {
		Common::String filename = Common::String::format("cap%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), array1[i - 1], array2[i - 1]);
		waitMillis(30);
	}
	waitSeconds(1);
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	// "The pinnacle of entertainment software" animation
	for (int i = 1; i < 10; ++i) {
		Common::String filename = Common::String::format("txt%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 0, 138);
		waitMillis(30);
	}
	waitSeconds(1);
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	// Shining "Capstone" animation
	for (int i = 1; i < 12; ++i) {
		Common::String filename = Common::String::format("captxt%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 113, 89);
		waitMillis(30);
	}
	waitSeconds(1);
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	// Removing "The pinnacle of entertainment software" animation
	for (int i = 9; i > 0; --i) {
		Common::String filename = Common::String::format("txt%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 0, 138);
		waitMillis(30);
	}
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	drawImageToScreen(oa2Gxl, "txtbkg.pcx", 0, 138);
	waitMillis(30);
	waitSeconds(1);
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	for (int i = 1; i < 7; ++i) {
		Common::String filename = Common::String::format("capsp%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 76, 66);
		waitMillis(30);
	}
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	for (int i = 1; i < 12; ++i) {
		Common::String filename = Common::String::format("presnt%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), 117, 112);
		waitMillis(60);
	}
	if (_escPressed) {
		delete oa2Gxl;
		return false;
	}

	for (int i = 1; i < 9; ++i) {
		Common::String filename = Common::String::format("capawy%02d.pcx", i);
		drawImageToScreen(oa2Gxl, filename.c_str(), array3[i - 1], array4[i - 1]);
		waitMillis(30);
	}

	delete oa2Gxl;
	if (_escPressed) {
		return false;
	}

	return true;
}

bool WaynesWorldEngine::introPt2() {
	paletteFadeOut(0, 256, 64);

	_oanGxl = new GxlArchive("oan");
	loadPalette(_oanGxl, "backg2.pcx");
	_demoPt2Surface = new WWSurface(320, 200);
	_midi->stopSong();

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

void WaynesWorldEngine::cleanPt3() {
	delete _outlineSurface;
	delete _logoSurface;
	delete _backg2Surface;
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

	if (_escPressed) {
		cleanPt3();
		return false;
	}

	_sound->playSound("sv42.snd", false);
	wwEffect(1, 1, flag);
	wwEffect(1, 2, flag);
	wwEffect(1, 3, flag);
	if (_escPressed) {
		cleanPt3();
		return false;
	}

	byte newColor[3] = {0, 0, 0};
	static const byte rArr[] = { 9,  9,  9,  9, 43, 43, 53, 63, 63, 63, 63, 63, 63, 63, 45, 28,  9,  9,  9};
	static const byte gArr[] = {33, 33, 40, 47, 47, 47, 47, 47, 35, 23,  0,  0,  0,  0,  0,  0,  0, 33, 33};
	static const byte bArr[] = {29, 20, 20, 20, 20,  0,  0,  0,  0,  0,  0, 23, 37, 50, 50, 50, 50, 50, 40};

	for (int i = 0; i < 32; ++i) {
		const int index = (i % 19);
		newColor[0] = rArr[index] * 4;
		newColor[1] = gArr[index] * 4;
		newColor[2] = bArr[index] * 4;

		g_system->getPaletteManager()->setPalette((const byte *)&newColor, 236, 1);
		wwEffect((i % 8) + 1, 4, flag);

		if (_escPressed) {
			cleanPt3();
			return false;
		}
	}

	wwEffect(1, 3, flag);
	wwEffect(1, 2, flag);
	wwEffect(1, 1, flag);
	wwEffect(1, 0, flag);
	if (_escPressed) {
		cleanPt3();
		return false;
	}

	while (_sound->isSFXPlaying())
		waitMillis(30);

	_sound->playSound("sv16.snd", false);

	waitSeconds(1);
	paletteFadeOut(0, 256, 4);

	cleanPt3();

	if (_escPressed) {
		return false;
	}

	return true;
}

bool WaynesWorldEngine::introPt4() {
	bool retVal = true;
	introPt4_init();

	if (!introPt4_intro()) {
		retVal = false;
	} else if (!introPt4_displayCallInTime()) {
		retVal = false;
	} else if (!introPt4_caller1()) {
		retVal = false;
	} else if (!introPt4_caller2()) {
		retVal = false;
	} else if (!introPt4_caller3()) {
		retVal = false;
	} else if (!introPt4_caller4()) {
		retVal = false;
	} else
		retVal = introPt4_playGuitar();

	introPt4_cleanup();

	return retVal;
}

void WaynesWorldEngine::introPt5() {
	delete _oanGxl;
	_oanGxl = nullptr;
}

void WaynesWorldEngine::introPt6() {
	WWSurface *introPt6Surface[5] = {nullptr};
	WWSurface *signBottomSurface = nullptr;
	WWSurface *scrollSurface = nullptr;

	_escPressed = false;

	while (_sound->isSFXPlaying())
		waitMillis(10);

	if (_escPressed)
		return;

	GxlArchive *oa3Gxl = new GxlArchive("oa3");
	for (int i = 0; i < 5; ++i) {
		introPt6Surface[i] = new WWSurface(320, 200);
		Common::String filename = Common::String::format("sign%d.pcx", i);
		drawImageToSurface(oa3Gxl, filename.c_str(), introPt6Surface[i], 0, 0);
	}

	_sound->playSound("sv14.snd", false);

	for (int i = 0; i < 5; ++i) {
		_screen->drawSurface(introPt6Surface[i], 0, 0);
		waitMillis(100);
		if (_escPressed) {
			break;
		}
	}

	if (!_escPressed) {
		signBottomSurface = new WWSurface(320, 94);
		drawImageToSurface(oa3Gxl, "signbot.pcx", signBottomSurface, 0, 0);
	}

	if (!_escPressed) {
		_musicIndex = 1;
		changeMusic();
		waitSeconds(4);
	}

	if (!_escPressed) {
		scrollSurface = new WWSurface(320, 200);

		for (int i = 199; i > 106; --i) {
			scrollSurface->copyRectToSurface((Graphics::Surface)*introPt6Surface[4], 0, 0, Common::Rect(0, 200 - i, 319, 200));
			scrollSurface->copyRectToSurface((Graphics::Surface)*signBottomSurface, 0, i, Common::Rect(0, 0, 319, 200 - i));
			_screen->drawSurface(scrollSurface, 0, 0);
		}
	}

	delete scrollSurface;
	delete oa3Gxl;
	delete signBottomSurface;
	for (int i = 0; i < 5; ++i)
		delete introPt6Surface[i];
}

void WaynesWorldEngine::introPt7() {
	_midi->stopSong();
	paletteFadeOut(0, 256, 4);
	_screen->clear(0);
}

void WaynesWorldEngine::sub3009A(int textId) {
	int startPos;
	int textColor;
	int textType = 0;
	Common::String filename;

	switch (textId) {
	case 0:
		filename = "oaw";
		startPos = _startOawPos;
		textColor = 147;
		break;
	case 1:
		filename = "oag";
		startPos = _startOagPos;
		textColor = 41;
		break;
	default:
		filename = "oao";
		startPos = _startOaoPos;
		++_startOaoPos;
		textColor = 11;
		textType = 1;
		break;
	}

	Common::String displayTxt = loadString(filename.c_str(), startPos, 0);

	if (textType)
		_fontWW->drawText(_demoPt2Surface, displayTxt.c_str(), 0, 187, textColor);
	else
		_fontWW->drawText(_demoPt2Surface, displayTxt.c_str(), 0, 2, textColor);
}

void WaynesWorldEngine::sub2FEFB(int arg_refreshBackgFl, int arg_wBodyIndex, int arg_gBodyIndex, int arg_wHead1Index, int arg_gHead1Index, int arg_TextId) {
	_demoPt2Surface->fillRect(0, 0, 319, 14, 0);
	_demoPt2Surface->fillRect(0, 185, 319, 199, 0);

	if (arg_refreshBackgFl != _old_arg_refreshBackgFl) {
		_demoPt2Surface->clear(0);
		_demoPt2Surface->drawSurface(_introBackg1Image, 0, 15);
		_old_arg_refreshBackgFl = arg_refreshBackgFl;
	}

	if (arg_wBodyIndex != _old_arg_wBodyIndex) {
		_demoPt2Surface->drawSurface(_introWbodyImage[arg_wBodyIndex], 0, 21);
		_old_arg_wBodyIndex = arg_wBodyIndex;
	}

	if (arg_gBodyIndex != _old_arg_gBodyIndex) {
		_demoPt2Surface->drawSurface(_introGbodyImage, 160, 25);
		_old_arg_gBodyIndex = arg_gBodyIndex;
	}

	if (arg_wHead1Index != _old_argWHead1Index) {
		_demoPt2Surface->drawSurface(_introWhead1[arg_wHead1Index], 12, 22);
		_old_argWHead1Index = arg_wHead1Index;
	}

	if (arg_gHead1Index != _old_argGHead1Index) {
		_demoPt2Surface->drawSurface(_introGhead1[arg_gHead1Index], 182, 21);
		_old_argGHead1Index = arg_gHead1Index;
	}

	if (arg_TextId != -1) {
		sub3009A(arg_TextId);
	}

	_screen->drawSurface(_demoPt2Surface, 0, 0);
	waitMillis(170);
}

void WaynesWorldEngine::introPt4_init() {
	_fontWW = new GFTFont();
	_fontWW->loadFromFile("ww.gft");

	while(_sound->isSFXPlaying())
		waitMillis(30);

	_musicIndex = 2;
	changeMusic();

	_introBackg1Image = new WWSurface(320, 170);
	drawImageToSurface(_oanGxl, "backg1.pcx", _introBackg1Image, 0, 0);
	for (int i = 0; i < 5; ++i) {
		_introWbodyImage[i] = new WWSurface(145, 118);
		Common::String filename = Common::String::format("wbody%d.pcx", i == 0 ? 0 : i + 8);
		drawImageToSurface(_oanGxl, filename.c_str(), _introWbodyImage[i], 0, 0);
	}
	_introGbodyImage = new WWSurface(160, 149);
	drawImageToSurface(_oanGxl, "gbody0.pcx", _introGbodyImage, 0, 0);

	for (int i = 0; i < 8; ++i) {
		_introWhead1[i] = new WWSurface(98, 71);
		Common::String filename = Common::String::format("whead1%d.pcx", i);
		drawImageToSurface(_oanGxl, filename.c_str(), _introWhead1[i], 0, 0);
	}

	for (int i = 0; i < 11; ++i) {
		_introGhead1[i] = new WWSurface(138, 80);
		Common::String filename = Common::String::format("ghead1%d.pcx", i);
		drawImageToSurface(_oanGxl, filename.c_str(), _introGhead1[i], 0, 0);
	}

	drawImageToScreen(_oanGxl, "backg1.pcx", 0, 15);
	paletteFadeIn(0, 256, 2);
}

bool WaynesWorldEngine::introPt4_intro() {
	_sound->playSound("sv27.snd", false);

	for (int i = 0; i < 12; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
	}

	if (_escPressed) {
		return false;
	}
	++_startOawPos;

	while (_sound->isSFXPlaying())
		waitMillis(30);

	_sound->playSound("sv25.snd", false);

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
	}
	++_startOawPos;

	if (_escPressed) {
		return false;
	}

	while (_sound->isSFXPlaying())
		waitMillis(30);

	_sound->playSound("sv20.snd", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
	}
	++_startOagPos;

	if (_escPressed) {
		return false;
	}

	for (int i = 0; i < 15; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
	}
	++_startOawPos;

	if (_escPressed) {
		return false;
	}

	return true;
}

bool WaynesWorldEngine::introPt4_displayCallInTime() {
	WWSurface *pt4Sub3Surface1 = new WWSurface(178, 21);
	WWSurface *pt4Sub3Surface2 = new WWSurface(178, 21);
	pt4Sub3Surface2->clear(0);
	drawImageToSurface(_oanGxl, "callin.pcx", pt4Sub3Surface1, 0, 0);

	for (int i = 0; i < 5; ++i) {
		drawImageToScreen(_oanGxl, "backg1.pcx", 0, 15);
		waitMillis(500);
		if (_escPressed) {
			break;
		}
		_screen->drawSurfaceTransparent(pt4Sub3Surface1, 66, 157);
		waitMillis(500);
		if (_escPressed) {
			break;
		}
	}

	delete pt4Sub3Surface1;
	delete pt4Sub3Surface2;

	if (_escPressed) {
		return false;
	}

	return true;
}

bool WaynesWorldEngine::introPt4_caller1() {
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 15; ++j) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 5; ++j) {
			sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
			if (_escPressed) {
				return false;
			}
		}
		++_startOagPos;
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	_sound->playSound("sv33.snd", 0);
	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}

	_sound->playSound("sv38.snd", 0);

	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}

	for (int j = 0; j < 15; ++j) {
		sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
		if (_escPressed) {
			return false;
		}
	}
	++_startOagPos;
	_sound->playSound("sv31.snd", 0);

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}
	_sound->playSound("sv28.snd", 0);

	for (int j = 0; j < 5; ++j) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;
	_sound->playSound("sv21.snd", 0);

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 15; ++j) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
	}
	_sound->playSound("sv29.snd", false);

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	if (_escPressed) {
		return false;
	}
	return true;
}

bool WaynesWorldEngine::introPt4_caller2() {
	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
		if (i == 2)
			_sound->playSound("sv19.snd", false);
	}

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
			if (_escPressed) {
				return false;
			}
		}
		++_startOagPos;
	}

	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}

	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
	waitSeconds(2);
	if (_escPressed) {
		return false;
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
			if (_escPressed) {
				return false;
			}
		}
		++_startOagPos;
	}
	_sound->playSound("sv21.snd", false);

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}
	_sound->playSound("sv15.snd", false);

	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}
	_sound->playSound("sv30.snd", false);

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
			if (_escPressed) {
				return false;
			}
		}
		++_startOagPos;
	}
	_sound->playSound("sv22.snd", false);

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	if (_escPressed) {
		return false;
	}
	return true;
}

bool WaynesWorldEngine::introPt4_caller3() {
	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(3);
		if (_escPressed) {
			return false;
		}
	}

	for (int i = 0; i < 10; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;
	++_startOagPos;
	_sound->playSound("sv46.snd", true);

	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	_sound->playSound("sv31.snd", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 0);
		if (_escPressed) {
			return false;
		}
	}

	++_startOawPos;
	++_startOagPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}

	for (int j = 0; j < 4; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}
	_sound->playSound("sv39.snd", false);

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;
	++_startOagPos;

	return true;
}

bool WaynesWorldEngine::introPt4_caller4() {
	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
	waitSeconds(2);
	if (_escPressed) {
		return false;
	}

	for (int i = 0; i < 15; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
			if (_escPressed) {
				return false;
			}
		}
		++_startOagPos;
	}

	_sound->playSound("sv37.snd", true);
	_sound->playSound("sv24.snd", true);

	for (int i = 0; i < 15; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
	waitSeconds(2);
	if (_escPressed) {
		return false;
	}

	_sound->playSound("sv06.snd", false);

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}
	_sound->playSound("sv32.snd", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
		if (_escPressed) {
			return false;
		}
	}
	++_startOagPos;

	sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
	waitSeconds(1);
	if (_escPressed) {
		return false;
	}

	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 8; ++i) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}

	_sound->playSound("sv18.snd", false);

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
		if (_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 6; ++i) {
		sub2FEFB(1, 0, 1, getRandom(3), getRandom(11), 2);
		waitSeconds(2);
		if (_escPressed) {
			return false;
		}
	}

	_sound->playSound("sv45.snd", true);

	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 8; ++i) {
			sub2FEFB(1, 0, 1, getRandom(3), 9, 0);
			if (_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	_sound->playSound("sv34.snd", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, 0, getRandom(11), 1);
		if (_escPressed) {
			return false;
		}
	}
	++_startOagPos;

	return true;
}

void WaynesWorldEngine::introPt4_cleanup() {
	delete _fontWW;
	_fontWW = nullptr;
	delete _introBackg1Image;
	_introBackg1Image = nullptr;
	for (int i = 0; i < 5; ++i) {
		delete _introWbodyImage[i];
		_introWbodyImage[i] = nullptr;
	}
	delete _introGbodyImage;
	_introGbodyImage = nullptr;
	for (int i = 0; i < 7; ++i) {
		delete _introWhead1[i];
		_introWhead1[i] = nullptr;
	}
	for (int i = 0; i < 11; ++i) {
		delete _introGhead1[i];
		_introGhead1[i] = nullptr;
	}
}

bool WaynesWorldEngine::introPt4_playGuitar() {
	sub2FEFB(1, 1, 1, 0, 9, -1);
	sub2FEFB(1, 2, 1, 0, 9, -1);
	sub2FEFB(1, 3, 1, 0, 9, -1);
	sub2FEFB(1, 4, 1, 0, 9, -1);

	_midi->stopSong();

	if (_escPressed) {
		return false;
	}

	return true;
}

} // End of namespace WaynesWorld
