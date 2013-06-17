#include "ptoc.h"

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 DROPDOWN         A customised version of Oopmenu (qv). */

#define __dropdown_implementation__


#include "dropdown.h"


/*#include "Crt.h"*/
#include "graph.h"
/*#include "Dos.h"*/
#include "lucerna.h"
#include "Gyro.h"
#include "acci.h"
#include "trip5.h"
#include "enid.h"
#include "basher.h"

/*$V-*/
const integer indent = 5;
const integer spacing = 10;

/* menu_b = blue; { Morpheus }
 menu_f = yellow;
 menu_border = black;
 highlight_b = lightblue;
 highlight_f = yellow;
 disabled = lightgray; */

const integer menu_b = lightgray; /* Windowsy */
const integer menu_f = black;
const integer menu_border = black;
const integer highlight_b = black;
const integer highlight_f = white;
const integer disabled = darkgray;

char r;
byte fv;

void find_what_you_can_do_with_it()
{;
 switch (thinks) {
  case wine:case ink: verbstr=string(vb_exam)+vb_drink; break;
  case bell: verbstr=string(vb_exam)+vb_ring; break;
  case potion:case wine: verbstr=string(vb_exam)+vb_drink; break;
  case chastity: verbstr=string(vb_exam)+vb_wear; break;
  case lute: verbstr=string(vb_exam)+vb_play; break;
  case mushroom:case onion: verbstr=string(vb_exam)+vb_eat; break;
  case clothes: verbstr=string(vb_exam)+vb_wear; break;
  default: verbstr=vb_exam; /* anything else */
 }
}

void chalk(integer x,integer y, char t, string z, boolean valid)
{
 byte fv,ff,p,bit;
 word pageseg;
 byte ander;
;

 pageseg=0xa000+cp*0x400;

 if (valid)  ander=255; else ander=170;

 for( fv=1; fv <= length(z); fv ++)
  for( ff=0; ff <= 7; ff ++)
   for( bit=0; bit <= 2; bit ++)
   {;
    port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
    mem[pageseg*x+fv-1+(y+ff)*80]=~ (little[z[fv]][ff] & ander);
   }

 for( ff=0; ff <= 8; ff ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << 3; port[0x3cf]=3;
  fillchar(mem[pageseg*x+(y+ff)*80],length(z),'\0'); /* blank it out. */
 }

 p=pos(t,z); if (p==0)  return; p -= 1;

 for( bit=0; bit <= 2; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  mem[pageseg*x+p+(y+8)*80]=~ ander;
 }

 blitfix();
}

void hlchalk(integer x,integer y, char t, string z, boolean valid)
 /* Highlighted. */
{
 byte fv,ff,p;
 word pageseg;
 byte ander;
;

 pageseg=0xa000+cp*0x400;

 if (valid)  ander=255; else ander=170;

 for( fv=1; fv <= length(z); fv ++)
  for( ff=0; ff <= 7; ff ++)
   mem[pageseg*x+fv-1+(y+ff)*80]=little[z[fv]][ff] & ander;

 p=pos(t,z); if (p==0)  return; p -= 1;

 mem[pageseg*x+p+(y+8)*80]=ander;
 blitfix();
}

/*procedure say(x,y:integer; t:char; z:string; f,b:byte);
begin;
 settextjustify(0,2); setfillstyle(1,b); setcolor(f);
 bar(x-3,y-1,x+textwidth(z)+3,y+textheight(z));
 chalk(x,y,t,z);
end;*/

void bleep()
{;
 sound(177); delay(7); nosound;
}

void onemenu::start_afresh()
{;
 number=0; width=0; firstlix=false; oldy=0; highlightnum=0;
}

onemenu* onemenu::init()
{;
 menunow=false; ddmnow=false; menunum=1;
 return this;
}

void onemenu::opt(string n, char tr, string key, boolean val)
{
    integer l;
;
 number += 1;
 l=length(n+key)+3; if (width<l)  width=l;
 {
 optiontype& with = oo[number]; 
 ;
  with.title=n;
  with.trigger=tr;
  with.shortcut=key;
  with.valid=val;
 }
}

void onemenu::displayopt(byte y, boolean highlit)
{
 string data;
;
 {
 optiontype& with = oo[y]; 
 ;

  if (highlit) 
   setfillstyle(1,0);
   else
    setfillstyle(1,7);
  bar((flx1+1)*8,3+y*10,(flx2+1)*8,12+y*10);

/*  settextjustify(2,2);
   if shortcut>'' then outtextxy(flx2,4+y*10,shortcut);*/

  data=with.title;

  while (length(data+with.shortcut)<width) 
   data=data+' ';  /* Pad with spaces. */

  data=data+with.shortcut;

  if (highlit) 
   hlchalk(left,4+y*10,with.trigger,data,with.valid);
  else
   chalk(left,4+y*10,with.trigger,data,with.valid);

 }
}

void onemenu::display()
{
    byte y;
;
 off();
 setactivepage(cp); setvisualpage(cp);
 setfillstyle(1,menu_b); setcolor(menu_border);
 firstlix=true;
 flx1=left-2; flx2=left+width; fly=14+number*10;
 menunow=true; ddmnow=true;

 bar((flx1+1)*8,12,(flx2+1)*8,fly);
 rectangle((flx1+1)*8-1,11,(flx2+1)*8+1,fly+1);

 displayopt(1,true);
 for( y=2; y <= number; y ++)
  { optiontype& with = oo[y];  displayopt(y,false);}
 defaultled=1; cmp=177; mousepage(cp); on(); /* 4= fletch */
}

void onemenu::wipe()
{
    bytefield r;
;
 setactivepage(cp);
 off();
 { headtype& with = ddm_m.ddms[ddm_o.menunum]; 
  chalk(with.xpos,1,with.trigger,with.title,true);}
/* mblit((flx1-3) div 8,11,((flx2+1) div 8)+1,fly+1,3,cp);*/

/* with r do
 begin;
  x1:=flx1;
  y1:=11;
  x2:=flx2+1;
  y2:=fly+1;
 end;
 getset[cp].remember(r);*/

 mblit(flx1,11,flx2+1,fly+1,3,cp); blitfix();
 menunow=false; ddmnow=false; firstlix=false; defaultled=2;
 on_virtual();
}

void onemenu::movehighlight(shortint add)
{
    shortint hn;
;
 if (add!=0) 
 {;
  hn=highlightnum+add;
  if ((hn<0) || (hn>=(unsigned char)number))  return;
  highlightnum=hn;
 }
 setactivepage(cp); off();
  displayopt(oldy+1,false);
  displayopt(highlightnum+1,true);
 setactivepage(1-cp);
 oldy=highlightnum; on();
}

void onemenu::lightup()    /* This makes the menu highlight follow the mouse.*/
{;
 if ((mx<(cardinal)flx1*8) || (mx>(cardinal)flx2*8) || (my<=12) || (my>(cardinal)fly-3))  return;
 highlightnum=(my-13) / 10;
 if (highlightnum==oldy)  return;
 movehighlight(0);
}

void onemenu::select(byte n)      /* Choose which one you want. */
{;
 if (! oo[n+1].valid)  return;
 choicenum=n; wipe();

 if (choicenum==number)  choicenum -= 1;  /* Off the bottom. */
 if (choicenum>number)  choicenum=0;    /* Off the top, I suppose. */

 ddm_m.ddms[menunum].do_choose();
}

void onemenu::keystroke(char c)
{
    byte fv; boolean found;
;
 c=upcase(c); found=false;
 for( fv=1; fv <= number; fv ++)
  { optiontype& with = oo[fv]; 
   if ((upcase(with.trigger)==c) && with.valid) 
   {;
    select(fv-1);
    found=true;
   }}
 if (! found)  blip();
}

  headtype* headtype::init
   (char trig,char alttrig, string name, byte p, proc dw,proc dc)
  {;
   trigger=trig; alttrigger=alttrig; title=name;
   position=p; xpos=(position-1)*spacing+indent;
   xright=position*spacing+indent;
   do_setup=dw; do_choose=dc;
   return this;
  }

  void headtype::display()
  {;
   off(); /*MT*/
   chalk(xpos,1,trigger,title,true);
   on(); /*MT*/
  }

  void headtype::highlight()
  {;
   off(); off_virtual(); nosound;
   setactivepage(cp);
    hlchalk(xpos,1,trigger,title,true);
   {;
    ddm_o.left=xpos;
    ddm_o.menunow=true; ddmnow=true; ddm_o.menunum=position;
   }
   cmp=177;  /* Force redraw of cursor. */
  }

  boolean headtype::extdparse(char c)
  {boolean extdparse_result;
  ;
   if (c!=alttrigger)  {; extdparse_result=true; return extdparse_result; }
   extdparse_result=false;
  return extdparse_result;
  }

  menuset* menuset::init()
  {;
   howmany=0;
   return this;
  }

  void menuset::create(char t, string n, char alttrig, proc dw,proc dc)
  {;
   howmany += 1;
   ddms[howmany].init(t,alttrig,n,howmany,dw,dc);
  }

  void menuset::update()
  {
   const bytefield menuspace = {0, 0, 80, 9};
   byte fv,page_,savecp;
  ;
   setactivepage(3);
   setfillstyle(1,menu_b); bar(0, 0,640, 9);
   savecp=cp; cp=3;

   for( fv=1; fv <= howmany; fv ++)
    ddms[fv].display();

   for( page_=0; page_ <= 1; page_ ++)
    getset[page_].remember(menuspace);

   cp=savecp;
  }

  void menuset::extd(char c)
  {
      byte fv;
  ;
   fv=1;
   while ((fv<=howmany) && (ddms[fv].extdparse(c)))  fv += 1;
   if (fv>howmany)  return; getcertain(fv);
  }

  void menuset::getcertain(byte fv)
  {;
   { headtype& with = ddms[fv]; 
    {;
     if (ddm_o.menunow) 
     {;
      wipe(); /* get rid of menu */
      if (ddm_o.menunum==with.position)  return; /* clicked on own highlight */
     }
     highlight(); do_setup();
    }}
  }

  void menuset::getmenu(integer x)
  {
      byte fv;
  ;
   fv=0;
   do {
    fv += 1;
    if ((x>ddms[fv].xpos*8) && (x<ddms[fv].xright*8)) 
    {;
     getcertain(fv);
     return;
    }
   } while (!(fv>howmany));
  }

void parsekey(char r,char re)
{;
  switch (r) {
   case '\0':case '\340': {;
        switch (re) {
         case 'K': if (ddm_o.menunum>1)  {;
               wipe();
               ddm_m.getcertain(ddm_o.menunum-1);
              } else
              {;     /* Get menu on the left-hand side */
               wipe();
               ddm_m.getmenu((ddm_m.howmany-1)*spacing+indent);
              }
              break;
         case 'M': if (ddm_o.menunum<ddm_m.howmany)  {;
               wipe();
               ddm_m.getcertain(ddm_o.menunum+1);
              } else
              {;     /* Get menu on the far right-hand side */
               wipe();
               ddm_m.getmenu(indent);
              }
              break;
         case 'H': movehighlight(-1); break;
         case 'P': movehighlight(1); break;
         default: ddm_m.extd(re);
         }
        }
        break;
   case '\15': select(ddm_o.highlightnum); break;
   default:
   {;
    if (ddm_o.menunow)  keystroke(r);
   }
  }
}

/*$F+  *** Here follow all the ddm__ and do__ procedures for the DDM system. */

void ddm__game()
{;
 {;
  start_afresh();
  opt("Help...",'H',"f1",true);
  opt("Boss Key",'B',"alt-B",true);
  opt("Untrash screen",'U',"ctrl-f7",true);
  opt("Score and rank",'S',"f9",true);
  opt("About Avvy...",'A',"shift-f10",true);
  display();
 }
}

void ddm__file()
{;
 {;
  start_afresh();
  opt("New game",'N',"f4",true);
  opt("Load...",'L',"^f3",true);
  opt("Save",'S',"^f2",alive);
  opt("Save As...",'v',"",alive);
  opt("DOS Shell",'D',atkey+'1',true);
  opt("Quit",'Q',"alt-X",true);
  display();
 }
}

void ddm__action()
{
    string n;
;
 n=copy(f5_does(),2,255);

 {;
  start_afresh();
  if (n=="") 
   opt("Do something",'D',"f5",false);
  else
   opt(copy(n,2,255),n[1],"f5",true);
  opt("Pause game",'P',"f6",true);
  if (dna.room==99) 
   opt("Journey thither",'J',"f7",neardoor());
  else
   opt("Open the door",'O',"f7",neardoor());
  opt("Look around",'L',"f8",true);
  opt("Inventory",'I',"Tab",true);
  if (tr[1].xs==walk) 
    opt("Run fast",'R',"^R",true);
  else
    opt("Walk slowly",'W',"^W",true);
  display();
 }
}

void ddm__people()
{
    byte here; char fv;
;

 people="";
 here=dna.room;

 {;
  start_afresh();
  for( fv='\226'; fv <= '\262'; fv ++)
   if (whereis[fv]==here) 
   {;
    opt(getname(fv),getnamechar(fv),"",true);
    people=people+fv;
   }
  display();
 }
}

void ddm__objects()
{
    char fv;
;
 {;
  start_afresh();
  for( fv='\1'; fv <= numobjs; fv ++)
   if (dna.obj[fv]) 
    opt(get_thing(fv),get_thingchar(fv),"",true);
  display();
 }
}

string himher(char x)           /* Returns "im" for boys, and "er" for girls.*/
{string himher_result;
;
 if (x<'\257')  himher_result="im"; else himher_result="er";
return himher_result;
}

void ddm__with()
{
    byte fv; varying_string<7> verb; char vbchar; boolean n;
;
 {;
  start_afresh();

  if (thinkthing) 
  {;

   find_what_you_can_do_with_it();

   for( fv=1; fv <= length(verbstr); fv ++)
   {;
    verbopt(verbstr[fv],verb,vbchar);
    opt(verb,vbchar,"",true);
   }

   /* We disable the "give" option if: (a), you haven't selected anybody,
      (b), the person you've selected isn't in the room,
      or (c), the person you've selected is YOU! */

   if ((set::of(nowt,pavalot, eos).has(last_person)) ||
    (whereis[last_person]!=dna.room)) 
    opt("Give to...",'G',"",false); /* Not here. */ else
    {;
     opt(string("Give to ")+getname(last_person),'G',"",true);
     verbstr=verbstr+vb_give;
    }

  } else
  {;
   opt("Examine",'x',"",true);
   opt(string("Talk to h")+himher(thinks),'T',"",true);
   verbstr=string(vb_exam)+vb_talk;
   switch (thinks) {

    case pgeida:case parkata:
     {;
      opt("Kiss her",'K',"",true);
      verbstr=verbstr+vb_kiss;
     }
     break;

    case pdogfood:
     {;
      opt("Play his game",'P',"",! dna.wonnim);   /* True if you HAVEN'T won. */
      verbstr=verbstr+vb_play;
     }
     break;

    case pmalagauche:
     {;
      n=! dna.teetotal;
      opt("Buy some wine",'w',"",! dna.obj[wine]);
      opt("Buy some beer",'b',"",n);
      opt("Buy some whisky",'h',"",n); opt("Buy some cider",'c',"",n);
      opt("Buy some mead",'m',"",n);
      verbstr=verbstr+'\145'+'\144'+'\146'+'\147'+'\150';
     }
     break;

    case ptrader:
     {;
      opt("Buy an onion",'o',"",! dna.obj[onion]);
      verbstr=verbstr+'\151';
     }
     break;

   }
  }
  display();
 }
}

/*procedure ddm__map;
begin;
 with ddm_o do
 begin;
  start_afresh;
  opt('Cancel map','G','f5',true);
  opt('Pause game','P','f6',true);
  opt('Journey thither','J','f7',neardoor);
  opt('Explanation','L','f8',true);
  display;
 end;
end;

procedure ddm__town;
begin;
 with ddm_o do
 begin;
  start_afresh;
  opt('Argent','A','',true);
  opt('Birmingham','B','',true);
  opt('Nottingham','N','',true);
  opt('Cardiff','C','',true);
  display;
 end;
end;*/

void do__game()
{;
 switch (ddm_o.choicenum) {
  /* Help, boss, untrash screen. */
   case 0: callverb(vb_help); break;
   case 1: callverb(vb_boss); break;
   case 2: major_redraw(); break;
   case 3: callverb(vb_score); break;
   case 4: callverb(vb_info); break;
 }
}

void do__file()
{;
 switch (ddm_o.choicenum) {
  /* New game, load, save, save as, DOS shell, about, quit. */
   case 0: callverb(vb_restart); break;
   case 1: {; realwords[2]=""; callverb(vb_load); } break;
   case 2: {; realwords[2]=""; callverb(vb_save); } break;
   case 3: filename_edit(); break;
   case 4: back_to_bootstrap(2); break;
   case 5: callverb(vb_quit); break;
 }
}

void do__action()
{
    string n;
;
 switch (ddm_o.choicenum) {
  /* Get up/pause game/open door/look/inv/walk-run */
  case 0: {
        person=pardon; thing=pardon;
        n=f5_does(); callverb(n[1]);
     }
     break;
  case 1: callverb(vb_pause); break;
  case 2: callverb(vb_open); break;
  case 3: callverb(vb_look); break;
  case 4: callverb(vb_inv); break;
  case 5: {
       if (tr[1].xs==walk)  tr[1].xs=run;
                        else tr[1].xs=walk;
       newspeed();
     }
     break;
 }
}

void do__objects()
{;
 thinkabout(objlist[ddm_o.choicenum+1],a_thing);
}

void do__people()
{;
 thinkabout(people[ddm_o.choicenum+1],a_person);
 last_person=people[ddm_o.choicenum+1];
}

void do__with()
{;
 thing=thinks;

 if (thinkthing) 
 {;

  thing += 49;

  if (verbstr[ddm_o.choicenum+1]==vb_give) 
   person=last_person;
  else
   person='\376';

 } else
 {;
  switch (verbstr[ddm_o.choicenum+1]) {
   case '\144': {; thing='\144'; callverb(vb_buy); return; } break; /* Beer */
   case '\145': {; thing= '\62'; callverb(vb_buy); return; } break; /* Wine */
   case '\146': {; thing='\146'; callverb(vb_buy); return; } break; /* Whisky */
   case '\147': {; thing='\147'; callverb(vb_buy); return; } break; /* Cider */
   case '\150': {; thing='\153'; callverb(vb_buy); return; } break; /* Mead */
   case '\151': {; thing= '\103'; callverb(vb_buy); return; } break; /* Onion (trader) */
   default:
   {;
    person=thing;
    thing='\376';
   }
  }
 }
 callverb(verbstr[ddm_o.choicenum+1]);
}

/*$F- That's all. Now for the ...bar procs. */

void standard_bar()     /* Standard menu bar */
{;
 ddm_m.init(); ddm_o.init();
 {;     /* Set up menus */
  create('F',"File",'!',ddm__file,do__file); /* same ones in map_bar, below, */
  create('G',"Game",'\42',ddm__game,do__game); /* Don't forget to change the */
  create('A',"Action",'\36',ddm__action,do__action); /* if you change them */
  create('O',"Objects",'\30',ddm__objects,do__objects); /* here... */
  create('P',"People",'\31',ddm__people,do__people);
  create('W',"With",'\21',ddm__with,do__with);
  update();
 }
}

/*procedure map_bar; { Special menu bar for the map (screen 99) }
begin;
 ddm_m.init; ddm_o.init;
 with ddm_m do
 begin; { Set up menus }
  create('G','Game','#',ddm__game,do__game);
  create('F','File','!',ddm__file,do__test);
  create('M','Map','2',ddm__map,do__test);
  create('T','Town',#20,ddm__town,do__test);
  update;
 end;
end;*/

void checkclick()     /* only for when the menu's displayed */
{;
 if (mpress>0) 
 {;
  if (mpy>10) 
    {;
     if (! ((ddm_o.firstlix) &&
      ((mpx>=(cardinal)ddm_o.flx1*8) && (mpx<=(cardinal)ddm_o.flx2*8) &&
      (mpy>=12) && (mpy<=(cardinal)ddm_o.fly)))) 
     {;     /* Clicked OUTSIDE the menu. */
      if (ddm_o.menunow)  wipe();
     }    /* No "else"- clicking on menu has no effect (only releasing) */
    } else
   {;     /* Clicked on menu bar */
    ddm_m.getmenu(mpx);
   }
 } else
 {;     /* NOT clicked button... */
  if (mrelease>0) 
  {;
    if ((ddm_o.firstlix) &&
     ((mrx>=(cardinal)ddm_o.flx1*8) && (mrx<=(cardinal)ddm_o.flx2*8) &&
     (mry>=12) && (mry<=(cardinal)ddm_o.fly))) 
      select((mry-13) / 10);
  }
 }
}

void menu_link()
{;
 {;
  if (! ddm_o.menunow)  return;

  check(); /* find mouse coords & click information */
  checkclick(); /* work out click codes */

  /* Change arrow... */

  switch (my) {
     case RANGE_11(0, 10): newpointer(1);
     break;                 /* up-arrow */
    case 11 ... 169: {;
              if ((mx>=(cardinal)ddm_o.flx1*8) && (mx<=(cardinal)ddm_o.flx2*8) && (my>10) && (my<=(cardinal)ddm_o.fly)) 
               newpointer(3); /* right-arrow */
              else newpointer(4); /* fletch */
             }
             break;
   case RANGE_32(169,200): newpointer(2);
   break;                   /* screwdriver */
  }

  if (! ddm_o.menunow)  return;

  lightup();
 }
}


