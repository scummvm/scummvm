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


#include "avalanche/avalanche.h"

#include "avalanche/timeout2.h"
#include "avalanche/visa2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/trip6.h"
#include "avalanche/scrolls2.h"
#include "avalanche/acci2.h"
#include "avalanche/sequence2.h"
#include "avalanche/enid2.h"
#include "avalanche/pingo2.h"

#include "common/textconsole.h"

namespace Avalanche {

Timeout::Timeout(AvalancheEngine *vm) {
	_vm = vm;

	for (byte i = 0; i < 7; i++) {
		times[i].time_left = 0;
		times[i].then_where = 0;
		times[i].what_for = 0;
	}
}

void Timeout::set_up_timer(int32 howlong, byte whither, byte why) {
	fv = 0;
	while ((fv < 7) && (times[fv].time_left != 0))
		fv++;

	if (fv == 7)
		return; /* Oh dear... */

	/* Everything's OK here! */
	times[fv].time_left = howlong;
	times[fv].then_where = whither;
	times[fv].what_for = why;
}

void Timeout::one_tick() {
	if (_vm->_gyro->ddmnow)
		return;

	for (fv = 0; fv < 7; fv++) {
		if (times[fv].time_left > 0) {
			times[fv].time_left--;

			if (times[fv].time_left == 0) {
				switch (times[fv].then_where) {
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
					_vm->_sequence->call_sequencer();
					break;
				case proccrapulus_splud_out:
					crapulus_says_splud_out();
					break;
				case procdawn_delay:
					_vm->_lucerna->dawn();
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
	}
	_vm->_gyro->roomtime++; /* Cycles since you've been in this room. */
	_vm->_gyro->dna.total_time++; /* Total amount of time for this game. */
}

void Timeout::lose_timer(byte which) {
	for (byte fv = 0; fv < 7; fv++) {
		if (times[fv].what_for == which)
			times[fv].time_left = 0; // Cancel this one!
	}
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

void Timeout::open_drawbridge() {
	_vm->_gyro->dna.drawbridge_open++;
	_vm->_celer->show_one(_vm->_gyro->dna.drawbridge_open - 1);

	if (_vm->_gyro->dna.drawbridge_open == 4)
		_vm->_gyro->magics[2].op = _vm->_gyro->nix; /* You may enter the drawbridge. */
	else
		set_up_timer(7, procopen_drawbridge, reason_drawbridgefalls);
}

/* --- */

void Timeout::avaricius_talks() {
	_vm->_visa->dixi('q', _vm->_gyro->dna.avaricius_talk);
	_vm->_gyro->dna.avaricius_talk++;

	if (_vm->_gyro->dna.avaricius_talk < 17)
		set_up_timer(177, procavaricius_talks, reason_avariciustalks);
	else
		_vm->_lucerna->points(3);
}

void Timeout::urinate() {
	_vm->_trip->tr[1].turn(_vm->_trip->up);
	_vm->_trip->stopwalking();
	_vm->_lucerna->showrw();
	set_up_timer(14, proctoilet2, reason_gototoilet);
}

void Timeout::toilet2() {
	_vm->_scrolls->display("That's better!");
}

void Timeout::bang() {
	_vm->_scrolls->display("\6< BANG! >");
	set_up_timer(30, procbang2, reason_explosion);
}

void Timeout::bang2() {
	_vm->_scrolls->display("Hmm... sounds like Spludwick's up to something...");
}

void Timeout::stairs() {
	_vm->_gyro->blip();
	_vm->_trip->tr[0].walkto(4);
	_vm->_celer->show_one(2);
	_vm->_gyro->dna.brummie_stairs = 2;
	_vm->_gyro->magics[11].op = _vm->_gyro->special;
	_vm->_gyro->magics[11].data = 2; /* Reached the bottom of the stairs. */
	_vm->_gyro->magics[4].op = _vm->_gyro->nix; /* Stop them hitting the sides (or the game will hang.) */
}

void Timeout::cardiff_survey() {
	switch (_vm->_gyro->dna.cardiff_things) {
	case 0:
		_vm->_gyro->dna.cardiff_things += 1;
		_vm->_visa->dixi('q', 27);
		break;
	}
	_vm->_visa->dixi('z', _vm->_gyro->dna.cardiff_things);

	_vm->_gyro->interrogation = _vm->_gyro->dna.cardiff_things;
	set_up_timer(182, proccardiffsurvey, reason_cardiffsurvey);
}

void Timeout::cardiff_return() {
	_vm->_visa->dixi('q', 28);
	cardiff_survey(); /* add end of question. */
}

void Timeout::cwytalot_in_herts() {
	_vm->_visa->dixi('q', 29);
}

void Timeout::get_tied_up() {
	_vm->_visa->dixi('q', 34); /* ...Trouble! */
	_vm->_gyro->dna.user_moves_avvy = false;
	_vm->_gyro->dna.been_tied_up = true;
	_vm->_trip->stopwalking();
	_vm->_trip->tr[2].stopwalk();
	_vm->_trip->tr[2].stophoming();
	_vm->_trip->tr[2].call_eachstep = true;
	_vm->_trip->tr[2].eachstep = _vm->_trip->procgrab_avvy;
	set_up_timer(70, procget_tied_up2, reason_getting_tied_up);
}

void Timeout::get_tied_up2() {
	_vm->_trip->tr[1].walkto(4);
	_vm->_trip->tr[2].walkto(5);
	_vm->_gyro->magics[4].op = _vm->_gyro->nix; /* No effect when you touch the boundaries. */
	_vm->_gyro->dna.friar_will_tie_you_up = true;
}

void Timeout::hang_around() {
	_vm->_trip->tr[2].check_me = false;
	_vm->_trip->tr[1].init(7, true, _vm->_trip); /* Robin Hood */
	_vm->_gyro->whereis[_vm->_gyro->probinhood] = r__robins;
	_vm->_trip->apped(1, 2);
	_vm->_visa->dixi('q', 39);
	_vm->_trip->tr[1].walkto(7);
	set_up_timer(55, prochang_around2, reason_hanging_around);
}

void Timeout::hang_around2() {
	_vm->_visa->dixi('q', 40);
	_vm->_trip->tr[2].vanishifstill = false;
	_vm->_trip->tr[2].walkto(4);
	_vm->_gyro->whereis[_vm->_gyro->pfriartuck] = r__robins;
	_vm->_visa->dixi('q', 41);
	_vm->_trip->tr[1].done();
	_vm->_trip->tr[2].done(); /* Get rid of Robin Hood and Friar Tuck. */

	set_up_timer(1, procafter_the_shootemup, reason_hanging_around); 
	/* Immediately call the following proc (when you have a chance). */

	_vm->_gyro->dna.tied_up = false;

	_vm->_enid->back_to_bootstrap(1); /* Call the shoot-'em-up. */
}

void Timeout::after_the_shootemup() {
	warning("STUB: Timeout::after_the_shootemup()");
}

void Timeout::jacques_wakes_up() {
	_vm->_gyro->dna.jacques_awake += 1;

	switch (_vm->_gyro->dna.jacques_awake) { /* Additional pictures. */
	case 1 :
		_vm->_celer->show_one(1); /* Eyes open. */
		_vm->_visa->dixi('Q', 45);
		break;
	case 2 : /* Going through the door. */
		_vm->_celer->show_one(2); /* Not on the floor. */
		_vm->_celer->show_one(3); /* But going through the door. */
		_vm->_gyro->magics[6].op = _vm->_gyro->nix; /* You can't wake him up now. */
		break;
	case 3 :  /* Gone through the door. */
		_vm->_celer->show_one(2); /* Not on the floor, either. */
		_vm->_celer->show_one(4); /* He's gone... so the door's open. */
		_vm->_gyro->whereis[_vm->_gyro->pjacques] = 0; /* Gone! */
		break;
	}


	if (_vm->_gyro->dna.jacques_awake == 5) {
		_vm->_gyro->dna.ringing_bells = true;
		_vm->_gyro->dna.ayles_is_awake = true;
		_vm->_lucerna->points(2);
	}

	switch (_vm->_gyro->dna.jacques_awake) {
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

void Timeout::naughty_duke() {
/* This is when the Duke comes in and takes your money. */
	_vm->_trip->tr[2].init(9, false, _vm->_trip); /* Here comes the Duke. */
	_vm->_trip->apped(2, 1); /* He starts at the door... */
	_vm->_trip->tr[2].walkto(3); /* He walks over to you. */

	/* Let's get the door opening. */
	_vm->_celer->show_one(1);
	_vm->_sequence->first_show(2);
	_vm->_sequence->start_to_close();

	set_up_timer(50, procnaughty_duke2, reason_naughty_duke);
}

void Timeout::naughty_duke2() {
	_vm->_visa->dixi('q', 48); /* Ha ha, it worked again! */
	_vm->_trip->tr[2].walkto(1); /* Walk to the door. */
	_vm->_trip->tr[2].vanishifstill = true; /* Then go away! */
	set_up_timer(32, procnaughty_duke3, reason_naughty_duke);
}

void Timeout::naughty_duke3() {
	_vm->_celer->show_one(1);
	_vm->_sequence->first_show(2);
	_vm->_sequence->start_to_close();
}

void Timeout::jump() {
	dnatype &with = _vm->_gyro->dna;

	with.jumpstatus += 1;

	triptype &with1 = _vm->_trip->tr[1];
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

	if (with.jumpstatus == 20) {
		/* End of jump. */
		_vm->_gyro->dna.user_moves_avvy = true;
		_vm->_gyro->dna.jumpstatus = 0;
	} else {
		/* Still jumping. */
		set_up_timer(1, procjump, reason_jumping);
	}

	if ((with.jumpstatus == 10) /* You're at the highest point of your jump. */
			&& (_vm->_gyro->dna.room == r__insidecardiffcastle)
			&& (_vm->_gyro->dna.arrow_in_the_door == true)
			&& (_vm->_trip->infield(3))) { /* beside the wall*/
		/* Grab the arrow! */
		if (_vm->_gyro->dna.carrying >= maxobjs)
			_vm->_scrolls->display("You fail to grab it, because your hands are full.");
		else {
			_vm->_celer->show_one(2);
			_vm->_gyro->dna.arrow_in_the_door = false; /* You've got it. */
			_vm->_gyro->dna.obj[_vm->_gyro->bolt] = true;
			_vm->_lucerna->objectlist();
			_vm->_visa->dixi('q', 50);
			_vm->_lucerna->points(3);
		}
	}
}

void Timeout::crapulus_says_splud_out() {
	_vm->_visa->dixi('q', 56);
	_vm->_gyro->dna.crapulus_will_tell = false;
}

void Timeout::buydrinks() {
	_vm->_celer->show_one(11); /* Malagauche gets up again. */
	_vm->_gyro->dna.malagauche = 0;

	_vm->_visa->dixi('D', _vm->_gyro->dna.drinking); /* _vm->_scrolls->display message about it. */
	_vm->_pingo->wobble(); /* Do the special effects. */
	_vm->_visa->dixi('D', 1); /* That'll be thruppence. */
	if (_vm->_gyro->pennycheck(3)) /* Pay 3d. */
		_vm->_visa->dixi('D', 3); /* Tell 'em you paid up. */
	_vm->_acci->have_a_drink();
}

void Timeout::buywine() {
	_vm->_celer->show_one(11); /* Malagauche gets up again. */
	_vm->_gyro->dna.malagauche = 0;

	_vm->_visa->dixi('D', 50); /* You buy the wine. */
	_vm->_visa->dixi('D', 1); /* It'll be thruppence. */
	if (_vm->_gyro->pennycheck(3)) {
		_vm->_visa->dixi('D', 4); /* You paid up. */
		_vm->_gyro->dna.obj[_vm->_gyro->wine] = true;
		_vm->_lucerna->objectlist();
		_vm->_gyro->dna.winestate = 1; /* OK Wine */
	}
}

void Timeout::callsguards() {
	_vm->_visa->dixi('Q', 58); /* GUARDS!!! */
	_vm->_lucerna->gameover();
}

void Timeout::greetsmonk() {
	_vm->_visa->dixi('Q', 59);
	_vm->_gyro->dna.entered_lusties_room_as_monk = true;
}

void Timeout::fall_down_oubliette() {
	_vm->_gyro->magics[9].op = _vm->_gyro->nix;
	_vm->_trip->tr[1].iy += 1; /* increments dx/dy! */
	_vm->_trip->tr[1].y += _vm->_trip->tr[1].iy;   /* Dowwwn we go... */
	set_up_timer(3, procfall_down_oubliette, reason_falling_down_oubliette);
}

void Timeout::meet_avaroid() {
	if (_vm->_gyro->dna.met_avaroid) {
		_vm->_scrolls->display("You can't expect to be \6that\22 lucky twice in a row!");
		_vm->_lucerna->gameover();
	} else {
		_vm->_visa->dixi('Q', 60);
		_vm->_gyro->dna.met_avaroid = true;
		set_up_timer(1, procrise_up_oubliette, reason_rising_up_oubliette);

		triptype &with = _vm->_trip->tr[1];
		with.face = _vm->_trip->left;
		with.x = 151;
		with.ix = -3;
		with.iy = -5;

		_vm->_gyro->background(2);
	}
}

void Timeout::rise_up_oubliette() {
	triptype &with = _vm->_trip->tr[1];

	with.visible = true;
	with.iy += 1; /* decrements dx/dy! */
	with.y -= with.iy; /* Uuuupppp we go... */
	if (with.iy > 0)
		set_up_timer(3, procrise_up_oubliette, reason_rising_up_oubliette);
	else
		_vm->_gyro->dna.user_moves_avvy = true;
}

void Timeout::robin_hood_and_geida() {
	_vm->_trip->tr[1].init(7, true, _vm->_trip);
	_vm->_trip->apped(1, 7);
	_vm->_trip->tr[1].walkto(6);
	_vm->_trip->tr[2].stopwalk();
	_vm->_trip->tr[2].face = _vm->_trip->left;
	set_up_timer(20, procrobin_hood_and_geida_talk, reason_robin_hood_and_geida);
	_vm->_gyro->dna.geida_follows = false;
}

void Timeout::robin_hood_and_geida_talk() {
	_vm->_visa->dixi('q', 66);
	_vm->_trip->tr[1].walkto(2);
	_vm->_trip->tr[2].walkto(2);
	_vm->_trip->tr[1].vanishifstill = true;
	_vm->_trip->tr[2].vanishifstill = true;
	set_up_timer(162, procavalot_returns, reason_robin_hood_and_geida);
}

void Timeout::avalot_returns() {
	_vm->_trip->tr[1].done();
	_vm->_trip->tr[2].done();
	_vm->_trip->tr[1].init(0, true, _vm->_trip);
	_vm->_trip->apped(1, 1);
	_vm->_visa->dixi('q', 67);
	_vm->_gyro->dna.user_moves_avvy = true;
}

void Timeout::avvy_sit_down() {
/* This is used when you sit down in the pub in Notts. It loops around so
	that it will happen when Avvy stops walking. */
	if (_vm->_trip->tr[1].homing)    /* Still walking */
		set_up_timer(1, procavvy_sit_down, reason_sitting_down);
	else {
		_vm->_celer->show_one(3);
		_vm->_gyro->dna.sitting_in_pub = true;
		_vm->_gyro->dna.user_moves_avvy = false;
		_vm->_trip->tr[1].visible = false;
	}
}

void Timeout::ghost_room_phew() {
	_vm->_scrolls->display("\6PHEW!\22 You're glad to get out of \6there!");
}

void Timeout::arkata_shouts() {
	if (_vm->_gyro->dna.teetotal)
		return;

	_vm->_visa->dixi('q', 76);
	
	set_up_timer(160, procarkata_shouts, reason_arkata_shouts);
}

void Timeout::winning() {
	_vm->_visa->dixi('q', 79);
	_vm->_pingo->winning_pic();

	do {
		_vm->_lucerna->checkclick();
	} while (!(_vm->_gyro->mrelease == 0));
	_vm->_lucerna->callverb(_vm->_acci->vb_score);
	_vm->_scrolls->display(" T H E    E N D ");
	_vm->_gyro->lmo = true;
}

void Timeout::avalot_falls() {
	if (_vm->_trip->tr[0].step < 5) {
		_vm->_trip->tr[0].step += 1;
		set_up_timer(3, procavalot_falls, reason_falling_over);
	} else
		//_vm->_scrolls->display("\r\r\r\r\r\r\n\n\n\n\n\n\23Z\26");

	warning("STUB: Timeout::avalot_falls()");
}

void Timeout::spludwick_goes_to_cauldron() {
	if (_vm->_trip->tr[2].homing)
		set_up_timer(1, procspludwick_goes_to_cauldron, reason_spludwalk);
	else
		set_up_timer(17, procspludwick_leaves_cauldron, reason_spludwalk);
}

void Timeout::spludwick_leaves_cauldron() {
	_vm->_trip->tr[2].call_eachstep = true; /* So that normal procs will continue. */
}

void Timeout::give_lute_to_geida() {   /* Moved here from Acci. */
	_vm->_visa->dixi('Q', 86);
	_vm->_lucerna->points(4);
	_vm->_gyro->dna.lustie_is_asleep = true;
	_vm->_sequence->first_show(5);
	_vm->_sequence->then_show(6); /* He falls asleep... */
	_vm->_sequence->start_to_close(); /* Not really closing, but we're using the same procedure. */
}

/* "This is all!" */


} // End of namespace Avalanche.
