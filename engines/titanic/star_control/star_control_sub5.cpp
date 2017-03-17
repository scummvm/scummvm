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

#define MKTAG_BE(a3,a2,a1,a0) ((uint32)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

void CStarControlSub5::SubEntry::clear() {
	_data1.clear();
	_data2.clear();
}

/*------------------------------------------------------------------------*/

bool CStarControlSub5::SineTable::setup() {
	if (_data.empty()) {
		_data.resize(1024);
		for (int idx = 0; idx < 1024; ++idx)
			_data[idx] = sin((double)idx * 2 * M_PI / 512.0);
	}

	return true;
}

/*------------------------------------------------------------------------*/

CStarControlSub5::CStarControlSub5() : _flag(true), _multiplier(0) {
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
	const double FACTOR = 2 * M_PI / 360.0;
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
			e->_pixel1 = 0x40;
			e->_pixel2 = 0x40;
			e->_pixel3 = 0x40;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 7.0;
			e->_field14 = 0.0084687499;

			++e;
			e->_field0 = 0x574;
			e->_pixel1 = 0x7f;
			e->_pixel2 = 0;
			e->_pixel3 = 0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 3.0;
			e->_field14 = 0.021011719;

			++e;
			e->_field0 = 0x603;
			e->_pixel1 = 0;
			e->_pixel2 = 0;
			e->_pixel3 = 0xff;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = 0;
			e->_field14 = 0.022144532;

			++e;
			e->_field0 = 0x712;
			e->_pixel1 = 0xff;
			e->_pixel2 = 0;
			e->_pixel3 = 0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 2.0;
			e->_field14 = 0.01178125;

			++e;
			e->_field0 = 0xe7f;
			e->_pixel1 = 0xe6;
			e->_pixel2 = 0xbe;
			e->_pixel3 = 0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 1.0;
			e->_field14 = 0.24791406;

			++e;
			e->_field0 = 0x173f;
			e->_pixel1 = 0xf0;
			e->_pixel2 = 0xf0;
			e->_pixel3 = 0xe6;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 3.0;
			e->_field14 = 0.20832032;

			++e;
			e->_field0 = 0x2ab8;
			e->_pixel1 = 0x28;
			e->_pixel2 = 0x32;
			e->_pixel3 = 0x28;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 1.0;
			e->_field14 = 0.088164061;

			++e;
			e->_field0 = 0x40ac;
			e->_pixel1 = 0x0;
			e->_pixel2 = 0xbe;
			e->_pixel3 = 0xf0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 2.0;
			e->_field14 = 0.084375001;

			++e;
			e->_field0 = 0x539c;
			e->_pixel1 = 0x20;
			e->_pixel2 = 0x20;
			e->_pixel3 = 0x20;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = FACTOR * 17.0;
			e->_field14 = 1 / 256.0;
		} else {
			for (int ctr = 0; ctr < 5; ++ctr) {
				e->_field0 = static_cast<int>(g_vm->getRandomFloat() * 1350.0
					- 675.0) + VALUES1[idx];
				int val = VALUES2[g_vm->getRandomNumber(15)];
				e->_pixel1 = val & 0xff;
				e->_pixel2 = (val >> 8) & 0xff;
				e->_pixel3 = (val >> 16) & 0xff;
				e->_field8 = g_vm->getRandomNumber(3) + 3;
				
				e->_fieldC = g_vm->getRandomNumber(255);
				e->_field10 = FACTOR * (double)g_vm->getRandomNumber(15);
				e->_field14 = ((double)g_vm->getRandomNumber(0xfffffffe)
					* 50.0 / 65536.0) / 256.0;
			}
		}
	}

	if (_sineTable.setup()) {
		_grid.resize((val2 - 2) * val1 + 2);
		return true;
	}

	return false;
}

void CStarControlSub5::proc2(CStarControlSub6 *sub6, FVector *vector, double v1, double v2, double v3,
		CSurfaceArea *surfaceArea, CStarControlSub12 *sub12) {
	const int VALUES[] = { 0, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4 };
	double val1 = sub12->proc25();
	int val2 = sub12->proc27();
	if (!_flag)
		return;

	int f1, f3, size2, size1;
	double f2, f4, f5, f6, f7, f8, f9;
	double f10, f11, f12, f13, f14, f15, f16, f17, f18, f19;
	double f20, f21, f22, f23, f24, f25, f26, f27, f28;
	double f34, f35, f36, f37, f38, f39, f40;
	double f41, f42, f43, f44, f45, f46;
	FVector tempV;

	if (v3 >= 6.0e9) {
		int count, start;
		if (vector->_x != 0.0 && (vector->_y != 0.0 || vector->_z != 0.0)) {
			// WORKAROUND: Ignoring non-sensical randSeed((int)vector->_x);
			count = VALUES[g_vm->getRandomNumber(15)];
			start = 5 * g_vm->getRandomNumber(255);
		} else {
			count = 9;
			start = 0;
		}

		Entry *entryP = &_entries[start];
		for (; count > 0; --count, ++entryP) {
			f1 = _multiplier * entryP->_field8;
			f2 = entryP->_field14;
			f3 = (f1 + entryP->_fieldC) & 0x1FF;
			f4 = _sineTable[f1 & 0x1FF] * entryP->_field10;
			f5 = _sineTable[f3];
			f6 = cos(f4);
			f7 = sin(f4);
			f8 = _sineTable[f3 + 128];
			f9 = f7;
			f10 = f6 * f8;
			f11 = f6;
			f12 = f6 * f5;
			f13 = f2 * f10;
			f14 = f8 * f2;
			f15 = f9 * f2;
			f16 = f2 * f12;
			f17 = -(f7 * f8 * f2);
			f18 = f11 * f2;
			f19 = -(f9 * f5 * f2);
			f20 = -(f5 * f2);
			f21 = f14;
			_sub1._row1._x = f13;
			_sub1._row1._y = f15;
			_sub1._row1._z = f16;
			_sub1._row2._x = f17;
			_sub1._row2._y = f18;
			_sub1._row2._z = f19;
			_sub1._row3._x = f20;
			_sub1._row3._z = f14;

			f22 = (double)entryP->_field0;
			_sub1._vector._x = f22 * f10 + vector->_x;
			_sub1._vector._y = f9 * f22 + vector->_y;
			_sub1._vector._z = f22 * f12 + vector->_z;
			_sub2._row1._x = sub6->_row1._x * f13 + f16 * sub6->_row3._x + f15 * sub6->_row2._x;
			_sub2._row1._y = f15 * sub6->_row2._y + f16 * sub6->_row3._y + f13 * sub6->_row1._y;
			_sub2._row1._z = f16 * sub6->_row3._z + f13 * sub6->_row1._z + f15 * sub6->_row2._z;
			_sub2._row2._x = sub6->_row1._x * f17 + f19 * sub6->_row3._x + f18 * sub6->_row2._x;
			_sub2._row2._y = f18 * sub6->_row2._y + f17 * sub6->_row1._y + f19 * sub6->_row3._y;
			_sub2._row2._z = f18 * sub6->_row2._z + f19 * sub6->_row3._z + f17 * sub6->_row1._z;
			_sub2._row3._x = sub6->_row1._x * f20 + f21 * sub6->_row3._x;
			_sub2._row3._y = f20 * sub6->_row1._y + f21 * sub6->_row3._y;
			_sub2._row3._z = f20 * sub6->_row1._z + f21 * sub6->_row3._z;

			f23 = _sub1._vector._y;
			f24 = _sub1._vector._z;
			f25 = _sub1._vector._x;
			f26 = _sub1._vector._z;
			f27 = _sub1._vector._x;

			f28 = _sub1._vector._y;
			_sub2._vector._x = sub6->_row1._x * _sub1._vector._x
				+ sub6->_row3._x * _sub1._vector._z
				+ sub6->_row2._x * f28
				+ sub6->_vector._x;
			_sub2._vector._y = f23 * sub6->_row2._y
				+ f24 * sub6->_row3._y
				+ f25 * sub6->_row1._y
				+ sub6->_vector._y;
			_sub2._vector._z = f26 * sub6->_row3._z
				+ f27 * sub6->_row1._z
				+ f28 * sub6->_row2._z
				+ sub6->_vector._z;

			size2 = (int)_array[1]._data2.size();
			size1 = (int)_array[1]._data1.size();

			if (size2 > 0) {
				for (int ctr2 = 0; ctr2 < size2; ++ctr2) {
					FVector &currVector = _array[1]._data2[ctr2];
					GridEntry &gridEntry = _grid[ctr2];

					f34 = currVector._x;
					f35 = currVector._y;
					f36 = f35 * _sub2._row2._x;
					f37 = currVector._z;
					f38 = f37 * _sub2._row3._x + f36;
					f39 = f38 + f34 * _sub2._row1._x;
					f40 = f39 + _sub2._vector._x;

					gridEntry._x = f40;
					gridEntry._y = f37 * _sub2._row3._y
						+ f35 * _sub2._row2._y
						+ f34 * _sub2._row1._y
						+ _sub2._vector._y;
					gridEntry._z = f37 * _sub2._row3._z
						+ f35 * _sub2._row2._z
						+ f34 * _sub2._row1._z
						+ _sub2._vector._z;
				}
			}

			if (val2 <= 0) {
				surfaceArea->setMode(SA_NONE);
				surfaceArea->_pixel = MKTAG_BE(entryP->_pixel1, entryP->_pixel2,
					entryP->_pixel3, 0);
				surfaceArea->setColorFromPixel();

				for (int ctr2 = 0; ctr2 < size2; ++ctr2) {
					GridEntry &gridEntry = _grid[ctr2];
					sub12->proc28(2, gridEntry, tempV);
					gridEntry._position._x = tempV._x;
					gridEntry._position._y = tempV._y + v2;
				}

				for (int ctr2 = 0; ctr2 < size1; ++ctr2) {
					Data1 &d1 = _array[1]._data1[ctr2];
					GridEntry &grid1 = _grid[d1._index1];
					GridEntry &grid2 = _grid[d1._index2];

					if (grid1._z > val1 && grid2._z > val1) {
						surfaceArea->fn1(FRect(grid1._position._x, grid1._position._y,
							grid2._position._x, grid2._position._y));
					}
				}
			} else {
				surfaceArea->setMode(SA_NONE);
				surfaceArea->_pixel = entryP->_pixel1;
				surfaceArea->setColorFromPixel();

				for (int ctr2 = 0; ctr2 < size2; ++ctr2) {
					GridEntry &gridEntry = _grid[ctr2];
					sub12->proc28(0, gridEntry, tempV);
					gridEntry._position._x = tempV._x + v1;
					gridEntry._position._y = tempV._y + v2;
				}

				for (int ctr2 = 0; ctr2 < size1; ++ctr2) {
					Data1 &d1 = _array[1]._data1[ctr2];
					GridEntry &grid1 = _grid[d1._index1];
					GridEntry &grid2 = _grid[d1._index2];

					if (grid1._z > val1 && grid2._z > val1) {
						surfaceArea->fn1(FRect(grid1._position._x, grid1._position._y,
							grid2._position._x, grid2._position._y));
					}
				}

				surfaceArea->_pixel = entryP->_pixel3;
				surfaceArea->setColorFromPixel();
				surfaceArea->setMode(SA_MODE2);

				for (int ctr2 = 0; ctr2 < size2; ++ctr2) {
					GridEntry &gridEntry = _grid[ctr2];
					sub12->proc28(1, gridEntry, tempV);
					gridEntry._position._x = tempV._x + v1;
					gridEntry._position._y = tempV._y + v2;
				}

				for (int ctr2 = 0; ctr2 < size1; ++ctr2) {
					Data1 &d1 = _array[1]._data1[ctr2];
					GridEntry &grid1 = _grid[d1._index1];
					GridEntry &grid2 = _grid[d1._index2];

					if (grid1._z > val1 && grid2._z > val1) {
						surfaceArea->fn1(FRect(grid1._position._x, grid1._position._y,
							grid2._position._x, grid2._position._y));
					}
				}
			}
		}
	}

	uint pixel1 = 0x81EEF5, pixel2 = 0xF5, pixel3 = 0x810000;
	int arrIndex = 0;

	if (v3 >= 200000000.0) {
		if (v3 >= 900000000.0) {
			if (v3 >= 6000000000.0) {
				arrIndex = 3;
				if (v3 >= 1.0e10)
					arrIndex = 4;
			} else {
				arrIndex = 2;
			}
		} else {
			arrIndex = 1;
		}
	} else {
		arrIndex = 0;
	}

	SubEntry &entry = _array[arrIndex];

	for (uint ctr = 0; ctr < entry._data2.size(); ++ctr) {
		GridEntry &gridEntry = _grid[ctr];
		const FVector &d2v = entry._data2[ctr];
		FVector newV = d2v + *vector;

		f41 = sub6->_row1._x;
		f42 = sub6->_row3._x;
		f43 = sub6->_row2._x;
		f44 = f43 * newV._y;
		f45 = f41 * newV._x + f42 * newV._z + f44;
		f46 = f45 + sub6->_vector._x;

		gridEntry._x = f46;
		gridEntry._y = newV._y * sub6->_row2._y
			+ newV._z * sub6->_row3._y
			+ newV._x * sub6->_row1._y
			+ sub6->_vector._y;
		gridEntry._z = newV._z * sub6->_row3._z
			+ newV._y * sub6->_row2._z
			+ newV._x * sub6->_row1._z
			+ sub6->_vector._z;
	}

	if (val2 <= 0) {
		surfaceArea->setMode(SA_NONE);
		surfaceArea->_pixel = pixel1;
		surfaceArea->setColorFromPixel();

		for (uint ctr = 0; ctr < entry._data2.size(); ++ctr) {
			GridEntry &gridEntry = _grid[ctr];
			sub12->proc28(2, gridEntry, tempV);
			gridEntry._position._x = tempV._x + v1;
			gridEntry._position._y = tempV._y + v2;
		}

		for (uint ctr = 0; ctr < entry._data1.size(); ++ctr) {
			Data1 &d1 = entry._data1[ctr];
			GridEntry &grid1 = _grid[d1._index1];
			GridEntry &grid2 = _grid[d1._index2];

			if (grid2._z > val1 && grid1._z > val1) {
				surfaceArea->fn1(FRect(grid1._position._x, grid1._position._y,
					grid2._position._x, grid2._position._y));
			}
		}
	} else {
		surfaceArea->setMode(SA_NONE);
		surfaceArea->_pixel = pixel2;
		surfaceArea->setColorFromPixel();

		for (uint ctr = 0; ctr < entry._data2.size(); ++ctr) {
			GridEntry &gridEntry = _grid[ctr];
			sub12->proc28(2, gridEntry, tempV);
			gridEntry._position._x = tempV._x + v1;
			gridEntry._position._y = tempV._y + v2;
		}

		for (uint ctr = 0; ctr < entry._data1.size(); ++ctr) {
			Data1 &d1 = entry._data1[ctr];
			GridEntry &grid1 = _grid[d1._index1];
			GridEntry &grid2 = _grid[d1._index2];

			if (grid2._z > val1 && grid1._z > val1) {
				surfaceArea->fn1(FRect(grid1._position._x, grid1._position._y,
					grid2._position._x, grid2._position._y));
			}
		}

		surfaceArea->_pixel = pixel3;
		surfaceArea->setColorFromPixel();
		surfaceArea->setMode(SA_MODE2);

		for (uint ctr = 0; ctr < entry._data2.size(); ++ctr) {
			GridEntry &gridEntry = _grid[ctr];
			sub12->proc28(2, gridEntry, tempV);
			gridEntry._position._x = tempV._x + v1;
			gridEntry._position._y = tempV._y + v2;
		}

		for (uint ctr = 0; ctr < entry._data1.size(); ++ctr) {
			Data1 &d1 = entry._data1[ctr];
			GridEntry &grid1 = _grid[d1._index1];
			GridEntry &grid2 = _grid[d1._index2];

			if (grid2._z > val1 && grid1._z > val1) {
				surfaceArea->fn1(FRect(grid1._position._x, grid1._position._y,
					grid2._position._x, grid2._position._y));
			}
		}
	}
}

void CStarControlSub5::proc3(CErrorCode *errorCode) {
	++_multiplier;
	errorCode->set();
}

void CStarControlSub5::fn1() {
	_flag = !_flag;
}

bool CStarControlSub5::setupEntry(int width, int height, int index, double val) {
	/*
	if (width < 2 || height < 3)
		return false;

	SubEntry &entry = _array[index];
	entry.clear();

	int height2 = height - 2;
	int height1 = height - 1;
	entry._data1.resize((2 * height - 3) * width);
	entry._data2.resize(width * height2 + 2);
	entry._data2[0] = FVector(0.0, val, 0.0);

	index = 1;
	double vy = 180.0 / (double)height1;
	double vx = 360.0 / (double)width;
	const double FACTOR = 3.1415927 * 0.0055555557;

	if (height1 > 1) {
		double yAmount = vy;
		int width12 = width * 12;
		for (int yCtr = height1 - 1; yCtr > 0; --yCtr, yAmount += vy) {
			double sineVal = sin(yAmount * FACTOR);


			| 0.0 * FACTOR |
			| cos(yAmount * FACTOR) * val |
			| 0.0 |
			|yAmount|
		}
	}
	// TODO

	*/
	return true;
}

} // End of namespace Titanic
