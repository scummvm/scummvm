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
#include "avalanche/sequence.h"
#include "avalanche/gyro.h"
#include "avalanche/timer.h"
#include "avalanche/background.h"
#include "avalanche/animation.h"

#include "common/scummsys.h"

namespace Avalanche {

Sequence::Sequence(AvalancheEngine *vm) {
	_vm = vm;
}

void Sequence::firstShow(byte what) {
	_seq[0] = what;

	for (uint i = 1; i < kSeqLength; i++)
		_seq[i] = 0;
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

	_vm->_gyro->_flipToWhere = where;
	_vm->_gyro->_flipToPed = ped;
}

void Sequence::startToClose() {
	_vm->_timer->loseTimer(Timer::kReasonSequencer);
	_vm->_timer->addTimer(7, Timer::kProcSequence, Timer::kReasonSequencer);
}

void Sequence::startToOpen() {
	_vm->_gyro->_userMovesAvvy = false; // They can't move.
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
		_vm->_gyro->_userMovesAvvy = true;
		_vm->_animation->flipRoom(_vm->_gyro->_flipToWhere, _vm->_gyro->_flipToPed);
		if (_seq[0] == 177)
			shoveLeft();
		break;
	}

	if ((_seq[0] >= 1) && (_seq[0] <= 176)) {
		// Show a frame.
		_vm->_background->drawBackgroundSprite(-1, -1, _seq[0] - 1);
		shoveLeft();
	}

	startToClose(); // Make sure this PROC gets called again.
}

} // End of namespace Avalanche.
