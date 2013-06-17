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

const string crlf = string('\15')+'\12'; const char eof_ = '\32';
const array<1,177,char> trip5head = 
         string("Sprite*.AVD  ...  data file for Trippancy Five")+crlf+crlf+
         "[Thorsoft relocatable fiveplane sprite image format]"+crlf+crlf+
         "Thomas Thurman was here.  ...  Have fun!"+crlf+crlf+eof_+
         "±±±±±±± * G. I. E. D. ! * ";

const array<1,4,char> tripid = string('\x30')+'\x1'+'\x75'+'\261';

const array<1,50,char> trip5foot = crlf+crlf+
         " and that's it! Enjoy the game. "+'\3'+crlf+crlf+
            "\n\n\n\n\n\n\n"+"tt";

struct adxotype {
           varying_string<12> name; /* name of character */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
};

struct adxtype {
           varying_string<12> name; /* name of character */
           varying_string<16> comment; /* comment */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
};

varying_string<2> sn;
adxotype oa;
adxtype a;
matrix<1,24,0,1,pointer> pic;     /* the pictures themselves */
array<1,16000,byte> aa;
untyped_file out;
integer bigsize;

void copyaoa()
{;
 {;
  a.name=oa.name;
  a.comment="Transferred";
  a.num=oa.num;
  a.xl=oa.xl;
  a.yl=oa.yl;
  a.seq=oa.seq;
  a.size=oa.size;
  a.fgc=oa.fgc;
  a.bgc=oa.bgc;
 }
}

void load()
{
 untyped_file f; byte gd,gm,sort,n; pointer p,q;
 pointer xf;
;
 assign(f,string("v:osprte")+sn+".avd"); reset(f,1); seek(f,59);
 blockread(f,oa,sizeof(oa)); blockread(f,bigsize,2);
 copyaoa();

 getmem(xf,a.size);

 for( sort=0; sort <= 1; sort ++)
 {;
  mark(q); getmem(p,bigsize);
  blockread(f,p,bigsize);
  putimage(0,0,p,0); release(q); n=1;

  if (sort==0)  setfillstyle(1,15); else setfillstyle(1,0);
  bar(177,125,300,200);

   for( gm=0; gm <= (a.num / a.seq)-1; gm ++) /* directions */
    for( gd=0; gd <= a.seq-1; gd ++) /* steps */
    {;
     getmem(pic[n][sort],a.size); /* grab the memory */
     getimage((gm / 2)*(a.xl*6)+gd*a.xl,(gm % 2)*a.yl,
       (gm / 2)*(a.xl*6)+gd*a.xl+a.xl-1,(gm % 2)*a.yl+a.yl-1,
       xf);
     putimage(177,125,xf,0);
     getimage(177,125,177+a.xl,125+a.yl,pic[n][sort]); /* grab the pic */
     n += 1;
   }
 }
 close(f);
 freemem(xf,a.size);
 cleardevice();
  for( gm=0; gm <= 1; gm ++)
   for( gd=1; gd <= a.num; gd ++)
    putimage(gd*15,gm*40,pic[gd][gm],0);
}

void setup()
{
    integer gd,gm;
;
 output << "TRIP5XF (c) 1992, Thomas Thurman." << NL; output << NL;
 output << "Enter number of SPRITE*.AVD file to convert:"; input >> sn >> NL;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 load();
}

string strf(longint x)
{
    string q;
string strf_result;
;
 str(x,q); strf_result=q;
return strf_result;
}

void save()
{
 byte sort,n;
 word fv,ff; char r; byte xw;
 byte nxl,nyl;
 word soa;
;
 cleardevice();
 {;
  a.size=imagesize(0,0,a.xl,a.yl);
  soa=sizeof(a);

  assign(out,string("v:sprite")+sn+".avd"); rewrite(out,1);
  blockwrite(out,trip5head,177);
  blockwrite(out,tripid,4);
  blockwrite(out,soa,2);
  blockwrite(out,a,soa);

  nxl=a.xl; nyl=a.yl;
  xw=nxl / 8;
  if ((nxl % 8)>0)  xw += 1;

  for( n=1; n <= a.num; n ++)
  {;
   putimage(  0,0,pic[n][0],0);
   getimage(  0,0,a.xl,a.yl,aa);
   for( fv=0; fv <= nyl; fv ++)
    blockwrite(out,aa[5+fv*xw*4],xw);

   putimage(100,0,pic[n][1],0);
   getimage(100,0,100+a.xl,a.yl,aa);
   putimage(100,100,aa,4);
/*   for ff:=1 to 4 do        { actually 2 to 5, but it doesn't matter here }
    for fv:=0 to nyl do*/
/*   for ff:=5 to size-2 do
    blockwrite(out,aa[ff],1);*/
   blockwrite(out,aa[5],a.size-6);
  }
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 setup();
 save();

 blockwrite(out,trip5foot,50);
 close(out);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.