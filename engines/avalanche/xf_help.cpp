


/*#include "Crt.h"*/
/*#include "Tommys.h"*/


const integer max_pages = 34;

byte fv;
text i;
untyped_file o;
string x;
char t;
byte p,w;

array<0,max_pages,word> where;

void out(string x)
{
    byte fz;

 for( fz=1; fz <= length(x); fz ++)
   x[fz]=chr(ord(x[fz]) ^ 177);
 blockwrite(o,x[0],1);
 blockwrite(o,x[1],length(x));
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 assign(o,"help.avd");
 rewrite(o,1);

 blockwrite(o,where,sizeof(where));

 for( fv=0; fv <= max_pages; fv ++)
 {
  where[fv]=filepos(o);

  assign(i,string('h')+strf(fv)+".raw");
  reset(i);

  i >> x >> NL; /* Title. */
  out(x);

  i >> p >> NL; /* Picture. */
  blockwrite(o,p,1);

  do {
   i >> x >> NL;
   out(x);
  } while (!(x=='!'));

  while (! eof(i)) 
  {
   i >> x >> NL;
   if (x=='-') 
   {     /* Null point */
    t='\0'; p=0; w=177;
   } else
   {     /* Has a point. */
    i >> t >> NL;
    i >> p >> NL;
    i >> w >> NL;
   }

   blockwrite(o,t,1);
   blockwrite(o,p,1);
   blockwrite(o,w,1);
  }

  t='\261';
  blockwrite(o,t,1);
  blockwrite(o,p,1);
  blockwrite(o,w,1);

  close(i);

 }

 seek(o,0); blockwrite(o,where,sizeof(where));

 close(o);
 return EXIT_SUCCESS;
}