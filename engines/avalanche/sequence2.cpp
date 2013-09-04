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

#include "avalanche/avalanche.h"

#include "avalanche/sequence2.h"
#include "avalanche/gyro2.h"
#include "avalanche/timeout2.h"
#include "avalanche/celer2.h"
#include "avalanche/trip6.h"

#include "common/scummsys.h"

namespace Avalanche {

Sequence::Sequence(AvalancheEngine *vm) {
	_vm = vm;
}

void Sequence::first_show(byte what) {
	/* First, we need to blank out the entire array. */
	for (uint i = 0; i < sizeof(seq); i++)
		seq[i] = 0;

	/* Then it's just the same as then_show. */
	then_show(what);

}

void Sequence::then_show(byte what) {
	for (int16 fv = 0; fv < seq_length; fv++) {
		if (seq[fv] == 0) {
			seq[fv] = what;
			return;
		}
	}
}

void Sequence::then_flip(byte where, byte ped) {
	then_show(now_flip);

	_vm->_gyro->dna.flip_to_where = where;
	_vm->_gyro->dna.flip_to_ped = ped;
}

void Sequence::start_to_close() {
	_vm->_timeout->lose_timer(_vm->_timeout->reason_sequencer);
	_vm->_timeout->set_up_timer(7, _vm->_timeout->procsequence, _vm->_timeout->reason_sequencer);
}

void Sequence::start_to_open() {
	_vm->_gyro->dna.user_moves_avvy = false; /* They can't move. */
	_vm->_trip->stopwalking(); /* And they're not moving now. */
	start_to_close(); /* Apart from that, it's the same thing. */
}



/* This PROC is called by Timeout when it's time to do another frame. */
void Sequence::shove_left() {
	memcpy(seq, seq+1, seq_length - 1); /* Shift everything to the left. */
}

void Sequence::call_sequencer() {
	switch (seq[0]) {
	case 0:
		return; // No more routines.
		break; 
	case 177: // Flip room.
		_vm->_gyro->dna.user_moves_avvy = true;
		_vm->_trip->fliproom(_vm->_gyro->dna.flip_to_where, _vm->_gyro->dna.flip_to_ped); 
		if (seq[0] == 177)
			shove_left();
		break;
	}

	if ((seq[0] >= 1) && (seq[0] <= 176)) {
		/* Show a frame. */
		_vm->_celer->drawBackgroundSprite(-1, -1, seq[0]);
		shove_left();
	}
	
	start_to_close(); /* Make sure this PROC gets called again. */
}

} // End of namespace Avalanche.
