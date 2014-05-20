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

#ifndef AVALANCHE_SHOOTEMUP_H
#define AVALANCHE_SHOOTEMUP_H

namespace Avalanche {
class AvalancheEngine;

class ShootEmUp {
public:
	ShootEmUp(AvalancheEngine *vm);

	uint16 run();

private:
	struct Sprite {
		int8 _ix, _iy;
		int16 _x, _y;
		int8 _p;
		int16 _timeout;
		bool _cameo;
		byte _cameoFrame;
		bool _missile;
		bool _wipe;
	};

	struct Runner {
		int16 _x, _y;
		byte _frame;
		byte _tooHigh;
		byte _lowest;
		int8 _ix, _iy;
		byte _frameDelay;
	};

	static const byte kStocks;
	static const byte kAvvyShoots;
	static const byte kFacingRight;
	static const byte kFacingLeft;
	static const long int kFlag;
	static const byte kFrameDelayMax;
	static const byte kAvvyY;
	static const byte kShooting[7];
	static const byte kTimesASecond;
	static const byte kFlashTime;
	static const byte kLeftMargin;
	static const int16 kRightMargin;

	AvalancheEngine *_vm;

	uint16 _score;
	byte _time;
	byte _stockStatus[7];
	Sprite _sprites[99];
	byte _rectNum; // Original: 'rsize'
	Common::Rect _rectangles[99];
	uint16 _avvyWas;
	uint16 _avvyPos;
	byte _avvyAnim;
	byte _avvyFacing;
	bool _altWasPressedBefore;
	byte _throwNext;
	bool _firing;
	Runner _running[4];
	bool _hasEscaped[7];
	byte _count321;
	byte _howManyHaveEscaped;
	uint16 _escapeCount;
	bool _escaping;
	byte _timeThisSecond;
	bool _cp;
	byte _wasFacing;
	byte _escapeStock;
	bool _gotOut;

	bool overlap(uint16 a1x, uint16 a1y, uint16 a2x, uint16 a2y, uint16 b1x, uint16 b1y, uint16 b2x, uint16 b2y);
	byte getStockNumber(byte index);
	void blankIt();
	void moveThem();
	void blank(Common::Rect rect);
	void plotThem();
	void define(int16 x, int16 y, int8 p, int8 ix, int8 iy, int16 time, bool isAMissile, bool doWeWipe);
	void defineCameo(int16 x, int16 y, int8 p, int16 time);
	void showStock(byte index);
	void drawNumber(int number, int size, int x);
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
