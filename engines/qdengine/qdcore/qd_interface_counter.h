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

#ifndef QDENGINE_QDCORE_QD_INTERFACE_COUNTER_H
#define QDENGINE_QDCORE_QD_INTERFACE_COUNTER_H

#include "qdengine/qdcore/qd_named_object_reference.h"
#include "qdengine/qdcore/qd_screen_text.h"
#include "qdengine/qdcore/qd_interface_element.h"


namespace QDEngine {

//! Interface widget for displaying a counter

class qdInterfaceCounter : public qdInterfaceElement {
public:
	qdInterfaceCounter();
	qdInterfaceCounter(const qdInterfaceCounter &counter);
	~qdInterfaceCounter();

	qdInterfaceCounter &operator = (const qdInterfaceCounter &counter);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const {
		return qdInterfaceElement::EL_COUNTER;
	}

	//! Обработчик событий мыши.
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(Common::KeyCode vkey);

	grScreenRegion screen_region() const;

	//! Инициализация элемента.
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	bool redraw() const;
	bool need_redraw() const;
	bool post_redraw();

	//! Возвращает размер элемента по горизонтали в пикселах.
	int size_x() const;
	//! Возвращает размер элемента по вертикали в пикселах.
	int size_y() const;

	/// количество отображаемых разрядов
	int digits() const {
		return _digits;
	}
	void setDigits(int digits) {
		_digits = digits;
	}

	const qdScreenTextFormat &textFormat() const {
		return _textFormat;
	}
	void setTextFormat(const qdScreenTextFormat &text_format) {
		_textFormat = text_format;
	}

	const qdCounter *counter() const {
		return _counter;
	}
	void setCounter(const qdCounter *counter);

	const char *counterName() const;
	void setCounterName(const char *name);

protected:

	//! Запись данных в скрипт.
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag *p);

	Common::String data() const;

private:

	qdScreenTextFormat _textFormat;

	int _digits;

	Common::String _counterName;
	const qdCounter *_counter;

	int _lastValue;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_COUNTER_H
