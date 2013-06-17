


/*#include "Dos.h"*/
/*#include "Crt.h"*/
#include "graph.h"
/*#include "Drivers.h"*/


const integer width = 88;
const integer height = 8; /* width & height of string returned by "time" */

integer gd,gm;
integer xx,yy;
shortint ix,iy;
byte cp;
byte count;
array<0,1,pointtype> pages;
boolean test;

boolean the_cows_come_home()
{
    registers rmove,rclick;
boolean the_cows_come_home_result;
;
 rmove.ax=11; intr(0x33,rmove);
 rclick.ax=3; intr(0x33,rclick);
 the_cows_come_home_result=
   (keypressed()) || /* key pressed */
   (rmove.cx>0) || /* mouse moved */
   (rmove.dx>0) ||
   (rclick.bx>0);  /* button clicked */
return the_cows_come_home_result;
}

string time1()
{
    word h,m,s,s1; varying_string<2> hh,mm,ss; char ampm;
string time1_result;
;
 gettime(h,m,s,s1);
 if (h<12) 
  ampm='a';
 else {;
  ampm='p';
  if (h==0)  h=12; else h -= 12;    /* 24-hr adjustment */
 }
 str(h,2,hh); str(m,2,mm); str(s,2,ss); /* stringify them */
 time1_result=hh+'.'+mm+'.'+ss+' '+ampm+'m';
return time1_result;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 test=the_cows_come_home();
 gm=registerbgidriver(&egavgadriverproc);
 gd=3; gm=1; initgraph(gd,gm,"");
 ix=3; iy=1; xx=177; yy=177; setcolor(11); cp=0;
 setfillstyle(1,0); count=2;
 do {
  setactivepage(cp); setvisualpage(1-cp); cp=1-cp;
  delay(20); if (count>0)  count -= 1;
  {
  pointtype& with = pages[cp]; 
  ;
   if (count==0) 
    bar(with.x,with.y,with.x+width,with.y+height);
   with.x=xx; with.y=yy; /* update record for next time */
  }
  outtextxy(xx,yy,time1());
  xx=xx+ix; yy=yy+iy;
  if (xx<= 10)  ix=Random(9)+1; if (xx>=629-width)   ix=-Random(9)+1;
  if (yy<= 10)  iy=Random(9)+1; if (yy>=339-height)  iy=-Random(9)+1;
 } while (!the_cows_come_home());
 closegraph();
 textattr=30; clrscr;
 output << "*** Bouncing Clock *** (c) 1992, Thomas Thurman. (An Avvy Screen Saver.)" << NL;
 for( gd=1; gd <= 48; gd ++) output << '~'; output << NL;
 output << "This program may be freely copied." << NL;
 output << NL;
 output << "Have fun!" << NL;
return EXIT_SUCCESS;
}