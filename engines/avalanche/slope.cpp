#include "ptoc.h"



struct joysetup {
            word xmid,ymid,xmin,ymin,xmax,ymax;
            byte centre; /* Size of centre in tenths */
};

const integer bug_twonames = 255;
const integer bug_pandl = 254;
const integer bug_weirdswitch = 253;
const integer bug_invalidini = 252;
const integer bug_notyesorno = 251;
const integer bug_weirdcard = 250;

byte fv;
char t;
string bugline;
boolean usingp,usingl;
boolean zoomy,numlockhold;
boolean doing_syntax;
joysetup js; boolean use_joy_a;

boolean filename_specified;
boolean soundfx;

string inihead,initail; /* For reading the .INI file. */

string filetoload;

boolean cl_override,keyboardclick,demo;

string slopeline;

word storage_seg,storage_ofs;

string argon;

longint soundcard,baseaddr,speed,irq,dma;

string strf(longint x)
{
    string q;

 string strf_result;
 str(x,q); strf_result=q;
 return strf_result;
}

void linebug(byte which)
{
 output << "AVALOT : ";
 switch (which) {
  case bug_twonames : output << "You may only specify ONE filename." << NL; break;
  case bug_pandl : output << "/p and /l cannot be used together." << NL; break;
  case bug_weirdswitch : output << "Unknown switch (\"" << bugline << 
                       "\"). Type AVALOT /? for a list of switches." << NL;
                       break;
  case bug_invalidini: output << "Invalid line in AVALOT.INI (\"" << bugline << "\")" << NL; break;
  case bug_notyesorno: output << "Error in AVALOT.INI: \"" << inihead << "\" must be \"yes\" or \"no.\"" << NL; break;
  case bug_weirdcard: output << "Unknown card: " << bugline << '.' << NL; break;
 }

 exit(which);
}

longint card(string x)
{
 longint card_result;
 if (x=="NONE")  card_result=0; else
 if (x=="SB")  card_result=1; else
 if (x=="SBPRO")  card_result=2; else
 if (x=="SB16")  card_result=3; else
 if (x=="PAS")  card_result=4; else
 if (x=="PASPLUS")  card_result=5; else
 if (x=="PAS16")  card_result=6; else
 if (x=="ARIA")  card_result=7; else
 if (x=="WINSOUND")  card_result=8; else
 if (x=="GRAVIS")  card_result=9; else
 if (x=="DACLPT")  card_result=10; else
 if (x=="STEREODACS")  card_result=11; else
 if (x=="STEREOON1")  card_result=12; else
 if (x=="SPEAKER")  card_result=13; else
  linebug(bug_weirdcard);
 return card_result;
}

void upstr(string& x)
{
    byte fv;

 for( fv=1; fv <= length(x); fv ++) x[fv]=upcase(x[fv]);
}

boolean yesno()
{
 boolean yesno_result;
 if (initail=="YES")  yesno_result=true; else
  if (initail=="NO")  yesno_result=false; else
   linebug(bug_notyesorno);
 return yesno_result;
}

word value(string x)
{
      const varying_string<15> hexdigits = "0123456789ABCDEF";
    word w; integer e;

   word value_result;
   if (x[1]=='$') 
   {
      w=0;
      for( e=2; e <= length(x); e ++)
      {
         w=w << 4;
         w += pos(upcase(x[e]),hexdigits)-1;
      }
      value_result=w;
   } else
   {
      val(x,w,e);
      if (e==0)  value_result=w; else value_result=0;
   }
   return value_result;
}

void ini_parse()
{
 upstr(inihead);
 upstr(initail);

 if (inihead=="QUIET")  soundfx=! yesno(); else
  if (inihead=="ZOOMYSTART")  zoomy=yesno(); else
   if (inihead=="NUMLOCKHOLD")  numlockhold=yesno(); else
    if (inihead=="LOADFIRST")  filetoload=initail; else
     if (inihead=="OVERRIDEEGACHECK")  cl_override=yesno(); else
      if (inihead=="KEYBOARDCLICK")  keyboardclick=yesno(); else
       if (inihead=="JOYTOP")  js.ymin=value(initail); else
        if (inihead=="JOYBOTTOM")  js.ymax=value(initail); else
         if (inihead=="JOYLEFT")  js.xmin=value(initail); else
          if (inihead=="JOYRIGHT")  js.xmax=value(initail); else
           if (inihead=="JOYMIDX")  js.xmid=value(initail); else
            if (inihead=="JOYMIDY")  js.ymid=value(initail); else
             if (inihead=="JOYCENTRINGFACTOR")  js.centre=value(initail); else
              if (inihead=="WHICHJOY")  use_joy_a=value(initail)==1; else
               if (inihead=="SOUNDCARD")  soundcard=card(initail); else
                if (inihead=="BASEADDRESS")  baseaddr=value(initail); else
                 if (inihead=="IRQ")  irq=value(initail); else
                  if (inihead=="DMA")  dma=value(initail); else
                   if (inihead=="SAMPLERATE")  speed=value(initail);
}

void strip_ini()
{
    byte fv;

 if (inihead=="")  return;

 /* Firstly, delete any comments. */
 fv=pos(";",inihead);
 if (fv>0)  Delete(inihead,fv,255);

 /* Lose the whitespace... */

 while (inihead[length(inihead)]==' ')  inihead[0] -= 1;
 while ((inihead!="") && (inihead[1]==' '))  Delete(inihead,1,1);

 /* It's possible that now we'll end up with a blank line. */

 if ((inihead=="") || (inihead[1]=='['))  return;

 fv=pos("=",inihead);

 if (fv==0) 
 {     /* No "="! Weird! */
  bugline=inihead;
  linebug(bug_invalidini);
 }

 initail=copy(inihead,fv+1,255);
 inihead[0]=chr(fv-1);
}

void load_ini()
{
    text ini;

 assign(ini,"AVALOT.INI");
 reset(ini);

 while (! eof(ini)) 
 {
  ini >> inihead >> NL;
  strip_ini();
  if (inihead!="")  ini_parse();
 }

 close(ini);
}

void parse(string x);
static string arg;

static string getarg(string otherwis)
{
 string getarg_result;
 if (arg=="")  getarg_result=otherwis; else getarg_result=arg;
 return getarg_result;
}

void parse(string x)

{
 switch (x[1]) {
  case '/':case '-': {
            arg=copy(x,3,255);
            switch (upcase(x[2])) {
             case '?': doing_syntax=true; break;
             case 'O': cl_override=true; break;
             case 'L': if (! usingp) 
                  {
                   usingl=true;
                   argon=getarg("avvy.log");
                  } else linebug(bug_pandl);
                  break;
             case 'P': if (! usingl) 
                  {
                   usingp=true;
                   argon=getarg("prn");
                  } else linebug(bug_pandl);
                  break;
             case 'Q': soundfx=false; break;
             case 'Z': zoomy=true; break;
             case 'K': keyboardclick=true; break;
             case 'D': demo=true; break;
             default: {
              bugline=x;
              linebug(bug_weirdswitch);
             }
            }
           }
           break;
  case '*': {
        inihead=copy(x,2,255);
        strip_ini();
        if (inihead!="")  ini_parse();
       }
       break;
  default: { /* filename */
        if (filename_specified) 
         linebug(bug_twonames);
        else
         filetoload=x;
        filename_specified=true;
       }
 }
}

void make_slopeline();

static char yn1(boolean b)
{
 char yn1_result;
 if (b)  yn1_result='y'; else yn1_result='n';
 return yn1_result;
}



static char pln()
{
 char pln_result;
 if ((! usingp) && (! usingl))  pln_result='n'; else
   if (usingp)  pln_result='p'; else
    if (usingl)  pln_result='l';
 return pln_result;
}

void make_slopeline()

{
 if (argon=="")  argon="nul";

  slopeline=string('1')+yn1(doing_syntax)+yn1(soundfx)+yn1(cl_override)+
   yn1(keyboardclick)+pln()+yn1(demo)+yn1(zoomy)+yn1(numlockhold)+
   yn1(use_joy_a)+
    ' '+strf(js.xmid)+
    ' '+strf(js.ymid)+
    ' '+strf(js.xmin)+
    ' '+strf(js.ymin)+
    ' '+strf(js.xmax)+
    ' '+strf(js.ymax)+
    ' '+strf(js.centre)+
    ' '+argon+' '+filetoload;
}

void store_slopeline()
{
 move(slopeline,mem[storage_seg*storage_ofs+3],sizeof(slopeline));
 move(js,mem[storage_seg*storage_ofs+300],sizeof(js));
 move(soundcard,mem[storage_seg*storage_ofs+5000],4);
 move(baseaddr,mem[storage_seg*storage_ofs+5004],4);
 move(irq,mem[storage_seg*storage_ofs+5008],4);
 move(dma,mem[storage_seg*storage_ofs+5012],4);
 move(speed,mem[storage_seg*storage_ofs+5016],4);
}

void get_storage_addr();

static void not_through_bootstrap()
{ output << "Not standalone!" << NL; exit(255); }

void get_storage_addr()
{
    integer e;

 if (paramstr(1)!="jsb")  not_through_bootstrap();
 val(paramstr(2),storage_seg,e); if (e!=0)  not_through_bootstrap();
 val(paramstr(3),storage_ofs,e); if (e!=0)  not_through_bootstrap();
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 get_storage_addr();

 filetoload=""; argon="";
 usingl=false;
 usingp=false;
 cl_override=false;
 soundfx=true;
 zoomy=false;
 numlockhold=false;
 filename_specified=false;
 keyboardclick=false;
 doing_syntax=false;
 soundcard=0; baseaddr=0; irq=0; dma=0;

 load_ini();

 for( fv=4; fv <= paramcount; fv ++)
  parse(paramstr(fv));

 make_slopeline();

 store_slopeline();
 return EXIT_SUCCESS;
}
