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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef __lucerna_h__
#define __lucerna_h__

namespace Avalanche {

#include "gyro.h"


void callverb(char n);

void draw_also_lines();

void mouse_init();

void mousepage(word page_);

void load(byte n);

void exitroom(byte x);

void enterroom(byte x, byte ped);

void thinkabout(char z, boolean th);      /* Hey!!! Get it and put it!!! */

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

void clock();

void flip_page();

void delavvy();

void gameover();

void minor_redraw();

void major_redraw();

word bearing(byte whichped);

void flesh_colours();

void sprite_run();

void fix_flashers();

} // End of namespace Avalanche.

#endif