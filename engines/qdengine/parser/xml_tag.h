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

#ifndef QDENGINE_PARSER_XML_TAG_H
#define QDENGINE_PARSER_XML_TAG_H

#include "qdengine/qdengine.h"

namespace QDEngine {

//! Парсер XML на базе expat.
namespace xml {

//! XML тег.
class tag {
public:
	typedef Std::list<tag> subtags_t;
	typedef subtags_t::const_iterator subtag_iterator;

	//! Формат данных тега.
	enum tag_data_format {
		//! данные отсутствуют
		TAG_DATA_VOID,
		//! данные типа int16 int
		TAG_DATA_SHORT,
		//! данные типа uint16 int
		TAG_DATA_UNSIGNED_SHORT,
		//! данные типа int
		TAG_DATA_INT,
		//! данные типа uint32
		TAG_DATA_UNSIGNED_INT,
		//! данные типа float
		TAG_DATA_FLOAT,
		//! строковые данные
		TAG_DATA_STRING
	};

	tag(int id = 0, tag_data_format data_fmt = TAG_DATA_VOID, int data_sz = 0, int data_offs = 0) : _ID(id), _data_format(data_fmt), _data_size(data_sz), _data_offset(data_offs), _data(NULL) { }
	tag(const tag &tg) : _ID(tg._ID), _data_format(tg._data_format), _data_size(tg._data_size), _data_offset(tg._data_offset), _data(tg._data), _subtags(tg._subtags) { }
	~tag() { }
	bool readTag(Common::SeekableReadStream *ff, tag &tg);

	tag &operator = (const tag &tg) {
		if (this == &tg) return *this;

		_ID = tg._ID;
		_data_format = tg._data_format;
		_data_size = tg._data_size;
		_data_offset = tg._data_offset;

		_subtags = tg._subtags;

		return *this;
	}

	//! Возвращает идентификатор тега.
	int ID() const {
		return g_engine->_tagMap[_ID - 1];
	}

	int origID() const {
		return _ID;
	}

	//! Возвращает формат данных тега.
	tag_data_format data_format() const {
		return _data_format;
	}

	//! Возвращает количество элеметов данных тега.
	/**
	Чтобы получить размер данных в байтах, надо это число
	умножить на размер элемента данных в байтах - data_elemet_size().
	*/
	int data_size() const {
		return _data_size;
	}
	//! Возвращает размер элемента данных тега в байтах.
	int data_element_size() const {
		switch (_data_format) {
		case TAG_DATA_VOID:
			return 0;
		case TAG_DATA_SHORT:
		case TAG_DATA_UNSIGNED_SHORT:
			return sizeof(uint16);
		case TAG_DATA_INT:
			return sizeof(int32);
		case TAG_DATA_UNSIGNED_INT:
			return sizeof(uint32);
		case TAG_DATA_FLOAT:
			return sizeof(float);
		case TAG_DATA_STRING:
			return sizeof(byte);
		}
		return 0;
	}

	//! Устанавливает количество элементов данных тега.
	void set_data_size(int sz) {
		_data_size = sz;
	}

	//! Возвращает смещение до данных тега в данных парсера.
	int data_offset() const {
		return _data_offset;
	}
	//! Устанавливает смещение до данных тега в данных парсера.
	void set_data_offset(int off) {
		_data_offset = off;
	}

	//! Возвращает указатель на данные тега.
	const char *data() const {
		return &*(_data->begin() + _data_offset);
	}

	//! Устанавливает указатель на общие данные.
	void set_data(const Std::vector<char> *p) {
		_data = p;

		for (subtags_t::iterator it = _subtags.begin(); it != _subtags.end(); ++it)
			it->set_data(p);
	}

	//! Очистка вложенных тегов.
	void clear() {
		_subtags.clear();
	}
	//! Добавляет вложенный тег.
	/**
	Возвращает ссылку на последний вложенный тег.
	*/
	tag &add_subtag(const tag &tg) {
		_subtags.push_back(tg);
		return _subtags.back();
	}
	//! Возвращает true, если список вложенных тегов не пустой .
	bool has_subtags() const {
		return !_subtags.empty();
	}
	//! Возвращает количество вложенных тэгов.
	int num_subtags() const {
		return _subtags.size();
	}
	//! Возвращает итератор начала списка вложенных тегов.
	subtag_iterator subtags_begin() const {
		return _subtags.begin();
	}
	//! Возвращает итератор конца списка вложенных тегов.
	subtag_iterator subtags_end() const {
		return _subtags.end();
	}
	//! Поиск вложенного тега по его идентификатору.
	const tag *search_subtag(int subtag_id) const {
		for (subtag_iterator it = subtags_begin(); it != subtags_end(); ++it)
			if (it->ID() == subtag_id) return &*it;

		return NULL;
	}

private:

	//! Идентификатор (тип) тега.
	int _ID;
	//! Формат данных тега.
	tag_data_format _data_format;
	//! Количество элементов данных тега.
	int _data_size;
	//! Смещение до данных тега в общих данных.
	int _data_offset;
	//! Указатель на данные.
	const Std::vector<char> *_data;

	//! Список вложенных тегов.
	subtags_t _subtags;
};

} /* namespace xml */

} // namespace QDEngine

#endif // QDENGINE_PARSER_XML_TAG_H
