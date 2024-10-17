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
	return obj_->hit_test(mgVect2i(round(point.x), round(point.y)));
}

float QDObject::depth() const {
	return runtime->getDepth(obj_);
}

void QDObject::setState(const char* name) {
	if (!obj_->is_state_active(name))
		obj_->set_state(name);
}

template<class T>
T getParameter(const char* name, const T& defValue) {
	return round(getParameter<float>(name, (float)defValue));
}

template<class T>
bool getParameter(const char* name, T& out, bool obligatory) {
	float retValue = out;
	if (getParameter<float>(name, retValue, obligatory)) {
		out = round(retValue);
		return true;
	}
	return false;
}

template<>
float getParameter(const char* name, const float &defValue) {
	if (const char * data = runtime->parameter(name, false)) {
		float retValue = defValue;
		if (_snscanf(data, 8, "%f", &retValue) == 1)
			return retValue;
		xxassert(false, (XBuffer() < "В параметре [" < name < "] неверный тип данных. Должно быть число.").c_str());
	}
	return defValue;

}

template<>
bool getParameter(const char* name, float &out, bool obligatory) {
	if (const char * data = runtime->parameter(name, obligatory)) {
		float retValue = out;
		if (_snscanf(data, 8, "%f", &retValue) == 1) {
			out = retValue;
			return true;
		}
		xxassert(false, (XBuffer() < "В параметре [" < name < "] неверный тип данных. Должно быть число.").c_str());
	}
	return false;

}

template<>
mgVect2f getParameter(const char* name, const mgVect2f& defValue) {
	if (const char * data = runtime->parameter(name, false)) {
		mgVect2f retValue = defValue;
		if (_snscanf(data, 16, "%f %f", &retValue.x, &retValue.y) == 2)
			return retValue;
		xxassert(false, (XBuffer() < "В параметре [" < name < "] неверный тип данных. Должна быть пара чисел.").c_str());
	}
	return defValue;

}

template<>
bool getParameter(const char* name, mgVect2f& out, bool obligatory) {
	if (const char * data = runtime->parameter(name, obligatory)) {
		mgVect2f retValue = out;
		if (_snscanf(data, 16, "%f %f", &retValue.x, &retValue.y) == 2) {
			out = retValue;
			return true;
		}
		xxassert(false, (XBuffer() < "В параметре [" < name < "] неверный тип данных. Должна быть пара чисел.").c_str());
	}
	return false;

}

template<>
mgVect2i getParameter(const char* name, const mgVect2i& defValue) {
	mgVect2f retValue = getParameter(name, mgVect2f(defValue.x, defValue.y));
	return mgVect2i(round(retValue.x), round(retValue.y));

}

template<>
bool getParameter(const char* name, mgVect2i& out, bool obligatory) {
	mgVect2f retValue = mgVect2f(out.x, out.y);
	if (getParameter<mgVect2f>(name, retValue, obligatory)) {
		out = mgVect2i(round(retValue.x), round(retValue.y));
		return true;
	}
	return false;
}

void dummyInstanceGetParameter() {
	bool db = false;
	getParameter("", db);
	getParameter("", db, false);

	int di = 0;
	getParameter("", di);
	getParameter("", di, false);

	float df = 0.f;
	getParameter("", df);
	getParameter("", df, false);

	mgVect2i d2i;
	getParameter("", d2i);
	getParameter("", d2i, false);

	mgVect2i d2f;
	getParameter("", d2f);
	getParameter("", d2f, false);
}

} // namespace QDEngine
