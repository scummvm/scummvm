


#include "graph.h"


const integer picsize = 966;

integer gd,gm;
untyped_file f;
pointer p;

void save()
{
    varying_string<40> adf;  untyped_file f; byte z; array<1,2,pointer> c;
string nam,screenname;
      const string header = 
 string("This is a file from an Avvy game, and its contents are subject to ")+
 "copyright."+'\15'+'\12'+'\15'+'\12'+"Have fun!"+'\32';
 byte a /*absolute $A000:1200i*/;
 byte bit;
;
 nam="d:thingtmp.avd";
 screenname="Temp.";
 assign(f,nam);

     assign(f,nam); rewrite(f,1); blockwrite(f,header[1],146);
     blockwrite(f,screenname,31);
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
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 assign(f,"thinks.avd");
 getmem(p,picsize);
 reset(f,1);
 seek(f,65);
 gd=10; gm=20;

 while (gm<120) 
 {;
  if (! eof(f)) 
   blockread(f,p,picsize);
  putimage(gd,gm,p,0);
  gd += 70;

  if (gd==640) 
  {;
   gd=10; gm += 40;
  }

 }

 close(f); freemem(p,picsize);
 save();
return EXIT_SUCCESS;
}
