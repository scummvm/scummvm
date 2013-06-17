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

typedef matrix<'\0','\377',0,15,byte> fonttype;

struct markertype {
              word length;
              longint offset;
              byte checksum;
};

string rawname;
array<0,1999,char> buffer,was;
word bufpos,bufsize;
fonttype font;
integer cpos;
char r;
boolean ok;

const char marker = '\33';

void cursor()
{
    byte fv;
;
 for( fv=12; fv <= 15; fv ++)
  mem[0xa000*cpos+fv*80]=~(mem[0xa000*cpos+fv*80]);
}

void xy()
{;
 cpos=(bufpos / 80)*1280+(bufpos % 80);
}

void show()
{
    word fv,orig,y;
;
 for( fv=0; fv <= 1999; fv ++)
  if (buffer[fv]!=was[fv]) 
  {;
   orig=(fv / 80)*1280+(fv % 80);
   for( y=0; y <= 15; y ++)
    mem[0xa000*orig+y*80]=(byte)(font[buffer[fv]][y]);
  }
 xy();
 move(buffer,was,sizeof(buffer));
}

void sizeup()
{;
 do {
  switch (buffer[bufsize]) {
   case marker: return; break; /* we've found it OK! */
   case '\0': bufsize -= 1; break;
   default: bufsize += 1;
  }
 } while (!false);
}

void graphics()
{
 integer gd,gm;
 file<fonttype> fontfile;
;
 gd=3; gm=1; initgraph(gd,gm,"c:\\bp\\bgi");
 assign(fontfile,"c:\\thomas\\lanche.fnt");
 reset(fontfile); fontfile >> font; close(fontfile);
}

void helpscreen();

static byte l,fv;

static void addon(char b, string st)
{;
 buffer[3+80*l]=b; b -= 64;
 buffer[5+80*l]=b;
 move(st[1],buffer[7+80*l],length(st));
 l += 1;
}

void helpscreen()
{
 array<0,1999,char> temp;
 char r;
;
 l=0; move(buffer,temp,2000); fillchar(buffer,sizeof(buffer),'\0');
 addon('B',"Bubble");
 addon('C',"Centre text");
 addon('D',"Don't add \20 (at end)");
 addon('F',"Italic Font");
 addon('G',"Bell");
 addon('H',"Not allocated (=backspace)");
 addon('I',"Not allocated (=tab)");
 addon('L',"Left-justify text");
 addon('K',"Keyboard input");
 addon('M',"Carriage return (same as \r key)");
 addon('P',"Scroll (Paragraph break)");
 addon('Q',"Yes/no scroll (question)");
 addon('R',"Roman font");
 addon('S',"Fix to sprite");
 addon('U',"Money (in œsd format)");
 addon('V',"View icon (number from ^S)");
 show(); do { r=readkey(); } while (!(! keypressed()));
 cleardevice();
 fillchar(was,sizeof(was),'\0'); move(temp,buffer,2000); show();
}

void edit_it();

static void legit(char r)
{;     /* it's a legit keystroke */
 move(buffer[bufpos],buffer[bufpos+1],1999-bufpos);
 buffer[bufpos]=r;
 bufpos += 1; show();
}

void edit_it()
{
    byte fv;
;
 bufpos=0;
 fillchar(was,sizeof(was),'\0'); show();
 do {
  cursor(); r=readkey(); cursor();
  switch (r) {
   case '\0': switch (readkey()) { /* extd keystrokes */
        case 'K': if (bufpos>0)     {; bufpos -= 1; xy(); } break; /* Left */
        case 'M': if (bufpos<bufsize)  {; bufpos += 1; xy(); } break; /* Right */
        case 'H': if (bufpos>80)    {; bufpos -= 80; xy(); } break; /* Up */
        case 'P': if (bufpos<(cardinal)bufsize-79)  {; bufpos += 80; xy(); } break; /* Down */
        case 's': if (bufpos>10)  {; bufpos -= 10; xy(); } break;
        case 't': if (bufpos<(cardinal)bufsize-10)  {; bufpos += 10; xy(); } break;
        case 'G': {; bufpos=0; xy(); } break; /* Home */
        case 'O': {; bufpos=bufsize; xy(); } break; /* End */
        case 'S': if (bufpos<bufsize)  {;  /* Del */
              move(buffer[bufpos+1],buffer[bufpos],1999-bufpos); show();
             }
             break;
        case ';': helpscreen(); break;
        case 'A': legit('\357'); break; /* copyright symbol */
        case 'B': legit('\221'); break; /* uppercase AE */
        case 'C': legit('\222'); break; /* lowercase AE */
       }
       break;
   case '\10': if (bufpos>0)  {; /* backspace */
        bufpos -= 1;
        move(buffer[bufpos+1],buffer[bufpos],1999-bufpos);
        show();
       }
       break;
   case '\33': {;
         restorecrtmode();
         return;
        }
        break; /* end of editing */
  default: legit(r);
  }
  sizeup();
 } while (!false);
}

void loadit()
{
    untyped_file f;
;
 if (pos(".",rawname)==0)  rawname=rawname+".raw";
 fillchar(buffer,sizeof(buffer),'\0');
 /*$I-*/
 assign(f,rawname);
 reset(f,1);
 if (ioresult!=0) 
 {;
  output << string('\7')+"New file!"+'\7' << NL;
  buffer[0]=marker;
  return;
 }
 bufsize=filesize(f);
 blockread(f,buffer,bufsize);
 close(f);
 while (buffer[bufsize]=='\0')  bufsize -= 1;
 if (buffer[bufsize]!=marker) 
 {;     /* add on a marker */
  bufsize += 1;
  buffer[bufsize]=marker;
 }
}

void saveit()
{
    untyped_file f;
;
 output << "Saving " << rawname << ", " << bufsize << " bytes..." << NL;
 assign(f,rawname); rewrite(f,1);
 blockwrite(f,buffer,bufsize);
 close(f);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 output << "Filename of .RAW file?"; input >> rawname >> NL;
 loadit();
 ok=false;
 do {
  graphics();
  edit_it();
  output << "Now what?" << NL;
  output << NL;
  output << " Filename: " << rawname << NL;
  output << " Size of text: " << bufsize << NL;
  output << " Cursor position: " << bufpos << NL;
  output << NL;
  output << " C) Cancel this & continue edit" << NL;
  output << " S) Save under current name" << NL;
  output << " A) Save under a different name" << NL;
  output << " X) Exit & lose all changes." << NL;
  output << NL;
  output << "Pick one!" << NL;
  do { r=upcase(readkey()); } while (!(set::of('C','S','A','X', eos).has(r)));
  switch (r) {
   case 'X': ok=true; break;
   case 'S': {; saveit(); exit(0); } break;
  }
 } while (!ok);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.