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

#ifndef QDENGINE_QDCORE_QD_ANIMATION_SET_H
#define QDENGINE_QDCORE_QD_ANIMATION_SET_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_animation.h"
#include "qdengine/qdcore/qd_animation_info.h"


namespace QDEngine {


class qdAnimationSet : public qdNamedObject {
public:
	qdAnimationSet();
	qdAnimationSet(const qdAnimationSet &set);
	~qdAnimationSet();

	qdAnimationSet &operator = (const qdAnimationSet &set);

	int named_object_type() const {
		return QD_NAMED_OBJECT_ANIMATION_SET;
	}

	int size() const {
		return _animations.size();
	}
	void resize(int sz);

	//! выдает индекс походки и остановки, соответствующих углу
	int get_angle_index(float direction_angle) const;

	//! выдает угол походки и остановки, соответствующих индексу
	float get_index_angle(int direction_index) const;

	//! Возвращает ближайшее к angle направление, для которого есть анимация.
	float adjust_angle(float angle) const;

	qdAnimationInfo *get_animation_info(int index = 0);
	qdAnimationInfo *get_animation_info(float direction_angle);

	qdAnimationInfo *get_static_animation_info(int index = 0);
	qdAnimationInfo *get_static_animation_info(float direction_angle);

	qdAnimationInfo *get_start_animation_info(int index = 0);
	qdAnimationInfo *get_start_animation_info(float direction_angle);

	qdAnimationInfo *get_stop_animation_info(int index = 0);
	qdAnimationInfo *get_stop_animation_info(float direction_angle);

	qdAnimation *get_turn_animation() const;
	qdAnimationInfo *get_turn_animation_info() {
		return &_turn_animation;
	}
	void set_turn_animation(const char *animation_name) {
		_turn_animation.set_animation_name(animation_name);
	}

	float walk_sound_frequency(int direction_index) const;
	float walk_sound_frequency(float direction_angle) const;
	void set_walk_sound_frequency(int direction_index, float freq);

	bool load_animations(const qdNamedObject *res_owner);
	bool free_animations(const qdNamedObject *res_owner);
	//! Регистрация ресурсов набора в диспетчере ресурсов.
	bool register_resources(const qdNamedObject *res_owner);
	//! Отмена регистрации ресурсов набора в диспетчере ресурсов.
	bool unregister_resources(const qdNamedObject *res_owner);

	bool scale_animations(float coeff_x, float coeff_y);

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	float start_angle() const {
		return _start_angle;
	}
	void set_start_angle(float v) {
		_start_angle = v;
	}

private:

	//! Угол, соответствующий первому направлению в походке.
	/**
	Если нулевой - первое направление вправо.
	*/
	float _start_angle;

	qdAnimationInfoVector _animations;
	qdAnimationInfoVector _static_animations;
	qdAnimationInfoVector _start_animations;
	qdAnimationInfoVector _stop_animations;

	/// анимация поворота, полный оборот начиная с направления вправо
	qdAnimationInfo _turn_animation;

	//! Коэффициенты для частоты звука походки.
	Std::vector<float> _walk_sound_frequency;

	static int get_angle_index(float direction_angle, int dir_count);
	static float get_index_angle(int index, int dir_count);
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_ANIMATION_SET_H
