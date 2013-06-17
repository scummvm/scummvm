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
/*#include "Dos.h"*/

namespace Avalanche {

const array<false,true,varying_string<7> > names = {{"Avalot","Dogfood"}};

array<1,3,byte> old,stones;
array<0,3,0,22,1,7,byte> stonepic;       /* picture of Nimstone */
byte turns;
boolean dogfoodsturn; byte fv; byte stonesleft;

boolean clicked;

byte row,number;

boolean squeak;
shortint mnum,mrow;

registers r;

void resetmouse()
{;
 r.ax=0; intr(0x33,r);
 squeak=r.ax>0;
}

void show()
{;
 r.ax=1; intr(0x33,r);
}

void hide()
{;
 r.ax=2; intr(0x33,r);
}

void chalk(integer x,integer y, string z)
{
      const array<0,3,byte> greys = {{0,8,7,15}};
    byte fv;
;
 for( fv=0; fv <= 3; fv ++)
 {;
  setcolor(greys[fv]);
  outtextxy(x-fv,y,z);
  sound(fv*100*length(z)); delay(3); nosound; delay(30);
 }
}

void setup()
{
 untyped_file f;
 integer gd,gm;
 byte bit;
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 assign(f,"c:\\avalot\\nim.avd");
 reset(f,1);
 seek(f,41);
 for( gm=0; gm <= 22; gm ++)
  for( bit=0; bit <= 3; bit ++)
  {;
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   blockread(f,stonepic[bit][gm],7);
  }
 for( gd=1; gd <= 3; gd ++)
  for( gm=0; gm <= 22; gm ++)
   for( bit=0; bit <= 3; bit ++)
   {;
    port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
    blockread(f,mem[0xa000*3200+gd*2800+gm*80],7);
   }
 for( gm=0; gm <= 36; gm ++)
  for( bit=0; bit <= 3; bit ++)
  {;
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   blockread(f,mem[0xa000*400+49+gm*80],30);
  }
 close(f);

 gd=getpixel(0,0);  /* clear codes */
 setcolor(4); rectangle(394,50,634,197);
 setfillstyle(1,6); bar(395,51,633,196);
 rectangle(10,5,380,70); bar(11,6,379,69);
 setcolor(15);
 outtextxy(475,53,"SCOREBOARD:");
 setcolor(14);
 outtextxy(420,63,"Turn:");
 outtextxy(490,63,"Player:");
 outtextxy(570,63,"Move:");

 for( gd=1; gd <= 3; gd ++) stones[gd]=gd+2;

 turns=0; dogfoodsturn=true;

 chalk(27,15,"Take pieces away with:");
 chalk(77,25,"1) the mouse (click leftmost)");
 chalk(53,35,"or 2) the keyboard:");
 chalk(220,35,string('\30')+'/'+'\31'+": choose row,");
 chalk(164,45,string("+/- or ")+'\33'+'/'+'\32'+": more/fewer,");
 chalk(204,55,"Enter: take stones.");

 row=1; number=1; fillchar(old,sizeof(old),'\0'); stonesleft=12;
 resetmouse();
}

void plotstone(byte x,byte y)
{
    byte fv,bit; word ofs;
;
 ofs=3200+y*2800+x*8;
 for( fv=0; fv <= 22; fv ++)
  for( bit=0; bit <= 3; bit ++)
  {;
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   move(stonepic[bit][fv],mem[0xa000*ofs+fv*80],7);
  }
}

void board()
{
    byte fv,ff;
;
 for( fv=1; fv <= 3; fv ++)
  for( ff=1; ff <= stones[fv]; ff ++)
   plotstone(ff,fv);
}

void startmove()
{
    varying_string<2> tstr; integer ypos;
;
 turns += 1; str(turns,2,tstr); ypos=63+turns*10;
 dogfoodsturn=! dogfoodsturn;
 chalk(433,ypos,tstr);
 chalk(493,ypos,names[dogfoodsturn]);
 old=stones;
}

void show_changes()
{
    byte fv,ff,fq;
;
 chalk(573,63+turns*10,string(chr(64+row))+chr(48+number));
 for( fv=1; fv <= 3; fv ++)
  if (old[fv]>stones[fv]) 
   for( ff=stones[fv]+1; ff <= old[fv]; ff ++)
    for( fq=0; fq <= 22; fq ++) fillchar(mem[0xa000*3200+fv*2800+ff*8+fq*80],7,'\0');
 stonesleft -= number;
}

void checkmouse();

static void blip() {; sound(1771); delay(3); nosound; clicked=false; }

void checkmouse()
{;
           {; ax=5; bx=0; } intr(0x33,r);
 clicked=r.bx>0;
 if (clicked) 
  {;     /* The mouse was clicked. Where?  */
   mrow=(dx-38) / 35;
   if ((mrow<1) || (mrow>3))  blip();
   mnum=stones[mrow]-(cx / 64)+1;
   if ((mnum<1) || (mnum>(unsigned char)stones[mrow]))  blip();
  }
}

void takesome();

static void less() {; if (number>1)  number -= 1; }

void takesome()
{
    char r; byte sr;
;
 number=1;
 do {
  do {
   sr=stones[row];
   if (sr==0)  {; row=row % 3+1; number=1; }
  } while (!(sr!=0));
  if (number>sr)  number=sr;
  setcolor(1); rectangle(63+(sr-number)*64,38+35*row,54+sr*64,63+35*row);
   /* Wait for choice */
  show();
  do { checkmouse(); } while (!(keypressed() || clicked));
  if (keypressed())  r=upcase(readkey());
  hide();

  setcolor(0); rectangle(63+(sr-number)*64,38+35*row,54+sr*64,63+35*row);

  if (clicked) 
  {;
   number=mnum;
   row=mrow;
   return;
  } else
  {;
   switch (r) {
    case '\0': switch (readkey()) {
         case 'H': if (row>1)  row -= 1; break; /* Up */
         case 'P': if (row<3)  row += 1; break; /* Down */
         case 'K': number += 1; break;
         case 'M': less(); break;
         case 'I': row=1; break; /* PgUp */
         case 'Q': row=3; break; /* PgDn */
         case 'G': number=5; break; /* Home- check routine will knock this down to size */
         case 'O': number=1; break; /* End */
        }
        break;
    case '+': number += 1; break;
    case '-': less(); break;
    case RANGE_3('A','C'): row=ord(r)-64;
    break;
    case RANGE_5('1','5'): number=ord(r)-48;
    break;
    case '\15': return; break; /* Enter was pressed */
   }
  }
 } while (!false);
}

void endofgame()
{
    char rr;
;
 chalk(595,63+turns*10,"Wins!");
 outtextxy(100,190,"- - -   Press any key...  - - -");
 while (keypressed())  rr=readkey();
           do { ax=5; bx=0; intr(0x33,r); } while (!(keypressed() || (bx>0)));
 if (keypressed())  rr=readkey();
}

void dogfood();
                   /* AI procedure to play the game */
const matrix<1,3,1,2,byte> other = {{{{2,3}},{{1,3}},{{1,2}}}};


static byte live,fv,ff,matches,thisone,where;

static array<1,3,byte> r,sr;
static array<1,3,boolean> inap;

static boolean lmo; /* Let Me Out! */

static byte ooo; /* Odd one out */


static boolean find(byte x)
 /* This gives True if there's a pile with x stones in. */
{
    boolean q; byte p;
boolean find_result;
;
 q=false;
 for( p=1; p <= 3; p ++) if (stones[p]==x)  {; q=true; inap[p]=true; }
 find_result=q;
return find_result;
}



static void find_ap(byte start,byte stepsize)
{
    byte ff;
;
 matches=0;
 fillchar(inap,sizeof(inap),'\0'); /* blank 'em all */
 for( ff=0; ff <= 2; ff ++) if (find(start+ff*stepsize))  matches += 1;
  else thisone=ff;

 /* Now.. Matches must be 0, 1, 2, or 3.
     0/1 mean there are no A.P.s here, so we'll keep looking,
     2 means there is a potential A.P. that we can create (ideal!), and
     3 means that we're already in an A.P. (Trouble!). */

 switch (matches) {
  case 2: {;
      for( ff=1; ff <= 3; ff ++) /* find which one didn't fit the A.P. */
       if (! inap[ff])  ooo=ff;
      if (stones[ooo]>(start+thisone*stepsize)) /* check it's possible! */
           {;     /* create an A.P. */
       row=ooo;  /* already calculated */
       /* Start+thisone*stepsize will give the amount we SHOULD have here. */
       number=stones[row]-(start+thisone*stepsize); lmo=true; return;
      }
     }
     break;
  case 3: {; /* we're actually IN an A.P! Trouble! Oooh dear. */
      row=r[3]; number=1; lmo=true; return;  /* take 1 from the largest pile */
     }
     break;
 }
}

void dogfood()
{
 boolean sorted; byte temp;

;
 live=0; lmo=false;
 for( fv=1; fv <= 3; fv ++)
 {;
  if (stones[fv]>0) 
  {;
   live += 1;
   r[live]=fv; sr[live]=stones[fv];
  }
 }
 switch (live) {
  case 1: /* Only one is free- so take 'em all */
       {; row=r[1]; number=stones[r[1]]; return; }
       break;
  case 2: /* Two are free- make them equal */
     {;
      if (sr[1]>sr[2]) 
      {; row=r[1]; number=sr[1]-sr[2]; return; } else       /* T > b */
       if (sr[1]<sr[2]) 
       {; row=r[2]; number=sr[2]-sr[1]; return; } else       /* B > t */
        {; row=r[1]; number=1; return; }        /* B = t... oh no, we've lost! */
     }
     break;
  case 3: /* Ho hum... this'll be difficult! */
     {;
      /* There are three possible courses of action when we have 3 lines left:
          1) Look for 2 equal lines, then take the odd one out.
          2) Look for A.P.s, and capitalise on them.
          3) Go any old where. */

      for( fv=1; fv <= 3; fv ++) /* Look for 2 equal lines */
       if (stones[other[fv][1]]==stones[other[fv][2]]) 
       {;
        row=fv;  /* this row */ number=stones[fv]; /* all of 'em */ return;
       }

      do {
       sorted=true;
       for( fv=1; fv <= 2; fv ++)
        if (sr[fv]>sr[fv+1]) 
        {;
         temp=sr[fv+1]; sr[fv+1]=sr[fv]; sr[fv]=temp;
         temp= r[fv+1];  r[fv+1]= r[fv];  r[fv]=temp;
         sorted=false;
        }
      } while (!sorted);
      /* Now we look for A.P.s ... */
      for( fv=1; fv <= 3; fv ++)
      {;
       find_ap(fv,1); /* there are 3 "1"s */
       if (lmo)  return; /* cut-out */
      }
      find_ap(1,2); /* only "2" possible */
      if (lmo)  return;

      /* A.P. search must have failed- use the default move. */
      row=r[3]; number=1; return;
     }
     break;
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 setup();
 board();
 do {
  startmove();
  if (dogfoodsturn)  dogfood(); else takesome();
  stones[row] -= number;
  show_changes();
 } while (!(stonesleft==0));
 endofgame(); /* Winning sequence is A1, B3, B1, C1, C1, btw. */
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.