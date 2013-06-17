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


enum flavourtype {ch_ega,ch_bgi, last_flavourtype};

struct chunkblocktype {
                  flavourtype flavour;
                  integer x,y;
                  integer xl,yl;
                  longint size;
                  boolean natural;

                  boolean memorise; /* Hold it in memory? */
};

untyped_file f;
longint offset;
chunkblocktype ch;
integer gd,gm;
byte bit;
pointer p;

void mdrop(integer x,integer y,integer xl,integer yl, pointer p) /* assembler;
asm
  push ds;      { Strictly speaking, we shouldn't modify DS, so we'll save it.}
  push bp;      { Nor BP! }


  { DI holds the offset on this page. It starts at the top left-hand corner. }
  { (It should equal ch.y*80+ch.x. }

  mov ax,y;
  mov dl,80;
  mul dl; { Line offset now calculated. }
  mov di,ax; { Move it into DI. }
  mov ax,x;
  add di,ax; { Full offset now calculated. }

  mov bx,yl; { No. of times to repeat lineloop. }
  inc bx;        { "loop" doesn't execute the zeroth time. }
  mov bh,bl;     { Put it into BH. }

  { BP holds the length of the string to copy. It's equal to ch.xl.}

  mov ax,word(p);   { Data is held at DS:SI. }
  mov si,ax;
  mov ax,word(p+2); { This will be moved over into ds in just a tick... }

  mov bp,xl;

  mov ds,ax;


  cld;          { We're allowed to hack around with the flags! }

  mov ax,$A000; { Top of the first EGA page. }
  mov es,ax;    { Offset on this page is calculated below... }


{    port[$3c4]:=2; port[$3ce]:=4; }

  mov dx,$3c4;
  mov al,2;
  out dx,al;
  mov dx,$3ce;
  mov al,4;
  out dx,al;

  mov cx,4;  { This loop executes for 3, 2, 1, and 0. }
  mov bl,0;


 @mainloop:

    push di;
    push cx;

{    port[$3C5]:=1 shl bit; }
    mov dx,$3C5;
    mov al,1;
    mov cl,bl; { BL = bit. }
    shl al,cl;
    out dx,al;
{     port[$3CF]:=bit; }
    mov dx,$3CF;
    mov al,bl; { BL = bit. }
    out dx,al;

    xor ch,ch;
    mov cl,bh; { BH = ch.yl. }

   @lineloop:

     push cx;

     mov cx,bp;

     repz movsb; { Copy the data. }

     sub di,bp;
     add di,80;

     pop cx;

   loop @lineloop;

    inc bl; { One more on BL. }

    pop cx;
    pop di;

 loop @mainloop;

  pop bp;
  pop ds;       { Get DS back again. }
*/
{;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 assign(f,"chunk21.avd");
 reset(f,1);

 seek(f,49);
 blockread(f,offset,4);

 seek(f,offset);

 blockread(f,ch,sizeof(ch));

 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 getmem(p,ch.size);
 blockread(f,p,ch.size);
/* putimage(0,0,p^,0);*/


/* with ch do
  for bit:=0 to 3 do
  begin;
   port[$3c4]:=2; port[$3ce]:=4; port[$3C5]:=1 shl bit; port[$3CF]:=bit;
   for gm:=0 to yl do
    blockread(f,mem[$A000*gm*80],(ch.xl+7) div 8);
  end;*/

  mdrop(ch.x,ch.y,ch.xl,ch.yl,p);

 close(f);
return EXIT_SUCCESS;
}
