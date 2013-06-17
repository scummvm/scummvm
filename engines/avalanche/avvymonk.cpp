


#include "graph.h"

integer gd,gm;
byte z;
array<1,4,pointer> a;
untyped_file f; word s;
string check;
integer x,y;
void savesc()
{
    varying_string<40> adf; untyped_file f; byte z; array<1,2,pointer> c; word s;
varying_string<14> nam; varying_string<30> screenname;
;
 nam="v:avvymonk.avd";
 adf=string("aved as a stuff file, so there! TJAT.")+'\15'+'\12'+'\32';
 adf[0]='S';
 screenname="Avalot in a monk's costume";
 assign(f,nam); /* not scrambled */
 rewrite(f,1);
 blockwrite(f,adf,41);
 blockwrite(f,nam,13);
 blockwrite(f,screenname,31);
 s=imagesize(0,0,getmaxx(),75);
 for( z=1; z <= 2; z ++)
 {;
  getmem(c[z],s);
  getimage(0,15+(z-1)*75,getmaxx(),15+(z)*75,c[z]);
  blockwrite(f,c[z],s);
  freemem(c[z],s);
 }
}
void loadscreen(string nam)
{;
 assign(f,nam);
 reset(f,1);
 blockread(f,check,41);
 blockread(f,check,13);
 blockread(f,check,31);
 s=imagesize(0,0,getmaxx(),75);
 for( z=1; z <= 2; z ++)
 {;
  getmem(a[z],s);
  blockread(f,a[z],s);
  setactivepage(0);
  putimage(0,15+(z-1)*75,a[z],0);
  freemem(a[z],s);
 }
 close(f);
}
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0;
 x=0; y=0;
 initgraph(gd,gm,"o:");
 loadscreen("v:legion.avd");
 while (y<100) 
 {;
  switch (getpixel(x,y)) {
   case 7: putpixel(x,y,0); break;
   case 8: putpixel(x,y,0); break;
   case 9: putpixel(x,y,15); break;
  }
  x += 1;
  if (x>640)  {; y += 1; x=0; }
 }
 savesc();
return EXIT_SUCCESS;
}