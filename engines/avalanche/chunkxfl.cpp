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

#include "graph.h"
/*#include "Tommys.h"*/
/*#include "Crt.h"*/
#include "cadburys.h"


void finder()
{
    char r; integer x,y;

 setfillstyle(0,0); setcolor(14);
 x=320; y=100; setwritemode(xorput);
 do {
  bar(0,0,200,10);
  outtextxy(0,0,strf(x)+','+strf(y));
  line(x-20,y,x+20,y); line(x,y-20,x,y+20);
  do {; } while (!keypressed());
  line(x-20,y,x+20,y); line(x,y-20,x,y+20);
  switch (readkey()) {
   case '\15': return; break;
   case '8': y -= 10; break;
   case '4': x -= 10; break;
   case '6': x += 10; break;
   case '2': y += 10; break;
    case '\0': switch (readkey()) {
         case cup: y -= 1; break;
         case cdown: y += 1; break;
         case cleft: x -= 1; break;
         case cright: x += 1; break;
        }
        break;
  }
 } while (!false);
}

void load()
{
 byte a1 /*absolute $A400:800*/;
 byte bit;
 untyped_file f;


 assign(f,"place51.avd"); reset(f,1);
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a1,12080);
 }

 close(f);
 bit=getpixel(0,0);
 setvisualpage(1); setactivepage(1);
 finder();
 setvisualpage(0); setactivepage(0);
}

void loadtemp()
{
 byte a0 /*absolute $A000:800*/;
 byte bit;
 untyped_file f;


 assign(f,"chunkbi4.avd"); reset(f,1);
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a0,12080);
 }
 close(f);

 bit=getpixel(0,0);

 finder();
}

void open_chunk()
{
 assign(f,"chunk51.avd");
 rewrite(f,1);
 blockwrite(f,chunkheader,sizeof(chunkheader));
 blockwrite(f,num_chunks,1);
 blockwrite(f,offsets,num_chunks*4);

 this_chunk=0;
}

void close_chunk()
{
 seek(f,45);
 blockwrite(f,offsets,num_chunks*4); /* make sure they're right! */
 close(f);
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 load();
 loadtemp();
 setwritemode(xorput);

 num_chunks=9;

 open_chunk();

 grab(346,119,404,154,539,116,ch_ega,true ,false); /*1 fire */
 grab(435,119,490,154,541,116,ch_ega,true ,false); /*2 fire */
 grab(435,119,490,154,541,116,ch_bgi,true ,true ); /*3 natural fire */

 grab(300, 58,315, 68,258, 95,ch_ega,true ,false); /*6 Duck's head 2 */
 grab(246, 52,259, 62,258, 95,ch_ega,true ,false); /*5 Duck blinks 1 */

 grab(300, 58,315, 68,258, 95,ch_bgi,true ,true ); /*4 Duck's head 1 */
 grab(262, 52,278, 62,257, 95,ch_ega,true ,false); /*7 Duck blinks 2 */

 grab(333, 58,347, 68,258, 95,ch_ega,true ,false); /*8 Duck's head 3 */
 grab(250, 63,265, 73,258, 95,ch_ega,true ,false); /*9 Duck blinks 3 */

 close_chunk();
 return EXIT_SUCCESS;
}
