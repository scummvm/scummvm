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

#ifndef LUCERNA2_H
#define LUCERNA2_H

#include "common/scummsys.h"

namespace Avalanche {
class AvalancheEngine;

class Lucerna {
public:
	Lucerna();

	void setParent(AvalancheEngine *vm);

	void callverb(char n);

	void draw_also_lines();

	void mouse_init();

	void mousepage(uint16 page_);

	void load(byte n);

	void exitroom(byte x);

	void enterroom(byte x, byte ped);

	void thinkabout(char z, bool th);      /* Hey!!! Get it and put it!!! */

	void load_digits();    /* Load the scoring digits & rwlites */

	void toolbar();

	void showscore();

	void points(byte num);      /* Add on no. of points */

	void mouseway();

	void inkey();

	void posxy();

	void fxtoggle();

	void objectlist();

	void checkclick();

	void errorled();

	void dusk();

	void dawn();

	void showrw();

	void mblit(byte x1, byte y1, byte x2, byte y2, byte f, byte t); /* The Minstrel Blitter */

	void blitfix();

	void clock_lucerna();

	void flip_page();

	void delavvy();

	void gameover();

	void minor_redraw();

	void major_redraw();

	uint16 bearing(byte whichped);

	void flesh_colours();

	void sprite_run();

	void fix_flashers();

private:
	AvalancheEngine *_vm;

	bool fxhidden;

	struct rgbrec {
		int16 red;
		int16 green;
		int16 blue;
	};

	struct palettetype {
		int32 size;
		rgbrec colors[256];
	};

	palettetype fxpal[4];
};

} // End of namespace Avalanche


#endif // LUCERNA2_H
