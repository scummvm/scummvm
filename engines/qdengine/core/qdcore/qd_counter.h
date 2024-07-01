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

#ifndef QDENGINE_CORE_QDCORE_QD_COUNTER_H
#define QDENGINE_CORE_QDCORE_QD_COUNTER_H

#include <vector>
#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/qdcore/qd_named_object_reference.h"

namespace Common {
class SeekableWriteStream;
}

namespace QDEngine {

class qdGameObjectState;

class qdCounterElement {
public:
	qdCounterElement();
	~qdCounterElement();
	explicit qdCounterElement(const qdGameObjectState *p, bool inc_value = true);

	bool operator == (const qdGameObjectState *p) const {
		return state_ == p;
	}

	const qdGameObjectState *state() const {
		return state_;
	}
	bool increment_value() const {
		return increment_value_;
	}

	bool init();
	bool quant();

	bool load_script(const xml::tag *p);
	bool save_script(XStream &fh, int indent = 0) const;
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream &fh) const;
	bool save_data(Common::SeekableWriteStream &fh) const;

private:

	qdNamedObjectReference state_reference_;
	const qdGameObjectState *state_;
	bool last_state_status_;
	bool increment_value_;
};

//! Счетчик состояний.
class qdCounter : public qdNamedObject {
public:
	qdCounter();
	~qdCounter();

	//! Флаги счетчика.
	enum {
		//! принимает только положительные значения
		POSITIVE_VALUE          = 0x01
	};

	int named_object_type() const {
		return QD_NAMED_OBJECT_COUNTER;
	}

	int value() const {
		return value_;
	}
	void set_value(int value);
	void add_value(int value_delta);

	int value_limit() const {
		return value_limit_;
	}
	void set_value_limit(int limit) {
		value_limit_ = limit;
	}

	typedef std::vector<qdCounterElement> element_container_t;
	const element_container_t &elements() const {
		return elements_;
	}

	//! Добавление состояния. Возвращает false, если оно уже добавлено ранее.
	bool add_element(const qdGameObjectState *p, bool inc_value = true);

	bool remove_element(const qdGameObjectState *p);
	bool remove_element(int idx);
#ifdef _QUEST_EDITOR
	void remove_all_elements();
#endif // _QUEST_EDITOR

	void quant();

	void init();

	bool load_script(const xml::tag *p);
	bool save_script(XStream &fh, int indent = 0) const;
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream &fh) const;
	bool save_data(Common::SeekableWriteStream &fh);

protected:
private:

	//! Состояния, которые при включении изменяют значение счетчика.
	element_container_t elements_;

	//! Текущее значение счетчика.
	int value_;
	//! Предельное значение, по достижении которого счетчик скидывается в ноль.
	/**
	Если меньше или равно нулю - не учитывается.
	*/
	int value_limit_;
};

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_COUNTER_H
