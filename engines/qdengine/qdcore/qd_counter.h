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

#ifndef QDENGINE_QDCORE_QD_COUNTER_H
#define QDENGINE_QDCORE_QD_COUNTER_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object_reference.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

class qdGameObjectState;

class qdCounterElement {
public:
	qdCounterElement();
	~qdCounterElement();
	explicit qdCounterElement(const qdGameObjectState *p, bool inc_value = true);

	bool operator == (const qdGameObjectState *p) const {
		return _state == p;
	}

	const qdGameObjectState *state() const {
		return _state;
	}
	bool increment_value() const {
		return _increment_value;
	}

	bool init();
	bool quant();

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

private:

	qdNamedObjectReference _state_reference;
	const qdGameObjectState *_state;
	bool _last_state_status;
	bool _increment_value;
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
		return _value;
	}
	void set_value(int value);
	void add_value(int value_delta);

	int value_limit() const {
		return _value_limit;
	}
	void set_value_limit(int limit) {
		_value_limit = limit;
	}

	typedef Std::vector<qdCounterElement> element_container_t;
	const element_container_t &elements() const {
		return _elements;
	}

	//! Добавление состояния. Возвращает false, если оно уже добавлено ранее.
	bool add_element(const qdGameObjectState *p, bool inc_value = true);

	bool remove_element(const qdGameObjectState *p);
	bool remove_element(int idx);

	void quant();

	void init();

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

protected:
private:

	//! Состояния, которые при включении изменяют значение счетчика.
	element_container_t _elements;

	//! Текущее значение счетчика.
	int _value;
	//! Предельное значение, по достижении которого счетчик скидывается в ноль.
	/**
	Если меньше или равно нулю - не учитывается.
	*/
	int _value_limit;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_COUNTER_H
