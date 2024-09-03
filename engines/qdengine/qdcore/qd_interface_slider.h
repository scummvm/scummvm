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


#ifndef QDENGINE_QDCORE_QD_INTERFACE_SLIDER_H
#define QDENGINE_QDCORE_QD_INTERFACE_SLIDER_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_interface_element_state.h"


namespace QDEngine {


//! Интерфейсный элемент - слайдер.
/**
Состоит из двух частей - неподвижной подложки и "ползунка".
Подложка не обязательна, она может быть нарисована прямо на фоне.
*/
class qdInterfaceSlider : public qdInterfaceElement {
public:
	//! Ориентация слайдера (по-умолчанию - горизонтально).
	enum orientation_t {
		SL_HORIZONTAL,
		SL_VERTICAL
	};

	qdInterfaceSlider();
	qdInterfaceSlider(const qdInterfaceSlider &sl);
	~qdInterfaceSlider();

	qdInterfaceSlider &operator = (const qdInterfaceSlider &sl);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const {
		return qdInterfaceElement::EL_SLIDER;
	}

	//! Обработчик событий мыши.
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(Common::KeyCode vkey);

	int option_value() const;
	bool set_option_value(int value);

	//! Инициализация элемента.
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	//! Отрисовка элемента.
	bool redraw() const;

	grScreenRegion screen_region() const;

	//! Возвращает размер элемента по горизонтали в пикселах.
	int size_x() const;
	//! Возвращает размер элемента по вертикали в пикселах.
	int size_y() const;

	bool inverse_direction() const {
		return _inverse_direction;
	}
	void set_inverse_direction(bool state) {
		_inverse_direction = state;
	}

	const Vect2i &active_rectangle() const {
		return _active_rectangle;
	}
	void set_active_rectangle(const Vect2i &rect) {
		_active_rectangle = rect;
	}

	const qdInterfaceElementState *background() const {
		return &_background;
	}
	void update_background(const qdInterfaceElementState &st) {
		_background = st;
	}

	const Vect2i &background_offset() const {
		return _background_offset;
	}
	void set_background_offset(const Vect2i &offs) {
		_background_offset = offs;
	}

	const qdInterfaceElementState *slider() const {
		return &_slider;
	}
	void update_slider(const qdInterfaceElementState &st) {
		_slider = st;
	}

	float phase() const {
		return _phase;
	}
	void set_phase(float ph) {
		_phase = ph;
	}

	orientation_t orientation() const {
		return _orientation;
	}
	void set_orientation(orientation_t v) {
		_orientation = v;
	}

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	bool hit_test(int x, int y) const;

protected:

	//! Запись данных в скрипт.
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag *p);

private:
	//! Подложка под ползунком.
	qdInterfaceElementState _background;
	/// Смещение подложки относительно центра ползунка.
	Vect2i _background_offset;

	//! Ползунок.
	qdInterfaceElementState _slider;

	//! Анимация ползунка.
	qdAnimation _slider_animation;

	//! Прямоугольник, внутри которого ездит ползунок.
	/**
	По нему же отслеживаются мышиные клики.
	Отсчитывается от центра элемента, в экранных координатах.
	*/
	Vect2i _active_rectangle;

	//! Текущая фаза, диапазон значений - [0.0, 1.0].
	float _phase;

	//! Ориентация - по горизонтали или по вертикали.
	orientation_t _orientation;
	bool _inverse_direction;

	//! true, если слайдер следит за перемещениями мышиного курсора
	bool _track_mouse;

	bool set_slider_animation(const qdAnimation *anm, int anm_flags = 0);

	//! Пересчет фазы в смещение от центра элемента в экранных координатах.
	Vect2i phase2offset(float ph) const;
	//! Пересчет смещения от центра элемента в экранных координатах в фазу.
	float offset2phase(const Vect2i &offs) const;

};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_SLIDER_H
