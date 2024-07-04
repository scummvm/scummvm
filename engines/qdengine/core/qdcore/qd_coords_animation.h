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

#ifndef QDENGINE_CORE_QDCORE_QD_COORDS_ANIMATION_H
#define QDENGINE_CORE_QDCORE_QD_COORDS_ANIMATION_H

#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/qdcore/qd_named_object.h"
#include "qdengine/core/qdcore/qd_coords_animation_point.h"
#include "qdengine/core/qdcore/qd_named_object_reference.h"


namespace QDEngine {

const int QD_COORDS_ANM_OBJECT_START_FLAG   = 0x01;
const int QD_COORDS_ANM_LOOP_FLAG           = 0x02;
const int QD_COORDS_ANM_RELATIVE_FLAG       = 0x04; // Флаг перемещения траектории в текущую точку нахождения объекта

class qdCoordsAnimation : public qdNamedObject {
public:
	enum qdCoordsAnimationType {
		CA_INTERPOLATE_COORDS,
		CA_WALK
	};

	qdCoordsAnimation();
	qdCoordsAnimation(const qdCoordsAnimation &anm);
	~qdCoordsAnimation();

	qdCoordsAnimation &operator = (const qdCoordsAnimation &anm);

	int named_object_type() const {
		return QD_NAMED_OBJECT_COORDS_ANIMATION;
	}

	float speed() const {
		return speed_;
	}
	void set_speed(float sp) {
		speed_ = sp;
	}

	const qdGameObject *start_object() const {
		return start_object_;
	}
	void set_start_object(const qdGameObject *p_obj);
	const qdNamedObjectReference &start_object_ref() const {
		return start_object_ref_;
	}

	bool is_playing() const {
		return status_;
	}
	bool is_finished() const {
		return is_finished_;
	}

	qdCoordsAnimationType type() const {
		return type_;
	}
	void set_type(qdCoordsAnimationType tp) {
		type_ = tp;
	}

	float animation_phase() const {
#ifdef _QUEST_EDITOR
		return animation_phase_ + animation_scroll_phase_;
#else
		return animation_phase_;
#endif
	}
	void set_animation_phase(float p) {
		animation_phase_ = p;
	}

	void start() const;
	void stop() const;
	void quant(float dt) const;

	bool reset_cur_point() const;
	bool set_cur_point(int point_num) const;
	int get_cur_point() const {
		return cur_point_;
	}

	void set_time_rel(float tm);

	void add_point(const qdCoordsAnimationPoint *p);
	void insert_point(const qdCoordsAnimationPoint *p, int insert_pos);

	void remove_point(int num);
	void clear();

	qdCoordsAnimationPoint *get_point(int index = 0) {
		return &points_[index];
	};
	const qdCoordsAnimationPoint *get_point(int index = 0) const {
		return &points_[index];
	};

	int size() const {
		return points_.size();
	}

	void load_script(const xml::tag *p);
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	bool is_empty() const {
		if (points_.empty()) return true;
		return false;
	}

#ifdef _QUEST_EDITOR
	// direction: true = next frame, false = prev frame
	bool change_animation_frame(bool direction = true) const;
	void reset_scroll() const {
		animation_scroll_phase_ = 0.f;
	}
#endif

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream &fh) const;

private:

	qdCoordsAnimationType type_;
	qdCoordsAnimationPointVector points_;
	float animation_phase_;
	float speed_;

	// Для относительного перемещения персонажа (QD_COORDS_ANM_RELATIVE_FLAG):
	// объект, относительно коориднат которого будет работать траектория движения (если не задано, то работаем
	// относительно координат объекта-владельца координатной анимации)
	const qdGameObject *start_object_;
	qdNamedObjectReference start_object_ref_;

#ifdef _QUEST_EDITOR
	//исопользуется для прокрутки анимации
	mutable float animation_scroll_phase_;
#endif
	mutable bool status_;
	mutable bool is_finished_;
	mutable qdCoordsAnimationPoint start_point_;
	mutable int cur_point_;

	Vect3f cur_pos() const;
	class qdGameObjectAnimated *object() const;

	void calc_paths() const;

	// Дельта (вектор) смещения координат анимации
	mutable Vect3f del_;
};

//typedef std::list<qdCoordsAnimation*> qdCoordsAnimationList;

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_COORDS_ANIMATION_H
