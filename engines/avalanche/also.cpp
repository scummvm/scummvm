#include "ptoc.h"


#include "graph.h"
/*#include "Rodent.h"*/
/*#include "Crt.h"*/

/*$R+,V-*/

typedef matrix<0,255,0,15,byte> fonttype;

class fieldtype {
public:
             integer x1,y1,x2,y2;
};

class linetype : public fieldtype {
public:
            byte col;
};

struct pedtype {
           integer x,y; byte dir;
};

struct magictype {
             byte op; /* one of the operations */
             word data; /* data for them */
};

const integer numlines = 50; const integer up = 0; const integer right = 1; const integer down = 2; const integer left = 3; const integer ur = 4; const integer dr = 5; const integer dl = 6; const integer ul = 7;
const integer still = 8;

const integer nay = maxint;

/* Magic commands are */

/*N*/ const integer nix = 0; /* ignore it if this line is touched */
/*B*/ const integer bounce = 1; /* bounce off this line */
/*E*/ const integer exclaim = 2; /* put up a chain of scrolls */
/*T*/ const integer transport = 3; /* enter new room */
/*U*/ const integer unfinished = 4; /* unfinished connection */
/*S*/ const integer special = 5; /* special call */
/*O*/ const integer opendoor = 6; /* slowly opening door. */

integer gd,gm;
array<1,numlines,linetype> lines;
array<1,numlines,fieldtype> fields;
boolean do1;
byte current;
string n;
matrix<0,29,1,2,string> names;
file<fonttype> f;
fonttype skinny;
byte tx,ty;
matrix<0,79,0,22,char> chars;
byte cursorflash;
array<1,15,pedtype> peds;
array<1,15,magictype> magics;
array<9,15,magictype> portals;
varying_string<26> flags;
string listen;

const graphcursmasktype crosshairs = 
{ 
    ((63551,63807,63807,63807,61727,257,897,32765,897,257,61727,63807,63807,63807,63551,65535),
     (4368,21140,8840,53910,640,640,31868,33026,31868,640,640,53910,8840,21140,4368,0)),
               7,
               7};

const graphcursmasktype hook = 
{ 
    ((32831,32831,49279,49279,57599,61695,61471,61447,63491,57089,36801,32771,49159,57375,63743,65535),
     (0,16256,7936,7936,3584,1536,1792,2016,248,28,8220,12344,8160,1792,0,0)),
               2,
               9};

const graphcursmasktype tthand = 
{ 
    ((62463,57855,57855,57855,57471,49167,32769,0,0,0,0,32768,49152,57344,61441,61443),
     (3072,4608,4608,4608,4992,12912,21070,36937,36873,36865,32769,16385,8193,4097,2050,4092)),
               4,
               0};

string strf(longint x)
{
    string q;
string strf_result;
;
 str(x,q); strf_result=q;
return strf_result;
}

void glimpse(byte ret)       /* glimpse of screen 3 */
{
    char sink;
;
 hidemousecursor; setvisualpage(3); setcrtpagenumber(3); showmousecursor;
 do {; } while (!(~ anymousekeypressed));
 do {; } while (!anymousekeypressed);
 hidemousecursor; setvisualpage(ret); setcrtpagenumber(ret); showmousecursor;
 while (keypressed())  sink=readkey();
}

void newline(byte t, integer p,integer q,integer r,integer s, byte c)
{;
 {
 linetype& with = lines[t]; 
 ;
  x1=p; y1=q; x2=r; y2=s; with.col=c;
 }
}

void newfield(byte t, integer p,integer q,integer r,integer s)
{; {
                         fieldtype& with = fields[t];  ; with.x1=p; with.y1=q; with.x2=r; with.y2=s; } }

void drawped(byte p)
{;
 { pedtype& with = peds[p]; 
  if (with.dir<177) 
  {;
   setcolor(p); circle(with.x,with.y,5); moveto(with.x,with.y);
   switch (with.dir) {
    case up:   linerel(0,-5); break;  case down:  linerel(0,5); break;
    case left: linerel(-7,0); break;  case right: linerel(7,0); break;
    case ul:   linerel(-7,-5); break; case dl:    linerel(-7, 5); break;
    case ur:   linerel( 7,-5); break; case dr:    linerel( 7, 5); break;
   }
  }}
}

void drawup()
{
    byte fv;
;
 cleardevice();
 for( fv=1; fv <= numlines; fv ++)
  { linetype& with = lines[fv]; 
   if (x1!=nay) 
   {;
    setcolor(with.col);
    line(x1,y1,x2,y2);
   }}
 for( fv=1; fv <= numlines; fv ++)
  { fieldtype& with = fields[fv]; 
   if (with.x1!=nay) 
   {;
    setcolor(fv);
    rectangle(with.x1,with.y1,with.x2,with.y2);
   }}
 for( fv=1; fv <= 15; fv ++) drawped(fv);
}

void addped()
{
    byte n,fv;
;
 n=0; do { n += 1; } while (!((n==16) || (peds[n].dir==177)));
 setcrtpagenumber(0); setactivepage(0); setvisualpage(0);
 drawup(); setgraphicscursor(tthand); showmousecursor;
 do {
  if (rightmousekeypressed)  return;
  if (keypressed())  glimpse(0);
 } while (!leftmousekeypressed);
 hidemousecursor;
 {
 pedtype& with = peds[n]; 
 ;
  with.x=mousex; with.y=mousey;
 }
 cleardevice(); setfillstyle(6,9); for( fv=1; fv <= 3; fv ++) bar(200*fv,0,200*fv,200);
 for( fv=1; fv <= 2; fv ++) bar(0,60*fv,640,60*fv);
 showmousecursor;
 do { if (rightmousekeypressed)  return; } while (!leftmousekeypressed);
 hidemousecursor;
 { pedtype& with = peds[n]; 
  switch (((mousex / 200)*10)+(mousey / 60)) {
   case 0: with.dir=ul; break;   case 10: with.dir=up; break;    case 20: with.dir=ur; break;
   case 1: with.dir=left; break; case 11: with.dir=still; break; case 21: with.dir=right; break;
   case 2: with.dir=dl; break;   case 12: with.dir=down; break;  case 22: with.dir=dr; break;
  }}
}

void addline(byte ccc)
{
    byte fv;
;
 do {
  for( fv=1; fv <= numlines; fv ++)
   { linetype& with = lines[fv]; 
    if (x1==nay) 
    {;
     x1=fv*17; x2=x1; y1=200; y2=190; with.col=ccc;
     return; /* bad style! */
    }}
 } while (!false);
}

byte colour()
{
    byte fv;
byte colour_result;
;
 setactivepage(0); setvisualpage(0); setcrtpagenumber(0);
 outtextxy(0,0,"Select a colour, please...");
 for( fv=1; fv <= 15; fv ++)
 {;
  setfillstyle(1,fv);
  bar(fv*40,27,39+fv*40,200);
 }
 showmousecursor;
 do {
  if (rightmousekeypressed)  {; hidemousecursor; return colour_result; }
  if (keypressed())  glimpse(2);
 } while (!leftmousekeypressed);
 hidemousecursor;
 colour_result=getpixel(mousex,mousey); cleardevice();
return colour_result;
}

void addfield()
{
    byte fv; boolean ok;
;
 do {
  fv=colour();
  ok=fields[fv].x1==nay;
  if (! ok)  output << '\7';
 } while (!ok);
 {
 fieldtype& with = fields[fv]; 
 ;
  with.x1=300+fv*17; with.x2=with.x1+1; with.y1=200; with.y2=177;
 }
}

byte checkline()
{
    byte fv,ans;
byte checkline_result;
;
 setgraphicscursor(crosshairs);
 setcrtpagenumber(0); setactivepage(0); setvisualpage(0); drawup();
 do {
  showmousecursor;
  do {
   if (rightmousekeypressed)  {; checkline_result=255; return checkline_result; }
   if (keypressed())  glimpse(0);
  } while (!leftmousekeypressed);
  hidemousecursor;
  setactivepage(1); ans=177;
  for( fv=1; fv <= numlines; fv ++) /*  */
  {;
   { linetype& with = lines[fv]; 
    if (x1!=nay) 
    {;
     setcolor( 9); line(x1,y1,x2,y2);
     if (getpixel(mousex,mousey)==9)  ans=fv;
     setcolor( 0); line(x1,y1,x2,y2);
    }}
   { fieldtype& with = fields[fv]; 
    if (with.x1!=nay) 
    {;
     setcolor( 9); rectangle(with.x1,with.y1,with.x2,with.y2);
     if (getpixel(mousex,mousey)==9)  ans=fv+100;
     setcolor( 0); rectangle(with.x1,with.y1,with.x2,with.y2);
    }}
  }
  setactivepage(0);
 } while (!(ans!=177));
 checkline_result=ans;
return checkline_result;
}

void chooseside();
static boolean itsaline; 

static void plotline()
{;
 if (itsaline) 
  { linetype& with = lines[gd]; 
   if (do1)  line(mousex,mousey,x2,y2); else
    line(x1,y1,mousex,mousey);}
 else
  { fieldtype& with = fields[gd]; 
   if (do1)  rectangle(mousex,mousey,with.x2,with.y2); else
    rectangle(with.x1,with.y1,mousex,mousey);}
}

void chooseside()
{
    byte clicol,savelcol;fieldtype current; integer temp;
;
 do {
  gd=checkline(); itsaline=gd<100;
  if (gd==255)  {; hidemousecursor; return; }
  if (! itsaline)  gd -= 100;
  setactivepage(2); setvisualpage(2); cleardevice();
  setgraphicscursor(tthand); setcrtpagenumber(2);
  if (itsaline) 
  {;
   current=lines[gd];
   savelcol=lines[gd].col;
  } else current=fields[gd];
  {;
   setcolor(9);
   if (itsaline)  line(current.x1,current.y1,current.x2,current.y2); else rectangle(current.x1,current.y1,current.x2,current.y2);
    setcolor(9);
    setfillstyle(1,red);   bar(current.x1-3,current.y1-3,current.x1+3,current.y1+3);
    setfillstyle(1,green); bar(current.x2-3,current.y2-3,current.x2+3,current.y2+3);
   do {; } while (!(~ anymousekeypressed));
   clicol=177; showmousecursor;
  do {
   if (anymousekeypressed) 
   {;
    hidemousecursor;
    clicol=getpixel(mousex,mousey);
    showmousecursor;
   }
   if (rightmousekeypressed) 
    {; hidemousecursor; return; }
   if (keypressed())  glimpse(2);
  } while (!(set::of(red,green, eos).has(clicol)));
  do1=clicol==red; hidemousecursor;
  setgraphicscursor(hook); setcrtpagenumber(0);
  setactivepage(0); setvisualpage(0); setcolor(0);
  if (itsaline) 
  {
                     linetype& with1 = lines[gd];  ; line(current.x1,current.y1,current.x2,current.y2); setcolor(with1.col); } else
  {
                     fieldtype& with1 = fields[gd];  ; rectangle(with1.x1,with1.y1,with1.x2,with1.y2); setcolor(gd); }
  setwritemode(xorput);
  while (~ anymousekeypressed) 
  {;
   plotline();
   showmousecursor; delay(1); hidemousecursor;
   plotline();
   if (rightmousekeypressed)  {; hidemousecursor; return; }
   if (keypressed())  glimpse(0);
  }
  /* update "current" rec */
  if (do1)  {; current.x1=mousex; current.y1=mousey; }
   else {; current.x2=mousex; current.y2=mousey; }
  if (! itsaline) 
  {;
   if (current.x1>current.x2)  {; temp=current.x2; current.x2=current.x1; current.x1=temp; }
   if (current.y1>current.y2)  {; temp=current.y2; current.y2=current.y1; current.y1=temp; }
  }
  /* copy "current" to line/field */
  if (itsaline) 
   {
   linetype& with1 = lines[gd]; 
   ;
    current.x1=current.x1; current.x2=current.x2; current.y1=current.y1; current.y2=current.y2;
    with1.col=savelcol;
   } else fields[gd]=current;
 }
 setwritemode(0);
 } while (!false);
}

void delped()
{;
 setcrtpagenumber(0); setactivepage(0); setvisualpage(0);
 drawup(); setgraphicscursor(tthand); showmousecursor;
 do {; } while (!leftmousekeypressed); peds[colour()].dir=177;
}

byte menu();

static void say(byte y, string x)
{;
 setfillstyle(1,y);
 bar(0,y*17,100,y*17+15); outtextxy(123,y*17,x);
}

byte menu()
{
    byte clicol;
byte menu_result;
;
  setcolor(15); settextstyle(0,0,2); clicol=0; setgraphicscursor(tthand);
  setvisualpage(2); setactivepage(2); setcrtpagenumber(2); cleardevice();
  say(3,"Move lines around");
  say(4,"Add a new line");
  say(5,"Delete a line");
  say(6,"Add a ped");
  say(7,"Delete a ped");
  say(8,"Add a field");
  say(10,"Return to Also.");
  showmousecursor;
  do {
   if (leftmousekeypressed) 
   {;
    hidemousecursor;
    clicol=getpixel(mousex,mousey);
    showmousecursor;
   }
   if (rightmousekeypressed)  {; hidemousecursor; return menu_result; }
   if (keypressed())  glimpse(2);
  } while (!(clicol>0));
  do {; } while (!(~ anymousekeypressed));
  hidemousecursor;
  menu_result=clicol;
return menu_result;
}

void removeline()
{;
 gd=checkline(); if (gd==255)  {; hidemousecursor; return; }
 if (gd>100) 
  fields[gd-100].x1=nay;
 else lines[gd].x1=nay;  /* cancels it out */
 cleardevice(); drawup();
}

void lino()
{;
 resetmouse;
 do {
  switch (menu()) {
   case 3: chooseside(); break;
   case 4: addline(colour()); break;
   case 5: removeline(); break;
   case 6: addped(); break;
   case 7: delped(); break;
   case 8: addfield(); break;
  case 10: return; break;
  }
 } while (!false);
}

void loadscreen()     /* load2 */
{
 byte a /*absolute $A000:246560*/;
 byte bit;
 untyped_file f;
;
 setactivepage(3); setvisualpage(3);
 assign(f,string("c:\\avalot\\place")+n+".avd"); reset(f,1); seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a,12080);
 }
 close(f);
 setvisualpage(0);
 outtextxy(0,190,"Look carefully, and then press any key...");
 setactivepage(0);
}

void ctrlsout(string& x)          /* Replace real ctrls with caret codes */
{
    byte fv; string xx;
;
 xx="";
 for( fv=1; fv <= length(x); fv ++)
  if (x[fv]>'\37')  xx=xx+x[fv]; else xx=xx+'^'+chr(ord(x[fv])+64);
 x=xx;
}

void ctrlsin(string& x)          /* Opposite of ctrlsout */
{
    byte fv; string xx; boolean ctrlwas;
;
 xx=""; ctrlwas=false;
 for( fv=1; fv <= length(x); fv ++)
  if (ctrlwas)    /* last char was a caret */
   {;
    xx=xx+chr(ord(upcase(x[fv]))-64);
    ctrlwas=false;
   }
  else
  {;     /* last char wasn't a caret... */
   if (x[fv]=='^')  ctrlwas=true; else  /* ...but this one is */
    xx=xx+x[fv];  /* ...but this one isn't */
  }
 x=xx;
}

void flipover()     /* temp view other screen */
{
    char r;
;
 setvisualpage(3); r=readkey(); setvisualpage(0);
}

void plotchar(byte x,byte y, char n)
{
    byte fv;
;
 if (chars[x][y]==n)  return;
 for( fv=0; fv <= 15; fv ++)
  mem[0xa000*y*1200+(fv+3)*80+x]=skinny[ord(n)][fv];
 chars[x][y]=n;
}

void cursor()
{
    byte fv;
;
 cursorflash += 1;
 switch (cursorflash) {
  case 1:case 127: for( fv=12; fv <= 14; fv ++)
          mem[0xa000*ty*1200+(3+fv)*80+tx]=~(mem[0xa000*ty*1200+(3+fv)*80+tx]);
          break;
  case 255: cursorflash=0; break;
 }
}

void losecursor()
{;
 if (cursorflash<127)  {; cursorflash=126; cursor(); }
 cursorflash=0;
}

void gwrite(string x)
{
    byte fv;
;
 for( fv=1; fv <= length(x); fv ++)
 {;
  plotchar(tx,ty,x[fv]);
  tx += 1;
  if (tx==80)  {; ty += 1; tx=0; }
 }
}

string typein(string x)
{
      const char marker = '\2';
    byte p; char r;
string typein_result;
;
 setvisualpage(0); setactivepage(0); cleardevice();
 settextstyle(0,0,1); setcolor(15);
 outtextxy( 0,  0,"Press TAB to see the room...");
 outtextxy( 0, 20,"You may use any of these Control Codes:");
 outtextxy(30, 30,"Anywhere: ^M = new line, ^P = new scroll, |1 fix to speaker 1.");
 outtextxy(90, 40,"^B = new bubble");
 outtextxy(30, 50,"At end of line: ^C = centre line, ^L = left justify.");
 outtextxy(30, 60,"At end of scroll: ^D = Don't add automatic ^P here.");
 outtextxy( 0, 80,"(Use by typing in (eg for ^P) ^ then P, not Ctrl-P.)");
 p=0; ctrlsout(x); fillchar(chars,sizeof(chars),'\40');
 do {
  tx=0; ty=6; gwrite(x+'\4'+'\40');
  tx=(p % 80); ty=(p / 80)+6;
  while (! keypressed())  {; delay(1); cursor(); } losecursor();
  r=readkey();
  switch (r) {
   case '\10': if (p>0)  {; x=copy(x,1,p-1)+copy(x,p+1,255); p -= 1; } break; /* backspace */
   case '\11': flipover(); break;
   case '\40' ... '\377': {; x=copy(x,1,p)+r+copy(x,p+1,255); p += 1; } break;
   case '\0': switch (readkey()) { /* extd. keystroke */
        case 'G': p=0; break; /* Home */
        case 'K': if (p>0)  p -= 1; break; /* left */
        case 'M': if (p<length(x))  p += 1; break; /* right */
        case 'H': if (p>80)  p -= 80; break; /* up */
        case 'P': if (p<length(x)-80)  p += 80; break; /* down */
        case 'O': p=length(x); break; /* End */
        case 'S': x=copy(x,1,p)+copy(x,p+2,255); break; /* Del */
       }
       break;
  }
 } while (!(r=='\15'));
 ctrlsin(x); typein_result=x;
return typein_result;
}

byte typeno(string title)
{
 varying_string<2> x; char r; integer e; word p;
byte typeno_result;
;
 cleardevice(); x="000";
 settextstyle(0,0,3); setcolor(9); outtextxy(0,0,title);
 setfillstyle(1,0); setcolor(10); fillchar(chars,sizeof(chars),'\40');
 do {
  bar(100,100,150,125);
  outtextxy(100,100,x);
  do { r=readkey(); } while (!(set::of(range('0','9'),'\33','\15', eos).has(r)));
  if (r=='\33')  {; typeno_result=255; return typeno_result; }
  if (r!='\15')  x=string(x[2])+r;
 } while (!(r=='\15'));
 val(x,p,e); typeno_result=p;
return typeno_result;
}

void showallnames()
{
    byte fv; varying_string<2> s; char r;
;
 settextstyle(0,0,2); cleardevice(); setcolor(13); outtextxy(0,0,"Descriptions start...");
 settextstyle(0,0,1); setcolor(7);
 for( fv=1; fv <= 29; fv ++)
 {;
  str(fv,2,s);
  outtextxy((fv / 15)*320,((fv % 15)*10)+30,s+'='+copy(names[fv][1],0,33));
 }
 setcolor(15); outtextxy(500,190,"Press any key...");
 r=readkey();
}

void showallassoc();


static void saascreen()
{;
 settextstyle(0,0,2); cleardevice(); setcolor(10); outtextxy(0,0,"Everything...");
 settextstyle(0,0,1); setcolor(2);
 outtextxy(17,20,"(Format: <number> : <start of names> : <start of desc.>)");
}

void showallassoc()
{
    byte fv; varying_string<2> s; char r;

;
 saascreen();
 for( fv=1; fv <= 30; fv ++)
 {;
  str(fv-1,2,s);
  outtextxy(0,(((fv-1) % 10)*10)+30,
   s+':'+copy(names[fv-1][1],1,7)+':'+copy(names[fv-1][2],1,70));
  if ((fv % 10)==0)  {; r=readkey(); saascreen(); }
 }
 setcolor(15); outtextxy(500,190,"Press any key...");
 r=readkey();
}

void clear()
{
    byte fv;
;
 fillchar(names ,sizeof(names ),  '\0');
 for( fv=1; fv <= numlines; fv ++) {; lines[fv].x1=nay; fields[fv].x1=nay; }
 fillchar(peds  ,sizeof(peds  ),'\261');
}

void scramble();

static void scram1(string& x)
{
    byte fz;
;
 for( fz=1; fz <= length(x); fz ++)
  x[fz]=chr(ord(x[fz]) ^ 177);
}

void scramble()     /* Works both ways. */
{
    byte fv,ff;
;
 for( fv=0; fv <= 29; fv ++)
  for( ff=1; ff <= 2; ff ++)
   scram1(names[fv][ff]);
 scram1(listen);
 scram1(flags);
}

void save()
{
    string x; untyped_file f; byte minnames,minlines,minpeds,minfields,fv,ff;
;
 minnames =0; for( fv=0; fv <= 29; fv ++) if (names[fv][1]!="")     minnames =fv;
 minlines =0; for( fv=1; fv <= numlines; fv ++)
                if (lines[fv].x1!=nay)   minlines =fv;
 minpeds  =0; for( fv=1; fv <= 15; fv ++) if (peds[fv].dir<177)    minpeds  =fv;
 minfields=0; for( fv=1; fv <= 30; fv ++) if (fields[fv].x1!=nay)  minfields=fv;
 assign(f,string("c:\\avalot\\also")+n+".avd");
 rewrite(f,1);
 x=string("This is an Also .AVD file, which belongs to AVALOT.EXE. Its contents")+
 '\15'+'\12'+"are subject to copyright, so there. Have fun!"+'\32'+" *Minstrel* ";
 blockwrite(f,x[1],128);
 scramble();
 blockwrite(f,minnames,1);
 for( fv=0; fv <= minnames; fv ++)
  for( ff=1; ff <= 2; ff ++)
   blockwrite(f,names[fv][ff],length(names[fv][ff])+1);
 blockwrite(f,minlines,1);
 blockwrite(f,lines,sizeof(lines[1])*minlines);
 blockwrite(f,minpeds,1);
 blockwrite(f,peds,sizeof(peds[1])*minpeds);
 blockwrite(f,minfields,1);
 blockwrite(f,fields,sizeof(fields[1])*minfields);
 blockwrite(f,magics,sizeof(magics));
 blockwrite(f,portals,sizeof(portals));
 blockwrite(f,flags,sizeof(flags));
 blockwrite(f,listen[0],1);
 blockwrite(f,listen[1],length(listen));
 close(f);
 scramble();
}

void load();
static untyped_file f; 


static string nextstring()
{
    byte l; string x;
string nextstring_result;
;
 x=""; blockread(f,l,1); blockread(f,x[1],l); x[0]=chr(l); nextstring_result=x;
return nextstring_result;
}

void load()
{
            byte minnames,minlines,minpeds,minfields; byte ff,fv;

;
 clear();
 assign(f,string("c:\\avalot\\also")+n+".avd");
/*$I-*/ reset(f,1); /*$I+*/ if (ioresult!=0)  return; /* no Also file */
 seek(f,128); blockread(f,minnames,1);
 for( fv=0; fv <= minnames; fv ++)
  for( ff=1; ff <= 2; ff ++)
   names[fv][ff]=nextstring();
 blockread(f,minlines,1);
 blockread(f,lines,sizeof(lines[1])*minlines);
 blockread(f,minpeds,1);
 blockread(f,peds,sizeof(peds[1])*minpeds);
 blockread(f,minfields,1);
 blockread(f,fields,sizeof(fields[1])*minfields);
 blockread(f,magics,sizeof(magics));
 blockread(f,portals,sizeof(portals));
 blockread(f,flags,sizeof(flags));
 blockread(f,listen[0],1);
 blockread(f,listen[1],length(listen));
 close(f);
 scramble();
}

void editmagics();
const array<1,15,char> codes = "123456789ABCDEF";

static integer y;


static void display()
{
    byte fv;
;
 cleardevice();
 settextstyle(0,0,2); setcolor(15); outtextxy(0,0,"Magics.");
 settextstyle(0,0,1);
 for( fv=1; fv <= 15; fv ++)
 {;
  y=23+fv*10;
  setcolor(fv); outtextxy(100,y,string('$')+codes[fv-1]);
  {
  magictype& with = magics[fv]; 
  ;
   switch (with.op) {
    case nix: {; setcolor(8); outtextxy(140,y,"Nix"); } break;
    case bounce: {; setcolor(10); outtextxy(143,y,"Bounce!"); } break;
    case exclaim: {;
              setcolor(14); outtextxy(143,y,string("Exclaim: ")+strf(with.data));
             }
             break;
    case transport: {;
                setcolor(12);
                outtextxy(143,y,string("Transport to ")+strf(hi(with.data))+
                 ", ped "+strf(lo(with.data)));
               }
               break;
    case unfinished: {;
                 setcolor(15); outtextxy(143,y,"*** UNFINISHED! ***");
                }
                break;
    case special: {;
              setcolor(6); outtextxy(143,y,string("Special call no. ")+strf(with.data));
             }
             break;
    case opendoor: {;
               setcolor(11);
               outtextxy(143,y,string("Opening door to ")+strf(hi(with.data))+
                ", ped "+strf(lo(with.data)));
              }
              break;
   }
  }
 }
 outtextxy(177,190,"Which do you want to change\? (Esc=Exit) $");
}



static word ask(string x)
{
    string q; word thomaswashere; integer e;
word ask_result;
;
 cleardevice();
 setcolor(10); settextstyle(0,0,3); outtextxy(0,100,x);
 do {
  input >> q >> NL; val(q,thomaswashere,e);
 } while (!(e==0)); ask_result=thomaswashere;
return ask_result;
}

void editmagics()
{
 char r,rr; byte p;

;
 do {
  display();
  do {
   r=upcase(readkey());
   if (r=='\33')  return;
   p=pos(r,codes);  /* which are we editing? */
  } while (!(p>0)); /* it must BE there... */
  setcolor(p); cleardevice();
  outtextxy(177,17,string("Editing magic $")+r+'.');
  outtextxy(0,30,"New operation ( (N)ix, (B)ounce, (E)xclaim, (T)ransport, (U)nfinished),");
  outtextxy(30,40,"(S)pecial, (O)pening Door?");
  do { rr=upcase(readkey()); } while (!(set::of('N','B','E','T','U','S','O','\33', eos).has(rr)));
  { magictype& with = magics[p]; 
   switch (rr) {
    case '\33': return; break; /* cancelling code */
    case 'N': with.op=nix; break;
    case 'B': with.op=bounce; break;
    case 'E': {; with.op=exclaim; with.data=ask("Which scroll?"); } break;
    case 'T': {; with.op=transport; with.data=ask("Ped no.?")+ask("Whither?")*256; } break;
    case 'U': with.op=unfinished; break;
    case 'S': {; with.op=special; with.data=ask("Which call?"); } break;
    case 'O': {; with.op=opendoor; with.data=ask("Ped no.?")+ask("Whither?")*256; } break;
   }}
 } while (!false);
}

void editportals();
const array<9,15,char> codes1 = "9ABCDEF";

static integer y2;


static void display1()
{
    byte fv;
;
 cleardevice();
 settextstyle(0,0,2); setcolor(15); outtextxy(0,0,"Portals.");
 settextstyle(0,0,1);
 for( fv=9; fv <= 15; fv ++)
 {;
  y2=fv*10-53;
  setcolor(fv); outtextxy(100,y2,string('$')+codes1[fv-1]);
  {
  magictype& with = portals[fv]; 
  ;
   switch (with.op) {
    case nix: {; setcolor(8); outtextxy(140,y2,"Nix"); } break;
    case exclaim: {;
              setcolor(14); outtextxy(143,y2,string("Exclaim: ")+strf(with.data));
             }
             break;
    case transport: {;
                setcolor(12);
                outtextxy(143,y2,string("Transport to ")+strf(hi(with.data))+
                 ", ped "+strf(lo(with.data)));
               }
               break;
    case unfinished: {;
                 setcolor(15); outtextxy(143,y2,"*** UNFINISHED! ***");
                }
                break;
    case special: {;
              setcolor(6); outtextxy(143,y2,string("Special call no. ")+strf(with.data));
             }
             break;
    case opendoor: {;
               setcolor(11);
               outtextxy(143,y2,string("Opening door to ")+strf(hi(with.data))+
                ", ped "+strf(lo(with.data)));
              }
              break;
   }
  }
 }
 outtextxy(177,190,"Which do you want to change\? (Esc=Exit) $");
}



static word ask1(string x)
{
    string q; word thomaswashere; integer e;
word ask1_result;
;
 cleardevice();
 setcolor(10); settextstyle(0,0,3); outtextxy(0,100,x);
 do {
  input >> q >> NL; val(q,thomaswashere,e);
 } while (!(e==0)); ask1_result=thomaswashere;
return ask1_result;
}

void editportals()     /* much t'same as editmagics */
{
 char r,rr; byte p;

;
 do {
  display1();
  do {
   r=upcase(readkey());
   if (r=='\33')  return;
   p=pos(r,codes1); /* which are we editing? */
  } while (!(p>0)); /* it must BE there... */
  p += 8; setcolor(p); cleardevice();
  outtextxy(177,17,string("Editing portal $")+r+'.');
  outtextxy(0,30,"New operation ( (N)ix, (E)xclaim, (T)ransport, (U)nfinished),");
  outtextxy(30,40,"(S)pecial, (O)pening Door?");
  do { rr=upcase(readkey()); } while (!(set::of('N','E','T','U','S','O','\33', eos).has(rr)));
  { magictype& with = portals[p]; 
   switch (rr) {
    case '\33': return; break; /* cancelling code */
    case 'N': with.op=nix; break;
    case 'E': {; with.op=exclaim; with.data=ask1("Which scroll?"); } break;
    case 'T': {; with.op=transport; with.data=ask1("Ped no.?")+ask1("Whither?")*256; } break;
    case 'U': with.op=unfinished; break;
    case 'S': {; with.op=special; with.data=ask1("Which call?"); } break;
    case 'O': {; with.op=opendoor; with.data=ask1("Ped no.?")+ask1("Whither?")*256; } break;
   }}
 } while (!false);
}

void editflags()
{
    char r;
;
 cleardevice();
 settextstyle(0,0,2); setcolor(15); outtextxy(0,0,"Flags.");
 settextstyle(0,0,1); setcolor(10);
 outtextxy(100,30,"Press the letter of the flag you want to toggle.");
 outtextxy(100,40,"Tab = flip screens, Esc/Enter = return to menu.");
 setcolor(14); setfillstyle(1,0);
 for( r='A'; r <= 'Z'; r ++)
  if (pos(r,flags)>0)  outtextxy(ord(r)*20-1223,77,r);
 do {
  do { r=upcase(readkey()); } while (!(set::of(range('A','Z'),'\33','\15','\11', eos).has(r)));
  switch (r) {
   case RANGE_26('A','Z'): {;
              if (pos(r,flags)>0) 
              {;     /* flag is on- switch it off */
               Delete(flags,pos(r,flags),1);
               bar(ord(r)*20-1223,77,ord(r)*20-1213,87);
               sound(1777); delay(7); nosound;
              } else
              {;     /* flag is off- switch it on */
               flags=flags+r;
               outtextxy(ord(r)*20-1223,77,r);
               sound(177); delay(7); nosound;
              }
             }
             break;
   case '\33':case '\15': return; break;
   case '\11': flipover(); break;
  }
 } while (!false);
}

void alsomenu()
{
    char r; byte t;
;
 do {
  setactivepage(0); setvisualpage(0);
  cleardevice(); setcolor(15); settextstyle(0,0,2);
  outtextxy(0,0,"Also... Main Menu");
  settextstyle(0,0,1); setcolor(10);
  outtextxy(100, 40,"1) Edit the names of an object");
  outtextxy(100, 50,"2) View all names");
  outtextxy(100, 60,"3) Edit the description of this object");
  outtextxy(100, 70,"4) View all associations.");
  outtextxy(100, 80,"5) Enter Lino mode.");
  outtextxy(100, 90,"6) Edit magics.");
  outtextxy(100,100,"7) Edit portals.");
  outtextxy(100,110,"8) Edit flags.");
  outtextxy(100,120,"9) Edit listen field.");
  outtextxy(100,160,"S) Save");
  outtextxy(100,170,"L) Load");
  outtextxy( 80,180,"Tab) View other screen");
  if (current==0)  outtextxy(0,140,"< Main description of room >"); else
   outtextxy(0,140,string('<')+names[current][1]+'>');
  do {
   r=upcase(readkey()); if (r=='\11')  flipover();
  } while (!(set::of(range('1','9'),'S','L','\0', eos).has(r)));
  switch (r) {
   case '1': {;
         do {
          t=typeno("Which object\? (0-30)");
         } while (!((t<30) || (t==255)));
         if ((t!=255) && (t!=0))  names[t][1]=typein(names[t][1]);
         current=t;
        }
        break;
   case '2': showallnames(); break;
   case '3': names[current][2]=typein(names[current][2]); break;
   case '4': showallassoc(); break;
   case '5': lino(); break;
   case '6': editmagics(); break;
   case '7': editportals(); break;
   case '8': editflags(); break;
   case '9': listen=typein(listen); break;
   case 'S': save(); break;
   case 'L': load(); break;
   case '\0': if (readkey()=='\55')  return; break;
  }
 } while (!false);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 output << "*** ALSO ***" << NL;
 output << NL;
 output << "No. of screen to edit?"; input >> n >> NL; load();
 assign(f,"v:avalot.fnt"); reset(f); f >> skinny; close(f);
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi"); current=0;
 loadscreen();
 alsomenu();
return EXIT_SUCCESS;
}
