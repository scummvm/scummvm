#include "ptoc.h"


#include "graph.h"

integer gd,gm;
string fn1,fn2;
varying_string<30> desc;

void loadscreen(string nam)
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

void load2(string name)
{
 byte a /*absolute $A000:1200*/;
 byte bit;
 untyped_file f;
;
 assign(f,name); reset(f,1); seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a,12080);
 }
 close(f);
}

void save2(string name)
{
      const string header = 
 string("This is a file from an Avvy game, and its contents are subject to ")+
 "copyright."+'\15'+'\12'+'\15'+'\12'+"Have fun!"+'\32';
 byte a /*absolute $A000:1200*/;
 byte bit;
 untyped_file f;
;
 assign(f,name); rewrite(f,1); blockwrite(f,header[1],146); /* really 90 */
 blockwrite(f,desc,31);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockwrite(f,a,12080);
 }
 close(f);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 output << "Filename?"; input >> fn1 >> NL;
 output << "New name?"; input >> fn2 >> NL;
 output << "Describe?"; input >> desc >> NL;
 gd=3; gm=0; initgraph(gd,gm,"");
 loadscreen(fn1);
 save2(fn2);
 closegraph();
return EXIT_SUCCESS;
}
