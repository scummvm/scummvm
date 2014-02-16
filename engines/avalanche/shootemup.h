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

#ifndef AVALANCHE_SHOOTEMUP_H
#define AVALANCHE_SHOOTEMUP_H

namespace Avalanche {
class AvalancheEngine;

class ShootEmUp {
public:
	ShootEmUp(AvalancheEngine *vm);

	void run();

private:
	static const byte kFacingRight;
	static const byte kStocks;

	AvalancheEngine *_vm;

	byte _time;

	bool overlap(uint16 a1x, uint16 a1y, uint16 a2x, uint16 a2y, uint16 b1x, uint16 b1y, uint16 b2x, uint16 b2y);
	byte getStockNumber(byte x);
	void blankIt();
	void moveThem();
	void plotThem();
	void define(int16 xx, int16 yy, byte pp, int8 ixx, int8 iyy, int16 time, bool isAMissile, bool doWeWipe);
	void defineCameo(int16 xx, int16 yy, byte pp, int16 time);
	void showStock(byte x);
	void showScore();
	void showTime();
	void gain(int8 howMuch);
	void newEscape();
	void nextPage(); // Internal function of 'instructions' in the original.
	void instructions();
	void setup();
	void initRunner(int16 xx, int16 yy, byte f1, byte f2, int8 ixx, int8 iyy);
	void moveAvvy();
	void readKbd();
	void animate();
	void collisionCheck();
	void turnAround(byte who, bool randomX);
	void bumpFolk();
	void peopleRunning();
	void updateTime();
	void hitPeople();
	void escapeCheck();
	void check321();
};

} // End of namespace Avalanche

#endif // AVALANCHE_SHOOTEMUP_H
