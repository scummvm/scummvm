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

/*#include "Crt.h"*/
/*#include "Dos.h"*/

namespace Avalanche {

matrix<1,7,1,3,byte> cols;
byte fv;

void adjust()
{
 registers r;
;
 {;
  ax=0x1012;
  bx=1;
  cx=2;
  es=seg(cols);
  dx=ofs(cols);

 }

 intr(0x10,r);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=0;
 clrscr;

 fillchar(cols,sizeof(cols),'\0');
 adjust();

 gotoxy(29,10); textattr=1; output << "Thorsoft of Letchworth";
 gotoxy(36,12); textattr=2; output << "presents";

 for( fv=1; fv <= 77; fv ++)
 {;
  delay(77);
  if (fv<64)  fillchar(cols[1],3,chr(fv));
  if (fv>14)  fillchar(cols[2],3,chr(fv-14));
  adjust();
 }

 delay(100);

 for( fv=63; fv >= 1; fv --)
 {;
  fillchar(cols,sizeof(cols),chr(fv));
  delay(37);
  adjust();
 }

return EXIT_SUCCESS;
}

} // End of namespace Avalanche.