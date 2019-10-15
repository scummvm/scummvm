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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

namespace Avalanche {

Sequence::Sequence(AvalancheEngine *vm) {
	_vm = vm;

	resetVariables();
}

void Sequence::resetVariables() {
	_flipToWhere = kRoomNowhere;
	_flipToPed = 0;
}

void Sequence::init(byte what) {
	_seq[0] = what;

	for (int i = 1; i < kSeqLength; i++)
		_seq[i] = 0;
}

void Sequence::add(byte what) {
	for (int16 i = 0; i < kSeqLength; i++) {
		if (_seq[i] == 0) {
			_seq[i] = what;
			return;
		}
	}
}

void Sequence::switchRoom(Room where, byte ped) {
	add(kNowFlip);

	_flipToWhere = where;
	_flipToPed = ped;
}

void Sequence::startTimer() {
	_vm->_timer->loseTimer(Timer::kReasonSequencer);
	_vm->_timer->addTimer(7, Timer::kProcSequence, Timer::kReasonSequencer);
}

void Sequence::startTimerImmobilized() {
	// They can't move.
	_vm->_userMovesAvvy = false;
	// And they're not moving now.
	_vm->_animation->stopWalking();
	// Apart from that, it's the same thing.
	startTimer();
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
		// No more routines.
		return;
		break;
	case kNowFlip:
		// Flip room.
		_vm->_userMovesAvvy = true;
		_vm->flipRoom(_flipToWhere, _flipToPed);
		shoveLeft();
		break;
	default:
		break;
	}

	if (curSeq <= 176) {
		// Show a frame.
		_vm->_background->draw(-1, -1, curSeq - 1);
		shoveLeft();
	}

	// Make sure this PROC gets called again.
	startTimer();
}

void Sequence::startHallSeq(Room whither, byte ped) {
	init(1);
	add(2);
	switchRoom(whither, ped);
	startTimerImmobilized();
}

void Sequence::startOutsideSeq(Room whither, byte ped) {
	init(1);
	add(2);
	add(3);
	switchRoom(whither, ped);
	startTimerImmobilized();
}

void Sequence::startCardiffSeq(Room whither, byte ped) {
	init(1);
	add(5);
	switchRoom(whither, ped);
	startTimerImmobilized();
}

void Sequence::startNaughtyDukeSeq() {
	init(2);
	startTimer();
}

void Sequence::startGardenSeq() {
	init(2);
	add(1);
	add(3);
	startTimer();
}

void Sequence::startDuckSeq() {
	init(3);
	add(2);
	add(1);
	add(4);
	startTimer();
}

void Sequence::startLustiesSeq3(Room whither, byte ped) {
	init(4);
	add(5);
	add(6);
	switchRoom(whither, ped);
	startTimerImmobilized();
}

void Sequence::startMusicRoomSeq2(Room whither, byte ped) {
	init(5);
	add(6);
	switchRoom(whither, ped);
	startTimerImmobilized();
}

void Sequence::startGeidaLuteSeq() {
	init(5);
	// He falls asleep...
	add(6);
	// Not really closing, but we're using the same procedure.
	startTimer();
}

void Sequence::startMusicRoomSeq() {
	init(6);
	add(5);
	add(7);
	startTimer();
}

void Sequence::startWinSeq() {
	init(7);
	add(8);
	add(9);
	startTimer();
}

void Sequence::startCupboardSeq() {
	init(8);
	add(7);
	startTimer();
}

void Sequence::startLustiesSeq2(Room whither, byte ped) {
	init(8);
	add(9);
	switchRoom(whither, ped);
	startTimerImmobilized();
}

void Sequence::startCardiffSeq2() {
	init(1);
	if (_vm->_arrowInTheDoor)
		add(3);
	else
		add(2);

	if (_vm->_takenPen)
		_vm->_background->draw(-1, -1, 3);

	startTimer();
}

void Sequence::startDummySeq(Room whither, byte ped) {
	switchRoom(whither, ped);
	startTimerImmobilized();
}

void Sequence::synchronize(Common::Serializer &sz) {
	sz.syncBytes(_seq, kSeqLength);
	sz.syncAsByte(_flipToWhere);
	sz.syncAsByte(_flipToPed);
}
} // End of namespace Avalanche.
