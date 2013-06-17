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

namespace Avalanche {

struct adxtype {
           varying_string<12> name; /* name of character */
           varying_string<16> comment; /* comment */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
           byte accinum; /* the number according to Acci (1=Avvy, etc.) */
};

integer gd,gm;
untyped_file sf;
longint id;
word soa;
adxtype a;
byte xw;
array<5,2053,byte> mani;
matrix<0,35,0,4,byte> sil;
array<1,16000,byte> aa;
untyped_file outfile;

void plotat(integer xx,integer yy) /* Does NOT cameo the picture!*/
{
    word soaa;
;
 move(mani,aa[5],sizeof(mani));
 {;
  aa[1]=a.xl; aa[2]=0; aa[3]=a.yl; aa[4]=0; /* set up x&y codes */
 }
 putimage(xx,yy,aa,0);
 soaa=sizeof(mani);
 blockwrite(outfile,soaa,2);
 blockwrite(outfile,aa,sizeof(mani));
}

const integer shouldid = -1317732048;

void explode(byte which)       /* 0 is the first one! */
 /* Each character takes five-quarters of (a.size-6) on disk. */
{
 byte fv,ff; word so1; /* size of one */
;
 {;
  so1=a.size-6; so1 += so1 / 4;
  seek(sf,183+soa+so1*which); /* First is at 221 */
/*  where:=filepos(sf);*/
  xw=a.xl / 8; if ((a.xl % 8)>0)  xw += 1;

  for( fv=0; fv <= a.yl; fv ++)
   blockread(sf,sil[fv],xw);
  blockread(sf,mani,a.size-6);
  aa[a.size-1]=0; aa[a.size]=0; /* footer */
 }
 plotat(100,100);
 delay(100);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 assign(outfile,"notts.avd");
 reset(outfile,1);
 seek(outfile,filesize(outfile));

 assign(sf,"sprite0.avd");
 reset(sf,1);

 seek(sf,177);
 blockread(sf,id,4);
 blockread(sf,soa,2);
 blockread(sf,a,soa);

 explode(1);
 for( gd=6; gd <= 11; gd ++) explode(gd);
 for( gd=18; gd <= 23; gd ++) explode(gd);

 close(sf);
 close(outfile);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.