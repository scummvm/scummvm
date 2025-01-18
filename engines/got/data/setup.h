/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GOT_DATA_SETUP_H
#define GOT_DATA_SETUP_H

#include "common/serializer.h"

namespace Got {

struct SetupFlags {
	bool _flags[64] = {};
	bool &f00 = _flags[0];
	bool &f01 = _flags[1];
	bool &f02 = _flags[2];
	bool &f03 = _flags[3];
	bool &f04 = _flags[4];
	bool &f05 = _flags[5];
	bool &f06 = _flags[6];
	bool &f07 = _flags[7];
	bool &f08 = _flags[8];
	bool &f09 = _flags[9];
	bool &f10 = _flags[10];
	bool &f11 = _flags[11];
	bool &f12 = _flags[12];
	bool &f13 = _flags[13];
	bool &f14 = _flags[14];
	bool &f15 = _flags[15];
	bool &f16 = _flags[16];
	bool &f17 = _flags[17];
	bool &f18 = _flags[18];
	bool &f19 = _flags[19];
	bool &f20 = _flags[20];
	bool &f21 = _flags[21];
	bool &f22 = _flags[22];
	bool &f23 = _flags[23];
	bool &f24 = _flags[24];
	bool &f25 = _flags[25];
	bool &f26 = _flags[26];
	bool &f27 = _flags[27];
	bool &f28 = _flags[28];
	bool &f29 = _flags[29];
	bool &f30 = _flags[30];
	bool &f31 = _flags[31];
	bool &f32 = _flags[32];
	bool &f33 = _flags[33];
	bool &f34 = _flags[34];
	bool &f35 = _flags[35];
	bool &f36 = _flags[36];
	bool &f37 = _flags[37];
	bool &f38 = _flags[38];
	bool &f39 = _flags[39];
	bool &f40 = _flags[40];
	bool &f41 = _flags[41];
	bool &f42 = _flags[42];
	bool &f43 = _flags[43];
	bool &f44 = _flags[44];
	bool &f45 = _flags[45];
	bool &f46 = _flags[46];
	bool &f47 = _flags[47];
	bool &f48 = _flags[48];
	bool &f49 = _flags[49];
	bool &f50 = _flags[50];
	bool &f51 = _flags[51];
	bool &f52 = _flags[52];
	bool &f53 = _flags[53];
	bool &f54 = _flags[54];
	bool &f55 = _flags[55];
	bool &f56 = _flags[56];
	bool &f57 = _flags[57];
	bool &f58 = _flags[58];
	bool &f59 = _flags[59];
	bool &f60 = _flags[60];
	bool &f61 = _flags[61];
	bool &f62 = _flags[62];
	bool &f63 = _flags[63];

	SetupFlags() {}
	virtual ~SetupFlags() {}
	virtual void sync(Common::Serializer &s);
	SetupFlags &operator=(const Got::SetupFlags &src);
};

struct Setup : public SetupFlags {
	byte _value[16] = {};
	byte _filler1 = 0;
	byte _game = 0;             // Unused
	byte _areaNum = 0;          // 1,2,3
	bool _speakerSound = false; // always disabled
	bool _digitalSound = false; // true = enabled
	bool _musicEnabled = false; // true = enabled
	bool _slowMode = false;     // true = slow mode (for slower 286's)
	bool _scrollFlag = false;   // true = scroll when changing from	a room to the other
	bool _bossDead[3] = {false, false, false};
	byte _difficultyLevel = 0;  // 0=easy, 1=normal, 2=hard
	byte _gameOver = 0;
	byte _filler2[19] = {};
	
	void sync(Common::Serializer &s) override;
};

} // namespace Got

#endif
