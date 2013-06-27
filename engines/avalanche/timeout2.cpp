/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* TIMEOUT	The scheduling unit. */

// DON'T FORGET ABOUT THE ARRAY INDEXES, THEY MAY'LL CAUSE TROUBLES!!!

#include "common/textconsole.h"

#include "avalanche/timeout2.h"

#include "avalanche/visa2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/trip6.h"
#include "avalanche/scrolls2.h"
#include "avalanche/acci2.h"
#include "avalanche/sequence2.h"
#include "avalanche/enid2.h"
#include "avalanche/pingo2.h"

namespace Avalanche {

	namespace Timeout {

	byte fv;

	void set_up_timer(int32 howlong, byte whither, byte why) {
		fv = 1;
		while ((fv < 8) && (times[fv].time_left != 0))  fv += 1;
		if (fv == 8)  return; /* Oh dear... */

		{
			timetype &with = times[fv];  /* Everything's OK here! */

			with.time_left = howlong;
			with.then_where = whither;
			with.what_for = why;
		}
	}

	void one_tick() {

		if (Gyro::ddmnow)  return;

		for (fv = 1; fv <= 7; fv ++) {
			timetype &with = times[fv];
			if (with.time_left > 0) {
				with.time_left -= 1;

				if (with.time_left == 0)
					switch (with.then_where) {
					case procopen_drawbridge :
						open_drawbridge();
						break;
					case procavaricius_talks :
						avaricius_talks();
						break;
					case procurinate :
						urinate();
						break;
					case proctoilet2 :
						toilet2();
						break;
					case procbang:
						bang();
						break;
					case procbang2:
						bang2();
						break;
					case procstairs:
						stairs();
						break;
					case proccardiffsurvey:
						cardiff_survey();
						break;
					case proccardiff_return:
						cardiff_return();
						break;
					case proc_cwytalot_in_herts:
						cwytalot_in_herts();
						break;
					case procget_tied_up:
						get_tied_up();
						break;
					case procget_tied_up2:
						get_tied_up2();
						break;
					case prochang_around:
						hang_around();
						break;
					case prochang_around2:
						hang_around2();
						break;
					case procafter_the_shootemup:
						after_the_shootemup();
						break;
					case procjacques_wakes_up:
						jacques_wakes_up();
						break;
					case procnaughty_duke:
						naughty_duke();
						break;
					case procnaughty_duke2:
						naughty_duke2();
						break;
					case procnaughty_duke3:
						naughty_duke3();
						break;
					case procjump:
						jump();
						break;
					case procsequence:
						Sequence::call_sequencer();
						break;
					case proccrapulus_splud_out:
						crapulus_says_splud_out();
						break;
					case procdawn_delay:
						Lucerna::dawn();
						break;
					case procbuydrinks:
						buydrinks();
						break;
					case procbuywine:
						buywine();
						break;
					case proccallsguards:
						callsguards();
						break;
					case procgreetsmonk:
						greetsmonk();
						break;
					case procfall_down_oubliette:
						fall_down_oubliette();
						break;
					case procmeet_avaroid:
						meet_avaroid();
						break;
					case procrise_up_oubliette:
						rise_up_oubliette();
						break;
					case procrobin_hood_and_geida:
						robin_hood_and_geida();
						break;
					case procrobin_hood_and_geida_talk:
						robin_hood_and_geida_talk();
						break;
					case procavalot_returns:
						avalot_returns();
						break;
					case procavvy_sit_down:
						avvy_sit_down();
						break;
					case procghost_room_phew:
						ghost_room_phew();
						break;
					case procarkata_shouts:
						arkata_shouts();
						break;
					case procwinning:
						winning();
						break;
					case procavalot_falls:
						avalot_falls();
						break;
					case procspludwick_goes_to_cauldron:
						spludwick_goes_to_cauldron();
						break;
					case procspludwick_leaves_cauldron:
						spludwick_leaves_cauldron();
						break;
					case procgive_lute_to_geida:
						give_lute_to_geida();
						break;
					}
			}
		}
		Gyro::roomtime += 1; /* Cycles since you've been in this room. */
		Gyro::dna.total_time += 1; /* Total amount of time for this game. */
	}

	void lose_timer(byte which) {
		byte fv;

		for (fv = 1; fv <= 7; fv ++) {
			timetype &with = times[fv];
			if (with.what_for == which)
				with.time_left = 0;
		} /* Cancel this one! */
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

	void open_drawbridge() {
		{
			Gyro::dna.drawbridge_open ++;
			Celer::show_one(Gyro::dna.drawbridge_open - 1);

			if (Gyro::dna.drawbridge_open == 4)
				Gyro::magics[2].op = Gyro::nix; /* You may enter the drawbridge. */
			else set_up_timer(7, procopen_drawbridge, reason_drawbridgefalls);
		}
	}

	/* --- */

	void avaricius_talks() {
		{
			Visa::dixi('q', Gyro::dna.avaricius_talk);
			Gyro::dna.avaricius_talk ++;

			if (Gyro::dna.avaricius_talk < 17)
				set_up_timer(177, procavaricius_talks, reason_avariciustalks);
			else Lucerna::points(3);

		}
	}

	void urinate() {
		Trip::tr[1].turn(Trip::up);
		Trip::stopwalking();
		Lucerna::showrw();
		set_up_timer(14, proctoilet2, reason_gototoilet);
	}

	void toilet2() {
		Scrolls::display("That's better!");
	}

	void bang() {
		Scrolls::display("\6< BANG! >");
		set_up_timer(30, procbang2, reason_explosion);
	}

	void bang2() {
		Scrolls::display("Hmm... sounds like Spludwick's up to something...");
	}

	void stairs() {
		Gyro::blip();
		Trip::tr[0].walkto(4);
		Celer::show_one(2);
		Gyro::dna.brummie_stairs = 2;
		Gyro::magics[11].op = Gyro::special;
		Gyro::magics[11].data = 2; /* Reached the bottom of the stairs. */
		Gyro::magics[4].op = Gyro::nix; /* Stop them hitting the sides (or the game will hang.) */
	}

	void cardiff_survey() {
		{
			switch (Gyro::dna.cardiff_things) {
			case 0: {
				Gyro::dna.cardiff_things += 1;
				Visa::dixi('q', 27);
			}
			break;
			}
			Visa::dixi('z', Gyro::dna.cardiff_things);
		}
		Gyro::interrogation = Gyro::dna.cardiff_things;
		set_up_timer(182, proccardiffsurvey, reason_cardiffsurvey);
	}

	void cardiff_return() {
		Visa::dixi('q', 28);
		cardiff_survey(); /* add end of question. */
	}

	void cwytalot_in_herts() {
		Visa::dixi('q', 29);
	}

	void get_tied_up() {
		Visa::dixi('q', 34); /* ...Trouble! */
		Gyro::dna.user_moves_avvy = false;
		Gyro::dna.been_tied_up = true;
		Trip::stopwalking();
		Trip::tr[2].stopwalk();
		Trip::tr[2].stophoming();
		Trip::tr[2].call_eachstep = true;
		Trip::tr[2].eachstep = Trip::procgrab_avvy;
		set_up_timer(70, procget_tied_up2, reason_getting_tied_up);
	}

	void get_tied_up2() {
		Trip::tr[1].walkto(4);
		Trip::tr[2].walkto(5);
		Gyro::magics[4].op = Gyro::nix; /* No effect when you touch the boundaries. */
		Gyro::dna.friar_will_tie_you_up = true;
	}

	void hang_around() {
		Trip::tr[2].check_me = false;
		Trip::tr[1].init(7, true); /* Robin Hood */
		Gyro::whereis[Gyro::probinhood] = r__robins;
		Trip::apped(1, 2);
		Visa::dixi('q', 39);
		Trip::tr[1].walkto(7);
		set_up_timer(55, prochang_around2, reason_hanging_around);
	}

	void hang_around2() {
		Visa::dixi('q', 40);
		Trip::tr[2].vanishifstill = false;
		Trip::tr[2].walkto(4);
		Gyro::whereis[Gyro::pfriartuck] = r__robins;
		Visa::dixi('q', 41);
		Trip::tr[1].done();
		Trip::tr[2].done(); /* Get rid of Robin Hood and Friar Tuck. */

		set_up_timer(1, procafter_the_shootemup, reason_hanging_around); 
		/* Immediately call the following proc (when you have a chance). */

		Gyro::dna.tied_up = false;

		Enid::back_to_bootstrap(1); /* Call the shoot-'em-up. */
	}

	void after_the_shootemup() {
		warning("STUB: Timeout::after_the_shootemup()");
	}

	void jacques_wakes_up() {
		Gyro::dna.jacques_awake += 1;

		switch (Gyro::dna.jacques_awake) { /* Additional pictures. */
		case 1 : {
			Celer::show_one(1); /* Eyes open. */
			Visa::dixi('Q', 45);
		}
		break;
		case 2 : { /* Going through the door. */
			Celer::show_one(2); /* Not on the floor. */
			Celer::show_one(3); /* But going through the door. */
			Gyro::magics[6].op = Gyro::nix; /* You can't wake him up now. */
		}
		break;
		case 3 : { /* Gone through the door. */
			Celer::show_one(2); /* Not on the floor, either. */
			Celer::show_one(4); /* He's gone... so the door's open. */
			Gyro::whereis[Gyro::pjacques] = 0; /* Gone! */
		}
		break;
		}


		if (Gyro::dna.jacques_awake == 5) {
			Gyro::dna.ringing_bells = true;
			Gyro::dna.ayles_is_awake = true;
			Lucerna::points(2);
		}

		switch (Gyro::dna.jacques_awake) {
		case 1:
		case 2:
		case 3:
			set_up_timer(12, procjacques_wakes_up, reason_jacques_waking_up);
			break;
		case 4:
			set_up_timer(24, procjacques_wakes_up, reason_jacques_waking_up);
			break;
		}

	}

	void naughty_duke()
	/* This is when the Duke comes in and takes your money. */
	{
		Trip::tr[2].init(9, false); /* Here comes the Duke. */
		Trip::apped(2, 1); /* He starts at the door... */
		Trip::tr[2].walkto(3); /* He walks over to you. */

		/* Let's get the door opening. */
		Celer::show_one(1);
		Sequence::first_show(2);
		Sequence::start_to_close();

		set_up_timer(50, procnaughty_duke2, reason_naughty_duke);
	}

	void naughty_duke2() {
		Visa::dixi('q', 48); /* Ha ha, it worked again! */
		Trip::tr[2].walkto(1); /* Walk to the door. */
		Trip::tr[2].vanishifstill = true; /* Then go away! */
		set_up_timer(32, procnaughty_duke3, reason_naughty_duke);
	}

	void naughty_duke3() {
		Celer::show_one(1);
		Sequence::first_show(2);
		Sequence::start_to_close();
	}

	void jump() {
		{
			Gyro::dnatype &with = Gyro::dna;

			with.jumpstatus += 1;

			{
				Trip::triptype &with1 = Trip::tr[1];
				switch (with.jumpstatus) {
				case 1:
				case 2:
				case 3:
				case 5:
				case 7:
				case 9:
					with1.y -= 1;
					break;
				case 12:
				case 13:
				case 14:
				case 16:
				case 18:
				case 19:
					with1.y += 1;
					break;
				}
			}

			if (with.jumpstatus == 20) {
				/* End of jump. */
				Gyro::dna.user_moves_avvy = true;
				Gyro::dna.jumpstatus = 0;
			} else {
				/* Still jumping. */
				set_up_timer(1, procjump, reason_jumping);
			}

			if ((with.jumpstatus == 10) /* You're at the highest point of your jump. */
					&& (Gyro::dna.room == r__insidecardiffcastle)
					&& (Gyro::dna.arrow_in_the_door == true)
					&& (Trip::infield(3))) { /* beside the wall*/
				/* Grab the arrow! */
				if (Gyro::dna.carrying >= Gyro::maxobjs)
					Scrolls::display("You fail to grab it, because your hands are full.");
				else {
					Celer::show_one(2);
					Gyro::dna.arrow_in_the_door = false; /* You've got it. */
					Gyro::dna.obj[Gyro::bolt] = true;
					Lucerna::objectlist();
					Visa::dixi('q', 50);
					Lucerna::points(3);
				}
			}
		}
	}

	void crapulus_says_splud_out() {
		Visa::dixi('q', 56);
		Gyro::dna.crapulus_will_tell = false;
	}

	void buydrinks() {
		Celer::show_one(11); /* Malagauche gets up again. */
		Gyro::dna.malagauche = 0;

		Visa::dixi('D', Gyro::dna.drinking); /* Scrolls::display message about it. */
		Pingo::wobble(); /* Do the special effects. */
		Visa::dixi('D', 1); /* That'll be thruppence. */
		if (Gyro::pennycheck(3)) /* Pay 3d. */
			Visa::dixi('D', 3); /* Tell 'em you paid up. */
		Acci::have_a_drink();
	}

	void buywine() {
		Celer::show_one(11); /* Malagauche gets up again. */
		Gyro::dna.malagauche = 0;

		Visa::dixi('D', 50); /* You buy the wine. */
		Visa::dixi('D', 1); /* It'll be thruppence. */
		if (Gyro::pennycheck(3)) {
			Visa::dixi('D', 4); /* You paid up. */
			Gyro::dna.obj[Gyro::wine] = true;
			Lucerna::objectlist();
			Gyro::dna.winestate = 1; /* OK Wine */
		}
	}

	void callsguards() {
		Visa::dixi('Q', 58); /* GUARDS!!! */
		Lucerna::gameover();
	}

	void greetsmonk() {
		Visa::dixi('Q', 59);
		Gyro::dna.entered_lusties_room_as_monk = true;
	}

	void fall_down_oubliette() {
		Gyro::magics[9].op = Gyro::nix;
		Trip::tr[1].iy += 1; /* increments dx/dy! */
		Trip::tr[1].y += Trip::tr[1].iy;   /* Dowwwn we go... */
		set_up_timer(3, procfall_down_oubliette, reason_falling_down_oubliette);
	}

	void meet_avaroid() {
		if (Gyro::dna.met_avaroid) {
			Scrolls::display("You can't expect to be \6that\22 lucky twice in a row!");
			Lucerna::gameover();
		} else {
			Visa::dixi('Q', 60);
			Gyro::dna.met_avaroid = true;
			set_up_timer(1, procrise_up_oubliette, reason_rising_up_oubliette);
			{
				Trip::triptype &with = Trip::tr[1];
				with.face = Trip::left;
				with.x = 151;
				with.ix = -3;
				with.iy = -5;
			}
			Gyro::background(2);
		}
	}

	void rise_up_oubliette() {
		{
			Trip::triptype &with = Trip::tr[1];

			with.visible = true;
			with.iy += 1; /* decrements dx/dy! */
			with.y -= with.iy; /* Uuuupppp we go... */
			if (with.iy > 0)
				set_up_timer(3, procrise_up_oubliette, reason_rising_up_oubliette);
			else
				Gyro::dna.user_moves_avvy = true;
		}
	}

	void robin_hood_and_geida() {
		Trip::tr[1].init(7, true);
		Trip::apped(1, 7);
		Trip::tr[1].walkto(6);
		Trip::tr[2].stopwalk();
		Trip::tr[2].face = Trip::left;
		set_up_timer(20, procrobin_hood_and_geida_talk, reason_robin_hood_and_geida);
		Gyro::dna.geida_follows = false;
	}

	void robin_hood_and_geida_talk() {
		Visa::dixi('q', 66);
		Trip::tr[1].walkto(2);
		Trip::tr[2].walkto(2);
		Trip::tr[1].vanishifstill = true;
		Trip::tr[2].vanishifstill = true;
		set_up_timer(162, procavalot_returns, reason_robin_hood_and_geida);
	}

	void avalot_returns() {
		Trip::tr[1].done();
		Trip::tr[2].done();
		Trip::tr[1].init(0, true);
		Trip::apped(1, 1);
		Visa::dixi('q', 67);
		Gyro::dna.user_moves_avvy = true;
	}

	void avvy_sit_down()
	/* This is used when you sit down in the pub in Notts. It loops around so
	  that it will happen when Avvy stops walking. */
	{
		if (Trip::tr[1].homing)    /* Still walking */
			set_up_timer(1, procavvy_sit_down, reason_sitting_down);
		else {
			Celer::show_one(3);
			Gyro::dna.sitting_in_pub = true;
			Gyro::dna.user_moves_avvy = false;
			Trip::tr[1].visible = false;
		}
	}

	void ghost_room_phew() {
		Scrolls::display("\6PHEW!\22 You're glad to get out of \6there!");
	}

	void arkata_shouts() {
		if (Gyro::dna.teetotal)  return;
		Visa::dixi('q', 76);
		set_up_timer(160, procarkata_shouts, reason_arkata_shouts);
	}

	void winning() {
		Visa::dixi('q', 79);
		Pingo::winning_pic();
		do {
			Lucerna::checkclick();
		} while (!(Gyro::mrelease == 0));
		Lucerna::callverb(Acci::vb_score);
		Scrolls::display(" T H E    E N D ");
		Gyro::lmo = true;
	}

	void avalot_falls() {
		if (Trip::tr[1].step < 5) {
			Trip::tr[1].step += 1;
			set_up_timer(3, procavalot_falls, reason_falling_over);
		} else
			Scrolls::display("\r\r\r\r\r\r\n\n\n\n\n\n\23Z\26");
	}

	void spludwick_goes_to_cauldron() {
		if (Trip::tr[2].homing)
			set_up_timer(1, procspludwick_goes_to_cauldron, reason_spludwalk);
		else
			set_up_timer(17, procspludwick_leaves_cauldron, reason_spludwalk);
	}

	void spludwick_leaves_cauldron() {
		Trip::tr[2].call_eachstep = true; /* So that normal procs will continue. */
	}

	void give_lute_to_geida() {   /* Moved here from Acci. */
		Visa::dixi('Q', 86);
		Lucerna::points(4);
		Gyro::dna.lustie_is_asleep = true;
		Sequence::first_show(5);
		Sequence::then_show(6); /* He falls asleep... */
		Sequence::start_to_close(); /* Not really closing, but we're using the same procedure. */
	}

	/* "This is all!" */

	class unit_timeout_initialize {
	public:
		unit_timeout_initialize();
	};
	static unit_timeout_initialize timeout_constructor;

	unit_timeout_initialize::unit_timeout_initialize() {
		for (int i = 0; i < sizeof(times); i++) {
			times[i].time_left = 0;
			times[i].then_where = 0;
			times[i].what_for = 0;
		}
	}


	} // End of namespace Timeout.

} // End of namespace Avalanche.
