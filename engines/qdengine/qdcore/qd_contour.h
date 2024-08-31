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

#ifndef QDENGINE_QDCORE_QD_CONTOUR_H
#define QDENGINE_QDCORE_QD_CONTOUR_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/xmath.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

//! Контур.
/**
Используется для задания масок на статических объектах,
зон на сетке и формы элементов GUI.
*/
class qdContour {
public:
	enum qdContourType {
		//! Прямоугольник
		CONTOUR_RECTANGLE,
		//! Окружность
		CONTOUR_CIRCLE,
		//! Произвольный многоугольник
		CONTOUR_POLYGON
	};

	qdContour(qdContourType tp = CONTOUR_POLYGON);
	qdContour(const qdContour &ct);
	virtual ~qdContour();

	qdContour &operator = (const qdContour &ct);

	//! Возвращает тип контура.
	qdContourType contour_type() const {
		return _contour_type;
	}
	//! Устанавливает тип контура.
	void set_contour_type(qdContourType tp) {
		_contour_type = tp;
	}

	//! Возвращает горизонтальный размер контура.
	int size_x() const {
		return _size.x;
	}
	//! Возвращает вертикальный размер контура.
	int size_y() const {
		return _size.y;
	}

	//! Возвращает true, если точка с координатами pos лежит внутри контура.
	bool is_inside(const Vect2s &pos) const;

	//! Запись данных в скрипт.
	virtual bool save_script(Common::WriteStream &fh, int indent = 0) const;
	//! Чтение данных из скрипта.
	virtual bool load_script(const xml::tag *p);

	//! Добавляет точку в контур.
	/**
	pt - координаты точки
	*/
	void add_contour_point(const Vect2s &pt);
	//! Вставляет точку в контур.
	/**
	pt - координаты точки, insert_pos - номер точки, перед которой добавится новая.
	*/
	void insert_contour_point(const Vect2s &pt, int insert_pos = 0);
	//! Удаляет точку номер pos из контура.
	bool remove_contour_point(int pos);
	//! Присваивает точке номер pos контура координаты pt.
	bool update_contour_point(const Vect2s &pt, int pos);

	//! Удаляет все точки из контура.
	void clear_contour() {
		_contour.clear();
	}

	//! Делит координаты контура на 2 в степени shift.
	void divide_contour(int shift = 1);
	//! Умножает координаты контура на 2 в степени shift.
	void mult_contour(int shift = 1);
	//! Добавляет к соответствующим координатам dx, dy.
	void shift_contour(int dx, int dy);

	//! Возвращает координаты точки контура номер pos.
	const Vect2s &get_point(int pos) const {
		return _contour[pos];
	}

	//! Возвращает размеры маски.
	const Vect2s &mask_size() const {
		return _size;
	}

	//! Возвращает координаты центра маски.
	const Vect2s &mask_pos() const {
		return _mask_pos;
	}

	//! Возвращает количество точек в контуре.
	int contour_size() const {
		return _contour.size();
	}
	//! Возвращает массив точек контура.
	const Std::vector<Vect2s> &get_contour() const {
		return _contour;
	}

	bool is_mask_empty() const {
		return _contour.empty();
	}

	bool update_contour();

	// можно ли замкнуть текущий контур.
	// для типов контура CONTOUR_CIRCLE и CONTOUR_RECTANGLE
	// всегда возвращается false
	bool can_be_closed() const;

	//! Возвращает true, если контур пустой.
	bool is_contour_empty() const;

private:
	//! Тип контура.
	qdContourType _contour_type;

	//! Размеры контура.
	Vect2s _size;

	Vect2s _mask_pos;

	//! Контур.
	/**
	Произвольный замкнутый контур. Задается для CONTOUR_POLYGON.
	*/
	Std::vector<Vect2s> _contour;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_CONTOUR_H
