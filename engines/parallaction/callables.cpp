/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "parallaction/disk.h"
#include "parallaction/parallaction.h"
#include "parallaction/graphics.h"
#include "parallaction/inventory.h"
#include "parallaction/menu.h"
#include "parallaction/music.h"
#include "parallaction/zone.h"

#include "common/file.h"

namespace Parallaction {




static Zone *_moveSarcZones[5];
static Zone *_moveSarcExaZones[5];

void _c_null(void *parm) {

	return;
}

void _c_play_boogie(void *parm) {

	static uint16 flag = 1;

	if (flag == 0)
		return;
	flag = 0;

	stopMusic();
	loadMusic("boogie2");
	playMusic();

	return;
}


void _c_score(void *parm) {
	_score += 5;
	return;
}

void _c_fade(void *parm) {
	byte palette[PALETTE_SIZE];
	_vm->_graphics->getBlackPalette(palette);
	_vm->_graphics->setPalette(palette);

	_vm->_graphics->swapBuffers();

	for (uint16 _di = 0; _di < 64; _di++) {
		_vm->_graphics->fadePalette(palette);
		_vm->_graphics->setPalette(palette);
	}

	_vm->waitTime( 1 );

	return;
}

Zone *_moveSarcZone0 = NULL;
int16 _introSarcData1 = 0;
Zone *_moveSarcZone1 = NULL;

void _c_moveSarc(void *parm) {

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

		_introSarcData1 = _introSarcData3 - _moveSarcZone1->_limits._left;
		a->_z = _introSarcData3;
		a->_frame = _moveSarcZone1->_limits._top - (_introSarcData1 / 20);
		_introSarcData3 = _moveSarcZone1->_limits._left;

		if (_introSarcData1 > 0) {
			a->_zone.pos._position._x = _introSarcData1 / 2;
		} else {
			a->_zone.pos._position._x = -_introSarcData1 / 2;
		}

		if (_introSarcData1 > 0) {
			a->_zone.pos._position._y = 2;
		} else {
			a->_zone.pos._position._y = -2;
		}

		return;

	}

	_introSarcData2 = 1;

	_moveSarcZone1->_limits._right += _introSarcData1;
	_moveSarcZone1->_limits._left += _introSarcData1;

	_moveSarcZone1->_limits._top -= (_introSarcData1 / 20);
	_moveSarcZone1->_limits._bottom -= (_introSarcData1 / 20);

	_moveSarcZone0->_limits._right += _introSarcData1;
	_moveSarcZone0->_limits._left += _introSarcData1;

	_moveSarcZone0->_limits._top -= (_introSarcData1 / 20);
	_moveSarcZone0->_limits._bottom -= (_introSarcData1 / 20);

	if (_moveSarcZones[0]->_limits._left == 35 &&
		_moveSarcZones[1]->_limits._left == 68 &&
		_moveSarcZones[2]->_limits._left == 101 &&
		_moveSarcZones[3]->_limits._left == 134 &&
		_moveSarcZones[4]->_limits._left == 167) {

		a = findAnimation("finito");

		a->_zone._flags |= (kFlagsActive | kFlagsActing);
		_localFlags[_vm->_currentLocationIndex] |= 0x20;		// GROSS HACK: activates 'finito' flag in dinoit_museo.loc
	}

	return;

}


static uint16 num_foglie = 0;

void _c_contaFoglie(void *parm) {

	num_foglie++;
	if (num_foglie != 6)
		return;

	_commandFlags |= 0x1000;

	return;
}

void _c_zeroFoglie(void *parm) {
	num_foglie = 0;
	return;
}

void _c_trasformata(void *parm) {
	_engineFlags ^= kEngineMiniDonna;
	return;
}

void _c_offMouse(void *parm) {
	_mouseHidden = 1;
	_engineFlags |= kEngineMouse;
	return;
}

void _c_onMouse(void *parm) {
	_engineFlags &= ~kEngineMouse;
	_mouseHidden = 0;
	return;
}



void _c_setMask(void *parm) {

	_vm->_graphics->intGrottaHackMask();

	return;
}

void _c_endComment(void *parm) {

}

void _c_frankenstein(void *parm) {
	byte pal0[PALETTE_SIZE], pal1[PALETTE_SIZE];

	for (uint16 i = 0; i <= PALETTE_COLORS; i++) {
		pal0[i] = _palette[i];
		pal0[i*3+1] = 0;
		pal0[i*3+2] = 0;
	}

	_vm->_graphics->getBlackPalette(pal1);

	for (uint16 _di = 0; _di < 30; _di++) {
		g_system->delayMillis(20);
		_vm->_graphics->setPalette(pal0);
		g_system->delayMillis(20);
		_vm->_graphics->setPalette(pal1);
	}

	_vm->_graphics->setPalette(_palette);

	return;
}
// part completion messages
const char *endMsg0[] = {"COMPLIMENTI!", "BRAVO!", "CONGRATULATIONS!", "PRIMA!"};
const char *endMsg1[] = {"HAI FINITO QUESTA PARTE", "TU AS COMPLETE' CETTE AVENTURE", "YOU HAVE COMPLETED THIS PART", "DU HAST EIN ABENTEUER ERFOLGREICH"};
const char *endMsg2[] = {"ORA COMPLETA IL RESTO ", "AVEC SUCCES.",  "NOW GO ON WITH THE REST OF", "ZU ENDE GEFUHRT"};
const char *endMsg3[] = {"DELL' AVVENTURA",  "CONTINUE AVEC LES AUTRES", "THIS ADVENTURE", "MACH' MIT DEN ANDEREN WEITER"};
// game completion messages
const char *endMsg4[] = {"COMPLIMENTI!", "BRAVO!", "CONGRATULATIONS!", "PRIMA!"};
const char *endMsg5[] = {"HAI FINITO LE TRE PARTI", "TU AS COMPLETE' LES TROIS PARTIES", "YOU HAVE COMPLETED THE THREE PARTS", "DU HAST DREI ABENTEURE ERFOLGREICH"};
const char *endMsg6[] = {"DELL' AVVENTURA", "DE L'AVENTURE", "OF THIS ADVENTURE", "ZU ENDE GEFUHRT"};
const char *endMsg7[] = {"ED ORA IL GRAN FINALE ", "ET MAINTENANT LE GRAND FINAL", "NOW THE GREAT FINAL", "UND YETZT DER GROSSE SCHLUSS!"};


void _c_finito(void *parm) {

	const char **v8C = endMsg0;
	const char **v7C = endMsg1;
	const char **v6C = endMsg2;
	const char **v5C = endMsg3;
	const char **v4C = endMsg4;
	const char **v3C = endMsg5;
	const char **v2C = endMsg6;
	const char **v1C = endMsg7;

	Common::File stream;

	stream.open(_vm->_characterName, Common::File::kFileWriteMode);
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
	refreshInventory(_vm->_characterName);

	_vm->_graphics->palUnk0(_palette);

	if (gameCompleted) {
		_vm->_graphics->loadExternalCnv("slidecnv", &Graphics::_font);
		_vm->_graphics->_proportionalFont = false;
		uint16 _ax = _vm->_graphics->getStringWidth(v4C[_language]);
		_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 70, v4C[_language]);
		_ax = _vm->_graphics->getStringWidth(v3C[_language]);
		_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 100, v3C[_language]);
		_ax = _vm->_graphics->getStringWidth(v2C[_language]);
		_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 130, v2C[_language]);
		_ax = _vm->_graphics->getStringWidth(v1C[_language]);
		_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 160, v1C[_language]);

		_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBitBack);
		_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBit2);
		waitUntilLeftClick();

		strcpy(_location, "estgrotta.drki");

		_engineFlags |= kEngineChangeLocation;
		_vm->_graphics->freeCnv(&Graphics::_font);
	} else {
		_vm->_graphics->loadExternalCnv("slidecnv", &Graphics::_font);
		_vm->_graphics->_proportionalFont = false;
		uint16 _ax = _vm->_graphics->getStringWidth(v8C[_language]);
		_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 70, v8C[_language]);
		_ax = _vm->_graphics->getStringWidth(v7C[_language]);
		_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 100, v7C[_language]);
		_ax = _vm->_graphics->getStringWidth(v6C[_language]);
		_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 130, v6C[_language]);
		_ax = _vm->_graphics->getStringWidth(v5C[_language]);
		_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 160, v5C[_language]);

		_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBitBack);
		_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBit2);
		waitUntilLeftClick();

		_vm->_graphics->freeCnv(&Graphics::_font);
		_vm->_menu->selectCharacter();
	}

	removeNode(&_yourself._zone._node);
	_vm->_locationNames[0][0] = '\0';
	_vm->_numLocations = 0;
	_commandFlags = 0;

	_engineFlags |= kEngineQuit;

	freeZones(_zones._next);
	freeNodeList(_zones._next);
	_zones._next = NULL;

	freeZones(_animations._next);
	freeNodeList(_animations._next);
	_animations._next = NULL;

	_engineFlags &= ~kEngineQuit;

	addNode(&_animations, &_yourself._zone._node);
	_score = 0;

	return;
}

void _c_ridux(void *parm) {
	_vm->changeCharacter("minidino");
	return;
}

void _c_testResult(void *parm) {
	_vm->_graphics->swapBuffers();
	_vm->parseLocation("common");
	closeArchive();

	_vm->_graphics->loadExternalCnv("slidecnv", &Graphics::_font);
	_vm->_graphics->_proportionalFont = false;

	uint16 _ax = _vm->_graphics->getStringWidth(_slideText[0]);
	_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 38, _slideText[0]);
	_ax = _vm->_graphics->getStringWidth(_slideText[1]);
	_vm->_graphics->displayString((SCREEN_WIDTH - _ax)/2, 58, _slideText[1]);

	_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBitBack);
	_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBit2);

	_vm->_graphics->freeCnv(&Graphics::_font);

	return;
}

} // namespace Parallaction
