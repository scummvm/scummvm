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

namespace Avalanche {

file<char> inf,outf;
char x;
string q;
byte fv;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 assign(inf,"v:thank.you");
 assign(outf,"d:hibits.out");
 reset(inf); rewrite(outf);

 q=string('\40')+"(Seven is a bit of a lucky number.)"+'\40'+'\215'+'\212'+'\212';

 for( fv=1; fv <= length(q); fv ++) outf << q[fv];

 while (! eof(inf)) 
 {;
  inf >> x;
  if (x<'\200')  x += 128;
  outf << x;
 }
 close(inf); close(outf);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.