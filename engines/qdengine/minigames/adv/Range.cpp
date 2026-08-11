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

#include "qdengine/minigames/adv/Range.h"

namespace QDEngine {

void Rangef::set(float min, float max) {
	_min = min;
	_max = max;
}

Rangef Rangef::intersection(const Rangef& range) {
	float begin;
	float end;
	if (maximum() < range.minimum() || minimum() > range.maximum())
		return Rangef(0.f, 0.f);

	if (include(range.minimum()))
		begin = range.minimum();
	else
		begin = minimum();

	if (include(range.maximum()))
		end = range.maximum();
	else
		end = maximum();
	return Rangef(begin, end);
}


float Rangef::clip(float &value) const {
	if (include(value))
		return value;
	else {
		if (value < minimum())
			return minimum();
		else
			return maximum();
	}
}

// --------------------- Rangei

void Rangei::set(int min, int max) {
	_min = min;
	_max = max;
}

Rangei Rangei::intersection(const Rangei& range) {
	int begin;
	int end;
	if (maximum() < range.minimum() || minimum() > range.maximum())
		return Rangei(0, 0);

	if (include(range.minimum()))
		begin = range.minimum();
	else
		begin = minimum();

	if (include(range.maximum()))
		end = range.maximum();
	else
		end = maximum();
	return Rangei(begin, end);
}


int Rangei::clip(int &value) {
	if (include(value))
		return value;
	else {
		if (value < minimum())
			return minimum();
		else
			return maximum();
	}
}

} // namespace QDEngine
