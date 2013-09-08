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
#include "avalanche/animation.h"

#include "common/scummsys.h"

namespace Avalanche {

Sequence::Sequence(AvalancheEngine *vm) {
	_vm = vm;
}

void Sequence::firstShow(byte what) {
	// First, we need to blank out the entire array.
	for (uint i = 0; i < sizeof(_seq); i++)
		_seq[i] = 0;

	// Then it's just the same as thenShow.
	thenShow(what);
}

void Sequence::thenShow(byte what) {
	for (int16 i = 0; i < kSeqLength; i++) {
		if (_seq[i] == 0) {
			_seq[i] = what;
			return;
		}
	}
}

void Sequence::thenFlip(byte where, byte ped) {
	thenShow(kNowFlip);

	_vm->_gyro->_dna._flipToWhere = where;
	_vm->_gyro->_dna._flipToPed = ped;
}

void Sequence::startToClose() {
	_vm->_timeout->loseTimer(_vm->_timeout->kReasonSequencer);
	_vm->_timeout->addTimer(7, _vm->_timeout->kProcSequence, _vm->_timeout->kReasonSequencer);
}

void Sequence::startToOpen() {
	_vm->_gyro->_dna._userMovesAvvy = false; // They can't move.
	_vm->_animation->stopWalking(); // And they're not moving now.
	startToClose(); // Apart from that, it's the same thing.
}

void Sequence::shoveLeft() {
	memcpy(_seq, _seq+1, kSeqLength - 1); // Shift everything to the left.
}

void Sequence::callSequencer() {
	switch (_seq[0]) {
	case 0:
		return; // No more routines.
		break;
	case 177: // Flip room.
		_vm->_gyro->_dna._userMovesAvvy = true;
		_vm->_animation->fliproom(_vm->_gyro->_dna._flipToWhere, _vm->_gyro->_dna._flipToPed);
		if (_seq[0] == 177)
			shoveLeft();
		break;
	}

	if ((_seq[0] >= 1) && (_seq[0] <= 176)) {
		// Show a frame.
		_vm->_celer->drawBackgroundSprite(-1, -1, _seq[0]);
		shoveLeft();
	}

	startToClose(); // Make sure this PROC gets called again.
}

} // End of namespace Avalanche.
