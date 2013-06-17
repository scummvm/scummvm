/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



                 /* Trippancy IV - original file */
#include "graph.h"

const integer taboo = cyan;

struct adxtype {
           varying_string<12> name; /* name of character */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of all the pictures */
};


integer gd,gm;
adxtype adx;
matrix<1,24,0,1,pointer> adxpic;     /* the pictures themselves */
untyped_file f; string x; byte n;

void load(string nam)
{
    byte z;
 array<1,4,pointer> a;
 untyped_file f; word s;
 varying_string<40> xxx;
 string check;

;
 assign(f,nam);
 reset(f,1);
 blockread(f,xxx,41);
 blockread(f,check,13);
 blockread(f,check,31);
 s=imagesize(0,0,getmaxx(),75);
 for( z=1; z <= 2; z ++)
 {;
  getmem(a[z],s);
  blockread(f,a[z],s);
  putimage(0,15+(z-1)*75,a[z],0);
  freemem(a[z],s);
 }
 close(f);
}

void silhouette()
{
    byte x,y,c;
;
 setvisualpage(1); setactivepage(1); setfillstyle(1,15);
 for( gm=0; gm <= 3; gm ++)
  for( gd=1; gd <= 6; gd ++)
  {;     /* 26,15 */
/*   bar((gm div 2)*320+gd*40,20+(gm mod 2)*40,(gm div 2)*320+gd*40+35,(gm mod 2)*40+60); */
   for( y=1; y <= adx.yl; y ++)
    for( x=1; x <= adx.xl; x ++)
    {;
     setactivepage(0);
     c=getpixel((gm / 2)*320+gd*40+x,20+(gm % 2)*40+y);
     setactivepage(1);
/*     if c<>taboo then putpixel((gm div 2)*320+gd*40+x,20+(gm mod 2)*40+y,0); */
     if (c==taboo)  putpixel((gm / 2)*320+gd*40+x,20+(gm % 2)*40+y,15);
    }
   getmem(adxpic[gm*6+gd][0],adx.size);
   getimage((gm / 2)*320+gd*40+1,20+(gm % 2)*40+1,
     (gm / 2)*320+gd*40+adx.xl,20+(gm % 2)*40+adx.yl,
      adxpic[gm*6+gd][0]);
  }
}

void standard()
{
    byte x,y,c;
;
 setvisualpage(2); setactivepage(2);
 for( gm=0; gm <= 3; gm ++)
  for( gd=1; gd <= 6; gd ++)
  {;     /* 26,15 */
   for( y=1; y <= adx.yl; y ++)
    for( x=1; x <= adx.xl; x ++)
    {;
     setactivepage(0);
     c=getpixel((gm / 2)*320+gd*40+x,20+(gm % 2)*40+y);
     setactivepage(2);
     if (c!=taboo)  putpixel((gm / 2)*320+gd*40+x,20+(gm % 2)*40+y,c);
    }
   getmem(adxpic[gm*6+gd][1],adx.size);
   getimage((gm / 2)*320+gd*40+1,20+(gm % 2)*40+1,
     (gm / 2)*320+gd*40+adx.xl,20+(gm % 2)*40+adx.yl,
      adxpic[gm*6+gd][1]);
  }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,""); fillchar(adxpic,sizeof(adxpic),'\261');
 load("v:avalots.avd");
/* getmem(adxpic[1,1,1],adx.size); getimage(40,20,75,60,adxpic[1,1,1]^);
 putimage(100,100,adxpic[1,1,1]^,0); */
 {;
  adx.name="Avalot";
  adx.num=24; adx.seq=6;
  adx.xl=32; adx.yl=35; /* 35,40 */

  adx.size=imagesize(40,20,40+adx.xl,60+adx.yl);
 }
 silhouette();
 standard();
 x=string("Sprite file for Avvy - Trippancy IV. Subject to copyright.")+'\32';
 assign(f,"v:sprite1.avd");
 rewrite(f,1);
 blockwrite(f,x[1],59);
 blockwrite(f,adx,sizeof(adx));
 for( gd=1; gd <= adx.num; gd ++)
  for( gm=0; gm <= 1; gm ++)
   blockwrite(f,adxpic[gd][gm],adx.size); /* next image */
 close(f);
 closegraph();
return EXIT_SUCCESS;
}