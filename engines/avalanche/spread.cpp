/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



#include "graph.h"
/*#include "Crt.h"*/


struct adxtype {
           varying_string<12> name; /* name of character */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
};

varying_string<3> sn;
adxtype a;
matrix<1,24,0,1,pointer> pic;     /* the pictures themselves */
word bigsize;

const array<0,15,varying_string<6> > col = 
 {{"Black","Blue","Green","Cyan","Red","Pink","Brown","Grey+",
  "Grey-","Blue+","Green+","Cyan+","Red+","Pink+","Yellow","White"}};
const string prompt = string('\32')+'\257';

void load()
{
 untyped_file f; byte gd,gm,sort,n; pointer p,q;
;
 assign(f,string("v:sprite")+sn+".avd"); reset(f,1); seek(f,59);
 blockread(f,a,sizeof(a)); blockread(f,bigsize,2);
 setactivepage(3);
 for( sort=0; sort <= 1; sort ++)
 {;
  mark(q); getmem(p,bigsize);
  blockread(f,p,bigsize);
  putimage(0,0,p,0); release(q); n=1;
   for( gm=0; gm <= (a.num / a.seq)-1; gm ++) /* directions */
    for( gd=0; gd <= a.seq-1; gd ++) /* steps */
    {;
     getmem(pic[n][sort],a.size); /* grab the memory */
     getimage((gm / 2)*(a.xl*6)+gd*a.xl,(gm % 2)*a.yl,
       (gm / 2)*(a.xl*6)+gd*a.xl+a.xl-1,(gm % 2)*a.yl+a.yl-1,
       pic[n][sort]); /* grab the pic */
     n += 1;
   }
 }
 close(f); setactivepage(0);
}

void save()
{
 untyped_file f; byte gd,gm,sort,n; pointer p,q;
 string x; integer txl,tyl;
;
 {;
  txl=a.seq*a.xl*2; tyl=a.yl*2;
 }

 assign(f,string("v:sprite")+sn+".avd");
 x=string("Sprite file for Avvy - Trippancy IV. Subject to copyright.")+'\32';
 rewrite(f,1); blockwrite(f,x[1],59);

 blockwrite(f,a,sizeof(a)); blockwrite(f,bigsize,2);
 setactivepage(3);
 for( sort=0; sort <= 1; sort ++)
 {;
  mark(q); getmem(p,bigsize); n=1;
   for( gm=0; gm <= (a.num / a.seq)-1; gm ++) /* directions */
    for( gd=0; gd <= a.seq-1; gd ++) /* steps */
    {;
     putimage((gm / 2)*(a.xl*6)+gd*a.xl,(gm % 2)*a.yl,
       pic[n][sort],0); /* drop the pic */
     n += 1;
   }
  getimage(0,0,txl,tyl,p);
  blockwrite(f,p,bigsize); release(q);
 }
 close(f); setactivepage(0);
}

void setup()
{
    integer gd,gm;
;
 output << "SPREAD (c) 1992, Thomas Thurman." << NL; output << NL;
 output << "Enter number of SPRITE*.AVD file to edit:"; input >> sn >> NL;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 load();
}

string strf(longint x)
{
    string q;
string strf_result;
;
 str(x,q); strf_result=q;
return strf_result;
}

void likethis()
{;
 {;
  setfillstyle(1,a.bgc); setcolor(a.fgc); settextstyle(0,0,1);
  bar(0,190,100,200); outtextxy(12,191,"Like this!");
 }
}

void values()
{
    byte fv;
;
 settextstyle(2,0,9); setcolor(14);
 outtextxy(277,0,string("Spread: editing ")+sn); setcolor(15);
           for( fv=0; fv <= 3; fv ++) putimage(77+(a.xl+10)*fv,17,pic[a.seq*fv+1][1],0);
 settextstyle(2,0,7);
 outtextxy(0, 30,"Views:");
 {;
  outtextxy(0, 50,string("N: Name: ")+a.name);
  outtextxy(0, 70,string("No. of pictures: ")+strf(a.num)+" ("+strf(a.num / a.seq)+" ways)");
  outtextxy(0, 90,string("XY: Size: ")+strf(a.xl)+'x'+strf(a.yl));
  outtextxy(0,110,string("S: Stride size: ")+strf(a.seq));
  outtextxy(0,130,string("Imagesize (bytes): ")+strf(a.size));
  outtextxy(0,150,string("B: Bubble background: ")+col[a.bgc]);
  outtextxy(0,170,string("F: Bubble foreground: ")+col[a.fgc]);
  likethis();
 }
 setcolor(lightgreen); settextstyle(0,0,1);
 outtextxy(400,50,"A) Animate");
 outtextxy(400,60,"E) Edit pictures");
 outtextxy(400,70,"alt-f2) Save");
 outtextxy(400,80,"ctrl-f3) Load");
 outtextxy(400,90,"or any key to the left...");
}

byte ccol()
{
    byte fv;
byte ccol_result;
;
 restorecrtmode();
 output << "Choose a colour- one of these..." << NL;
 for( fv=0; fv <= 15; fv ++)
 {;
  textattr=14; output << fv << ") "; textattr=fv; output << string('\376')+'\40';
  textattr=14; output << col[fv] << NL;
 }
 textattr=14;
 do {
  output << prompt; input >> fv >> NL;
 } while (!(fv<16));
 ccol_result=fv; setgraphmode(0);
return ccol_result;
}

string cstr(string oc)
{
    string x;
string cstr_result;
;
 restorecrtmode();
 output << string("Old choice is: <")+oc+'>' << NL;
 output << NL;
 output << "(Anything after a semi-colon will not be displayed by the game, e.g. Avvy;Monk" << NL;
 output << " will be displayed as Avvy.)" << NL;
 output << NL;
 output << string("New choice, Enter for no change, Space+Enter for a blank?")+prompt; input >> x >> NL;
 if (x=="")  cstr_result=oc; else if (x==' ')  cstr_result=""; else cstr_result=x;
 setgraphmode(0);
return cstr_result;
}

longint cnum(longint on)
{
    string x; longint q; integer e;
longint cnum_result;
;
 restorecrtmode();
 do {
  output << "Old value is: " << on << '.' << NL;
  output << string("New choice, or Enter for no change?")+prompt; input >> x >> NL;
  if (x=="") 
  {;
   e=0;  /* must be OK here */ q=on;
  } else val(x,q,e);
  if (e!=0)  output << x << " isn't a number, silly!" << NL;
 } while (!(e==0));
 setgraphmode(0); cnum_result=q;
return cnum_result;
}

void animate()
{
    byte facing,step,slow,fv;
;
 cleardevice();
 settextstyle(0,0,2); setcolor(12); outtextxy(0,0,"Animate");
 settextstyle(0,0,1); setcolor(15);
 outtextxy(0,20,"Enter = Turn, + = Faster, - = Slower, Esc = stop this.");
 facing=0; step=1; slow=100;
  do {
   for( fv=0; fv <= 1; fv ++)
    putimage(200*fv+177,77,pic[facing*a.seq+step][fv],4-fv*4);
   if (keypressed()) 
    switch (upcase(readkey())) {
     case '\15': {;
           facing += 1; if (facing*a.seq>=a.num)  facing=0;
          }
          break;
     case '\33': {; cleardevice(); return; } break;
     case '+': if (slow>0)  slow -= 5; break;
     case '-': if (slow<255)  slow += 5; break; default: output << '\7';
    }
   step += 1; if (step>a.seq)  step=1;
   delay(slow);
  } while (!false);
}

byte tabpel(integer x,integer y)
{byte tabpel_result;
;
 if (getpixel(400+x,17+y)==15)  tabpel_result=17;
  else tabpel_result=getpixel(500+x,17+y);
return tabpel_result;
}

void bigpixel(integer x,integer y, byte size,byte col)
{;
 if (col==17)  setfillstyle(9,8); else setfillstyle(1,col);
 bar(x*size,y*size,x*size+size-2,y*size+size-2);
}

void blowup(byte n)
{
    byte fv,x,y,xyl;
;
 {;
  for( fv=0; fv <= 1; fv ++) putimage(400+fv*100,17,pic[n][fv],0);
  xyl=200 / a.yl;
  for( x=0; x <= a.xl; x ++)
   for( y=0; y <= a.yl; y ++)
    bigpixel(x,y,xyl,tabpel(x,y));
 }
}

void edit();

static void putnum(byte x,byte p)
{
    varying_string<2> z;
;
 str(x,z); outtextxy(x*53+17,87,z); putimage(x*53,100,pic[p][1],0);
}


static void title()
{;
 cleardevice(); setcolor(11); settextstyle(0,0,2);
 outtextxy(0,0,"Edit- which one?"); settextstyle(0,0,1); setcolor(15);
}

void edit()
{
 byte fv,ra,rb;
;
 {;
  title(); for( fv=1; fv <= (a.num / a.seq); fv ++) putnum(fv,fv*a.seq);
  do { ra=ord(readkey())-48; } while (!(ra<(a.num / a.seq))); ra -= 1;
  title(); for( fv=1; fv <= a.seq; fv ++) putnum(fv,ra*a.seq+fv);
  do { rb=ord(readkey())-48; } while (!(rb<a.seq));
  cleardevice();
  blowup(ra*a.seq+rb); input >> NL;
  cleardevice();
 }
}

void pickone()
{
    char r;
;
 r=upcase(readkey());
  switch (r) {
   case 'N': a.name=cstr(a.name); break;
   case 'S': a.seq=cnum(a.seq); break;
   case 'F': a.fgc=ccol(); break;
   case 'B': a.bgc=ccol(); break;
   case 'A': animate(); break;
   case 'E': edit(); break;
   case '\0': switch (readkey()) {
        case 'i': save(); break; /* alt-f2 */
       }
       break;
  }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 setup();
 do {
  values();
  pickone();
 } while (!false);
return EXIT_SUCCESS;
}