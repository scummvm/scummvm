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
/*#include "Ansi.h"*/

namespace Avalanche {

integer gd,gm;
untyped_file f;
array<1,16000,byte> aa;
matrix<0,27,0,35,byte> cols;
text t;
string x;
byte n;
byte spaces;
byte cfg,cbg; /* Current foreground & background. */
byte ofg,obg;  /* Old fg & bg. */

void do_spaces()
{;
 if (spaces==0)  return;
 along(spaces);
 spaces=0;
}

void finishline();
static byte wx,wy;

static void jumpto(byte xx)
{;
 along(xx-wx);
}

void finishline()
{;
 wx=29-spaces; wy=gm+1;
 switch (wy) {
  case 1: {;
      sgr(7); jumpto(35); output << "Back in good old A.D. ";
      sgr(15); output << "1189"; sgr(7); output << "..." << NL; cfg=7;
     }
     break;
  case RANGE_5(3,7): {;
         t >> x >> NL;
         while (x[length(x)]=='\40')  x[0] -= 1;
         if (x!="") 
         {;
          jumpto(30);
          sgr(9);
          spaces=0;
          while (x!="") 
          {;
           if (x[1]==' ') 
            spaces += 1;
           else
           {;
            do_spaces();
            output << x[1];
           }
           Delete(x,1,1);
          }
          if (wy==7)  close(t);
          output << NL;
         }
        }
        break;
  case 8: {;
      jumpto(67); sgr(9); output << "d'Argent" << NL; cfg=9;
     }
     break;
  case 11: {;
       jumpto(37); sgr(14); output << "He's back..." << NL;
      }
      break;
  case 13: {;
       jumpto(47); sgr(14); output << "And this time," << NL;
      }
      break;
  case 14: {;
       jumpto(52); sgr(14); output << "he's wearing tights..." << NL;
      }
      break;
  case 16: {;
       jumpto(35); sgr(4);
       output << "A Thorsoft of Letchworth game. * Requires EGA" << NL;
      }
      break;
  case 17: {;
       jumpto(37); sgr(4);
       output << "and HD. * By Mike, Mark and Thomas Thurman." << NL;
      }
      break;
  case 18: {;
       jumpto(39);
       sgr( 4); output << "Sfx archive- ";
       sgr( 9); output << "Download ";
       sgr(14); output << "AVLT10.EXE";
       sgr( 9); output << " now!";
      }
      break;
  default: output << NL;
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 fillchar(cols,sizeof(cols),'\0');
 assign(f,"v:avvypic.ptx");
 reset(f,1);
 blockread(f,aa,filesize(f));
 close(f);
 putimage(0,0,aa,0);
 for( gd=0; gd <= 27; gd ++)
  for( gm=0; gm <= 34; gm ++)
   cols[gd][gm+1]=getpixel(gd,gm);

 restorecrtmode();

 assign(output,"v:avalot.ans"); rewrite(output); normal; ed;
/* assign(output,''); rewrite(output); normal; ed;*/
 assign(t,"v:avalot.txt"); reset(t);

 for( gm=0; gm <= 17; gm ++)
 {;
  spaces=0;
  for( gd=0; gd <= 27; gd ++)
  {;
   if ((gd==22) && (gm==4)) 
   {;
    do_spaces();
    sgr(red); output << 'ß';
   } else
   {;
    if (cols[gd][2*gm]==cols[gd][2*gm+1]) 
    {;
     if (cols[gd][2*gm]==0) 
      spaces += 1; /* a blank space */
     else {;
      do_spaces();

      if (cfg==cols[gd][2*gm])  output << 'Û'; else
       if (cbg==cols[gd][2*gm])  output << ' '; else
       {;
        sgr((cols[gd][2*gm])+(cbg*16));
        cfg=cols[gd][2*gm];
        output << 'Û';
       }
     }
    } else
     if ((cols[gd][2*gm]>7) && (cols[gd][2*gm+1]<8)) 
     {;
      do_spaces();
      sgr(cols[gd][2*gm]+cols[gd][2*gm+1]*16);
      cfg=cols[gd][2*gm]; cbg=cols[gd][2*gm+1]*16;
      output << 'ß';
     } else
     {;
      do_spaces();

      ofg=cfg; obg=cbg;
      cbg=cols[gd][2*gm]; cfg=cols[gd][2*gm+1];

      if ((cbg==ofg) && (cfg==obg)) 
      {;
       n=cfg*16+cbg;
       if (n>128)  n -= 128;
       output << 'ß';
      } else
      {;
       n=cbg*16+cfg;
       if (n>128)  n -= 128;
       if ((cfg!=ofg) || (cbg!=obg))  sgr(n);
       output << 'Ü';
      }

     }
    }
   } finishline();
  }
 output << NL;
 normal;
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.