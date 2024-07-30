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

#ifndef QDENGINE_QDCORE_QD_GAME_OBJECT_MOUSE_H
#define QDENGINE_QDCORE_QD_GAME_OBJECT_MOUSE_H

#include "qdengine/qdcore/qd_game_object_animated.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

//! Mouse
class qdGameObjectMouse : public qdGameObjectAnimated {
public:
	qdGameObjectMouse();
	qdGameObjectMouse(const qdGameObjectMouse &obj);
	~qdGameObjectMouse();

	qdGameObjectMouse &operator = (const qdGameObjectMouse &obj);

	int named_object_type() const {
		return QD_NAMED_OBJECT_MOUSE_OBJ;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	bool load_data(Common::SeekableReadStream &fh, int save_version);
	bool save_data(Common::WriteStream &fh) const;

	void take_object(qdGameObjectAnimated *p);
	qdGameObjectAnimated *object() {
		return _object;
	}

	//! Идентификаторы для курсоров по-умолчанию.
	enum cursor_ID_t {
		//! общий курсор по-умолчанию
		DEFAULT_CURSOR,
		//! курсор для анимированных объектов
		OBJECT_CURSOR,
		//! курсор для зон на сетке
		ZONE_CURSOR,
		//! курсор для main menu
		MAIN_MENU_CURSOR,
		//! курсор для внутриигрового интерфейса
		INGAME_INTERFACE_CURSOR,
		//! курсор для объектов в инвентори
		INVENTORY_OBJECT_CURSOR,

		MAX_CURSOR_ID
	};

	//! Установка курсора по-умолчанию.
	/**
	Номер состояния отсчитывается с нуля.
	*/
	void set_default_cursor(cursor_ID_t id, int state_index) {
		_default_cursors[id] = state_index;
	}
	//! Возвращает курсор по-умолчанию.
	int default_cursor(cursor_ID_t id) const {
		return _default_cursors[id];
	}

	void set_cursor(cursor_ID_t id);
	void set_cursor_state(int state_idx) {
		if (cur_state() != state_idx) set_state(state_idx);
	}

	void pre_redraw();
	void redraw(int offs_x = 0, int offs_y = 0) const;
	void post_redraw();

	void quant(float dt);

	bool update_screen_pos();

	const Vect2f &screen_pos_offset() const {
		return _screen_pos_offset;
	}

protected:

	bool load_script_body(const xml::tag *p);
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;

private:

	//! Объект, который прицеплен к курсору.
	qdGameObjectAnimated *_object;

	//! Номера состояний, соответствующие курсорам по-умолчанию.
	int _default_cursors[MAX_CURSOR_ID];

	grScreenRegion _object_screen_region;

	Vect2f _screen_pos_offset;
	Vect2f _screen_pos_offset_delta;

	void update_object_position() const;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_OBJECT_MOUSE_H
