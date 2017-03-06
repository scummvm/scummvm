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

#include "titanic/star_control/star_control_sub5.h"
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/titanic.h"

namespace Titanic {

void CStarControlSub5::SubEntry::clear() {
	if (_data2P) {
		delete[] _data2P;
		_data2P = nullptr;
		_field4 = 0;
	}

	if (_data1P) {
		delete[] _data1P;
		_data1P = nullptr;
		_field0 = 0;
	}
}

/*------------------------------------------------------------------------*/

bool CStarControlSub5::MemoryBlock::allocate() {
	// TODO
	return true;
}

/*------------------------------------------------------------------------*/

CStarControlSub5::CStarControlSub5() : _flag(true), _dataP(nullptr),
		_field78B0(0) {
}

CStarControlSub5::~CStarControlSub5() {
	delete[] _dataP;
	_dataP = nullptr;
}

bool CStarControlSub5::setup() {
	bool success = setupEntry(5, 5, 4, 1024.0)
		&& setupEntry(7, 7, 3, 1024.0)
		&& setupEntry(8, 8, 2, 1024.0)
		&& setupEntry(16, 16, 1, 1024.0)
		&& setupEntry(24, 24, 0, 1024.0);
	if (success)
		success = setup2(24, 24);

	return success;
}

bool CStarControlSub5::setup2(int val1, int val2) {
	// TODO: Original set an explicit random seed here. Could be
	// problematic if following random values need to be deterministic
	double FACTOR = 3.1415927 * 0.0055555557;
	const int VALUES1[] = { 0x800, 0xC00, 0x1000, 0x1400, 0x1800 };
	const int VALUES2[] = {
		0xF95BCD, 0xA505A0, 0xFFAD43, 0x98F4EB, 0xF3EFA5, 0,
		0xFFFFFF, 0x81EEF5, 0x5FFD3, 0x4EE4FA, 0x11C3FF, 0x28F3F4, 
		0x36FCF2, 0x29F1FD, 0x29BCFD, 0x98E3F4, 0xBBF3D9, 0x8198F5, 
		0x5BE4F9, 0x0D6E2, 0x74EEF6, 0x68DEF8 
	};

	Entry *e = &_entries[0];
	for (int idx = 0; idx < 256; ++idx) {
		if (idx == 0) {
			e->_field0 = 0x4C8;
			e->_field4 = 0x40;
			e->_field5 = 0x40;
			e->_field6 = 0x40;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 7.0;
			e->_field14 = 0.0084687499;

			++e;
			e->_field0 = 0x574;
			e->_field4 = 0x7f;
			e->_field5 = 0;
			e->_field6 = 0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 3.0;
			e->_field14 = 0.021011719;

			++e;
			e->_field0 = 0x603;
			e->_field4 = 0;
			e->_field5 = 0;
			e->_field6 = 0xff;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = 0;
			e->_field14 = 0.022144532;

			++e;
			e->_field0 = 0x712;
			e->_field4 = 0xff;
			e->_field5 = 0;
			e->_field6 = 0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 2.0;
			e->_field14 = 0.01178125;

			++e;
			e->_field0 = 0xe7f;
			e->_field4 = 0xe6;
			e->_field5 = 0xbe;
			e->_field6 = 0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 1.0;
			e->_field14 = 0.24791406;

			++e;
			e->_field0 = 0x173f;
			e->_field4 = 0xf0;
			e->_field5 = 0xf0;
			e->_field6 = 0xe6;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 3.0;
			e->_field14 = 0.20832032;

			++e;
			e->_field0 = 0x2ab8;
			e->_field4 = 0x28;
			e->_field5 = 0x32;
			e->_field6 = 0x28;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 1.0;
			e->_field14 = 0.088164061;

			++e;
			e->_field0 = 0x40ac;
			e->_field4 = 0x0;
			e->_field5 = 0xbe;
			e->_field6 = 0xf0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 2.0;
			e->_field14 = 0.084375001;

			++e;
			e->_field0 = 0x539c;
			e->_field4 = 0x20;
			e->_field5 = 0x20;
			e->_field6 = 0x20;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 17.0;
			e->_field14 = 0.00390625;
		} else {
			for (int ctr = 0; ctr < 5; ++ctr) {
				e->_field0 = static_cast<int>(g_vm->getRandomFloat() * 1350.0
					- 675.0) + VALUES1[idx];
				int val = VALUES2[g_vm->getRandomNumber(15)];
				e->_field4 = val & 0xff;
				e->_field5 = (val >> 8) & 0xff;
				e->_field6 = (val >> 16) & 0xff;
				e->_field8 = g_vm->getRandomNumber(3) + 3;
				
				e->_fieldC = g_vm->getRandomNumber(255);
				e->_field10 = FACTOR * (double)g_vm->getRandomNumber(15);
				e->_field14 = ((double)g_vm->getRandomNumber(0xffffffff)
					* 50.0 * 0.000015259022) * 0.00390625;
			}
		}
	}

	if (_memoryBlock.allocate()) {
		_dataP = new byte[((val2 - 2) * val1 + 2) * 24];
		return true;
	}

	return false;
}

void CStarControlSub5::proc2(CStarControlSub6 *sub6, FVector *vector, double v1, double v2, double v3,
		CSurfaceArea *surfaceArea, CStarControlSub12 *sub12) {
	// TODO
}

void CStarControlSub5::proc3(CErrorCode *errorCode) {
	++_field78B0;
	errorCode->set();
}

void CStarControlSub5::fn1() {
	_flag = !_flag;
}

bool CStarControlSub5::setupEntry(int val1, int val2, int index, double val3) {
	// TODO
	return true;
}

} // End of namespace Titanic
