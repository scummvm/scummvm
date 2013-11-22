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
	void playNim();

private:
	AvalancheEngine *_vm;

	static const char * const names[2];
	byte old[3];
	byte stones[3];
	byte stonePic[4][23][7]; // Picture of Nimstone.
	byte turns;
	bool dogfoodsTurn;
	byte stonesLeft;
	bool clicked;
	byte row;
	byte number;
   	bool squeak;
	int8 mNum, mRow;

	void chalk(int x,int y, Common::String z);
	void setup();
   	void plotStone(byte x,byte y);
	void board();
	void startMove();
 	void showChanges();
   	void blip();
	void checkMouse();
  	void less();
	void takeSome();
   	void endOfGame();
   	void dogFood();
	bool find(byte x);
	void findAp(byte start,byte stepsize);
};

} // End of namespace Avalanche

#endif // AVALANCHE_NIM_H
