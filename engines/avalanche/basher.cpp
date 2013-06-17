

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 BASHER           Handles the keyboard. */

#define __basher_implementation__


 /* Note: this unit can record keystrokes, for the demo. If you want it
   to do this, define the o.c.c. RECORD. Setting gyro.demo to True will
   cause them to be played back. */

#include "basher.h"


#include "enhanced.h"
#include "lucerna.h"
#include "Graph.h"
#include "scrolls.h"
#include "acci.h"
#include "trip5.h"
#include "pingo.h"
#include "dropdown.h"
#include "logger.h"
#include "Sticks.h"
#include "enid.h"


boolean entering_filename;
byte left_margin;

void plottext()
{
      const array<0,2,byte> p = {{0,1,3}};
    byte x,y,n;

 x=0; if (mouse_near_text())  super_off();
 cursor_off();
 for( n=0; n <= 2; n ++)
  for( y=0; y <= 7; y ++)
  {
   for( x=1; x <= length(current); x ++)
    mem[0xa000*p[n]*pagetop+12882+y*80+x]=little[current[x]][y];
   fillchar(mem[0xa000*p[n]*pagetop+12883+y*80+x],77-x,'\0');
  }
 cursor_on();
 super_on();
}

void wipetext()
{
      const array<0,2,byte> n = {{0,1,3}};
    byte y,p;

 if (mouse_near_text())  super_off();
 cursor_off();
 for( y=0; y <= 7; y ++)
  for( p=0; p <= 2; p ++)
   fillchar(mem[0xa000*n[p]*pagetop+12883+y*80],77,'\0');
 quote=true; curpos=1; cursor_on();
 super_on();
}

/*procedure cursor;
begin
 if curflash=0 then
 begin
  if mouse_near_text then Super_Off;
  cursoron:=not cursoron;
  mem[$A000*13442+curpos]:=not mem[$A000*13442+curpos];
  mem[$A000*pagetop+13442+curpos]:=not mem[$A000*pagetop+13442+curpos];
  curflash:=17;
  Super_On;
 end else dec(curflash);
end;*/

void do_cursor()
{
 bytefield bf;
 byte fv;

 mem[0xac00*13442+curpos]=~ mem[0xac00*13442+curpos];
 {
  bf.x1=curpos+1; bf.x2=curpos+2; bf.y1=168; bf.y2=168;
 }
 for( fv=0; fv <= 1; fv ++) getset[fv].remember(bf);
}

void cursor_on()
{
 if (cursoron)  return;
 do_cursor(); cursoron=true;
}

void cursor_off()
{
 if (! cursoron)  return;
 do_cursor(); cursoron=false;
}


void get_demorec()
{
 demofile >> demo_rec;
 inchar=demo_rec.key;
   extd=demo_rec.extd;
   demo_rec.delay -= 1;
}

#ifdef RECORD
  void record_one()
  {
   demo_rec.delay=count;
   demo_rec.key=inchar;
   demo_rec.extd=extd;

   demofile << demo_rec;
   count=0;
  }
#endif
/*
procedure storeline(whatwhat:string);
var
 fv:byte;
 what:string[77];
 ok:boolean;

  function upline(x:string):string;
  var fv:byte; n:string[77];
  begin
   for fv:=1 to length(x) do n[fv]:=upcase(x[fv]);
   n[0]:=x[0]; upline:=n;
  end;
begin

 what:=upline(whatwhat); ok:=false;
 for fv:=1 to 20 do
  if what=upline(previous^[fv]) then
  begin { it already exists, in string "fv" }
   move(previous^[fv+1],previous^[fv],(20-fv)*78);
   previous^[20]:=whatwhat; ok:=true;
  end;
 if ok then exit;
 { it's not on the list, so add it }
 move(previous^[2],previous^[1],1482); { shove up }
 previous^[20]:=whatwhat;
end;
*/
void typein();

static char firstchar(string x) { char firstchar_result;
                                         firstchar_result=x[1]; return firstchar_result;
                                }


static void try_dd() /* This asks the Parsekey proc in Dropdown if it knows it. */
{
 parsekey(inchar,extd);
}

void typein()
{
    byte w;

 inkey();
 #ifdef RECORD record_one(); #endif

 switch (inchar) {
  case RANGE_15('\40','\56'):case '\60' ... '\337':case '\341' ... '\377': if (ddm_o.menunow) 
             {
              parsekey(inchar,extd);
             } else
             {
              if (length(current)<76) 
              {
               if ((inchar=='"') || (inchar=='`')) 
               {
                if (quote)  inchar='`'; else inchar='"';
                quote=! quote;    /* Quote- Unquote... */
               }
               insert(inchar,current,curpos);
               curpos += 1;
               plottext();
              } else blip();
             }
             break;
  case '\10': if (! ddm_o.menunow) 
      {
       if (curpos>left_margin) 
       {
        curpos -= 1;
        if (set::of('"','`', eos).has(current[curpos]))  quote=! quote;
        Delete(current,curpos,1);
        plottext();
       } else blip();
      }
      break;
  case '\0':case '\340': {
       switch (extd) {
              /* Function keys: */
        case cf1: callverb(vb_help); break; /* f1 - help (obviously) */
        case cf2: fxtoggle(); break; /* f2 - sound */
        case ccf2:case cf11: { clearwords(); callverb(vb_save); } break; /* ^f2 - save */
        case cf3: if (length(current)<length(/*previous^[20]*/last))  /* f3 - rec last */
             {
              current=current+copy( /*previous^[20]*/last,length(current)+1,255);
              curpos=length(current)+1;
              plottext();
             }
             break;
        case ccf3:case cf12: { clearwords(); callverb(vb_load); } break; /* ^f3 - load */
        case cf4: callverb(vb_restart); break; /* f4 - restart game */
        case cf5: {
                person=pardon; thing=pardon;
                callverb(firstchar(f5_does())); /* f5 - get up/ whatever */
             }
             break;
        case cf6: callverb(vb_pause); break; /* f6 - pause */
        case cf7: callverb(vb_open); break; /* f7 - open door */
        case cf8: callverb(vb_look); break; /* f8 - look */
        case cf9: callverb(vb_score); break; /* f9 - score */
        case ccf7: major_redraw(); break; /* ^f7 - open door */
        case cf10:case c_ax:case caf4: {
                      #ifdef RECORD
                       display("Hi. You have just finished recording. GIED.");
                       close(demofile); exit(0);
                      #endif
                       callverb(vb_quit); /* f10, alt-X, alt-f4 - quit */
                     }
                     break;
        case ccf5: back_to_bootstrap(2); break; /* ^f5 - Dos shell. */
        case csf10: callverb(vb_info); break; /* sf10 - version */

        case c_ab: callverb(vb_boss); break; /* alt-B */
        case c_ad: display("Wrong game!"); break; /* alt-D */
        case ccleft: if (curpos>left_margin) 
              { cursor_off(); curpos -= 1; cursor_on(); }
              break;                                         /* ^left */
        case ccright: if (curpos<=length(current))  /* ^right */
              { cursor_off(); curpos += 1; cursor_on(); }
              break;
        case cchome: { cursor_off(); curpos=1; cursor_on(); } break; /* ^home */
        case ccend: { cursor_off(); curpos=length(current)+1; cursor_on(); } break; /* ^end */
        case c_ar: oh=177; break; /* alt-R = repeater (re-chime) */
        case cup:case cdown:case cleft:case cright:case cpgup:case cpgdn:case chome:case cend: if (ddm_o.menunow)  try_dd();
              else tripkey(extd);
              break;              /* Up/Down/Left/Right/PgUp/PgDn */
        case cnum5: tripkey(extd); break; /* Numeric 5 */
        case cdel: if (! ddm_o.menunow) 
             {
              if (curpos<=length(current)) 
              {
               if (set::of('"','`', eos).has(current[curpos]))  quote=! quote;
               Delete(current,curpos,1);
               plottext();
              } else blip();
             }
             break;
        default: try_dd();
       }
      }
      break;
  case cescape:case '/': if (ddm_o.menunow)  /* Escape */
           { ddm_o.wipe(); } else
            if (entering_filename) 
              { normal_edit(); wipetext(); } else
                ddm_m.getcertain(ddm_o.menunum);
                break;
  case creturn: if (ddm_o.menunow)  try_dd(); /* Return */
       else {
        log_command(current);
        if (entering_filename) 
        {
          edna_save(copy(current,24,255));
          normal_edit(); wipetext();
        } else
        {
          if (current!="")  last=current;
          parse(); do_that();
          if (! entering_filename) 
          {
            current="";
            wipetext();
          }
        }
       }
       break;
  case '\11': callverb(vb_inv); break; /* Ctrl-I= inventory */
  case '\7': errorled(); break;
  case '\25': {
       current="";
       wipetext();
      }
      break;

  case '\27': { tr[1].xs=walk; newspeed(); } break;
  case '\22': { tr[1].xs=run;  newspeed(); } break;

  case '\2': bosskey(); break;
  case '\12': ctrl=cjoy; break; /* Joystick */
  case '\13': ctrl=ckey; break; /* Keyboard */
  case '\3': callverb(vb_quit); break; /* Ctrl-C= request to quit */

 }
 showrw();

 if (demo)  get_demorec();
}

/*        'x'..'z': begin setvisualpage(ord(extd)-63); write(#7); inkey; end;
        'Å': begin setvisualpage(0); write(#7); inkey; end;*/

void keyboard_link()
{
 state(defaultled); /* if defaultled=1 then on; { For the menus }*/
 joykeys(); /* Test joystick buttons- I know that's not keyboard. */

 if (demo) 
 {
  if (keypressede())  exit(0);
  if (demo_rec.delay>0) 
   demo_rec.delay -= 1;
  else typein();
  return;
 }

 #ifdef RECORD count += 1;   #endif

 if (! keypressede())  return;
 if (keyboardclick)  click();
 typein();
}

boolean demo_ready()
{
 boolean demo_ready_result;
 if (demo_rec.delay>0) 
 {
  slowdown();
  demo_rec.delay -= 1;
 }
 demo_ready_result=demo_rec.delay==0;
 return demo_ready_result;
}

void filename_edit()
{
 entering_filename=true;
 current="Filename\? (Esc=cancel):";
 left_margin=24; curpos=24;
 plottext();
}

void normal_edit()
{
 entering_filename=false;
 current="";
 left_margin=1; curpos=1;
}

class unit_basher_initialize {
  public: unit_basher_initialize();
};
static unit_basher_initialize basher_constructor;

unit_basher_initialize::unit_basher_initialize() {
/* new(previous);*/ last=""; normal_edit();

 if (demo) 
 {
  assign(demofile,"demo.avd");
  reset(demofile);
 }

 #ifdef RECORD
  count=0;
  assign(demofile,"demo.avd");
  rewrite(demofile);
 #endif

}
