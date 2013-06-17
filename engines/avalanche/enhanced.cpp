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

#define __enhanced_implementation__


/* This is the unit set up by Thomas with help from all the people on
  CIS:BPROGA to read the *enhanced* keyboard codes (as opposed to the
  readkey-type ones.) */

#include "enhanced.h"


/*#include "Dos.h"*/
/*#include "Crt.h"*/


boolean isenh()
{
 byte statefrom16;
 registers r;

  boolean isenh_result;
  isenh_result = false;
  {;
   ah=0x12;
   intr(0x16,r);
   statefrom16=al;
  }
  if (statefrom16 != shiftstate)  return isenh_result;
  shiftstate = shiftstate ^ 0x20;
  {;
   ah=0x12;
   intr(0x16,r);
   statefrom16=al;
  }
  isenh_result = statefrom16 == shiftstate;
  shiftstate = shiftstate ^ 0x20;
  return isenh_result;
}

void readkeye()
/*  function fancystuff:word;
   inline( $B4/ $10/  { MOV AH,10 }
           $CD/ $16); { INT 16 }
  function notfancystuff:word;
   inline( $B4/ $00/  { MOV AH,0 }
           $CD/ $16); { INT 16 }
*/
{
 registers r; word fs;
;
 if (atbios) 
            fs=fancystuff; /* We're using an AT */
  else fs=notfancystuff;  /* ditto, an XT */
 inchar=chr(lo(fs));
 extd=chr(hi(fs));
}

boolean keypressede()
/*
 function fancystuff:boolean;
  inline( $B4/ $11/  { MOV AH,11 }
          $CD/ $16/  { INT 16 }
          $B8/ $00/ $00/ { MOV AX, 0000 }
          $74/ $01/  { JZ 0112 (or wherever- the next byte after $40, anyway) }
          $40);      { INC AX }
*/
{
    registers r;
boolean keypressede_result;
;
 if (atbios) 
   keypressede_result=fancystuff; /* ATs get the fancy stuff */
  else keypressede_result=keypressed(); /* XTs get the usual primitive... */
return keypressede_result;
}

class unit_enhanced_initialize {
  public: unit_enhanced_initialize();
};
static unit_enhanced_initialize enhanced_constructor;

unit_enhanced_initialize::unit_enhanced_initialize() {;
 /* determine bios type */
 atbios=isenh();
}
