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

#ifndef QDENGINE_CORE_QDCORE_QD_ANIMATION_INFO_H
#define QDENGINE_CORE_QDCORE_QD_ANIMATION_INFO_H

#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/qdcore/qd_named_object.h"


namespace QDEngine {

class XStream;
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
		return speed_;
	}
	void set_speed(float sp) {
		speed_ = sp;
	}

	float animation_speed() const {
		return animation_speed_;
	}
	void set_animation_speed(float sp) {
		animation_speed_ = sp;
	}

	qdAnimation *animation() const;
	const char *animation_name() const {
		return name();
	}

	void load_script(const xml::tag *p);
	bool save_script(XStream &fh, int indent = 0) const;

	void set_animation_name(const char *name) {
		set_name(name);
	}
	void free_animation_name() {
		set_name(0);
	}

private:

	float speed_;
	float animation_speed_;
};

typedef std::vector<qdAnimationInfo> qdAnimationInfoVector;

} // namespace QDEngine

#endif /* QDENGINE_CORE_QDCORE_QD_ANIMATION_INFO_H */
