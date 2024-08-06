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

#ifndef QDENGINE_QDCORE_QD_COORDS_ANIMATION_H
#define QDENGINE_QDCORE_QD_COORDS_ANIMATION_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_coords_animation_point.h"
#include "qdengine/qdcore/qd_named_object_reference.h"


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
		return _speed;
	}
	void set_speed(float sp) {
		_speed = sp;
	}

	const qdGameObject *start_object() const {
		return _start_object;
	}
	void set_start_object(const qdGameObject *p_obj);
	const qdNamedObjectReference &start_object_ref() const {
		return _start_object_ref;
	}

	bool is_playing() const {
		return _status;
	}
	bool is_finished() const {
		return _is_finished;
	}

	qdCoordsAnimationType type() const {
		return _type;
	}
	void set_type(qdCoordsAnimationType tp) {
		_type = tp;
	}

	float animation_phase() const {
		return _animation_phase;
	}
	void set_animation_phase(float p) {
		_animation_phase = p;
	}

	void start() const;
	void stop() const;
	void quant(float dt) const;

	bool reset_cur_point() const;
	bool set_cur_point(int point_num) const;
	int get_cur_point() const {
		return _cur_point;
	}

	void set_time_rel(float tm);

	void add_point(const qdCoordsAnimationPoint *p);
	void insert_point(const qdCoordsAnimationPoint *p, int insert_pos);

	void remove_point(int num);
	void clear();

	qdCoordsAnimationPoint *get_point(int index = 0) {
		return &_points[index];
	};
	const qdCoordsAnimationPoint *get_point(int index = 0) const {
		return &_points[index];
	};

	int size() const {
		return _points.size();
	}

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	bool is_empty() const {
		if (_points.empty()) return true;
		return false;
	}

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

private:

	qdCoordsAnimationType _type;
	qdCoordsAnimationPointVector _points;
	float _animation_phase;
	float _speed;

	// Для относительного перемещения персонажа (QD_COORDS_ANM_RELATIVE_FLAG):
	// объект, относительно коориднат которого будет работать траектория движения (если не задано, то работаем
	// относительно координат объекта-владельца координатной анимации)
	const qdGameObject *_start_object;
	qdNamedObjectReference _start_object_ref;

	mutable bool _status;
	mutable bool _is_finished;
	mutable qdCoordsAnimationPoint _start_point;
	mutable int _cur_point;

	Vect3f cur_pos() const;
	class qdGameObjectAnimated *object() const;

	void calc_paths() const;

	// Дельта (вектор) смещения координат анимации
	mutable Vect3f _del;
};

//typedef Std::list<qdCoordsAnimation*> qdCoordsAnimationList;

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_COORDS_ANIMATION_H
