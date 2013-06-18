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

#define __joystick_implementation__

/*
Copyright (c) 1989, 1990 David B. Howorth

Requires Turbo Pascal 5.0 or later.

Unit last revised May 9, 1989.
This comment last revised October 22, 1990.

This file, when compiled to disk, creates JOYSTICK.TPU, a Turbo Pascal unit
containing all necessary routines for joystick control.  The routines can
be demonstrated by running the accompanying program JOYDEMO (after first
compiling JOYSTICK.PAS to disk).

For further information see the accompanying file, JOYSTICK.DOC.

Permission is granted to distribute this file and the accompanying files
(JOYDEMO.PAS and JOYSTICK.DOC) provided (1) all three files are distributed
together and (2) no fee is charged.

Permission is granted to include compiled versions of the routines in these
files in any program, commercial or noncommercial, provided only that if the
program is distributed, whether commercially or noncommercially, a copy
(including any documentation) be sent to me; and, if you distribute your
program as shareware, treat me as registered.  My address is 01960 SW Palatine
Hill Road, Portland, Oregon 97219.
*/

#include "joystick.h"


/*#include "Dos.h"*/

namespace Avalanche {

typedef void(*readjoyproc)(byte a, byte b, word &c, word &d);
typedef boolean(*buttonfunc)(byte a);

readjoyproc readjoy;
buttonfunc button;
registers reg;

/*----------------------------- private routines ----------------------------*/

boolean newbios() {
	char decadechar   /*absolute $F000:$FFFB*/;
	char yearchar   /*absolute $F000:$FFFC*/;

	boolean newbios_result;
	newbios_result = (set::of('9', '0', eos).has(decadechar)) /*an optimistic view of software life*/
	                 || ((decadechar == '8') && (set::of(range('4', '9'), eos).has(yearchar)));
	return newbios_result;
}

/*$F+*/

void oldreadjoy(byte xbit, byte ybit, word &xaxis, word &yaxis) {
	/*
	inline(
	  $BA/$01/$02/    {mov  dx, 201h      ;load dx with joystick port address   }
	  $C4/$BE/>XAxis/ {les  di, XAxis[bp] ;load es with segment and di w/offset }
	  $8A/$66/<xbit/  {mov  ah, xbit[bp]  ;set appropriate bit in ah            }
	  $E8/$0C/$00/    {call SUBR                                                }
	  $C4/$BE/>YAxis/ {les  di, YAxis[bp]                                       }
	  $8A/$66/<ybit/  {mov  ah, ybit[bp]  ;set appropriate bit in ah            }
	  $E8/$02/$00/    {call SUBR                                                }
	  $EB/$1D/        {jump short END     ;we're done!                          }
	                  {SUBR:              ;first wait, if necessary, until      }
	                  {                   ; relevant bit is 0:                  }
	  $B9/$FF/$FF/    {       mov  cx, 0ffffh ;fill cx to the brim              }
	  $EC/            {WAIT:  in   al, dx     ;get input from port 201h         }
	  $84/$E0/        {       test al, ah     ;is the relevant bit 0 yet?       }
	  $E0/$FB/        {       loopne WAIT     ;if not, go back to wait          }

	  $B9/$FF/$FF/    {       mov  cx, 0ffffh ;fill cx to the brim again        }
	  $FA/            {       cli             ;disable interrupts               }
	  $EE/            {       out  dx, al     ;'nudge' port 201h                }
	  $EC/            {AGAIN: in   al, dx     ;get input from port 201h         }
	  $84/$E0/        {       test al, ah     ;is the relevant bit 0 yet?       }
	  $E0/$FB/        {       loopne AGAIN    ;if not, go back to AGAIN         }
	  $FB/            {       sti             ;reenable interrupts              }
	  $F7/$D9/        {       neg  cx         ;negative cx                      }
	  $81/$C1/$FF/$FF/{       add  cx, 0ffffh ;add 0ffffh back to value in cx   }
	  $26/            {       es:             ;segment override                 }
	  $89/$0D/        {       mov  [di], cx   ;store value of cx in location    }
	                  {                       ; of relevant variable            }
	  $C3);           {       ret                                               }
	                  {END:                                                     }
	*/
}    /* OldReadJoy */

void newreadjoy(byte which, byte meaningless, word &xaxis, word &yaxis) {
	reg.ah = 0x84;
	reg.dx = 1;
	intr(0x15, reg);
	if (which == 1) {
		xaxis = reg.ax;
		yaxis = reg.bx;
	} else {
		xaxis = reg.cx;
		yaxis = reg.dx;
	}
}

boolean oldbutton(byte mask) {
	boolean oldbutton_result;
	oldbutton_result = ((port[0x201] & mask) == 0);
	return oldbutton_result;
}

boolean newbutton(byte mask) {
	boolean newbutton_result;
	reg.ah = 0x84;
	reg.dx = 0;
	intr(0x15, reg);
	newbutton_result = ((reg.al & mask) == 0);
	return newbutton_result;
}

/*$F-*/

/*----------------------------- public routines -----------------------------*/

void readjoya(word &xaxis, word &yaxis) {
	readjoy(1, 2, xaxis, yaxis);
}

void readjoyb(word &xaxis, word &yaxis) {
	readjoy(4, 8, xaxis, yaxis);
}

boolean buttona1() {
	boolean buttona1_result;
	buttona1_result = button(0x10);
	return buttona1_result;
}

boolean buttona2() {
	boolean buttona2_result;
	buttona2_result = button(0x20);
	return buttona2_result;
}

boolean buttonb1() {
	boolean buttonb1_result;
	buttonb1_result = button(0x40);
	return buttonb1_result;
}

boolean buttonb2() {
	boolean buttonb2_result;
	buttonb2_result = button(0x80);
	return buttonb2_result;
}

boolean joystickpresent() {
	boolean joystickpresent_result;
	intr(0x11, reg);
	joystickpresent_result = ((reg.ax & 0x1000) != 0);
	return joystickpresent_result;
}

/*------------------------------ initialization -----------------------------*/

class unit_joystick_initialize {
public:
	unit_joystick_initialize();
};
static unit_joystick_initialize joystick_constructor;

unit_joystick_initialize::unit_joystick_initialize() {
	if (newbios()) {
		/* use BIOS routines */
		readjoy = newreadjoy;
		button = newbutton;
	} else {                           /* use work-around routines */
		readjoy = oldreadjoy;
		button = oldbutton;
	}
}

} // End of namespace Avalanche.