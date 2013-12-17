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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#ifndef AVALANCHE_NIM_H
#define AVALANCHE_NIM_H

namespace Avalanche {

class Nim {
public:
	Nim(AvalancheEngine *vm);
	void resetVariables();
	void synchronize(Common::Serializer &sz);
	void playNim();

private:
	AvalancheEngine *_vm;

	static const char * const kNames[2];

	byte _old[3];
	byte _stones[3];
	byte _turns;
	bool _dogfoodsTurn;
	byte _stonesLeft;
	bool _clicked;
	byte _row;
	byte _number;
	bool _squeak;
	int8 _mNum, _mRow;
	byte _playedNim; // How many times you've played Nim.

	// Inner variables for dogFood(), find() and findAp().
	bool _inAp[3];
	bool _lmo; // Let Me Out!
	byte _r[3];

	void chalk(int x, int y, Common::String text);
	void setup();
	void board();
	void startMove();
	void showChanges();
	void blip();
	void checkMouse();
	void less();
	void takeSome();
	void endOfGame();
	void dogFood();
	bool find(byte x); // This gives TRUE if there's a pile with x stones in.
	void findAp(byte start, byte stepSize);
};

} // End of namespace Avalanche

#endif // AVALANCHE_NIM_H
