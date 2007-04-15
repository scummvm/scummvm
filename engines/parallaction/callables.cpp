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


#include "common/stdafx.h"
#include "common/system.h"

#include "common/file.h"

#include "parallaction/disk.h"
#include "parallaction/parallaction.h"
#include "parallaction/graphics.h"
#include "parallaction/inventory.h"
#include "parallaction/menu.h"
#include "parallaction/music.h"
#include "parallaction/zone.h"

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

	_vm->_midiPlayer->play("boogie2");

	return;
}


void _c_score(void *parm) {
	_score += 5;
	return;
}

void _c_fade(void *parm) {
	_vm->_gfx->setBlackPalette();

	_vm->_gfx->swapBuffers();

	Gfx::Palette pal;
	for (uint16 _di = 0; _di < 64; _di++) {
		_vm->_gfx->fadePalette(pal);
		_vm->_gfx->setPalette(pal);
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

			_moveSarcZones[0] = _vm->findZone("sarc1");
			_moveSarcZones[1] = _vm->findZone("sarc2");
			_moveSarcZones[2] = _vm->findZone("sarc3");
			_moveSarcZones[3] = _vm->findZone("sarc4");
			_moveSarcZones[4] = _vm->findZone("sarc5");

			_moveSarcExaZones[0] = _vm->findZone("sarc1exa");
			_moveSarcExaZones[1] = _vm->findZone("sarc2exa");
			_moveSarcExaZones[2] = _vm->findZone("sarc3exa");
			_moveSarcExaZones[3] = _vm->findZone("sarc4exa");
			_moveSarcExaZones[4] = _vm->findZone("sarc5exa");

		}

		a = _vm->findAnimation("sposta");

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

		a = _vm->findAnimation("finito");

		a->_flags |= (kFlagsActive | kFlagsActing);
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
	_engineFlags ^= kEngineTransformedDonna;
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

	_vm->_gfx->intGrottaHackMask();

	return;
}

void _c_endComment(void *param) {

	byte* _enginePal = _vm->_gfx->_palette;
	Gfx::Palette pal;

	uint32 si;
	for (si = 0; si < 32; si++) {

		byte al = _enginePal[si*3+1];
		if (al > _enginePal[si*3+2]) {
			al = _enginePal[si*3+1];
		} else {
			al = _enginePal[si*3+2];
		}

		if (al < _enginePal[si*3]) {
			al = _enginePal[si*3];
		} else {
			al = _enginePal[si*3+1];
		}

		if (al > _enginePal[si*3+2]) {
			al = _enginePal[si*3+1];
		} else {
			al = _enginePal[si*3+2];
		}

		pal[si*3] = al;
		pal[si*3+2] = al;
		pal[si*3+1] = al;

	}

	int16 w = 0, h = 0;
	_vm->_gfx->getStringExtent(_vm->_location._endComment, 130, &w, &h);

	Common::Rect r(w+5, h+5);
	r.moveTo(5, 5);
	_vm->_gfx->floodFill(Gfx::kBitFront, r, 0);

	r.setWidth(w+3);
	r.setHeight(w+4);
	r.moveTo(7, 7);
	_vm->_gfx->floodFill(Gfx::kBitFront, r, 1);

	_vm->_gfx->setFont(kFontDialogue);
	_vm->_gfx->displayWrappedString(_vm->_location._endComment, 3, 5, 130, 0);

	uint32 di = 0;
	for (di = 0; di < PALETTE_COLORS; di++) {
		for (si = 0; si <= 93; si +=3) {

			char al;

			if (_enginePal[si] != pal[si]) {
				al = _enginePal[si];
				if (al > pal[si])
					al = 1;
				else
					al = -1;
				_enginePal[si] += al;
			}

			if (_enginePal[si+1] != pal[si+1]) {
				al = _enginePal[si+1];
				if (al > pal[si+1])
					al = 1;
				else
					al = -1;
				_enginePal[si+1] += al;
			}

			if (_enginePal[si+2] != pal[si+2]) {
				al = _enginePal[si+2];
				if (al > pal[si+2])
					al = 1;
				else
					al = -1;
				_enginePal[si+2] += al;
			}

		}

		_vm->_gfx->setPalette(_enginePal);
	}

	waitUntilLeftClick();

	return;
}

void _c_frankenstein(void *parm) {
	Gfx::Palette pal0;

	for (uint16 i = 0; i <= BASE_PALETTE_COLORS; i++) {
		pal0[(i+FIRST_BASE_COLOR)] = _vm->_gfx->_palette[i];
		pal0[(i+FIRST_BASE_COLOR)*3+1] = 0;
		pal0[(i+FIRST_BASE_COLOR)*3+2] = 0;
		pal0[(i+FIRST_EHB_COLOR)*3+1] = 0;
		pal0[(i+FIRST_EHB_COLOR)*3+2] = 0;
	}

	for (uint16 _di = 0; _di < 30; _di++) {
		g_system->delayMillis(20);
		_vm->_gfx->setPalette(pal0, FIRST_BASE_COLOR, BASE_PALETTE_COLORS);
		g_system->delayMillis(20);
		_vm->_gfx->setPalette(pal0, FIRST_EHB_COLOR, EHB_PALETTE_COLORS);
	}

	_vm->_gfx->setPalette(_vm->_gfx->_palette);

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

	_vm->_gfx->extendPalette(_vm->_gfx->_palette);

	if (gameCompleted) {
		_vm->_gfx->setFont(kFontMenu);
		_vm->_gfx->_proportionalFont = false;
		uint16 _ax = _vm->_gfx->getStringWidth(v4C[_language]);
		_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 70, v4C[_language]);
		_ax = _vm->_gfx->getStringWidth(v3C[_language]);
		_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 100, v3C[_language]);
		_ax = _vm->_gfx->getStringWidth(v2C[_language]);
		_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 130, v2C[_language]);
		_ax = _vm->_gfx->getStringWidth(v1C[_language]);
		_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 160, v1C[_language]);

		_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);
		_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBit2);
		waitUntilLeftClick();

		strcpy(_vm->_location._name, "estgrotta.drki");

		_engineFlags |= kEngineChangeLocation;
	} else {
		_vm->_gfx->setFont(kFontMenu);
		_vm->_gfx->_proportionalFont = false;
		uint16 _ax = _vm->_gfx->getStringWidth(v8C[_language]);
		_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 70, v8C[_language]);
		_ax = _vm->_gfx->getStringWidth(v7C[_language]);
		_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 100, v7C[_language]);
		_ax = _vm->_gfx->getStringWidth(v6C[_language]);
		_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 130, v6C[_language]);
		_ax = _vm->_gfx->getStringWidth(v5C[_language]);
		_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 160, v5C[_language]);

		_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);
		_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBit2);
		waitUntilLeftClick();

		_vm->_menu->selectCharacter();
	}

	// this code saves main character animation from being removed from the following code
	_vm->_animations.remove(&_vm->_char._ani);
	_vm->_locationNames[0][0] = '\0';
	_vm->_numLocations = 0;
	_commandFlags = 0;

	// this flag tells freeZones to unconditionally remove *all* Zones
	_engineFlags |= kEngineQuit;

	// TODO (LIST): this sequence should be just _zones.clear()
	_vm->freeZones();

	// TODO (LIST): this sequence should be just _animations.clear()
	_vm->freeAnimations();

	// this dangerous flag can now be cleared
	_engineFlags &= ~kEngineQuit;

	// main character animation is restored
	_vm->_animations.push_front(&_vm->_char._ani);
	_score = 0;

	return;
}

void _c_ridux(void *parm) {
	_vm->changeCharacter(_minidinoName);
	return;
}

void _c_testResult(void *parm) {
	_vm->_gfx->swapBuffers();
	_vm->parseLocation("common");

	_vm->_gfx->setFont(kFontMenu);
	_vm->_gfx->_proportionalFont = false;

	uint16 _ax = _vm->_gfx->getStringWidth(_slideText[0]);
	_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 38, _slideText[0]);
	_ax = _vm->_gfx->getStringWidth(_slideText[1]);
	_vm->_gfx->displayString((SCREEN_WIDTH - _ax)/2, 58, _slideText[1]);

	_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);
	_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBit2);

	return;
}

} // namespace Parallaction
