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

#ifndef AVALANCHE_SEQUENCE2_H
#define AVALANCHE_SEQUENCE2_H

#include "common/scummsys.h"

namespace Avalanche {
class AvalancheEngine;

class Sequence {
public:
	static const int16 kNowFlip = 177;
	static const int16 kSeqLength = 10;

	byte _seq[kSeqLength];

	Sequence(AvalancheEngine *vm);

	void firstShow(byte what);
	void thenShow(byte what);
	void thenFlip(byte where, byte ped);
	void startToClose();
	void startToOpen();
	void callSequencer();

private:
	AvalancheEngine *_vm;

	void shoveLeft(); // This PROC is called by Timeout when it's time to do another frame.
};

} // End of namespace Avalanche.

#endif // AVALANCHE_SEQUENCE2_H
