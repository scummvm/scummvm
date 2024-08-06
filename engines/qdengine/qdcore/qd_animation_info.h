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

#ifndef QDENGINE_QDCORE_QD_ANIMATION_INFO_H
#define QDENGINE_QDCORE_QD_ANIMATION_INFO_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object.h"


namespace QDEngine {

class qdAnimation;

//! Информация об анимации.
class qdAnimationInfo : public qdNamedObject {
public:
	qdAnimationInfo();
	qdAnimationInfo(const qdAnimationInfo &p);
	~qdAnimationInfo();

	qdAnimationInfo &operator = (const qdAnimationInfo &p);

	int named_object_type() const {
		return QD_NAMED_OBJECT_ANIMATION_INFO;
	}

	float speed() const {
		return _speed;
	}
	void set_speed(float sp) {
		_speed = sp;
	}

	float animation_speed() const {
		return _animation_speed;
	}
	void set_animation_speed(float sp) {
		_animation_speed = sp;
	}

	qdAnimation *animation() const;
	const char *animation_name() const {
		return name();
	}

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	void set_animation_name(const char *name) {
		set_name(name);
	}
	void free_animation_name() {
		set_name(0);
	}

private:

	float _speed;
	float _animation_speed;
};

typedef Std::vector<qdAnimationInfo> qdAnimationInfoVector;

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_ANIMATION_INFO_H
