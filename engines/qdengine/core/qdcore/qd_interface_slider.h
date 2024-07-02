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


#ifndef QDENGINE_CORE_QDCORE_QD_INTERFACE_SLIDER_H
#define QDENGINE_CORE_QDCORE_QD_INTERFACE_SLIDER_H

#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/qdcore/qd_interface_element.h"
#include "qdengine/core/qdcore/qd_interface_element_state.h"


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
	bool keyboard_handler(int vkey);

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
		return inverse_direction_;
	}
	void set_inverse_direction(bool state) {
		inverse_direction_ = state;
	}

	const Vect2i &active_rectangle() const {
		return active_rectangle_;
	}
	void set_active_rectangle(const Vect2i &rect) {
		active_rectangle_ = rect;
	}

	const qdInterfaceElementState *background() const {
		return &background_;
	}
	void update_background(const qdInterfaceElementState &st) {
		background_ = st;
	}

	const Vect2i &background_offset() const {
		return background_offset_;
	}
	void set_background_offset(const Vect2i &offs) {
		background_offset_ = offs;
	}

	const qdInterfaceElementState *slider() const {
		return &slider_;
	}
	void update_slider(const qdInterfaceElementState &st) {
		slider_ = st;
	}

	float phase() const {
		return phase_;
	}
	void set_phase(float ph) {
		phase_ = ph;
	}

	orientation_t orientation() const {
		return orientation_;
	}
	void set_orientation(orientation_t v) {
		orientation_ = v;
	}

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	bool hit_test(int x, int y) const;

protected:

	//! Запись данных в скрипт.
	bool save_script_body(XStream &fh, int indent = 0) const;
	bool save_script_body(Common::SeekableWriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag *p);

private:
	//! Подложка под ползунком.
	qdInterfaceElementState background_;
	/// Смещение подложки относительно центра ползунка.
	Vect2i background_offset_;

	//! Ползунок.
	qdInterfaceElementState slider_;

	//! Анимация ползунка.
	qdAnimation slider_animation_;

	//! Прямоугольник, внутри которого ездит ползунок.
	/**
	По нему же отслеживаются мышиные клики.
	Отсчитывается от центра элемента, в экранных координатах.
	*/
	Vect2i active_rectangle_;

	//! Текущая фаза, диапазон значений - [0.0, 1.0].
	float phase_;

	//! Ориентация - по горизонтали или по вертикали.
	orientation_t orientation_;
	bool inverse_direction_;

	//! true, если слайдер следит за перемещениями мышиного курсора
	bool track_mouse_;

	bool set_slider_animation(const qdAnimation *anm, int anm_flags = 0);

	//! Пересчет фазы в смещение от центра элемента в экранных координатах.
	Vect2i phase2offset(float ph) const;
	//! Пересчет смещения от центра элемента в экранных координатах в фазу.
	float offset2phase(const Vect2i &offs) const;

};

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_INTERFACE_SLIDER_H
