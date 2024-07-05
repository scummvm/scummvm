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

#ifndef QDENGINE_CORE_QDCORE_QD_GAME_OBJECT_MOUSE_H
#define QDENGINE_CORE_QDCORE_QD_GAME_OBJECT_MOUSE_H

#include "qdengine/qdcore/qd_game_object_animated.h"

namespace Common {
class SeekableWriteStream;
}

namespace QDEngine {

//! Мышь.
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
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	bool load_data(qdSaveStream &fh, int save_version);
	bool save_data(qdSaveStream &fh) const;
	bool save_data(Common::SeekableWriteStream &fh);

	void take_object(qdGameObjectAnimated *p);
	qdGameObjectAnimated *object() {
		return object_;
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
		default_cursors_[id] = state_index;
	}
	//! Возвращает курсор по-умолчанию.
	int default_cursor(cursor_ID_t id) const {
		return default_cursors_[id];
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
		return screen_pos_offset_;
	}

protected:

	bool load_script_body(const xml::tag *p);
	bool save_script_body(Common::SeekableWriteStream &fh, int indent = 0) const;

private:

	//! Объект, который прицеплен к курсору.
	qdGameObjectAnimated *object_;

	//! Номера состояний, соответствующие курсорам по-умолчанию.
	int default_cursors_[MAX_CURSOR_ID];

	grScreenRegion object_screen_region_;

	Vect2f screen_pos_offset_;
	Vect2f screen_pos_offset_delta_;

	void update_object_position() const;
};

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_GAME_OBJECT_MOUSE_H
