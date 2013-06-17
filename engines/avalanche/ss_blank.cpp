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

/*#include "Dos.h"*/
/*#include "Crt.h"*/

byte fv;
boolean test;

boolean the_cows_come_home()
{
    registers rmove,rclick;
boolean the_cows_come_home_result;
;
 rmove.ax=11; intr(0x33,rmove);
 rclick.ax=3; intr(0x33,rclick);
 the_cows_come_home_result=
   (keypressed()) | /* key pressed */
   (rmove.cx>0) || /* mouse moved */
   (rmove.dx>0) ||
   (rclick.bx>0);  /* button clicked */
return the_cows_come_home_result;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 test=the_cows_come_home();
 textattr=0; clrscr;
 do {; } while (!the_cows_come_home());
 textattr=30; clrscr;
 output << "*** Blank Screen *** (c) 1992, Thomas Thurman. (An Avvy Screen Saver.)" << NL;
 for( fv=1; fv <= 46; fv ++) output << '~'; output << NL;
 output << "This program may be freely copied." << NL;
 output << NL;
 output << "Have fun!" << NL;
return EXIT_SUCCESS;
}