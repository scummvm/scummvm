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

#include "graph.h"


namespace Avalanche {

integer gd, gm;
int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");
	/* Screen size: Starts at $A000:$320 (coords 0,10).
	Length: 12000 (150*80) bytes.
	If we divide it into sets of 5 lines (400 bytes) each, there are 30.
	There are 40 words on a line.

	Patterns:
	 4AAA = "AVVY"
	 AAAE
	 EAA4
	 A444

	 A4AA = "YAVV"
	 AEAA
	 4EAA
	 4A44 */
	/*
	 asm
	  mov bx,$A000;
	  mov es,bx; { The segment to copy it to... }
	  mov di,$370;  { The offset (10 pixels down, plus 1 offset.) }

	  mov cx,10;
	  mov ax,$AA4A; call @sameline; { Print "AVVY" }
	  mov ax,$AEAA; call @sameline;
	  mov ax,$A4EA; call @sameline;
	  mov ax,$44A4; call @sameline;

	  mov cx,9;
	  mov ax,$AAA4; call @sameline; { Print "YAVV" }
	  mov ax,$AAEA; call @sameline;
	  mov ax,$AA4E; call @sameline;
	  mov ax,$444A; call @sameline;

	  mov ax,$4AAA; call @sameline; { Print "VYAV" }
	  mov ax,$AAAE; call @sameline;
	  mov ax,$EAA4; call @sameline;
	  mov ax,$A444; call @sameline;

	  mov ax,$A4AA; call @sameline; { Print "VVYA" }
	  mov ax,$EAAA; call @sameline;
	  mov ax,$4EAA; call @sameline;
	  mov ax,$4A44; call @sameline;

	  jmp @the_end;

	  { Replicate the same line many times. }

	  @sameline:
	   { Requires:
	      what to copy in AX,
	      how many lines in CX, and
	      original offset in DI. }
	   push cx;
	   push di;

	   @samelineloop:

	    push cx;
	    mov cx,40; { No. of times to repeat it on one line. }

	    repz stosw; { Fast word-copying }

	    pop cx;

	    add di,1200; { The next one will be 16 lines down. }

	   loop @samelineloop;
	   pop di;
	   add di,80;
	   pop cx;

	   ret;

	  @the_end:
	 end;
	*/
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.