


/*#include "Dos.h"*/
#include "graph.h"
/*#include "Crt.h"*/

 /* This is a stand-alone program. */

/* 0, black, remains 0.
   Other numbers: the bits take precedence from the left.
    e.g. for 9 = 1001, => fourth bit.

    First 1 is in:

     Fourth bit: 63 (egaWhite)
      Third bit: 57 (egaLightBlue)
     Second bit: 7  (light grey)
      First bit: 1  (blue). */

const palettetype our_palette = 
                {16,
                         /* sic */
{{  0,  1, 57, 57,  7,  7,  7,  7, 63, 63, 63, 63, 63, 63, 63, 63}}};

typedef matrix<'\0','\377',1,16,byte> fonttype;

fonttype f;
matrix<0,39,1,16,byte> next_line;

byte next_bitline;

byte displaycounter;

boolean cut_out;

word cut_out_time;

array<1,117,varying_string<40> > x;

byte this_line;

word* skellern;

/*$L intro.obj*/
extern void introduction(); 

void graphmode(integer mode)
{
    registers regs;

 regs.ax=mode;
 intr(0x10,regs);
}

/* Firstly, port[$3C4]:=2; port[$3CF]:=4;,
  Then port[$3C5]:=1 shl bit; port[$3CF]:=bit;. */

void loadfont()
{
    file<fonttype> ff;

 assign(ff,"avalot.fnt");
 reset(ff);
 ff >> f;
 close(ff);
}

void calc_next_line()
 /* This proc sets up next_line. */
{
 string l;
 byte fv,ff;
 boolean oddlen;
 byte start;
 byte this_;

 fillchar(next_line,sizeof(next_line),'\0'); /* All blanks. */

 if (this_line==117) 
 {
  cut_out=true;
  return;
 }

 l=x[this_line];
 this_line += 1;

 start=(20-length(l) / 2)-1;
 oddlen=odd(length(l));

 for( fv=1; fv <= length(l); fv ++)
  for( ff=1; ff <= 16; ff ++)
  {
   this_=f[l[fv]][ff];
   if (oddlen) 
   {     /* Odd, => 4 bits shift to the right. */
    next_line[start+fv][ff] += this_ << 4;
    next_line[start+fv-1][ff] += (cardinal)this_ >> 4;
   } else
   {     /* Even, => no bit shift. */
    next_line[start+fv][ff]=this_;
   }
  }
 next_bitline=1;
}

void display()
{
    byte fv,ff;


 if (next_bitline == 17)  calc_next_line();

 if (cut_out) 
 {
  cut_out_time -= 1;
  return;
 }

 move(mem[0xa000*40],mem[0xa000*0],7960);
 for( fv=0; fv <= 39; fv ++)
   mem[0xa1f1*8+fv]=next_line[fv][next_bitline];
 next_bitline += 1;

}

void plot_a_star(integer x,integer y)
{
    byte ofs;

 ofs=x % 8;
 x=x / 8;
 mem[0xa000*x+y*40] += (cardinal)128 >> ofs;
}

void plot_some_stars(integer y)
{
    byte fv,times;

 switch (Random(7)) {
  case 1: times=1; break;
  case 2: times=2; break;
  case 3: times=3; break;
  default: return;
 }

 for( fv=1; fv <= times; fv ++)
  plot_a_star(Random(320),y);
}

void starry_starry_night()
{
 integer y;
 byte bit;

 port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;

 for( bit=0; bit <= 2; bit ++)
 {
  port[0x3c5]=1 << bit; port[0x3cf]=bit;
  for( y=1; y <= 200; y ++)
   plot_some_stars(y);
 }
}

void setupgraphics()     /* Fix this proc. This prog SHOULDN'T use the
 Graph unit. */
{
    integer gd,gm;

 gd=3; gm=1; initgraph(gd,gm,"");
}

void shovestars()
{
 move(mem[0xa000*0],mem[0xa000*40],7960);
 fillchar(mem[0xa000*0],40,'\0');
 plot_some_stars(0);
}

void do_next_line()
{
    byte bit;

 port[0x3c4]=2; port[0x3ce]=4;

 for( bit=0; bit <= 3; bit ++)
 {
  port[0x3c5]=1 << bit; port[0x3cf]=bit;

  switch (bit) {
   case 0: if ((displaycounter % 10)==0)  shovestars(); break;
   case 1: if ((displaycounter % 2)==0)  shovestars(); break;
   case 2: shovestars(); break;
   case 3: display(); break; /* The text. */
  }
 }

 if (displaycounter==40)  displaycounter=0;

}

void load_text()
{
 word fv;
 char* c;
 byte thisline;


 c=addr(introduction());
 thisline=0;
 fillchar(x,sizeof(x),'\0');

 for( fv=1; fv <= 2456; fv ++)
 {
  switch (*c) {
   case '\15': thisline += 1; break;
   case '\12':/*nop*/; break;
   default: x[thisline]=x[thisline]+*c;
  }

  c += 1;
 }
}

void check_params()
{
    word s,o; integer e;

 if (paramstr(1)!="jsb")  exit(0);
 val(paramstr(2),s,e); if (e!=0)  exit(0);
 val(paramstr(3),o,e); if (e!=0)  exit(0);
 skellern=ptr(s,o+1);
}

int main(int argc, const char* argv[])
{

 pio_initialize(argc, argv);
 check_params();

 setupgraphics();

 randseed=177; checkbreak=false;

 load_text();

 this_line=1;

 graphmode(0xd);
 loadfont();

 next_bitline=17;
 displaycounter=0;

 cut_out_time=333;

 setallpalette(our_palette);

 starry_starry_night();

 while ((cut_out_time>0) && (! keypressed())) 
 {

  *skellern=0;

  do_next_line();

  displaycounter += 1;

  do {; } while (!(*skellern>0));
 }

 graphmode(3);
 return EXIT_SUCCESS;
}
