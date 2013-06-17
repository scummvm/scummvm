

               /* Demonstration of the Bigo II system. */
/* $I c:\sleep5\DSMI.INC*/
#include "graph.h"
/*#include "Crt.h"*/
/*#include "Rodent.h"*/
/*#include "Tommys.h"*/

integer gd,gm;

typedef matrix<'\0','\377',0,15,byte> fonttype;

boolean reverse;
word spinnum;
array<0,1,fonttype> f;
file<fonttype> ff;
array<0,7,0,15,0,79,byte> strip;
byte across;
word w; byte y;
byte charnum;
byte cfont; /* current font. 0=roman, 1=italic. */

char* c;

boolean nomusic;

const integer colours = 12; /* Run Workout to see possible values of these two. */
const integer steps = 6; /* 60,30,20,15,12,10,6,5,4,3,2,1 */
const integer gmtop = 360 / steps;

 const integer scardcount = 13;

     const array<0,scardcount-1,integer> soundcards = 
       {{1,2,6,3,4,5,8,9,10,7,7,7,7}};

/*$L credits.obj*/ extern void credits(); 

void bigo2(string date)
{
 integer gd,gm;
 byte c;
 palettetype p;
 untyped_file f; pointer pp; word s;


 getpalette(p);
 setvisualpage(1); setactivepage(0);
 assign(f,"logo.avd"); reset(f,1);
 for( gd=7; gd <= 194; gd ++)
  blockread(f,mem[0xa000*(gd*80)],53);
 close(f);
 s=imagesize(0,7,415,194); getmem(pp,s); getimage(0,7,415,194,pp);

 cleardevice();
 for( gd=1; gd <= 64; gd ++)
 {
  for( gm=0; gm <= gmtop; gm ++)
  {
   c=(c % colours)+1;
/*   putpixel(trunc(sin(gm*steps*n)*gd*6)+320,
              trunc(cos(gm*steps*n)*gd*3)+175,c); */
   if (c>5)  continue_;
   setcolor(c); arc(320,175,gm*steps,gm*steps+1,gd*6);
  }
  if (keypressed())  { closegraph(); exit(0); }
 }
 settextstyle(0,0,1); setcolor(13);
 outtextxy(550,343,"(press any key)");

 putimage(112,0,pp,orput); freemem(pp,s);
 resetmouse; setvisualpage(0);
}

void nextchar()     /* Sets up charnum & cline for the next character. */
{

 c += 1;
}

void getchar()
{
 do {
  nextchar();

  switch (*c) {
   case '@': { cfont=1; nextchar(); } break;
   case '^': { cfont=0; nextchar(); } break;
   case '%': if (nomusic) 
        {
          closegraph(); exit(0);      /* End of text and no music => stop. */
        }
        else c=addr(credits());
        break;                   /* End of test, but still playing => again. */
  }

 } while (!((*c!='\15') && (*c!='\12')));

 for( w=0; w <= 7; w ++)
  for( y=0; y <= 15; y ++)
   move(strip[w][y][1],strip[w][y][0],79);

 for( w=0; w <= 7; w ++)
  for( y=0; y <= 15; y ++)
   strip[w][y][79]=(byte)((strip[7][y][78] << (w+1)))+
    ((cardinal)f[cfont][*c][y] >> (7-w));

 across=0;
}

void scrolltext()
{
 byte c,w,y;

 across += 1;
 if (across==8)  getchar();

 for( y=0; y <= 15; y ++)
  for( w=0; w <= 1; w ++)
  move(strip[across][y][0],mem[0xa000*24000+(y*2+w)*80],80);
}

void do_stuff()
{
 switch (spinnum) {
  case 50 ... 110: {
            setfillstyle(1,14);
            bar(0,315+(spinnum-50) / 3,640,315+(spinnum-50) / 3);
            bar(0,316-(spinnum-50) / 3,640,316-(spinnum-50) / 3);
            if (spinnum>56) 
            {
             setfillstyle(1,13);
             bar(0,315+(spinnum-56) / 3,640,315+(spinnum-56) / 3);
             bar(0,316-(spinnum-56) / 3,640,316-(spinnum-56) / 3);
            }
          }
          break;
  case 150 ... 198: {
             setfillstyle(1,0);
             bar(0,315+(spinnum-150) / 3,640,315+(spinnum-150) / 3);
             bar(0,316-(spinnum-150) / 3,640,316-(spinnum-150) / 3);
            }
            break;
  case 200: scrolltext(); break;
 }
}

void setcol(byte which,byte what)
/*var dummy:byte;*/
{;
/* setpalette(which,what);
 asm
(*  mov dx,$3DA;
  in ax,dx;

  or ah,ah;

  mov dx,$3C0;
  mov al,which;
  out dx,al;

  mov dx,$3C0;
  mov al,what;
  out dx,al;
 end;
(* dummy:=port[$3DA];
 port[$3C0]:=which; port[$3C0]:=what;*/
}

void bigo2go()
{
 palettetype p; byte c; boolean lmo;
 boolean altnow,altbefore;

 for( gd=0; gd <= 13; gd ++) p.colors[gd]=0;

 setcol(13,24); /* murk */ setcol(14,38); /* gold */
 setcol(15,egawhite); /* white- of course */
 p.colors[13]=24; p.colors[14]=38; p.colors[15]=egawhite;

 /***/

    p.colors[5]=egawhite;
    p.colors[4]=egalightcyan;
    p.colors[3]=egacyan;
    p.colors[2]=egalightblue;
    p.colors[1]=egablue;

 /***/

 c=1; p.size=16; lmo=false;
 setallpalette(p);

 do {
/*  if reverse then
  begin
   dec(c); if c=0 then c:=colours;
  end else
  begin
   inc(c); if c>colours then c:=1;
  end;
  for gm:=1 to colours do
   case p.colors[gm] of
    egaWhite: begin p.colors[gm]:=egaLightcyan; setcol(gm,egaLightCyan); end;
    egaLightcyan: begin p.colors[gm]:=egaCyan; setcol(gm,egaCyan); end;
    egaCyan: begin p.colors[gm]:=egaLightblue; setcol(gm,egaLightblue); end;
    egaLightblue: begin p.colors[gm]:=egaBlue; setcol(gm,egaBlue); end;
    egaBlue: begin p.colors[gm]:=0; setcol(gm,0); end;
   end;
  p.colors[c]:=egaWhite; setcol(c,egaWhite);

  AltBefore:=AltNow; AltNow:=testkey(sAlt);*/

  if (anymousekeypressed)  lmo=true;
  if (keypressed())  lmo=true;
  if ((! nomusic) && (ampgetmodulestatus != md_playing))  lmo=true;

 /* if (AltNow=True) and (AltBefore=False) then reverse:=not reverse;*/

  do_stuff();
  if (spinnum<200)  spinnum += 1;
 } while (!lmo);
}

void parse_cline()
{
    integer e;

 if (paramstr(1)!="jsb") 
 {
  output << "Not a standalone program." << NL; exit(255);
 }
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
 parse_cline();

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
    module=amploadmod("avalot2.mod",0);
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

 gd=3; gm=1; initgraph(gd,gm,"");
 assign(ff,"avalot.fnt"); reset(ff); ff >> f[0]; close(ff);
 assign(ff,"avitalic.fnt"); reset(ff); ff >> f[1]; close(ff);

 c=addr(credits()); c -= 1;

 fillchar(strip,sizeof(strip),'\0');
 reverse=false; spinnum=0; across=7; charnum=1; cfont=0;
 bigo2("1189"); /* 1189? 79? 2345? 1967? */
 bigo2go();

 if (! nomusic)  ampstopmodule;
 closegraph();
 return EXIT_SUCCESS;
}
