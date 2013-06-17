#include "ptoc.h"

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 TIMEOUT          The scheduling unit. */

#define __timeout_implementation__


#include "timeout.h"


#include "visa.h"
#include "lucerna.h"
#include "trip5.h"
#include "scrolls.h"
#include "sequence.h"
#include "pingo.h"
#include "Acci.h"
#include "enid.h"


byte fv;

void set_up_timer(longint howlong, byte whither,byte why)
{
 fv=1;
 while ((fv<8) && (times[fv].time_left!=0))  fv += 1;
 if (fv==8)  return; /* Oh dear... */

 {
  timetype& with = times[fv];  /* Everything's OK here! */

  with.time_left=howlong;
  with.then_where=whither;
  with.what_for=why;
 }
}

void one_tick()
{

 if (ddmnow)  return;

 for( fv=1; fv <= 7; fv ++)
  { timetype& with = times[fv]; 
   if (with.time_left>0) 
   {
    with.time_left -= 1;

    if (with.time_left==0) 
    switch (with.then_where) {
     case procopen_drawbridge : open_drawbridge(); break;
     case procavaricius_talks : avaricius_talks(); break;
     case procurinate : urinate(); break;
     case proctoilet2 : toilet2(); break;
     case procbang: bang(); break;
     case procbang2: bang2(); break;
     case procstairs: stairs(); break;
     case proccardiffsurvey: cardiff_survey(); break;
     case proccardiff_return: cardiff_return(); break;
     case proc_cwytalot_in_herts: cwytalot_in_herts(); break;
     case procget_tied_up: get_tied_up(); break;
     case procget_tied_up2: get_tied_up2(); break;
     case prochang_around: hang_around(); break;
     case prochang_around2: hang_around2(); break;
     case procafter_the_shootemup: after_the_shootemup(); break;
     case procjacques_wakes_up: jacques_wakes_up(); break;
     case procnaughty_duke: naughty_duke(); break;
     case procnaughty_duke2: naughty_duke2(); break;
     case procnaughty_duke3: naughty_duke3(); break;
     case procjump: jump(); break;
     case procsequence: call_sequencer(); break;
     case proccrapulus_splud_out: crapulus_says_splud_out(); break;
     case procdawn_delay: dawn(); break;
     case procbuydrinks: buydrinks(); break;
     case procbuywine: buywine(); break;
     case proccallsguards: callsguards(); break;
     case procgreetsmonk: greetsmonk(); break;
     case procfall_down_oubliette: fall_down_oubliette(); break;
     case procmeet_avaroid: meet_avaroid(); break;
     case procrise_up_oubliette: rise_up_oubliette(); break;
     case procrobin_hood_and_geida: robin_hood_and_geida(); break;
     case procrobin_hood_and_geida_talk: robin_hood_and_geida_talk(); break;
     case procavalot_returns: avalot_returns(); break;
     case procavvy_sit_down: avvy_sit_down(); break;
     case procghost_room_phew: ghost_room_phew(); break;
     case procarkata_shouts: arkata_shouts(); break;
     case procwinning: winning(); break;
     case procavalot_falls: avalot_falls(); break;
     case procspludwick_goes_to_cauldron: spludwick_goes_to_cauldron(); break;
     case procspludwick_leaves_cauldron: spludwick_leaves_cauldron(); break;
     case procgive_lute_to_geida: give_lute_to_geida(); break;
    }
   }}
 roomtime += 1; /* Cycles since you've been in this room. */
 dna.total_time += 1; /* Total amount of time for this game. */
}

void lose_timer(byte which)
{
    byte fv;

 for( fv=1; fv <= 7; fv ++)
  { timetype& with = times[fv]; 
   if (with.what_for==which) 
    with.time_left=0;} /* Cancel this one! */
}

/*function timer_is_on(which:byte):boolean;
var fv:byte;
begin
 for fv:=1 to 7 do
  with times[fv] do
   if (what_for=which) and (time_left>0) then
   begin
    timer_is_on:=true;
    exit;
   end;
 timer_is_on:=false;
end;*/

/* Timeout procedures: */

void open_drawbridge()
{
 {
  dnatype& with = dna; 

  with.drawbridge_open += 1;
  show_one(with.drawbridge_open-1);

  if (with.drawbridge_open==4) 
   magics[2].op=nix; /* You may enter the drawbridge. */
  else set_up_timer(7,procopen_drawbridge,reason_drawbridgefalls);
 }
}

/* --- */

void avaricius_talks()
{
 {
  dnatype& with = dna; 

  dixi('q',with.avaricius_talk);
  with.avaricius_talk += 1;

  if (with.avaricius_talk<17) 
   set_up_timer(177,procavaricius_talks,reason_avariciustalks);
  else points(3);

 }
}

void urinate()
{
 tr[1].turn(up);
 stopwalking();
 showrw();
 set_up_timer(14,proctoilet2,reason_gototoilet);
}

void toilet2()
{
 display("That's better!");
}

void bang()
{
 display("\6< BANG! >");
 set_up_timer(30,procbang2,reason_explosion);
}

void bang2()
{
 display("Hmm... sounds like Spludwick's up to something...");
}

void stairs()
{
 blip();
 tr[1].walkto(4);
 show_one(2);
 dna.brummie_stairs=2;
 magics[11].op=special;
 magics[11].data=2;  /* Reached the bottom of the stairs. */
 magics[4].op=nix;  /* Stop them hitting the sides (or the game will hang.) */
}

void cardiff_survey()
{
 {
  dnatype& with = dna; 

  switch (with.cardiff_things) {
   case 0: {
       with.cardiff_things += 1;
       dixi('q',27);
      }
      break;
  }
  dixi('z',with.cardiff_things);
 }
 interrogation=dna.cardiff_things;
 set_up_timer(182,proccardiffsurvey,reason_cardiffsurvey);
}

void cardiff_return()
{
 dixi('q',28);
 cardiff_survey(); /* add end of question. */
}

void cwytalot_in_herts()
{
 dixi('q',29);
}

void get_tied_up()
{
 dixi('q',34); /* ...Trouble! */
 dna.user_moves_avvy=false;
 dna.been_tied_up=true;
 stopwalking();
 tr[2].stopwalk(); tr[2].stophoming();
 tr[2].call_eachstep=true;
 tr[2].eachstep=procgrab_avvy;
 set_up_timer(70,procget_tied_up2,reason_getting_tied_up);
}

void get_tied_up2()
{
 tr[1].walkto(4);
 tr[2].walkto(5);
 magics[4].op=nix;  /* No effect when you touch the boundaries. */
 dna.friar_will_tie_you_up=true;
}

void hang_around()
{
 tr[2].check_me=false;
 tr[1].init(7,true); /* Robin Hood */
 whereis[probinhood]=r__robins;
 apped(1,2);
 dixi('q',39);
 tr[1].walkto(7);
 set_up_timer(55,prochang_around2,reason_hanging_around);
}

void hang_around2()
{
 dixi('q',40);
 tr[2].vanishifstill=false;
 tr[2].walkto(4);
 whereis[pfriartuck]=r__robins;
 dixi('q',41);
 tr[1].done(); tr[2].done(); /* Get rid of Robin Hood and Friar Tuck. */

 set_up_timer(1,procafter_the_shootemup,reason_hanging_around); /* Immediately
  call the following proc (when you have a chance). */

 dna.tied_up=false;

 back_to_bootstrap(1); /* Call the shoot-'em-up. */
}

void after_the_shootemup()
{
    byte shootscore,gain;

 tr[1].init(0,true); /* Avalot. */
 apped(1,2); dna.user_moves_avvy=true;
 dna.obj[crossbow]=true; objectlist();

 shootscore=mem[storage_seg*storage_ofs];
 gain=(shootscore+5) / 10;    /* Rounding up. */

 display(string("\6Your score was ")+strf(shootscore)+'.'+"\r\rYou gain ("+
    strf(shootscore)+" ˆ 10) = "+strf(gain)+" points.");

 if (gain>20) 
 {
   display("But we won't let you have more than 20 points!");
   points(20);
 } else
   points(gain);

 dixi('q',70);
}

void jacques_wakes_up()
{
 dna.jacques_awake += 1;

 switch (dna.jacques_awake) { /* Additional pictures. */
  case 1 : {
       show_one(1); /* Eyes open. */
       dixi('Q',45);
      }
      break;
  case 2 : { /* Going through the door. */
       show_one(2); /* Not on the floor. */
       show_one(3); /* But going through the door. */
       magics[6].op=nix;  /* You can't wake him up now. */
      }
      break;
  case 3 : { /* Gone through the door. */
       show_one(2); /* Not on the floor, either. */
       show_one(4); /* He's gone... so the door's open. */
       whereis[pjacques]=0;  /* Gone! */
      }
      break;
 }


 if (dna.jacques_awake==5) 
 {
  dna.ringing_bells=true;
  dna.ayles_is_awake=true;
  points(2);
 }

 switch (dna.jacques_awake) {
  case RANGE_3(1,3): set_up_timer(12,procjacques_wakes_up,reason_jacques_waking_up);
  break;
  case 4: set_up_timer(24,procjacques_wakes_up,reason_jacques_waking_up); break;
 }

}

void naughty_duke()
 /* This is when the Duke comes in and takes your money. */
{
 tr[2].init(9,false); /* Here comes the Duke. */
 apped(2,1); /* He starts at the door... */
 tr[2].walkto(3); /* He walks over to you. */

 /* Let's get the door opening. */

 show_one(1); first_show(2); start_to_close();

 set_up_timer(50,procnaughty_duke2,reason_naughty_duke);
}

void naughty_duke2()
{
 dixi('q',48); /* Ha ha, it worked again! */
 tr[2].walkto(1); /* Walk to the door. */
 tr[2].vanishifstill=true;  /* Then go away! */
 set_up_timer(32,procnaughty_duke3,reason_naughty_duke);
}

void naughty_duke3()
{
 show_one(1); first_show(2); start_to_close();
}

void jump()
{
 {
  dnatype& with = dna; 

  with.jumpstatus += 1;

  { triptype& with1 = tr[1]; 
   switch (with.jumpstatus) {
    case 1:case 2:case 3:case 5:case 7:case 9: with1.y -= 1; break;
    case 12:case 13:case 14:case 16:case 18:case 19: with1.y += 1; break;
   }}

  if (with.jumpstatus==20) 
  {     /* End of jump. */
   dna.user_moves_avvy=true;
   dna.jumpstatus=0;
  } else
  {     /* Still jumping. */
   set_up_timer(1,procjump,reason_jumping);
  }

  if ((with.jumpstatus==10) /* You're at the highest point of your jump. */
   && (dna.room==r__insidecardiffcastle)
    && (dna.arrow_in_the_door==true)
     && (infield(3))) /* beside the wall*/ 
    {     /* Grab the arrow! */
       if (dna.carrying>=maxobjs) 
          display("You fail to grab it, because your hands are full.");
       else
       {
          show_one(2);
          dna.arrow_in_the_door=false;  /* You've got it. */
          dna.obj[bolt]=true;
          objectlist();
          dixi('q',50);
          points(3);
       }
    }
 }
}

void crapulus_says_splud_out()
{
 dixi('q',56);
 dna.crapulus_will_tell=false;
}

void buydrinks()
{
 show_one(11); /* Malagauche gets up again. */
 dna.malagauche=0;

 dixi('D',ord(dna.drinking)); /* Display message about it. */
 wobble(); /* Do the special effects. */
 dixi('D',1); /* That'll be thruppence. */
 if (pennycheck(3)) /* Pay 3d. */
       dixi('D',3); /* Tell 'em you paid up. */
 have_a_drink();
}

void buywine()
{
 show_one(11); /* Malagauche gets up again. */
 dna.malagauche=0;

 dixi('D',50); /* You buy the wine. */
 dixi('D',1); /* It'll be thruppence. */
 if (pennycheck(3)) 
 {
  dixi('D',4); /* You paid up. */
  dna.obj[wine]=true;
  objectlist();
  dna.winestate=1;  /* OK Wine */
 }
}

void callsguards()
{
 dixi('Q',58); /* GUARDS!!! */
 gameover();
}

void greetsmonk()
{
 dixi('Q',59); dna.entered_lusties_room_as_monk=true;
}

void fall_down_oubliette()
{
 magics[9].op=nix;
 tr[1].iy += 1; /* increments dx/dy! */
 tr[1].y += tr[1].iy;   /* Dowwwn we go... */
 set_up_timer(3,procfall_down_oubliette,reason_falling_down_oubliette);
}

void meet_avaroid()
{
 if (dna.met_avaroid) 
 {
  display("You can't expect to be \6that\22 lucky twice in a row!");
  gameover();
 } else
 {
  dixi('Q',60); dna.met_avaroid=true;
  set_up_timer(1,procrise_up_oubliette,reason_rising_up_oubliette);
  {
                      triptype& with = tr[1];   with.face=left; with.x=151; with.ix=-3; with.iy=-5; }
  background(2);
 }
}

void rise_up_oubliette()
{
 {
  triptype& with = tr[1]; 

  with.visible=true;
  with.iy += 1; /* decrements dx/dy! */
  with.y -= with.iy; /* Uuuupppp we go... */
  if (with.iy>0) 
   set_up_timer(3,procrise_up_oubliette,reason_rising_up_oubliette);
  else
   dna.user_moves_avvy=true;
 }
}

void robin_hood_and_geida()
{
 tr[1].init(7,true);
 apped(1,7);
 tr[1].walkto(6);
 tr[2].stopwalk();
 tr[2].face=left;
 set_up_timer(20,procrobin_hood_and_geida_talk,reason_robin_hood_and_geida);
 dna.geida_follows=false;
}

void robin_hood_and_geida_talk()
{
 dixi('q',66); tr[1].walkto(2); tr[2].walkto(2);
 tr[1].vanishifstill=true; tr[2].vanishifstill=true;
 set_up_timer(162,procavalot_returns,reason_robin_hood_and_geida);
}

void avalot_returns()
{
 tr[1].done(); tr[2].done();
 tr[1].init(0,true);
 apped(1,1);
 dixi('q',67);
 dna.user_moves_avvy=true;
}

void avvy_sit_down()
 /* This is used when you sit down in the pub in Notts. It loops around so
   that it will happen when Avvy stops walking. */
{
 if (tr[1].homing)    /* Still walking */
  set_up_timer(1,procavvy_sit_down,reason_sitting_down);
 else
 {
  show_one(3);
  dna.sitting_in_pub=true;
  dna.user_moves_avvy=false;
  tr[1].visible=false;
 }
}

void ghost_room_phew()
{
 display("\6PHEW!\22 You're glad to get out of \6there!");
}

void arkata_shouts()
{
  if (dna.teetotal)  return;
  dixi('q',76);
  set_up_timer(160,procarkata_shouts,reason_arkata_shouts);
}

void winning()
{
 dixi('q',79);
 winning_pic();
 do { checkclick(); } while (!(mrelease==0));
 callverb(vb_score);
 display(" T H E    E N D ");
 lmo=true;
}

void avalot_falls()
{
 if (tr[1].step<5) 
 {
  tr[1].step += 1;
  set_up_timer(3,procavalot_falls,reason_falling_over);
 } else
  display("\r\r\r\r\r\r\n\n\n\n\n\n\23Z\26");
}

void spludwick_goes_to_cauldron()
{
  if (tr[2].homing) 
    set_up_timer(1,procspludwick_goes_to_cauldron,reason_spludwalk);
  else
    set_up_timer(17,procspludwick_leaves_cauldron,reason_spludwalk);
}

void spludwick_leaves_cauldron()
{
  tr[2].call_eachstep=true;  /* So that normal procs will continue. */
}

void give_lute_to_geida()     /* Moved here from Acci. */
{
  dixi('Q',86);
  points(4);
  dna.lustie_is_asleep=true;
  first_show(5); then_show(6); /* He falls asleep... */
  start_to_close(); /* Not really closing, but we're using the same procedure. */
}

/* "This is all!" */

class unit_timeout_initialize {
  public: unit_timeout_initialize();
};
static unit_timeout_initialize timeout_constructor;

unit_timeout_initialize::unit_timeout_initialize() {
 fillchar(times,sizeof(times),'\0');
}
