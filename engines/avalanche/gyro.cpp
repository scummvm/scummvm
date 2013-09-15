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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* GYRO		It all revolves around this bit! */

#include "avalanche/avalanche.h"

#include "avalanche/gyro.h"
#include "avalanche/pingo.h"
#include "avalanche/scrolls.h"
#include "avalanche/lucerna.h"
#include "avalanche/acci.h"
#include "avalanche/animation.h"

#include "common/file.h"
#include "common/random.h"
#include "common/textconsole.h"

namespace Avalanche {

const char *Gyro::kVersionNum = "1.30";
const char *Gyro::kCopyright = "1995";

const MouseHotspotType Gyro::kMouseHotSpots[9] = {
	{8,0}, // 1 - up-arrow
	{0,0}, // 2 - screwdriver
	{15,6}, // 3 - right-arrow
	{0,0}, // 4 - fletch
	{8,7}, // 5 - hourglass
	{4,0}, // 6 - TTHand
	{8,5}, // 7- Mark's crosshairs
	{8,7}, // 8- I-beam
	{0,0} // 9 - question mark
};

// Art gallery at 2,1; notice about this at 2,2.
const int32 Gyro::kCatacombMap[8][8] = {
	// Geida's room
	// 1       2	   3     | 4	   5	   6	   7       8
	{0x204,	 0x200,  0xd0f0, 0xf0ff, 0xff,   0xd20f, 0xd200, 0x200},
	{0x50f1, 0x20ff, 0x2ff,  0xff,   0xe0ff, 0x20ff, 0x200f, 0x7210},
	{0xe3f0, 0xe10f, 0x72f0, 0xff,   0xe0ff, 0xff,   0xff,   0x800f},
	{0x2201, 0x2030, 0x800f, 0x220,  0x20f,  0x30,   0xff,   0x23f}, // >> Oubliette
	{0x5024, 0xf3,   0xff,   0x200f, 0x22f0, 0x20f,  0x200,  0x7260},
	{0xf0,   0x2ff,  0xe2ff, 0xff,   0x200f, 0x50f0, 0x72ff, 0x201f},
	{0xf6,   0x220f, 0x22f0, 0x30f,  0xf0,   0x20f,  0x8200, 0x2f0}, // <<< In here
	{0x34,   0x200f, 0x51f0, 0x201f, 0xf1,   0x50ff, 0x902f, 0x2062}
};
// vv Stairs trap.

/* Explanation: $NSEW.
		Nibble N: North.
		0     = no connection,
		2     = (left,) middle(, right) door with left-hand handle,
		5     = (left,) middle(, right) door with right-hand handle,
		7     = arch,
		8     = arch and 1 north of it,
		9     = arch and 2 north of it,
		D     = no connection + WINDOW,
		E     = no connection + TORCH,
		F     = recessed door (to Geida's room.)

		Nibble S: South.
		0     = no connection,
		1,2,3 = left, middle, right door.

		Nibble E: East.
		0     = no connection (wall),
		1     = no connection (wall + window),
		2     = wall with door,
		3     = wall with door and window,
		6     = wall with candles,
		7     = wall with door and candles,
		F     = straight-through corridor.

		Nibble W: West.
		0     = no connection (wall),
		1     = no connection (wall + shield),
		2     = wall with door,
		3     = wall with door and shield,
		4     = no connection (window),
		5     = wall with door and window,
		6     = wall with candles,
		7     = wall with door and candles,
		F     = straight-through corridor. */

const char Gyro::kSpludwicksOrder[3] = {kObjectOnion, kObjectInk, kObjectMushroom};

// A quasiped defines how people who aren't sprites talk. For example, quasiped
// "A" is Dogfood. The rooms aren't stored because I'm leaving that to context.
const QuasipedType Gyro::kQuasipeds[16] = {
	{2, kColorLightgray, 19, kColorBrown, kPeopleDogfood}, // A: Dogfood (screen 19).
	{3, kColorGreen, 19, kColorWhite, kPeopleIbythneth}, // B: Ibythneth (screen 19).
	{3, kColorWhite, 1, kColorMagenta, kPeopleArkata}, // C: Arkata (screen 1).
	{3, kColorBlack, 23, kColorRed, 177}, // D: Hawk (screen 23).
	{3, kColorLightgreen, 50, kColorBrown, kPeopleTrader}, // E: Trader (screen 50).
	{6, kColorYellow, 42, kColorRed, kPeopleAvalot}, // F: Avvy, tied up (scr.42)
	{2, kColorBlue, 16, kColorWhite, kPeopleAyles}, // G: Ayles (screen 16).
	{2, kColorBrown, 7, kColorWhite, kPeopleJacques}, // H: Jacques (screen 7).
	{2, kColorLightgreen, 47, kColorGreen, kPeopleSpurge}, // I: Spurge (screen 47).
	{3, kColorYellow, 47, kColorRed, kPeopleAvalot}, // J: Avalot (screen 47).
	{2, kColorLightgray, 23, kColorBlack, kPeopleDuLustie}, // K: du Lustie (screen 23).
	{2, kColorYellow, 27, kColorRed, kPeopleAvalot}, // L: Avalot (screen 27).
	{3, kColorWhite, 27, kColorRed, 177}, // M: Avaroid (screen 27).
	{4, kColorLightgray, 19, kColorDarkgray, kPeopleMalagauche}, // N: Malagauche (screen 19).
	{5, kColorLightmagenta, 47, kColorRed, kPeoplePort}, // O: Port (screen 47).
	{2, kColorLightgreen, 51, kColorDarkgray, kPeopleDrDuck} // P: Duck (screen 51).
};

const char Gyro::kMusicKeys[] = "QWERTYUIOP[]";

const uint16 Gyro::kNotes[12] = {196, 220, 247, 262, 294, 330, 350, 392, 440, 494, 523, 587};

const TuneType Gyro::kTune = {
	kPitchHigher, kPitchHigher, kPitchLower, kPitchSame, kPitchHigher, kPitchHigher, kPitchLower, kPitchHigher, kPitchHigher, kPitchHigher,
	kPitchLower, kPitchHigher, kPitchHigher, kPitchSame, kPitchHigher, kPitchLower, kPitchLower, kPitchLower, kPitchLower, kPitchHigher,
	kPitchHigher, kPitchLower, kPitchLower, kPitchLower, kPitchLower, kPitchSame, kPitchLower, kPitchHigher, kPitchSame, kPitchLower, kPitchHigher
};

byte Gyro::_whereIs[29] = {
	// The Lads
	kRoomYours, // Avvy
	kRoomSpludwicks, // Spludwick
	kRoomOutsideYours, // Crapulus
	kRoomDucks, // Duck - r__DucksRoom's not defined yet.
	kRoomArgentPub, // Malagauche
	kRoomRobins, // Friar Tuck.
	177, // Robin Hood - can't meet him at the start.
	kRoomBrummieRoad, // Cwytalot
	kRoomLustiesRoom, // Baron du Lustie.
	kRoomOutsideCardiffCastle, // The Duke of Cardiff.
	kRoomArgentPub, // Dogfood
	kRoomOutsideDucks, // Trader
	kRoomArgentPub, // Ibythneth
	kRoomAylesOffice, // Ayles
	kRoomNottsPub, // Port
	kRoomNottsPub, // Spurge
	kRoomMusicRoom, // Jacques
	0, 0, 0, 0, 0, 0, 0, 0,
	// The Lasses
	kRoomYours, // Arkata
	kRoomGeidas, // Geida
	177, // nobody allocated here!
	kRoomWiseWomans  // The Wise Woman.
};

Gyro::Gyro(AvalancheEngine *vm) : _interrogation(0), _onCanDoPageSwap(true) {
	_vm = vm;

	// Needed because of Lucerna::load_also()
	for (int fv = 0; fv < 31; fv++) {
		for (int ff = 0; ff < 2; ff++)
			_also[fv][ff] = 0;
	}

	_totalTime = 0;
}

Gyro::~Gyro() {
	for (byte i = 0; i < 9; i++) {
		_digits[i].free();
		_directions[i].free();
	}
	_digits[9].free();
}

Common::String Gyro::intToStr(int32 num) {
	return Common::String::format("%d", num);
}

void Gyro::newMouse(byte id) {
	if (id == _currentMouse)
		return;
	_currentMouse = id;

	loadMouse(id);
}

/**
 * Set the mouse pointer to 'HourGlass"
 * @remarks	Originally called 'wait'
 */
void Gyro::setMousePointerWait() {
	newMouse(5);
}

void Gyro::note(uint16 hertz) {
	warning("STUB: Gyro::note()");
}

void Gyro::blip() {
	warning("STUB: Gyro::blip()");
}

void Gyro::drawShadow(int16 x1, int16 y1, int16 x2, int16 y2, byte hc, byte sc) {
	warning("STUB: Gyro::shadow()");
}

void Gyro::drawShadowBox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String t) {
	warning("STUB: Gyro::shbox()");
}

void Gyro::resetVariables() {
// Replaces memset(&_vm->_gyro->_dna, 0, sizeof(DnaType));
	_vm->_animation->_direction = 0;
	_carryNum = 0;
	for (int i = 0; i < kObjectNum; i++)
		_objects[i] = false;

	_dnascore = 0;
	_money = 0;
	_room = 0;
	_wearing = 0;
	_sworeNum = 0;
	_saveNum = 0;
	for (int i = 0; i < 100; i++)
		_roomCount[i] = 0;

	_alcoholLevel = 0;
	_playedNim = 0;
	_wonNim = false;
	_wineState = 0;
	_cwytalotGone = false;
	_passwordNum = 0;
	_aylesIsAwake = false;
	_drawbridgeOpen = 0;
	_avariciusTalk = 0;
	_boughtOnion = false;
	_rottenOnion = false;
	_onionInVinegar = false;
	_givenToSpludwick = 0;
	_brummieStairs = 0;
	_cardiffQuestionNum = 0;
	_passedCwytalotInHerts = false;
	_avvyIsAwake = false;
	_avvyInBed = false;
	_userMovesAvvy = false;
	_npcFacing = 0;
	_givenBadgeToIby = false;
	_friarWillTieYouUp = false;
	_tiedUp = false;
	_boxContent = 0;
	_talkedToCrapulus = false;
	_jacquesState = 0;
	_bellsAreRinging = false;
	_standingOnDais = false;
	_takenPen = false;
	_arrowTriggered = false;
	_arrowInTheDoor = false;
	_favouriteDrink = "";
	_favouriteSong = "";
	_worstPlaceOnEarth = "";
	_spareEvening = "";
	_totalTime = 0;
	_jumpStatus = 0;
	_mushroomGrowing = false;
	_spludwickAtHome = false;
	_lastRoom = 0;
	_lastRoomNotMap = 0;
	_crapulusWillTell = false;
	_enterCatacombsFromLustiesRoom = false;
	_teetotal = false;
	_malagauche = 0;
	_drinking = 0;
	_enteredLustiesRoomAsMonk = false;
	_catacombX = 0;
	_catacombY = 0;
	_avvysInTheCupboard = false;
	_geidaFollows = false;
	_geidaSpin = 0;
	_geidaTime = 0;
	_nextBell = 0;
	_givenPotionToGeida = false;
	_lustieIsAsleep = false;
	_flipToWhere = 0;
	_flipToPed = 0;
	_beenTiedUp = false;
	_sittingInPub = false;
	_spurgeTalkCount = 0;
	_metAvaroid = false;
	_takenMushroom = false;
	_givenPenToAyles = false;
	_askedDogfoodAboutNim = false;
}

void Gyro::newGame() {
	for (byte i = 0; i < kMaxSprites; i++) {
		AnimationType *spr = &_vm->_animation->_sprites[i];
		if (spr->_quick)
			spr->remove();
	}
	// Deallocate sprite. Sorry, beta testers!

	AnimationType *avvy = &_vm->_animation->_sprites[0];
	avvy->init(0, true, _vm->_animation);

	_alive = true;
	resetVariables();

	_vm->_scrolls->setBubbleStateNatural();

	_spareEvening = "answer a questionnaire";
	_favouriteDrink = "beer";
	_money = 30; // 2/6
	_vm->_animation->_direction = Animation::kDirStopped;
	_wearing = kObjectClothes;
	_objects[kObjectMoney - 1] = true;
	_objects[kObjectBodkin - 1] = true;
	_objects[kObjectBell - 1] = true;
	_objects[kObjectClothes - 1] = true;

	_thinkThing = true;
	_thinks = 2;
	_vm->_lucerna->refreshObjectList();
	_onToolbar = false;
	_seeScroll = false;

	avvy->appear(300, 117, Animation::kDirRight); // Needed to initialize Avalot.
	//for (gd = 0; gd <= 30; gd++) for (gm = 0; gm <= 1; gm++) also[gd][gm] = nil;
	// fillchar(previous^,sizeof(previous^),#0); { blank out array }
	_him = 254;
	_her = 254;
	_it = 254;
	_lastPerson = 254; // = Pardon?
	_passwordNum = _vm->_rnd->getRandomNumber(30) + 1; //Random(30) + 1;
	_userMovesAvvy = false;
	_doingSpriteRun = false;
	_avvyInBed = true;
	_enidFilename = "";

	_vm->_lucerna->enterRoom(1, 1);
	avvy->_visible = false;
	_vm->_lucerna->drawScore();
	_vm->_dropdown->setupMenu();
	_vm->_lucerna->_clock.update();
	_vm->_lucerna->spriteRun();
}

void Gyro::click() {
	warning("STUB: Gyro::click()");
}

void Gyro::slowDown() {
	warning("STUB: Gyro::slowdown()");
}

bool Gyro::setFlag(char x) {
	for (uint16 i = 0; i < _flags.size(); i++) {
		if (_flags[i] == x)
			return true;
	}

	return false;
}

void Gyro::forceNumlock() {
	warning("STUB: Gyro::force_numlock()");
}

bool Gyro::decreaseMoney(uint16 howmuchby) {
	_money -= howmuchby;
	if (_money < 0) {
		_vm->_scrolls->displayScrollChain('Q', 2); // "You are now denariusless!"
		_vm->_lucerna->gameOver();
		return false;
	} else
		return true;
}

Common::String Gyro::getName(byte whose) {
	static const Common::String kLads[17] = {
		"Avalot", "Spludwick", "Crapulus", "Dr. Duck", "Malagauche", "Friar Tuck",
		"Robin Hood", "Cwytalot", "du Lustie", "the Duke of Cardiff", "Dogfood",
		"A trader", "Ibythneth", "Ayles", "Port", "Spurge", "Jacques"
	};

	static const Common::String kLasses[4] = {"Arkata", "Geida", "\0xB1", "the Wise Woman"};

	if (whose < 175)
		return kLads[whose - 150];
	else
		return kLasses[whose - 175];
}

byte Gyro::getNameChar(byte whose) {
	static const char kLadChar[] = "ASCDMTRwLfgeIyPu";
	static const char kLassChar[] = "kG\0xB1o";

	if (whose < 175)
		return kLadChar[whose - 150];
	else
		return kLassChar[whose - 175];
}

Common::String Gyro::getThing(byte which) {
	static const Common::String kThings[kObjectNum] = {
		"Wine", "Money-bag", "Bodkin", "Potion", "Chastity belt",
		"Crossbow bolt", "Crossbow", "Lute", "Pilgrim's badge", "Mushroom", "Key",
		"Bell", "Scroll", "Pen", "Ink", "Clothes", "Habit", "Onion"
	};

	Common::String get_thing_result;
	switch (which) {
	case kObjectWine:
		switch (_wineState) {
		case 1:
		case 4:
			get_thing_result = kThings[which - 1];
			break;
		case 3:
			get_thing_result = "Vinegar";
			break;
		}
		break;
	case kObjectOnion:
		if (_rottenOnion)
			get_thing_result = "rotten onion";
		else
			get_thing_result = kThings[which - 1];
		break;
	default:
		get_thing_result = kThings[which - 1];
	}
	return get_thing_result;
}

char Gyro::getThingChar(byte which) {
	static const char kThingsChar[] = "WMBParCLguKeSnIohn"; // V=Vinegar

	char get_thingchar_result;
	switch (which) {
	case kObjectWine:
		if (_wineState == 3)
			get_thingchar_result = 'V'; // Vinegar
		else
			get_thingchar_result = kThingsChar[which - 1];
		break;
	default:
		get_thingchar_result = kThingsChar[which - 1];
	}
	return get_thingchar_result;
}

Common::String Gyro::getItem(byte which) {
	static const Common::String kItems[kObjectNum] = {
		"some wine", "your money-bag", "your bodkin", "a potion", "a chastity belt",
		"a crossbow bolt", "a crossbow", "a lute", "a pilgrim's badge", "a mushroom",
		"a key", "a bell", "a scroll", "a pen", "some ink", "your clothes", "a habit",
		"an onion"
	};

	Common::String get_better_result;
	if (which > 150)
		which -= 149;

	switch (which) {
	case kObjectWine:
		switch (_wineState) {
		case 0:
		case 1:
		case 4:
			get_better_result = kItems[which - 1];
			break;
		case 3:
			get_better_result = "some vinegar";
			break;
		}
		break;
	case kObjectOnion:
		if (_rottenOnion)
			get_better_result = "a rotten onion";
		else if (_onionInVinegar)
			get_better_result = "a pickled onion (in the vinegar)";
		else
			get_better_result = kItems[which - 1];
		break;
	default:
		if ((which < kObjectNum) && (which > 0))
			get_better_result = kItems[which - 1];
		else
			get_better_result = "";
	}
	return get_better_result;
}


Common::String Gyro::f5Does() {
	switch (_room) {
	case kRoomYours:
		if (!_avvyIsAwake)
			return Common::String::format("%cWWake up", Acci::kVerbCodeWake);
		else if (_avvyInBed)
			return Common::String::format("%cGGet up", Acci::kVerbCodeStand);
		break;
	case kRoomInsideCardiffCastle:
		if (_standingOnDais)
			return Common::String::format("%cCClimb down", Acci::kVerbCodeClimb);
		else
			return Common::String::format("%cCClimb up", Acci::kVerbCodeClimb);
		break;
	case kRoomNottsPub:
		if (_sittingInPub)
			return Common::String::format("%cSStand up", Acci::kVerbCodeStand);
		else
			return Common::String::format("%cSSit down", Acci::kVerbCodeSit);
		break;
	case kRoomMusicRoom:
		if (_vm->_animation->inField(7))
			return Common::String::format("%cPPlay the harp", Acci::kVerbCodePlay);
		break;
	}

	return Common::String::format("%c", _vm->_acci->kPardon); // If all else fails...
}

void Gyro::loadMouse(byte which) {
	Common::File f;

	if (!f.open("mice.avd"))
		error("AVALANCHE: Gyro: File not found: mice.avd");

	::Graphics::Surface cursor;
	cursor.create(16, 32, ::Graphics::PixelFormat::createFormatCLUT8());
	cursor.fillRect(Common::Rect(0, 0, 16, 32), 255);


	// The AND mask.
	f.seek(kMouseSize * 2 * (which - 1) + 134);

	::Graphics::Surface mask = _vm->_graphics->loadPictureGraphic(f);

	for (byte j = 0; j < mask.h; j++)
		for (byte i = 0; i < mask.w; i++)
			for (byte k = 0; k < 2; k++)
				if (*(byte *)mask.getBasePtr(i, j) == 0)
					*(byte *)cursor.getBasePtr(i, j * 2 + k) = 0;

	mask.free();

	// The OR mask.
	f.seek(kMouseSize * 2 * (which - 1) + 134 * 2);

	mask = _vm->_graphics->loadPictureGraphic(f);

	for (byte j = 0; j < mask.h; j++)
		for (byte i = 0; i < mask.w; i++)
			for (byte k = 0; k < 2; k++) {
				byte pixel = *(byte *)mask.getBasePtr(i, j);
				if (pixel != 0)
					*(byte *)cursor.getBasePtr(i, j * 2 + k) = pixel;
			}

	mask.free();
	f.close();

	CursorMan.replaceCursor(cursor.getPixels(), 16, 32, kMouseHotSpots[which - 1]._horizontal, kMouseHotSpots[which - 1]._vertical * 2, 255, false);
	cursor.free();
}

void Gyro::setBackgroundColor(byte x) {
	warning("STUB: Gyro::background()");
}

void Gyro::hangAroundForAWhile() {
	for (byte i = 0; i < 28; i++)
		slowDown();
}

} // End of namespace Avalanche
