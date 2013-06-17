#include "ptoc.h"


#include "graph.h"

integer gd,gm;
byte a /*absolute $A000:(15*80)*/;
byte bit;
untyped_file f;

void load(string nam)
{
    byte z;
 array<1,4,pointer> a;
 untyped_file f; word s;
 string check;
;
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
 gd=3; gm=0; initgraph(gd,gm,"");
 load("d:britain.avd");
 assign(f,"c:\\sleep\\test.ega"); rewrite(f,1);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockwrite(f,a,12000);
 }
 close(f);
return EXIT_SUCCESS;
}
