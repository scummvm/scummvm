

/*  $D-*/
/*
  ÛßÜ ÛßÜ ÜßßÜ  ßÛß Ûßß  Üß ßÛß      ÜßÛßÜ  ßÛß ÛÜ  Û ÜÛßß  ßÛß ÛßÜ Ûßß Û
  Ûß  ÛÛ  Û  Û Ü Û  Ûßß ßÜ   Û      Û  Û  Û  Û  Û ÛÜÛ  ßßßÜ  Û  ÛÛ  Ûßß Û
  ß   ß ß  ßß   ßß  ßßß   ß  ß      ß  ß  ß ßßß ß  ßß  ßßß   ß  ß ß ßßß ßßß

                 ACCIDENCE II     The parser. */
#define __acci_implementation__


/*$V-*/

#include "acci.h"

/*#include "Gyro.h"*/
/*#include "Lucerna.h"*/
/*#include "Scrolls.h"*/
/*#include "Pingo.h"*/
/*#include "Trip5.h"*/
/*#include "Visa.h"*/
/*#include "Enid.h"*/
/*#include "NimUnit.h"*/
/*#include "Timeout.h"*/
/*#include "Celer.h"*/
/*#include "Highs.h"*/
/*#include "Helper.h"*/
/*#include "Sequence.h"*/


byte fv;

string wordnum(string x);
static char whatsit; 
static word fv; 
static boolean gotcha;

static void checkword(string& x) /* Checks word "fv". */
{
 {
  vocab& with = words[fv]; 

  if ((with.w==x) || ((copy(with.w,1,length(x))==x) && ! gotcha))
        whatsit=chr(with.n);
  if (with.w==x)  gotcha=true;
 }
}

string wordnum(string x)

{
 string wordnum_result;
 if (x=="")  { wordnum_result=""; return wordnum_result; }
 whatsit=pardon; gotcha=false;
 for( fv=nowords; fv >= 1; fv --) checkword(x);
 wordnum_result=whatsit;
 return wordnum_result;
}

void replace(string old1,string new1)
{
    byte q;

 q=pos(old1,thats);
 while (q!=0) 
 {
  thats=copy(thats,1,q-1)+new1+copy(thats,q+length(old1),255);
  q=pos(old1,thats);
 }
}

/*procedure ninetydump;
var f:file; y:integer; bit:byte; a:byte absolute $A000:800;
begin
 off;
 assign(f,'avvydump.avd');
 rewrite(f,1);
 blockwrite(f,dna,177); { just anything }
 for bit:=0 to 3 do
 begin
  port[$3c4]:=2; port[$3ce]:=4; port[$3C5]:=1 shl bit; port[$3CF]:=bit;
  blockwrite(f,a,12080);
 end;
 close(f); on;
 display('Dumped.');
end;*/

string rank()
{
    byte fv;

 string rank_result;
 for( fv=1; fv <= 8; fv ++)
  if ((dna.score>=ranks[fv].score)
   && (dna.score<ranks[fv+1].score)) 
   {
    rank_result=ranks[fv].title;
    return rank_result;
   }
 return rank_result;
}

string totaltime()
{
      const real ticks_in_1_sec = (real)(65535)/3600;
 word h,m,s; varying_string<70> a;

  /* There are 65535 clock ticks in a second,
    1092.25 in a minute, and
    65535 in an hour. */
 string totaltime_result;
 h=trunc(dna.total_time/ticks_in_1_sec);  /* No. of seconds. */
 m=h % 3600; h=h / 3600;
 s=m % 60;   m=m / 60;

 a="You've been playing for ";

 if (h>0)  a=a+strf(h)+" hours, ";
 if ((m>0) || (h!=0))  a=a+strf(m)+" minutes and ";
 a=a+strf(s)+" seconds.";

 totaltime_result=a;
 return totaltime_result;
}

void cheatparse(string codes);
static word num; 
static integer e;

static void number(string& codes)
{
 val(codes,num,e);
}

void cheatparse(string codes)
{
    char cmd;integer se,sx,sy;

 if (~ cheat) 
 {     /* put them off the scent! */
  display("Have you gone dotty\??!");
  return;
 }
 cmd=upcase(codes[2]); Delete(codes,1,2);  /* strip header */
 display("\6Ninety: \22\4");
 switch (cmd) {
  case 'R': {
        number(codes); if (e!=0)  return;
        display(string("room swap to ")+codes+'.');
        fliproom(num,1);
       }
       break;
  case 'Z': { zonk; display("Zonk OK!"); } break;
  case 'W': { wobble; display("Ow my head!"); } break;
  case 'A': {
        tr[1].done;
        tr[1].init(1,true);
        dna.user_moves_avvy=true;
        alive=true;
        display("Reincat.");
       }
       break;
  case 'B': {
        sx=tr[1].x;
        sy=tr[1].y;
        se=tr[1].face;
        delavvy;
        number(codes);
        {
         void& with = tr[1]; 

         done;
         init(num,true);
         display(string("Become ")+codes+':'+"\r\r"+a.name+'\15'+a.comment);
         appear(sx,sy,se);
        }
       }
       break;
/*  'D': ninetydump;*/
  case 'G': play_nim; break;
  case '±': display("\232\25"); break;
  default: display("unknown code!");
 }
}

void punctustrip(string& x)          /* Strips punctuation from x. */
{
      const varying_string<32> punct = "~`!@#$%^&*()_+-={}[]:\"|;'\\,./<>?";
    byte fv,p;

 for( fv=1; fv <= 32; fv ++)
  do {
   p=pos(punct[fv-1],x);
   if (p==0)  flush(); /* <<< The first time I've ever used it! */
   Delete(x,p,1);
  } while (!false);
}

boolean do_pronouns();
static boolean ambiguous;

static void displaywhat(char ch, boolean animate) /* << it's an adjective! */
{
    byte ff; string z;

 if (ch==pardon) 
 {
  ambiguous=true;
  if (animate)  display("Whom?"); else display("What?");
 } else
 {
  if (animate)  display(string("{ ")+getname(ch)+" }");
   else
   {
     z=get_better(ch);
     if (z!="")  display(string("{ ")+z+" }");
   }
 }
}

boolean do_pronouns()
{
    byte fv;

 boolean do_pronouns_result;
 ambiguous=false;
 for( fv=1; fv <= length(thats); fv ++)
  switch (thats[fv]) {
   case '\310': {
          displaywhat(him,true);
          thats[fv]=him;
         }
         break;
   case '\311': {
          displaywhat(her,true);
          thats[fv]=her;
         }
         break;
   case '\312': {
          displaywhat(it,false);
          thats[fv]=it;
         }
         break;
  }
 do_pronouns_result=ambiguous;
 return do_pronouns_result;
}

void store_interrogation(byte interrogation);


static void lowercase()
{
    byte fv;

 for( fv=1; fv <= length(current); fv ++)
  if (set::of(range('A','Z'), eos).has(current[fv])) 
   current[fv] += 32;
}



static void propernouns()
{
    byte fv;

 lowercase();
 for( fv=2; fv <= length(current)-1; fv ++)
  if (current[fv]==' ') 
   current[fv+1]=upcase(current[fv+1]);
 current[1]=upcase(current[1]);
}



static void sayit() /* This makes Avalot say the response. */
{
    string x;

 x=current; x[1]=upcase(x[1]);
 display(string("\231")+x+'.'+'\2'+'\23'+'2');
}

void store_interrogation(byte interrogation)
{
    byte fv;


 if (current=="")  return;

 /* Strip current: */
 while ((current[1]==' ') && (current!=""))  Delete(current,1,1);
 while ((current[length(current)]==' ') && (current!=""))  current[0] -= 1;

 lose_timer(reason_cardiffsurvey); /* if you want to use any other timer,
  put this into the case statement. */
  switch (interrogation) {
   case 1: {
       lowercase(); sayit();
       like2drink=current;
       dna.cardiff_things=2;
      }
      break;
   case 2: {
       propernouns(); sayit();
       favourite_song=current;
       dna.cardiff_things=3;
      }
      break;
   case 3: {
       propernouns(); sayit();
       worst_place_on_earth=current;
       dna.cardiff_things=4;
      }
      break;
   case 4: {
       lowercase(); sayit();
       fillchar(spare_evening,sizeof(spare_evening),'\261');
       spare_evening=current;
       dixi('z',5); /* His closing statement... */
       tr[2].walkto(4); /* The end of the drawbridge */
       tr[2].vanishifstill=true;  /* Then go away! */
       magics[2].op=nix;
       dna.cardiff_things=5;
      }
      break;

   case 99: store_high(current); break;
  }
 if (interrogation<4)  cardiff_survey;
}

void clearwords()
{
 fillchar(realwords,sizeof(realwords),'\0');
}

void parse()
{
    byte n,fv,ff; string c,cc,thisword; varying_string<1> answer; boolean notfound;

 /* first parsing - word identification */

 thats=""; c=current+'\40'; n=1; polite=false;
 verb=pardon; thing=pardon; thing2=pardon; person=pardon;
 clearwords();
 if (current[1]=='.') 
 {     /* a cheat mode attempt */
  cheatparse(current); thats=nowt; return;
 }    /* not our department! Otherwise... */

 /* Are we being interrogated right now? */

 if (interrogation>0) 
 {
  store_interrogation(interrogation);
  weirdword=true;
  return;
 }

 cc=c; for( fv=1; fv <= length(c); fv ++) c[fv]=upcase(c[fv]);
 while (c!="") 
 {
   while ((c[1]=='\40') && (c!="")) 
     { Delete(c,1,1); Delete(cc,1,1); }
  thisword=copy(c,1,pos("\40",c)-1);
  realwords[n]=copy(cc,1,pos("\40",cc)-1);
  punctustrip(c);

  notfound=true;

  if (thisword!="") 
  {
   for( ff=1; ff <= 30; ff ++)
   {     /* Check Also, FIRST! */
    if (pos(string(',')+thisword,also[ff][0])>0) 
    {
     thats=thats+chr(99+ff);
     notfound=false;
    }
   }
  }

  if (notfound) 
  {
   answer=wordnum(thisword);
   if (answer==pardon) 
   {
    notfound=true;
    thats=thats+pardon;
   } else
    thats=thats+wordnum(thisword);
   n += 1;
  }
  Delete(c,1,pos("\40",c)); Delete(cc,1,pos("\40",cc));
 }

 if (pos("\376",thats)>0)  unknown=realwords[pos("\376",thats)]; else unknown=""; replace("\377",""); /* zap noise words */
 replace(string('\15')+'\342',"\1"); /* "look at" = "examine" */
 replace(string('\15')+'\344',"\1"); /* "look in" = "examine" */
 replace(string('\4')+'\343',"\21"); /* "get up" = "stand" */
 replace(string('\4')+'\347',"\21"); /* "get down" = "stand"... well, why not? */
 replace(string('\22')+'\344',"\2"); /* "go in" = "open [door]" */
 replace(string('\34')+'\345',"\375"); /* "P' off" is a swear word */
 replace(string('\4')+'\6',"\6"); /* "Take inventory" (remember Colossal Adventure?) */
 replace(string('\50')+'\350',"\25"); /* "put on" = "don" */
 replace(string('\4')+'\345',"\24"); /* "take off" = "doff" */

             /* Words that could mean more than one person */
 {
  if (room==r__nottspub)  replace("\314","\244"); /* Barman = Port */
   else replace("\314","\232");                   /* Barman = Malagauche */
  switch (room) {
   case r__aylesoffice: replace("\313","\243"); break;        /* Monk = Ayles */
   case r__musicroom: replace("\313","\246"); break;          /* Monk = Jacques */
   default: replace("\313","\242");                   /* Monk = Ibythneth */
  }
 }

 if (do_pronouns()) 
 {
  weirdword=true;
  thats=nowt;
  return;
 }

 /* second parsing - accidence */

 subject=""; subjnumber=0;   /* Find subject of conversation. */
 for( fv=1; fv <= 11; fv ++)
  if (set::of('`','\'', eos).has(realwords[fv][1])) 
  {
   subjnumber=ord(thats[fv]);
   thats[fv]=moved;
   flush(); /* Only the second time I've used that! */
  }
 if (subjnumber==0)   /* Still not found. */
  for( fv=1; fv <= 10; fv ++)
   if (thats[fv]=='\374')  /* the word is "about", or something similar */
   {
    subjnumber=ord(thats[fv+1]);
    thats[fv+1]='\0';
    flush(); /* ...Third! */
   }
 if (subjnumber==0)   /* STILL not found! Must be the word after "say". */
  for( fv=1; fv <= 10; fv ++)
   if ((thats[fv]=='\7') && ! (set::of('\0',range('\341','\345'), eos).has(thats[fv+1]))) 
   {     /* SAY not followed by a preposition */
    subjnumber=ord(thats[fv+1]);
    thats[fv+1]='\0';
    flush(); /* ...Fourth! */
   }

 for( fv=length(thats); fv >= 1; fv --) /* Reverse order- so first'll be used */
  switch (thats[fv]) {
   case '\1' ... '\61':case '\375':case '\371': verb=thats[fv]; break;
   case '\62' ... '\225': { thing2=thing; thing=thats[fv]; } break;
   case '\226' ... '\307': person=thats[fv]; break;
   case '\373': polite=true; break;
  }

 if ((unknown!="") && !
  (set::of(vb_exam,vb_talk,vb_save,vb_load,vb_dir, eos).has(verb))) 
 {
  display(string("Sorry, but I have no idea what `")+unknown+
    "\" means. Can you rephrase it?");
  weirdword=true;
 } else weirdword=false;
 if (thats=="")  thats=nowt;

 if (thing!=pardon)  it=thing;
 if (person!=pardon) 
 {
  if (person<'\257')  him=person; else her=person;
 }
}

void examobj()     /* Examine a standard object-thing */
{
 if (thing!=thinks)  thinkabout(thing,a_thing);
  switch (thing) {
   case wine : switch (winestate) {
         /* 4 is perfect wine. 0 is not holding the wine. */
           case 1: dixi('t',1); break; /* Normal examine wine scroll */
           case 2: dixi('d',6); break; /* Bad wine */
           case 3: dixi('d',7); break; /* Vinegar */
          }
          break;
   case onion: if (rotten_onion) 
           dixi('q',21); /* Yucky onion. */
          else
           dixi('t',18);
           break;        /* Normal onion scroll */
  default:
   dixi('t',ord(thing)); /* <<< Ordinarily */
  }
}

boolean personshere()         /* Person equivalent of "holding" */
{
 boolean personshere_result;
 if ((person==pardon) || (person=='\0')
   || (whereis[person]==dna.room))  personshere_result=true;
 else {
  if (person<'\257')  display("H\4"); else display("Sh\4");
  display("e isn't around at the moment.");
  personshere_result=false;
 }
 return personshere_result;
}

void exampers()
{
 if (personshere()) 
 {
  if (thing!=thinks)  thinkabout(person,a_person);
  person -= 149;
   switch (person) { /* Special cases */
    case '\13': if (wonnim) 
         {
          dixi('Q',8); /* "I'm Not Playing!" */
          return;
         }
         break;
    case '\11': if (lustie_is_asleep) 
        { dixi('Q',65);     /* He's asleep. (65! Wow!) */ return; }
        break;
   }
   /* Otherwise... */ dixi('p',ord(person));
 }    /* And afterwards... */
 switch (person) {
  case '\16': if (~ dna.ayles_is_awake)  dixi('Q',13); break; /* u.f.s.? */
 }
}

boolean holding()
{
 boolean holding_result;
 if (set::of(range('\63','\143'), eos).has(thing))  { holding_result=true; return holding_result; } /* Also */
 holding_result=false;
 if (thing>'\144')  display("Be reasonable!"); else
  if (~ dna.obj[thing])      /* verbs that need "thing" to be in the inventory */
   display("You're not holding it, Avvy."); else
    holding_result=true;
 return holding_result;
}

void examine();

static void special(boolean before)
{
 switch (dna.room) {
  case r__yours: switch (thing) {
             case '\66': if (before)  show_one(5); else show_one(6); break;
            }
            break;
 }
}

void examine()
{
 /* Examine. EITHER it's an object OR it's an Also OR it's a person OR
    it's something else. */
 if ((person==pardon) && (thing!=pardon)) 
 {
  if (holding()) 
   switch (thing) { /* remember it's been Slipped- ie subtract 49 */
       case '\1' ... '\61' : examobj(); break; /* Standard object */
     case '\62' ... '\144' : {
                  special(true);
                  display(also[ord(thing)-50][1]); /* Also thing */
                  special(false);
                 }
                 break;
   }
 } else
  if (person!=pardon)  exampers();
   else display("It's just as it looks on the picture.");  /* don't know- guess */
}

void inv()     /* the time-honoured command... */
{
    char fv; byte q;

 q=0; display(string("You're carrying ")+'\4');
 {
  for( fv='\1'; fv <= numobjs; fv ++)
   if (obj[fv]) 
   {
    q += 1; if (q==carrying)  display(string("and ")+'\4');
    display(get_better(fv)+'\4');
    if (fv==wearing)  display(string(", which you're wearing")+'\4');
     if (q<carrying)  display(string(", ")+'\4');
   }
  if (wearing==nowt)  display("...\r\r...and you're stark naked!"); else
   display('.');
 }
}

void swallow()     /* Eat something. */
{
 switch (thing) {
  case wine: switch (dna.winestate) {
         /* 4 is perfect */
          case 1: {
              if (dna.teetotal)  { dixi('D',6); return; }
              dixi('U',1); wobble; dixi('U',2);
              dna.obj[wine]=false; objectlist;
              have_a_drink();
             }
             break;
          case 2:case 3: dixi('d',8); break; /* You can't drink it! */
         }
         break;
  case potion: { background(4); dixi('U',3); gameover; background(0); } break;
  case ink: dixi('U',4); break;
  case chastity: dixi('U',5); break;
  case mushroom: {
             dixi('U',6);
             gameover;
            }
            break;
  case onion: if (dna.rotten_onion)  dixi('U',11);
         else {
          dixi('U',8);
          dna.obj[onion]=false;
          objectlist;
         }
         break;
  default:
    if (set::of(r__argentpub,r__nottspub, eos).has(dna.room)) 
      display("Try BUYing things before you drink them!");
    else
      display("The taste of it makes you retch!");
         /* Constant- leave this one */
 }
}

void others()
 /* This lists the other people in the room. */
{
 char fv;
 byte num_people,this_person,here;


 num_people=0;
 this_person=0;
 here=dna.room;

 for( fv='\227'; fv <= '\262'; fv ++) /* Start at 151 so we don't list Avvy himself! */
  if (whereis[fv]==here) 
   num_people += 1;

 /* If nobody's here, we can cut out straight away. */

 if (num_people==0)  return; /* Leave the procedure. */

 for( fv='\227'; fv <= '\262'; fv ++)
  if (whereis[fv]==here) 
  {
   this_person += 1;
   if (this_person==1)   /* First on the list. */
    display(getname(fv)+'\4'); else
    if (this_person<num_people)    /* The middle... */
     display(string(", ")+getname(fv)+'\4'); else
      display(string(" and ")+getname(fv)+'\4'); /* The end. */
  }

 if (num_people==1)  display(" is\4"); else display(" are\4");

 display(" here."); /* End and display it. */
}

void lookaround()
/* This is called when you say "look." */
{
 display(also[0][1]);
 switch (dna.room) {
  case r__spludwicks: if (dna.avaricius_talk>0)  dixi('q',23); else others(); break;
  case r__robins: {
              if (dna.tied_up)  dixi('q',38);
              if (dna.mushroom_growing)  dixi('q',55);
             }
             break;
  case r__insidecardiffcastle: if (~ dna.taken_pen)  dixi('q',49); break;
  case r__lustiesroom: if (dna.lustie_is_asleep)  dixi('q',65); break;
  case r__catacombs: switch (dna.cat_y*256+dna.cat_x) {
                 case 258 : dixi('q',80); break; /* Inside art gallery */
                 case 514 : dixi('q',81); break; /* Outside ditto */
                 case 260 : dixi('q',82); break; /* Outside Geida's room. */
                }
                break;
  default: others();
 }
}

void opendoor()     /* so whaddya THINK this does?! */
{
    byte fv;

 switch (dna.room) {   /* Special cases. */
   case r__yours: if (infield(2)) 
             {     /* Opening the box. */
              thing='\66'; /* The box. */ person=pardon;
              examine();
              return;
             }
             break;
   case r__spludwicks: if (thing=='\75')  {
                    dixi('q',85);
                    return;
                  }
                  break;
 }


 if ((~ dna.user_moves_avvy) & (dna.room!=r__lusties))
        return; /* No doors can open if you can't move Avvy. */
 for( fv=9; fv <= 15; fv ++)
  if (infield(fv)) 
  {
   { void& with = portals[fv]; 
    switch (op) {
     case exclaim: { tr[1].bounce; dixi('x',data); } break;
     case transport: fliproom(hi(data),lo(data)); break;
     case unfinished: {
                  tr[1].bounce;
                  display("\7Sorry.\3\rThis place is not available yet!");
                 }
                 break;
     case special: call_special(data); break;
     case mopendoor: open_the_door(hi(data),lo(data),fv); break;
    }}
    return;
   }
 if (dna.room==r__map) 
  display(string("Avvy, you can complete the whole game without ever going ")+
                 "to anywhere other than Argent, Birmingham, Cardiff, "+
                 "Nottingham and Norwich.");
 else display("Door? What door?");
}

void putproc();


static void silly()
{
 display("Don't be silly!");
}

void putproc()     /* Called when you call vb_put. */
{
    char temp;


 if (! holding())  return;
 thing2 -= 49;   /* Slip the second object */
 temp=thing; thing=thing2; if (! holding())  return;
 thing=temp;

 /* Thing is the thing which you're putting in. Thing2 is where you're
   putting it. */
             /* Convenience thing. */
  switch (thing2) {
   case wine: if (thing==onion) 
         {
          if (dna.rotten_onion) 
          display(string("That's a bit like shutting the stable door after the ")+
                   "horse has bolted!");
          else {     /* Put onion into wine? */
           if (dna.winestate!=3) 
            display("\6Oignon au vin\22 is a bit too strong for your tastes!");
           else {     /* Put onion into vinegar! Yes! */
            onion_in_vinegar=true;
            points(7);
            dixi('u',9);
           }
          }
         } else silly();
         break;

    case '\66': if (room==1)  /* Put something into the box. */
         {
          if (box_contents!=nowt) 
           display(string("There's something in the box already, Avvy. Try taking")+
            " that out first.");
          else
           switch (thing) {
            case money: display("You'd better keep some ready cash on you!"); break;
            case bell: display("That's a silly place to keep a bell."); break;
            case bodkin: display("But you might need it!"); break;
            case onion: display("Just give it to Spludwick, Avvy!"); break;
            default:
            {     /* Put the object into the box... */
             if (wearing==thing) 
              display(string("You'd better take ")+get_better(thing)+" off first!");
             else
             {
              show_one(5); /* Open box. */
              box_contents=thing;
              dna.obj[thing]=false;
              objectlist;
              display("OK, it's in the box.");
              show_one(6); /* Shut box. */
             }
            }
           }
         } else silly();
         break;

   default: silly();
  }
}

boolean give2spludwick();

 /* The result of this fn is whether or not he says "Hey, thanks!" */
static void not_in_order()
{
 display(string("Sorry, I need the ingredients in the right order for this potion.")+
  " What I need next is "+
   get_better(spludwick_order[dna.given2spludwick])+".\232\2");
}



static void go_to_cauldron()
{
   tr[2].call_eachstep=false;  /* Stops Geida_Procs. */
   set_up_timer(1,procspludwick_goes_to_cauldron,reason_spludwalk);
   tr[2].walkto(2);
}

boolean give2spludwick()

{
 boolean give2spludwick_result;
 {

  give2spludwick_result=false;

  if (spludwick_order[given2spludwick]!=thing) 
  {
   not_in_order();
   return give2spludwick_result;
  }

  switch (thing) {
   case onion:
    {
     obj[onion]=false;
     if (rotten_onion) 
      dixi('q',22);
     else {
      given2spludwick += 1;
      dixi('q',20);
      go_to_cauldron();
      points(3);
     }
     objectlist;
    }
    break;
   case ink: {
         obj[ink]=false;
         objectlist;
         given2spludwick += 1;
         dixi('q',24);
         go_to_cauldron();
         points(3);
        }
        break;
   case mushroom: {
              obj[mushroom]=false;
              dixi('q',25);
              points(5);
              given2spludwick += 1;
              go_to_cauldron();
              obj[potion]=true;
              objectlist;
             }
             break;
   default: give2spludwick_result=true;
  }
 }
 return give2spludwick_result;
}

void have_a_drink()
{
 {
  alcohol += 1;
  if (alcohol==5) 
  {
   obj[key]=true;  /* Get the key. */
   teetotal=true;
   avvy_is_awake=false;
   avvy_in_bed=true;
   objectlist;
   dusk;
   hang_around_for_a_while;
   fliproom(1,1);
   background(14);
   new_game_for_trippancy; /* Not really */
  }
 }
}

void cardiff_climbing()
{
 if (dna.standing_on_dais) 
 {     /* Clamber up. */
  display("You climb down, back onto the floor.");
  dna.standing_on_dais=false;
  apped(1,3);
 } else
 {     /* Clamber down. */
  if (infield(1)) 
  {
    display("You clamber up onto the dais.");
    dna.standing_on_dais=true;
    apped(1,2);
  } else
    display("Get a bit closer, Avvy.");
 }
}

void stand_up();

  /* Called when you ask Avvy to stand. */
static void already()
{
 display("You're already standing!");
}

void stand_up()
{
  switch (dna.room) {
   case r__yours: /* Avvy isn't asleep. */
              if (avvy_in_bed)    /* But he's in bed. */
              {
               if (teetotal) 
               {
                dixi('d',12);
                background(0);
                dixi('d',14);
               }
               tr[1].visible=true;
               user_moves_avvy=true;
               apped(1,2);
               dna.rw=left;
               show_one(4); /* Picture of empty pillow. */
               points(1);
               avvy_in_bed=false;
               lose_timer(reason_arkata_shouts);
              } else already();
              break;

    case r__insidecardiffcastle: cardiff_climbing(); break;

    case r__nottspub: if (sitting_in_pub)  {
                  show_one(4); /* Not sitting down. */
                  tr[1].visible=true;  /* But standing up. */
                  apped(1,4); /* And walking away. */
                  sitting_in_pub=false;  /* Really not sitting down. */
                  user_moves_avvy=true;  /* And ambulant. */
                 } else already();
                 break;
   default: already();
  }
}

void getproc(char thing)
{
  switch (room) {
   case r__yours:
     if (infield(2)) 
     {
      if (box_contents==thing) 
      {
       show_one(5);
       display("OK, I've got it.");
       obj[thing]=true; objectlist;
       box_contents=nowt;
       show_one(6);
      } else
       display(string("I can't see ")+get_better(thing)+" in the box.");
     } else dixi('q',57);
     break;
  case r__insidecardiffcastle:
    switch (thing) {
     case pen:
     {
      if (infield(2)) 
      {     /* Standing on the dais. */

       if (dna.taken_pen) 
        display("It's not there, Avvy.");
       else
       {     /* OK: we're taking the pen, and it's there. */
        show_one(4); /* No pen there now. */
        call_special(3); /* Zap! */
        dna.taken_pen=true;
        dna.obj[pen]=true;
        objectlist;
        display("Taken.");
       }
      } else if (dna.standing_on_dais)  dixi('q',53); else dixi('q',51);
     }
     break;
      case bolt: dixi('q',52); break;
     default: dixi('q',57);
    }
    break;
  case r__robins: if ((thing==mushroom) & (infield(1)) & (dna.mushroom_growing))
                  {
              show_one(3);
              display("Got it!");
              dna.mushroom_growing=false;
              dna.taken_mushroom=true;
              dna.obj[mushroom]=true;
              objectlist;
              points(3);
             } else dixi('q',57);
             break;
  default: dixi('q',57);
 }
}

void give_geida_the_lute()
{
 {
  if (room!=r__lustiesroom) 
  {
   display("Not yet. Try later!\232\2");
   return;
  }
  dna.obj[lute]=false;
  objectlist;
  dixi('q',64); /* She plays it. */

   /* And the rest has been moved to Timeout... under give_lute_to_Geida. */

  set_up_timer(1,procgive_lute_to_geida,reason_geida_sings);
  back_to_bootstrap(4);
 }
}

void play_harp()
{
 if (infield(7)) 
  musical_scroll;
 else display("Get a bit closer to it, Avvy!");
}

void winsequence()
{
 dixi('q',78);
 first_show(7); then_show(8); then_show(9);
 start_to_close;
 set_up_timer(30,procwinning,reason_winning);
}

void person_speaks()
{
    boolean found; byte fv; char cfv;


  if ((person==pardon) || (person=='\0')) 
  {
   if ((him==pardon) || (whereis[him]!=dna.room))  person=her;
    else person=him;
  }

  if (whereis[person]!=dna.room) 
  {
   display("\231\4"); /* Avvy himself! */
   return;
  }

  found=false;  /* The person we're looking for's code is in Person. */

  for( fv=1; fv <= numtr; fv ++)
   if (tr[fv].quick & (chr(tr[fv].a.accinum+149)==person)) 
   {
    display(string('\23')+chr(fv+48)+'\4');
    found=true;
   }

  if (! found) 
    for( fv=10; fv <= 25; fv ++)
     { void& with = quasipeds[fv]; 
     if ((who==person) && (room==dna.room)) 
     {
      display(string('\23')+chr(fv+55)+'\4');
     }}
}

void do_that();

static void heythanks()
{
  person_speaks();
  display("Hey, thanks!\2(But now, you've lost it!)");
  dna.obj[thing]=false;
}

void do_that()
{
      const array<'\63','\72',varying_string<6> > booze = {{"Bitter","GIED","Whisky","Cider","","","","Mead"}};
    byte fv,ff; integer sx,sy; boolean ok;


 if (thats==nowt)  { thats=""; return; }
 if (weirdword)  return;
 if (thing<'\310')  thing -= 49;   /* "Slip" */

 if ((~ alive) &
  ! (set::of(vb_load,vb_save,vb_quit,vb_info,vb_help,vb_larrypass,
     vb_phaon,vb_boss,vb_cheat,vb_restart,vb_dir,vb_score,
     vb_highscores,vb_smartalec, eos).has(verb)))
       {
        display(string("You're dead, so don't talk. What are you, a ghost ")+
         "or something? Try restarting, or restoring a saved game!"); return;
       }

 if ((~ dna.avvy_is_awake) &
  ! (set::of(vb_load,vb_save,vb_quit,vb_info,vb_help,vb_larrypass,
     vb_phaon,vb_boss,vb_cheat,vb_restart,vb_dir,vb_die,vb_score,
     vb_highscores,vb_smartalec,vb_expletive,vb_wake, eos).has(verb)))
       {
        display("Talking in your sleep? Try waking up!"); return;
       }


 switch (verb) {
  case vb_exam: examine(); break;
  case vb_open: opendoor(); break;
  case vb_pause: display(string("Game paused.")+'\3'+'\15'+'\15'+"Press Enter, Esc, or click "+
             "the mouse on the `O.K.\" box to continue.");
             break;
  case vb_get: {
           if (thing!=pardon) 
           {     /* Legitimate try to pick something up. */
            if (dna.carrying>=maxobjs)  display("You can't carry any more!");
            else getproc(thing);

           } else
           {     /* Not... ditto. */
            if (person!=pardon) 
             display("You can't sweep folk off their feet!"); else
            display("I assure you, you don't need it.");
           }
          }
          break;
  case vb_drop: display(string("Two years ago you dropped a florin in the street. Three days ")+
       "later it was gone! So now you never leave ANYTHING lying around. OK?");
       break;
/*       begin dna.obj[thing]:=false; objectlist; end;*/
  case vb_inv: inv(); break;
  case vb_talk:  if (person==pardon) 
            {
              if (subjnumber==99)   /* They typed "say password". */
                display("Yes, but what \6is\22 the password?");
              else if (set::of(range(1,49),253,249, eos).has(subjnumber)) 
              {
                Delete(thats,1,1);
                move(realwords[2],realwords[1],sizeof(realwords)-sizeof(realwords[1]));
                verb=chr(subjnumber);
                do_that(); return;
              } else
              {
                person=chr(subjnumber); subjnumber=0;
                if (set::of(pardon,'\0', eos).has(person))  display("Talk to whom?");
                 else if (personshere())  talkto(ord(person));
              }
            } else if (person==pardon)  display("Talk to whom?");
            else if (personshere())  talkto(ord(person));
            break;

  case vb_give: if (holding()) 
           {
            if (person==pardon)  display("Give to whom?"); else
            if (personshere()) 
            {
             switch (thing) {
              case money : display("You can't bring yourself to give away your moneybag."); break;
              case bodkin:case bell:case clothes:case habit :
                    display("Don't give it away, it might be useful!");
                    break;
              default: switch (person) {
                      case pcrapulus: switch (thing) {
                                 case wine: {
                                        display("Crapulus grabs the wine and gulps it down.");
                                        dna.obj[wine]=false;
                                       }
                                       break;
                                 default: heythanks();
                                }
                                break;
                     case pcwytalot: if (set::of(crossbow,bolt, eos).has(thing)) 
                                 display(string("You might be able to influence ")+
                                 "Cwytalot more if you used it!");
                                else heythanks();
                                break;
                     case pspludwick: if (give2spludwick())  heythanks(); break;
                     case pibythneth: if (thing==badge) 
                                 {
                                  dixi('q',32); /* Thanks! Wow! */
                                  points(3);
                                  dna.obj[badge]=false;
                                  dna.obj[habit]=true;
                                  dna.givenbadgetoiby=true;
                                  show_one(8); show_one(9);
                                 } else heythanks();
                                 break;
                     case payles: if (dna.ayles_is_awake) 
                             {
                               if (thing==pen) 
                               {
                                dna.obj[pen]=false;
                                dixi('q',54);
                                dna.obj[ink]=true;
                                dna.given_pen_to_ayles=true;
                                objectlist;
                                points(2);
                               } else heythanks();
                             } else
                               display("But he's asleep!");
                               break;
                     case pgeida: switch (thing) {
                              case potion : {
                                        dna.obj[potion]=false;
                                        dixi('u',16); /* She drinks it. */
                                        points(2);
                                        dna.geida_given_potion=true;
                                        objectlist;
                                       }
                                       break;
                              case lute: give_geida_the_lute(); break;
                              default: heythanks();
                             }
                             break;
                     case parkata: switch (thing) {
                              case potion: if (dna.geida_given_potion) 
                                       winsequence();
                                      else dixi('q',77);
                                      break;             /* That Geida woman! */
                              default: heythanks();
                             }
                             break;
                    default: heythanks();
                   }
             }
            }
            objectlist; /* Just in case... */
           }
           break;

  case vb_eat:case vb_drink: if (holding())  swallow(); break;
  case vb_load: edna_load(realwords[2]); break;
  case vb_save: if (alive)  edna_save(realwords[2]);
           else display("It's a bit late now to save your game!");
           break;
  case vb_pay: display("No money need change hands."); break;
  case vb_look: lookaround(); break;
  case vb_break: display("Vandalism is prohibited within this game!"); break;
  case vb_quit: { /* quit */
            if (demo) 
            {
             dixi('q',31);
             close(demofile);
             exit(0); /* Change this later!!! */
            }
        if (! polite)  display("How about a `please\", Avvy?"); else
         if (ask("\23C\26Do you really want to quit?"))  lmo=true;
       }
       break;
  case vb_go: display("Just use the arrow keys to walk there."); break;
  case vb_info: {
            aboutscroll=true;
/*            display('Thorsoft of Letchworth presents:'+^c+^m+^m+
             'The medi‘val descendant of'+^m+
             'Denarius Avaricius Sextus'+^m+'in:'+
             ^m+^m+'LORD AVALOT D''ARGENT'+
             ^m+'version '+vernum+^m+^m+'Copyright ï '
             +copyright+', Mark, Mike and Thomas Thurman.');*/
             display(string("\r\r\r\r\r\r\r")+"LORD AVALOT D'ARGENT"+"\3\r"+
              "The medi‘val descendant of"+'\15'+
              "Denarius Avaricius Sextus"+
              '\15'+'\15'+"version "+vernum+'\15'+'\15'+"Copyright ï "
              +copyright+", Mark, Mike and Thomas Thurman."+'\23'+'Y'+'\26');
             aboutscroll=false;
           }
           break;
  case vb_undress: if (dna.wearing==nowt)  display("You're already stark naked!");
               else
            if (dna.avvys_in_the_cupboard) 
            {
             display(string("You take off ")+get_better(dna.wearing)+'.');
             dna.wearing=nowt; objectlist;
            } else
            display("Hadn't you better find somewhere more private, Avvy?");
            break;
  case vb_wear: if (holding()) 
       {     /* wear something */
        switch (thing) {
         case chastity: display("Hey, what kind of a weirdo are you\??!"); break;
         case clothes:case habit: { /* Change this! */
                         if (dna.wearing!=nowt) 
                         {
                          if (dna.wearing==thing) 
                            display("You're already wearing that.");
                          else
                            display(string("You'll be rather warm wearing two ")+
                           "sets of clothes!");
                          return;
                         } else
                         dna.wearing=thing; objectlist;
                         if (thing==habit)  fv=3; else fv=0;
                         { void& with = tr[1]; 
                          if (whichsprite!=fv) 
                          {
                           sx=tr[1].x; sy=tr[1].y;
                           done;
                           init(fv,true);
                           appear(sx,sy,left);
                           tr[1].visible=false;
                          }}
                        }
                        break;
         default: display(what);
        }
       }
       break;
  case vb_play: if (thing==pardon) 
            switch (dna.room) { /* They just typed "play"... */
             case r__argentpub: play_nim; break; /* ...in the pub, => play Nim. */
             case r__musicroom: play_harp(); break;
            }
           else if (holding()) 
           {
            switch (thing) {
             case lute : {
                     dixi('U',7);
                     if (whereis[pcwytalot]==dna.room)  dixi('U',10);
                     if (whereis[pdulustie]==dna.room)  dixi('U',15);
                    }
                    break;
             case '\64' : if (dna.room==r__musicroom)  play_harp();
                    else display(what);
                    break;
             case '\67' : if (dna.room==r__argentpub)  play_nim;
                    else display(what);
                    break;
             default: display(what);
            }
           }
           break;
  case vb_ring: if (holding()) 
       {
        if (thing==bell) 
        {
         display("Ding, dong, ding, dong, ding, dong, ding, dong...");
         if ((dna.ringing_bells) & (flagset('B'))) 
          display("(Are you trying to join in, Avvy\?\?!)");
        } else display(what);
       }
       break;
  case vb_help: boot_help; break;
  case vb_larrypass: display("Wrong game!"); break;
  case vb_phaon: display("Hello, Phaon!"); break;
  case vb_boss: bosskey; break;
  case vb_pee: if (flagset('P')) 
          {
           display("Hmm, I don't think anyone will notice...");
           set_up_timer(4,procurinate,reason_gototoilet);
          } else display("It would be \6VERY\22 unwise to do that here, Avvy!");
          break;
  case vb_cheat: {
             display(string('\6')+"Cheat mode now enabled.");
             cheat=true;
            }
            break;
  case vb_magic: if (dna.avaricius_talk>0) 
             dixi('q',19);
            else {
             if ((dna.room==12) & (infield(2))) 
             {     /* Avaricius appears! */
              dixi('q',17);
              if (whereis['\227']==12) 
               dixi('q',18);
              else
              {
               tr[2].init(1,false); /* Avaricius */
               apped(2,4);
               tr[2].walkto(5);
               tr[2].call_eachstep=true;
               tr[2].eachstep=procback_and_forth;
               dna.avaricius_talk=14;
               set_up_timer(177,procavaricius_talks,reason_avariciustalks);
              }
             } else display("Nothing appears to happen...");
            }
            break;
  case vb_smartalec: display("Listen, smart alec, that was just rhetoric."); break;
  case vb_expletive: {
         switch (swore) {
          case 0: display(string("Avvy! Do you mind? There might be kids playing!\r\r")+
              "(I shouldn't say it again, if I were you!)");
              break;
          case 1: display(string("You hear a distant rumble of thunder. Must you always ")+
              "do things I tell you not to?\r\rDon't do it again!");
              break;
         default:
          {
           zonk;
           display(string("A crack of lightning shoots from the sky, ")+
            "and fries you.\r\r(`Such is the anger of the gods, Avvy!\")");
           gameover;
          }
         }
         swore += 1;
        }
        break;
  case vb_listen: if ((dna.ringing_bells) & (flagset('B'))) 
              display(string("All other noise is drowned out by the ringing of ")+
                       "the bells.");
             else
              if (listen=="") 
               display("You can't hear anything much at the moment, Avvy.");
                else display(listen);
                break;
  case vb_buy: {
           /* What are they trying to buy? */
           switch (dna.room) {
            case r__argentpub: if (infield(6)) 
                {     /* We're in a pub, and near the bar. */
                 switch (thing) {
                  case '\63':case '\65':case '\66':case '\72': { /* Beer, whisky, cider or mead */
                            if (dna.malagauche==177)   /* Already getting us one. */
                              { dixi('D',15); return; }
                            if (dna.teetotal)  { dixi('D',6); return; }
                            if (dna.alcohol==0)  points(3);
                            show_one(12);
                            display(booze[thing]+", please.\231\2");
                            dna.drinking=thing;

                            show_one(10);
                            dna.malagauche=177;
                            set_up_timer(27,procbuydrinks,reason_drinks);
                           }
                           break;
                  case '\64': examine(); break; /* We have a right one here- buy Pepsi??! */
                  case wine: if (dna.obj[wine])  /* We've already got the wine! */
                          dixi('D',2); /* 1 bottle's shufishent! */
                         else {
                          if (dna.malagauche==177)   /* Already getting us one. */
                            { dixi('D',15); return; }
                          if (dna.carrying>=maxobjs) 
                             { display("Your hands are full."); return; }
                          show_one(12); display("Wine, please.\231\2");
                          if (dna.alcohol==0)  points(3);
                          show_one(10);
                          dna.malagauche=177;

                          set_up_timer(27,procbuywine,reason_drinks);
                         }
                         break;
                 }
                } else dixi('D',5);
                break;                /* Go to the bar! */

            case r__outsideducks: if (infield(6)) 
             {
              if (thing==onion) 
              {
               if (dna.obj[onion]) 
                dixi('D',10); /* not planning to juggle with the things! */
               else
               if (dna.carrying>=maxobjs) 
                  display("Before you ask, you remember that your hands are full.");
               else
               {
                if (dna.bought_onion) 
                 dixi('D',11); else
                  { dixi('D',9); points(3); }
                pennycheck(3); /* It costs thruppence. */
                dna.obj[onion]=true;
                objectlist;
                dna.bought_onion=true;
                dna.rotten_onion=false;  /* It's OK when it leaves the stall! */
                dna.onion_in_vinegar=false;
               }
              } else dixi('D',0);
             } else dixi('D',0);
             break;

             case r__nottspub: dixi('n',15); break; /* Can't sell to southerners. */
            default: dixi('D',0); /* Can't buy that. */
           }
          }
          break;
  case vb_attack: {
              if ((dna.room==r__brummieroad) &&
               ((person=='\235') || (thing==crossbow) || (thing==bolt))
               && (whereis['\235']==dna.room)) 
              {
               switch (ord(dna.obj[bolt])+ord(dna.obj[crossbow])*2) {
                /* 0 = neither, 1 = only bolt, 2 = only crossbow,
                  3 = both. */
                case 0: {
                    dixi('Q',10);
                    display("(At the very least, don't use your bare hands!)");
                   }
                   break;
                case 1: display(string("Attack him with only a crossbow bolt? Are you ")+
                    "planning on playing darts?!");
                    break;
                case 2: display(string("Come on, Avvy! You're not going to get very far ")+
                     "with only a crossbow!");
                     break;
                case 3: {
                    dixi('Q',11);
                    dna.cwytalot_gone=true;
                    dna.obj[bolt]=false; dna.obj[crossbow]=false;
                    objectlist;
                    magics[12].op=nix;
                    points(7);
                    tr[2].walkto(2);
                    tr[2].vanishifstill=true;
                    tr[2].call_eachstep=false;
                    whereis['\235']=177;
                   }
                   break;
                default: dixi('Q',10); /* Please try not to be so violent! */
               }
              } else dixi('Q',10);
             }
             break;
  case vb_password: if (dna.room!=r__bridge) 
                dixi('Q',12); else
               {
                ok=true;
                for( ff=1; ff <= length(thats); ff ++)
                 for( fv=1; fv <= length(words[dna.pass_num+first_password].w); fv ++)
                  if (words[dna.pass_num+first_password].w[fv] !=
                          upcase(realwords[ff][fv]))
                        ok=false;
                if (ok) 
                {
                 if (dna.drawbridge_open!=0) 
                   display("Contrary to your expectations, the drawbridge fails to close again.");
                 else
                 {
                    points(4);
                    display("The drawbridge opens!");
                    set_up_timer(7,procopen_drawbridge,reason_drawbridgefalls);
                    dna.drawbridge_open=1;
                 }
                } else dixi('Q',12);
               }
               break;
   case vb_dir: dir(realwords[2]); break;
   case vb_die: gameover; break;
   case vb_score: display(string("Your score is ")+strf(dna.score)+",\3\rout of a "+
                      "possible 128.\r\rThis gives you a rank of "+rank()+
                      ".\r\r"+totaltime());
                      break;
   case vb_put: putproc(); break;
   case vb_stand: stand_up(); break;

   case vb_kiss: if (person==pardon)
                   display("Kiss whom?");
            else if (personshere()) 
            switch (person) {
             case '\226' ... '\256': display("Hey, what kind of a weirdo are you??"); break;
             case parkata: dixi('U',12); break;
             case pgeida: dixi('U',13); break;
             case pwisewoman: dixi('U',14); break;
             default: dixi('U',5); /* You WHAT? */
            }
            break;

   case vb_climb: if (dna.room==r__insidecardiffcastle)  cardiff_climbing();
             else /* In the wrong room! */
              display("Not with your head for heights, Avvy!");
              break;

   case vb_jump: {
             set_up_timer(1,procjump,reason_jumping);
             dna.user_moves_avvy=false;
            }
            break;

   case vb_highscores: show_highs; break;

   case vb_wake: if (personshere()) 
              switch (person) {
               case pardon:case pavalot:case '\0': if (~ avvy_is_awake) 
                    {
                     avvy_is_awake=true;
                     points(1);
                     avvy_in_bed=true;
                     show_one(3); /* Picture of Avvy, awake in bed. */
                     if (teetotal)  dixi('d',13);
                    } else display("You're already awake, Avvy!");
                    break;
               case payles: if (~ ayles_is_awake)  display("You can't seem to wake him by yourself."); break;
               case pjacques: display(string("Brother Jacques, Brother Jacques, are you asleep?\231\2")+
                          "Hmmm... that doesn't seem to do any good...");
                          break;
               default: display("It's difficult to awaken people who aren't asleep...!");
              }
              break;

   case vb_sit: if (dna.room==r__nottspub) 
           {
            if (dna.sitting_in_pub) 
             display("You're already sitting!");
            else
            {
             tr[1].walkto(4); /* Move Avvy to the place, and sit him down. */
             set_up_timer(1,procavvy_sit_down,reason_sitting_down);
            }
           } else
           {     /* Default doodah. */
            dusk;
            hang_around_for_a_while;
            dawn;
            display("A few hours later...\20nothing much has happened...");
           }
           break;

   case vb_restart: if (ask("Restart game and lose changes?"))  {
                dusk;
                newgame;
                dawn;
               }
               break;

  case pardon: display("Hey, a verb would be helpful!"); break;

  case vb_hello: { person_speaks(); display("Hello.\2"); } break;
  case vb_thanks: { person_speaks(); display("That's OK.\2"); } break;
  default: display(string('\7')+"Parser bug!");
 }
}

void verbopt(char n, string& answer, char& anskey)
{
 switch (n) {
   case vb_exam: { answer="Examine"; anskey='x'; } break; /* the ubiqutous one */
   /* vb_give isn't dealt with by this procedure, but by ddm__with */
  case vb_drink: { answer="Drink";   anskey='D'; } break;
  case vb_wear:  { answer="Wear";    anskey='W'; } break;
  case vb_ring:  { answer="Ring";    anskey='R'; } break; /* only the bell! */
  case vb_play:  { answer="Play";    anskey='P'; } break;
  case vb_eat:   { answer="Eat";     anskey='E'; } break;
  default:      { answer="? Unknown!"; anskey='?'; }      /* Bug! */
 }
}

class unit_acci_initialize {
  public: unit_acci_initialize();
};
static unit_acci_initialize acci_constructor;

unit_acci_initialize::unit_acci_initialize() {
 weirdword=false;
}
