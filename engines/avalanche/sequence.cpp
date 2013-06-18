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

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 SEQUENCE         The sequencer. */

#define __sequence_implementation__


#include "sequence.h"


/*#include "Gyro.h"*/
/*#include "Timeout.h"*/
/*#include "Celer.h"*/
/*#include "Trip5.h"*/

namespace Avalanche {

void then_show(byte what) {
	byte fv;
	;
	for (fv = 1; fv <= seq_length; fv ++)
		if (seq[fv] == 0) {
			;
			seq[fv] = what;
			return;
		}
}

void first_show(byte what) {
	;
	/* First, we need to blank out the entire array. */
	fillchar(seq, sizeof(seq), '\0');

	/* Then it's just the same as then_show. */
	then_show(what);

}

void then_flip(byte where, byte ped) {
	;
	then_show(now_flip);

	dna.flip_to_where = where;
	dna.flip_to_ped = ped;
}

void start_to_close() {
	;
	lose_timer(reason_sequencer);
	set_up_timer(7, procsequence, reason_sequencer);
}

void start_to_open() {
	;
	dna.user_moves_avvy = false; /* They can't move. */
	stopwalking; /* And they're not moving now. */
	start_to_close(); /* Apart from that, it's the same thing. */
}

void call_sequencer();

/* This proc is called by Timeout when it's time to do another frame. */
static void shove_left() {
	;
	move(seq[2], seq[1], seq_length - 1); /* Shift everything to the left. */
}

void call_sequencer() {
	;
	switch (seq[1]) {
	case 0:
		return;
		break; /* No more routines. */
	case 1 ... 176: {
		; /* Show a frame. */
		show_one(seq[1]);
		shove_left();
	}
	break;
	case 177: {
		;
		user_moves_avvy = true;
		fliproom(flip_to_where, flip_to_ped); /* 177 = Flip room. */
		if (seq[1] == 177)  shove_left();
	}
	break;
	}

	start_to_close(); /* Make sure this proc gets called again. */
}

} // End of namespace Avalanche.