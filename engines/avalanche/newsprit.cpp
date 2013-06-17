


#include "graph.h"
/*#include "Crt.h"*/


const string crlf = string('\15')+'\12'; const char eof_ = '\32';
const array<1,177,char> trip5head = 
         string("Sprite*.AVD  ...  data file for Trippancy Five")+crlf+crlf+
         "[Thorsoft relocatable fiveplane sprite image format]"+crlf+crlf+
         "Thomas Thurman was here.  ...  Have fun!"+crlf+crlf+eof_+
         "±±±±±±± * G. I. E. D. ! * ";

const array<1,4,char> tripid = string('\x30')+'\x1'+'\x75'+'\261';

const array<1,50,char> trip5foot = crlf+crlf+
         " and that's it! Enjoy the game. "+'\3'+crlf+crlf+
            "\n\n\n\n\n\n\n"+"tt";

struct adxotype {
           varying_string<12> name; /* name of character */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
};

struct adxtype {
           varying_string<12> name; /* name of character */
           varying_string<16> comment; /* comment */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
           byte accinum; /* the number according to Acci (1=Avvy, etc.) */
};

varying_string<2> sn;
adxotype oa;
adxtype a;
matrix<1,24,0,1,pointer> pic;     /* the pictures themselves */
array<1,16000,byte> aa;
untyped_file out;
integer bigsize;

void copyaoa()
{;
 {;
  a.name=oa.name;
  a.comment="Transferred";
  a.num=oa.num;
  a.xl=oa.xl;
  a.yl=oa.yl;
  a.seq=oa.seq;
  a.size=oa.size;
  a.fgc=oa.fgc;
  a.bgc=oa.bgc;
 }
}

void setup()
{
    integer gd,gm;
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
}

string strf(longint x)
{
    string q;
string strf_result;
;
 str(x,q); strf_result=q;
return strf_result;
}

void save()
{
 byte sort,n;
 word fv,ff; char r; byte xw;
 byte nxl,nyl;
 word soa;
;
 cleardevice();
 {;
  a.xl=45; a.yl=10; a.num=1; a.seq=1;
  a.size=imagesize(0,0,a.xl,a.yl);
  soa=sizeof(a);

  assign(out,"v:sprite10.avd"); rewrite(out,1);
  blockwrite(out,trip5head,177);
  blockwrite(out,tripid,4);
  blockwrite(out,soa,2);
  blockwrite(out,a,soa);

  nxl=a.xl; nyl=a.yl;
  xw=nxl / 8;
  if ((nxl % 8)>0)  xw += 1;

  for( n=1; n <= a.num; n ++)
  {;
   getimage(  0,0,a.xl,a.yl,aa);
   for( fv=0; fv <= nyl; fv ++)
    blockwrite(out,aa[5+fv*xw*4],xw);

   getimage(100,0,100+a.xl,a.yl,aa);
   blockwrite(out,aa[5],a.size-6);
  }
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 setup();
 save();

 blockwrite(out,trip5foot,50);
 close(out);
return EXIT_SUCCESS;
}