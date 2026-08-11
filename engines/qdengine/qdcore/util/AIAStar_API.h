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

#ifndef QDENGINE_QDCORE_UTIL_AIASTAR_API_H
#define QDENGINE_QDCORE_UTIL_AIASTAR_API_H

#include "qdengine/qdcore/util/AIAStar.h"


namespace QDEngine {

class qdCamera;
class qdGameObjectMoving;

//! Эвристика для поиска пути.
class qdHeuristic {
public:
	qdHeuristic();
	~qdHeuristic();

	int getH(int x, int y);
	int getG(int x1, int y1, int x2, int y2);
	bool isEndPoint(int x, int y) {
		return (x == _target.x && y == _target.y);
	}

	void init(const Vect3f trg);
	void set_camera(const qdCamera *cam) {
		_camera_ptr = cam;
	}
	void set_object(const qdGameObjectMoving *obj) {
		_object_ptr = obj;
	}

private:

	Vect2i _target;
	Vect3f _target_f;

	const qdCamera *_camera_ptr;
	const qdGameObjectMoving *_object_ptr;
};

typedef AIAStar<qdHeuristic, int> qdAStar;

} // namespace QDEngine

#endif // _AIASTAR_API_H
