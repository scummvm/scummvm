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

#ifndef QDENGINE_QDCORE_QD_NAMED_OBJECT_H
#define QDENGINE_QDCORE_QD_NAMED_OBJECT_H

#include "common/stream.h"
#include "qdengine/qdcore/qd_named_object_base.h"

namespace QDEngine {

//! Типы поименованных объектов.
enum qdNamedObjectType {
	QD_NAMED_OBJECT_GENERIC,
	QD_NAMED_OBJECT_SCALE_INFO,
	QD_NAMED_OBJECT_TRIGGER_CHAIN,
	QD_NAMED_OBJECT_SOUND,
	QD_NAMED_OBJECT_ANIMATION,
	QD_NAMED_OBJECT_ANIMATION_INFO,
	QD_NAMED_OBJECT_COORDS_ANIMATION,
	QD_NAMED_OBJECT_OBJ_STATE,
	QD_NAMED_OBJECT_STATIC_OBJ,
	QD_NAMED_OBJECT_ANIMATED_OBJ,
	QD_NAMED_OBJECT_MOVING_OBJ,
	QD_NAMED_OBJECT_MOUSE_OBJ,
	QD_NAMED_OBJECT_SCENE,
	QD_NAMED_OBJECT_LOCATION,
	QD_NAMED_OBJECT_DISPATCHER,

	QD_NAMED_OBJECT_ANIMATION_SET,
	QD_NAMED_OBJECT_GRID_ZONE,
	QD_NAMED_OBJECT_VIDEO,
	QD_NAMED_OBJECT_INVENTORY,
	QD_NAMED_OBJECT_MINIGAME,
	QD_NAMED_OBJECT_MUSIC_TRACK,
	QD_NAMED_OBJECT_GRID_ZONE_STATE,

	QD_NAMED_OBJECT_SOUND_INFO,
	QD_NAMED_OBJECT_ANIMATION_SET_INFO,

	QD_NAMED_OBJECT_GAME_END,
	QD_NAMED_OBJECT_COUNTER,

	QD_NAMED_OBJECT_FONT_INFO,

	QD_NAMED_OBJECT_MAX_TYPE
};

const char *objectType2str(int id);

//! Поименованный объект.
class qdNamedObject : public qdNamedObjectBase {
public:
	qdNamedObject();
	qdNamedObject(const qdNamedObject &p);
	~qdNamedObject();

	qdNamedObject &operator = (const qdNamedObject &p);

	//! Возвращает владельца объекта.
	qdNamedObject *owner() const {
		return _owner;
	}
	//! Возвращает владельца объекта, тип которого tp.
	qdNamedObject *owner(qdNamedObjectType tp) const;

	//! Устанавливает владельца объекта.
	void set_owner(qdNamedObject *p) {
		_owner = p;
	}

	//! Устанавливает флаг.
	void set_flag(int fl) {
		_flags |= fl;
	}
	//! Скидывает флаг.
	void drop_flag(int fl) {
		_flags &= ~fl;
	}
	//! Возвращает true, если установлен флаг fl.
	bool check_flag(int fl) const {
		if (_flags & fl) return true;
		return false;
	}
	//! Очищает флаги.
	void clear_flags() {
		_flags = 0;
	}
	//! Возвращает значение флагов объекта.
	int flags() const {
		return _flags;
	}

	//! Возвращает тип объекта.
	virtual int named_object_type() const = 0;

	//! Загрузка данных из сэйва.
	virtual bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	virtual bool save_data(Common::WriteStream &fh) const;


	//! Добавляет ссылку из триггеров на объект.
	void add_trigger_reference() {
		_trigger_reference_count++;
	}
	//! Удаляет ссылку из триггеров на объект.
	void remove_trigger_reference() {
		if (_trigger_reference_count) _trigger_reference_count--;
	}
	//! Очищает счётчик ссылок из триггеров на объект.
	void clear_trigger_references() {
		_trigger_reference_count = 0;
	}
	//! Возвращает true, если на объект есть ссылки из триггеров.
	bool is_in_triggers() const {
		return (_trigger_reference_count > 0);
	}

	Common::String toString() const;

private:

	//! Некие свойства объекта.
	int _flags;

	//! Количество ссылок на объект из триггеров.
	int _trigger_reference_count;

	//! Владелец объекта.
	mutable qdNamedObject *_owner;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_NAMED_OBJECT_H
