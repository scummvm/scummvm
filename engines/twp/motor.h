
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
#include "twp/lip.h"

namespace Twp {

template<typename T>
struct Tween {
public:
	Tween(T f, T t, float d, InterpolationMethod im)
		: frm(f), to(t), delta(t - f), duration(d), value(f), easing_f(easing(im.kind)), swing(im.swing), loop(im.loop) {
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
				value = frm + delta * f;
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

protected:
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

class MoveTo : public Motor {
public:
	virtual ~MoveTo();
	MoveTo(float duration, Object *obj, Math::Vector2d pos, InterpolationMethod im);

private:
	virtual void update(float elasped) override;

private:
	Object *_obj = nullptr;
	Tween<Math::Vector2d> _tween;
};

class AlphaTo : public Motor {
public:
	virtual ~AlphaTo();
	AlphaTo(float duration, Object *obj, float to, InterpolationMethod im);

private:
	virtual void update(float elasped) override;

private:
	Object *_obj = nullptr;
	Tween<float> _tween;
};

class Node;
class RotateTo : public Motor {
public:
	virtual ~RotateTo();
	RotateTo(float duration, Node *obj, float to, InterpolationMethod im);

private:
	virtual void update(float elasped) override;

private:
	Node *_node = nullptr;
	Tween<float> _tween;
};

class RoomRotateTo : public Motor {
public:
	virtual ~RoomRotateTo();
	RoomRotateTo(Room *room, float to);

private:
	virtual void update(float elasped) override;

private:
	Room *_room = nullptr;
	Tween<float> _tween;
};

class ScaleTo : public Motor {
public:
	virtual ~ScaleTo();
	ScaleTo(float duration, Node *node, float to, InterpolationMethod im);

private:
	virtual void update(float elasped) override;

private:
	Node *_node = nullptr;
	Tween<float> _tween;
};

class Shake : public Motor {
public:
	virtual ~Shake();
	Shake(Node *node, float amount);

private:
	virtual void update(float elasped) override;

private:
	Node *_node = nullptr;
	float _amount = 0.f;
	float _shakeTime = 0.f;
	float _elapsed = 0.f;
};

class OverlayTo : public Motor {
public:
	virtual ~OverlayTo();
	OverlayTo(float duration, Room *room, Color to);

	virtual void update(float elapsed) override;

private:
	Room *_room = nullptr;
	Color _to;
	Tween<Color> _tween;
};

class ReachAnim : public Motor {
public:
	virtual ~ReachAnim();
	ReachAnim(Object *actor, Object *obj);

	virtual void update(float elasped) override;

private:
	void playReachAnim();

private:
	Object *_actor = nullptr;
	Object *_obj = nullptr;
	int _state = 0;
	float _elapsed = 0.f;
};

class WalkTo : public Motor {
public:
	WalkTo(Object *obj, Vector2i dest, int facing = 0);
	virtual void disable() override;

	const Common::Array<Vector2i> &getPath() const { return _path; }

private:
	void actorArrived();
	virtual void update(float elapsed) override;

private:
	Object *_obj = nullptr;
	Common::Array<Vector2i> _path;
	int _facing = 0;
	float _wsd;
};

// Creates a talking animation for a specified object.
class Talking : public Motor {
public:
	Talking(Object *obj, const Common::StringArray &texts, Color color);
	virtual ~Talking() {}

	void append(const Common::StringArray &texts);

	virtual void update(float elapsed) override;
	virtual void disable() override;

private:
	void say(const Common::String &text);
	int onTalkieId(int id);
	Common::String talkieKey();
	void setDuration(const Common::String &text);
	int loadActorSpeech(const Common::String &name);

private:
	Object *_obj = nullptr;
	Node *_node = nullptr;
	Lip _lip;
	float _elapsed = 0.f;
	float _duration = 0.f;
	Color _color;
	Common::StringArray _texts;
};

class Jiggle : public Motor {
public:
	Jiggle(Node *node, float amount);
	virtual ~Jiggle();

private:
	virtual void update(float elapsed) override;

private:
	Node *_node = nullptr;
	float _amount = 0.f;
	float _jiggleTime = 0.f;
};

} // namespace Twp

#endif
