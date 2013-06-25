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

#include "avalanche/scrolls2.h"
#include "avalanche/gyro2.h"
#include "avalanche/logger2.h"
#include "avalanche/enhanced2.h"

//#include "lucerna.h"
//#include "trip5.h"
//#include "Acci.h"
//#include "basher.h"
//#include "visa.h"
//#include "timeout.h"

#include "common/textconsole.h"

namespace Avalanche {

	namespace Scrolls {

	const int16 roman = 0;
	const int16 italic = 1;

	const int16 halficonwidth = 19; /* Half the width of an icon. */

	int16 dix, diy;
	Gyro::raw ch[2];
	byte cfont; /* Current font */

	int16 dodgex, dodgey;
	byte param; /* For using arguments code */

	byte use_icon;


	void initialize_scrolls() {
		warning("STUB: Scrolls::initialize_scrolls()");
	}

	void state(byte x) {     /* Sets "Ready" light to whatever */
		byte page_;

		if (Gyro::ledstatus == x)  return; /* Already like that! */

		warning("STUB: Scrolls::state(). Calls of Pascal units need to be removed.");
		//switch (x) {
		//case 0:
		//	setfillstyle(1, black);
		//	break; /* Off */
		//case 1:
		//	setfillstyle(9, green);
		//	break; /* Half-on (menus) */
		//case 2:
		//	setfillstyle(1, green);
		//	break; /* On (kbd) */
		//case 3:
		//	setfillstyle(6, green);
		//	break; /* Hit a key */
		//}

		Gyro::super_off();

		/*	for (page_ = 0; page_ <= 1; page_ ++) {
		setactivepage(page_);
		bar(419, 195, 438, 197);
		}*/

		Gyro::super_on();
		Gyro::ledstatus = x;
	}

	void easteregg() {
		uint16 fv, ff;

		warning("STUB: Scrolls::easteregg(). Calls of Pascal units need to be removed.");

		/*	background(15);
		for (fv = 4; fv <= 100; fv ++)
		for (ff = 0; ff <= 70; ff ++) {
		sound(fv * 100 + ff * 10);
		delay(1);
		}
		nosound;
		setcolor(10);
		settextstyle(0, 0, 3);
		settextjustify(1, 1);
		outtextxy(320, 100, "GIED");
		settextstyle(0, 0, 1);
		settextjustify(0, 2);*/

		Gyro::background(0);
	}

	void say(int16 x, int16 y, Common::String z) { /* Fancy FAST screenwriting */
		const int16 locol = 2;
		byte xx, yy, ox, bit, lz, t;
		int16 yp;
		bool offset;
		byte itw[12][80];

		offset = x % 8 == 4;
		x = x / 8;
		lz = z.size();
		ox = 0;
		Logger::log_scrollline();

		for (xx = 1; xx <= lz; xx ++) {
			switch (z[xx]) {
			case '\22': {
				cfont = roman;
				Logger::log_roman();
						}
						break;
			case '\6': {
				cfont = italic;
				Logger::log_italic();
					   }
					   break;
			default: {
				ox += 1;
				for (yy = 1; yy <= 12; yy ++) itw[yy][ox] = ~ ch[cfont][z[xx]][yy + 1];
				Logger::log_scrollchar(Common::String(z[xx]));
					 }
			}
		}

		lz = ox;
		if (offset) {
			/* offsetting routine */
			lz += 1;
			for (yy = 1; yy <= 12; yy ++) {
				bit = 240;
				itw[yy][lz] = 255;
				for (xx = 1; xx <= lz; xx ++) {
					t = itw[yy][xx];
					itw[yy][xx] = bit + t / 16;
					bit = t << 4;
				}
			}
		}
		yp = x + y * 80 + (1 - Gyro::cp) * Gyro::pagetop;
		for (yy = 1; yy <= 12; yy ++) {
			yp += 80;
			for (bit = 0; bit <= locol; bit ++) {
				/*port[0x3c4] = 2;
				port[0x3ce] = 4;
				port[0x3c5] = 1 << bit;
				port[0x3cf] = bit;
				move(itw[yy], mem[0xa000 * yp], lz);
				
				Some old Pascal-ish. To be removed. */

				warning("STUB: Scrolls::say()");
			}
		}

	}

	/* Here are the procedures that Scroll calls */ /* So they must be... */ /*$F+*/

	void normscroll() {
		warning("STUB: Scrolls::normscroll()");
	}

	void dialogue() {
		warning("STUB: Scrolls::dialogue()");
	}

	void music_scroll();


	static void store_(byte what, Gyro::tunetype &played) {
		memcpy(played, played+1, sizeof(played) - 1);
		played[30] = what;
	}



	static bool they_match(Gyro::tunetype &played) {
		byte fv, mistakes;

		bool they_match_result;
		mistakes = 0;

		for (fv = 1; fv <= sizeof(played); fv ++)
			if (played[fv] != Gyro::tune[fv]) {
				mistakes++;
			}

			they_match_result = mistakes < 5;
			return they_match_result;
	}

	void music_scroll() {
		char r;
		byte value;

		byte last_one, this_one;

		Gyro::tunetype played;


		state(3);
		Gyro::seescroll = true;
		Gyro::on();
		Gyro::newpointer(4);
		//do {
			//do {
			//	Gyro::check(); /* was "checkclick;" */
			//	if (Enhanced::keypressede())  break;
			//} while (!(Gyro::mpress > 0) || buttona1() || buttonb1());
			//
			// Needs joystick - not sure it will be implemented.
			
			//if (Gyro::mpress == 0) {
			//	inkey(); Needs Lucerna to proceed.
			//	
			//}
		//}
	
	}











	

	} // End of namespace Scrolls

} // End of namespace Avalanche
