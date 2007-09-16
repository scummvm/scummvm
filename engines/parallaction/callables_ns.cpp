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


#include "common/stdafx.h"
#include "common/system.h"

#include "common/file.h"

#include "graphics/primitives.h"			// for Graphics::drawLine

#include "parallaction/parallaction.h"
#include "parallaction/menu.h"
#include "parallaction/sound.h"


namespace Parallaction {

/*
	game callables data members
*/

static Zone *_moveSarcZone0 = NULL;
static int16 _introSarcData1 = 0;
static Zone *_moveSarcZone1 = NULL;

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

static uint16 num_foglie = 0;
static Zone *_moveSarcZones[5];
static Zone *_moveSarcExaZones[5];

/*
	intro callables data members
*/

static Animation *_rightHandAnim;

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

	Animation *a;

	if (_introSarcData2 != 0) {

		_introSarcData2 = 0;
		if (_moveSarcZones[0] == NULL) {

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

		_moveSarcZone1 = (Zone*)parm;

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
		_localFlags[_currentLocationIndex] |= 0x20;		// GROSS HACK: activates 'finito' flag in dinoit_museo.loc
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
	return;
}

void Parallaction_ns::_c_offMouse(void *parm) {
	showCursor(false);
	_engineFlags |= kEngineBlockInput;
	return;
}

void Parallaction_ns::_c_onMouse(void *parm) {
	_engineFlags &= ~kEngineBlockInput;
	showCursor(true);
	return;
}



void Parallaction_ns::_c_setMask(void *parm) {

	memset(_backgroundInfo->mask.data + 3600, 0, 3600);
	_gfx->_bgLayers[1] = 500;

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

	waitUntilLeftClick();

	return;
}

void Parallaction_ns::_c_frankenstein(void *parm) {

	Palette pal0(_gfx->_palette);
	Palette pal1;

	for (uint16 i = 0; i <= 32; i++) {
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

	const char **v8C = endMsg0;
	const char **v7C = endMsg1;
	const char **v6C = endMsg2;
	const char **v5C = endMsg3;
	const char **v4C = endMsg4;
	const char **v3C = endMsg5;
	const char **v2C = endMsg6;
	const char **v1C = endMsg7;

	Common::File stream;

	stream.open(_characterName, Common::File::kFileWriteMode);
	if (stream.isOpen())
		stream.close();

	Common::File streamDino, streamDough, streamDonna;

	streamDino.open("dino");
	streamDough.open("dough");
	streamDonna.open("donna");

	bool gameCompleted = streamDino.isOpen() && streamDough.isOpen() && streamDonna.isOpen();

	streamDino.close();
	streamDough.close();
	streamDonna.close();

	cleanInventory();

	_gfx->setPalette(_gfx->_palette);

	if (gameCompleted) {
		_gfx->setFont(_menuFont);
		_gfx->displayCenteredString(70, v4C[_language]);
		_gfx->displayCenteredString(100, v3C[_language]);
		_gfx->displayCenteredString(130, v2C[_language]);
		_gfx->displayCenteredString(160, v1C[_language]);

		_gfx->updateScreen();
		waitUntilLeftClick();

		strcpy(_location._name, "estgrotta.drki");

		_engineFlags |= kEngineChangeLocation;
	} else {
		_gfx->setFont(_menuFont);
		_gfx->displayCenteredString(70, v8C[_language]);
		_gfx->displayCenteredString(100, v7C[_language]);
		_gfx->displayCenteredString(130, v6C[_language]);
		_gfx->displayCenteredString(160, v5C[_language]);

		_gfx->updateScreen();
		waitUntilLeftClick();

		_menu->selectCharacter();
	}

	// this code saves main character animation from being removed from the following code
	_animations.remove(&_char._ani);
	_locationNames[0][0] = '\0';
	_numLocations = 0;
	_commandFlags = 0;

	// this flag tells freeZones to unconditionally remove *all* Zones
	_engineFlags |= kEngineQuit;

	freeZones();
	freeAnimations();

	// this dangerous flag can now be cleared
	_engineFlags &= ~kEngineQuit;

	// main character animation is restored
	_animations.push_front(&_char._ani);
	_score = 0;

	return;
}

void Parallaction_ns::_c_ridux(void *parm) {
	changeCharacter(_minidinoName);
	return;
}

void Parallaction_ns::_c_testResult(void *parm) {
	_gfx->swapBuffers();

	_disk->selectArchive("disk1");
	parseLocation("common");

	_gfx->setFont(_menuFont);

	_gfx->displayCenteredString(38, _slideText[0]);
	_gfx->displayCenteredString(58, _slideText[1]);

	_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);
	_gfx->copyScreen(Gfx::kBitFront, Gfx::kBit2);

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

	_gfx->setFont(_menuFont);

	debugC(1, kDebugExec, "endIntro()");

	for (uint16 _si = 0; _si < 6; _si++) {
		_gfx->displayCenteredString(80, _credits[_si]._role);
		_gfx->displayCenteredString(100, _credits[_si]._name);

		_gfx->updateScreen();

		for (uint16 v2 = 0; v2 < 100; v2++) {
			_mouseButtons = kMouseNone;
			updateInput();
			if (_mouseButtons == kMouseLeftUp)
				break;

			waitTime( 1 );
		}

		_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
	}
	debugC(1, kDebugExec, "endIntro(): done showing credits");

	if ((getFeatures() & GF_DEMO) == 0) {
		_gfx->displayCenteredString(80, "CLICK MOUSE BUTTON TO START");
		_gfx->updateScreen();

		waitUntilLeftClick();

		_engineFlags &= ~kEngineBlockInput;
		_menu->selectCharacter();
	} else {
		waitUntilLeftClick();
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
	_gfx->floodFill(Gfx::kBitBack, r, 1);
	_gfx->floodFill(Gfx::kBit2, r, 1);

	if (x >= 104) return;

	r.left = x+215;
	r.top = 47;
	r.right = (x + 247 > 319) ? 319 : (x + 247);
	r.bottom = 199;
	_gfx->floodFill(Gfx::kBitBack, r, 12);
	_gfx->floodFill(Gfx::kBit2, r, 12);

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

	Graphics::drawLine(oldx, oldy, newx, newy, 0, zeroMask, _backgroundInfo);

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

	uint16 _di = r.left/4 + r.top * _backgroundInfo->mask.internalWidth;

	for (uint16 _si = r.top; _si < r.bottom; _si++) {
		memset(_backgroundInfo->mask.data + _di, 0, r.width()/4+1);
		_di += _backgroundInfo->mask.internalWidth;
	}

	return;

}

void Parallaction_ns::_c_projector(void*) {
#ifdef HALFBRITE
	static int dword_16032 = 0;

//	Bitmap bm;
//	InitBitMap(&bm);

	if (dword_16032 != 0) {
/*		// keep drawing spotlight in its final place
		_gfx->flatBlitCnv(&scnv, 110, 25, Gfx::kBitFront);
		BltBitMap(&bm, 0, 0, &_screen._bitMap, 110, 25, a3->??, a3->??, 0x20, 0x20);
*/		return;
	}

	_gfx->setHalfbriteMode(true);
/*
	// move spot light around the stage
	int d7, d6;
	for (d7 = 0; d7 < 150; d7++) {

		if (d7 < 100) {
			int d1 = d7;
			if (d1 < 0)
				d1++;

			d1 >>= 1;
			d6 = 50 - d1;
		} else {
			int d1 = d7 / 100;
			if (d1 < 0)
				d1++;

			d1 >>= 1;
			d6 = d1;
		}

		BltBitMap(&bm, 0, 0, &_screen._bitMap, d7+20, d6, a3->??, a3->??, 0x20, 0x20);
		sub_1590C(d6 + a3->??);
		BltBitMap(&bm, 0, 0, &_screen._bitMap, d7+20, d6, a3->??, a3->??, 0xFA, 0x20);
	}

	for (d7 = 50; d7 > -10; d7--) {
		BltBitMap(&bm, 0, 0, &_screen._bitMap, d7+120, d6, a3->??, a3->??, 0x20, 0x20);
		sub_1590C(d6 + a3->??);
		BltBitMap(&bm, 0, 0, &_screen._bitMap, d7+120, d6, a3->??, a3->??, 0xFA, 0x20);
	}

	BltBitMap(&bm, 0, 0, &_screen._bitMap, d7+120, d6, a3->??, a3->??, 0x20, 0x20);
	_gfx->flatBlitCnv(&scnv, d7+120, d6, Gfx::kBitFront);
*/

	dword_16032 = 1;
	return;
#endif
}

void Parallaction_ns::_c_HBOff(void*) {
#ifdef HALFBRITE
	_gfx->setHalfbriteMode(false);
#endif
}

void Parallaction_ns::_c_HBOn(void*) {
#ifdef HALFBRITE
	_gfx->setHalfbriteMode(true);
#endif
}


} // namespace Parallaction
