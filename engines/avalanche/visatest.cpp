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

namespace Avalanche {

char block;
word point;

array<1,2000,char> result;
word result_len;

void unskrimble()
{
    word fv;

 for( fv=1; fv <= 2000; fv ++) result[fv]=(char)((~(ord(result[fv])-fv)) % 256);
}

void visa_get_scroll(char block, word point)
{
 untyped_file indexfile,sezfile;
 word idx_offset,sez_offset;

 assign(indexfile,"avalot.idx"); assign(sezfile,"avalot.sez");

 reset(indexfile,1);
 seek(indexfile,(ord(upcase(block))-65)*2);
 blockread(indexfile,idx_offset,2);
 seek(indexfile,idx_offset+point*2);
 blockread(indexfile,sez_offset,2);
 close(indexfile);

 reset(sezfile,1);
 seek(sezfile,sez_offset);
 blockread(sezfile,result_len,2);
 blockread(sezfile,result,result_len);
 close(sezfile);
 unskrimble();
}

void access_get_scroll(char block, word point)
{
 string x;
 untyped_file f;

 str(point,x);
 x=string('S')+block+x+".RAW";
 assign(f,x);
 reset(f,1);
 result_len=filesize(f);
 blockread(f,result,result_len);
 close(f);
}

void display_it()
{
    word fv;

 for( fv=1; fv <= result_len; fv ++) output << result[fv];
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 do {
  output << NL;
  output << NL;
  output << "Block?"; input >> block >> NL;
  output << "Point?"; input >> point >> NL;

  output << "ACCESS reports (this one is always correct):" << NL;
  output << NL; 
  access_get_scroll(block,point);
  display_it();

  output << NL; output << NL;
  output << "VISA reports:" << NL;
  output << NL;
  visa_get_scroll(block,point);
  display_it();
 } while (!false);
 return EXIT_SUCCESS;
}

} // End of namespace Avalanche.