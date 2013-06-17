/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 SCROLLS          The scroll driver. */
/*  $D-*/
#define __scrolls_implementation__

/*$V-*/

#include "scrolls.h"

#include "lucerna.h"
 #include "Graph.h"
 /*#include "Crt.h"*/
#include "trip5.h"
#include "enhanced.h"
 /*#include "Dos.h"*/
#include "logger.h"
 #include "Acci.h"
#include "basher.h"
#include "visa.h"
#include "timeout.h"


const integer roman = 0;
const integer italic = 1;

const integer halficonwidth = 19; /* Half the width of an icon. */

integer dix,diy;
array<roman,italic,raw> ch;
byte cfont; /* Current font */

integer dodgex,dodgey;
byte param; /* For using arguments code */

byte use_icon;

void state(byte x)       /* Sets "Ready" light to whatever */
{
    byte page_;

 if (ledstatus==x)  return; /* Already like that! */
 switch (x) {
  case 0: setfillstyle(1,black); break; /* Off */
  case 1: setfillstyle(9,green); break; /* Half-on (menus) */
  case 2: setfillstyle(1,green); break; /* On (kbd) */
  case 3: setfillstyle(6,green); break; /* Hit a key */
 }
 super_off();
 for( page_=0; page_ <= 1; page_ ++)
  { setactivepage(page_); bar(419,195,438,197); } super_on();
 ledstatus=x;
}

void easteregg()
{
 word fv,ff;

 background(15);
 for( fv=4; fv <= 100; fv ++)
  for( ff=0; ff <= 70; ff ++)
   { sound(fv*100+ff*10); delay(1); }
 nosound; setcolor(10);
 settextstyle(0,0,3); settextjustify(1,1); outtextxy(320,100,"GIED");
 settextstyle(0,0,1); settextjustify(0,2);
 background(0);
}

void say(integer x,integer y, string z) /* Fancy FAST screenwriting */
{
      const integer locol = 2;
 byte xx,yy,ox,bit,lz,t; integer yp; boolean offset;
 matrix<1,12,1,80,byte> itw;

 offset=x % 8==4; x=x / 8; lz=length(z); ox=0;
 log_scrollline();

 for( xx=1; xx <= lz; xx ++)
 {
  switch (z[xx]) {
   case '\22': { cfont=roman; log_roman(); } break;
   case '\6': { cfont=italic; log_italic(); } break;
   default: {
    ox += 1;
    for( yy=1; yy <= 12; yy ++) itw[yy][ox]=~ ch[cfont][z[xx]][yy+1];
    log_scrollchar(z[xx]);
   }
  }
 }

 lz=ox;
 if (offset) 
 {     /* offsetting routine */
  lz += 1;
  for( yy=1; yy <= 12; yy ++)
  {
   bit=240; itw[yy][lz]=255;
   for( xx=1; xx <= lz; xx ++)
   {
    t=itw[yy][xx];
    itw[yy][xx]=bit+t / 16;
    bit=t << 4;
   }
  }
 }
 yp=x+y*80+(1-cp)*pagetop;
 for( yy=1; yy <= 12; yy ++)
 {
  yp += 80;
  for( bit=0; bit <= locol; bit ++)
  {
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   move(itw[yy],mem[0xa000*yp],lz);
  }
 }
}

/* Here are the procedures that Scroll calls */ /* So they must be... */ /*$F+*/

void normscroll()
{
 const array<1,8,char> egg = string("\20\f\25\7\b")+"***";
 const array<1,8,char> e = "(c) 1994";
 char r;
 boolean oktoexit;

 state(3); seescroll=true;
 off_virtual();
 on(); newpointer(4);
 mousepage(1-cp);

 if (demo)  get_demorec();

 do {
  do {
   check(); /* was "checkclick;" */

 #ifdef RECORD slowdown(); basher::count += 1; #endif

   if (demo) 
   {
    if (demo_ready())  flush();
    if (keypressede())  exit(0);
   } else
    if (keypressede())  flush();
  } while (!((mrelease>0) || (buttona1()) || (buttonb1())));


  if (mrelease==0) 
  {
   inkey();
   if (aboutscroll) 
   {
    move(e[2-1],e[1-1],7);
    e[8-1]=inchar;
    if (egg==e)  easteregg();
   }
   oktoexit=set::of('\15','\33','+','#', eos).has(inchar);
   if (! oktoexit)  errorled();
  }

 } while (!((oktoexit) || (mrelease>0)));
 #ifdef RECORD record_one(); #endif
 screturn=r=='#'; /* "back door" */
 state(0); seescroll=false; mousepage(cp); off();
}

void dialogue()
{
    char r;

 state(3); seescroll=true; r='\0';
 newpointer(6); on();
 mousepage(1-cp);
 do {
  do {
   check();
   if (mrelease>0) 
   {
    if ((mx>=(cardinal)dix-65) && (my>=(cardinal)diy-24) && (mx<=(cardinal)dix- 5) && (my<=(cardinal)diy-10))
           r='Y';
    if ((mx>=(cardinal)dix+ 5) && (my>=(cardinal)diy-24) && (mx<=(cardinal)dix+65) && (my<=(cardinal)diy-10))
           r='N';
   } else
       if (keypressede()) 
       {
        inkey();
        r=upcase(inchar);
       }
  } while (!(r!='\0'));
 } while (!(set::of('Y','N','O','J', eos).has(r))); /* Yes, Ja, Oui, or No, Non, Nein */
 screturn=r!='N';
 state(0); seescroll=false; mousepage(cp); off();
}

void music_scroll();


static void store_(byte what, tunetype& played)
{
 move(played[2],played[1],sizeof(played)-1);
 played[31]=what;
}



static boolean they_match(tunetype& played)
{
    byte fv,mistakes;

 boolean they_match_result;
 mistakes=0;

 for( fv=1; fv <= sizeof(played); fv ++)
  if (played[fv]!=tune[fv]) 
  {
   mistakes += 1;
  }

 they_match_result=mistakes<5;
 return they_match_result;
}

void music_scroll()
{
 char r;
 byte value;

 byte last_one,this_one;

 tunetype played;


 state(3); seescroll=true; on();
 newpointer(4);
 do {
  do {
   check(); /* was "checkclick;" */
   if (keypressede())  flush();
  } while (!((mpress>0) || (buttona1()) || (buttonb1())));

  if (mpress==0) 
  {
   inkey();
   r=upcase(inchar);  if (r=='Z')  r='Y';    /* Euro keyboards */

   value=pos(r,keys);

   if (value>0) 
   {

    last_one=this_one;
    this_one=value;

    sound(notes[this_one]);
    delay(100);
    nosound;

    if (! dna.ringing_bells) 
    {     /* These handle playing the right tune. */

     if (this_one<last_one) 
      store_(lower, played); else

       if (this_one==last_one) 
        store_(same, played); else

         store_(higher, played);

     if (they_match(played)) 
     {
      screturn=true;
      off();
      state(0); seescroll=false;

      set_up_timer(8,procjacques_wakes_up,reason_jacques_waking_up);
      return;
     }

    }

   }

  }

 } while (!((set::of('\15','\33','+','#', eos).has(r)) || (mpress>0)));
 screturn=false;
 off();
 state(0); seescroll=false;
}

/* ThatsAll, so put us back to */ /*$F-*/

void resetscrolldriver()     /* phew */
{
 scrollbells=0; cfont=roman; log_epsonroman(); use_icon=0;
 interrogation=0;  /* always reset after a scroll comes up. */
}

void dingdongbell()     /* Pussy's in the well. Who put her in? Little... */
{
    byte fv;

 for( fv=1; fv <= scrollbells; fv ++) errorled(); /* ring the bell "x" times */
}

void dodgem()     /* This moves the mouse pointer off the scroll so that
 you can read it. */
{
 xycheck(); /* Mx & my now contain xy pos of mouse */
 dodgex=mx; dodgey=my;   /* Store 'em */
 hopto(dodgex,underscroll); /* Move the pointer off the scroll. */
}

void undodgem()     /* This is the opposite of Dodgem. It moves the
 mouse pointer back, IF you haven't moved it in the meantime. */
{
 xycheck();
 if ((mx==(cardinal)dodgex) && (my==(cardinal)underscroll)) 
  /* No change, so restore the pointer's original position. */
  hopto(dodgex,dodgey);
}

void geticon(integer x,integer y, byte which)
{
 untyped_file f;
 pointer p;

 assign(f,"icons.avd");
 reset(f,1);
 which -= 1;
 seek(f,which*426);
 getmem(p,426);
 blockread(f,p,426);
 putimage(x,y,p,0);
 freemem(p,426);
 close(f);
}

void block_drop(string fn, integer xl,integer yl,integer y)
{
 untyped_file f; byte bit;
 integer fv; word st;

 st=(y-1)*80+(40-xl / 2)+((1-cp)*pagetop);

 assign(f,fn+".avd");
 reset(f,1);

 for( fv=1; fv <= yl; fv ++)
  for( bit=0; bit <= 3; bit ++)
  {;
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   blockread(f,mem[0xa000*st+(fv*80)],xl);
  }

 close(f);
 bit=getpixel(0,0);
}

void drawscroll(proc gotoit)       /* This is one of the oldest procs in the game. */
{
 byte b,groi; integer lx,ly,mx,my,ex,ey; boolean centre;
 byte icon_indent;

 off_virtual();
 setvisualpage(cp); setactivepage(1-cp);
 oncandopageswap=false;  /* On can now no longer swap pages. So we can
  do what we want without its interference! */
 log_epsonroman(); /* Scrolls always START with Roman. */
 lx=0; ly=scrolln*6;
 for( b=1; b <= scrolln; b ++)
 {
  ex=length(scroll[b])*8; if (lx<ex)  lx=ex;
 }
 mx=320; my=100;   /* Getmaxx & getmaxy div 2, both. */
 lx=lx / 2; ly -= 2;

 if (set::of(range(1,34), eos).has(use_icon))  lx += halficonwidth;

 off();
/* mblit(mx-lx-46,my-ly-6,mx+lx+15,my+ly+6,0,3);*/
 setfillstyle(1,7);
 setcolor(7);
 pieslice(mx+lx,my-ly,360,90,15);
 pieslice(mx+lx,my+ly,270,360,15);
 setcolor(4);
 arc(mx+lx,my-ly,360,90,15);
 arc(mx+lx,my+ly,270,360,15);
 bar(mx-lx-30,my+ly+6,mx+lx,my+ly);
 bar(mx-lx-30,my-ly-6,mx+lx,my-ly);
 bar(mx-lx-15,my-ly,mx+lx+15,my+ly);
 setfillstyle(1,8);
 pieslice(mx-lx-31,my-ly,360,180,15);
 pieslice(mx-lx-31,my+ly,180,360,15);
 setfillstyle(1,4);
 bar(mx-lx-30,my-ly-6,mx+lx,my-ly-6);
 bar(mx-lx-30,my+ly+6,mx+lx,my+ly+6);
 bar(mx-lx-15,my-ly,mx-lx-15,my+ly);
 bar(mx+lx+15,my-ly,mx+lx+15,my+ly);
 ex=mx-lx; ey=my-ly;
 mx -= lx; my -= ly+2;
 setcolor(0); centre=false;

 switch (use_icon) {
      case 0: icon_indent=0; break; /* No icon. */
  case 1 ... 33: {           /* Standard icon */
          geticon(mx,my+ly / 2,use_icon);
          icon_indent=53;
         }
         break;
     case 34: { block_drop("about",28,76,15);    icon_indent=0; } break;
     case 35: { block_drop("gameover",52,59,71); icon_indent=0; } break;
 }

 for( b=1; b <= scrolln; b ++)
 {
  switch (scroll[b][length(scroll[b])]) {
   case '\3' : { centre=true;  scroll[b][0] -= 1; } break;
   case '\14' : { centre=false; scroll[b][0] -= 1; } break;
   case '\21' : { settextjustify(1,1);
         dix=mx+lx; diy=my+ly; scroll[b][1]='\40'; groi=getpixel(0,0);
/*         inc(diy,14);*/
         shbox(dix-65,diy-24,dix- 5,diy-10,"Yes.");
         shbox(dix+ 5,diy-24,dix+65,diy-10,"No.");
        }
        break;
  }

  if (centre) 
   say(320-length(scroll[b])*4+icon_indent,my,scroll[b]);
  else
   say(mx+icon_indent,my,scroll[b]);
  log_scrollendline(centre);
  my += 12;
 }

 underscroll=my+3;
 setvisualpage(1-cp); dingdongbell();
 my=getpixel(0,0); dropsok=false; dodgem();

 gotoit();

 undodgem(); dropsok=true;
 log_divider();
 setvisualpage(cp); mousepage(cp); off();
/* mblit(ex-46,ey-6,ex+lx*2+15,ey+ly*2+6,3,0);*/
 mblit((ex-46) / 8,ey-6,1+(ex+lx*2+15) / 8,ey+ly*2+6,cp,1-cp);
 blitfix();
 oncandopageswap=true;  /* Normality again */
 on(); settextjustify(0,0); /*sink*/
 resetscrolldriver();
 if (mpress>0)  after_the_scroll=true;
}

void bubble(proc gotoit)
{
 integer xl,yl,my,xw,yw; byte fv; array<1,3,pointtype> p;
 pointer rp1,rp2; /* replace: 1=bubble, 2=pointer */
 integer xc; /* x correction */

 setvisualpage(cp); setactivepage(1-cp);
 oncandopageswap=false;  /* On can now no longer swap pages. So we can
  do what we want without its interference! */
 mousepage(1-cp); /* Mousepage */

 setfillstyle(1,talkb); setcolor(talkb); off();

 xl=0; yl=scrolln*5;
 for( fv=1; fv <= scrolln; fv ++)
  if (textwidth(scroll[fv])>xl)  xl=textwidth(scroll[fv]);
 xl=xl / 2;

 xw=xl+18; yw=yl+7;
 my=yw*2-2; xc=0;

 if ((talkx-xw)<0)  xc=-(talkx-xw);
 if ((talkx+xw)>639)  xc=639-(talkx+xw);

 p[1].x=talkx-10; p[1].y=yw;
 p[2].x=talkx+10; p[2].y=yw;
 p[3].x=talkx;    p[3].y=talky;

/* mblit(talkx-xw+xc,7,talkx+xw+xc,my,0,3);
 mblit(talkx-10,my,talkx+10,talky,0,3);*/
 bar(xc+talkx-xw+10,7,talkx+xw-10+xc,my);
 bar(xc+talkx-xw,12,talkx+xw+xc,my-5);
 pieslice(xc+talkx+xw-10,12,360,90,9);    /* TR */
 pieslice(xc+talkx+xw-10,my-5,270,360,9); /* BR */
 pieslice(xc+talkx-xw+10,12,90,180,9);    /* TL */
 pieslice(xc+talkx-xw+10,my-5,180,270,9); /* BL */
 fillpoly(3,p.body());

 setcolor(talkf); yl -= 3; settextjustify(1,2);
 for( fv=0; fv <= scrolln-1; fv ++)
  outtextxy(talkx+xc,(fv*10)+12,scroll[fv+1]);
 for( fv=1; fv <= scrolln; fv ++) /* These should be separate loops. */
  log_bubbleline(fv,param,scroll[fv]);
 log_divider();

 setvisualpage(1-cp);
 dingdongbell();
 oncandopageswap=false;
 on(); dropsok=false; gotoit(); off(); dropsok=true;
 mblit((talkx-xw+xc) / 8,7,1+(talkx+xw+xc) / 8,my,3,1-cp);
 mblit((talkx-10) / 8,my,1+(talkx+10) / 8,talky,3,1-cp);
 blitfix();

 setvisualpage(cp);
 settextjustify(0,0); on(); /*sink;*/
 oncandopageswap=true;
 resetscrolldriver();
 if (mpress>0)  after_the_scroll=true;
}

boolean ask(string question)
{
 boolean ask_result;
 display(question+'\15'+'\21');
 if (screturn && (Random(2)==0)) /* half-and-half chance */ 
 {
  display("...Positive about that?\23I\26\r\21"); /* be annoying! */
  if (screturn && (Random(4)==3)) /* another 25% chance */ 
   display("\n100% certain\??!\n\26\r\21"); /* be very annoying! */
 }
 ask_result=screturn;
 return ask_result;
}

void resetscroll()
{
 scrolln=1; fillchar(scroll,sizeof(scroll),'\0');
}

void natural()     /* Natural state of bubbles */
{
 talkx=320; talky=200; talkb=8; talkf=15;
}

string lsd()
{
    string x;

 string lsd_result;
 if (dna.pence<12) 
 {     /* just pence */
  x=strf(dna.pence)+'d';
 } else
  if (dna.pence<240) 
  {     /* shillings & pence */
   x=strf(dna.pence / longint(12))+'/';
   if ((dna.pence % longint(12))==0)  x=x+'-'; else x=x+strf(dna.pence % longint(12));
  } else   /* L, s & d */
   x=string('ú')+strf(dna.pence / longint(240))+'.'+strf((dna.pence / longint(12)) % longint(20))+'.'+
    strf(dna.pence % longint(12));
 if (dna.pence>12)  x=x+" (that's "+strf(dna.pence)+"d)";
 lsd_result=x;
 return lsd_result;
}

void calldrivers();
                      /* Was the mouse cursor virtual on entry to this proc? */

static void strip(string& q)
{
 while (pos("\40",q[length(q)])>0)  q[0] -= 1; /* strip trailing spaces */
}



static void solidify(byte n)
{
 if (pos("\40",scroll[n])==0)  return; /* no spaces */
 /* so there MUST be a space there, somewhere... */
 do {
  scroll[n+1]=string(scroll[n][length(scroll[n])])+scroll[n+1];
  scroll[n][0] -= 1;
 } while (!(scroll[n][length(scroll[n])]=='\40'));
 strip(scroll[n]);
}

void calldrivers()
{
 word fv; byte nn; char nnn; boolean mouthnext;
 boolean call_spriterun; /* Only call sprite_run the FIRST time. */

 boolean was_virtual;


 nosound; state(0); screturn=false; mouthnext=false;
 call_spriterun=true;

 switch (buffer[bufsize]) {
   case '\4': bufsize -= 1; break; /* ^D = (D)on't include pagebreak */
   case '\2':case '\21':; break; /* ^B = speech (B)ubble, ^Q = (Q)uestion in dialogue box */
  default: {
        bufsize += 1; buffer[bufsize]='\20';
       }
 }
 for( fv=1; fv <= bufsize; fv ++)
  if (mouthnext) 
  {
   if (buffer[fv]=='\23')  param=0; else
   switch (buffer[fv]) {
    case RANGE_10('0','9'): param=ord(buffer[fv])-48;
    break;
    case RANGE_26('A','Z'): param=ord(buffer[fv])-55;
    break;
   }
   mouthnext=false;
  } else
   switch (buffer[fv]) {
    case '\20': {
         if ((scrolln==1) && (scroll[1]==""))  flush();

         if (call_spriterun)  sprite_run();
         call_spriterun=false;

         was_virtual=visible==m_virtual;
         if (was_virtual)  off_virtual();
         drawscroll(normscroll);
         if (was_virtual)  on_virtual();
         resetscroll();
         if (screturn)  return;
        }
        break;
    case '\7': scrollbells += 1; break; /* #7 = "Bel" */
    case '\2': {
         if ((scrolln==1) && (scroll[1]==""))  flush();

         if (call_spriterun)  sprite_run();
         call_spriterun=false;
         switch (param) {
          case 0: natural(); break; /* Not attached: generic bubble with no speaker. */
          case RANGE_9(1,9): if ((param>numtr) || (! tr[param].quick)) 
                 {     /* not valid */
                  errorled();
                  natural();
                 }
                 else tr[param].chatter();
                 break;                  /* Normal sprite talking routine. */
          case 10 ... 36: {
                   quasiped_type& with = quasipeds[param]; 
                        /* Quasi-peds. (This routine performs the same
                   thing with QPs as triptype.chatter does with the
                   sprites.) */
                   {
                    pedtype& with1 = peds[with.whichped]; 

                    talkx=with1.x; talky=with1.y; /* Position. */
                   }
                   talkf=with.fgc; talkb=with.bgc; /* Colours. */
                          }
                          break;
          default: { errorled(); natural(); } /* not valid */
         }

         was_virtual=visible==m_virtual;
         if (was_virtual)  off_virtual();
         bubble(normscroll);
         if (was_virtual)  on_virtual();
         resetscroll();
         if (screturn)  return;
        }
        break;
    case '\25': {
         { dnatype& with = dna; 
          switch (param) {
           case 1: display(lsd()+'\4'); break; /* insert cash balance (recursion) */
           case 2: display(words[first_password+with.pass_num].w+'\4'); break;
           case 3: display(with.like2drink+'\4'); break;
           case 4: display(with.favourite_song+'\4'); break;
           case 5: display(with.worst_place_on_earth+'\4'); break;
           case 6: display(with.spare_evening+'\4'); break;
           /* ... */
           case 9: display(strf(with.cat_x)+','+strf(with.cat_y)+'\4'); break;
           case 10: switch (with.box_contents) {
                case '\0': { /* Sixpence. */
                     dixi('q',37); /* You find the sixpence. */
                     with.pence += 6;
                     with.box_contents=nowt;
                     points(2); return;
                    }
                    break;
                case nowt: display("nothing at all. It's completely empty."); break;
                default: display(get_better(with.box_contents)+'.');
               }
               break;
           case 11: {
                nn=1;
                for( nnn='\1'; nnn <= numobjs; nnn ++)
                if (with.obj[nnn]) 
                {
                 nn += 1;
                 display(get_better(nnn)+", "+'\4');
                }
               }
               break;
          }}
        }
        break;
    case '\26': use_icon=param; break;
    case '\15': scrolln += 1; break;
    case '\21': {
         if (call_spriterun)  sprite_run();
         call_spriterun=false;

         scrolln += 1; scroll[scrolln]='\21';
         was_virtual=visible==m_virtual;
         if (was_virtual)  off_virtual();
         drawscroll(dialogue); 
         if (was_virtual)  on_virtual();
         resetscroll();
        }
        break;
    case '\23' : mouthnext=true; break;
    case '\11' : for( nn=1; nn <= 9; nn ++) scroll[scrolln]=scroll[scrolln]+' ';
    break;
    default:
    {     /* add new char */
     if (length(scroll[scrolln])==50) 
     {
      solidify(scrolln);
      scrolln += 1;
     }
     scroll[scrolln]=scroll[scrolln]+buffer[fv];
   }
  }
}

void display(string z)
{
 bufsize=length(z);
 move(z[1],buffer,bufsize);
 calldrivers();
}

void loadfont()
{
    file<raw> f;

 assign(f,"avalot.fnt"); reset(f); f >> ch[0]; close(f);
 assign(f,"avitalic.fnt"); reset(f); f >> ch[1]; close(f);
 assign(f,"ttsmall.fnt"); reset(f); f >> little; close(f);
}

void okay()
{
 display("Okay!");
}

void musical_scroll()
{
    boolean was_virtual;

 display(string("To play the harp...\r\rUse these keys:\r\n")+
  "Q W E R T Y U I O P [ ]\r\rOr press Enter to stop playing.\4");

 sprite_run();

 was_virtual=visible==m_virtual;
 if (was_virtual)  off_virtual();
 drawscroll(music_scroll);
 if (was_virtual)  on_virtual();
 resetscroll();
}

class unit_scrolls_initialize {
  public: unit_scrolls_initialize();
};
static unit_scrolls_initialize scrolls_constructor;

unit_scrolls_initialize::unit_scrolls_initialize() {
 loadfont();
 resetscrolldriver();
}
