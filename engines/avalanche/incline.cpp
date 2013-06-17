#include "ptoc.h"

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 INCLINE          The command-line parser. */

#define __incline_implementation__


#include "incline.h"

#include "gyro.h"
#include "logger.h"


byte fv;
char t;
string bugline;
boolean zoomy,numlockhold;

boolean filename_specified;

void syntax()
{
 assign(output,""); rewrite(output);
 output << NL;
 output << string("Lord Avalot d'Argent\n\n(c) ")+copyright+" Mark, Mike and Thomas Thurman." << NL;
 output << string("~~~~~~~~~~~~~~~~~~~~~\n\n")+vernum << NL;
 output << NL;
 output << "Syntax:" << NL;
 output << "\n/?\ndisplays this screen," << NL;
 output << "\n/O\noverrides EGA check," << NL;
 output << "\n/L<f>\nlogs progress to <f>, default AVVY.LOG," << NL;
 output << "\n/P<x>\nlogs with Epson codes to <x>, default PRN," << NL;
 output << "\n/Q\ncancels sound effects," << NL;
 output << "\n/Z\ngoes straight into the game." << NL;
 output << NL;
 output << "\n\n\n\n\n\n\n... Have fun!" << NL;
 exit(177);
}

void not_through_bootstrap()
{
 output << "Avalot must be loaded through the bootstrap." << NL;
 exit(0);
}

void get_storage_addr()
{
    integer e;

 val(paramstr(2),storage_seg,e); if (e!=0)  not_through_bootstrap();
 val(paramstr(3),storage_ofs,e); if (e!=0)  not_through_bootstrap();
 skellern=storage_ofs+1;
}

longint value(string x)
{
    longint q; integer e;

 longint value_result;
 val(x,q,e); value_result=q;
 return value_result;
}

void undo_js()
{
 {
  joysetup& with = js; 

  cxmin=with.xmid-(((with.xmid-with.xmin) / 10)*with.centre);
  cxmax=with.xmid+(((with.xmax-with.xmid) / 10)*with.centre);
  cymin=with.ymid-(((with.ymid-with.ymin) / 10)*with.centre);
  cymax=with.ymid+(((with.ymax-with.ymid) / 10)*with.centre);

/*  writeln(lst,'MID ',xmid,'x',ymid);
  writeln(lst,'MAX ',xmax,'x',ymax);
  writeln(lst,'MIN ',xmin,'x',ymin);
  writeln(lst,'CENTRE ',xmid);
  writeln(lst,cxmin);
  writeln(lst,cxmax);
  writeln(lst,cymin);
  writeln(lst,cymax);*/
 }
}

void check_slope_line();
static string slope;

static boolean yn1(byte where) { boolean yn1_result;
                                       yn1_result=slope[where]=='y'; return yn1_result;
                               }

void check_slope_line()
{
 slope=paramstr(4);

/* if slope='' then fillchar(slope,sizeof(slope),'n');*/

 if (slope[1]!='1')  not_through_bootstrap();

 if (yn1(2))  syntax();

 soundfx=yn1(3);
 cl_override=yn1(4);
 keyboardclick=yn1(5); /* 6 - see below */
 demo=yn1(7);
 zoomy=yn1(8);
 numlockhold=yn1(9);
 use_joy_a=yn1(10);

 {
    joysetup& with = js; 

    with.xmid=value(paramstr( 5));
    with.ymid=value(paramstr( 6));
    with.xmin=value(paramstr( 7));
    with.ymin=value(paramstr( 8));
    with.xmax=value(paramstr( 9));
    with.ymax=value(paramstr(10));
  with.centre=value(paramstr(11));

    undo_js();
 }

 switch (slope[6]) {
  case 'l': log_setup(paramstr(12),false); break;
  case 'p': log_setup(paramstr(12),true); break;
 }
}

void get_extra_data()
{
 if (! reloaded)  return;

 move(mem[storage_seg*storage_ofs+300],js,sizeof(js));

 undo_js();
}

class unit_incline_initialize {
  public: unit_incline_initialize();
};
static unit_incline_initialize incline_constructor;

unit_incline_initialize::unit_incline_initialize() {
/* writeln('Load code: ',paramstr(1));
 writeln('Seg & ofs: ',paramstr(2),':',paramstr(3));
 writeln('Slope line: ',paramstr(4));
 writeln('Log file: ',paramstr(5));
 writeln('File to load: ',paramstr(6));
 readln;*/

 filetoload=paramstr(13);
 filename_specified = filetoload != "";

 logging=false;

 if ((paramcount<3) ||
  ((paramstr(1)!="Go") && (paramstr(1)!="et")))  not_through_bootstrap();

 reloaded=paramstr(1)=="et";

 get_storage_addr();

 get_extra_data();

 check_slope_line();
}
