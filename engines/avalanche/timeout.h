/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */
#ifndef __timeout_h__
#define __timeout_h__


#include "gyro.h"
#include "celer.h"


/* reason_ now runs between 1 and 28. */

const integer reason_drawbridgefalls = 2;
const integer reason_avariciustalks = 3;
const integer reason_gototoilet = 4;
const integer reason_explosion = 5;
const integer reason_brummiestairs = 6;
const integer reason_cardiffsurvey = 7;
const integer reason_cwytalot_in_herts = 8;
const integer reason_getting_tied_up = 9;
const integer reason_hanging_around = 10; /* Tied to the tree in Nottingham. */
const integer reason_jacques_waking_up = 11;
const integer reason_naughty_duke = 12;
const integer reason_jumping = 13;
const integer reason_sequencer = 14;
const integer reason_crapulus_says_spludwick_out = 15;
const integer reason_dawndelay = 16;
const integer reason_drinks = 17;
const integer reason_du_lustie_talks = 18;
const integer reason_falling_down_oubliette = 19;
const integer reason_meeting_avaroid = 20;
const integer reason_rising_up_oubliette = 21;
const integer reason_robin_hood_and_geida = 22;
const integer reason_sitting_down = 23;
const integer reason_ghost_room_phew = 1;
const integer reason_arkata_shouts = 24;
const integer reason_winning = 25;
const integer reason_falling_over = 26;
const integer reason_spludwalk = 27;
const integer reason_geida_sings = 28;

/* PROCx now runs between 1 and 41. */

const integer procopen_drawbridge = 3;

const integer procavaricius_talks = 4;

const integer procurinate = 5;

const integer proctoilet2 = 6;

const integer procbang = 7;

const integer procbang2 = 8;

const integer procstairs = 9;

const integer proccardiffsurvey = 10;

const integer proccardiff_return = 11;

const integer proc_cwytalot_in_herts = 12;

const integer procget_tied_up = 13;

const integer procget_tied_up2 = 1;

const integer prochang_around = 14;

const integer prochang_around2 = 15;

const integer procafter_the_shootemup = 32;

const integer procjacques_wakes_up = 16;

const integer procnaughty_duke = 17;

const integer procnaughty_duke2 = 18;

const integer procnaughty_duke3 = 38;

const integer procjump = 19;

const integer procsequence = 20;

const integer proccrapulus_splud_out = 21;

const integer procdawn_delay = 22;

const integer procbuydrinks = 23;

const integer procbuywine = 24;

const integer proccallsguards = 25;

const integer procgreetsmonk = 26;

const integer procfall_down_oubliette = 27;

const integer procmeet_avaroid = 28;

const integer procrise_up_oubliette = 29;

const integer procrobin_hood_and_geida = 2;

const integer procrobin_hood_and_geida_talk = 30;

const integer procavalot_returns = 31;

const integer procavvy_sit_down = 33; /* In Nottingham. */

const integer procghost_room_phew = 34;

const integer procarkata_shouts = 35;

const integer procwinning = 36;

const integer procavalot_falls = 37;

const integer procspludwick_goes_to_cauldron = 39;

const integer procspludwick_leaves_cauldron = 40;

const integer procgive_lute_to_geida = 41;

struct timetype {
            longint time_left;
            byte then_where;
            byte what_for;
};


#ifdef __timeout_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<1,7,timetype> times;
#undef EXTERN
#define EXTERN extern


void set_up_timer(longint howlong, byte whither,byte why);

void one_tick();

void lose_timer(byte which);

/*function timer_is_on(which:byte):boolean;*/

 /* Procedures to do things at the end of amounts of time: */

 void open_drawbridge();

 void avaricius_talks();

 void urinate();

 void toilet2();

 void bang();

 void bang2();

 void stairs();

 void cardiff_survey();

 void cardiff_return();

 void cwytalot_in_herts();

 void get_tied_up();

 void get_tied_up2();

 void hang_around();

 void hang_around2();

 void after_the_shootemup();

 void jacques_wakes_up();

 void naughty_duke();

 void naughty_duke2();

 void naughty_duke3();

 void jump();

 void crapulus_says_splud_out();

 void buydrinks();

 void buywine();

 void callsguards();

 void greetsmonk();

 void fall_down_oubliette();

 void meet_avaroid();

 void rise_up_oubliette();

 void robin_hood_and_geida();

 void robin_hood_and_geida_talk();

 void avalot_returns();

 void avvy_sit_down();

 void ghost_room_phew();

 void arkata_shouts();

 void winning();

 void avalot_falls();

 void spludwick_goes_to_cauldron();

 void spludwick_leaves_cauldron();

 void give_lute_to_geida();

#endif
