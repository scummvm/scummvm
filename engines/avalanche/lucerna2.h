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

/* LUCERNA		The screen, [keyboard] and mouse handler.*/

#ifndef AVALANCHE_LUCERNA2_H
#define AVALANCHE_LUCERNA2_H

#include "common/scummsys.h"
#include "common/file.h"

namespace Avalanche {
class AvalancheEngine;

class Lucerna {
public:
	bool holdLeftMouse;

	Lucerna(AvalancheEngine *vm);
	~Lucerna();

	void init();
	void callVerb(byte id);
	void draw_also_lines();
	void mouse_init();
	void mousepage(uint16 page_);
	void load(byte n);
	void exitroom(byte x);
	void enterroom(byte x, byte ped);
	void thinkabout(byte z, bool th);      // Hey!!! Get it and put it!!!
	void load_digits();    // Load the scoring digits & rwlites
	void toolbar();
	void showscore();
	void points(byte num);      // Add on no. of points
	void mouseway(const Common::Point &cursorPos);
	void posxy();
	void fxtoggle();
	void objectlist();
	void checkclick();
	void errorled();
	void dusk();
	void dawn();
	void showrw();
	void clock_lucerna();
	void delavvy();
	void gameover();
	void minor_redraw();
	void major_redraw();
	uint16 bearing(byte whichped); // Returns the bearing from ped Whichped to Avvy, in degrees.
	void flesh_colors();
	void sprite_run();
	void fix_flashers();
	void load_also(Common::String n);

private:
	AvalancheEngine *_vm;

	struct rgbrec {
		int16 red;
		int16 green;
		int16 blue;
	};

	struct palettetype {
		int32 size;
		rgbrec colors[256];
	};

	//Clock
	static const int16 _clockCenterX = 510;
	static const int16 _clockCenterY = 183;
	Common::Point ah, am;
	uint16 nh;


	palettetype fxpal[4];

	bool fxhidden;

	Common::File f;




	Common::String nextstring();
	void scram1(Common::String &x);
	void unscramble();

	void zoomout(int16 x, int16 y);

	void find_people(byte room);

	void new_town();

	void put_geida_at(byte whichped, byte &ped);

	void topcheck(Common::Point cursorPos);

	void verte(Common::Point cursorPos);

	int8 fades(int8 x);

	void fadeout(byte n);

	void fadein(byte n);

	// clock_lucerna
	void calchand(uint16 ang, uint16 length, Common::Point &a, byte c);
	void hand(const Common::Point &a, byte c);
	void refresh_hands();
	void plothands();
	void chime();

};

} // End of namespace Avalanche

#endif // AVALANCHE_LUCERNA2_H
