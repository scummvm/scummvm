

/*
  ÛßÜ ÛßÜ ÜßßÜ  ßÛß Ûßß  Üß ßÛß      ÜßÛßÜ  ßÛß ÛÜ  Û ÜÛßß  ßÛß ÛßÜ Ûßß Û
  Ûß  ÛÛ  Û  Û Ü Û  Ûßß ßÜ   Û      Û  Û  Û  Û  Û ÛÜÛ  ßßßÜ  Û  ÛÛ  Ûßß Û
  ß   ß ß  ßß   ßß  ßßß   ß  ß      ß  ß  ß ßßß ß  ßß  ßßß   ß  ß ß ßßß ßßß

                 TRIP42           Trippancy IV point 2 - "Trip Oop 2" */

#define __trip42_implementation__
             /* Trippancy IV.2 (Trip Oop) */
#include "trip42.h"


/*#include "Scrolls.h"*/
/*#include "Lucerna.h"*/
/*#include "Dropdown.h"*/
/*#include "Access.h"*/


/*procedure copier(x1,y1,x2,y2,x3,y3,x4,y4:integer);

  function dropin(xc,yc,x1,y1,x2,y2:integer):boolean;
  { Dropin returns True if the point xc,yc falls within the 1-2 rectangle. }
  begin;
   dropin:=((xc>=x1) and (xc<=x2) and (yc>=y1) and (yc<=y2));
  end;

  function lesser(a,b:integer):integer;
  begin;
   if a<b then lesser:=a else lesser:=b;
  end;

  function greater(a,b:integer):integer;
  begin;
   if a>b then greater:=a else greater:=b;
  end;

begin;
 if dropin(x3,y3,x1,y1,x2,y2)
 or dropin(x3,y4,x1,y1,x2,y2)
 or dropin(x4,y3,x1,y1,x2,y2)
 or dropin(x4,y4,x1,y1,x2,y2) then
 begin; { Overlaps }
  mblit(lesser(x1,x3),lesser(y1,y3),greater(x2,x4),greater(y2,y4),1,0);
 end else
 begin; { Doesn't overlap- copy both of them seperately }
  mblit(x3,y3,x4,y4,1,0); { backwards- why not...? }
  mblit(x1,y1,x2,y2,1,0);
 end;
end; */

void loadtrip()
{
    byte gm;
;
 for( gm=1; gm <= numtr; gm ++) tr[gm].original();
 tr[1].init(0);
}

byte checkfeet(integer x1,integer x2,integer oy,integer y, byte yl)
{
    byte a,c; integer fv,ff;
byte checkfeet_result;
;
 a=0; setactivepage(2); if (x1<0)  x1=0; if (x2>639)  x2=639;
 if (oy<y) 
  for( fv=x1; fv <= x2; fv ++)
   for( ff=oy+yl; ff <= y+yl; ff ++)
   {;
    c=getpixel(fv,ff);
    if (c>a)  a=c;
   } else
  for( fv=x1; fv <= x2; fv ++)
   for( ff=y+yl; ff <= oy+yl; ff ++)
   {;
    c=getpixel(fv,ff);
    if (c>a)  a=c;
   }
 checkfeet_result=a; setactivepage(1-cp);
return checkfeet_result;
}

void touchcol(byte tc);
static boolean bug; 

static void fr(byte a,byte b) {; fliproom(a,b); bug=false; }

void touchcol(byte tc)
{;
 bug=true;  /* j.i.c. */
 switch (dna.room) {
  case 1: fr(2,3); break;
  case 2: {; /* main corridor */
      switch (tc) {
       case 1: fr(3,1); break; /* to the other corridor */
       case 2: fr(2,1); break; /* to this corridor! Fix this later... */
      }
     }
     break;
  case 3: {; /* turn corridor */
      switch (tc) {
       case 1: fr(2,1); break; /* to the other corridor */
       case 2: fr(12,1); break; /* through Spludwick's door */
      }
     }
     break;
  case 12: fr(3,2); break;
 }
 if (bug) 
 {;
  display(string('\7')+"Unknown touchcolour ("+strf(tc)+')'+" in "+strf(dna.room)
   +'.'); tr[1].bounce();
 }
}

triptype* triptype::init(byte spritenum)
{
    integer gd,gm; word s; untyped_file f; varying_string<2> xx; byte sort,n;
 word bigsize; pointer p,q;
;
 str(spritenum,xx); assign(f,string("c:\\avalot\\sprite")+xx+".avd");
 reset(f,1); seek(f,59);
 blockread(f,a,sizeof(a)); blockread(f,bigsize,2);
 setactivepage(3);
 for( sort=0; sort <= 1; sort ++)
 {;
  mark(q); getmem(p,bigsize);
  blockread(f,p,bigsize);
  off; putimage(0,0,p,0); release(q); n=1;
  { adxtype& with = a; 
   for( gm=0; gm <= (with.num / with.seq)-1; gm ++) /* directions */
    for( gd=0; gd <= with.seq-1; gd ++) /* steps */
    {;
     getmem(pic[n][sort],a.size); /* grab the memory */
     getimage((gm / 2)*(with.xl*6)+gd*with.xl,(gm % 2)*with.yl,
       (gm / 2)*(with.xl*6)+gd*with.xl+with.xl-1,(gm % 2)*with.yl+with.yl-1,
       pic[n][sort]); /* grab the pic */
     n += 1;
   }}   /*on;*/
 }
 close(f); setactivepage(1-cp);

 x=0; y=0; quick=true; visible=false;
/* for x:=0 to 1 do getmem(behind[x],a.size);*/
 homing=false; ix=0; iy=0; step=0; check_me=a.name=="Avalot";
 count=0;
return this;
}

void triptype::original()
{;
 quick=false;
}

void triptype::andexor()
{
    byte picnum; /* Picnum, Picnic, what ye heck */
;
 picnum=face*a.seq+step+1; off;
 putimage(x,y,pic[picnum][0],andput);
 putimage(x,y,pic[picnum][1],xorput); /*on;*/
}

void triptype::turn(byte whichway)
{;
 if (whichway==8)  face=0; else face=whichway;
}

void triptype::appear(integer wx,integer wy, byte wf)
{;
 x=(wx / 8)*8; y=wy; ox[cp]=wx; oy[cp]=wy; turn(wf);
 visible=true; ix=0; iy=0;
}

void triptype::walk()
{
    byte tc; fieldtype r;
;
 {;
  x1=x-1; y1=y-2; x2=x+a.xl+1; y2=y+a.yl+2;
 }
 getset[1-cp].remember(r);
 ox[cp]=x; oy[cp]=y;
 if (homing)  homestep();
 x=x+ix; y=y+iy;
 if (check_me)  {;
  tc=checkfeet(x,x+a.xl,oy[cp],y,a.yl);
  if (tc!=0) 
   { void& with = magics[tc]; 
    switch (op) {
     case exclaim: {; bounce(); dixi('x',data); } break;
     case bounces: bounce(); break;
     case transport: fliproom(hi(data),lo(data)); break;
     case unfinished: {;
                  bounce();
                  display("\7Sorry.\3\rThis place is not avaliable yet!");
                 }
                 break;
    }}
 }
 count += 1;
 if (((ix!=0) || (iy!=0)) && (count>1)) 
 {;
  step += 1; if (step==a.seq)  step=0; count=0;
 }
}

void triptype::bounce()
{; x=ox[cp]; y=oy[cp]; stopwalk(); return; }

void triptype::walkto(integer xx,integer yy)
{;
 speed(xx-x,yy-y); hx=xx; hy=yy; homing=true;
}

void triptype::stophoming()
{;
 homing=false;
}

void triptype::homestep()
{
    integer temp;
;
 if ((hx==x) && (hy==y)) 
 {;     /* touching the target */
  homing=false; return;
 }
 ix=0; iy=0;
 if (hy!=y) 
 {;
  temp=hy-y; if (temp>4)  iy=4; else if (temp<-4)  iy=-4; else iy=temp;
 }
 if (hx!=x) 
 {;
  temp=hx-x; if (temp>4)  ix=4; else if (temp<-4)  ix=-4; else ix=temp;
 }
}

void triptype::speed(shortint xx,shortint yy)
{;
 ix=xx; iy=yy;
 if ((ix==0) && (iy==0))  return; /* no movement */
 if (ix==0) 
 {;     /* No horz movement */
  if (iy<0)  turn(up); else turn(down);
 } else
 {;
  if (ix<0)  turn(left); else turn(right);
 }
}

void triptype::stopwalk()
{;
 ix=0; iy=0; homing=false;
}

void triptype::chatter()
{;
 talkx=x+a.xl / 2; talky=y; talkf=a.fgc; talkb=a.bgc;
}

getsettype* getsettype::init()
{;
 numleft=0;  /* initialise array pointer */
 return this;
}

void getsettype::remember(fieldtype r)
{;
 numleft += 1;
 gs[numleft]=r;
}

void getsettype::recall(fieldtype& r)
{;
 r=gs[numleft];
 numleft -= 1;
}

void rwsp(byte t,byte r)
{
      const integer xs = 3; const integer ys = 1;
;
 { triptype& with = tr[t];  switch (r) {
      case up: speed(  0,-ys); break; case down: speed(  0, ys); break; case left: speed(-xs,  0); break;
   case right: speed( xs,  0); break;   case ul: speed(-xs,-ys); break;   case ur: speed( xs,-ys); break;
      case dl: speed(-xs, ys); break;   case dr: speed( xs, ys); break;
  }}
}

void apped(byte trn,byte np)
{;
 {
 triptype& with = tr[trn]; 
 ; { void& with1 = peds[np];  appear(with.x-with.a.xl / 2,with.y-with.a.yl,dir);}
  rwsp(trn,tr[trn].face); }
}

void getback()
{
 byte fv;
 fieldtype r;
;
  { getsettype& with = getset[1-cp]; 
   while (with.numleft>0) 
   {;
    recall(r);
    mblit(x1,y1,x2,y2,3,1-cp);
   }}
}

void trippancy_link()
{
 byte fv;
;
 if ((ddm_o.menunow) | ontoolbar | seescroll | (~ alive))  return;
 for( fv=1; fv <= numtr; fv ++) { triptype& with = tr[fv];  if (with.quick)  getback();}
 for( fv=1; fv <= numtr; fv ++) { triptype& with = tr[fv];  if (with.quick)  walk();}
 for( fv=1; fv <= numtr; fv ++) { triptype& with = tr[fv];  if (with.quick && with.visible)  andexor();}
}

void stopwalking()
{;
 tr[1].stopwalk(); dna.rw=stopped;
}

void tripkey(char dir)
{;
 if (ctrl==cjoy)  return;

 { triptype& with = tr[1]; 
  {;
   switch (dir) {
    case 'H': if (rw!=up)    
            {; rw=up;    rwsp(1,rw); } else stopwalking();
            break;
    case 'P': if (rw!=down)  
            {; rw=down;  rwsp(1,rw); } else stopwalking();
            break;
    case 'K': if (rw!=left)  
            {; rw=left;  rwsp(1,rw); } else stopwalking();
            break;
    case 'M': if (rw!=right) 
            {; rw=right; rwsp(1,rw); } else stopwalking();
            break;
    case 'I': if (rw!=ur)    
            {; rw=ur;    rwsp(1,rw); } else stopwalking();
            break;
    case 'Q': if (rw!=dr)    
            {; rw=dr;    rwsp(1,rw); } else stopwalking();
            break;
    case 'O': if (rw!=dl)    
            {; rw=dl;    rwsp(1,rw); } else stopwalking();
            break;
    case 'G': if (rw!=ul)    
            {; rw=ul;    rwsp(1,rw); } else stopwalking();
            break;
    case 'L': stopwalking(); break;
   }
 }}
}

void getsetclear()
{
    byte fv;
;
 for( fv=0; fv <= 1; fv ++) getset[fv].init();
}

void fliproom(byte room,byte ped)
{;
 dusk; getsetclear(); enterroom(room); apped(1,ped);
 oldrw=dna.rw; dna.rw=tr[1].face; showrw; dawn;
}

boolean infield(byte x)           /* returns True if you're within field "x" */
{
    integer yy;
boolean infield_result;
;
 { void& with = fields[x];  {
 triptype& with = tr[1]; 
 ;
  yy=with.y+with.a.yl;
  infield_result=(with.x>=x1) && (with.x<=x2) && (yy>=y1) && (yy<=y2);
                            }}
return infield_result;
}

boolean neardoor()         /* returns True if you're near a door! */
{
    integer ux,uy; byte fv; boolean nd;
boolean neardoor_result;
;
 if (numfields<9) 
 {;     /* there ARE no doors here! */
  neardoor_result=false;
  return neardoor_result;
 }
 {
 triptype& with = tr[1]; 
 ;
  ux=with.x;
  uy=with.y+with.a.yl;
 } nd=false;
 for( fv=9; fv <= numfields; fv ++)
  {
  void& with = fields[fv]; 
  ;
   if ((ux>=x1) && (ux<=x2) && (uy>=y1) && (uy<=y2))  nd=true;
  }
 neardoor_result=nd;
return neardoor_result;
}

void readstick()
{
    byte jw;
;
 if (ctrl==ckey)  return;

 jw=joyway;

 {
 triptype& with = tr[1]; 
 ;
  if (jw==stopped)  stopwalking(); else
  {;
   dna.rw=jw; rwsp(1,dna.rw);
  }
 }

 if (jw!=oldjw) 
 {;
  showrw;
  oldjw=jw;
 }

}

class unit_trip42_initialize {
  public: unit_trip42_initialize();
};
static unit_trip42_initialize trip42_constructor;

unit_trip42_initialize::unit_trip42_initialize() {;
 getsetclear();
}
