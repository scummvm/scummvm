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

QDObject QDObject::ZERO(0, "ZERO OBJECT");

const char *QDObject::getName() const {
#ifdef _DEBUG
	return name_.c_str();
#else
	return "";
#endif
}

bool QDObject::hit(const mgVect2f& point) const {
	return _obj->hit_test(mgVect2i(round(point.x), round(point.y)));
}

float QDObject::depth() const {
	return g_runtime->getDepth(_obj);
}

void QDObject::setState(const char* name) {
	if (!_obj->is_state_active(name))
		_obj->set_state(name);
}

int getParameter(const char* name, const int& defValue) {
	return round(getParameter(name, (float)defValue));
}

bool getParameter(const char* name, int& out, bool obligatory) {
	float retValue = out;
	if (getParameter(name, retValue, obligatory)) {
		out = round(retValue);
		return true;
	}
	return false;
}

float getParameter(const char* name, const float &defValue) {
	if (const char *data = g_runtime->parameter(name, false)) {
		float retValue = defValue;
		if (sscanf(data, "%f", &retValue) == 1)
			return retValue;
		error("The parameter [%s] contains wrong data type. It must be a number", name);
	}
	return defValue;

}

bool getParameter(const char* name, float &out, bool obligatory) {
	if (const char * data = g_runtime->parameter(name, obligatory)) {
		float retValue = out;
		if (sscanf(data, "%f", &retValue) == 1) {
			out = retValue;
			return true;
		}
		error("The parameter [%s] contains wrong data type. It must be a number", name);
	}
	return false;

}

mgVect2f getParameter(const char* name, const mgVect2f& defValue) {
	if (const char * data = g_runtime->parameter(name, false)) {
		mgVect2f retValue = defValue;
		if (sscanf(data, "%f %f", &retValue.x, &retValue.y) == 2)
			return retValue;
		error("The parameter [%s] contains wrong data type. It must be a pair of numbers", name);
	}
	return defValue;

}

bool getParameter(const char* name, mgVect2f& out, bool obligatory) {
	if (const char * data = g_runtime->parameter(name, obligatory)) {
		mgVect2f retValue = out;
		if (sscanf(data, "%f %f", &retValue.x, &retValue.y) == 2) {
			out = retValue;
			return true;
		}
		error("The parameter [%s] contains wrong data type. It must be a pair of numbers", name);
	}
	return false;

}

mgVect2i getParameter(const char* name, const mgVect2i& defValue) {
	mgVect2f retValue = getParameter(name, mgVect2f(defValue.x, defValue.y));
	return mgVect2i(round(retValue.x), round(retValue.y));

}

bool getParameter(const char* name, mgVect2i& out, bool obligatory) {
	mgVect2f retValue = mgVect2f(out.x, out.y);
	if (getParameter(name, retValue, obligatory)) {
		out = mgVect2i(round(retValue.x), round(retValue.y));
		return true;
	}
	return false;
}

} // namespace QDEngine
