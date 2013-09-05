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

#include "avalanche/gyro2.h"
#include "avalanche/pingo2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/visa2.h"
#include "avalanche/acci2.h"
#include "avalanche/trip6.h"

#include "common/textconsole.h"
#include "common/file.h"

#include "common/random.h"

//#include "dropdown.h"
//#include "basher.h"

namespace Avalanche {

const char *Gyro::vernum = "1.30";
const char *Gyro::copyright = "1995";


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

const Common::String Gyro::kLads[17] = {
	"Avalot", "Spludwick", "Crapulus", "Dr. Duck", "Malagauche", "Friar Tuck",
	"Robin Hood", "Cwytalot", "du Lustie", "the Duke of Cardiff", "Dogfood",
	"A trader", "Ibythneth", "Ayles", "Port", "Spurge", "Jacques"
};

const Common::String Gyro::kLasses[4] =
{"Arkata", "Geida", "±", "the Wise Woman"};

const char Gyro::kLadChar[] = "ASCDMTRwLfgeIyPu";

const char Gyro::kLassChar[] = "kG±o";

// Art gallery at 2,1; notice about this at 2,2.
const int32 Gyro::kCatacombMap[8][8] = {
	// Geida's room
	// 1     2			3   | 4     5		6		7     8
	{0x204, 0x200, 0xd0f0, 0xf0ff, 0xff, 0xd20f, 0xd200, 0x200},
	{0x50f1, 0x20ff, 0x2ff, 0xff, 0xe0ff, 0x20ff, 0x200f, 0x7210},
	{0xe3f0, 0xe10f, 0x72f0, 0xff, 0xe0ff, 0xff, 0xff, 0x800f},
	{0x2201, 0x2030, 0x800f, 0x220, 0x20f, 0x30, 0xff, 0x23f}, // >> Oubliette
	{0x5024, 0xf3, 0xff, 0x200f, 0x22f0, 0x20f, 0x200, 0x7260},
	{0xf0, 0x2ff, 0xe2ff, 0xff, 0x200f, 0x50f0, 0x72ff, 0x201f},
	{0xf6, 0x220f, 0x22f0, 0x30f, 0xf0, 0x20f, 0x8200, 0x2f0}, // <<< In here
	{0x34, 0x200f, 0x51f0, 0x201f, 0xf1, 0x50ff, 0x902f, 0x2062}
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

const char Gyro::kSpludwicksOrder[3] = {onion, ink, mushroom};

const QuasipedType Gyro::kQuasipeds[16] = {
	{2, kColorLightgray, 19, kColorBrown, pdogfood},                       // A: Dogfood (screen 19). 
	{3, kColorGreen,     19, kColorWhite, pibythneth},                     // B: Ibythneth (screen 19).
	{3, kColorWhite,     1, kColorMagenta, parkata},                       // C: Arkata (screen 1).
	{3, kColorBlack,     23, kColorRed, 177},							   // D: Hawk (screen 23).
	{3, kColorLightgreen, 50, kColorBrown, ptrader},                       // E: Trader (screen 50).
	{6, kColorYellow,    42, kColorRed, pavalot},                          // F: Avvy, tied up (scr.42)
	{2, kColorBlue,      16, kColorWhite, payles},                         // G: Ayles (screen 16).
	{2, kColorBrown,     7, kColorWhite, pjacques},                        // H: Jacques (screen 7).
	{2, kColorLightgreen, 47, kColorGreen, pspurge},                       // I: Spurge (screen 47).
	{3, kColorYellow,    47, kColorRed, pavalot},                          // J: Avalot (screen 47).
	{2, kColorLightgray, 23, kColorBlack, pdulustie},                      // K: du Lustie (screen 23).
	{2, kColorYellow,    27, kColorRed, pavalot},                          // L: Avalot (screen 27).
	{3, kColorWhite,     27, kColorRed, 177},                              // M: Avaroid (screen 27).
	{4, kColorLightgray, 19, kColorDarkgray, pmalagauche},                 // N: Malagauche (screen 19).
	{5, kColorLightmagenta, 47, kColorRed, pport},                         // O: Port (screen 47).
	{2, kColorLightgreen, 51, kColorDarkgray, pdrduck}                     // P: Duck (screen 51).
};

const char Gyro::kMusicKeys[] = "QWERTYUIOP[]";

const uint16 Gyro::kNotes[12] = {196, 220, 247, 262, 294, 330, 350, 392, 440, 494, 523, 587};

const TuneType Gyro::kTune = {
	kPitchHigher, kPitchHigher, kPitchLower, kPitchSame, kPitchHigher, kPitchHigher, kPitchLower, kPitchHigher, kPitchHigher, kPitchHigher,
	kPitchLower, kPitchHigher, kPitchHigher,
	kPitchSame, kPitchHigher, kPitchLower, kPitchLower, kPitchLower, kPitchLower, kPitchHigher, kPitchHigher, kPitchLower, kPitchLower, kPitchLower,
	kPitchLower, kPitchSame, kPitchLower, kPitchHigher, kPitchSame, kPitchLower, kPitchHigher
};

byte Gyro::_whereIs[29] = {
	// The Lads
	r__yours, // Avvy
	r__spludwicks, // Spludwick
	r__outsideyours, // Crapulus
	r__ducks, // Duck - r__DucksRoom's not defined yet.
	r__argentpub, // Malagauche
	r__robins, // Friar Tuck.
	177, // Robin Hood - can't meet him at the start.
	r__brummieroad, // Cwytalot
	r__lustiesroom, // Baron du Lustie.
	r__outsidecardiffcastle, // The Duke of Cardiff.
	r__argentpub, // Dogfood
	r__outsideducks, // Trader
	r__argentpub, // Ibythneth
	r__aylesoffice, // Ayles
	r__nottspub, // Port
	r__nottspub, // Spurge
	r__musicroom, // Jacques
	0, 0, 0, 0, 0, 0, 0, 0,
	// The Lasses
	r__yours, // Arkata
	r__geidas, // Geida
	177, // nobody allocated here!
	r__wisewomans  // The Wise Woman.
};

const Common::String Gyro::things[numobjs] = {
	"Wine", "Money-bag", "Bodkin", "Potion", "Chastity belt",
	"Crossbow bolt", "Crossbow", "Lute", "Pilgrim's badge", "Mushroom", "Key",
	"Bell", "Scroll", "Pen", "Ink", "Clothes", "Habit", "Onion"
};

const char Gyro::thingchar[] = "WMBParCLguKeSnIohn"; // V=Vinegar

const Common::String Gyro::better[numobjs] = {
	"some wine", "your money-bag", "your bodkin", "a potion", "a chastity belt",
	"a crossbow bolt", "a crossbow", "a lute", "a pilgrim's badge", "a mushroom",
	"a key", "a bell", "a scroll", "a pen", "some ink", "your clothes", "a habit",
	"an onion"
};

const char Gyro::betterchar[] = "WMBParCLguKeSnIohn";

Gyro::Gyro(AvalancheEngine *vm) : _interrogation(0), _onCanDoPageSwap(true) {
	_vm = vm;

	// Needed because of Lucerna::load_also()
	for (int fv = 0; fv < 31; fv++) {
		for (int ff = 0; ff < 2; ff++)
			also[fv][ff] = 0;
	}
	dna.total_time = 0;
}

Gyro::~Gyro() {
	for (byte i = 0; i < 9; i++) {
		digit[i].free();
		rwlite[i].free();
	}
	digit[9].free();
}

Common::String Gyro::strf(int32 x) {
	Common::String q = Common::String::format("%d", x);
	return q;
}

void Gyro::newpointer(byte id) {
	if (id == cmp)
		return;
	cmp = id;

	load_a_mouse(id);
}

void Gyro::wait() {
	newpointer(5);
}

void Gyro::on() {
	warning("STUB: Gyro::on()");
}

void Gyro::off() {
	warning("STUB: Gyro::off()");
}

void Gyro::xycheck() { 
	warning("STUB: Gyro::xycheck()");
}

void Gyro::check() {
	warning("STUB: Gyro::check()");
}

void Gyro::note(uint16 hertz) {
	warning("STUB: Gyro::note()");
}

void Gyro::blip() {
	warning("STUB: Gyro::blip()");
}

void Gyro::shadow(int16 x1, int16 y1, int16 x2, int16 y2, byte hc, byte sc) {
	warning("STUB: Gyro::shadow()");
}

void Gyro::shbox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String t) {
	warning("STUB: Gyro::shbox()");
}

void Gyro::newgame() { 
	for (byte gm = 0; gm < kMaxSprites; gm++) {
		if (_vm->_trip->tr[gm].quick)
			_vm->_trip->tr[gm].done();
	}
	// Deallocate sprite. Sorry, beta testers!

	_vm->_trip->tr[0].init(0, true, _vm->_trip);
	alive = true;

	score = 0;
	//for gd:=0 to 5 do which[gd]:=1;
	memset(&_vm->_gyro->dna, 0, sizeof(dnatype));
	_vm->_scrolls->natural();
	_vm->_lucerna->mousepage(0);
	dna.spare_evening = "answer a questionnaire";
	dna.like2drink = "beer";

	dna.pence = 30; // 2/6
	dna.rw = kDirectionStopped;
	dna.wearing = clothes;
	dna.obj[money - 1] = true;
	dna.obj[bodkin - 1] = true;
	dna.obj[bell - 1] = true;
	dna.obj[clothes - 1] = true;
	
	thinkthing = true;
	thinks = 2;
	_vm->_lucerna->objectlist();
	ontoolbar = false;
	seescroll = false;

	ppos[0][1] = -177; 
	_vm->_trip->tr[0].appear(300,117,kDirectionRight); // Needed to initialize Avalot.
	//for (gd = 0; gd <= 30; gd++) for (gm = 0; gm <= 1; gm++) also[gd][gm] = nil;
	// fillchar(previous^,sizeof(previous^),#0); { blank out array }
	him = 254;
	her = 254;
	it = 254;
	last_person = 254; // = Pardon?
	dna.pass_num = _vm->_rnd->getRandomNumber(30) + 1; //Random(30) + 1;
	after_the_scroll = false;
	dna.user_moves_avvy = false;
	doing_sprite_run = false;
	dna.avvy_in_bed = true;
	enid_filename = "";

	for (byte gd = 0; gd <= 1; gd++) {
		cp = 1 - cp;
		_vm->_trip->getback();
	}

	_vm->_lucerna->enterroom(1, 1);
	_vm->_trip->new_game_for_trippancy();
	_vm->_lucerna->showscore();

	_vm->_dropdown->setupMenu();
	_vm->_lucerna->clock_lucerna();
	_vm->_lucerna->sprite_run();
}

void Gyro::click() { 
	warning("STUB: Gyro::click()");
}

void Gyro::slowdown() {
	warning("STUB: Gyro::slowdown()");
}

bool Gyro::flagset(char x) {
	for (uint16 i = 0; i < flags.size(); i++) {
		if (flags[i] == x)
			return true;
	}

	return false;
}

void Gyro::force_numlock() {
	warning("STUB: Gyro::force_numlock()");
}

bool Gyro::pennycheck(uint16 howmuchby) {
	dna.pence -= howmuchby;
	if (dna.pence < 0) {
		_vm->_visa->dixi('Q', 2); // "You are now denariusless!"
		_vm->_lucerna->gameover();
		return false;
	} else
		return true;
}

// There'll may be problems with calling these functions because of the conversion of the arrays!!!
// Keep an eye open!
Common::String Gyro::getname(byte whose) { 
	if (whose < 175)
		return kLads[whose - 150];
	else
		return kLasses[whose - 175];
}

// Keep an eye open! ^
byte Gyro::getnamechar(byte whose) {
	if (whose < 175) 
		return kLadChar[whose - 150];
	else
		return kLassChar[whose - 175];
}

// Keep an eye open! ^^
Common::String Gyro::get_thing(byte which) {
	Common::String get_thing_result;
	switch (which) {
	case wine:
		switch (dna.winestate) {
		case 1:
		case 4:
			get_thing_result = things[which - 1];
			break;
		case 3:
			get_thing_result = "Vinegar";
			break;
		}
		break;
	case onion:
		if (dna.rotten_onion)
			get_thing_result = "rotten onion";
		else
			get_thing_result = things[which - 1];
		break;
	default:
		get_thing_result = things[which - 1];
	}
	return get_thing_result;
}

// Keep an eye open! ^^^
char Gyro::get_thingchar(byte which) {
	char get_thingchar_result;
	switch (which) {
	case wine:
		if (dna.winestate == 3)
			get_thingchar_result = 'V'; // Vinegar
		else
			get_thingchar_result = thingchar[which - 1];
		break;
	default:
		get_thingchar_result = thingchar[which - 1];
	}
	return get_thingchar_result;
}

// Keep an eye open! ^^^^
Common::String Gyro::get_better(byte which) {
	Common::String get_better_result;
	if (which > 150)
		which -= 149;

	switch (which) {
	case wine:
		switch (dna.winestate) {
		case 0:
		case 1:
		case 4:
			get_better_result = better[which - 1];
			break;
		case 3:
			get_better_result = "some vinegar";
			break;
		}
		break;
	case onion:
		if (dna.rotten_onion)
			get_better_result = "a rotten onion";
		else if (dna.onion_in_vinegar)
			get_better_result = "a pickled onion (in the vinegar)";
		else
			get_better_result = better[which - 1];
		break;
	default:
		if ((which < numobjs) && (which > 0))
			get_better_result = better[which - 1];
		else
			get_better_result = "";
	}
	return get_better_result;
}


Common::String Gyro::f5_does() {
	Common::String f5_does_result;
	switch (dna.room) {
	case r__yours: {
			if (!dna.avvy_is_awake)
				return Common::String(_vm->_acci->kVerbCodeWake) + "WWake up";
			else if (dna.avvy_in_bed)
				return Common::String(_vm->_acci->kVerbCodeStand) + "GGet up";
		}
		break;
	case r__insidecardiffcastle: 
		if (dna.standing_on_dais)
			return Common::String(_vm->_acci->kVerbCodeClimb) + "CClimb down";
		else
			return Common::String(_vm->_acci->kVerbCodeClimb) + "CClimb up";
		break;
	case r__nottspub:
		if (dna.sitting_in_pub)
			return Common::String(_vm->_acci->kVerbCodeStand) + "SStand up";
		else
			return Common::String(_vm->_acci->kVerbCodeSit) + "SSit down";
		break;
	case r__musicroom:
		if (_vm->_trip->infield(7))
			return Common::String(_vm->_acci->kVerbCodePlay) + "PPlay the harp";
		break;
	}

	return Common::String(_vm->_acci->kPardon); // If all else fails...
}

void Gyro::load_a_mouse(byte which) {
	Common::File f;

	if (!f.open("mice.avd")) {
		warning("AVALANCHE: Gyro: File not found: mice.avd");
		return;
	}

	::Graphics::Surface cursor;
	cursor.create(16, 32, ::Graphics::PixelFormat::createFormatCLUT8());
	cursor.fillRect(Common::Rect(0, 0, 16, 32), 255);


	// The AND mask.
	f.seek(mouse_size * 2 * (which - 1) + 134);

	::Graphics::Surface mask = _vm->_graphics->loadPictureGraphic(f);

	for (byte j = 0; j < mask.h; j++)
		for (byte i = 0; i < mask.w; i++)
			for (byte k = 0; k < 2; k++)
				if (*(byte *)mask.getBasePtr(i, j) == 0)
					*(byte *)cursor.getBasePtr(i, j * 2 + k) = 0;
	
	mask.free();

	// The OR mask.
	f.seek(mouse_size * 2 * (which - 1) + 134 * 2);

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



	CursorMan.replaceCursor(cursor.pixels, 16, 32, kMouseHotSpots[which - 1]._horizontal, kMouseHotSpots[which - 1]._vertical * 2, 255, false);

	cursor.free();
}

void Gyro::background(byte x) {
	warning("STUB: Gyro::background()");
}

void Gyro::hang_around_for_a_while() {
	for (byte i = 0; i < 28; i++)
		slowdown();
}

bool Gyro::mouse_near_text() {
	return (my > 144) && (my < 188);
}

} // End of namespace Avalanche
