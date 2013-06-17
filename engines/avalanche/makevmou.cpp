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
/*#include "Squeak.h"*/
/*#include "Crt.h"*/

/*$R+*/

struct mp { /* mouse-pointer */
      matrix<0,1,0,15,word> mask;
      integer horzhotspot,verthotspot;
};

struct vmctype { /* Virtual Mouse Cursor */
           pointer andpic,xorpic;
           array<0,1,pointer> backpic;
           array<0,1,pointtype> wherewas;
           byte picnumber;
           shortint ofsx,ofsy;
};

const array<1,9,arrowtype> mps = 
{{{     /* 1 - up-arrow */
    ((65151,64575,64575,63519,63519,61455,61455,57351,57351,49155,49155,64575,64575,64575,64575,64575),
     (0,384,384,960,960,2016,2016,4080,4080,8184,384,384,384,384,384,0)),
               8,
               0},

{       /* 2 - screwdriver */
    ((8191,4095,2047,34815,50175,61951,63743,64543,65039,65031,65027,65281,65408,65472,65505,65523),
     (0,24576,28672,12288,2048,1024,512,256,224,176,216,96,38,10,12,0)),
               0,
               0},

{       /* 3 - right-arrow */
    ((65535,65535,64639,64543,7,1,0,1,7,64543,64639,65535,65535,65535,65535,65535),
     (0,0,0,384,480,32760,32766,32760,480,384,0,0,0,0,0,0)),
               15,
               6},

{       /* 4 - fletch */
    ((255,511,1023,2047,1023,4607,14591,31871,65031,65283,65281,65280,65280,65409,65473,65511),
     (0,10240,20480,24576,26624,17408,512,256,128,88,32,86,72,20,16,0)),
               0,
               0},

{       /* 5 - hourglass */
    ((0,0,0,34785,50115,61455,61455,63519,63519,61839,61455,49155,32769,0,0,0),
     (0,32766,16386,12300,2064,1440,1440,576,576,1056,1440,3024,14316,16386,32766,0)),
               8,
               7},

{       /* 6 - TTHand */
    ((62463,57855,57855,57855,57471,49167,32769,0,0,0,0,32768,49152,57344,61441,61443),
     (3072,4608,4608,4608,4992,12912,21070,36937,36873,36865,32769,16385,8193,4097,2050,4092)),
               4,
               0},

{       /* 7- Mark's crosshairs */
    ((65535,65151,65151,65151,65151,0,65151,65151,65151,65151,65535,65535,65535,65535,65535,65535),
     (0,384,384,384,384,65535,384,384,384,384,0,0,0,0,0,0)),
               8,
               5},

{       /* 8- I-beam. */
    ((65535,65535,63631,63503,63503,65087,65087,65087,65087,65087,63503,63503,63631,65535,65535,65535),
     (0,0,0,864,128,128,128,128,128,128,128,864,0,0,0,0)),
               8,
               7},

{       /* 9- Question mark. */
    ((511,1023,2047,31,15,8199,32647,65415,63503,61471,61503,61695,63999,63999,61695,61695),
     (65024,33792,34816,34784,40976,57224,32840,72,1936,2080,2496,2304,1536,1536,2304,3840)),
               0,
               0}}};

const integer mouse_size = 134;

const array<1,134,char> mice_header = 
 string("Mouse file copyright (c) 1993. I saw a mouse! Where? SQUEEAAAKK!!! Cheese ")+ /* 74 */
  "cheese cheese. Cheddar, Stilton, Double Gloucester. Squeak."+'\32'; /* 60 */

integer gd,gm;
matrix<0,50,1,40,word> a     /*absolute $A000:0i*/;
byte fv;
vmctype vmc;
byte plot; integer plx,ply;

word swapbits(word a)
{word swapbits_result;
;
 swapbits_result=lo(a)*256+hi(a);
return swapbits_result;
}

void plot_vmc(integer xx,integer yy, byte page_)
{;
 {;
  xx=xx+vmc.ofsx;
  yy=yy+vmc.ofsy;

  getimage(xx,yy,xx+15,yy+15,vmc.backpic[page_]);
  putimage(xx,yy,vmc.andpic,andput);
  putimage(xx,yy,vmc.xorpic,xorput);
  {
  pointtype& with1 = vmc.wherewas[page_]; 
  ;
   with1.x=xx;
   with1.y=yy;
  }
 }
}

void wipe_vmc(byte page_)
{;
  { pointtype& with1 = vmc.wherewas[page_]; 
   if (with1.x!=maxint) 
    putimage(with1.x,with1.y,vmc.backpic[page_],0);}
}

void setup_vmc()
{
    byte fv;
;
/* gd:=imagesize(0,0,15,15);*/

 {;
  getmem(vmc.andpic,mouse_size);
  getmem(vmc.xorpic,mouse_size);

  for( fv=0; fv <= 1; fv ++)
  {;
   getmem(vmc.backpic[fv],mouse_size);
   vmc.wherewas[fv].x=maxint;
  }
 }
}

void show_off_mouse()
{;

 setcolor(14); settextstyle(0,0,2);

 for( gm=0; gm <= 1; gm ++)
 {;
  setactivepage(gm);
  setfillstyle(1,blue); bar(0,0,getmaxx(),getmaxy());
  outtextxy(400,20,chr(48+gm));
 }

 gd=0;
 do {
  setactivepage(gd);
  setvisualpage(1-gd);
  gd=1-gd;

  delay(56);

  getbuttonstatus;
  wipe_vmc(gd);

  if (plot>0) 
  {;
   putpixel(plx,ply,red);
   plot -= 1;
  }

  plot_vmc(mx,my,gd);

  if ((mkey==left) && (plot==0)) 
  {;
   plot=2;
   plx=mx;
   ply=my;
  }

 } while (!(mkey==right));

 for( gm=0; gm <= 1; gm ++)
 {;
  setactivepage(1-gm);
  wipe_vmc(gm);
 }

 setvisualpage(0);
 setactivepage(0);
}

void grab_cursor(byte n)
{;
 getimage(32*n-16, 0,32*n-1,15,vmc.andpic);
 getimage(32*n-16,20,32*n-1,35,vmc.xorpic);
}

void save_mice()
{
 untyped_file f;
 byte fv;
;
 assign(f,"v:mice.avd");
 rewrite(f,1);

 blockwrite(f,mice_header,mouse_size);

 for( fv=1; fv <= 9; fv ++)
  {;
   grab_cursor(fv);
   putimage(100,100,vmc.xorpic,0);
   blockwrite(f,vmc.andpic,mouse_size);
   blockwrite(f,vmc.xorpic,mouse_size);
  }

 close(f);
}

void load_a_mouse(byte which)
{
    untyped_file f;
;
 assign(f,"v:mice.avd");
 reset(f,1);
 seek(f,mouse_size*2*(which-1)+134);

 {;
  blockread(f,vmc.andpic,mouse_size);
  blockread(f,vmc.xorpic,mouse_size);
  close(f);
  {
  void& with1 = mps[which]; 
  ;
   vmc.ofsx=-horzhotspot;
   vmc.ofsy=-verthotspot;

   setminmaxhorzcurspos(horzhotspot,624+horzhotspot);
   setminmaxvertcurspos(verthotspot,199);
  }
 }

}

void draw_mouse_cursors()
{;
 for( fv=1; fv <= 9; fv ++)
  for( gm=0; gm <= 1; gm ++)
   for( gd=0; gd <= 15; gd ++)
    a[gd+gm*20][fv*2]=swapbits(mps[fv].mask[gm][gd]);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 resetmouse;
 setup_vmc();

 draw_mouse_cursors();


 save_mice();

/* grab_cursor(3);*/
/* load_a_mouse(4);
 setgraphicscursor(mps[4]);

 show_off_mouse;
 on; repeat getbuttonstatus until mkey=left; off;
 show_off_mouse;*/
return EXIT_SUCCESS;
}
