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

#ifndef TWP_EASING_H
#define TWP_EASING_H

#include "common/func.h"
#include "math/vector2d.h"
#include "twp/rectf.h"

namespace Twp {

typedef float EasingFunc(float t);

typedef struct EasingFunc_t {
	EasingFunc *func;
} EasingFunc_t;

enum InterpolationKind {
	IK_LINEAR = 0,
	IK_EASEIN = 1,
	IK_EASEINOUT = 2,
	IK_EASEOUT = 3,
	IK_SLOWEASEIN = 4,
	IK_SLOWEASEOUT = 5
};

struct InterpolationMethod {
	InterpolationKind kind = IK_LINEAR;
	bool loop = false;
	bool swing = false;
};

InterpolationMethod intToInterpolationMethod(int value);

static float linear(float t) { return t; }

static float easeIn(float t) {
	return t * t * t * t;
}

static float easeOut(float t) {
	float f = (t - 1.0f);
	return f * f * f * (1.0f - t) + 1.0f;
}

static float easeInOut(float t) {
	if (t < 0.5f)
		return 8.0f * t * t * t * t;
	float f = (t - 1.0f);
	return -8.f * f * f * f * f + 1.f;
}

inline EasingFunc_t easing(InterpolationKind kind) {
	switch (kind) {
	case IK_LINEAR:
		return {&linear};
	case IK_EASEIN:
		return {&easeIn};
	case IK_EASEINOUT:
		return {&easeInOut};
	case IK_EASEOUT:
		return {&easeOut};
	case IK_SLOWEASEIN:
		return {&easeIn};
	case IK_SLOWEASEOUT:
		return {&easeOut};
	}
	error("Invalid interpolation kind: %d", kind);
	return {&linear};
}

} // namespace Twp

#endif
