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
/*#include "Crt.h"*/

/*$R+*/

namespace Avalanche {

struct adxtype {
           varying_string<12> name; /* name of character */
           varying_string<16> comment; /* comment */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
};

integer gd,gm;
untyped_file inf;
adxtype a;
array<1,16000,byte> aa;
array<5,2053,byte> mani;
matrix<0,35,0,4,byte> sil;
byte xw;

void filesetup()
{
      const integer idshould = -1317732048;
 longint id;
 word soa;
;
 assign(inf,"v:sprite2.avd");
 reset(inf,1);
 seek(inf,177);
 blockread(inf,id,4);
 if (id!=idshould) 
 {;
  output << '\7';
  close(inf);
  exit(0);
 }

 blockread(inf,soa,2);
 blockread(inf,a,soa);
}

void loadpic()
{
    byte fv,ff;
;
 {;
  xw=a.xl / 8; if ((a.xl % 8)>0)  xw += 1;

/*  aa[1]:=xl; aa[2]:=0; aa[3]:=yl; aa[4]:=0;*/
   /* The putimage x&y codes are words but xl & yl are bytes, hence the #0s. */
/*  seek(inf,filepos(inf)+xw*(yl+1));*/
  for( fv=0; fv <= a.yl; fv ++)
   blockread(inf,sil[fv],xw);
  blockread(inf,mani,a.size-6);
/*    blockread(inf,aa[5+fv*xw*4+xw*ff],xw);*/
  aa[a.size-1]=0; aa[a.size]=0; /* footer */
 }
/* putimage(0,0,aa,0);*/
}

void plotone(integer xx,integer yy)
{
 word s;
 word ofs,fv;
 byte x,y,z;
;
 {;
  s=imagesize(x,y,xx+a.xl,yy+a.yl);
  getimage(xx,yy,xx+a.xl,yy+a.yl,aa); /* Now loaded into our local buffer. */

  /* Now we've got to modify it! */

  for( x=0; x <= 3; x ++)
   for( y=0; y <= 35; y ++)
    for( z=0; z <= 4; z ++)
    {;
     ofs=5+y*xw*4+xw*x+z;
     aa[ofs]=aa[ofs] & sil[y][z];
    }

 /* mov ax,5   ; AX = ofs
   mov bx,xw  ; wherever we get xw from
   mov cx,x   ; ditto
   mov dx,y   ; ditto
   mul cx,bx  ; x*xw
   mul dx,bx  ; y*yw
   add ax,cx  ; now add 'em all up
   add ax,dx  ; ...
   mov bx,z   ; get z (we don't need x any more)
   mov cx,syz ; get silyz (where from??!)
   add ax,bx  ; add on the last part of the addition
   and ax,cx  ; AND ax with cx. That's it! */

/*
  for x:=1 to 4 do
  begin;
   for y:=0 to 35 do
    for z:=0 to 4 do
    begin;
     ofs:=5+y*xw*4+xw*x+z;
     aa[ofs]:=aa[ofs] xor pic[x,y,z];
    end;
  end;
*/

  for( fv=5; fv <= a.size-2; fv ++)
   aa[fv]=aa[fv] ^ mani[fv];

  /* Now.. let's try pasting it back again! */

  putimage(xx,yy,aa,0);
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 initgraph(gd,gm,"");
 setfillstyle(6,1); bar(0,0,640,200);
 filesetup();
 for( gd=1; gd <= 9; gd ++) loadpic();
 do {
  plotone(Random(500),Random(150));
 } while (!keypressed());
 plotone(0,0);
 close(inf);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.