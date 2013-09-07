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

#ifndef AVALANCHE_TIMEOUT2_H
#define AVALANCHE_TIMEOUT2_H

#include "common/scummsys.h"

namespace Avalanche {
class AvalancheEngine;

class Timeout {
public:
	// reason_ now runs between 1 and 28.

	static const int16 reason_drawbridgefalls = 2;
	static const int16 reason_avariciustalks = 3;
	static const int16 reason_gototoilet = 4;
	static const int16 reason_explosion = 5;
	static const int16 reason_brummiestairs = 6;
	static const int16 reason_cardiffsurvey = 7;
	static const int16 reason_cwytalot_in_herts = 8;
	static const int16 reason_getting_tied_up = 9;
	static const int16 reason_hanging_around = 10; // Tied to the tree in Nottingham.
	static const int16 reason_jacques_waking_up = 11;
	static const int16 reason_naughty_duke = 12;
	static const int16 reason_jumping = 13;
	static const int16 reason_sequencer = 14;
	static const int16 reason_crapulus_says_spludwick_out = 15;
	static const int16 reason_dawndelay = 16;
	static const int16 reason_drinks = 17;
	static const int16 reason_du_lustie_talks = 18;
	static const int16 reason_falling_down_oubliette = 19;
	static const int16 reason_meeting_avaroid = 20;
	static const int16 reason_rising_up_oubliette = 21;
	static const int16 reason_robin_hood_and_geida = 22;
	static const int16 reason_sitting_down = 23;
	static const int16 reason_ghost_room_phew = 1;
	static const int16 reason_arkata_shouts = 24;
	static const int16 reason_winning = 25;
	static const int16 reason_falling_over = 26;
	static const int16 reason_spludwalk = 27;
	static const int16 reason_geida_sings = 28;

	// procx now runs between 1 and 41.

	static const int16 procopen_drawbridge = 3;
	static const int16 procavaricius_talks = 4;
	static const int16 procurinate = 5;
	static const int16 proctoilet2 = 6;
	static const int16 procbang = 7;
	static const int16 procbang2 = 8;
	static const int16 procstairs = 9;
	static const int16 proccardiffsurvey = 10;
	static const int16 proccardiff_return = 11;
	static const int16 proc_cwytalot_in_herts = 12;
	static const int16 procget_tied_up = 13;
	static const int16 procget_tied_up2 = 1;
	static const int16 prochang_around = 14;
	static const int16 prochang_around2 = 15;
	static const int16 procafter_the_shootemup = 32;
	static const int16 procjacques_wakes_up = 16;
	static const int16 procnaughty_duke = 17;
	static const int16 procnaughty_duke2 = 18;
	static const int16 procnaughty_duke3 = 38;
	static const int16 procjump = 19;
	static const int16 procsequence = 20;
	static const int16 proccrapulus_splud_out = 21;
	static const int16 procdawn_delay = 22;
	static const int16 procbuydrinks = 23;
	static const int16 procbuywine = 24;
	static const int16 proccallsguards = 25;
	static const int16 procgreetsmonk = 26;
	static const int16 procfall_down_oubliette = 27;
	static const int16 procmeet_avaroid = 28;
	static const int16 procrise_up_oubliette = 29;
	static const int16 procrobin_hood_and_geida = 2;
	static const int16 procrobin_hood_and_geida_talk = 30;
	static const int16 procavalot_returns = 31;
	static const int16 procavvy_sit_down = 33; // In Nottingham.
	static const int16 procghost_room_phew = 34;
	static const int16 procarkata_shouts = 35;
	static const int16 procwinning = 36;
	static const int16 procavalot_falls = 37;
	static const int16 procspludwick_goes_to_cauldron = 39;
	static const int16 procspludwick_leaves_cauldron = 40;
	static const int16 procgive_lute_to_geida = 41;

	struct timetype {
		int32 time_left;
		byte then_where;
		byte what_for;
	};



	timetype times[7];

	bool timerLost; // Is the timer "lost"? (Because of using lose_timer())



	Timeout(AvalancheEngine *vm);

	void setParent(AvalancheEngine *vm);

	void set_up_timer(int32 howlong, byte whither, byte why);

	void one_tick();

	void lose_timer(byte which);

	// Procedures to do things at the end of amounts of time:

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

private:
	AvalancheEngine *_vm;
};

} // End of namespace Avalanche.

#endif // AVALANCHE_TIMEOUT2_H
