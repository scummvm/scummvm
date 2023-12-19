
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

#ifndef TWP_MOTOR_H
#define TWP_MOTOR_H

#include "math/vector2d.h"
#include "twp/camera.h"
#include "twp/util.h"

namespace Twp {

template<typename T>
struct Tween {
public:
	Tween(T f, T t, float duration, InterpolationMethod im)
		: frm(f), to(t), delta(to - frm), duration(duration), value(frm), easing_f(easing(im.kind)), swing(im.swing), loop(im.loop) {
	}

	bool running() {
		if (swing || loop)
			return true;
		else
			return elapsed < duration;
	}

	void update(float el) {
		if (enabled && running()) {
			elapsed += el;
			float f = clamp(elapsed / duration, 0.0f, 1.0f);
			if (!dir_forward)
				f = 1.0 - f;
			if ((elapsed > duration) && (swing || loop)) {
				elapsed = elapsed - duration;
				if (swing)
					dir_forward = !dir_forward;
			}
			if (easing_f.func) {
				f = easing_f.func(f);
				value = frm + f * delta;
			}
		} else {
			value = to;
		}
	}

	T current() const { return value; }

public:
	T frm, to, delta;
	float elapsed = 0.f;
	float duration = 0.f; // duration in ms
	T value;
	EasingFunc_t easing_f;
	bool enabled = true;
	bool dir_forward = true;
	bool swing = false;
	bool loop = false;
};

class Motor {
public:
	virtual ~Motor() {}
	virtual void disable() {
		_enabled = false;
	}
	virtual bool isEnabled() const { return _enabled; }
	virtual void update(float elapsed) = 0;

private:
	bool _enabled = true;
};

class Object;
class OffsetTo : public Motor {
public:
	virtual ~OffsetTo();
	OffsetTo(float duration, Object *obj, Math::Vector2d pos, InterpolationMethod im);

private:
	virtual void update(float elasped) override;

private:
	Object *_obj = nullptr;
	Tween<Math::Vector2d> _tween;
};

} // namespace Twp

#endif
