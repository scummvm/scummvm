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

#include "avalanche/gyro2.h"
#include "common/textconsole.h"

//#include "pingo.h"
//#include "scrolls.h"
//#include "lucerna.h"
//#include "visa.h"
//#include "acci.h"
//#include "trip5.h"
//#include "dropdown.h"
//#include "basher.h"



namespace Avalanche {

	namespace Gyro {

	const Common::String things[numobjs] = {
		"Wine", "Money-bag", "Bodkin", "Potion", "Chastity belt",
		"Crossbow bolt", "Crossbow", "Lute", "Pilgrim's badge", "Mushroom", "Key",
		"Bell", "Scroll", "Pen", "Ink", "Clothes", "Habit", "Onion"
	};

	const char thingchar[] = "WMBParCLguKeSnIohn"; /* V=Vinegar */

	const Common::String better[numobjs] = {
		"some wine", "your money-bag", "your bodkin", "a potion", "a chastity belt",
		"a crossbow bolt", "a crossbow", "a lute", "a pilgrim's badge", "a mushroom",
		"a key", "a bell", "a scroll", "a pen", "some ink", "your clothes", "a habit",
		"an onion"
	};

	const char betterchar[] = "WMBParCLguKeSnIohn";

	void newpointer(byte m) {
		warning("STUB: Gyro::newpointer()");
	}

	void wait() {   /* makes hourglass */
		newpointer(5);
	}

	void on() {
		warning("STUB: Gyro::on()");
	}

	void on_virtual() {
		switch (visible) {
		case m_virtual:
			return;
			break;
		case m_yes:
			off();
			break;
		}

		visible = m_virtual;
	}

	void off() {
		warning("STUB: Gyro::off()");
	}

	void off_virtual() {
		warning("STUB: Gyro::off_virtual()");
	}

	void xycheck() {   /* only updates mx & my, not all other mouse vars */
		warning("STUB: Gyro::xycheck()");
	}

	void hopto(int16 x, int16 y) { /* Moves mouse void *to x,y */
		warning("STUB: Gyro::hopto()");
	}

	void check() {
		warning("STUB: Gyro::check()");
	}

	void note(uint16 hertz) {
		warning("STUB: Gyro::note()");
	}

	void blip() {
		warning("STUB: Gyro::blip()");
	}

	Common::String strf(int32 x) {
		Common::String q = Common::String::format("%d", x);
		return q;
	}

	void shadow(int16 x1, int16 y1, int16 x2, int16 y2, byte hc, byte sc) {
		warning("STUB: Gyro::shadow()");
	}

	void shbox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String t) {
		warning("STUB: Gyro::shbox()");
	}

	void newgame() {   /* This sets up the DNA for a completely new game. */
		warning("STUB: Gyro::newgame()");
	}

	void click() {   /* "Audio keyboard feedback" */
		warning("STUB: Gyro::click()");
	}

	void slowdown() {
		warning("STUB: Gyro::slowdown()");
	}

	bool flagset(char x) {
		for (uint16 i = 0; i < flags.size(); i++)
			if (flags[i] == x)
				return true;
		return false;
	}

	void force_numlock() {
		if ((locks & num) > 0)  locks -= num;
	}

	bool pennycheck(uint16 howmuchby) {
		warning("STUB: Gyro::pennycheck()");
		return true;
	}

	// There'll may be problems with calling these functions becouse of the conversion of the arrays!!!
	// Keep an eye open!
	Common::String getname(byte whose) { 
		Common::String getname_result;
		if (whose < 17)
			getname_result = lads[whose];
		else
			getname_result = lasses[whose-17];
		return getname_result;
	}

	// Keep an eye open! ^
	char getnamechar(byte whose) {
		char getnamechar_result;
		if (whose < 16) 
			getnamechar_result = ladchar[whose];
		else
			getnamechar_result = lasschar[whose-16];
		return getnamechar_result;
	}

	// Keep an eye open! ^^
	Common::String get_thing(byte which) {
		Common::String get_thing_result;
		switch (which) {
		case wine:
			switch (dna.winestate) {
			case 1:
			case 4:
				get_thing_result = things[which];
				break;
			case 3:
				get_thing_result = "Vinegar";
				break;
			}
			break;
		case onion:
			if (dna.rotten_onion)
				get_thing_result = "rotten onion";
			else get_thing_result = things[which];
			break;
		default:
			get_thing_result = things[which];
		}
		return get_thing_result;
	}

	// Keep an eye open! ^^^
	char get_thingchar(byte which) {
		char get_thingchar_result;
		switch (which) {
		case wine:
			if (dna.winestate == 3)
				get_thingchar_result = 'V'; /* Vinegar */
			else
				get_thingchar_result = thingchar[which];
			break;
		default:
			get_thingchar_result = thingchar[which];
		}
		return get_thingchar_result;
	}

	// Keep an eye open! ^^^^
	Common::String get_better(byte which) {
		Common::String get_better_result;
		if (which > 150)  which -= 149;
		switch (which) {
		case wine:
			switch (dna.winestate) {
			case 0:
			case 1:
			case 4:
				get_better_result = better[which];
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
			else get_better_result = better[which];
			break;
		default:
			if ((which < numobjs) && (which > '\0'))
				get_better_result = better[which];
			else
				get_better_result = "";
		}
		return get_better_result;
	}

	// Get back here after finished with acci.pas, where vb_-s are resided.
	Common::String f5_does()
	/* This procedure determines what f5 does. */
	{
		warning("STUB: Gyro::f5_does()");
		return "STUB: Gyro::f5_does()";
	}

	// Pobably vmc functions will deal with the mouse cursor.
	void plot_vmc(int16 xx, int16 yy, byte page_) {
		warning("STUB: Gyro::plot_vmc()");
	}

	void wipe_vmc(byte page_) {
		warning("STUB: Gyro::wipe_vmc()");
	}

	void setup_vmc() {
		warning("STUB: Gyro::setup_vmc()");
	}

	void clear_vmc() {
		warning("STUB: Gyro::clear_vmc()");
	}

	void setminmaxhorzcurspos(uint16 min, uint16 max) { /* phew */
		warning("STUB: Gyro::setminmaxhorzcurspos()");
	}

	void setminmaxvertcurspos(uint16 min, uint16 max) {
		warning("STUB: Gyro::setminmaxvertcurspos()");
	}

	void load_a_mouse(byte which) {
		warning("STUB: Gyro::load_a_mouse()");
	}

	void background(byte x) {
		warning("STUB: Gyro::background()");
	}

	void hang_around_for_a_while() {
		byte fv;

		for (fv = 1; fv <= 28; fv ++) slowdown();
	}

	bool mouse_near_text() {
		bool mouse_near_text_result;
		mouse_near_text_result = (my > 144) && (my < 188);
		return mouse_near_text_result;
	}

	/* Super_Off and Super_On are two very useful procedures. Super_Off switches
	  the mouse cursor off, WHATEVER it's like. Super_On restores it again
	  afterwards. */

	void super_off() {
		super_was_off = visible == m_no;
		if (super_was_off)  return;

		super_was_virtual = visible == m_virtual;

		if (visible == m_virtual)  off_virtual();
		else off();
	}

	void super_on() {
		if ((visible != m_no) || (super_was_off))  return;

		if (super_was_virtual)  on_virtual();
		else on();
	}

	} // End of namespace Gyro

} // End of namespace Avalanche
