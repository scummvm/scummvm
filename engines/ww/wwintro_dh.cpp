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

#include "ww/graphics.h"
#include "ww/gxlarchive.h"
#include "ww/ww.h"
#include "ww/wwintro.h"

#include "audio/audiostream.h"

namespace WW {

DHIntro::DHIntro(WWEngine *vm) : Intro(vm) {
}

void DHIntro::runIntro() {
	bool continueFl = init();

	if (continueFl)
		continueFl = introPt1();
	if (continueFl)
		continueFl = introPt2();
	if (continueFl)
		continueFl = introPt3();
	if (continueFl)
		continueFl = introPt4();
	if (continueFl)
		continueFl = introPt5();

	while (!_vm->_midi->checkMidiDone())
		_vm->waitMillis(10);

	if (continueFl)
		continueFl = introPt6();
	if (continueFl)
		continueFl = intro_credits();
	if (continueFl)
		continueFl = introPt8();
	if (continueFl)
		introPt9();

	_vm->_midi->stopSong();
}

bool DHIntro::init() {
	GxlArchive *koa00Gxl = new GxlArchive("koa00");

	_vm->loadPalette(koa00Gxl, "credit1.pcx");

	delete koa00Gxl;
	_vm->_midi->stopSong();

	return true;
}

bool DHIntro::introPt1() {
	GxlArchive *koa00Gxl = new GxlArchive("koa00");
	_vm->paletteFadeOut(0, 256, 3);
	_vm->changeMusic("meanwhil.xmi");

	_vm->drawImageToScreen(koa00Gxl, "pyramid.pcx", 58, 21);
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
		{"pyramid.pcx", 58, 21, 250} // TODO : 7250 in the original
	};

	for (const Frame &frame : animation) {
		_vm->drawImageToScreen(koa00Gxl, frame.filename, frame.x, frame.y);
		_vm->waitMillis(frame.delay);
		if (_vm->_escPressed) {
			break;
		}
	}

	_vm->_midi->stopSong();
	_vm->paletteFadeOut(0, 256, 2);

	delete koa00Gxl;
	return !_vm->_escPressed;
}

bool DHIntro::introPt2() {
	int deyeProba = 3;
	int deyeId = 1;
	int neyeProba = 3;
	int neyeId = 1;

	GxlArchive *koa05Gxl = new GxlArchive("koa05");
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(koa05Gxl, "surg.pcx", 0, 15);
	_vm->paletteFadeIn(0, 256, 2);
	_vm->changeMusic("o_r_.xmi");


	for (int loopCtr = 1; loopCtr < 30; ++loopCtr) {
		if (_vm->getRandom(20) <= neyeProba) {
			Common::String filename = Common::String::format("neye%d.pcx", neyeId);
			_vm->drawImageToScreen(koa05Gxl, filename.c_str(), 74, 75);
			neyeProba = neyeId ? 8 : 3;
			neyeId ^= 1;
		}

		if (_vm->getRandom(20) <= deyeProba) {
			Common::String filename = Common::String::format("deye%d.pcx", deyeId);
			_vm->drawImageToScreen(koa05Gxl, filename.c_str(), 205, 65);
			deyeProba = deyeId ? 8 : 3;
			deyeId ^= 1;
		}

		if (loopCtr == 15) {
			for (int i = 1; i < 5; ++i) {
				Common::String filename = Common::String::format("arm%d.pcx", i);
				_vm->drawImageToScreen(koa05Gxl, filename.c_str(), 78, 83);
			}
		}

		_vm->waitMillis(50);
		if (_vm->_escPressed) {
			delete koa05Gxl;
			return false;
		}
	}

	_vm->playSound("saw.snd", false);


	delete koa05Gxl;
	_vm->waitSeconds(1);

	return !_vm->_escPressed;
}

bool DHIntro::introPt3() {
	GxlArchive *koa03Gxl = new GxlArchive("koa03");
	_vm->paletteFadeOut(0, 256, 32);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(koa03Gxl, "face.pcx", 61, 15);
	_vm->paletteFadeIn(0, 256, 64);
	_vm->waitMillis(500);
	if (_vm->_escPressed) {
		delete koa03Gxl;
		return false;
	}

	_vm->playSound("surgt1.snd", false, 10000);
	_vm->drawImageToScreen(koa03Gxl, "ceye.pcx", 126, 79);
	_vm->waitSeconds(1);
	if (_vm->_escPressed) {
		delete koa03Gxl;
		return false;
	}

	_vm->drawImageToScreen(koa03Gxl, "oeye.pcx", 123, 70);
	_vm->waitMillis(500);
	if (_vm->_escPressed) {
		delete koa03Gxl;
		return false;
	}

	_vm->drawImageToScreen(koa03Gxl, "text.pcx", 11, 149);
	for (int i = 0; i < 5; ++i) {
		_vm->drawImageToScreen(koa03Gxl, "mouth.pcx", 114, 96);
		_vm->waitMillis(_vm->getRandom(300));
		_vm->drawImageToScreen(koa03Gxl, "face.pcx", 61, 15);
		_vm->waitMillis(_vm->getRandom(300));
		if (_vm->_escPressed) {
			break;
		}
	}

	delete koa03Gxl;

	return !_vm->_escPressed;
}

bool DHIntro::introPt4() {
	GxlArchive *koa02Gxl = new GxlArchive("koa02");
	_vm->paletteFadeOut(0, 256, 32);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(koa02Gxl, "brain.pcx", 21, 12);
	_vm->paletteFadeIn(0, 256, 64);
	_vm->playSound("surgt2.snd", true, 10000);
	_vm->playSound("scream.snd", true, 10000);
	_vm->waitSeconds(1);
	if (_vm->_escPressed) {
		delete koa02Gxl;
		return false;
	}
	for (int i = 0; i < 20; ++i) {
		int rnd = _vm->getRandom(20);
		if (rnd < 5) {
			Common::String filename = Common::String::format("eye%d.pcx", rnd);
			_vm->drawImageToScreen(koa02Gxl, filename.c_str(), 147, 63, true);
		}

		Common::String filename = Common::String::format("tube%d.pcx", (i + i) % 4);
		_vm->drawImageToScreen(koa02Gxl, filename.c_str(), 103, 109, true);
		_vm->waitMillis(150);
		if (_vm->_escPressed) {
			delete koa02Gxl;
			return false;
		}
	}

	_vm->drawImageToScreen(koa02Gxl, "text.pcx", 11, 150);

	delete koa02Gxl;

	return !_vm->_escPressed;
}

bool DHIntro::introPt5() {
	const char *openNames[] = {"opena04.pcx", "opena08.pcx", "opena12.pcx", "opena16.pcx", "opena17.pcx", "opena18.pcx", "opena20.pcx", "opena21.pcx", "opena23.pcx", "opena24.pcx", "opena27.pcx"};

	GxlArchive *koa01Gxl = new GxlArchive("koa01");
	WWSurface *title1 = new WWSurface(320, 171);
	WWSurface *title2 = new WWSurface(320, 171);
	WWSurface *title3a = new WWSurface(179, 131);
	WWSurface *title3b = new WWSurface(239, 24);
	WWSurface *title3c = new WWSurface(278, 14);

	_vm->drawImageToSurface(koa01Gxl, "title1.pcx", title1, 0, 0);
	_vm->drawImageToSurface(koa01Gxl, "title2.pcx", title2, 0, 0);
	_vm->drawImageToSurface(koa01Gxl, "title3a.pcx", title3a, 0, 0);
	_vm->drawImageToSurface(koa01Gxl, "title3b.pcx", title3b, 0, 0);
	_vm->drawImageToSurface(koa01Gxl, "title3c.pcx", title3c, 0, 0);
	
	while (!_vm->_midi->checkMidiDone())
		_vm->waitMillis(10);

	_vm->paletteFadeOut(0, 256, 32);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(koa01Gxl, "opena01.pcx", 53, 43);
	_vm->paletteFadeIn(0, 256, 64);
	
	_vm->changeMusic("maintitl.xmi");
	_vm->waitSeconds(1);

	for (int i = 0; i < 11; ++i) {
		_vm->drawImageToScreen(koa01Gxl, openNames[i], 53, 43);
		_vm->waitMillis(100);
	}

	_vm->waitSeconds(1);
	WWSurface *sub_title1 = new WWSurface(214, 114);
	sub_title1->copyRectToSurface((Graphics::Surface)*title1, 0, 0, Common::Rect(53, 28, 266, 141));
	_vm->drawRandomEffect(sub_title1, 53, 43, 1, 1);
	_vm->waitMillis(500);
	delete sub_title1;

	if (!_vm->_escPressed) {
		_vm->drawSpiralEffect(title1, 0, 15, 5, 5);
		_vm->drawImageToScreen(koa01Gxl, "titlex.pcx", 0, 15);
		_vm->drawRandomEffect(title2, 0, 15, 4, 4);
		_vm->drawSpiralEffect(title3a, 64, 25, 6, 6);
		_vm->waitSeconds(6);
	}
	if (!_vm->_escPressed) {
		_vm->drawWeaveEffect(title3b, 41, 155, 2, 2, 25);
		_vm->waitSeconds(3);
	}

	if (!_vm->_escPressed) {
		_vm->drawSlideEffect(title3c, 21, 186, 1, 1, 50);
		_vm->waitMillis(300);
	}

	delete title3c;
	delete title3b;
	delete title3a;
	delete title2;
	delete title1;

	while (!_vm->_midi->checkMidiDone())
		_vm->waitMillis(10);
	
	_vm->paletteFadeOut(0, 256, 2);

	delete koa01Gxl;

	return !_vm->_escPressed;
}

bool DHIntro::introPt6() {
	// The original has all the frames hardcoded one after the other, I used a loop instead.
	Frame animation[] = {
		{"syml1.pcx", 75, 22, 50},
		{"syml2.pcx", 75, 22, 50},
		{"syml3.pcx", 75, 22, 50},
		{"syml4.pcx", 75, 22, 50},
		{"syml3.pcx", 75, 22, 50},
		{"syml2.pcx", 75, 22, 50},
		{"syml1.pcx", 75, 22, 50},
		{"syml5.pcx", 75, 22, 50},
		{"syml6.pcx", 75, 22, 50},
		{"syml7.pcx", 75, 22, 50},
		{"syml8.pcx", 75, 22, 50},
		{"syml9.pcx", 75, 22, 50},
		{"syml10.pcx", 75, 22, 50},
		{"syml11.pcx", 75, 22, 50},
		{"syml13.pcx", 75, 22, 50},
		{"syml17.pcx", 75, 22, 50}
	};
	Frame animation2[] = {
		{"syml21.pcx", 75, 22, 50},
		{"syml25.pcx", 75, 22, 50},
		{"syml29.pcx", 75, 22, 50},
		{"syml33.pcx", 75, 22, 50},
		{"syml36.pcx", 75, 22, 50}
	};

	GxlArchive *koa00Gxl = new GxlArchive("koa00");
//	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	_vm->drawImageToScreen(koa00Gxl, "syml1.pcx", 75, 22);
	_vm->paletteFadeIn(0, 256, 64);
	_vm->changeMusic("symtus1.xmi");

	for (const Frame &frame : animation) {
		_vm->drawImageToScreen(koa00Gxl, frame.filename, frame.x, frame.y);
		_vm->waitMillis(frame.delay);
		if (_vm->_escPressed) {
			delete koa00Gxl;
			return false;
		}
	}
	_vm->changeMusic("symtus4.xmi");

	for (const Frame &frame : animation2) {
		_vm->drawImageToScreen(koa00Gxl, frame.filename, frame.x, frame.y);
		_vm->waitMillis(frame.delay);
		if (_vm->_escPressed) {
			delete koa00Gxl;
			return false;
		}
	}
	while (!_vm->_midi->checkMidiDone())
		_vm->waitMillis(10);

	delete koa00Gxl;
	return !_vm->_escPressed;
}

bool DHIntro::intro_credits() {
	GxlArchive *koa00Gxl = new GxlArchive("koa00");
	_vm->paletteFadeOut(0, 256, 2);

	WWSurface *credit1 = new WWSurface(320, 200);
	_vm->drawImageToSurface(koa00Gxl, "credit1.pcx", credit1, 0, 0);
	_vm->_screen->drawSurface(credit1, 0, 0);
	_vm->changeMusic("credits.xmi");
	_vm->paletteFadeIn(0, 256, 2);
	_vm->waitSeconds(9);
	if (!_vm->_escPressed) {
		_vm->drawImageToSurface(koa00Gxl, "credit2.pcx", credit1, 0, 0);
		_vm->_screen->drawSurface(credit1, 0, 0);
		_vm->waitSeconds(8);
	}

	if (!_vm->_escPressed) {
		_vm->drawImageToSurface(koa00Gxl, "credit3.pcx", credit1, 0, 0);
		_vm->_screen->drawSurface(credit1, 0, 0);

		while (!_vm->_midi->checkMidiDone())
			_vm->waitMillis(10);
	}

	_vm->paletteFadeOut(0, 256, 2);

	delete credit1;
	delete koa00Gxl;
	return !_vm->_escPressed;
}

bool DHIntro::introPt8() {
	GxlArchive *koa04Gxl = new GxlArchive("koa04");
//	_vm->paletteFadeOut(0, 256, 2);

	WWSurface *background = new WWSurface(534, 170);
	WWSurface *text = new WWSurface(284, 20);

	_vm->drawImageToSurface(koa04Gxl, "house.pcx", background, 0, 0);
	_vm->drawImageToSurface(koa04Gxl, "lake.pcx", background, 214, 0);

	_vm->_screen->clear(0);

	WWSurface *scroll = new WWSurface(320, 170);
	scroll->copyRectToSurface((Graphics::Surface)*background, 0, 0, Common::Rect(214, 0, 533, 169));

	_vm->_screen->drawSurface(scroll, 0, 10);
	if (!_vm->_escPressed) {
		_vm->changeMusic("23years.xmi");
		_vm->paletteFadeIn(0, 256, 4);
		_vm->waitSeconds(1);
	}
	if (!_vm->_escPressed) {
		_vm->drawImageToSurface(koa04Gxl, "text.pcx", text, 0, 0);
		_vm->_screen->drawSurfaceTransparent(text, 18, 150);
		_vm->waitSeconds(1);
	}

	for (int16 i = 213; i >= 0 && !_vm->_escPressed; --i) {
		scroll->copyRectToSurface((Graphics::Surface)*background, 0, 0, Common::Rect(i, 0, i + 319, 169));
		scroll->drawSurfaceTransparent(text, 18, 140);
		_vm->_screen->drawSurface(scroll, 0, 10);
	}

	_vm->waitSeconds(3);
	_vm->paletteFadeOut(0, 256, 8);

	delete scroll;
	delete text;
	delete background;

	delete koa04Gxl;
	return !_vm->_escPressed;
}

bool DHIntro::introPt9() {
	GxlArchive *koa00Gxl = new GxlArchive("koa00");
	//	_vm->paletteFadeOut(0, 256, 2);
	_vm->_screen->clear(0);

	WWSurface *background = new WWSurface(320, 200);
	_vm->drawImageToSurface(koa00Gxl, "thadtxt.pcx", background, 0, 0);
	_vm->_screen->drawSurface(background, 0,0);
	_vm->paletteFadeIn(0, 256, 2);
	_vm->waitSeconds(9);

	if (!_vm->_escPressed) {
		_vm->drawImageToSurface(koa00Gxl, "newsp.pcx", background, 0, 0);
		_vm->_screen->drawSurface(background, 0, 0);
		_vm->waitSeconds(12);
	}

	if (!_vm->_escPressed) {
		_vm->drawImageToSurface(koa00Gxl, "grave.pcx", background, 0, 0);
		_vm->_screen->drawSurface(background, 0, 0);

		while (!_vm->_midi->checkMidiDone())
			_vm->waitMillis(10);
	}

	_vm->waitSeconds(2);
	_vm->paletteFadeOut(0, 256, 2);
	_vm->_screen->clear(0);
	_vm->paletteFadeIn(0, 256, 2);

	delete background;
	delete koa00Gxl;
	return !_vm->_escPressed;
}
} // End of namespace WW
