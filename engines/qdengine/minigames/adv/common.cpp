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

#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/RunTime.h"

namespace QDEngine {

const char *QDObject::getName() const {
#ifdef _DEBUG
	return _name.c_str();
#else
	return "";
#endif
}

bool QDObject::hit(const mgVect2f& point) const {
	return _obj->hit_test(mgVect2i(round(point.x), round(point.y)));
}

float QDObject::depth(MinigameManager *runtime) const {
	return runtime->getDepth(_obj);
}

void QDObject::setState(const char* name) {
	if (!_obj->is_state_active(name))
		_obj->set_state(name);
}

} // namespace QDEngine
