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

namespace WaynesWorld {

WWIntro_demo1::WWIntro_demo1(WaynesWorldEngine *vm) : WWIntro(vm) {
}

WWIntro_demo1::~WWIntro_demo1() {
}

void WWIntro_demo1::runIntro() {
	// continueFl is used like in the full version, but for the moment it's not possible to skip (demo is not interactive)
	bool continueFl = initOanGxl();

	if (continueFl) {
		Common::File f;

		if (f.exists("capspin.gxl"))
			continueFl = introPt1();
		else
			continueFl = introPt1_selectware();
	}
	if (continueFl)
		continueFl = introPt3();
	if (continueFl)
		continueFl = introPt4();
	if (continueFl)
		continueFl = introPt3Bis();

	cleanOanGxl();

	if (continueFl)
		continueFl = introDisplaySign();
	if (continueFl)
		continueFl = introPreviewRoom00();
	if (continueFl)
		continueFl = introMapStonebridge();
	if (continueFl)
		continueFl = introPreviewRoom08and22();
	if (continueFl)
		continueFl = introMapButterfield();
	if (continueFl)
		continueFl = introPreviewRoom07and15and16();
	if (continueFl)
		continueFl = introMapDowntown();
	if (continueFl)
		continueFl = introPreviewRoom10();
	if (continueFl)
		continueFl = introMapNorthAurora();
	if (continueFl)
		continueFl = introPreviewRoom03and23();
	if (continueFl)
		continueFl = introMapSouthEastArea();
	if (continueFl)
		continueFl = introPreviewRoom13and18();
	if (continueFl)
		continueFl = introMapWestAurora();

	if (continueFl)
		continueFl = introPreviewRoom01();

	if (continueFl) {
		_oanGxl = new GxlArchive("oan");
		_vm->loadPalette(_oanGxl, "backg2.pcx");
		while (_vm->_sound->isSFXPlaying())
			_vm->waitMillis(30);
		_vm->stopMusic();

		introPt3();

		_vm->paletteFadeOut(0, 256, 4);
		_vm->_screen->clear(0);
		while (_vm->_sound->isSFXPlaying())
			_vm->waitMillis(30);

		delete _oanGxl;
	}
}

bool WWIntro_demo1::introPt1() {
	GxlArchive *capspinGxl = new GxlArchive("capspin");
	_vm->loadPalette(capspinGxl, "paramnt.pcx");
	_vm->paletteFadeOut(0, 256, 64);
	_vm->drawImageToScreen(capspinGxl, "paramnt.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 3);
	_vm->waitSeconds(1);

	WWSurface *paramaSprite = new WWSurface(190, 112);
	// "And" animation
	_vm->drawImageToSurface(capspinGxl, "parama.pcx", paramaSprite, 0, 0);
	_vm->drawRandomEffect(paramaSprite, 66, 30, 2, 2);
	delete paramaSprite;
	_vm->waitSeconds(2);

	_vm->paletteFadeOut(0, 256, 6);
	_vm->_screen->clear(0);
	_vm->loadPalette(capspinGxl, "backg.pcx");
	_vm->paletteFadeOut(0, 256, 64);
	_vm->drawImageToScreen(capspinGxl, "backg.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 6);

	// The original has all the frames hardcoded one after the other, I used a loop instead.
	Frame animation[] = {
		{"cap01.pcx", 179, 97, 30},
		{"cap02.pcx", 179, 97, 30},
		{"cap03.pcx", 181, 98, 30},
		{"cap04.pcx", 183, 98, 30},
		{"cap05.pcx", 186, 97, 30},
		{"cap06.pcx", 189, 96, 30},
		{"cap07.pcx", 192, 94, 30},
		{"cap08.pcx", 198, 91, 30},
		{"cap09.pcx", 208, 88, 30},
		{"cap10.pcx", 212, 85, 30},
		{"cap11.pcx", 217, 84, 30},
		{"cap12.pcx", 223, 82, 30},
		{"cap13.pcx", 234, 81, 30},
		{"cap14.pcx", 233, 78, 30},
		{"cap15.pcx", 236, 76, 30},
		{"cap16.pcx", 240, 76, 30},
		{"cap17.pcx", 241, 75, 30},
		{"cap18.pcx", 238, 74, 30},
		{"cap19.pcx", 238, 72, 30},
		{"cap20.pcx", 238, 72, 30},
		{"cap21.pcx", 225, 71, 30},
		{"cap22.pcx", 218, 69, 30},
		{"cap23.pcx", 218, 68, 30},
		{"cap24.pcx", 192, 68, 30},
		{"cap25.pcx", 164, 67, 30},
		{"cap26.pcx", 133, 67, 30},
		{"cap27.pcx", 103, 66, 30},
		{"cap28.pcx", 87, 68, 1030},
		{"txt01.pcx", 0, 138, 30},
		{"txt02.pcx", 0, 138, 30},
		{"txt03.pcx", 0, 138, 30},
		{"txt04.pcx", 0, 138, 30},
		{"txt05.pcx", 0, 138, 30},
		{"txt06.pcx", 0, 138, 30},
		{"txt07.pcx", 0, 138, 30},
		{"txt08.pcx", 0, 138, 30},
		{"txt09.pcx", 0, 138, 1030},
		{"captxt01.pcx", 113, 89, 30},
		{"captxt02.pcx", 113, 89, 30},
		{"captxt03.pcx", 113, 89, 30},
		{"captxt04.pcx", 113, 89, 30},
		{"captxt05.pcx", 113, 89, 30},
		{"captxt06.pcx", 113, 89, 30},
		{"captxt07.pcx", 113, 89, 30},
		{"captxt08.pcx", 113, 89, 30},
		{"captxt09.pcx", 113, 89, 30},
		{"captxt10.pcx", 113, 89, 30},
		{"captxt11.pcx", 113, 89, 1030},
		{"txt09.pcx", 0, 138, 30},
		{"txt08.pcx", 0, 138, 30},
		{"txt07.pcx", 0, 138, 30},
		{"txt06.pcx", 0, 138, 30},
		{"txt05.pcx", 0, 138, 30},
		{"txt04.pcx", 0, 138, 30},
		{"txt03.pcx", 0, 138, 30},
		{"txt02.pcx", 0, 138, 30},
		{"txt01.pcx", 0, 138, 30},
		{"txtbkg.pcx", 0, 138, 1030},
		{"capawy01.pcx", 80, 67, 30},
		{"capawy02.pcx", 80, 67, 30},
		{"capawy03.pcx", 69, 64, 30},
		{"capawy04.pcx", 37, 54, 30},
		{"capawy05.pcx", 0, 41, 30},
		{"capawy06.pcx", 0, 27, 30},
		{"capawy07.pcx", 0, 27, 30},
		{"capawy08.pcx", 0, 27, 1030}
	};

	for (const Frame &frame : animation) {
		_vm->drawImageToScreen(capspinGxl, frame.filename, frame.x, frame.y);
		_vm->waitMillis(frame.delay);
	}

	delete capspinGxl;
	return true;
}

bool WWIntro_demo1::introPt1_selectware() {
	GxlArchive *capGxl = new GxlArchive("cap");
	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	_vm->loadPalette(capGxl, "paramnt.pcx");
	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(capGxl, "paramnt.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 3);
	_vm->waitSeconds(1);

	_vm->paletteFadeOut(0, 256, 3);
	_vm->_screen->clear(0);
	_vm->loadPalette(capGxl, "pyramid.pcx");
	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(capGxl, "pyramid.pcx", 58, 21);
	_vm->paletteFadeIn(0, 256, 3);

	// The original has all the frames hardcoded one after the other, I used a loop instead.
	Frame animation[] = {
		{"prestxt1.pcx", 40, 125, 60},
		{"prestxt2.pcx", 40, 125, 60},
		{"prestxt3.pcx", 40, 125, 60},
		{"prestxt4.pcx", 40, 125, 60},
		{"prestxt5.pcx", 40, 125, 60},
		{"prestxt6.pcx", 40, 125, 60},
		{"present1.pcx", 115, 156, 60},
		{"present2.pcx", 115, 156, 60},
		{"present3.pcx", 115, 156, 60},
		{"present4.pcx", 115, 156, 60},
		{"present5.pcx", 115, 156, 60},
		{"present6.pcx", 115, 156, 60},
		{"star1.pcx", 146, 17, 150},
		{"star2.pcx", 146, 17, 150},
		{"star3.pcx", 146, 17, 150},
		{"star4.pcx", 146, 17, 150},
		{"pyramid.pcx", 58, 21, 4250}
	};

	for (const Frame &frame : animation) {
		_vm->drawImageToScreen(capGxl, frame.filename, frame.x, frame.y);
		_vm->waitMillis(frame.delay);
	}

	delete capGxl;
	return true;
}

bool WWIntro_demo1::introPt3() {
	// sub1 - Parameter is always 'true' so it has been removed and the code simplified
	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	_vm->loadPalette(_oanGxl, "backg1.pcx");
	_vm->paletteFadeOut(0, 256, 64);
	
	introPt3_init();
	// End of sub1

	_demoPt2Surface->clear(0);
	_demoPt2Surface->drawSurface(_logoSurface, 47, 25);
	wwEffect(1, 0);
	_vm->paletteFadeIn(0, 256, 6);

	_vm->playSound("theme1.snd", 0);

	wwEffect(1, 1);
	wwEffect(1, 2);
	wwEffect(1, 3);

	for (int i = 0; i < 24; ++i) {
		setColor236(i % 19);
		wwEffect((i % 8) + 1, 4);
	}

	wwEffect(1, 3);
	wwEffect(1, 2);
	wwEffect(1, 1);
	wwEffect(1, 0);

	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(30);

	_vm->_sound->playSound("exclnt.snd", false);

	introPt3_clean();

	return true;
}

bool WWIntro_demo1::introPt3Bis() {
	// sub1 - Parameter is always 'true' so it has been removed and the code simplified
	introPt3_init();
	// End of sub1
	_vm->stopMusic();
	_vm->playSound("theme1.snd", false);

	wwEffect(1, 0);
	wwEffect(1, 1);
	wwEffect(1, 2);
	wwEffect(1, 3);

	for (int i = 0; i < 20; ++i) {
		setColor236(i % 19);
		wwEffect((i % 8) + 1, 4);
	}

	introPt3_clean();
	_vm->waitSeconds(9);

	return true;
}

void WWIntro_demo1::introPt4_init() {
	_vm->_fontWW = new GFTFont();
	_vm->_fontWW->loadFromFile("ww.gft");

	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(30);

	_introBackg1Image = new WWSurface(320, 170);
	_vm->drawImageToSurface(_oanGxl, "backg1.pcx", _introBackg1Image, 0, 0);

	for (int i = 0; i < 5; ++i) {
		_introWbodyImage[i] = new WWSurface(145, 118);
		Common::String filename = Common::String::format("wbody%d.pcx", i == 0 ? 0 : i + 8);
		_vm->drawImageToSurface(_oanGxl, filename.c_str(), _introWbodyImage[i], 0, 0);
	}

	_introGbodyImage = new WWSurface(160, 149);
	_vm->drawImageToSurface(_oanGxl, "gbody0.pcx", _introGbodyImage, 0, 0);

	// The original is overwriting the song name (default metal1.xmi) instead of setting the musicIndex.
	_vm->_musicIndex = 2; // metal2.xmi
	_vm->changeMusic();

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

bool WWIntro_demo1::introPt4_intro() {
	_vm->_sound->playSound("ok.abt", false);
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 20; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}

		++_startOawPos;
	}

	return true;
}

bool WWIntro_demo1::introPt4_caller1() {
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 15; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 5; ++j) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
		}
		++_startOagPos;

		for (int j = 0; j < 5; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}
	_vm->playSound("ok.abt", false);

	for (int i = 0; i < 10; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	_vm->playSound("shya!2.abt", false);

	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	for (int j = 0; j < 15; ++j) {
		sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
	}
	++_startOagPos;
	
	_vm->playSound("not!.abt", true);
	
	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 15; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}
	_vm->_sound->playSound("hello.abt", true);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	return true;
}

bool WWIntro_demo1::introPt4_caller2() {
	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}
	_vm->playSound("flash-bk.abt", true);

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
		}
		++_startOagPos;
	}

	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	for (int j = 0; j < 10; ++j) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
	_vm->waitSeconds(2);

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
		}
		++_startOagPos;
	}

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	for (int i = 0; i < 10; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	for (int i = 0; i < 4; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}
	_vm->_sound->playSound("no-way.abt", false);

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
		}
		++_startOagPos;
	}
	_vm->_sound->playSound("goodc-g.abt", false);

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 10; ++j) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}

	return true;
}

bool WWIntro_demo1::introPt4_caller3() {
	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	for (int i = 0; i < 10; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 0);
	}
	++_startOawPos;
	++_startOagPos;
	_vm->_sound->playSound("worthy.snd", true);

	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}

	_vm->_sound->playSound("not!.abt", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 0);
	}
	++_startOawPos;
	++_startOagPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	for (int j = 0; j < 4; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}
	_vm->_sound->playSound("as-if.abt", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 0);
	}
	++_startOawPos;
	++_startOagPos;

	return true;
}

bool WWIntro_demo1::introPt4_caller4() {
	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}

	sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
	_vm->waitSeconds(2);

	for (int i = 0; i < 15; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	for (int j = 0; j < 2; ++j) {
		for (int i = 0; i < 15; ++i) {
			sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
		}
		++_startOagPos;
	}

	_vm->_sound->playSound("ilovetw.abt", true);

	for (int i = 0; i < 15; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
	_vm->waitSeconds(2);
	_vm->_sound->playSound("cool!.abt", false);

	for (int i = 0; i < 3; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	for (int i = 0; i < 2; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}
	_vm->_sound->playSound("nway-way.abt", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
	}
	++_startOagPos;

	sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
	_vm->waitSeconds(1);

	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 8; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}
	_vm->_sound->playSound("exskweez.abt", false);

	for (int i = 0; i < 8; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
	}
	++_startOawPos;

	for (int i = 0; i < 6; ++i) {
		sub2FEFB(1, 0, 1, _vm->getRandom(3), _vm->getRandom(11), 2);
		_vm->waitSeconds(2);
	}

	_vm->_sound->playSound("donut.snd", true);

	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 8; ++i) {
			sub2FEFB(1, 0, 1, _vm->getRandom(3), 9, 0);
		}
		++_startOawPos;
	}

	_vm->_sound->playSound("partyon.abt", false);

	for (int i = 0; i < 5; ++i) {
		sub2FEFB(1, 0, 1, 0, _vm->getRandom(11), 1);
	}
	++_startOagPos;

	return true;
}

bool WWIntro_demo1::introDisplaySign() {
	WWSurface *introPt6Surface[5] = {nullptr};

	GxlArchive *signGxl = new GxlArchive("sign");
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);

	for (int i = 0; i < 5; ++i) {
		introPt6Surface[i] = new WWSurface(320, 200);
		Common::String filename = Common::String::format("sign%d.pcx", i);
		_vm->drawImageToSurface(signGxl, filename.c_str(), introPt6Surface[i], 0, 0);
	}

	_vm->_sound->playSound("ex-clsp2.snd", false);

	for (int i = 0; i < 5; ++i) {
		_vm->_screen->drawSurface(introPt6Surface[i], 0, 0);
		_vm->waitMillis(150);
	}

	WWSurface *signBottomSurface = new WWSurface(320, 94);
	_vm->drawImageToSurface(signGxl, "signbot.pcx", signBottomSurface, 0, 0);

	_vm->waitSeconds(4);
	_vm->_musicIndex = 1;
	_vm->changeMusic();

	WWSurface *scrollSurface = new WWSurface(320, 200);

	for (int i = 199; i > 106; --i) {
		scrollSurface->copyRectToSurface((Graphics::Surface)*introPt6Surface[4], 0, 0, Common::Rect(0, 200 - i, 319, 200));
		scrollSurface->copyRectToSurface((Graphics::Surface)*signBottomSurface, 0, i, Common::Rect(0, 0, 319, 200 - i));
		_vm->_screen->drawSurface(scrollSurface, 0, 0);
	}

	delete scrollSurface;
	delete signBottomSurface;
	for (int i = 0; i < 5; ++i)
		delete introPt6Surface[i];

	delete signGxl;

	return true;
}

bool WWIntro_demo1::introPreviewRoom00() {
	GxlArchive *r00Gxl = new GxlArchive("r00");
	GxlArchive *m00Gxl = new GxlArchive("m00");

	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	_vm->loadPalette(r00Gxl, "backg.pcx");
	_vm->paletteFadeOut(0, 256, 64);
	_vm->_musicIndex = 0;
	_vm->changeMusic();
	_vm->drawImageToScreen(r00Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(m00Gxl, "ginter.pcx", 0, 151);
	_vm->paletteFadeIn(0, 256, 3);
	_vm->waitSeconds(3);

	delete m00Gxl;
	delete r00Gxl;

	return true;
}

bool WWIntro_demo1::introMapStonebridge() {
	GxlArchive *m02Gxl = new GxlArchive("m02");

	WWSurface *zmPcx[12] = {nullptr};
	for (int i = 0; i < 12; ++i) {
		zmPcx[i] = new WWSurface(137, 109);
		Common::String filename = Common::String::format("stn_zm%d.pcx", i);
		_vm->drawImageToSurface(m02Gxl, filename.c_str(), zmPcx[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(m02Gxl, "main_map.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 4);
	_vm->drawImageToScreen(m02Gxl, "stn_tag.pcx", 173, 90);
	_vm->playSound("flash-bk.abt", false);
	_vm->waitSeconds(1);

	for (int i = 0; i < 12; ++i) {
		_vm->_screen->drawSurface(zmPcx[i], 173, 13);
		_vm->waitMillis(75);
	}

	_vm->drawImageToScreen(m02Gxl, "may_tag.pcx", 193, 40);
	_vm->drawImageToScreen(m02Gxl, "eug_tag.pcx", 241, 82);
	_vm->waitSeconds(4);

	for (int i = 0; i < 12; ++i)
		delete zmPcx[i];

	delete m02Gxl;

	return true;
}

bool WWIntro_demo1::introPreviewRoom08and22() {
	GxlArchive *r08Gxl = new GxlArchive("r08");
	GxlArchive *m00Gxl = new GxlArchive("m00");
	GxlArchive *r22Gxl = new GxlArchive("r22");

	WWSurface *r22Back = new WWSurface(320, 150);
	_vm->drawImageToSurface(r22Gxl, "backg.pcx", r22Back, 0, 0);

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(r08Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(m00Gxl, "ginter.pcx", 0, 151);
	_vm->paletteFadeIn(0, 256, 3);
	_vm->waitSeconds(3);

	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);

	_vm->playSound("zeetsa.abt", false);
	_vm->drawSpiralEffect(r22Back, 0, 0, 5, 5);
	_vm->waitSeconds(3);

	delete r22Back;

	delete r22Gxl;
	delete m00Gxl;
	delete r08Gxl;

	return true;
}

bool WWIntro_demo1::introMapButterfield() {
	GxlArchive *m02Gxl = new GxlArchive("m02");

	WWSurface *zmPcx[12] = {nullptr};
	for (int i = 0; i < 12; ++i) {
		zmPcx[i] = new WWSurface(171, 109);
		Common::String filename = Common::String::format("but_zm%d.pcx", i);
		_vm->drawImageToSurface(m02Gxl, filename.c_str(), zmPcx[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(m02Gxl, "main_map.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 4);
	_vm->drawImageToScreen(m02Gxl, "but_tag.pcx", 247, 38);
	_vm->playSound("flash-bk.abt", false);
	_vm->waitSeconds(1);

	for (int i = 0; i < 12; ++i) {
		_vm->_screen->drawSurface(zmPcx[i], 127, 11);
		_vm->waitMillis(75);
	}

	_vm->drawImageToScreen(m02Gxl, "cin_tag.pcx", 147, 27);
	_vm->drawImageToScreen(m02Gxl, "c35_tag.pcx", 208, 72);
	_vm->waitSeconds(4);

	for (int i = 0; i < 12; ++i)
		delete zmPcx[i];

	delete m02Gxl;

	return true;
}

bool WWIntro_demo1::introPreviewRoom07and15and16() {
	GxlArchive *r07Gxl = new GxlArchive("r07");
	GxlArchive *m00Gxl = new GxlArchive("m00");
	GxlArchive *r15Gxl = new GxlArchive("r15");
	GxlArchive *r16Gxl = new GxlArchive("r16");

	WWSurface *r15Back = new WWSurface(320, 150);
	_vm->drawImageToSurface(r15Gxl, "backg.pcx", r15Back, 0, 0);

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(r07Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(m00Gxl, "ginter.pcx", 0, 151);
	_vm->paletteFadeIn(0, 256, 3);
	_vm->waitSeconds(3);

	_vm->drawSpiralEffect(r15Back, 0, 0, 5, 5);
	_vm->waitSeconds(3);
	delete r15Back;

	WWSurface *r16Back = new WWSurface(320, 150);
	_vm->drawImageToSurface(r16Gxl, "backg.pcx", r16Back, 0, 0);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);
	_vm->_sound->playSound("zang!.abt", false);
	_vm->waitSeconds(3);
	_vm->drawSpiralEffect(r16Back, 0, 0, 5, 5);
	_vm->waitSeconds(3);
	delete r16Back;

	delete r16Gxl;
	delete r15Gxl;
	delete m00Gxl;
	delete r07Gxl;

	return true;
}

bool WWIntro_demo1::introMapDowntown() {
	GxlArchive *m02Gxl = new GxlArchive("m02");

	WWSurface *zmPcx[12] = {nullptr};
	for (int i = 0; i < 12; ++i) {
		zmPcx[i] = new WWSurface(137, 110);
		Common::String filename = Common::String::format("dt_zm%d.pcx", i);
		_vm->drawImageToSurface(m02Gxl, filename.c_str(), zmPcx[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(m02Gxl, "main_map.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 4);
	_vm->drawImageToScreen(m02Gxl, "dt_tag.pcx", 125, 137);
	_vm->waitSeconds(1);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);
	_vm->playSound("flash-bk.abt", false);

	for (int i = 3; i < 12; ++i) {
		// The original starts the loop at 3. It looks like a typo, but it's on purpose: there's a visible glitch on the left on the first 3 frames.
		_vm->_screen->drawSurface(zmPcx[i], 70, 69);
		_vm->waitMillis(75);
	}

	_vm->drawImageToScreen(m02Gxl, "jun_tag.pcx", 78, 87);
	_vm->drawImageToScreen(m02Gxl, "cih_tag.pcx", 134, 102);
	_vm->drawImageToScreen(m02Gxl, "uno_tag.pcx", 142, 137);
	_vm->waitSeconds(4);

	for (int i = 0; i < 12; ++i)
		delete zmPcx[i];

	delete m02Gxl;

	return true;
}

bool WWIntro_demo1::introPreviewRoom10() {
	GxlArchive *r10Gxl = new GxlArchive("r10");
	GxlArchive *m00Gxl = new GxlArchive("m00");

	WWSurface *r10Win = new WWSurface(141, 92);
	_vm->drawImageToSurface(r10Gxl, "win.pcx", r10Win, 0, 0);
	WWSurface *r10Getmon[5] = {nullptr};
	for (int i = 0; i < 5; ++i) {
		r10Getmon[i] = new WWSurface(84, 100);
		Common::String filename = Common::String::format("getmon%d.pcx", i);
		_vm->drawImageToSurface(r10Gxl, filename.c_str(), r10Getmon[i], 0, 0);
	}

	WWSurface *r10Puttick[4] = {nullptr};
	for (int i = 0; i < 4; ++i) {
		r10Puttick[i] = new WWSurface(84, 100);
		Common::String filename = Common::String::format("puttick%d.pcx", i);
		_vm->drawImageToSurface(r10Gxl, filename.c_str(), r10Puttick[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(r10Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(m00Gxl, "ginter.pcx", 0, 151);
	_vm->drawImageToScreen(r10Gxl, "money.pcx", 136, 100);
	_vm->paletteFadeIn(0, 256, 3);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);
	_vm->playSound("ilovetw.abt", false);
	_vm->waitSeconds(1);

	for (int i = 0; i < 5; ++i) {
		_vm->_screen->drawSurface(r10Getmon[i], 98, 7);
		_vm->waitMillis(75);
	}
	
	for (int i = 0; i < 4; ++i) {
		_vm->_screen->drawSurface(r10Puttick[i], 98, 7);
		_vm->waitMillis(75);
	}

	_vm->drawImageToScreen(r10Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(r10Gxl, "cticket.pcx", 147, 101);

	for (int i = 0; i < 4; ++i)
		delete r10Puttick[i];
	for (int i = 0; i < 5; ++i)
		delete r10Getmon[i];

	_vm->waitSeconds(2);
	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);
	_vm->playSound("ex-clsp2.snd", false);
	_vm->drawImageToScreen(r10Gxl, "ticket.pcx", 0, 13);
	_vm->paletteFadeIn(0, 256, 64);
	_vm->waitSeconds(2);

	_vm->drawRandomEffect(r10Win, 157, 38, 1, 1);

	delete r10Win;

	delete m00Gxl;
	delete r10Gxl;

	return true;
}

bool WWIntro_demo1::introMapNorthAurora() {
	GxlArchive *m02Gxl = new GxlArchive("m02");

	WWSurface *zmPcx[12] = {nullptr};
	for (int i = 0; i < 12; ++i) {
		zmPcx[i] = new WWSurface(137, 109);
		Common::String filename = Common::String::format("na_zm%d.pcx", i);
		_vm->drawImageToSurface(m02Gxl, filename.c_str(), zmPcx[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(m02Gxl, "main_map.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 4);
	_vm->drawImageToScreen(m02Gxl, "na_tag.pcx", 84, 49);
	_vm->waitSeconds(1);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);
	_vm->playSound("flash-bk.abt", false);

	for (int i = 1; i < 12; ++i) {
		_vm->_screen->drawSurface(zmPcx[i], 53, 6);
		_vm->waitMillis(75);
	}

	_vm->drawImageToScreen(m02Gxl, "cab_tag.pcx", 61, 18);
	_vm->drawImageToScreen(m02Gxl, "ad_tag.pcx", 141, 31);
	_vm->drawImageToScreen(m02Gxl, "cas_tag.pcx", 69, 80);
	_vm->waitSeconds(4);

	for (int i = 0; i < 12; ++i)
		delete zmPcx[i];

	delete m02Gxl;

	return true;
}

bool WWIntro_demo1::introPreviewRoom03and23() {
	GxlArchive *r03Gxl = new GxlArchive("r03");
	GxlArchive *m00Gxl = new GxlArchive("m00");
	GxlArchive *r23Gxl = new GxlArchive("r23");

	WWSurface *r23Back = new WWSurface(320, 150);
	_vm->drawImageToSurface(r23Gxl, "backg.pcx", r23Back, 0, 0);

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(r03Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(m00Gxl, "ginter.pcx", 0, 151);
	_vm->paletteFadeIn(0, 256, 3);
	_vm->waitSeconds(3);

	_vm->drawSpiralEffect(r23Back, 0, 0, 5, 5);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);

	_vm->playSound("schwing.abt", false);
	_vm->waitSeconds(3);

	delete r23Back;

	delete r23Gxl;
	delete m00Gxl;
	delete r03Gxl;

	return true;
}

bool WWIntro_demo1::introMapSouthEastArea() {
	GxlArchive *m02Gxl = new GxlArchive("m02");

	WWSurface *zmPcx[12] = {nullptr};
	for (int i = 0; i < 12; ++i) {
		zmPcx[i] = new WWSurface(137, 109);
		Common::String filename = Common::String::format("sea_zm%d.pcx", i);
		_vm->drawImageToSurface(m02Gxl, filename.c_str(), zmPcx[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(m02Gxl, "main_map.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 4);
	_vm->drawImageToScreen(m02Gxl, "sea_tag.pcx", 218, 142);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);
	_vm->playSound("flash-bk.abt", false);
	_vm->waitSeconds(1);

	for (int i = 1; i < 12; ++i) {
		_vm->_screen->drawSurface(zmPcx[i], 139, 57);
		_vm->waitMillis(75);
	}

	_vm->drawImageToScreen(m02Gxl, "pep_tag.pcx", 151, 101);
	_vm->drawImageToScreen(m02Gxl, "gil_tag.pcx", 224, 106);
	_vm->waitSeconds(4);

	for (int i = 0; i < 12; ++i)
		delete zmPcx[i];

	delete m02Gxl;

	return true;
}

bool WWIntro_demo1::introPreviewRoom13and18() {
	GxlArchive *r13Gxl = new GxlArchive("r13");
	GxlArchive *m00Gxl = new GxlArchive("m00");
	GxlArchive *r18Gxl = new GxlArchive("r18");

	WWSurface *machinePcx[9] = {nullptr};
	for (int i = 0; i < 9; ++i) {
		machinePcx[i] = new WWSurface(171, 68);
		Common::String filename = Common::String::format("machine%d.pcx", i);
		_vm->drawImageToSurface(r18Gxl, filename.c_str(), machinePcx[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(r13Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(m00Gxl, "ginter.pcx", 0, 151);
	_vm->paletteFadeIn(0, 256, 3);
	_vm->waitSeconds(3);

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);

	_vm->drawImageToScreen(r18Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(m00Gxl, "ginter.pcx", 0, 151);
	_vm->paletteFadeIn(0, 256, 3);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);

	_vm->playSound("cool!.abt", false);
	for (int i = 0; i < 12; ++i) {
		for (int j = 0; j < 9; ++j) {
			_vm->_screen->drawSurface(machinePcx[j], 115, 51);
			_vm->waitMillis(75);
		}
	}

	for (int i = 0; i < 9; ++i)
		delete machinePcx[i];

	delete r18Gxl;
	delete m00Gxl;
	delete r13Gxl;

	return true;
}

bool WWIntro_demo1::introMapWestAurora() {
	GxlArchive *m02Gxl = new GxlArchive("m02");

	WWSurface *zmPcx[12] = {nullptr};
	for (int i = 0; i < 12; ++i) {
		zmPcx[i] = new WWSurface(137, 109);
		Common::String filename = Common::String::format("wa_zm%d.pcx", i);
		_vm->drawImageToSurface(m02Gxl, filename.c_str(), zmPcx[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(m02Gxl, "main_map.pcx", 0, 0);
	_vm->paletteFadeIn(0, 256, 4);
	_vm->drawImageToScreen(m02Gxl, "wa_tag.pcx", 56, 87);
	_vm->waitSeconds(1);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);
	_vm->playSound("flash-bk.abt", false);

	for (int i = 1; i < 12; ++i) {
		_vm->_screen->drawSurface(zmPcx[i], 34, 32);
		_vm->waitMillis(75);
	}

	_vm->drawImageToScreen(m02Gxl, "inv_tag.pcx", 49, 52);
	_vm->drawImageToScreen(m02Gxl, "don_tag.pcx", 120, 47);
	_vm->drawImageToScreen(m02Gxl, "way_tag.pcx", 73, 86);
	_vm->waitSeconds(4);

	for (int i = 0; i < 12; ++i)
		delete zmPcx[i];

	delete m02Gxl;

	return true;
}

bool WWIntro_demo1::introPreviewRoom01() {
	GxlArchive *r01Gxl = new GxlArchive("r01");
	GxlArchive *m00Gxl = new GxlArchive("m00");

	WWSurface *r01Term[10] = {nullptr};
	for (int i = 0; i < 10; ++i) { // The demo has a bug, it stops the initialization at 8 instead of 9, so the last frame is thrown uninitialized
		r01Term[i] = new WWSurface(42, 25);
		Common::String filename = Common::String::format("term%d.pcx", i);
		_vm->drawImageToSurface(r01Gxl, filename.c_str(), r01Term[i], 0, 0);
	}

	WWSurface *r01Screen[6] = {nullptr};
	for (int i = 0; i < 6; ++i) {
		r01Screen[i] = new WWSurface(41, 49);
		Common::String filename = Common::String::format("screen%d.pcx", i);
		_vm->drawImageToSurface(r01Gxl, filename.c_str(), r01Screen[i], 0, 0);
	}

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(r01Gxl, "backg.pcx", 0, 0);
	_vm->drawImageToScreen(m00Gxl, "ginter.pcx", 0, 151);
	_vm->paletteFadeIn(0, 256, 3);
	while (_vm->_sound->isSFXPlaying())
		_vm->waitMillis(10);
	_vm->playSound("hello.abt", false);

	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 10; ++j) {
			_vm->_screen->drawSurface(r01Term[j], 270, 47);
			_vm->waitMillis(75);
		}
	}

	for (int i = 0; i < 6; ++i) {
		_vm->_screen->drawSurface(r01Screen[i], 118, 53);
		_vm->waitMillis(75);
	}

	for (int i = 0; i < 6; ++i)
		delete r01Screen[i];
	for (int i = 0; i < 9; ++i)
		delete r01Term[i];

	_vm->waitSeconds(3);
	_vm->playSound("goodnite.abt", false);

	_vm->paletteFadeOut(0, 256, 4);
	_vm->_screen->clear(0);

	delete m00Gxl;
	delete r01Gxl;

	return true;
}

} // End of namespace WaynesWorld
