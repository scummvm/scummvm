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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "common/system.h"

#include "common/file.h"

#include "graphics/primitives.h"			// for Graphics::drawLine

#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/sound.h"


namespace Parallaction {

// part completion messages
static const char *endMsg0[] = {"COMPLIMENTI!", "BRAVO!", "CONGRATULATIONS!", "PRIMA!"};
static const char *endMsg1[] = {"HAI FINITO QUESTA PARTE", "TU AS COMPLETE' CETTE AVENTURE", "YOU HAVE COMPLETED THIS PART", "DU HAST EIN ABENTEUER ERFOLGREICH"};
static const char *endMsg2[] = {"ORA COMPLETA IL RESTO ", "AVEC SUCCES.",  "NOW GO ON WITH THE REST OF", "ZU ENDE GEFUHRT"};
static const char *endMsg3[] = {"DELL' AVVENTURA",  "CONTINUE AVEC LES AUTRES", "THIS ADVENTURE", "MACH' MIT DEN ANDEREN WEITER"};
// game completion messages
static const char *endMsg4[] = {"COMPLIMENTI!", "BRAVO!", "CONGRATULATIONS!", "PRIMA!"};
static const char *endMsg5[] = {"HAI FINITO LE TRE PARTI", "TU AS COMPLETE' LES TROIS PARTIES", "YOU HAVE COMPLETED THE THREE PARTS", "DU HAST DREI ABENTEURE ERFOLGREICH"};
static const char *endMsg6[] = {"DELL' AVVENTURA", "DE L'AVENTURE", "OF THIS ADVENTURE", "ZU ENDE GEFUHRT"};
static const char *endMsg7[] = {"ED ORA IL GRAN FINALE ", "ET MAINTENANT LE GRAND FINAL", "NOW THE GREAT FINAL", "UND YETZT DER GROSSE SCHLUSS!"};


/*
	intro callables data members
*/


static uint16 _rightHandPositions[684] = {
	0x0064, 0x0046, 0x006c, 0x0046, 0x0074, 0x0046, 0x007c, 0x0046,
	0x0084, 0x0046, 0x008c, 0x0046, 0x0094, 0x0046, 0x009c, 0x0046,
	0x00a4, 0x0046, 0x00ac, 0x0046, 0x00b4, 0x0046, 0x00bc, 0x0046,
	0x00c4, 0x0046, 0x00cc, 0x0046, 0x00d4, 0x0046, 0x00dc, 0x0046,
	0x00e4, 0x0046, 0x00ec, 0x0046, 0x00f4, 0x0046, 0x00fc, 0x0046,
	0x0104, 0x0046, 0x00ff, 0x0042, 0x00ff, 0x004a, 0x00ff, 0x0052,
	0x00ff, 0x005a, 0x00ff, 0x0062, 0x00ff, 0x006a, 0x00ff, 0x0072,
	0x00ff, 0x007a, 0x00ff, 0x0082, 0x00ff, 0x008a, 0x00ff, 0x0092,
	0x00ff, 0x009a, 0x00ff, 0x00a2, 0x0104, 0x0097, 0x00fc, 0x0097,
	0x00f4, 0x0097, 0x00ec, 0x0097, 0x00e4, 0x0097, 0x00dc, 0x0097,
	0x00d4, 0x0097, 0x00cc, 0x0097, 0x00c4, 0x0097, 0x00bc, 0x0097,
	0x00b4, 0x0097, 0x00ac, 0x0097, 0x00a4, 0x0097, 0x009c, 0x0097,
	0x0094, 0x0097, 0x008c, 0x0097, 0x0084, 0x0097, 0x007c, 0x0097,
	0x0074, 0x0097, 0x006c, 0x0097, 0x0064, 0x0097, 0x0066, 0x0042,
	0x0066, 0x004a, 0x0066, 0x0052, 0x0066, 0x005a, 0x0066, 0x0062,
	0x0066, 0x006a, 0x0066, 0x0072, 0x0066, 0x007a, 0x0066, 0x0082,
	0x0066, 0x008a, 0x0066, 0x0092, 0x0066, 0x009a, 0x0066, 0x00a2,
	0x008c, 0x0091, 0x0099, 0x0042, 0x0099, 0x004a, 0x0099, 0x0052,
	0x0099, 0x005a, 0x0099, 0x0062, 0x0099, 0x006a, 0x0099, 0x0072,
	0x0099, 0x007a, 0x0099, 0x0082, 0x0099, 0x008a, 0x0099, 0x0092,
	0x0099, 0x009a, 0x0099, 0x00a2, 0x00a0, 0x004d, 0x00cc, 0x0042,
	0x00cc, 0x004a, 0x00cc, 0x0052, 0x00cc, 0x005a, 0x00cc, 0x0062,
	0x00cc, 0x006a, 0x00cc, 0x0072, 0x00cc, 0x007a, 0x00cc, 0x0082,
	0x00cc, 0x008a, 0x00cc, 0x0092, 0x00cc, 0x009a, 0x00cc, 0x00a2,
	0x00ca, 0x0050, 0x00b1, 0x0050, 0x0081, 0x0052, 0x007e, 0x0052,
	0x007c, 0x0055, 0x007c, 0x005c, 0x007e, 0x005e, 0x0080, 0x005e,
	0x0082, 0x005c, 0x0082, 0x0054, 0x0080, 0x0052, 0x0078, 0x0052,
	0x007c, 0x005e, 0x0077, 0x0061, 0x0074, 0x006e, 0x0074, 0x0078,
	0x0076, 0x007a, 0x0079, 0x0078, 0x0079, 0x0070, 0x0078, 0x0070,
	0x0078, 0x006b, 0x007b, 0x0066, 0x007a, 0x006f, 0x0084, 0x006f,
	0x0085, 0x0066, 0x0086, 0x0070, 0x0085, 0x0070, 0x0085, 0x0079,
	0x0088, 0x0079, 0x008a, 0x0078, 0x008a, 0x006c, 0x0087, 0x0061,
	0x0085, 0x005f, 0x0082, 0x005f, 0x0080, 0x0061, 0x007e, 0x0061,
	0x007b, 0x005f, 0x007c, 0x006f, 0x007c, 0x0071, 0x0079, 0x0074,
	0x0079, 0x0089, 0x0076, 0x008c, 0x0076, 0x008e, 0x007a, 0x008e,
	0x007f, 0x0089, 0x007f, 0x0083, 0x007e, 0x0083, 0x007e, 0x0077,
	0x0080, 0x0077, 0x0080, 0x0083, 0x0080, 0x008b, 0x0084, 0x0090,
	0x0088, 0x0090, 0x0088, 0x008e, 0x0085, 0x008b, 0x0085, 0x0074,
	0x0082, 0x0071, 0x00b2, 0x0052, 0x00b0, 0x0054, 0x00b0, 0x0056,
	0x00ae, 0x0058, 0x00af, 0x0059, 0x00af, 0x005e, 0x00b2, 0x0061,
	0x00b5, 0x0061, 0x00b8, 0x005e, 0x00b8, 0x005a, 0x00b9, 0x0059,
	0x00b9, 0x0058, 0x00b7, 0x0056, 0x00b7, 0x0054, 0x00b5, 0x0052,
	0x00b2, 0x0052, 0x00ae, 0x005a, 0x00ab, 0x005b, 0x00ab, 0x006d,
	0x00ae, 0x0072, 0x00b8, 0x0072, 0x00bc, 0x006d, 0x00bc, 0x005b,
	0x00b9, 0x005a, 0x00bc, 0x005c, 0x00be, 0x005c, 0x00c1, 0x005f,
	0x00c4, 0x0067, 0x00c4, 0x006d, 0x00c1, 0x0076, 0x00c0, 0x0077,
	0x00bd, 0x0077, 0x00bb, 0x0075, 0x00bd, 0x0073, 0x00bb, 0x0072,
	0x00be, 0x0070, 0x00be, 0x006a, 0x00a9, 0x006a, 0x00a9, 0x0070,
	0x00ac, 0x0072, 0x00aa, 0x0073, 0x00ac, 0x0075, 0x00aa, 0x0077,
	0x00a7, 0x0077, 0x00a3, 0x006d, 0x00a3, 0x0067, 0x00a6, 0x005f,
	0x00a9, 0x005c, 0x00ab, 0x005c, 0x00ac, 0x0077, 0x00ac, 0x007c,
	0x00ab, 0x007c, 0x00ab, 0x0084, 0x00ac, 0x0084, 0x00ac, 0x008b,
	0x00a9, 0x008e, 0x00a9, 0x0090, 0x00ae, 0x0090, 0x00ae, 0x008d,
	0x00b2, 0x008c, 0x00b2, 0x0087, 0x00b1, 0x0086, 0x00b1, 0x007b,
	0x00b2, 0x0079, 0x00b4, 0x0079, 0x00b4, 0x007d, 0x00b5, 0x007d,
	0x00b5, 0x0087, 0x00b4, 0x0087, 0x00b4, 0x008c, 0x00b6, 0x008c,
	0x00b9, 0x0091, 0x00b4, 0x0091, 0x00bd, 0x008f, 0x00ba, 0x008c,
	0x00ba, 0x0083, 0x00bb, 0x0082, 0x00bb, 0x0075, 0x00cc, 0x006e,
	0x00d4, 0x006c, 0x00db, 0x0069, 0x00d9, 0x0068, 0x00d9, 0x0064,
	0x00dc, 0x0064, 0x00dc, 0x0060, 0x00df, 0x0056, 0x00e5, 0x0052,
	0x00e7, 0x0052, 0x00ec, 0x0056, 0x00ef, 0x005d, 0x00f1, 0x0065,
	0x00f3, 0x0064, 0x00f3, 0x0069, 0x00f0, 0x0069, 0x00ec, 0x0065,
	0x00ec, 0x005e, 0x00e9, 0x005f, 0x00e9, 0x005a, 0x00e7, 0x0058,
	0x00e4, 0x0058, 0x00e3, 0x0054, 0x00e3, 0x0058, 0x00e1, 0x005c,
	0x00e4, 0x0061, 0x00e7, 0x0061, 0x00e9, 0x005f, 0x00eb, 0x005d,
	0x00e4, 0x0062, 0x00e0, 0x0064, 0x00e0, 0x0069, 0x00e2, 0x006b,
	0x00e0, 0x0072, 0x00e0, 0x0077, 0x00ec, 0x0077, 0x00ec, 0x0071,
	0x00ea, 0x006b, 0x00ec, 0x006a, 0x00ec, 0x0063, 0x00e7, 0x0063,
	0x00e7, 0x0065, 0x00e1, 0x0069, 0x00e3, 0x0068, 0x00e6, 0x0069,
	0x00ec, 0x005e, 0x00ea, 0x006b, 0x00e7, 0x006b, 0x00e7, 0x006a,
	0x00e5, 0x006a, 0x00e5, 0x006b, 0x00e2, 0x006b, 0x00df, 0x006c,
	0x00dc, 0x006f, 0x00dc, 0x0071, 0x00da, 0x0073, 0x00d8, 0x0073,
	0x00d8, 0x006f, 0x00dc, 0x006b, 0x00dc, 0x0069, 0x00dd, 0x0068,
	0x00ef, 0x0068, 0x00f0, 0x0069, 0x00f0, 0x006b, 0x00f4, 0x006f,
	0x00f4, 0x0072, 0x00f3, 0x0073, 0x00f2, 0x0073, 0x00f0, 0x0071,
	0x00f0, 0x006f, 0x00ec, 0x006b, 0x00ec, 0x007a, 0x00eb, 0x007b,
	0x00eb, 0x007f, 0x00ec, 0x0080, 0x00ec, 0x0084, 0x00eb, 0x0085,
	0x00eb, 0x008b, 0x00ec, 0x008c, 0x00ec, 0x008f, 0x00ed, 0x0091,
	0x00e9, 0x0091, 0x00e9, 0x008f, 0x00e7, 0x008d, 0x00e7, 0x0090,
	0x00e7, 0x0089, 0x00e8, 0x0088, 0x00e8, 0x0086, 0x00e7, 0x0085,
	0x00e7, 0x007d, 0x00e6, 0x007c, 0x00e6, 0x0078, 0x00e5, 0x007d,
	0x00e5, 0x0085, 0x00e4, 0x0086, 0x00e4, 0x0088, 0x00e5, 0x0089,
	0x00e5, 0x0090, 0x00e5, 0x008b, 0x00e3, 0x0091, 0x00df, 0x0091,
	0x00e0, 0x0090, 0x00e0, 0x008c, 0x00e2, 0x008b, 0x00e1, 0x0085,
	0x00e0, 0x0084, 0x00e0, 0x0080, 0x00e1, 0x007f, 0x00e1, 0x007c,
	0x00e0, 0x007b, 0x00e0, 0x0077
};

struct Credit {
	const char *_role;
	const char *_name;
} _credits[] = {
	{"Music and Sound Effects", "MARCO CAPRELLI"},
	{"PC Version", "RICCARDO BALLARINO"},
	{"Project Manager", "LOVRANO CANEPA"},
	{"Production", "BRUNO BOZ"},
	{"Special Thanks to", "LUIGI BENEDICENTI - GILDA and DANILO"},
	{"Copyright 1992 Euclidea s.r.l ITALY", "All rights reserved"}
};

/*
	game callables
*/

void Parallaction_ns::_c_null(void *parm) {

	return;
}

void Parallaction_ns::_c_play_boogie(void *parm) {

	static uint16 flag = 1;

	if (flag == 0)
		return;
	flag = 0;

	_soundMan->setMusicFile("boogie2");
	_soundMan->playMusic();

	return;
}


void Parallaction_ns::_c_score(void *parm) {
	_score += 5;
	return;
}

void Parallaction_ns::_c_fade(void *parm) {

	Palette pal;
	_gfx->setPalette(pal);

	for (uint16 _di = 0; _di < 64; _di++) {
		pal.fadeTo(_gfx->_palette, 1);
		_gfx->setPalette(pal);

		_gfx->updateScreen();
		g_system->delayMillis(20);
	}

	return;
}



void Parallaction_ns::_c_moveSarc(void *parm) {

	AnimationPtr a;

	if (_introSarcData2 != 0) {

		_introSarcData2 = 0;
		if (!_moveSarcZones[0]) {

			_moveSarcZones[0] = findZone("sarc1");
			_moveSarcZones[1] = findZone("sarc2");
			_moveSarcZones[2] = findZone("sarc3");
			_moveSarcZones[3] = findZone("sarc4");
			_moveSarcZones[4] = findZone("sarc5");

			_moveSarcExaZones[0] = findZone("sarc1exa");
			_moveSarcExaZones[1] = findZone("sarc2exa");
			_moveSarcExaZones[2] = findZone("sarc3exa");
			_moveSarcExaZones[3] = findZone("sarc4exa");
			_moveSarcExaZones[4] = findZone("sarc5exa");

		}

		a = findAnimation("sposta");

		_moveSarcZone1 = *(ZonePtr*)parm;

		for (uint16 _si = 0; _si < 5; _si++) {
			if (_moveSarcZones[_si] == _moveSarcZone1) {
				_moveSarcZone0 = _moveSarcExaZones[_si];
			}
		}

		_introSarcData1 = _introSarcData3 - _moveSarcZone1->_left;
		a->_z = _introSarcData3;
		a->_frame = _moveSarcZone1->_top - (_introSarcData1 / 20);
		_introSarcData3 = _moveSarcZone1->_left;

		if (_introSarcData1 > 0) {
			a->_left = _introSarcData1 / 2;
		} else {
			a->_left = -_introSarcData1 / 2;
		}

		if (_introSarcData1 > 0) {
			a->_top = 2;
		} else {
			a->_top = -2;
		}

		return;

	}

	_introSarcData2 = 1;
	_moveSarcZone1->translate(_introSarcData1, -_introSarcData1 / 20);
	_moveSarcZone0->translate(_introSarcData1, -_introSarcData1 / 20);

	if (_moveSarcZones[0]->_left == 35 &&
		_moveSarcZones[1]->_left == 68 &&
		_moveSarcZones[2]->_left == 101 &&
		_moveSarcZones[3]->_left == 134 &&
		_moveSarcZones[4]->_left == 167) {

		a = findAnimation("finito");

		a->_flags |= (kFlagsActive | kFlagsActing);
		setLocationFlags(0x20);		// GROSS HACK: activates 'finito' flag in dinoit_museo.loc
	}

	return;

}




void Parallaction_ns::_c_contaFoglie(void *parm) {

	num_foglie++;
	if (num_foglie != 6)
		return;

	_commandFlags |= 0x1000;

	return;
}

void Parallaction_ns::_c_zeroFoglie(void *parm) {
	num_foglie = 0;
	return;
}

void Parallaction_ns::_c_trasformata(void *parm) {
	_engineFlags ^= kEngineTransformedDonna;
	// No need to invoke changeCharacter here, as
	// transformation happens on a location switch
	// and character change is automatically triggered.
	return;
}

void Parallaction_ns::_c_offMouse(void *parm) {
	_input->showCursor(false);
	_engineFlags |= kEngineBlockInput;
	return;
}

void Parallaction_ns::_c_onMouse(void *parm) {
	_engineFlags &= ~kEngineBlockInput;
	_input->showCursor(true);
	return;
}



void Parallaction_ns::_c_setMask(void *parm) {

	memset(_gfx->_backgroundInfo.mask.data + 3600, 0, 3600);
	_gfx->_backgroundInfo.layers[1] = 500;

	return;
}

void Parallaction_ns::_c_endComment(void *param) {

	showLocationComment(_location._endComment, true);

	Palette pal(_gfx->_palette);
	pal.makeGrayscale();

	for (uint di = 0; di < 64; di++) {
		_gfx->_palette.fadeTo(pal, 1);
		_gfx->setPalette(_gfx->_palette);

		_gfx->updateScreen();
		g_system->delayMillis(20);
	}

	_input->waitForButtonEvent(kMouseLeftUp);
	_balloonMan->freeBalloons();

	return;
}

void Parallaction_ns::_c_frankenstein(void *parm) {

	Palette pal0(_gfx->_palette);
	Palette pal1;

	for (uint16 i = 0; i < 32; i++) {
		pal0.setEntry(i, -1, 0, 0);			// leaves reds unchanged while zeroing other components
	}

	for (uint16 _di = 0; _di < 30; _di++) {
		g_system->delayMillis(20);
		_gfx->setPalette(pal0);
		_gfx->updateScreen();
		g_system->delayMillis(20);
		_gfx->setPalette(pal1);
		_gfx->updateScreen();
	}

	_gfx->setPalette(_gfx->_palette);
	_gfx->updateScreen();

	return;
}


void Parallaction_ns::_c_finito(void *parm) {

	setPartComplete(_char);

	cleanInventory();
	_gfx->setPalette(_gfx->_palette);

	uint id[4];

	if (allPartsComplete()) {
		id[0] = _gfx->createLabel(_menuFont, endMsg4[_language], 1);
		id[1] = _gfx->createLabel(_menuFont, endMsg5[_language], 1);
		id[2] = _gfx->createLabel(_menuFont, endMsg6[_language], 1);
		id[3] = _gfx->createLabel(_menuFont, endMsg7[_language], 1);
	} else {
		id[0] = _gfx->createLabel(_menuFont, endMsg0[_language], 1);
		id[1] = _gfx->createLabel(_menuFont, endMsg1[_language], 1);
		id[2] = _gfx->createLabel(_menuFont, endMsg2[_language], 1);
		id[3] = _gfx->createLabel(_menuFont, endMsg3[_language], 1);
	}

	_gfx->showLabel(id[0], CENTER_LABEL_HORIZONTAL, 70);
	_gfx->showLabel(id[1], CENTER_LABEL_HORIZONTAL, 100);
	_gfx->showLabel(id[2], CENTER_LABEL_HORIZONTAL, 130);
	_gfx->showLabel(id[3], CENTER_LABEL_HORIZONTAL, 160);

	_gfx->updateScreen();
	_input->waitForButtonEvent(kMouseLeftUp);

	_gfx->freeLabels();

	if (allPartsComplete()) {
		scheduleLocationSwitch("estgrotta.drki");
	} else {
		selectStartLocation();
	}

	cleanupGame();

	return;
}

void Parallaction_ns::_c_ridux(void *parm) {
	changeCharacter(_minidinoName);
	return;
}

void Parallaction_ns::_c_testResult(void *parm) {
	if (_inTestResult) {		// NOTE: _inTestResult has been added because the scripts call _c_testResult multiple times to cope with
								// the multiple buffering that was used in the original engine. _inTestResult now prevents the engine
								// from crashing when the scripts are executed.
		return;
	}
	_inTestResult = true;

	_gfx->freeLabels();
	_gfx->updateScreen();

	_disk->selectArchive("disk1");
	parseLocation("common");

	uint id[2];
	id[0] = _gfx->createLabel(_menuFont, _location._slideText[0], 1);
	id[1] = _gfx->createLabel(_menuFont, _location._slideText[1], 1);

	_gfx->showLabel(id[0], CENTER_LABEL_HORIZONTAL, 38);
	_gfx->showLabel(id[1], CENTER_LABEL_HORIZONTAL, 58);

	return;
}

void Parallaction_ns::_c_offSound(void*) {
	_soundMan->stopSfx(0);
	_soundMan->stopSfx(1);
	_soundMan->stopSfx(2);
	_soundMan->stopSfx(3);
}

void Parallaction_ns::_c_startMusic(void*) {
	_soundMan->playMusic();
}

void Parallaction_ns::_c_closeMusic(void*) {
	_soundMan->stopMusic();
}

/*
	intro callables
*/

void Parallaction_ns::_c_startIntro(void *parm) {
	_rightHandAnim = findAnimation("righthand");

	if (getPlatform() == Common::kPlatformPC) {
		_soundMan->setMusicFile("intro");
		_soundMan->playMusic();
	}

	_engineFlags |= kEngineBlockInput;

	return;
}

void Parallaction_ns::_c_endIntro(void *parm) {

	debugC(1, kDebugExec, "endIntro()");

	uint id[2];
	for (uint16 _si = 0; _si < 6; _si++) {
		id[0] = _gfx->createLabel(_menuFont, _credits[_si]._role, 1);
		id[1] = _gfx->createLabel(_menuFont, _credits[_si]._name, 1);

		_gfx->showLabel(id[0], CENTER_LABEL_HORIZONTAL, 80);
		_gfx->showLabel(id[1], CENTER_LABEL_HORIZONTAL, 100);

		_gfx->updateScreen();

		_input->waitForButtonEvent(kMouseLeftUp, 5500);

		_gfx->freeLabels();
	}
	debugC(1, kDebugExec, "endIntro(): done showing credits");

	_soundMan->stopMusic();

	if ((getFeatures() & GF_DEMO) == 0) {

		id[0] = _gfx->createLabel(_menuFont, "CLICK MOUSE BUTTON TO START", 1);
		_gfx->showLabel(id[0], CENTER_LABEL_HORIZONTAL, 80);
		_gfx->updateScreen();
		_input->waitForButtonEvent(kMouseLeftUp);
		_gfx->freeLabels();
		_engineFlags &= ~kEngineBlockInput;
		selectStartLocation();
		cleanupGame();
	} else {
		_gfx->updateScreen();
		_input->waitForButtonEvent(kMouseLeftUp);
	}

	return;
}

void Parallaction_ns::_c_moveSheet(void *parm) {

	static uint16 x = 319;

	if (x > 66)
		x -= 16;

	Common::Rect r;

	r.left = x;
	r.top = 47;
	r.right = (x + 32 > 319) ? 319 : (x + 32);
	r.bottom = 199;
	_gfx->fillBackground(r, 1);

	if (x >= 104) return;

	r.left = x+215;
	r.top = 47;
	r.right = (x + 247 > 319) ? 319 : (x + 247);
	r.bottom = 199;
	_gfx->fillBackground(r, 12);

	return;
}

void zeroMask(int x, int y, int color, void *data) {
	//_vm->_gfx->zeroMaskValue(x, y, color);

	BackgroundInfo* info = (BackgroundInfo*)data;

	uint16 _ax = x + y * info->width;
	info->mask.data[_ax >> 2] &= ~(3 << ((_ax & 3) << 1));

}

void Parallaction_ns::_c_sketch(void *parm) {

	static uint16 index = 1;

	uint16 newx;
	uint16 newy;

	uint16 oldy = _rightHandPositions[2*(index-1)+1];
	uint16 oldx = _rightHandPositions[2*(index-1)];

	// WORKAROUND: original code overflowed _rightHandPositions by trying
	// to access elements at positions 684 and 685. That used to happen
	// when index == 342. Code now checks for this possibility and assigns
	// the last valid value to the new coordinates for drawing without
	// accessing the array.
	if (index == 342) {
		newy = oldy;
		newx = oldx;
	} else {
		newy = _rightHandPositions[2*index+1];
		newx = _rightHandPositions[2*index];
	}

	Graphics::drawLine(oldx, oldy, newx, newy, 0, zeroMask, &_gfx->_backgroundInfo);

	_rightHandAnim->_left = newx;
	_rightHandAnim->_top = newy - 20;

	index++;

	return;
}




void Parallaction_ns::_c_shade(void *parm) {

	Common::Rect r(
		_rightHandAnim->_left - 36,
		_rightHandAnim->_top - 36,
		_rightHandAnim->_left,
		_rightHandAnim->_top
	);

	uint16 _di = r.left/4 + r.top * _gfx->_backgroundInfo.mask.internalWidth;

	for (uint16 _si = r.top; _si < r.bottom; _si++) {
		memset(_gfx->_backgroundInfo.mask.data + _di, 0, r.width()/4+1);
		_di += _gfx->_backgroundInfo.mask.internalWidth;
	}

	return;

}

int16 projectorProgram[] = {
	0, 50, 1, 50, 2, 49, 3, 49, 4, 48, 5, 48, 6, 47, 7, 47, 8, 46, 9, 46, 10, 45, 11, 45,
   12, 44, 13, 44, 14, 43, 15, 43, 16, 42, 17, 42, 18, 41, 19, 41, 20, 40, 21, 40, 22, 39,
   23, 39, 24, 38, 25, 38, 26, 37, 27, 37, 28, 36, 29, 36, 30, 35, 31, 35, 32, 34, 33, 34,
   34, 33, 35, 33, 36, 32, 37, 32, 38, 31, 39, 31, 40, 30, 41, 30, 42, 29, 43, 29, 44, 28,
   45, 28, 46, 27, 47, 27, 48, 26, 49, 26, 50, 25, 51, 25, 52, 24, 53, 24, 54, 23, 55, 23,
   56, 22, 57, 22, 58, 21, 59, 21, 60, 20, 61, 20, 62, 19, 63, 19, 64, 18, 65, 18, 66, 17,
   67, 17, 68, 16, 69, 16, 70, 15, 71, 15, 72, 14, 73, 14, 74, 13, 75, 13, 76, 12, 77, 12,
   78, 11, 79, 11, 80, 10, 81, 10, 82, 9, 83, 9, 84, 8, 85, 8, 86, 7, 87, 7, 88, 6, 89, 6,
   90, 5, 91, 5, 92, 4, 93, 4, 94, 3, 95, 3, 96, 2, 97, 2, 98, 1, 99, 1, 100, 0, 101, 0,
   102, 1, 103, 1, 104, 2, 105, 2, 106, 3, 107, 3, 108, 4, 109, 4, 110, 5, 111, 5, 112, 6,
   113, 6, 114, 7, 115, 7, 116, 8, 117, 8, 118, 9, 119, 9, 120, 10, 121, 10, 122, 11, 123,
   11, 124, 12, 125, 12, 126, 13, 127, 13, 128, 14, 129, 14, 130, 15, 131, 15, 132, 16, 133,
   16, 134, 17, 135, 17, 136, 18, 137, 18, 138, 19, 139, 19, 140, 20, 141, 20, 142, 21, 143,
   21, 144, 22, 145, 22, 146, 23, 147, 23, 148, 24, 149, 24, 150, 25, 149, 25, 148, 25, 147,
   25, 146, 25, 145, 25, 144, 25, 143, 25, 142, 25, 141, 25, 140, 25, 139, 25, 138, 25, 137,
   25, 136, 25, 135, 25, 134, 25, 133, 25, 132, 25, 131, 25, 130, 25, 129, 25, 128, 25, 127,
   25, 126, 25, 125, 25, 124, 25, 123, 25, 122, 25, 121, 25, 120, 25, 119, 25, 118, 25, 117,
   25, 116, 25, 115, 25, 114, 25, 113, 25, 112, 25, 111, 25, 110, 25, -1, -1
};

void Parallaction_ns::_c_projector(void*) {
	_gfx->setHalfbriteMode(true);
	_gfx->setProjectorProgram(projectorProgram);
}

void Parallaction_ns::_c_HBOff(void*) {
	_gfx->setHalfbriteMode(false);
}

void Parallaction_ns::_c_HBOn(void*) {
	_gfx->setHalfbriteMode(true);
}


} // namespace Parallaction
