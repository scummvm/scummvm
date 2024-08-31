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


#ifndef QDENGINE_QDCORE_QD_CAMERA_MODE_H
#define QDENGINE_QDCORE_QD_CAMERA_MODE_H

#include "qdengine/xmath.h"
#include "qdengine/parser/xml_fwd.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace QDEngine {

class qdGameObjectState;
class qdGameObjectAnimated;

//! Режим работы камеры.
class qdCameraMode {
public:
	qdCameraMode();
	~qdCameraMode() { }

	//! идентификаторы режимов камеры
	enum camera_mode_t {
		//! камера не выпускает объект с экрана
		MODE_OBJECT_ON_SCREEN,
		//! камера жестко привязана к объекту
		MODE_CENTER_OBJECT,
		//! камера все время центруется на объекте
		MODE_FOLLOW_OBJECT,
		//! когда объект подходит к краю экрана камера плавно на нем центруется
		MODE_CENTER_OBJECT_WHEN_LEAVING,
		//! режим не установлен
		MODE_UNASSIGNED
	};

	void set_camera_mode(camera_mode_t mode) {
		_camera_mode = mode;
	}
	camera_mode_t camera_mode() const {
		return _camera_mode;
	}

	void set_work_time(float tm) {
		_work_time = tm;
	}
	float work_time() const {
		return _work_time;
	}
	bool has_work_time() const {
		return _work_time > 0.001f;
	}

	void set_scrolling_speed(float v) {
		_scrolling_speed = v;
	}
	float scrolling_speed() const {
		return _scrolling_speed;
	}

	void set_scrolling_distance(int dist) {
		_scrolling_distance = dist;
	}
	int scrolling_distance() const {
		return _scrolling_distance;
	}

	bool smooth_switch() const {
		return _smooth_switch;
	}
	void set_smooth_switch(bool v) {
		_smooth_switch = v;
	}

	const Vect2i &center_offset() const {
		return _center_offset;
	}
	void set_center_offset(const Vect2i &offs) {
		_center_offset = offs;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

private:

	//! Идентификатор режима.
	camera_mode_t _camera_mode;

	//! Время работы режима (в секундах).
	/**
	По истечении этого времени камера переключается
	на активного персонажа (если он есть).

	Если это время нулевое, то сменить режим можно будет
	только в принудительном порядке из триггера.
	*/
	float _work_time;

	//! Скорость, с которой камера скроллируется (в пикселах в секунду).
	float _scrolling_speed;

	//! Расстояние от центра объекта до края экрана, при котором включается скроллинг.
	/**
	В режимах, когда камера не дает объекту выйти за пределы экрана или
	центруется на объекте когда он близко к краю экрана.

	Задается в пикселах.
	*/
	int _scrolling_distance;

	//! Смещение центра экрана в пикселах.
	/**
	Позволяет задавать режим, когда камера, центруясь на объекте,
	держит его в определенной точке экрана.
	*/
	Vect2i _center_offset;

	//! Плавное переключение в режим, если true.
	bool _smooth_switch;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_CAMERA_MODE_H
