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

#include "waynesworld/wwintro.h"
#include "waynesworld/waynesworld.h"
#include "waynesworld/graphics.h"
#include "waynesworld/gxlarchive.h"

#include "audio/audiostream.h"
#include "graphics/paletteman.h"

namespace WaynesWorld {

WWIntro_full::WWIntro_full(WaynesWorldEngine *vm) : WWIntro(vm) {
}

WWIntro_full::~WWIntro_full() {
}

void WWIntro_full::runIntro() {
	bool continueFl = introPt1();

	if (continueFl)
		continueFl = initOanGxl();

	if (continueFl)
		continueFl = introPt3(false);

	if (continueFl)
		continueFl = introPt4();

	if (continueFl)
		continueFl = introPt3(false);

	cleanOanGxl();

	if (continueFl)
		introDisplaySign();

	introPt7();
}

bool WWIntro_full::introPt1() {
	static const int16 array1[] = {179, 179, 181, 183, 186, 189, 192, 198, 208, 212, 217, 223, 234, 233, 236, 240, 241, 238, 238, 238, 225, 218, 218, 192, 164, 133, 103, 87};
	static const int16 array2[] = {97, 97, 98, 98, 97, 96, 94, 91, 88, 85, 84, 82, 81, 78, 76, 76, 75, 74, 72, 72, 71, 69, 68, 68, 67, 67, 66, 68};
	static const int16 array3[] = {80, 80, 69, 37, 0, 0, 0, 0};
	static const int16 array4[] = {67, 67, 64, 54, 41, 27, 27, 27};

	GxlArchive *oa2Gxl = new GxlArchive("oa2");
	_vm->loadPalette(oa2Gxl, "paramnt.pcx");

	_vm->_musicIndex = 0;
	_vm->changeMusic();

	// "Paramount" background
	_vm->drawImageToScreen(oa2Gxl, "paramnt.pcx", 0, 0);
	_vm->waitSeconds(1);
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	WWSurface *paramaSprite = new WWSurface(190, 112);
	// "And" animation
	_vm->drawImageToSurface(oa2Gxl, "parama.pcx", paramaSprite, 0, 0);
	_vm->drawRandomEffect(paramaSprite, 66, 30, 2, 2);
	delete paramaSprite;

	_vm->waitSeconds(2);
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	_vm->paletteFadeOut(0, 256, 6);

	_vm->loadPalette(oa2Gxl, "backg.pcx");
	_vm->drawImageToScreen(oa2Gxl, "backg.pcx", 0, 0);

	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	// "Capstone logo" animation
	for (int i = 1; i < 29; ++i) {
		Common::String filename = Common::String::format("cap%02d.pcx", i);
		_vm->drawImageToScreen(oa2Gxl, filename.c_str(), array1[i - 1], array2[i - 1]);
		_vm->waitMillis(30);
	}
	_vm->waitSeconds(1);
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	// "The pinnacle of entertainment software" animation
	for (int i = 1; i < 10; ++i) {
		Common::String filename = Common::String::format("txt%02d.pcx", i);
		_vm->drawImageToScreen(oa2Gxl, filename.c_str(), 0, 138);
		_vm->waitMillis(30);
	}
	_vm->waitSeconds(1);
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	// Shining "Capstone" animation
	for (int i = 1; i < 12; ++i) {
		Common::String filename = Common::String::format("captxt%02d.pcx", i);
		_vm->drawImageToScreen(oa2Gxl, filename.c_str(), 113, 89);
		_vm->waitMillis(30);
	}
	_vm->waitSeconds(1);
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	// Removing "The pinnacle of entertainment software" animation
	for (int i = 9; i > 0; --i) {
		Common::String filename = Common::String::format("txt%02d.pcx", i);
		_vm->drawImageToScreen(oa2Gxl, filename.c_str(), 0, 138);
		_vm->waitMillis(30);
	}
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	_vm->drawImageToScreen(oa2Gxl, "txtbkg.pcx", 0, 138);
	_vm->waitMillis(30);
	_vm->waitSeconds(1);
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	for (int i = 1; i < 7; ++i) {
		Common::String filename = Common::String::format("capsp%02d.pcx", i);
		_vm->drawImageToScreen(oa2Gxl, filename.c_str(), 76, 66);
		_vm->waitMillis(30);
	}
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	for (int i = 1; i < 12; ++i) {
		Common::String filename = Common::String::format("presnt%02d.pcx", i);
		_vm->drawImageToScreen(oa2Gxl, filename.c_str(), 117, 112);
		_vm->waitMillis(60);
	}
	if (_vm->_escPressed) {
		delete oa2Gxl;
		return false;
	}

	for (int i = 1; i < 9; ++i) {
		Common::String filename = Common::String::format("capawy%02d.pcx", i);
		_vm->drawImageToScreen(oa2Gxl, filename.c_str(), array3[i - 1], array4[i - 1]);
		_vm->waitMillis(30);
	}

	delete oa2Gxl;
	if (_vm->_escPressed) {
		return false;
	}

	return true;
}

bool WWIntro_full::introPt3(bool flag) {
	introPt3_init();

	wwEffect(1, 0, flag);

	if (flag)
		_vm->waitSeconds(1);

	if (_vm->_escPressed) {
		introPt3_clean();
		return false;
	}

	_vm->_sound->playSound("sv42.snd", false);
	wwEffect(1, 1, flag);
	wwEffect(1, 2, flag);
	wwEffect(1, 3, flag);
	if (_vm->_escPressed) {
		introPt3_clean();
		return false;
	}

	for (int i = 0; i < 32; ++i) {
		setColor236(i % 19);
		wwEffect((i % 8) + 1, 4, flag);

		if (_vm->_escPressed) {
			introPt3_clean();
			return false;
		}
	}

	wwEffect(1, 3, flag);
	wwEffect(1, 2, flag);
	wwEffect(1, 1, flag);
	wwEffect(1, 0, flag);
	if (_vm->_escPressed) {
		introPt3_clean();
		return false;
	}

	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(30);

	_vm->_sound->playSound("sv16.snd", false);

	_vm->waitSeconds(1);
	_vm->paletteFadeOut(0, 256, 4);

	introPt3_clean();

	if (_vm->_escPressed) {
		return false;
	}

	return true;
}

void WWIntro_full::introDisplaySign() {
	WWSurface *introPt6Surface[5] = {nullptr};
	WWSurface *signBottomSurface = nullptr;
	WWSurface *scrollSurface = nullptr;

	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);

	if (_vm->_escPressed)
		return;

	GxlArchive *oa3Gxl = new GxlArchive("oa3");
	for (int i = 0; i < 5; ++i) {
		introPt6Surface[i] = new WWSurface(320, 200);
		Common::String filename = Common::String::format("sign%d.pcx", i);
		_vm->drawImageToSurface(oa3Gxl, filename.c_str(), introPt6Surface[i], 0, 0);
	}

	_vm->_sound->playSound("sv14.snd", false);

	for (int i = 0; i < 5; ++i) {
		_vm->_screen->drawSurface(introPt6Surface[i], 0, 0);
		_vm->waitMillis(100);
		if (_vm->_escPressed) {
			break;
		}
	}

	if (!_vm->_escPressed) {
		signBottomSurface = new WWSurface(320, 94);
		_vm->drawImageToSurface(oa3Gxl, "signbot.pcx", signBottomSurface, 0, 0);
	}

	if (!_vm->_escPressed) {
		_vm->_musicIndex = 1;
		_vm->changeMusic();
		_vm->waitSeconds(4);
	}

	if (!_vm->_escPressed) {
		scrollSurface = new WWSurface(320, 200);

		for (int i = 199; i > 106; --i) {
			scrollSurface->copyRectToSurface((Graphics::Surface)*introPt6Surface[4], 0, 0, Common::Rect(0, 200 - i, 319, 200));
			scrollSurface->copyRectToSurface((Graphics::Surface)*signBottomSurface, 0, i, Common::Rect(0, 0, 319, 200 - i));
			_vm->_screen->drawSurface(scrollSurface, 0, 0);
		}
	}

	delete scrollSurface;
	delete oa3Gxl;
	delete signBottomSurface;
	for (int i = 0; i < 5; ++i)
		delete introPt6Surface[i];
}

void WWIntro_full::introPt7() {
	_vm->_midi->stopSong();
	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
}

void WWIntro_full::introPt4_init() {
	_vm->_fontWW = new GFTFont();
	_vm->_fontWW->loadFromFile("ww.gft");

	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(30);

	_vm->_musicIndex = 2; // metal2.xmi
	_vm->changeMusic();

	_introBackg1Image = new WWSurface(320, 170);
	_vm->drawImageToSurface(_oanGxl, "backg1.pcx", _introBackg1Image, 0, 0);
	for (int i = 0; i < 5; ++i) {
		_introWbodyImage[i] = new WWSurface(145, 118);
		Common::String filename = Common::String::format("wbody%d.pcx", i == 0 ? 0 : i + 8);
		_vm->drawImageToSurface(_oanGxl, filename.c_str(), _introWbodyImage[i], 0, 0);
	}
	_introGbodyImage = new WWSurface(160, 149);
	_vm->drawImageToSurface(_oanGxl, "gbody0.pcx", _introGbodyImage, 0, 0);

	for (int i = 0; i < 8; ++i) {
		_introWhead1[i] = new WWSurface(98, 71);
		Common::String filename = Common::String::format("whead1%d.pcx", i);
		_vm->drawImageToSurface(_oanGxl, filename.c_str(), _introWhead1[i], 0, 0);
	}

	for (int i = 0; i < 11; ++i) {
		_introGhead1[i] = new WWSurface(138, 80);
		Common::String filename = Common::String::format("ghead1%d.pcx", i);
		_vm->drawImageToSurface(_oanGxl, filename.c_str(), _introGhead1[i], 0, 0);
	}

	_vm->drawImageToScreen(_oanGxl, "backg1.pcx", 0, 15);
	_vm->paletteFadeIn(0, 256, 2);
}

bool WWIntro_full::introPt4_intro() {
	_vm->_sound->playSound("sv27.snd", false);

	for (int i = 0; i < 12; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}

	if (_vm->_escPressed) {
		return false;
	}
	++_startOawPos;

	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(30);

	_vm->_sound->playSound("sv25.snd", false);

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	if (_vm->_escPressed) {
		return false;
	}

	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(30);

	_vm->_sound->playSound("sv20.snd", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
	}
	++_startOagPos;

	if (_vm->_escPressed) {
		return false;
	}

	for (int i = 0; i < 15; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	if (_vm->_escPressed) {
		return false;
	}

	return true;
}

bool WWIntro_full::introPt4_caller1() {
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 15; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 5; ++j) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOagPos;

		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	_vm->_sound->playSound("sv33.snd", false);
	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}

	_vm->_sound->playSound("sv38.snd", false);

	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}

	for (int j = 0; j < 15; ++j) {
		sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOagPos;
	_vm->_sound->playSound("sv31.snd", false);

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}
	_vm->_sound->playSound("sv28.snd", false);

	for (int j = 0; j < 5; ++j) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;
	_vm->_sound->playSound("sv21.snd", false);

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 15; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}
	_vm->_sound->playSound("sv29.snd", false);

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	if (_vm->_escPressed) {
		return false;
	}
	return true;
}

bool WWIntro_full::introPt4_caller2() {
	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
		if (i == 2)
			_vm->_sound->playSound("sv19.snd", false);
	}

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOagPos;
	}

	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}

	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
	_vm->waitSeconds(2);
	if (_vm->_escPressed) {
		return false;
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOagPos;
	}
	_vm->_sound->playSound("sv21.snd", false);

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}
	_vm->_sound->playSound("sv15.snd", false);

	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}
	_vm->_sound->playSound("sv30.snd", false);

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOagPos;
	}
	_vm->_sound->playSound("sv22.snd", false);

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	if (_vm->_escPressed) {
		return false;
	}
	return true;
}

bool WWIntro_full::introPt4_caller3() {
	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(3);
		if (_vm->_escPressed) {
			return false;
		}
	}

	for (int i = 0; i < 10; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;
	++_startOagPos;
	_vm->_sound->playSound("sv46.snd", true);

	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	_vm->_sound->playSound("sv31.snd", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 0);
		if (_vm->_escPressed) {
			return false;
		}
	}

	++_startOawPos;
	++_startOagPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}

	for (int j = 0; j < 4; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}
	_vm->_sound->playSound("sv39.snd", false);

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;
	++_startOagPos;

	return true;
}

bool WWIntro_full::introPt4_caller4() {
	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
	_vm->waitSeconds(2);
	if (_vm->_escPressed) {
		return false;
	}

	for (int i = 0; i < 15; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOagPos;
	}

	_vm->_sound->playSound("sv37.snd", true);
	_vm->_sound->playSound("sv24.snd", true);
	
	for (int i = 0; i < 15; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
	_vm->waitSeconds(2);
	if (_vm->_escPressed) {
		return false;
	}

	_vm->_sound->playSound("sv06.snd", false);

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}
	_vm->_sound->playSound("sv32.snd", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOagPos;

	sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
	_vm->waitSeconds(1);
	if (_vm->_escPressed) {
		return false;
	}

	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 8; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}

	_vm->_sound->playSound("sv18.snd", false);

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOawPos;

	for (int i = 0; i < 6; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
		if (_vm->_escPressed) {
			return false;
		}
	}

	_vm->_sound->playSound("sv45.snd", true);

	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 8; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
			if (_vm->_escPressed) {
				return false;
			}
		}
		++_startOawPos;
	}

	_vm->_sound->playSound("sv34.snd", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
		if (_vm->_escPressed) {
			return false;
		}
	}
	++_startOagPos;

	return true;
}

} // End of namespace WaynesWorld
