


#include "graph.h"

integer gd,gm;

void load()     /* Load2, actually */
{
 byte a0 /*absolute $A000:800*/;
 byte bit;
 untyped_file f;

 assign(f,"maintemp.avd"); reset(f,1);
 /* Compression method byte follows this... */
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a0,12080);
 }
 close(f);
 bit=getpixel(0,0);
}

void save()
{
    byte bit; untyped_file f;

 assign(f,"v:mainmenu.avd");
 rewrite(f,1);
 for( bit=0; bit <= 3; bit ++)
  {
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   blockwrite(f,mem[0xa000*48*80],59*80);
  }
 close(f);
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 gd=3; gm=1; initgraph(gd,gm,"");
 load(); /* Between 48 and 107. */

 save();
 return EXIT_SUCCESS;
}
