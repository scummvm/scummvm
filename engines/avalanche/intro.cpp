


/*$I c:\sleep5\DSMI.INC*/
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

  const integer scardcount = 13;

     const array<0,scardcount-1,integer> soundcards = 
       {{1,2,6,3,4,5,8,9,10,7,7,7,7}};

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
boolean nomusic;

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
  if (nomusic) 
    cut_out_time -= 1;
  else
   if (ampgetmodulestatus != md_playing)  cut_out_time=0;
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

integer getsoundhardware(psoundcard scard)
{
    integer sc,i,autosel,select;
    char ch;
    integer e;



integer getsoundhardware_result;
Lagain:
  sc=detectgus(scard);
  if (sc!=0)  sc=detectpas(scard);
  if (sc!=0)  sc=detectaria(scard);
  if (sc!=0)  sc=detectsb(scard);

  /* if no sound card found, zero scard */
  if (sc!=0)  fillchar(scard,sizeof(tsoundcard),0);

  autosel=-1;
/*  if sc=0 then
    for i:=0 to scardcount-1 do
      if scard^.ID=soundcards[i].ID then begin
        { Set auto selection mark }
        autosel:=i+1;
        break;
      end;*/

  /* Print the list of sound cards */

  val(paramstr(13),select,e);

  /* Default entry? */
  if (select==0)  select=autosel;
  if (select!=autosel)  {
    /* clear all assumptions */
    sc=-1;
    fillchar(scard,sizeof(tsoundcard),0);
    scard->id=soundcards[select-1];  /* set correct ID */
  }

  /* Query I/O address */
  if (scard->id==id_dac)  scard->ioport=0x378;

  /* Read user input */
  val(paramstr(15),i,e);

  if (i!=0)  scard->ioport=i;
  if (sc!=1)    /* Not autodetected */
    switch (scard->id) {
      case id_sb16:
      case id_pas16:
      case id_wss:
      case id_aria:
      case id_gus    : scard->samplesize=2;
      break;                            /* 16-bit card */
      case id_sbpro:
      case id_pas:
      case id_pasplus: scard->stereo=true;
      break;                            /* enable stereo */
      default: {
        scard->samplesize=1;
        scard->stereo=false;
      }
    }

  if (scard->id!=id_dac)  {
    val(paramstr(17),i,e);

    if (i!=0)  scard->dmairq=i;

    val(paramstr(16),i,e);

    if (i!=0)  scard->dmachannel=i;
  } else {
    /* Select correct DAC */
    scard->maxrate=44100;
    if (select==11)  {
      scard->stereo=true;
      scard->dmachannel=1;    /* Special 'mark' */
      scard->maxrate=60000;
    } else
    if (select==12)  {
      scard->stereo=true;
      scard->dmachannel=2;
      scard->maxrate=60000;
      if (scard->ioport==0)  scard->ioport=0x378;
    } else
    if (select==13)  {
      scard->dmachannel=0;
      scard->ioport=0x42;     /* Special 'mark' */
      scard->maxrate=44100;
    }
  }

/*    writeln('Your selection: ',select,' at ',scard^.ioPort,
            ' using IRQ ',scard^.dmaIRQ,' and DMA channel ',scard^.dmaChannel);
  readln;*/

  getsoundhardware_result=0;
return getsoundhardware_result;
}

tsoundcard scard;
tmcpstruct mcpstrc;
tdds dds;
pmodule module;
tsdi_init sdi;
integer e,
bufsize;
char ch;
boolean v86,
vdsok;
longint a,rate,
tempseg;
string answer;
pointer temp;
word flags;
word curch;
byte modulevolume;
array<0,4,tsampleinfo> sample;
array<0,31,word> voltable;

int main(int argc, const char* argv[])
{

 pio_initialize(argc, argv);
 check_params();

 nomusic=paramstr(13)=='0';

 if (! nomusic) 
 {
    /* Read sound card information */
    if (getsoundhardware(&scard)==-1)  exit(1);


    /* Initialize Timer Service */
    tsinit;
    atexit(&tsclose);
    if (scard.id==id_gus)  {
      /* Initialize GUS player */
      #ifndef DPMI
      scard.extrafield[2]=1;  /* GUS DMA transfer does not work in V86 */
      #endif
      gusinit(&scard);
      atexit(&gusclose);

      /* Initialize GUS heap manager */
      gushminit;

      /* Init CDI */
      cdiinit;

      /* Register GUS into CDI */
      cdiregister(&cdi_gus,0,31);

      /* Add GUS event player engine into Timer Service */
      tsaddroutine(&gusinterrupt,gus_timer);
    } else {
      /* Initialize Virtual DMA Specification */
      #ifndef DPMI
      vdsok=vdsinit==0;
      #else
      vdsok=false;
      #endif

      fillchar(mcpstrc,sizeof(tmcpstruct),0);

      /* Query for sampling rate */
      val(paramstr(14),a,e);
      if (a>4000)  rate=a; else rate=21000;

      /* Query for quality */
      mcpstrc.options=mcp_quality;

      switch (scard.id) {
        case id_sb     : {
                      sdi=sdi_sb;
                      scard.maxrate=22000;
                    }
                    break;
        case id_sbpro  : {
                      sdi=sdi_sbpro;
                      scard.maxrate=22000;
                    }
                    break;
        case id_pas:
        case id_pasplus:
        case id_pas16  : {
                      sdi=sdi_pas;
                      scard.maxrate=44100;
                    }
                    break;
        case id_sb16   : {
                      sdi=sdi_sb16;
                      scard.maxrate=44100;
                    }
                    break;
        case id_aria   : {
                      sdi=sdi_aria;
                      scard.maxrate=44100;
                    }
                    break;
        case id_wss    : {
                      sdi=sdi_wss;
                      scard.maxrate=48000;
                    }
                    break;
        #ifndef DPMI
        case id_dac    : sdi=sdi_dac; break; /* Only available in real mode */
        #endif
      }

      mcpinitsounddevice(sdi,&scard);
      a=mcp_tablesize;
      mcpstrc.reqsize=0;

      /* Calculate mixing buffer size */
      bufsize=(longint)(2800*(integer)(scard.samplesize) << (byte)(scard.stereo))*
               (longint)(rate) / (longint)(22000);
      mcpstrc.reqsize=0;
      if ((mcpstrc.options & mcp_quality)>0) 
        if (scard.samplesize==1)  a += mcp_qualitysize; else
          a=mcp_tablesize16+mcp_qualitysize16;
      if ((longint)(bufsize)+(longint)(a)>65500)  bufsize=longint(65500)-a;

      #ifdef DPMI
      dpmiversion((byte)(e),(byte)(e),(byte)(e),flags);
      v86=(flags & 2)==0;
      #endif

      /* Allocate volume table + mixing buffer */
      #ifdef DPMI

      /* In the V86 mode, the buffer must be allocated below 1M */
      if (v86)  {
        tempseg=0;
        dpmiallocdos((a+longint(bufsize)) / longint(16)+longint(1),flags,(word)(tempseg));
      } else {
      #endif
      getmem(temp,a+longint(bufsize));
      if (temp==nil)  exit(2);
      #ifdef DPMI
      tempseg=seg(temp);
      }
      #else
      tempseg=seg(temp)+ofs(temp) / 16+1;
      #endif
      mcpstrc.bufferseg=tempseg;
      mcpstrc.bufferphysical=-1;

      if (vdsok && (scard.id!=id_dac))  {
        dds.size=bufsize;
        dds.segment=tempseg;
        dds.offset=0;

        /* Lock DMA buffer if VDS present */
        if (vdslockdma(&dds)==0)  mcpstrc.bufferphysical=dds.address;
      }
      if (mcpstrc.bufferphysical==-1) 
        #ifdef DPMI
        mcpstrc.bufferphysical=dpmigetlinearaddr(tempseg);
        #else
        mcpstrc.bufferphysical=(longint)(tempseg) << 4;
        #endif

      mcpstrc.buffersize=bufsize;
      mcpstrc.samplingrate=rate;
      /* Initialize Multi Channel Player */
      if (mcpinit(&mcpstrc)!=0)  exit(3);
      atexit(&mcpclose);

      /* Initialize Channel Distributor */
      cdiinit;

      /* Register MCP into CDI*/
      cdiregister(&cdi_mcp,0,31);
    }

    /* Try to initialize AMP */
    if (ampinit(0)!=0)  exit(3);
    atexit(&ampclose);

    /* Hook AMP player routine into Timer Service */
    tsaddroutine(&ampinterrupt,amp_timer);

    #ifndef DPMI
    /* If using DAC, then adjust DAC timer */
    if (scard.id==id_dac)  setdactimer(tsgettimerrate);
    #endif

    if (scard.id!=id_gus)  mcpstartvoice; else gusstartvoice;

    /* Load an example AMF */
    module=amploadmod("glover.mod",0);
    if (module==nil)  exit(4);

    /* Is it MCP, Quality mode and 16-bit card? */
    if ((scard.id!=id_gus) && ((mcpstrc.options & mcp_quality)>0)
       && (scard.samplesize==2))  {
      /* Open module+2 channels with amplified volumetable (4.7 gain) */
      for( a=1; a <= 32; a ++) voltable[a-longint(1)]=a*longint(150) / longint(32);
      cdisetupchannels(0,module->channelcount+2,&voltable);
    } else {
      /* Open module+2 channels with regular volumetable */
      cdisetupchannels(0,module->channelcount+2,nil);
    }

    curch=module->channelcount;
    modulevolume=64;

   /***/ ampplaymodule(module,0);
 }

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

 if (! nomusic)  ampstopmodule;
 graphmode(3);
 return EXIT_SUCCESS;
}
