

/*
  ÛßÜ ÛßÜ ÜßßÜ  ßÛß Ûßß  Üß ßÛß      ÜßÛßÜ  ßÛß ÛÜ  Û ÜÛßß  ßÛß ÛßÜ Ûßß Û
  Ûß  ÛÛ  Û  Û Ü Û  Ûßß ßÜ   Û      Û  Û  Û  Û  Û ÛÜÛ  ßßßÜ  Û  ÛÛ  Ûßß Û
  ß   ß ß  ßß   ßß  ßßß   ß  ß      ß  ß  ß ßßß ß  ßß  ßßß   ß  ß ß ßßß ßßß

                 PINGO            Full-screen sub-parts of the game. */

#define __pingo_implementation__


#include "pingo.h"


#include "graph.h"
#include "gyro.h"
#include "lucerna.h"
/*#include "Crt.h"*/
#include "trip5.h"
#include "scrolls.h"


void dplot(integer x,integer y, string z)
{;
 setcolor( 9); outtextxy(x,y  ,z);
 setcolor(11); outtextxy(x,y-1,z);
}

void bosskey()
/*const
 months : array[0..11] of char = 'JFMAMJJASOND';
 title = 'Net Profits';
 fish = #224; { à }
var fv:byte; gd,gm:integer; r:char;
begin;
 dusk; delavvy;
 setactivepage(3); mousepage(3); setvisualpage(3); off;
 cleardevice; setfillstyle(xhatchfill,11);
 settextstyle(1,0,4); settextjustify(1,1);
 dplot(320,10,title);
 settextstyle(1,0,0); setusercharsize(4,3,7,12);
 for fv:=0 to 11 do
 begin;
  dplot(26+fv*52,187,months[fv]);
  bar(fv*52,177-fv*14,51+fv*52,180);
  rectangle(fv*52,177-fv*14,51+fv*52,180);
 end;
 settextstyle(0,0,1);
 for fv:=1 to 177 do
 begin;
  gd:=random(630); gm:=random(160)+30;
  setcolor(lightred); outtextxy(gd  ,gm  ,fish);
  setcolor(yellow);   outtextxy(gd+1,gm-1,fish);
 end;
 newpointer(6); { TTHand }
 dawn; on; setbkcolor(1); repeat check until (mpress>0) or keypressed;
 while keypressed do r:=readkey; setbkcolor(0); settextjustify(0,0);
 dusk; setvisualpage(0); setactivepage(0); mousepage(0); dawn;
 copy02;*/
{
    byte fv;
;
 dusk();
 off_virtual();
 for( fv=0; fv <= 1; fv ++)
 {;
  setactivepage(fv);
  cleardevice();
 }
 load(98); off();
 setactivepage(1); setvisualpage(1);
 settextjustify(1,0); setcolor(8);
 outtextxy(320,177,"Graph/Histo/Draw/Sample: \"JANJUN93.GRA\": (W3-AB3)");
 outtextxy(320,190,"Press any key or click the mouse to return.");
 settextjustify(2,0);
 on(); mousepage(1); newpointer(1); dawn();
 do { check(); } while (!((mpress>0) || keypressed()));
 off(); on_virtual();
 major_redraw();

 mousepage(cp);
}

void copy02()     /* taken from Wobble (below) */
{
    byte a0 /*absolute $A000:0*/; byte a2/*absolute $A800:0*/; byte bit;
;
 off();
 for( bit=0; bit <= 3; bit ++) {;
  port[0x3c4]=2; port[0x3ce]=4;
  port[0x3c5]=1 << bit;
  port[0x3cf]=bit;
  move(a0,a2,16000);
 } on();
}

void copy03()     /* taken from Wobble (below) */
{
    byte a0 /*absolute $A000:0*/; byte a2/*absolute $AC00:0*/; byte bit;
 byte squeaky_code;

   switch (visible) {
      case m_virtual : { squeaky_code = 1; off_virtual(); } break;
      case m_no      :       squeaky_code = 2; break;
      case m_yes     : { squeaky_code = 3; off();         } break;
   }

 for( bit=0; bit <= 3; bit ++) {;
  port[0x3c4]=2; port[0x3ce]=4;
  port[0x3c5]=1 << bit;
  port[0x3cf]=bit;
  move(a0,a2,16000);
 }

 switch (squeaky_code) {
   case 1 : on_virtual(); break;
   case 2 :; break; /* zzzz, it was off anyway */
   case 3 : on(); break;
 }

}

void copypage(byte frp,byte top)  /* taken from Copy02 (above) */
{
 byte bit;
;
 off();
 for( bit=0; bit <= 3; bit ++) {;
  port[0x3c4]=2; port[0x3ce]=4;
  port[0x3c5]=1 << bit;
  port[0x3cf]=bit;
  move(mem[0xa000*frp*pagetop],mem[0xa000*top*pagetop],16000);
 } on();
}

void wobble()
{
 byte bit;
 byte a2 /*absolute $A800:80i*/;
;
 off();
 setactivepage(2); bit=getpixel(0,0);
 cleardevice();
 for( bit=0; bit <= 3; bit ++) {;
  port[0x3c4]=2; port[0x3ce]=4;
  port[0x3c5]=1 << bit;
  port[0x3cf]=bit;
  move(mem[0xa000*cp*pagetop],a2,16000);
 }
 for( bit=0; bit <= 25; bit ++)
 {;
  setvisualpage(2);  delay(bit*7);
  setvisualpage(cp); delay(bit*7);
 }
 bit=getpixel(0,0);
 draw_also_lines();
 setactivepage(1-cp); on();
}

void zonk();

static void zl(integer x1,integer y1,integer x2,integer y2)
{;
 setlinestyle(0,0,3); setcolor( 1); line(x1,y1,x2,y2);
 setlinestyle(0,0,1); setcolor(11); line(x1,y1,x2,y2);
}

void zonk()
{
 integer xx,yy;
/* a0:byte absolute $A000:0; a3:byte absolute $A000:245760;*/ byte bit,fv;
;
 off();
 copypage(3,1-cp);
 {
 triptype& with = tr[1]; 
 ; xx=with.x+with.a.xl / 2; yy=with.y; }

 setactivepage(3); cleardevice();
/* for bit:=0 to 3 do begin;
  port[$3C4]:=2; port[$3CE]:=4;
  port[$3C5]:=1 shl bit;
  port[$3CF]:=bit;
  move(a0,a3,16000);
 end;*/
 copypage(cp,3); off();
 zl(640,0,0,yy / 4);
 zl(0,yy / 4,640,yy / 2);
 zl(640,yy / 2,xx,yy); setbkcolor(yellow);

 for( bit=0; bit <= 255; bit ++)
 {;
  note(270-bit); setvisualpage(3);
  note(2700-10*bit); delay(5); nosound;
  note(270-bit); setvisualpage(cp);
  note(2700-10*bit); delay(5); nosound;
 } setactivepage(0); setbkcolor(black); on(); state(2);
 copypage(1-cp,3);
}

void winning_pic()
{
 byte bit;
 untyped_file f;
 char r;

 dusk();

 assign(f,"finale.avd");
 reset(f,1);
 for( bit=0; bit <= 3; bit ++)
 {
  port[0x3c4]=2; port[0x3ce]=4;
  port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,mem[0xa000*0],16000);
 }
 close(f); blitfix();

 setvisualpage(0);

 dawn();

 do { check(); } while (!(keypressed() || (mrelease>0)));
 while (keypressed())  r=readkey();
 major_redraw();
}


