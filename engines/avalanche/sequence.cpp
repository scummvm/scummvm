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

	for (int i = 1; i < kSeqLength; i++)
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

void Sequence::thenFlip(Room where, byte ped) {
	thenShow(kNowFlip);

	_vm->_avalot->_flipToWhere = where;
	_vm->_avalot->_flipToPed = ped;
}

void Sequence::startToClose() {
	_vm->_timer->loseTimer(Timer::kReasonSequencer);
	_vm->_timer->addTimer(7, Timer::kProcSequence, Timer::kReasonSequencer);
}

void Sequence::startToOpen() {
	_vm->_avalot->_userMovesAvvy = false; // They can't move.
	_vm->_animation->stopWalking(); // And they're not moving now.
	startToClose(); // Apart from that, it's the same thing.
}

void Sequence::shoveLeft() {
	for (uint i = 0; i < kSeqLength - 1; i++)
		_seq[i] = _seq[i + 1];
	_seq[kSeqLength - 1] = 0;
}

void Sequence::callSequencer() {
	byte curSeq = _seq[0];

	switch (curSeq) {
	case 0:
		return; // No more routines.
		break;
	case 177: // Flip room.
		_vm->_avalot->_userMovesAvvy = true;
		_vm->_avalot->flipRoom(_vm->_avalot->_flipToWhere, _vm->_avalot->_flipToPed);
		if (curSeq == 177)
			shoveLeft();
		break;
	}

	if (curSeq <= 176) { // Show a frame.
		_vm->_background->draw(-1, -1, curSeq - 1);
		shoveLeft();
	}

	startToClose(); // Make sure this PROC gets called again.
}

void Sequence::startHallSeq(Room whither, byte ped) {
	firstShow(1);
	thenShow(2);
	thenFlip(whither, ped);
	startToOpen();
}

void Sequence::startOutsideSeq(Room whither, byte ped) {
	firstShow(1);
	thenShow(2);
	thenShow(3);
	thenFlip(whither, ped);
	startToOpen();
}

void Sequence::startCardiffSeq(Room whither, byte ped) {
	firstShow(1);
	thenShow(5);
	thenFlip(whither, ped);
	startToOpen();
}

void Sequence::startNaughtyDukeSeq() {
	firstShow(2);
	startToClose();
}

void Sequence::startGardenSeq() {
	firstShow(2);
	thenShow(1);
	thenShow(3);
	startToClose();
}

// FIXME! 2 firstShow()!
void Sequence::startDuckSeq() {
	firstShow(3);
	firstShow(2);
	thenShow(1);
	thenShow(4);
	startToClose();
}

void Sequence::startNottsSeq() {
	firstShow(3);
	thenShow(2);
	thenShow(1);
	thenShow(4);
	startToClose();
}

void Sequence::startLustiesSeq3(Room whither, byte ped) {
	firstShow(4);
	thenShow(5);
	thenShow(6);
	thenFlip(whither, ped);
	startToOpen();
}

void Sequence::startMusicRoomSeq2(Room whither, byte ped) {
	firstShow(5);
	thenShow(6);
	thenFlip(whither, ped);
	startToOpen();
}

void Sequence::startGeidaLuteSeq() {
	firstShow(5);
	thenShow(6); // He falls asleep...
	startToClose(); // Not really closing, but we're using the same procedure.
}

void Sequence::startMusicRoomSeq() {
	firstShow(6);
	thenShow(5);
	thenShow(7);
	startToClose();
}

void Sequence::startWinSeq() {
	firstShow(7);
	thenShow(8);
	thenShow(9);
	startToClose();
}

void Sequence::startCupboardSeq() {
	firstShow(8);
	thenShow(7);
	startToClose();
}

void Sequence::startLustiesSeq1() {
	firstShow(8);
	thenShow(7);
	startToClose();
}

void Sequence::startLustiesSeq2(Room whither, byte ped) {
	firstShow(8);
	thenShow(9);
	thenFlip(whither, ped);
	startToOpen();
}

void Sequence::startCardiffSeq2() {
	firstShow(1);
	if (_vm->_avalot->_arrowInTheDoor)
		thenShow(3);
	else
		thenShow(2);

	if (_vm->_avalot->_takenPen)
		_vm->_background->draw(-1, -1, 3);

	startToClose();
}

void Sequence::startDummySeq(Room whither, byte ped) {
	thenFlip(whither, ped);
	startToOpen();
}

void Sequence::synchronize(Common::Serializer &sz) {
	sz.syncBytes(_seq, kSeqLength);
}
} // End of namespace Avalanche.
