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

#ifndef QDENGINE_QDCORE_QD_GRID_ZONE_H
#define QDENGINE_QDCORE_QD_GRID_ZONE_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_contour.h"
#include "qdengine/qdcore/qd_grid_zone_state.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

class qdCamera;

const int QD_NO_SHADOW_ALPHA        = -1;
const int QD_SHADOW_ALPHA_MIN       = 0;
const int QD_SHADOW_ALPHA_MAX       = 255;

//! Зона на сетке сцены.
class qdGridZone : public qdNamedObject, public qdContour {
public:
	qdGridZone();
	qdGridZone(const qdGridZone &gz);

	qdGridZone &operator = (const qdGridZone &gz);

	~qdGridZone();

	//! Команды для изменения состояния зоны в триггерах.
	enum status_change {
		//! выключить зону
		ZONE_OFF = 0,
		//! включить зону
		ZONE_ON,
		//! переключить зону
		ZONE_TOGGLE
	};

	//! Флаги
	enum {
		//! т.н. "зона выхода" - при наведении на нее мышью курсор меняется
		ZONE_EXIT_FLAG = 0x01
	};

	int named_object_type() const {
		return QD_NAMED_OBJECT_GRID_ZONE;
	}

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int saveVersion);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	bool state() const {
		return _state;
	}
	bool set_state(bool st);
	bool toggle_state() {
		return set_state(!_state);
	}

	uint32 height() const {
		return _height;
	}
	bool set_height(int _h);

	bool select(qdCamera *camera, bool bSelect) const;
	bool select(bool bSelect) const;

	bool is_object_in_zone(const qdGameObject *obj) const;
	bool is_point_in_zone(const Vect2f &vPoint) const;

	uint32 update_timer() const {
		return _update_timer;
	}

	qdGridZoneState *get_state(const char *state_name);

	//! Инициализация данных, вызывается при старте и перезапуске игры.
	bool init();

	bool is_any_personage_in_zone() const;

	int shadow_alpha() const {
		return _shadow_alpha;
	}
	void set_shadow_alpha(int alpha) {
		_shadow_alpha = alpha;
	}

	uint32 shadow_color() const {
		return _shadow_color;
	}
	void set_shadow_color(uint32 color) {
		_shadow_color = color;
	}
	bool has_shadow() const {
		return (_shadow_alpha != QD_NO_SHADOW_ALPHA);
	}

private:

	//! Текущее состояние зоны (вкл/выкл).
	bool _state;
	//! Начальное состояние зоны (вкл/выкл).
	bool _initial_state;

	//! Цвет затенения персонажа, когда он находится в зоне, RGB().
	uint32 _shadow_color;
	//! Прозрачность затенения персонажа, когда он находится в зоне.
	/**
	Значения - [0, 255].
	Допускается также QD_NO_SHADOW_ALPHA, затенения при этом отключается.
	*/
	int _shadow_alpha;

	//! Высота зоны.
	uint32 _height;

	//! Время изменения состояния.
	uint32 _update_timer;

	//! Состояние включающее зону.
	qdGridZoneState _state_on;
	//! Состояние выключающее зону.
	qdGridZoneState _state_off;

	bool apply_zone() const;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GRID_ZONE_H
