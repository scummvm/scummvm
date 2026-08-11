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


#ifndef QDENGINE_SYSTEM_GRAPHICS_GR_SCREEN_REGION_H
#define QDENGINE_SYSTEM_GRAPHICS_GR_SCREEN_REGION_H

namespace QDEngine {

#define grScreenRegion_EMPTY grScreenRegion(0, 0, 0, 0)

//! Прямоугольная область на экране.
class grScreenRegion {
public:
	grScreenRegion() : _x(0), _y(0), _size_x(0), _size_y(0) {}
	grScreenRegion(int x, int y, int sx, int sy) : _x(x), _y(y), _size_x(sx), _size_y(sy) {}

	bool operator == (const grScreenRegion &reg) const {
		if (_x == reg._x && _y == reg._y && _size_x == reg._size_x && _size_y == reg._size_y)
			return true;

		if (is_empty() && reg.is_empty())
			return true;

		return false;
	}

	bool operator != (const grScreenRegion &reg) const {
		if (is_empty() && reg.is_empty())
			return false;

		if (_x != reg._x || _y != reg._y || _size_x != reg._size_x || _size_y != reg._size_y)
			return true;

		return false;
	}

	grScreenRegion &operator += (const grScreenRegion &reg) {
		if (reg.is_empty()) return *this;

		if (is_empty()) {
			*this = reg;
			return *this;
		}

		int x0 = (min_x() < reg.min_x()) ? min_x() : reg.min_x();
		int x1 = (max_x() > reg.max_x()) ? max_x() : reg.max_x();

		int y0 = (min_y() < reg.min_y()) ? min_y() : reg.min_y();
		int y1 = (max_y() > reg.max_y()) ? max_y() : reg.max_y();

		_x = (x0 + x1) / 2;
		_y = (y0 + y1) / 2;

		_size_x = x1 - x0;
		_size_y = y1 - y0;

		return *this;
	}

	int x() const {
		return _x;
	}
	int y() const {
		return _y;
	}

	int size_x() const {
		return _size_x;
	}
	int size_y() const {
		return _size_y;
	}

	int min_x() const {
		return _x - _size_x / 2;
	}
	int max_x() const {
		return _x + _size_x / 2;
	}

	int min_y() const {
		return _y - _size_y / 2;
	}
	int max_y() const {
		return _y + _size_y / 2;
	}

	void move(int dx, int dy) {
		_x += dx;
		_y += dy;
	}

	bool is_empty() const {
		return (!_size_x || !_size_y);
	}

	bool is_inside(int x, int y) const {
		if (x >= min_x() && x < max_x() && y >= min_y() && y < max_y()) return true;
		return false;
	}

	void clear() {
		_size_x = 0;
	}

	static const grScreenRegion EMPTY;

private:

	//! координаты центра области
	int _x;
	int _y;

	int _size_x;
	int _size_y;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_GR_SCREEN_REGION_H
