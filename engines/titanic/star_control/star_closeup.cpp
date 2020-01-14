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

#include "titanic/star_control/star_closeup.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/camera.h"
#include "titanic/star_control/surface_area.h"
#include "titanic/titanic.h"

#include "common/math.h"

namespace Titanic {

#define MKTAG_BE(a3,a2,a1,a0) ((uint32)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

void CStarCloseup::SubEntry::clear() {
	_data1.clear();
	_data2.clear();
}

/*------------------------------------------------------------------------*/

bool CStarCloseup::SineTable::setup() {
	if (_data.empty()) {
		_data.resize(1024);
		for (int idx = 0; idx < 1024; ++idx)
			_data[idx] = sin((float)idx * 2 * M_PI / 512.0);
	}

	return true;
}

/*------------------------------------------------------------------------*/

CStarCloseup::CStarCloseup() : _flag(true), _multiplier(0) {
}

bool CStarCloseup::setup() {
	bool success = setupEntry(5, 5, 4, 1024.0)
		&& setupEntry(7, 7, 3, 1024.0)
		&& setupEntry(8, 8, 2, 1024.0)
		&& setupEntry(16, 16, 1, 1024.0)
		&& setupEntry(24, 24, 0, 1024.0);
	if (success)
		success = setup2(24, 24);

	return success;
}

bool CStarCloseup::setup2(int val1, int val2) {
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
			e->_field10 = Common::deg2rad<double>(7.0);
			e->_field14 = 0.0084687499;

			++e;
			e->_field0 = 0x574;
			e->_pixel1 = 0x7f;
			e->_pixel2 = 0;
			e->_pixel3 = 0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = Common::deg2rad<double>(3.0);
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
			e->_field10 = Common::deg2rad<double>(2.0);
			e->_field14 = 0.01178125;

			++e;
			e->_field0 = 0xe7f;
			e->_pixel1 = 0xe6;
			e->_pixel2 = 0xbe;
			e->_pixel3 = 0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = Common::deg2rad<double>(1.0);
			e->_field14 = 0.24791406;

			++e;
			e->_field0 = 0x173f;
			e->_pixel1 = 0xf0;
			e->_pixel2 = 0xf0;
			e->_pixel3 = 0xe6;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = Common::deg2rad<double>(3.0);
			e->_field14 = 0.20832032;

			++e;
			e->_field0 = 0x2ab8;
			e->_pixel1 = 0x28;
			e->_pixel2 = 0x32;
			e->_pixel3 = 0x28;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = Common::deg2rad<double>(1.0);
			e->_field14 = 0.088164061;

			++e;
			e->_field0 = 0x40ac;
			e->_pixel1 = 0x0;
			e->_pixel2 = 0xbe;
			e->_pixel3 = 0xf0;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = Common::deg2rad<double>(2.0);
			e->_field14 = 0.084375001;

			++e;
			e->_field0 = 0x539c;
			e->_pixel1 = 0x20;
			e->_pixel2 = 0x20;
			e->_pixel3 = 0x20;
			e->_field8 = g_vm->getRandomNumber(3) + 3;
			e->_fieldC = g_vm->getRandomNumber(255);
			e->_field10 = Common::deg2rad<double>(17.0);
			e->_field14 = 1 / 256.0;
		} else {
			for (int ctr = 0; ctr < 5; ++ctr) {
				e->_field0 = static_cast<int>(g_vm->getRandomFloat() * 1350.0
					- 675.0) + VALUES1[ctr];
				int val = VALUES2[g_vm->getRandomNumber(15)];
				e->_pixel1 = val & 0xff;
				e->_pixel2 = (val >> 8) & 0xff;
				e->_pixel3 = (val >> 16) & 0xff;
				e->_field8 = g_vm->getRandomNumber(3) + 3;
				e->_fieldC = g_vm->getRandomNumber(255);
				e->_field10 = Common::deg2rad<double>((double)g_vm->getRandomNumber(15));
				e->_field14 = ((float)g_vm->getRandomNumber(0xfffffffe)
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

void CStarCloseup::draw(const FPose &pose, const FVector &vector, const FVector &vector2,
		CSurfaceArea *surfaceArea, CCamera *camera) {
	const int VALUES[] = { 0, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4 };
	float val1 = camera->getFrontClip();
	StarColor starColor = camera->getStarColor();
	if (!_flag)
		return;

	int f1, f3, size2, size1;
	float f2, f4, f5, f6, f7, f8, f9;
	float f10, incr, f12, f13, f14, f15, f16, f17, f18, f19;
	float f20, f21, f22;
	FVector tempV;

	if (vector2._z < 6.0e9) {
		int count, start;
		if (vector._x != 0.0 && (vector._y != 0.0 || vector._z != 0.0)) {
			// WORKAROUND: Ignoring non-sensical randSeed((int)vector._x);
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
			incr = f6;
			f12 = f6 * f5;
			f13 = f2 * f10;
			f14 = f8 * f2;
			f15 = f9 * f2;
			f16 = f2 * f12;
			f17 = -(f7 * f8 * f2);
			f18 = incr * f2;
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

			f22 = (float)entryP->_field0;
			_sub1._vector._x = f22 * f10 + vector._x;
			_sub1._vector._y = f9 * f22 + vector._y;
			_sub1._vector._z = f22 * f12 + vector._z;
			_sub2._row1._x = pose._row1._x * f13 + f16 * pose._row3._x + f15 * pose._row2._x;
			_sub2._row1._y = f15 * pose._row2._y + f16 * pose._row3._y + f13 * pose._row1._y;
			_sub2._row1._z = f16 * pose._row3._z + f13 * pose._row1._z + f15 * pose._row2._z;
			_sub2._row2._x = pose._row1._x * f17 + f19 * pose._row3._x + f18 * pose._row2._x;
			_sub2._row2._y = f18 * pose._row2._y + f17 * pose._row1._y + f19 * pose._row3._y;
			_sub2._row2._z = f18 * pose._row2._z + f19 * pose._row3._z + f17 * pose._row1._z;
			_sub2._row3._x = pose._row1._x * f20 + f21 * pose._row3._x;
			_sub2._row3._y = f20 * pose._row1._y + f21 * pose._row3._y;
			_sub2._row3._z = f20 * pose._row1._z + f21 * pose._row3._z;

			_sub2._vector._x = pose._row1._x * _sub1._vector._x
				+ pose._row3._x * _sub1._vector._z
				+ pose._row2._x * _sub1._vector._y + pose._vector._x;
			_sub2._vector._y = pose._row2._y * _sub1._vector._y
				+ pose._row3._y * _sub1._vector._z
				+ pose._row1._y * _sub1._vector._x + pose._vector._y;
			_sub2._vector._z = pose._row3._z * _sub1._vector._z
				+ pose._row1._z * _sub1._vector._x
				+ pose._row2._z * _sub1._vector._y + pose._vector._z;

			size2 = (int)_array[1]._data2.size();
			size1 = (int)_array[1]._data1.size();

			if (size2 > 0) {
				for (int ctr2 = 0; ctr2 < size2; ++ctr2) {
					FVector &currVector = _array[1]._data2[ctr2];
					GridEntry &gridEntry = _grid[ctr2];

					gridEntry._x = currVector._z * _sub2._row3._x
						+ currVector._y * _sub2._row2._x
						+ currVector._x * _sub2._row1._x + _sub2._vector._x;
					gridEntry._y = currVector._z * _sub2._row3._y
						+ currVector._y * _sub2._row2._y
						+ currVector._x * _sub2._row1._y + _sub2._vector._y;
					gridEntry._z = currVector._z * _sub2._row3._z
						+ currVector._y * _sub2._row2._z
						+ currVector._x * _sub2._row1._z + _sub2._vector._z;
				}
			}

			switch (starColor) {
			case WHITE:
				surfaceArea->setMode(SA_SOLID);
				surfaceArea->_pixel = MKTAG_BE(entryP->_pixel1, entryP->_pixel2,
					entryP->_pixel3, 0);
				surfaceArea->setColorFromPixel();

				for (int ctr2 = 0; ctr2 < size2; ++ctr2) {
					GridEntry &gridEntry = _grid[ctr2];
					tempV = camera->getRelativePos(2, gridEntry);
					gridEntry._position._x = tempV._x;
					gridEntry._position._y = tempV._y + vector2._y;
				}

				for (int ctr2 = 0; ctr2 < size1; ++ctr2) {
					Data1 &d1 = _array[1]._data1[ctr2];
					GridEntry &grid1 = _grid[d1._index1];
					GridEntry &grid2 = _grid[d1._index2];

					if (grid1._z > val1 && grid2._z > val1) {
						surfaceArea->drawLine(FRect(grid1._position._x, grid1._position._y,
							grid2._position._x, grid2._position._y));
					}
				}
				break;
			case PINK:
				surfaceArea->setMode(SA_SOLID);
				surfaceArea->_pixel = entryP->_pixel1;
				surfaceArea->setColorFromPixel();

				for (int ctr2 = 0; ctr2 < size2; ++ctr2) {
					GridEntry &gridEntry = _grid[ctr2];
					tempV = camera->getRelativePos(0, gridEntry);
					gridEntry._position._x = tempV._x + vector2._x;
					gridEntry._position._y = tempV._y + vector2._y;
				}

				for (int ctr2 = 0; ctr2 < size1; ++ctr2) {
					Data1 &d1 = _array[1]._data1[ctr2];
					GridEntry &grid1 = _grid[d1._index1];
					GridEntry &grid2 = _grid[d1._index2];

					if (grid1._z > val1 && grid2._z > val1) {
						surfaceArea->drawLine(FRect(grid1._position._x, grid1._position._y,
							grid2._position._x, grid2._position._y));
					}
				}

				surfaceArea->_pixel = entryP->_pixel3;
				surfaceArea->setColorFromPixel();
				surfaceArea->setMode(SA_MODE2);

				for (int ctr2 = 0; ctr2 < size2; ++ctr2) {
					GridEntry &gridEntry = _grid[ctr2];
					tempV = camera->getRelativePos(1, gridEntry);
					gridEntry._position._x = tempV._x + vector2._x;
					gridEntry._position._y = tempV._y + vector2._y;
				}

				for (int ctr2 = 0; ctr2 < size1; ++ctr2) {
					Data1 &d1 = _array[1]._data1[ctr2];
					GridEntry &grid1 = _grid[d1._index1];
					GridEntry &grid2 = _grid[d1._index2];

					if (grid1._z > val1 && grid2._z > val1) {
						surfaceArea->drawLine(FRect(grid1._position._x, grid1._position._y,
							grid2._position._x, grid2._position._y));
					}
				}
				break;
			default:
				assert(0);
			}
		}
	}

	uint pixel1 = 0x81EEF5, pixel2 = 0xF5, pixel3 = 0x810000;
	int arrIndex = 0;

	if (vector2._z >= 200000000.0) {
		if (vector2._z >= 900000000.0) {
			if (vector2._z >= 6000000000.0) {
				arrIndex = 3;
				if (vector2._z >= 1.0e10)
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
		FVector newV = d2v + vector;

		gridEntry._x = pose._row1._x * newV._x + pose._row3._x * newV._z
			+ pose._row2._x * newV._y + pose._vector._x;
		gridEntry._y = newV._y * pose._row2._y + newV._z * pose._row3._y
			+ newV._x * pose._row1._y + pose._vector._y;
		gridEntry._z = newV._z * pose._row3._z + newV._y * pose._row2._z
			+ newV._x * pose._row1._z + pose._vector._z;
	}

	switch(starColor) {
	case WHITE:
		surfaceArea->setMode(SA_SOLID);
		surfaceArea->_pixel = pixel1;
		surfaceArea->setColorFromPixel();

		for (uint ctr = 0; ctr < entry._data2.size(); ++ctr) {
			GridEntry &gridEntry = _grid[ctr];
			tempV = camera->getRelativePos(2, gridEntry);
			gridEntry._position._x = tempV._x + vector2._x;
			gridEntry._position._y = tempV._y + vector2._y;
		}

		for (uint ctr = 0; ctr < entry._data1.size(); ++ctr) {
			Data1 &d1 = entry._data1[ctr];
			GridEntry &grid1 = _grid[d1._index1];
			GridEntry &grid2 = _grid[d1._index2];

			if (grid2._z > val1 && grid1._z > val1) {
				surfaceArea->drawLine(FRect(grid1._position._x, grid1._position._y,
					grid2._position._x, grid2._position._y));
			}
		}
		break;
	case PINK:
		surfaceArea->setMode(SA_SOLID);
		surfaceArea->_pixel = pixel2;
		surfaceArea->setColorFromPixel();

		for (uint ctr = 0; ctr < entry._data2.size(); ++ctr) {
			GridEntry &gridEntry = _grid[ctr];
			tempV = camera->getRelativePos(2, gridEntry);
			gridEntry._position._x = tempV._x + vector2._x;
			gridEntry._position._y = tempV._y + vector2._y;
		}

		for (uint ctr = 0; ctr < entry._data1.size(); ++ctr) {
			Data1 &d1 = entry._data1[ctr];
			GridEntry &grid1 = _grid[d1._index1];
			GridEntry &grid2 = _grid[d1._index2];

			if (grid2._z > val1 && grid1._z > val1) {
				surfaceArea->drawLine(FRect(grid1._position._x, grid1._position._y,
					grid2._position._x, grid2._position._y));
			}
		}

		surfaceArea->_pixel = pixel3;
		surfaceArea->setColorFromPixel();
		surfaceArea->setMode(SA_MODE2);

		for (uint ctr = 0; ctr < entry._data2.size(); ++ctr) {
			GridEntry &gridEntry = _grid[ctr];
			tempV = camera->getRelativePos(2, gridEntry);
			gridEntry._position._x = tempV._x + vector2._x;
			gridEntry._position._y = tempV._y + vector2._y;
		}

		for (uint ctr = 0; ctr < entry._data1.size(); ++ctr) {
			Data1 &d1 = entry._data1[ctr];
			GridEntry &grid1 = _grid[d1._index1];
			GridEntry &grid2 = _grid[d1._index2];

			if (grid2._z > val1 && grid1._z > val1) {
				surfaceArea->drawLine(FRect(grid1._position._x, grid1._position._y,
					grid2._position._x, grid2._position._y));
			}
		}
		break;
	default:
		assert(0);
	}
}

void CStarCloseup::proc3(CErrorCode *errorCode) {
	++_multiplier;
	errorCode->set();
}

void CStarCloseup::fn1() {
	_flag = !_flag;
}

bool CStarCloseup::setupEntry(int width, int height, int index, float val) {
	if (width < 2 || height < 3)
		return false;

	SubEntry &entry = _array[index];
	entry.clear();

	int d1Count, d2Count, size3, height1;
	int ctr, ctr2, idx, incr;
	float vx, vy, yVal, degrees, cosVal, sinVal, angle;

	d1Count = width * (2 * height - 3);
	d2Count = (height - 2) * width + 2;
	entry._data1.resize(d1Count);
	entry._data2.resize(d2Count);

	height1 = height - 1;
	vy = 180.0 / (float)height1;
	vx = 360.0 / (float)width;

	// Build up the vector list
	entry._data2[0]._y = val;

	for (ctr = height - 2, idx = 1, yVal = vy; ctr > 0; --ctr, yVal += vy) {
		degrees = 0.0;
		cosVal = cos(Common::deg2rad<float>(yVal));
		sinVal = sin(Common::deg2rad<float>(yVal));

		if (width > 0) {
			for (int xCtr = 0; xCtr < width; ++xCtr, ++idx, degrees += vx) {
				angle = Common::deg2rad<float>(degrees);

				FVector &tempV = entry._data2[idx];
				tempV._x = sin(angle) * sinVal * val;
				tempV._y = cosVal * val;
				tempV._z = cos(angle) * sinVal * val;
			}
		}
	}
	entry._data2[idx] = FVector(0.0, -1.0 * val, 0.0);

	size3 = width * (height - 3) + 1;
	Data1 *data1P = &entry._data1[0];
	for (ctr = 0; ctr < width; ++ctr, ++size3) {
		data1P->_index1 = 0;
		data1P->_index2 = size3 - width * (height - 3);
		++data1P;

		data1P->_index1 = d2Count - 1;
		data1P->_index2 = size3;
		++data1P;
	}

	incr = 1;
	for (ctr = 1; ctr < height1; ++ctr, incr += width) {
		for (ctr2 = 0; ctr2 < width; ++ctr2, ++data1P) {
			data1P->_index1 = ctr2 + incr;
			if (ctr2 == width - 1)
				data1P->_index2 = incr;
			else
				data1P->_index2 = ctr2 + incr + 1;

			if (ctr < height - 2) {
				++data1P;
				data1P->_index1 = ctr2 + incr;
				data1P->_index2 = width + ctr2 + incr;
			}
		}
	}

	return true;
}

} // End of namespace Titanic
