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

#include "qdengine/qd_fwd.h"
#include "qdengine/xmath.h"
#include "qdengine/qdcore/qd_camera.h"
#include "qdengine/qdcore/qd_game_object_moving.h"
#include "qdengine/qdcore/util/AIAStar_API.h"


namespace QDEngine {

qdHeuristic::qdHeuristic() : _camera_ptr(NULL), _object_ptr(NULL) {
}

qdHeuristic::~qdHeuristic() {
}

int qdHeuristic::getH(int x, int y) {
	x -= _target.x;
	y -= _target.y;

	//return sqrt(static_cast<float>(x * x + y * y));
	// Достаточно будет эвристики без квадратного корня, который медленный
	return static_cast<float>(x * x + y * y);
}

int qdHeuristic::getG(int x1, int y1, int x2, int y2) {
	if (!_object_ptr->is_walkable(Vect2s(x2, y2)))
		return 10000;
	// Для диагональных перемещений смотрим еще и перемещения по катетам,
	// потому как туда может попасть персонаж из-за погрешностей интерполирования позиции
	if ((x1 != x2) && (y1 != y2) &&
	        (
	            !_object_ptr->is_walkable(Vect2s(x1, y2)) ||
	            !_object_ptr->is_walkable(Vect2s(x2, y1))
	        ))
		return 10000;

	//return abs(x2 - x1) + abs(y2 - y1);
	// 14 - приближение корня из 2, умноженного на 10. 10 - единица*10.
	if ((x1 != x2) && (y1 != y2))
		return 14;
	else return 10;
}

void qdHeuristic::init(const Vect3f trg) {
	_target_f = trg;
	_target = _camera_ptr->get_cell_index(trg.x, trg.y);
}

} // namespace QDEngine
