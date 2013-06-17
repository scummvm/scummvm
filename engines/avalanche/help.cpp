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
/*#include "Rodent.h"*/
/*#include "Crt.h"*/

typedef graphcursmasktype cursor;
const char vernum[] = "v1ù00";
const char copyright[] = "1992";
const cursor questionmark = 
{ 
    ((511,1023,2047,31,15,8199,32647,65415,63503,61471,61503,61695,63999,63999,61695,61695),
     (65024,33792,34816,34784,40976,57224,32840,72,1936,2080,2496,2304,1536,1536,2304,3840)),
               0,
               0};

const array<1,7,varying_string<12> > topics = 
 {{"Front page","Background","Toolbar","Menus",
  "Keyboard","Shareware","Exit Help"}};

const array<1,6,char> keys = "FBTMKS";

byte page_;
palettetype dp,dark;
char r;
boolean lmo;

void hilight(integer x,integer y, byte c1,byte c2, string z)
{
    string w;
;
 w=z; w[1]='\40'; setcolor(c1); outtextxy(x,y,w);
 w=z; fillchar(w[2],length(z)-1,'\40'); setcolor(c2);
 outtextxy(x,y,w); outtextxy(x-1,y,w);
}

void showpage(byte x)
{;
 if (x==page_)  return; /* twit */
 if (x==7)  {; lmo=true; return; }
 setallpalette(dark); hidemousecursor; settextjustify(1,1);
 if (page_!=177) 
 {;
  setfillstyle(1,1);  bar(507,page_*25+2,607,page_*25+22);
  setfillstyle(1,9);  bar(500,page_*25,600,page_*25+20);
/*  setcolor(11); outtextxy(550,page*25+10,topics[page]);*/
  hilight(550,page_*25+10,11,14,topics[page_]);
 }
 page_=x;
  setfillstyle(1,4);  bar(507,page_*25+2,607,page_*25+22);
  setfillstyle(1,12); bar(500,page_*25,600,page_*25+20);
/*  setcolor(14); outtextxy(550,page*25+10,topics[page]); */
  hilight(550,page_*25+10,14,15,topics[page_]);
 setfillstyle(1,7); bar(0,27,470,189); settextjustify(0,2); setcolor(1);
 switch (page_) {
  case 1: {; /* Front page */
      setcolor(black);
      outtextxy( 10, 40,"Thorsoft of Letchworth presents");
      outtextxy(300, 80,vernum);
      outtextxy( 10, 90,string("(c) ")+copyright+", Mike, Mark and Thomas Thurman.");
      setcolor(red);
      outtextxy(100,129,"Finding your way around it...");
      setcolor(blue);
      outtextxy( 10,120,"You are now using the Help System.");
      outtextxy( 10,138,"Press the first letter of the topic that you want to");
      outtextxy( 10,147,"read (eg T for (T)oolbar), or click on its button (on");
      outtextxy( 10,156,"the right) using the mouse. Use \"Exit Help\" to quit.");
      outtextxy( 10,174,"(Fastest exit- just hit Esc!)");
     }
     break;
  case 2: {; /* Background */
      setcolor(red);
      outtextxy(300, 30,"Remember this chap?");
      setcolor(blue);
      outtextxy( 10, 55,"Back in good old A.D. 79, there lived a Roman");
      outtextxy( 10, 64,"whose name was Denarius Avaricius Sextus, known");
      outtextxy( 10, 73,"to his friends as Avvy. His wife was called Arkata,");
      outtextxy( 10, 82,"and he had a slave named Crapulus. His grandson");
      outtextxy( 10, 91,"joined the army, was posted to Gaul, and liked it");
      outtextxy( 10,100,"so much that he stayed there, telling everyone the");
      outtextxy( 10,109,"one about the centurion and the Turkish bath. His");
      outtextxy( 10,118,"thirty-sixth male-line descendant accidentally");
      outtextxy( 10,127,"poisoned an old enemy of William of Normandy, and");
      outtextxy( 10,136,"to his great surprise came so much into Bill's favour");
      outtextxy( 10,145,"that, after the Battle of Hastings a few years");
      outtextxy( 10,154,"later, he was made the lord of a small town in");
      outtextxy( 10,163,"Hertfordshire called Argent. It is his great-grandson");
      outtextxy( 10,172,"who stars in this game, back in good old A.D. 1189.");
     }
     break;
  case 3: {; /* ? */
      outtextxy( 15, 30,"The Toolbar is there so that (along with the menus)");
      outtextxy( 15, 39,"you can perform a lot of the functions contained in");
      outtextxy( 15, 48,"the game, using the mouse.");
      setcolor(red);
      outtextxy( 15, 66,"COMPASS:");
      setcolor(blue);
      outtextxy( 90, 66,"Used to point Avvy in the right direction.");
      setcolor(red);
      outtextxy( 15, 75,"THINKS:");
      setcolor(blue);
      outtextxy( 90, 75,"Shows the person/object you're thinking of.");
      setcolor(red);
      outtextxy( 15, 84,"SCORE:");
      setcolor(blue);
      outtextxy( 90, 84,"Shows how many points you've got.");
      setcolor(red);
      outtextxy( 15, 93,"SPEED:");
      setcolor(blue);
      outtextxy( 90, 93,"Adjusts the speed of the game.");
      setcolor(red);
      outtextxy( 15,102,"L.E.D.s:");
      setcolor(blue);
      outtextxy( 90,102,"Shows whether sound is on (toggle with <F2>),");
      outtextxy( 90,111,"the computer is ready, or there is an error.");
      setcolor(red);
      outtextxy( 15,120,"CLOCK:");
      setcolor(blue);
      outtextxy( 90,120,"Shows the time.");
      setcolor(red);
      outtextxy( 15,129,"'OK' box:");
      setcolor(blue);
      outtextxy( 90,129,"Works the same as pressing <ENTER>.");
      setcolor(0);
      outtextxy( 15,147,"N.B. The game action is suspended while you are");
      outtextxy( 15,156,"using the toolbar.");
     }
     break;
  case 4: {; /* menus */
      outtextxy( 15, 60,"To use the drop-down menus, either click on the bar");
      outtextxy( 15, 69,"at the top with the mouse, or press Alt and the first");
      outtextxy( 15, 78,string("letter of the menu's name (eg alt-A = (A)ction.) The ")+'\3');
      outtextxy( 15, 87,"menu is alt-H, for (H)eart.");
      outtextxy( 15,105,"When you have a menu, either click on the option you");
      outtextxy( 15,114,"want, or press its initial letter (eg O for \"OS Shell\").");
      outtextxy( 15,132,"To do something to, or with, an object or person, first");
      outtextxy( 15,141,"select them (or it) from the People or Things menu. Then");
      outtextxy( 15,150,"select whatever you wanted to do from the Use menu.");
      setcolor(red);
      outtextxy(15,177,"(OK, so it sounds complicated, but then it's intuitive!)");
     }
     break;
  case 5: {; /* Keyboard */
      outtextxy(15, 60,"The keyboard interface is even simpler than the mouse");
      outtextxy(15, 70,"interface! Just type what you want Avvy to do.");
      outtextxy(15, 80,"For example, to open a door, type in:");
      setcolor(red); outtextxy(100,95,"open door"); setcolor(blue);
      outtextxy(15,110,"And to look at Crapulus, type:");
      setcolor(red); outtextxy(100,125,"look at Crapulus");
      setcolor(blue); outtextxy(15,140,"(Don't forget to press Enter after each command!)");
      outtextxy(15,160,"This is a traditional command-line parser interface,");
      outtextxy(15,170,"similar to the one in \"Avaricius\", only more advanced.");
     }
     break;
  case 6: {; /* ? */
      outtextxy(15, 30,"This game is Shareware. Most programs are sold through");
      outtextxy(15, 40,"shops, and the authors prevent you from copying them.");
      outtextxy(15, 50,"Shareware is different. You may copy it and give it to");
      outtextxy(15, 60,"ANYBODY at all. You may post it on any BBS, give it to");
      outtextxy(15, 70,"friends, etc. If you like it, we ask you to pay us for");
      outtextxy(15, 80,"the software directly through the post. We're relying");
      outtextxy(15, 90,"on you to register!");
      outtextxy(99,177,"{ ETC }");
     }
     break;
  }
 setallpalette(dp); showmousecursor;
}

void helpsetup()
{
    integer gd,gm;
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi"); settextstyle(1,0,0); setcolor(11);
 getpalette(dp); dark.size=dp.size; fillchar(dark.colors,dark.size,'\0');
 setallpalette(dark); setusercharsize(3,1,8,10);
 for( gm=1; gm <= 3; gm ++) outtextxy(gm,0,"Avalot- HELP!");
 resetmouse; setgraphicscursor(questionmark); showmousecursor;
 settextstyle(0,0,1); settextjustify(1,1);
 for( gd=2; gd <= 7; gd ++)
 {;
  setfillstyle(1,1);  bar(507,gd*25+2,607,gd*25+22);
  setfillstyle(1,9);  bar(500,gd*25,600,gd*25+20);
/*  setcolor(11); outtextxy(550,gd*25+10,topics[gd]); */
  hilight(550,gd*25+10,11,14,topics[gd]);
 }
 showmousecursor;
 page_=177; showpage(1); lmo=false; setfillstyle(1,7);
 for( gd=1; gd <= 3; gd ++)
 {;
  bar( 10-gd*3,200-gd*3,490-gd*6,200-gd*3);
  bar(490-gd*6, 37-gd*3,491-gd*6,200-gd*3);
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 helpsetup();
 do {
  do { getbuttonstatus; } while (!((mkey==left) || keypressed()));
  if (keypressed()) 
  {;     /* keyboard choice */
   r=upcase(readkey());
   if (pos(r,keys)>0)  showpage(pos(r,keys)); else
    if (r=='\0') 
    switch (readkey()) { /* grab extd keystroke */
     case 'H': if (page_>1)  showpage(page_-1); break;
     case 'P': if (page_<6)  showpage(page_+1); break;
     case 'G':case 'I': showpage(1); break;
     case 'O':case 'Q': showpage(6); break;
    } else
     if (set::of('\33','Q','X','E','H', eos).has(r))  lmo=true; /* quit */
  }
  else
  {;     /* mouse choice */
   if ((mousex>400) && (mousey>25)) 
   {;
    showpage(mousey / 25);
   }
  }
 } while (!lmo);
return EXIT_SUCCESS;
}