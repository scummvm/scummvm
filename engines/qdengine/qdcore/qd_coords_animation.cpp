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

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/xmath.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/qdcore/qd_coords_animation.h"
#include "qdengine/qdcore/qd_game_object_animated.h"
#include "qdengine/qdcore/qd_game_object_moving.h"


namespace QDEngine {

qdCoordsAnimation::qdCoordsAnimation() : _status(false),
	_is_finished(false),
	_type(CA_INTERPOLATE_COORDS),
	_animation_phase(0.0f),
	_speed(100.0f),
	_start_object(NULL),
	_cur_point(0) {
}

qdCoordsAnimation::qdCoordsAnimation(const qdCoordsAnimation &anm) : qdNamedObject(anm),
	_status(false),
	_is_finished(false),
	_type(anm._type),
	_animation_phase(anm._animation_phase),
	_speed(anm._speed),
	_start_object(anm._start_object),
	_start_object_ref(anm.start_object_ref()),
	_points(anm._points),
	_cur_point(0) {
}

qdCoordsAnimation &qdCoordsAnimation::operator = (const qdCoordsAnimation &anm) {
	qdNamedObject::operator=(anm);

	_status = false;
	_is_finished = false;
	_type = anm._type;
	_animation_phase = anm._animation_phase;
	_speed = anm._speed;
	// Внутри функции устанавливается и _start_object_ref
	set_start_object(anm.start_object());
	_points = anm._points;
	_cur_point = 0;

	return *this;
}

qdCoordsAnimation::~qdCoordsAnimation() {
	_points.clear();
}

void qdCoordsAnimation::set_start_object(const qdGameObject *p_obj) {
	_start_object = p_obj;
	if (NULL != p_obj) {
		qdNamedObjectReference ref(p_obj);
		_start_object_ref = ref;
	} else
		_start_object_ref.clear();
}

void qdCoordsAnimation::calc_paths() const {
	for (size_t i = 1; i < _points.size(); i++)
		_points[i].calc_path(_points[i - 1]);
}

void qdCoordsAnimation::add_point(const qdCoordsAnimationPoint *p) {
	_points.push_back(*p);
	calc_paths();
}

void qdCoordsAnimation::insert_point(const qdCoordsAnimationPoint *p, int insert_pos) {
	_points.insert(_points.begin() + insert_pos, *p);
	calc_paths();
}

void qdCoordsAnimation::remove_point(int num) {
	assert(0 <= num && num < (int)_points.size());

	_points.erase(_points.begin() + num);

	if (_cur_point >= (int)_points.size())
		_cur_point = _points.size() - 1;

	calc_paths();
}

void qdCoordsAnimation::clear() {
	if (!_points.empty())
		set_cur_point(0);
	_cur_point = -1;
	_points.clear();
	clear_flags();
}

void qdCoordsAnimation::start() const {
	_cur_point = 0;

	if (!_points.empty()) {
		qdGameObjectAnimated *p = object();
		if (!p) return;

		// Объект будет двигаться по шаблону =>
		// он выходит из состояния следования
		qdGameObjectMoving *obj = dynamic_cast<qdGameObjectMoving *>(p);
		if (NULL != obj)
			obj->set_follow_condition(qdGameObjectMoving::FOLLOW_DONE);

		// Если траектория должна выполняться относительно текущего положения (или
		// от центра заданного объекта), то считаем дельту
		if (true == check_flag(QD_COORDS_ANM_RELATIVE_FLAG)) {
			if (NULL != _start_object)
				_del = _points[0].dest_pos() - _start_object->R();
			else _del = _points[0].dest_pos() - p->R();
		} else {
			_del.x = 0;
			_del.y = 0;
			_del.z = 0;
		};

		if (check_flag(QD_COORDS_ANM_OBJECT_START_FLAG))
			_start_point.set_dest_pos(p->R());
		else {
			_start_point.set_dest_pos(_points[0].dest_pos() - _del);
			// Задана коорд. анимация с перемещением в точку и задан угол =>
			// устанавливаем направление для движущегося объекта
			qdGameObjectMoving *mov_obj = dynamic_cast<qdGameObjectMoving *>(obj);
			if ((NULL != mov_obj) &&
			        (qdCoordsAnimationPoint::NO_DIRECTION != _points[0].direction_angle()))
				mov_obj->set_direction(_points[0].direction_angle());
		}

		_points[0].calc_path(_start_point, _del);

		_points[0].start();
		_status = true;

		p->set_pos(cur_pos());
	}

	_is_finished = false;
}

void qdCoordsAnimation::stop() const {
	_status = false;
}

void qdCoordsAnimation::quant(float dt) const {
	if (!_status || !_points.size()) return;

	if (_type == CA_INTERPOLATE_COORDS) {
		float path = _speed * dt;
		while (_points[_cur_point].move(path)) {
			if (++_cur_point >= (int)_points.size()) {
				_is_finished = true;
				if (!check_flag(QD_COORDS_ANM_LOOP_FLAG)) {
					_cur_point --;
					stop();
					break;
				}

				_start_point.set_dest_pos(_points[_points.size() - 1].dest_pos() - _del);
				// Расстояние считаем, переместившись в глобальне координаты, так как _points в глобальных
				_points[0].calc_path(_start_point, _del);

				_cur_point = 0;
			}
			_points[_cur_point].start();
		}

		qdGameObjectAnimated *obj = object();
		if (obj) obj->set_pos(cur_pos());
	} else { // _type == CA_WALK
		qdGameObjectAnimated *obj = object();
		if (obj->named_object_type() != QD_NAMED_OBJECT_MOVING_OBJ) return;
		qdGameObjectMoving *p = static_cast<qdGameObjectMoving *>(obj);

		if (p->is_in_position(_points[_cur_point].dest_pos() - _del)) {
			if (++_cur_point >= (int)_points.size()) {
				_is_finished = true;
				if (!check_flag(QD_COORDS_ANM_LOOP_FLAG)) {
					stop();
					return;
				}

				_start_point.set_dest_pos(_points[_points.size() - 1].dest_pos() - _del);
				// Расстояние считаем, переместившись в глобальне координаты
				_points[0].calc_path(_start_point, _del);

				_cur_point = 0;
			}

			_points[_cur_point].start();
		}

		if (!p->is_moving())
			p->move(_points[_cur_point].dest_pos() - _del, _points[_cur_point].direction_angle());
	}
}

void qdCoordsAnimation::load_script(const xml::tag *p) {
	int v;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_COORDS_ANIMATION_POINT: {
			qdCoordsAnimationPoint fp;
			fp.load_script(&*it);
			add_point(&fp);
		}
		break;
		case QDSCR_TYPE:
			xml::tag_buffer(*it) > v;
			set_type((qdCoordsAnimationType)v);
			break;
		case QDSCR_FLAG:
			xml::tag_buffer(*it) > v;
			set_flag(v);
			break;
		case QDSCR_SPEED:
			xml::tag_buffer(*it) > _speed;
			break;
		case QDSCR_ANIMATION_PHASE:
			xml::tag_buffer(*it) > _animation_phase;
			break;
		case QDSCR_NAMED_OBJECT:
			_start_object_ref.load_script(&*it);
			break;
		}
	}
}

bool qdCoordsAnimation::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<coords_animation");

	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	} else {
		fh.writeString(" name=\" \"");
	}

	fh.writeString(Common::String::format(" type=\"%d\"", (int)_type));
	fh.writeString(Common::String::format(" speed=\"%f\"", _speed));
	fh.writeString(Common::String::format(" animation_phase=\"%f\"", _animation_phase));

	fh.writeString(">\r\n");

	for (auto &it: _points) {
		it.save_script(fh, indent + 1);
	}

	if (flags()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<flag>%d</flag>\r\n", flags()));
	}

	if (NULL != _start_object) {
		_start_object_ref.save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</coords_animation>\r\n");

	return true;
}

Vect3f qdCoordsAnimation::cur_pos() const {
	// В cur_pos() учитываем дельту относительных координат
	qdCoordsAnimationPoint p0;
	if (0 != _cur_point) {
		p0 = _points[_cur_point - 1];
		p0.set_dest_pos(p0.dest_pos() - _del); // Перемещаемся в относительные координаты
	} else p0 = _start_point;
	qdCoordsAnimationPoint p1 = _points[_cur_point];
	p1.set_dest_pos(p1.dest_pos() - _del);     // Перемещаемся в относительные координаты

	Vect3f v = p0.dest_pos() + p1.passed_path() * (p1.dest_pos() - p0.dest_pos());
	return v;
}

qdGameObjectAnimated *qdCoordsAnimation::object() const {
	if (owner() && owner()->named_object_type() == QD_NAMED_OBJECT_OBJ_STATE) {
		qdNamedObject *p = owner()->owner();
		if (p && (p->named_object_type() == QD_NAMED_OBJECT_ANIMATED_OBJ || p->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ))
			return static_cast<qdGameObjectAnimated *>(p);
	}

	return 0;
}

bool qdCoordsAnimation::set_cur_point(int point_num) const {
	if (!_points.size()) return false;

	start();

	if (_type == CA_WALK) {
		if (!object() || object()->named_object_type() != QD_NAMED_OBJECT_MOVING_OBJ) return false;
		qdGameObjectMoving *p = static_cast<qdGameObjectMoving *>(object());
		p->set_pos(cur_pos());
		p->drop_flag(QD_OBJ_MOVING_FLAG);

		p->move(_points[_cur_point].dest_pos());
		p->skip_movement();

		for (int i = 0; i < point_num; i++) {
			if (++_cur_point >= (int)_points.size()) {
				if (!check_flag(QD_COORDS_ANM_LOOP_FLAG)) {
					stop();
					return false;
				}

				_start_point.set_dest_pos(_points[_points.size() - 1].dest_pos());
				_points[0].calc_path(_start_point);

				_cur_point = 0;
			}

			_points[_cur_point].start();

			if (!p->move(_points[_cur_point].dest_pos())) return false;
			if (!p->skip_movement()) return false;
		}

		return true;
	} else {
		if (!object() || _speed < 0.01f) return false;
		qdGameObjectAnimated *p = object();
		p->get_animation()->set_time_rel(animation_phase());
		p->set_pos(cur_pos());

		for (int i = 0; i < point_num; i++) {
			if (++_cur_point >= (int)_points.size()) {
				if (!check_flag(QD_COORDS_ANM_LOOP_FLAG)) {
					stop();
					if (p->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
						static_cast<qdGameObjectMoving *>(p)->adjust_z();

					return false;
				}

				_start_point.set_dest_pos(_points[_points.size() - 1].dest_pos());
				_points[0].calc_path(_start_point);

				_cur_point = 0;
			}

			_points[_cur_point].start();

			p->set_pos(_points[_cur_point].dest_pos());
			p->get_animation()->advance_time(_points[_cur_point].path_length() / _speed);
		}

		if (p->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			static_cast<qdGameObjectMoving * >(p)->adjust_z();

		return true;
	}
	/*
	        const float dt = 0.005f;
	        while(_cur_point < point_num || _points[_cur_point].passed_path() < 0.99f){
	            quant(dt);
	            p->quant(dt);
	        }

	        return true;
	    }
	*/
	return false;
}

bool qdCoordsAnimation::reset_cur_point() const {
	return set_cur_point(_cur_point);
}

void qdCoordsAnimation::set_time_rel(float tm) {
	assert(tm >= 0.0f && tm <= 1.0f);

	if (!_points.size()) return;

	int pt = tm * float(_points.size() - 1);
	set_cur_point(pt);
}

bool qdCoordsAnimation::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(4, kDebugSave, "    qdCoordsAnimation::load_data(): before: %d", (int)fh.pos());

	if (!qdNamedObject::load_data(fh, save_version)) return false;

	int v;
	v = fh.readSint32LE();
	_status = (v) ? true : false;

	v = fh.readSint32LE();
	_is_finished = (v) ? true : false;

	_cur_point = fh.readSint32LE();
	v = fh.readSint32LE();
	if ((int)_points.size() != v) return false;

	if (save_version >= 101) {
		_del.x = fh.readFloatLE();
		_del.y = fh.readFloatLE();
		_del.z = fh.readFloatLE();
	} else {
		_del = Vect3f(0, 0, 0);
	}

	for (auto &it : _points)
		it.load_data(fh, save_version);

	_start_point.load_data(fh, save_version);

	Vect3f vec;
	vec.x = fh.readFloatLE();
	vec.y = fh.readFloatLE();
	vec.z = fh.readFloatLE();
	_start_point.set_dest_pos(vec);

	debugC(4, kDebugSave, "    qdCoordsAnimation::load_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdCoordsAnimation::save_data(Common::WriteStream &fh) const {
	debugC(4, kDebugSave, "    qdCoordsAnimation::save_data(): before: %d", (int)fh.pos());
	if (!qdNamedObject::save_data(fh)) return false;

	fh.writeSint32LE(static_cast<int>(_status));
	fh.writeSint32LE(static_cast<int>(_is_finished));
	fh.writeSint32LE(_cur_point);
	fh.writeUint32LE(_points.size());

	fh.writeFloatLE(_del.x);
	fh.writeFloatLE(_del.y);
	fh.writeFloatLE(_del.z);

	for (qdCoordsAnimationPointVector::const_iterator it = _points.begin(); it != _points.end(); ++it)
		it->save_data(fh);

	_start_point.save_data(fh);

	fh.writeFloatLE(_start_point.dest_pos().x);
	fh.writeFloatLE(_start_point.dest_pos().y);
	fh.writeFloatLE(_start_point.dest_pos().z);
	debugC(4, kDebugSave, "    qdCoordsAnimation::save_data(): after: %d", (int)fh.pos());
	return true;
}
} // namespace QDEngine
