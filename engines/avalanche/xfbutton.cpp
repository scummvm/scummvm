#include "ptoc.h"


#include "graph.h"

integer x,y;
untyped_file f,out;

void load()     /* Load2, actually */
{
 byte a0;  /*absolute $A000:800;*/
 byte bit;
 untyped_file f;
 integer gd,gm;

 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 assign(f,"d:butnraw.avd"); reset(f,1);
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a0,12080);
 }
 close(f);
 bit=getpixel(0,0);
}

void grab(integer x1,integer y1,integer x2,integer y2) /* s=930 */
{
    word s; pointer p;

 s=imagesize(x1,y1,x2,y2);
 getmem(p,s);
 getimage(x1,y1,x2,y2,p);
 putimage(0,0,p,0);
 blockwrite(out,p,s);
 freemem(p,s);
 rectangle(x1,y1,x2,y2);
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 assign(f,"d:butnraw.avd");
 assign(out,"v:buttons.avd"); rewrite(out,1);
 load();
 for( x=0; x <= 5; x ++)
  for( y=0; y <= 3; y ++)
  {
   if (!
    (((x==1) && (y==0))
  || ((x==4) && (y==2))
  || ((y==3) && (x>2) && (x<5)))) 
     {
      input >> NL;
      grab(100+x*83,51+y*22,180+x*83,71+y*22);
     }
  }
 close(out);
 return EXIT_SUCCESS;
}
