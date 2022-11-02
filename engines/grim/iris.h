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

#ifndef GRIM_IRIS_H
#define GRIM_IRIS_H

namespace Grim {

class SaveGame;

class Iris {
public:
	enum Direction {
		Open = 0,
		Close = 1
	};

	Iris();
	~Iris();

	void play(Direction dir, int x, int y, int length);
	void draw();
	void update(int frameTime);

	void saveState(SaveGame *state) const;
	void restoreState(SaveGame *state);

private:
	bool _playing;
	Direction _direction;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
	int _targetX;
	int _targetY;
	int _length;
	int _currTime;
};

} // end of namespace Grim

#endif
