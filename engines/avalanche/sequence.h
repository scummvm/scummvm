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

#ifndef AVALANCHE_SEQUENCE_H
#define AVALANCHE_SEQUENCE_H

namespace Avalanche {
class AvalancheEngine;

class Sequence {
private:
	static const int16 kNowFlip = 177;
	static const int16 kSeqLength = 10;

	byte _seq[kSeqLength];

	Room _flipToWhere;
	byte _flipToPed;

	AvalancheEngine *_vm;

	void shoveLeft(); // This is called by Timer when it's time to do another frame. It shifts everything to the left.
	void init(byte what);
	void add(byte what);
	void switchRoom(Room where, byte ped);
	void startTimer();
	void startTimerImmobilized();

public:
	Sequence(AvalancheEngine *vm);
	void synchronize(Common::Serializer &sz);
	void resetVariables();
	void callSequencer();

	void startCupboardSeq();
	void startMusicRoomSeq();
	void startMusicRoomSeq2(Room whither, byte ped);
	void startGardenSeq();
	void startGeidaLuteSeq();
	void startWinSeq();
	void startNaughtyDukeSeq();
	void startLustiesSeq2(Room whither, byte ped);
	void startLustiesSeq3(Room whither, byte ped);
	void startHallSeq(Room whither, byte ped);
	void startCardiffSeq(Room whither, byte ped);
	void startOutsideSeq(Room whither, byte ped);
	void startDuckSeq();
	void startCardiffSeq2();
	void startDummySeq(Room whither, byte ped);
};

} // End of namespace Avalanche.

#endif // AVALANCHE_SEQUENCE_H
