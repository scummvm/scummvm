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

/* SEQUENCE		The sequencer. */

#include "avalanche/sequence2.h"
#include "common/scummsys.h"
#include "avalanche/gyro2.h"
#include "avalanche/timeout2.h"
#include "avalanche/celer2.h"
#include "avalanche/trip6.h"

namespace Avalanche {

	namespace Sequence {

	void then_show(byte what) {
		byte fv;
		for (fv = 1; fv <= seq_length; fv ++)
			if (seq[fv] == 0) {
				seq[fv] = what;
				return;
			}
	}

	void first_show(byte what) {
		/* First, we need to blank out the entire array. */
		for (int i = 0; i < sizeof(seq); i++)
			seq[i] = 0;

		/* Then it's just the same as then_show. */
		then_show(what);

	}

	void then_flip(byte where, byte ped) {
		then_show(now_flip);

		Gyro::dna.flip_to_where = where;
		Gyro::dna.flip_to_ped = ped;
	}

	void start_to_close() {
		Timeout::lose_timer(Timeout::reason_sequencer);
		Timeout::set_up_timer(7, Timeout::PROCsequence, Timeout::reason_sequencer);
	}

	void start_to_open() {
		Gyro::dna.user_moves_avvy = false; /* They can't move. */
		Trip::stopwalking(); /* And they're not moving now. */
		start_to_close(); /* Apart from that, it's the same thing. */
	}

	void call_sequencer();

	/* This PROC is called by Timeout when it's time to do another frame. */
	static void shove_left() {
		memcpy(seq, seq+1, seq_length - 1); /* Shift everything to the left. */
	}

	void call_sequencer() {
		switch (seq[0]) {
		case 0:
			return;
			break; /* No more routines. */
		case 177: {
			Gyro::dna.user_moves_avvy = true;
			Trip::fliproom(Gyro::dna.flip_to_where, Gyro::dna.flip_to_ped); /* 177 = Flip room. */
			if (seq[0] == 177)  shove_left();
			}
			break;
		}

		if ((seq[0] >= 1) && (seq[0] <= 176)) {
			/* Show a frame. */
			Celer::show_one(seq[1]);
			shove_left();
		}
	
		start_to_close(); /* Make sure this PROC gets called again. */
	}

	} // End of namespace Sequence .

} // End of namespace Avalanche.
