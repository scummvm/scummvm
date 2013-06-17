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

/*$M 6000,600,600*/
/*$V-,I-,R-,F+*/ /* Do not change these directives. */
#include "graph.h"
/*#include "Crt.h"*/
/*#include "Dos.h"*/
/*#include "Tsru.h"*/

namespace Avalanche {

    /* program's signature */
const varying_string<20> tsr_tmark = "FISH FISH FISH!!!";

integer gd,gm;
byte a /*absolute $A000:0*/;
matrix<1,800,0,3,byte> sv;
byte bit; string nam;
byte tsr_int; word tsr_ax;

void grab()
{;
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  move(a,sv[1][bit],800);
 }
}

void drop()
{;
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  move(sv[1][bit],a,800);
 }
}

void say(string x)
{;
 grab(); output << string('\15')+x;
}

void pak(string x)
{
    char r;
;
 say(x+" (press any key...)"); r=readkey(); drop();
}

string typein()
{
    char r; string x;
string typein_result;
;
 x="";
 do {
  r=readkey();
  switch (r) {
    case '\10': if (x[0]>'\0')  {; output << string('\10')+'\40'+'\10'; x[0] -= 1; } break;
   case '\15': {; typein_result=x;  return typein_result; } break;
   case '\33': {; typein_result=""; return typein_result; } break;
   default: if (x[0]<'\62')  {; x=x+r; output << r; }
  }
 } while (!false);
return typein_result;
}

void load()
{
    byte a /*absolute $A000:1200*/; untyped_file f;
;
 say("LOAD: filename?"); nam=typein(); drop();
 if (nam=="")  return;
 assign(f,nam); reset(f,1);
 if (ioresult!=0) 
 {;
  pak("LOAD: file not found."); return;
 }
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a,12080);
  if (ioresult!=0) 
  {;
   pak("LOAD: error whilst loading."); close(f); return;
  }
 }
 close(f);
}

void save()
{
      const string header = 
 string("This is a file from an Avvy game, and its contents are subject to ")+
 "copyright."+'\15'+'\12'+'\15'+'\12'+"Have fun!"+'\32';
    untyped_file f; varying_string<30> screenname; searchrec s; char r;
byte a /*absolute $A000:1200*/;
;
 say("SAVE: filename?"); nam=typein(); drop();
 if (nam=="")  return;
 findfirst(nam,anyfile,s); if (doserror==0) 
 {;
  say("SAVE: That exists, are you sure\? (Y/N)");
  do { r=upcase(readkey()); } while (!(set::of('Y','N', eos).has(r))); drop();
  if (r=='N')  return;
 }
 screenname="Saved with HIZ.";
 assign(f,nam); rewrite(f,1); blockwrite(f,header[1],146);
 blockwrite(f,screenname,31);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockwrite(f,a,12080);
  if (ioresult!=0) 
  {;
   pak("SAVE: error whilst saving."); close(f); return;
  }
 }
}

void hedges()
{;
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  fillchar(mem[0xa000*14*80],80,'\377');
  fillchar(mem[0xa000*166*80],80,'\377');
 }
}

void reset_()
{
    registers regs;
;
 regs.ax=14;
 intr(0x10,regs);
 directvideo=false;
}

void do_pop()
{
    char r;
;
 do {
  say("HIZ: Load Save Hedges Reset eXit?"); r=upcase(readkey()); drop();
  switch (r) {
   case 'L': load(); break;
   case 'S': save(); break;
   case 'H': hedges(); break;
   case 'R': reset_(); break;
   case 'X': return; break;
  }
 } while (!false);
}

/* Now for the TSR stuff */

void mypoprtn()
{
    registers r;
;
 beginpop;
 do_pop();
 endpop;
}
/**********************/
void stop_tsr()
{;
 if (tsrexit) 
  output << "HIZ stopped" << NL;
 else
  output << "Unable to stop HIZ - other TSR has been installed." << NL;
}
/**********************/
/* This interrupt is called at program start-up. Its purpose is to provide
 a way to communicate with an installed copy of the TSR through cmdline
 params. The installation of the intrpt serves to prevent any attempt to
 install a 2nd copy of the TSR */
void tsr_intrtn(word flags,word cs,word ip,word ax,word bx,word cx,word dx,word si,word di,word ds,word es,word bp)
/*interrupt;*/
{;
 tsr_ax=ax;
 cli;
 beginint;
 sti;

 switch (tsr_ax) {
 case 1: {;
     stop_tsr(); /* Terminate TSR, if poss.*/
    }
    break;
 case 2: {;
     tsroff= true;  /* Suspend TSR */
     output << "HIZ suspended." << NL;
    }
    break;
 case 3: {;
     tsroff=false;
     output << "HIZ restarted" << NL;
    }
    break;
  }
 cli;
 endint;
 sti;
}
/*******************/
byte i;
registers r;
string st;
boolean b;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;

 /********************************************/
 /* Check to see if TSR is already installed */
 /********************************************/

 tsr_int=dupcheck(tsr_tmark,&tsr_intrtn);

 /*****************************************/
 /* If it IS already installed, check for */
 /* parameter.                            */
 /*****************************************/

 if (tsr_int > 0) 
 {
  if (paramcount>0) 
  {
   st=paramstr(1);
   for( i=1; i <= length(st); i ++)
   st[i]=upcase(st[i]);
   if (st=="STOP") 
    r.ax=1;
   else if (st=="HOLD")  r.ax=2;
   else if (st=="RSTR")  r.ax=3;
   else r.ax=4;

   if (r.ax<4) 
   {;
    intr(tsr_int,r);
    return 0;
   }
   else
   {;
    output << "HIZ: invalid parameter " << paramstr(1) << NL;
    output << "Syntax: HIZ stop/hold/rstr" << NL;
    return 0;
   }
   }
  else
  {;
   output << "HIZ already installed." << NL;
   output << "(If you're sure it isn't, try running BLANKINT /I." << NL;
   output << "It's in the TURBO directory.)" << NL;
  }
  return 0;
 }

 output << string("Wait...")+'\15'; /* tell 'em to wait...! */

 /* Call PopSetUp to point to PopUp routine. Include the
   scancode and the keymask to activate the program. In
   this example, the scancode is $23 (H) and the
   keymask is 08h (Alt.) */

 popsetup(&mypoprtn,0x23,0x8);

 directvideo=false;
 output << "Û Û ßÛß ßßÛ v1.0, (c) 1992, Thomas Thurman." << NL;
 output << "ÛßÛ  Û  Üß" << NL;
 output << "ß ß ßßß ßßß The AVD saving/loading/hedging program." << NL;
 output << "            Use with Dr. Genius. The Hot Key is Ctrl-Alt-H." << NL;

 output << NL;
 output << " Enter \"HIZ stop\" to remove Hiz from memory" << NL;
 output << "       \"HIZ hold\" to temporarily halt Hiz" << NL;
 output << "       \"HIZ rstr\" to restart Hiz" << NL;
 stacksw=-1;
 install_int;
 keep(0);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.