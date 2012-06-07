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
 */

#include "common/events.h"

#include "gob/global.h"
#include "gob/util.h"
#include "gob/palanim.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/decfile.h"
#include "gob/cmpfile.h"
#include "gob/anifile.h"
#include "gob/aniobject.h"

#include "gob/sound/sound.h"

#include "gob/minigames/geisha/penetration.h"
#include "gob/minigames/geisha/meter.h"
#include "gob/minigames/geisha/mouth.h"

namespace Gob {

namespace Geisha {

static const int kColorShield    = 11;
static const int kColorHealth    = 15;
static const int kColorBlack     = 10;
static const int kColorFloor     = 13;
static const int kColorFloorText = 14;
static const int kColorExitText  = 15;

enum Sprite {
	kSpriteFloorShield = 25,
	kSpriteExit        = 29,
	kSpriteFloor       = 30,
	kSpriteWall        = 31,
	kSpriteMouthBite   = 32,
	kSpriteMouthKiss   = 33
};

enum Animation {
	kAnimationMouthKiss = 33,
	kAnimationMouthBite = 34
};

static const int kMapTileWidth  = 24;
static const int kMapTileHeight = 24;

static const int kPlayAreaX      = 120;
static const int kPlayAreaY      =   7;
static const int kPlayAreaWidth  = 192;
static const int kPlayAreaHeight = 113;

static const int kPlayAreaBorderWidth  = kPlayAreaWidth  / 2;
static const int kPlayAreaBorderHeight = kPlayAreaHeight / 2;

static const int kTextAreaLeft   =   9;
static const int kTextAreaTop    =   7;
static const int kTextAreaRight  = 104;
static const int kTextAreaBottom = 107;

static const int kTextAreaBigBottom = 142;

const byte Penetration::kPalettes[kFloorCount][3 * kPaletteSize] = {
	{
		0x16, 0x16, 0x16,
		0x12, 0x14, 0x16,
		0x34, 0x00, 0x25,
		0x1D, 0x1F, 0x22,
		0x24, 0x27, 0x2A,
		0x2C, 0x0D, 0x22,
		0x2B, 0x2E, 0x32,
		0x12, 0x09, 0x20,
		0x3D, 0x3F, 0x00,
		0x3F, 0x3F, 0x3F,
		0x00, 0x00, 0x00,
		0x15, 0x15, 0x3F,
		0x25, 0x22, 0x2F,
		0x1A, 0x14, 0x28,
		0x3F, 0x00, 0x00,
		0x15, 0x3F, 0x15
	},
	{
		0x16, 0x16, 0x16,
		0x12, 0x14, 0x16,
		0x37, 0x00, 0x24,
		0x1D, 0x1F, 0x22,
		0x24, 0x27, 0x2A,
		0x30, 0x0E, 0x16,
		0x2B, 0x2E, 0x32,
		0x22, 0x0E, 0x26,
		0x3D, 0x3F, 0x00,
		0x3F, 0x3F, 0x3F,
		0x00, 0x00, 0x00,
		0x15, 0x15, 0x3F,
		0x36, 0x28, 0x36,
		0x30, 0x1E, 0x2A,
		0x3F, 0x00, 0x00,
		0x15, 0x3F, 0x15
	},
	{
		0x16, 0x16, 0x16,
		0x12, 0x14, 0x16,
		0x3F, 0x14, 0x22,
		0x1D, 0x1F, 0x22,
		0x24, 0x27, 0x2A,
		0x30, 0x10, 0x10,
		0x2B, 0x2E, 0x32,
		0x2A, 0x12, 0x12,
		0x3D, 0x3F, 0x00,
		0x3F, 0x3F, 0x3F,
		0x00, 0x00, 0x00,
		0x15, 0x15, 0x3F,
		0x3F, 0x23, 0x31,
		0x39, 0x20, 0x2A,
		0x3F, 0x00, 0x00,
		0x15, 0x3F, 0x15
	}
};

const byte Penetration::kMaps[kModeCount][kFloorCount][kMapWidth * kMapHeight] = {
	{
		{ // Real mode, floor 0
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,
			50, 50,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50, 50,
			50,  0,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0,  0, 50,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50,  0,  0, 50, 50, 50, 50,  0, 54, 55,  0,  0, 50,  0, 50,
			50,  0, 50, 49,  0, 50,  0, 52, 53,  0, 50, 50, 50,  0,  0,  0, 50,
			50, 57,  0, 50,  0,  0,  0, 50, 50, 50,  0,  0, 56, 50, 54, 55, 50,
			50, 50,  0,  0, 50, 50, 50,  0,  0,  0,  0, 50,  0,  0, 50,  0, 50,
			50, 51, 50,  0, 54, 55,  0,  0, 50, 50, 50, 50, 52, 53, 50,  0, 50,
			50,  0, 50,  0,  0,  0,  0,  0, 54, 55,  0,  0,  0, 50,  0,  0, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0,  0, 50,
			50, 50,  0, 52, 53,  0,  0,  0,  0,  0,  0, 52, 53,  0,  0, 50, 50,
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0
		},
		{ // Real mode, floor 1
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,
			50,  0, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0, 50, 51, 52, 53,  0,  0, 52, 53,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50,  0, 50, 50,  0, 50,  0, 50,  0, 50, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 52, 53,  0,  0,  0,  0,  0, 52, 53,  0, 52, 53, 50,
			50, 57, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0, 50, 52, 53,  0,  0, 52, 53,  0,  0,  0,  0,  0, 54, 55, 50,
			50,  0, 50,  0, 50,  0, 50, 50,  0, 50, 50,  0, 50,  0, 50, 50, 50,
			50,  0, 50, 49,  0,  0, 52, 53,  0, 52, 53,  0,  0,  0, 50, 56, 50,
			50,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0,  0, 50,
			50,  0,  0,  0,  0,  0,  0,  0, 54, 55,  0,  0,  0,  0,  0,  0, 50,
			 0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0
		},
		{ // Real mode, floor 2
			 0, 50, 50, 50, 50, 50, 50, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0,
			50, 52, 53,  0,  0,  0,  0, 50, 50, 50,  0,  0,  0,  0, 52, 53, 50,
			50,  0, 50, 50, 50,  0,  0,  0, 50,  0,  0,  0, 50, 50, 50,  0, 50,
			50,  0, 50, 52, 53, 50, 50, 52, 53,  0, 50, 50, 54, 55, 50,  0, 50,
			50,  0, 50,  0,  0,  0,  0, 50,  0, 50,  0,  0,  0,  0, 50,  0, 50,
			50,  0,  0,  0, 50,  0,  0,  0, 50,  0,  0,  0, 50,  0, 52, 53, 50,
			 0, 50,  0, 50, 50, 50,  0, 57, 50, 51,  0, 50, 50, 50,  0, 50,  0,
			50,  0,  0,  0, 50,  0,  0,  0, 50,  0, 52, 53, 50,  0,  0,  0, 50,
			50,  0, 50,  0,  0,  0,  0, 50, 56, 50,  0,  0,  0,  0, 50,  0, 50,
			50,  0, 50, 54, 55, 50, 50,  0,  0,  0, 50, 50, 54, 55, 50,  0, 50,
			50,  0, 50, 50, 50,  0,  0,  0, 50,  0,  0,  0, 50, 50, 50,  0, 50,
			50, 52, 53,  0,  0,  0,  0, 50, 50, 50,  0,  0,  0,  0, 52, 53, 50,
			 0, 50, 50, 50, 50, 50, 50, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0
		}
	},
	{
		{ // Test mode, floor 0
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 56,  0, 50,  0,  0, 52, 53,  0,  0,  0,  0, 52, 53,  0, 51, 50,
			50,  0,  0, 50,  0,  0,  0, 50,  0, 54, 55, 50,  0, 50, 50, 50, 50,
			50, 52, 53, 50, 50,  0,  0, 50, 50, 50, 50, 50,  0, 50,  0,  0, 50,
			50,  0,  0,  0,  0, 56,  0,  0,  0,  0,  0, 50, 49, 50,  0,  0, 50,
			50,  0, 54, 55,  0, 50, 50, 54, 55,  0, 50, 50, 50,  0,  0,  0, 50,
			50,  0,  0,  0,  0,  0,  0,  0,  0,  0, 52, 53,  0,  0, 54, 55, 50,
			50,  0, 50,  0, 50,  0,  0, 50,  0,  0,  0, 50,  0,  0,  0,  0, 50,
			50,  0, 50,  0, 50, 54, 55, 50,  0, 50, 50, 50,  0, 50,  0,  0, 50,
			50, 50, 50, 50, 50,  0,  0, 50,  0,  0,  0,  0,  0, 50, 54, 55, 50,
			50,  0,  0,  0,  0,  0,  0,  0, 50, 50, 50, 50, 50,  0,  0,  0, 50,
			50, 57,  0, 52, 53,  0,  0,  0,  0, 54, 55,  0,  0,  0,  0, 56, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		},
		{ // Test mode, floor 1
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 52, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 50,
			50,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 54, 55,  0, 50,
			50,  0, 50, 52, 53,  0,  0, 50,  0,  0, 54, 55, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0,  0, 52, 53,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50, 50, 50, 50, 50, 49, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0,  0, 50, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50, 51,  0,  0, 52, 53, 50,  0, 50,  0, 50,
			50, 57, 50,  0, 50,  0, 50, 50, 50, 50, 50, 50, 50,  0, 50,  0, 50,
			50, 50, 50,  0, 50, 56,  0,  0,  0, 54, 55,  0,  0,  0, 50,  0, 50,
			50, 56,  0,  0,  0, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,  0, 50,
			50, 50, 50, 50,  0,  0,  0,  0, 52, 53,  0,  0,  0,  0,  0,  0, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		},
		{ // Test mode, floor 2
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
			50, 57, 50, 54, 55,  0, 50, 54, 55,  0, 50,  0, 52, 53, 50, 51, 50,
			50,  0, 50,  0, 50,  0, 50,  0,  0,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50, 52, 53,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,  0, 50,
			50,  0,  0,  0, 50,  0, 50,  0, 50,  0,  0,  0, 50,  0, 50,  0, 50,
			50,  0,  0,  0, 50, 52, 53,  0, 50, 52, 53, 56, 50,  0, 54, 55, 50,
			50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
		}
	}
};

static const int kLanguageCount    = 5;
static const int kFallbackLanguage = 2; // English

enum String {
	kString3rdBasement = 0,
	kString2ndBasement,
	kString1stBasement,
	kStringNoExit,
	kStringYouHave,
	kString2Exits,
	kString1Exit,
	kStringToReach,
	kStringUpperLevel1,
	kStringUpperLevel2,
	kStringLevel0,
	kStringPenetration,
	kStringSuccessful,
	kStringDanger,
	kStringGynoides,
	kStringActivated,
	kStringCount
};

static const char *kStrings[kLanguageCount][kStringCount] = {
	{ // French
		"3EME SOUS-SOL",
		"2EME SOUS-SOL",
		"1ER SOUS-SOL",
		"SORTIE REFUSEE",
		"Vous disposez",
		"de deux sorties",
		"d\'une sortie",
		"pour l\'acc\212s au",
		"niveau",
		"sup\202rieur",
		"- NIVEAU 0 -",
		"PENETRATION",
		"REUSSIE",
		"DANGER",
		"GYNOIDES",
		"ACTIVEES"
	},
	{ // German
		// NOTE: The original had very broken German there. We provide proper(ish) German instead.
		//       B0rken text in the comments after each line
		"3. UNTERGESCHOSS", // "3. U.-GESCHOSS""
		"2. UNTERGESCHOSS", // "2. U.-GESCHOSS"
		"1. UNTERGESCHOSS", // "1. U.-GESCHOSS"
		"AUSGANG GESPERRT",
		"Sie haben",
		"zwei Ausg\204nge", // "zwei Ausgang"
		"einen Ausgang",    // "Fortsetztung"
		"um das obere",     // ""
		"Stockwerk zu",     // ""
		"erreichen",        // ""
		"- STOCKWERK 0 -",  // "0 - HOHE"
		"PENETRATION",      // "DURCHDRIGEN"
		"ERFOLGREICH",      // "ERFOLG"
		"GEFAHR",
		"GYNOIDE",
		"AKTIVIERT",
	},
	{ // English
		"3RD BASEMENT",
		"2ND BASEMENT",
		"1ST BASEMENT",
		"NO EXIT",
		"You have",
		"2 exits",
		"1 exit",
		"to reach upper",
		"level",
		"",
		"- 0 LEVEL -",
		"PENETRATION",
		"SUCCESSFUL",
		"DANGER",
		"GYNOIDES",
		"ACTIVATED",
	},
	{ // Spanish
		"3ER. SUBSUELO",
		"2D. SUBSUELO",
		"1ER. SUBSUELO",
		"SALIDA RECHAZADA",
		"Dispones",
		"de dos salidas",
		"de una salida",
		"para acceso al",
		"nivel",
		"superior",
		"- NIVEL 0 -",
		"PENETRACION",
		"CONSEGUIDA",
		"PELIGRO",
		"GYNOIDAS",
		"ACTIVADAS",
	},
	{ // Italian
		"SOTTOSUOLO 3",
		"SOTTOSUOLO 2",
		"SOTTOSUOLO 1",
		"NON USCITA",
		"avete",
		"due uscite",
		"un\' uscita",
		"per accedere al",
		"livello",
		"superiore",
		"- LIVELLO 0 -",
		"PENETRAZIONE",
		"RIUSCITA",
		"PERICOLO",
		"GYNOIDI",
		"ATTIVATE",
	}
};

Penetration::Position::Position(uint16 pX, uint16 pY) : x(pX), y(pY) {
}


Penetration::ManagedMouth::ManagedMouth(uint16 pX, uint16 pY, MouthType t) :
	Position(pX, pY), mouth(0), type(t) {
}

Penetration::ManagedMouth::~ManagedMouth() {
	delete mouth;
}


Penetration::ManagedSub::ManagedSub(uint16 pX, uint16 pY) : Position(pX, pY), sub(0) {
	mapX = x * kMapTileWidth;
	mapY = y * kMapTileHeight;
}

Penetration::ManagedSub::~ManagedSub() {
	delete sub;
}


Penetration::Penetration(GobEngine *vm) : _vm(vm), _background(0), _sprites(0), _objects(0), _sub(0),
	_shieldMeter(0), _healthMeter(0), _floor(0) {

	_background = new Surface(320, 200, 1);

	_shieldMeter = new Meter(11, 119, 92, 3, kColorShield, kColorBlack, 920, Meter::kFillToRight);
	_healthMeter = new Meter(11, 137, 92, 3, kColorHealth, kColorBlack, 920, Meter::kFillToRight);

	_map = new Surface(kMapWidth  * kMapTileWidth  + kPlayAreaWidth ,
	                   kMapHeight * kMapTileHeight + kPlayAreaHeight, 1);
}

Penetration::~Penetration() {
	deinit();

	delete _map;

	delete _shieldMeter;
	delete _healthMeter;

	delete _background;
}

bool Penetration::play(bool hasAccessPass, bool hasMaxEnergy, bool testMode) {
	_hasAccessPass = hasAccessPass;
	_hasMaxEnergy  = hasMaxEnergy;
	_testMode      = testMode;

	init();
	initScreen();

	drawFloorText();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();

	while (!_vm->shouldQuit() && !_quit && !isDead() && !hasWon()) {
		updateAnims();

		// Draw, fade in if necessary and wait for the end of the frame
		_vm->_draw->blitInvalidated();
		fadeIn();
		_vm->_util->waitEndFrame();

		// Handle the input
		checkInput();

		// Handle the sub movement
		handleSub();

		checkExited();
	}

	deinit();
	drawEndText();

	return hasWon();
}

void Penetration::init() {
	// Load sounds
	_vm->_sound->sampleLoad(&_soundShield, SOUND_SND, "boucl.snd");
	_vm->_sound->sampleLoad(&_soundBite  , SOUND_SND, "pervet.snd");
	_vm->_sound->sampleLoad(&_soundKiss  , SOUND_SND, "baise.snd");
	_vm->_sound->sampleLoad(&_soundShoot , SOUND_SND, "tirgim.snd");
	_vm->_sound->sampleLoad(&_soundExit  , SOUND_SND, "trouve.snd");

	_quit = false;
	for (int i = 0; i < kKeyCount; i++)
		_keys[i] = false;

	_background->clear();

	_vm->_video->drawPackedSprite("hyprmef2.cmp", *_background);

	_sprites = new CMPFile(_vm, "tcifplai.cmp", 320, 200);
	_objects = new ANIFile(_vm, "tcite.ani", 320);

	// The shield starts down
	_shieldMeter->setValue(0);

	// If we don't have the max energy tokens, the health starts at 1/3 strength
	if (_hasMaxEnergy)
		_healthMeter->setMaxValue();
	else
		_healthMeter->setValue(_healthMeter->getMaxValue() / 3);

	_floor = 0;

	createMap();
}

void Penetration::deinit() {
	_soundShield.free();
	_soundBite.free();
	_soundKiss.free();
	_soundShoot.free();
	_soundExit.free();

	clearMap();

	delete _objects;
	delete _sprites;

	_objects = 0;
	_sprites = 0;
}

void Penetration::clearMap() {
	_mapAnims.clear();
	_anims.clear();

	_exits.clear();
	_shields.clear();
	_mouths.clear();

	delete _sub;

	_sub = 0;

	_map->fill(kColorBlack);
}

void Penetration::createMap() {
	if (_floor >= kFloorCount)
		error("Geisha: Invalid floor %d in minigame penetration", _floor);

	clearMap();

	const byte *mapTiles = kMaps[_testMode ? 1 : 0][_floor];

	bool exitWorks;

	// Draw the map tiles
	for (int y = 0; y < kMapHeight; y++) {
		for (int x = 0; x < kMapWidth; x++) {
			const byte mapTile = mapTiles[y * kMapWidth + x];

			bool *walkMap = _walkMap + (y * kMapWidth + x);

			const int posX = kPlayAreaBorderWidth  + x * kMapTileWidth;
			const int posY = kPlayAreaBorderHeight + y * kMapTileHeight;

			*walkMap = true;

			switch (mapTile) {
			case 0: // Floor
				_sprites->draw(*_map, kSpriteFloor, posX, posY);
				break;

			case 49: // Emergency exit (needs access pass)

				exitWorks = _hasAccessPass;
				if (exitWorks) {
					_exits.push_back(Position(x, y));
					_sprites->draw(*_map, kSpriteExit, posX, posY);
				} else {
					_sprites->draw(*_map, kSpriteWall, posX, posY);
					*walkMap = false;
				}

				break;

			case 50: // Wall
				_sprites->draw(*_map, kSpriteWall, posX, posY);
				*walkMap = false;
				break;

			case 51: // Regular exit

				// A regular exit works always in test mode.
				// But if we're in real mode, and on the last floor, it needs an access pass
				exitWorks = _testMode || (_floor < 2) || _hasAccessPass;

				if (exitWorks) {
					_exits.push_back(Position(x, y));
					_sprites->draw(*_map, kSpriteExit, posX, posY);
				} else {
					_sprites->draw(*_map, kSpriteWall, posX, posY);
					*walkMap = false;
				}

				break;

			case 52: // Left side of biting mouth
				_mouths.push_back(ManagedMouth(x, y, kMouthTypeBite));

				_mouths.back().mouth =
					new Mouth(*_objects, *_sprites, kAnimationMouthBite, kSpriteMouthBite, kSpriteFloor);

				_mouths.back().mouth->setPosition(posX, posY);
				break;

			case 53: // Right side of biting mouth
				break;

			case 54: // Left side of kissing mouth
				_mouths.push_back(ManagedMouth(x, y, kMouthTypeKiss));

				_mouths.back().mouth =
					new Mouth(*_objects, *_sprites, kAnimationMouthKiss, kSpriteMouthKiss, kSpriteFloor);

				_mouths.back().mouth->setPosition(posX, posY);
				break;

			case 55: // Right side of kissing mouth
				break;

			case 56: // Shield lying on the floor
				_sprites->draw(*_map, kSpriteFloor      , posX    , posY    ); // Floor
				_sprites->draw(*_map, kSpriteFloorShield, posX + 4, posY + 8); // Shield

				_map->fillRect(posX +  4, posY + 8, posX +  7, posY + 18, kColorFloor); // Area left to shield
				_map->fillRect(posX + 17, posY + 8, posX + 20, posY + 18, kColorFloor); // Area right to shield

				_shields.push_back(Position(x, y));
				break;

			case 57: // Start position
				_sprites->draw(*_map, kSpriteFloor, posX, posY);

				delete _sub;

				_sub = new ManagedSub(x, y);

				_sub->sub = new Submarine(*_objects);
				_sub->sub->setPosition(kPlayAreaX + kPlayAreaBorderWidth, kPlayAreaY + kPlayAreaBorderHeight);
				break;
			}
		}
	}

	if (!_sub)
		error("Geisha: No starting position in floor %d (testmode: %d)", _floor, _testMode);

	for (Common::List<ManagedMouth>::iterator m = _mouths.begin(); m != _mouths.end(); m++)
		_mapAnims.push_back(m->mouth);

	_anims.push_back(_sub->sub);
}

void Penetration::drawFloorText() {
	_vm->_draw->_backSurface->fillRect(kTextAreaLeft, kTextAreaTop, kTextAreaRight, kTextAreaBottom, kColorBlack);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, kTextAreaLeft, kTextAreaTop, kTextAreaRight, kTextAreaBottom);

	const Font *font = _vm->_draw->_fonts[2];
	if (!font)
		return;

	const char **strings = kStrings[getLanguage()];

	const char *floorString = 0;
	if      (_floor == 0)
		floorString = strings[kString3rdBasement];
	else if (_floor == 1)
		floorString = strings[kString2ndBasement];
	else if (_floor == 2)
		floorString = strings[kString1stBasement];

	if (floorString)
		_vm->_draw->drawString(floorString, 10, 15, kColorFloorText, kColorBlack, 1,
		                       *_vm->_draw->_backSurface, *font);

	if (_exits.size() > 0) {
		int exitCount = kString2Exits;
		if (_exits.size() == 1)
			exitCount = kString1Exit;

		_vm->_draw->drawString(strings[kStringYouHave]    , 10, 38, kColorExitText, kColorBlack, 1,
		                       *_vm->_draw->_backSurface, *font);
		_vm->_draw->drawString(strings[exitCount]         , 10, 53, kColorExitText, kColorBlack, 1,
		                       *_vm->_draw->_backSurface, *font);
		_vm->_draw->drawString(strings[kStringToReach]    , 10, 68, kColorExitText, kColorBlack, 1,
		                       *_vm->_draw->_backSurface, *font);
		_vm->_draw->drawString(strings[kStringUpperLevel1], 10, 84, kColorExitText, kColorBlack, 1,
		                       *_vm->_draw->_backSurface, *font);
		_vm->_draw->drawString(strings[kStringUpperLevel2], 10, 98, kColorExitText, kColorBlack, 1,
		                       *_vm->_draw->_backSurface, *font);

	} else
		_vm->_draw->drawString(strings[kStringNoExit], 10, 53, kColorExitText, kColorBlack, 1,
		                       *_vm->_draw->_backSurface, *font);
}

void Penetration::drawEndText() {
	// Only draw the end text when we've won and this isn't a test run
	if (!hasWon() || _testMode)
		return;

	_vm->_draw->_backSurface->fillRect(kTextAreaLeft, kTextAreaTop, kTextAreaRight, kTextAreaBigBottom, kColorBlack);

	const Font *font = _vm->_draw->_fonts[2];
	if (!font)
		return;

	const char **strings = kStrings[getLanguage()];

	_vm->_draw->drawString(strings[kStringLevel0]     , 11, 21, kColorExitText, kColorBlack, 1,
	                       *_vm->_draw->_backSurface, *font);
	_vm->_draw->drawString(strings[kStringPenetration], 11, 42, kColorExitText, kColorBlack, 1,
	                       *_vm->_draw->_backSurface, *font);
	_vm->_draw->drawString(strings[kStringSuccessful] , 11, 58, kColorExitText, kColorBlack, 1,
	                       *_vm->_draw->_backSurface, *font);

	_vm->_draw->drawString(strings[kStringDanger]   , 11,  82, kColorFloorText, kColorBlack, 1,
	                       *_vm->_draw->_backSurface, *font);
	_vm->_draw->drawString(strings[kStringGynoides] , 11,  98, kColorFloorText, kColorBlack, 1,
	                       *_vm->_draw->_backSurface, *font);
	_vm->_draw->drawString(strings[kStringActivated], 11, 113, kColorFloorText, kColorBlack, 1,
	                       *_vm->_draw->_backSurface, *font);

	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, kTextAreaLeft, kTextAreaTop, kTextAreaRight, kTextAreaBigBottom);
	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();
}

void Penetration::fadeIn() {
	if (!_needFadeIn)
		return;

	// Fade to palette
	_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
	_needFadeIn = false;
}

void Penetration::setPalette() {
	// Fade to black
	_vm->_palAnim->fade(0, 0, 0);

	// Set palette
	memcpy(_vm->_draw->_vgaPalette     , kPalettes[_floor], 3 * kPaletteSize);
	memcpy(_vm->_draw->_vgaSmallPalette, kPalettes[_floor], 3 * kPaletteSize);

	_needFadeIn = true;
}

void Penetration::initScreen() {
	_vm->_util->setFrameRate(15);

	setPalette();

	// Draw the shield meter
	_sprites->draw(*_background,   0,   0,  95,   6, 9, 117, 0); // Meter frame
	_sprites->draw(*_background, 271, 176, 282, 183, 9, 108, 0); // Shield

	// Draw the health meter
	_sprites->draw(*_background,   0,   0,  95,   6, 9, 135, 0); // Meter frame
	_sprites->draw(*_background, 283, 176, 292, 184, 9, 126, 0); // Heart

	_vm->_draw->_backSurface->blit(*_background);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, 0, 0, 319, 199);
}

void Penetration::checkInput() {
	Common::Event event;
	Common::EventManager *eventMan = g_system->getEventManager();

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if      (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				_quit = true;
			else if (event.kbd.keycode == Common::KEYCODE_UP)
				_keys[kKeyUp   ] = true;
			else if (event.kbd.keycode == Common::KEYCODE_DOWN)
				_keys[kKeyDown ] = true;
			else if (event.kbd.keycode == Common::KEYCODE_LEFT)
				_keys[kKeyLeft ] = true;
			else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
				_keys[kKeyRight] = true;
			else if (event.kbd.keycode == Common::KEYCODE_SPACE)
				_keys[kKeySpace] = true;
			break;

		case Common::EVENT_KEYUP:
			if      (event.kbd.keycode == Common::KEYCODE_UP)
				_keys[kKeyUp   ] = false;
			else if (event.kbd.keycode == Common::KEYCODE_DOWN)
				_keys[kKeyDown ] = false;
			else if (event.kbd.keycode == Common::KEYCODE_LEFT)
				_keys[kKeyLeft ] = false;
			else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
				_keys[kKeyRight] = false;
			else if (event.kbd.keycode == Common::KEYCODE_SPACE)
				_keys[kKeySpace] = false;
			break;

		default:
			break;
		}
	}
}

bool Penetration::isWalkable(int16 x, int16 y) const {
	if ((x < 0) || (x >= kMapWidth) || (y < 0) || (y >= kMapHeight))
		return false;

	return _walkMap[y * kMapWidth + x];
}

void Penetration::handleSub() {
	int x, y;
	Submarine::Direction direction = getDirection(x, y);

	subMove(x, y, direction);

	if (_keys[kKeySpace])
		subShoot();
}

void Penetration::subMove(int x, int y, Submarine::Direction direction) {
	if (!_sub->sub->canMove())
		return;

	// Limit the movement to walkable tiles

	int16 minX = 0;
	if (!isWalkable(_sub->x - 1, _sub->y))
		minX = _sub->x * kMapTileWidth;

	int16 maxX = kMapWidth * kMapTileWidth;
	if (!isWalkable(_sub->x + 1, _sub->y))
		maxX = _sub->x * kMapTileWidth;

	int16 minY = 0;
	if (!isWalkable(_sub->x, _sub->y - 1))
		minY = _sub->y * kMapTileHeight;

	int16 maxY = kMapHeight * kMapTileHeight;
	if (!isWalkable(_sub->x, _sub->y + 1))
		maxY = _sub->y * kMapTileHeight;

	_sub->mapX = CLIP<int16>(_sub->mapX + x, minX, maxX);
	_sub->mapY = CLIP<int16>(_sub->mapY + y, minY, maxY);

	// The tile the sub is on is where its mid-point is
	_sub->x = (_sub->mapX + (kMapTileWidth  / 2)) / kMapTileWidth;
	_sub->y = (_sub->mapY + (kMapTileHeight / 2)) / kMapTileHeight;

	_sub->sub->turn(direction);

	checkShields();
	checkMouths();
	checkExits();
}

void Penetration::subShoot() {
	if (!_sub->sub->canMove())
		return;

	_sub->sub->shoot();

	_vm->_sound->blasterPlay(&_soundShoot, 1, 0);
}

Submarine::Direction Penetration::getDirection(int &x, int &y) const {
	x = _keys[kKeyRight] ? 3 : (_keys[kKeyLeft] ? -3 : 0);
	y = _keys[kKeyDown ] ? 3 : (_keys[kKeyUp  ] ? -3 : 0);

	if ((x > 0) && (y > 0))
		return Submarine::kDirectionSE;
	if ((x > 0) && (y < 0))
		return Submarine::kDirectionNE;
	if ((x < 0) && (y > 0))
		return Submarine::kDirectionSW;
	if ((x < 0) && (y < 0))
		return Submarine::kDirectionNW;
	if (x > 0)
		return Submarine::kDirectionE;
	if (x < 0)
		return Submarine::kDirectionW;
	if (y > 0)
		return Submarine::kDirectionS;
	if (y < 0)
		return Submarine::kDirectionN;

	return Submarine::kDirectionNone;
}

void Penetration::checkShields() {
	for (Common::List<Position>::iterator pos = _shields.begin(); pos != _shields.end(); ++pos) {
		if ((pos->x == _sub->x) && (pos->y == _sub->y)) {
			// Charge shields
			_shieldMeter->setMaxValue();

			// Play the shield sound
			_vm->_sound->blasterPlay(&_soundShield, 1, 0);

			// Erase the shield from the map
			const int mapX = kPlayAreaBorderWidth  + pos->x * kMapTileWidth;
			const int mapY = kPlayAreaBorderHeight + pos->y * kMapTileHeight;
			_sprites->draw(*_map, 30, mapX, mapY);

			_shields.erase(pos);
			break;
		}
	}
}

void Penetration::checkMouths() {
	for (Common::List<ManagedMouth>::iterator m = _mouths.begin(); m != _mouths.end(); ++m) {
		if (!m->mouth->isDeactivated())
			continue;

		if ((( m->x      == _sub->x) && (m->y == _sub->y)) ||
		    (((m->x + 1) == _sub->x) && (m->y == _sub->y))) {

			m->mouth->activate();

			// Play the mouth sound and do health gain/loss
			if        (m->type == kMouthTypeBite) {
				_vm->_sound->blasterPlay(&_soundBite, 1, 0);
				healthLose(230);
			} else if (m->type == kMouthTypeKiss) {
				_vm->_sound->blasterPlay(&_soundKiss, 1, 0);
				healthGain(120);
			}
		}
	}
}

void Penetration::checkExits() {
	if (!_sub->sub->canMove())
		return;

	for (Common::List<Position>::iterator e = _exits.begin(); e != _exits.end(); ++e) {
		if ((e->x == _sub->x) && (e->y == _sub->y)) {
			_sub->mapX = e->x * kMapTileWidth;
			_sub->mapY = e->y * kMapTileHeight;

			_sub->sub->leave();

			_vm->_sound->blasterPlay(&_soundExit, 1, 0);
			break;
		}
	}
}

void Penetration::healthGain(int amount) {
	if (_shieldMeter->getValue() > 0)
		_healthMeter->increase(_shieldMeter->increase(amount));
	else
		_healthMeter->increase(amount);
}

void Penetration::healthLose(int amount) {
	_healthMeter->decrease(_shieldMeter->decrease(amount));

	if (_healthMeter->getValue() == 0)
		_sub->sub->die();
}

void Penetration::checkExited() {
	if (_sub->sub->hasExited()) {
		_floor++;

		if (_floor >= kFloorCount)
			return;

		setPalette();
		createMap();
		drawFloorText();
	}
}

bool Penetration::isDead() const {
	return _sub && _sub->sub->isDead();
}

bool Penetration::hasWon() const {
	return _floor >= kFloorCount;
}

int Penetration::getLanguage() const {
	if (_vm->_global->_language < kLanguageCount)
		return _vm->_global->_language;

	return kFallbackLanguage;
}

void Penetration::updateAnims() {
	int16 left = 0, top = 0, right = 0, bottom = 0;

	// Clear the previous map animation frames
	for (Common::List<ANIObject *>::iterator a = _mapAnims.reverse_begin();
			 a != _mapAnims.end(); --a) {

		(*a)->clear(*_map, left, top, right, bottom);
	}

	// Draw the current map animation frames
	for (Common::List<ANIObject *>::iterator a = _mapAnims.begin();
			 a != _mapAnims.end(); ++a) {

		(*a)->draw(*_map, left, top, right, bottom);
		(*a)->advance();
	}

	// Clear the previous animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.reverse_begin();
			 a != _anims.end(); --a) {

		if ((*a)->clear(*_vm->_draw->_backSurface, left, top, right, bottom))
			_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
	}

	if (_sub) {
		// Draw the map

		_vm->_draw->_backSurface->blit(*_map, _sub->mapX, _sub->mapY,
				_sub->mapX + kPlayAreaWidth - 1, _sub->mapY + kPlayAreaHeight - 1, kPlayAreaX, kPlayAreaY);
		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, kPlayAreaX, kPlayAreaY,
				kPlayAreaX + kPlayAreaWidth - 1, kPlayAreaY + kPlayAreaHeight - 1);
	}

	// Draw the current animation frames
	for (Common::List<ANIObject *>::iterator a = _anims.begin();
			 a != _anims.end(); ++a) {

		if ((*a)->draw(*_vm->_draw->_backSurface, left, top, right, bottom))
			_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

		(*a)->advance();
	}

	// Draw the meters
	_shieldMeter->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);

	_healthMeter->draw(*_vm->_draw->_backSurface, left, top, right, bottom);
	_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, left, top, right, bottom);
}

} // End of namespace Geisha

} // End of namespace Gob
