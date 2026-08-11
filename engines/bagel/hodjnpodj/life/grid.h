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

#ifndef HODJNPODJ_LIFE_GRID_H
#define HODJNPODJ_LIFE_GRID_H

namespace Bagel {
namespace HodjNPodj {
namespace Life {

// For colony class
#define LIFE            1 // '+'
#define NO_LIFE         0 // '-'
#define BOARD_START_ROW 58
#define BOARD_START_COL 63
//#define BOARD_SPACING 1.5
#define BOARD_SPACING_TIMES_TWO 3
#define ROWS            15
#define COLS            20


class colony {
	int grid[ROWS][COLS] = {};
	int rows = 0, cols = 0;
public:

	//creator
	colony(int r, int c);
	colony(const colony &) = default;

	//destroy construct
	~colony();
	//copy operator
	colony &operator=(const colony &c);
	//equality operators: assuming equal dims
	bool operator==(const colony &c);
	bool operator!=(const colony &c);

	//accessors
	int row() const {
		return rows;
	};
	int col() const {
		return cols;
	};
	int islife(int r, int c) const {
		return grid[r][c];
	};
	void evolve(const colony &c);
	void flip(int r, int c);
	//void print();
public:
	int m_nColony_count = 0;
};

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel

#endif
