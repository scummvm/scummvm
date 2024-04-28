
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
		if (swing || loop) {
			return true;
		}
		return elapsed < duration;
	}

	void update(float el) {
		if (enabled && running()) {
			elapsed += el;
			float f = CLIP(elapsed / duration, 0.0f, 1.0f);
			if (!dir_forward)
				f = 1.0f - f;
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
	void update(float elapsed);

protected:
	virtual void onUpdate(float elapsed) = 0;

protected:
	bool _enabled = true;
};

class Object;
class OffsetTo : public Motor {
public:
	virtual ~OffsetTo();
	OffsetTo(float duration, Common::SharedPtr<Object> obj, const Math::Vector2d &pos, InterpolationMethod im);

private:
	virtual void onUpdate(float elasped) override;

private:
	Common::SharedPtr<Object> _obj;
	Tween<Math::Vector2d> _tween;
};

class MoveTo : public Motor {
public:
	virtual ~MoveTo();
	MoveTo(float duration, Common::SharedPtr<Object> obj, const Math::Vector2d &pos, InterpolationMethod im);

private:
	virtual void onUpdate(float elasped) override;

private:
	Common::SharedPtr<Object> _obj;
	Tween<Math::Vector2d> _tween;
};

class AlphaTo : public Motor {
public:
	virtual ~AlphaTo();
	AlphaTo(float duration, Common::SharedPtr<Object> obj, float to, InterpolationMethod im);

private:
	virtual void onUpdate(float elasped) override;

private:
	Common::SharedPtr<Object> _obj;
	Tween<float> _tween;
};

class Node;
class RotateTo : public Motor {
public:
	virtual ~RotateTo();
	RotateTo(float duration, Node *obj, float to, InterpolationMethod im);

private:
	virtual void onUpdate(float elasped) override;

private:
	Node *_node = nullptr;
	Tween<float> _tween;
};

class RoomRotateTo : public Motor {
public:
	virtual ~RoomRotateTo();
	RoomRotateTo(Common::SharedPtr<Room> room, float to);

private:
	virtual void onUpdate(float elasped) override;

private:
	Common::SharedPtr<Room> _room;
	Tween<float> _tween;
};

class ScaleTo : public Motor {
public:
	virtual ~ScaleTo();
	ScaleTo(float duration, Node *node, float to, InterpolationMethod im);

private:
	virtual void onUpdate(float elasped) override;

private:
	Node *_node = nullptr;
	Tween<float> _tween;
};

class Shake : public Motor {
public:
	virtual ~Shake();
	Shake(Node *node, float amount);

private:
	virtual void onUpdate(float elasped) override;

private:
	Node *_node = nullptr;
	float _amount = 0.f;
	float _shakeTime = 0.f;
	float _elapsed = 0.f;
};

class OverlayTo : public Motor {
public:
	virtual ~OverlayTo();
	OverlayTo(float duration, Common::SharedPtr<Room> room, const Color &to);

	virtual void onUpdate(float elapsed) override;

private:
	Common::SharedPtr<Room> _room;
	Color _to;
	Tween<Color> _tween;
};

class ReachAnim : public Motor {
public:
	virtual ~ReachAnim();
	ReachAnim(Common::SharedPtr<Object> actor, Common::SharedPtr<Object> obj);

	virtual void onUpdate(float elasped) override;

private:
	void playReachAnim();

private:
	Common::SharedPtr<Object> _actor;
	Common::SharedPtr<Object> _obj;
	int _state = 0;
	float _elapsed = 0.f;
};

enum WalkToState {
	kWalking,
	kArrived,
	kReach
};

class WalkTo : public Motor {
public:
	WalkTo(Common::SharedPtr<Object> obj, const Math::Vector2d &dest, int facing = 0);
	void disable() override;

	const Common::Array<Math::Vector2d> &getPath() const { return _path; }

private:
	void actorArrived();
	void onUpdate(float elapsed) override;

private:
	Common::SharedPtr<Object> _obj;
	Common::Array<Math::Vector2d> _path;
	int _facing = 0;
	float _wsd;
	WalkToState _state = kWalking;
};

class TextNode;

class TalkingBase : public Motor {
protected:
	TalkingBase(Common::SharedPtr<Object> actor, float duration);

public:
	virtual ~TalkingBase() {}

protected:
	Common::String talkieKey();
	int onTalkieId(int id);
	int loadActorSpeech(const Common::String &name);
	void setDuration(const Common::String &text);
	float getTalkSpeed() const;

protected:
	Common::SharedPtr<Object> _actor;
	float _duration = 0.f;
	float _elapsed = 0.f;
};

// Creates a talking animation for a specified object.
class Talking : public TalkingBase {
public:
	Talking(Common::SharedPtr<Object> obj, const Common::StringArray &texts, const Color &color);
	virtual ~Talking() {}

	void append(const Common::StringArray &texts, const Color &color);

	virtual void onUpdate(float elapsed) override;
	virtual void disable() override;

private:
	void say(const Common::String &text);

private:
	Common::SharedPtr<TextNode> _node;
	Lip _lip;
	Color _color;
	Common::StringArray _texts;
};

class SayLineAt : public TalkingBase {
public:
	SayLineAt(const Math::Vector2d &pos, const Color &color, Common::SharedPtr<Object> actor, float duration, const Common::String &text);
	virtual ~SayLineAt() {}

	virtual void onUpdate(float elapsed) override;
	virtual void disable() override;

private:
	void say(const Common::String &text);

private:
	const Math::Vector2d _pos;
	Color _color;
	Common::String _text;
	Common::SharedPtr<TextNode> _node;
};

class Jiggle : public Motor {
public:
	Jiggle(Node *node, float amount);
	virtual ~Jiggle();

private:
	virtual void onUpdate(float elapsed) override;

private:
	Node *_node = nullptr;
	float _amount = 0.f;
	float _jiggleTime = 0.f;
};

class MoveCursorTo : public Motor {
public:
	MoveCursorTo(const Math::Vector2d &pos, float time);
	virtual ~MoveCursorTo() {}

private:
	virtual void onUpdate(float elapsed) override;

private:
	Tween<Math::Vector2d> _tween;
	Math::Vector2d _pos;
};

} // namespace Twp

#endif
