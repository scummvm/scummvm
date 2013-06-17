/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



               /* Demonstration of the Bigo II system. */
#include "graph.h"
/*#include "Crt.h"*/
/*#include "Rodent.h"*/
/*#include "Tommys.h"*/

integer gd,gm;

typedef matrix<'\0','\377',0,15,byte> fonttype;

boolean reverse;
word spinnum;
array<0,1,fonttype> f;
file<fonttype> ff;
array<0,7,0,15,0,79,byte> strip;
byte across;
word w; byte y;
byte charnum;
byte cfont; /* current font. 0=roman, 1=italic. */

char* c;

const integer colours = 12; /* Run Workout to see possible values of these two. */
const integer steps = 6; /* 60,30,20,15,12,10,6,5,4,3,2,1 */
const integer gmtop = 360 / steps;

/*$L credits.obj*/ extern void credits(); 

void bigo2(string date)
{
 integer gd,gm;
 byte c;
 palettetype p;
 untyped_file f; pointer pp; word s;


 getpalette(p);
 setvisualpage(1); setactivepage(0);
 assign(f,"logo.avd"); reset(f,1);
 for( gd=7; gd <= 194; gd ++)
  blockread(f,mem[0xa000*(gd*80)],53);
 close(f);
 s=imagesize(0,7,415,194); getmem(pp,s); getimage(0,7,415,194,pp);

 cleardevice();
 for( gd=1; gd <= 64; gd ++)
 {
  for( gm=0; gm <= gmtop; gm ++)
  {
   c=(c % colours)+1;
/*   putpixel(trunc(sin(gm*steps*n)*gd*6)+320,
              trunc(cos(gm*steps*n)*gd*3)+175,c); */
   if (c>5)  continue_;
   setcolor(c); arc(320,175,gm*steps,gm*steps+1,gd*6);
  }
  if (keypressed())  { closegraph(); exit(0); }
 }
 settextstyle(0,0,1); setcolor(13);
 outtextxy(550,343,"(press any key)");

 putimage(112,0,pp,orput); freemem(pp,s);
 resetmouse; setvisualpage(0);
}

void nextchar()     /* Sets up charnum & cline for the next character. */
{

 c += 1;
}

void getchar()
{
 do {
  nextchar();

  switch (*c) {
   case '@': { cfont=1; nextchar(); } break;
   case '^': { cfont=0; nextchar(); } break;
   case '%': { closegraph(); exit(0); } break;
  }

 } while (!((*c!='\15') && (*c!='\12')));

 for( w=0; w <= 7; w ++)
  for( y=0; y <= 15; y ++)
   move(strip[w][y][1],strip[w][y][0],79);

 for( w=0; w <= 7; w ++)
  for( y=0; y <= 15; y ++)
   strip[w][y][79]=(byte)((strip[7][y][78] << (w+1)))+
    ((cardinal)f[cfont][*c][y] >> (7-w));

 across=0;
}

void scrolltext()
{
 byte c,w,y;

 across += 1;
 if (across==8)  getchar();

 for( y=0; y <= 15; y ++)
  for( w=0; w <= 1; w ++)
  move(strip[across][y][0],mem[0xa000*24000+(y*2+w)*80],80);
}

void do_stuff()
{
 switch (spinnum) {
  case 50 ... 110: {
            setfillstyle(1,14);
            bar(0,315+(spinnum-50) / 3,640,315+(spinnum-50) / 3);
            bar(0,316-(spinnum-50) / 3,640,316-(spinnum-50) / 3);
            if (spinnum>56) 
            {
             setfillstyle(1,13);
             bar(0,315+(spinnum-56) / 3,640,315+(spinnum-56) / 3);
             bar(0,316-(spinnum-56) / 3,640,316-(spinnum-56) / 3);
            }
          }
          break;
  case 150 ... 198: {
             setfillstyle(1,0);
             bar(0,315+(spinnum-150) / 3,640,315+(spinnum-150) / 3);
             bar(0,316-(spinnum-150) / 3,640,316-(spinnum-150) / 3);
            }
            break;
  case 200: scrolltext(); break;
 }
}

void setcol(byte which,byte what)
/*var dummy:byte;*/
{;
/* setpalette(which,what);
 asm
(*  mov dx,$3DA;
  in ax,dx;

  or ah,ah;

  mov dx,$3C0;
  mov al,which;
  out dx,al;

  mov dx,$3C0;
  mov al,what;
  out dx,al;
 end;
(* dummy:=port[$3DA];
 port[$3C0]:=which; port[$3C0]:=what;*/
}

void bigo2go()
{
 palettetype p; byte c; boolean lmo;
 boolean altnow,altbefore;

 for( gd=0; gd <= 13; gd ++) p.colors[gd]=0;

 setcol(13,24); /* murk */ setcol(14,38); /* gold */
 setcol(15,egawhite); /* white- of course */
 p.colors[13]=24; p.colors[14]=38; p.colors[15]=egawhite;

 /***/

    p.colors[5]=egawhite;
    p.colors[4]=egalightcyan;
    p.colors[3]=egacyan;
    p.colors[2]=egalightblue;
    p.colors[1]=egablue;

 /***/

 c=1; p.size=16; lmo=false;
 setallpalette(p);

 do {
/*  if reverse then
  begin
   dec(c); if c=0 then c:=colours;
  end else
  begin
   inc(c); if c>colours then c:=1;
  end;
  for gm:=1 to colours do
   case p.colors[gm] of
    egaWhite: begin p.colors[gm]:=egaLightcyan; setcol(gm,egaLightCyan); end;
    egaLightcyan: begin p.colors[gm]:=egaCyan; setcol(gm,egaCyan); end;
    egaCyan: begin p.colors[gm]:=egaLightblue; setcol(gm,egaLightblue); end;
    egaLightblue: begin p.colors[gm]:=egaBlue; setcol(gm,egaBlue); end;
    egaBlue: begin p.colors[gm]:=0; setcol(gm,0); end;
   end;
  p.colors[c]:=egaWhite; setcol(c,egaWhite);

  AltBefore:=AltNow; AltNow:=testkey(sAlt);*/

  if (anymousekeypressed)  lmo=true;
  if (keypressed())  lmo=true;

 /* if (AltNow=True) and (AltBefore=False) then reverse:=not reverse;*/

  do_stuff();
  if (spinnum<200)  spinnum += 1;
 } while (!lmo);
}

void parse_cline()
{
    integer e;

 if (paramstr(1)!="jsb") 
 {
  output << "Not a standalone program." << NL; exit(255);
 }
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 parse_cline();

 gd=3; gm=1; initgraph(gd,gm,"");
 assign(ff,"avalot.fnt"); reset(ff); ff >> f[0]; close(ff);
 assign(ff,"avitalic.fnt"); reset(ff); ff >> f[1]; close(ff);

 c=addr(credits()); c -= 1;

 fillchar(strip,sizeof(strip),'\0');
 reverse=false; spinnum=0; across=7; charnum=1; cfont=0;
 bigo2("1189"); /* 1189? 79? 2345? 1967? */
 bigo2go();
 return EXIT_SUCCESS;
}
