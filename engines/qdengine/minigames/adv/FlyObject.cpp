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
#include "qdengine/minigames/adv/FlyObject.h"
#include "qdengine/minigames/adv/qdMath.h"
#include "qdengine/minigames/adv/RunTime.h"

namespace QDEngine {

FlyObjectBase::FlyObjectBase(const mgVect2f& _c, const mgVect2f& _t, float _s)
	: current(_c)
	, target(_t)
	, speed(_s) {
}

bool FlyObjectBase::quant(float dt) {
	mgVect2f dir = target;
	dir -= current;
	float step = speed * dt;
	if (abs(dir) < step) {
		current = target;
		return false;
	}
	norm(dir);
	dir *= step;
	current += dir;
	return true;
}


bool FlyQDObject::quant(float dt, QDObject& obj) {
	bool ret = FlyObjectBase::quant(dt);
	obj->set_R(g_runtime->game2world(current, depth));
	return ret;
}

} // namespace QDEngine
