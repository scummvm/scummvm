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


#ifndef QDENGINE_QDCORE_QD_INTERFACE_BUTTON_H
#define QDENGINE_QDCORE_QD_INTERFACE_BUTTON_H

#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_interface_element_state.h"


namespace QDEngine {

//! Интерфейсный элемент - кнопка.
class qdInterfaceButton : public qdInterfaceElement {
public:
	qdInterfaceButton();
	qdInterfaceButton(const qdInterfaceButton &bt);
	~qdInterfaceButton();

	qdInterfaceButton &operator = (const qdInterfaceButton &bt);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const {
		return qdInterfaceElement::EL_BUTTON;
	}

	//! Возвращает количество состояний кнопки.
	int num_states() const {
		return _states.size();
	}

	//! Включает состояние номер state_num.
	bool activate_state(int state_num);
	//! Включает состояние с именем state_name.
	bool activate_state(const char *state_name);

	int option_value() const {
		return _cur_state;
	}
	bool set_option_value(int value);

	//! Возвращает указатель на состояние кнопки.
	const qdInterfaceElementState *get_state(int state_num) const {
		assert(state_num >= 0 && state_num < (int)_states.size());
#ifndef _QD_INTERFACE_BUTTON_PTR_CONTAINER
		return &_states[state_num];
#else
		return &*_states[state_num];
#endif
	}
	//! Возвращает указатель на состояние кнопки.
	qdInterfaceElementState *get_state(int state_num) {
		assert(state_num >= 0 && state_num < (int)_states.size());
#ifndef _QD_INTERFACE_BUTTON_PTR_CONTAINER
		return &_states[state_num];
#else
		return &*_states[state_num];
#endif
	}

	//! Добавляет состояние кнопки.
	/**
	Параметры из состояния st копируются во внутренние данные.
	*/
	bool add_state(const qdInterfaceElementState &st);

	//! Добавляет состояние кнопки.
	/**
	Новое состояние добавляется в список перед состоянием номер insert_before.
	Параметры из состояния st копируются во внутренние данные.
	*/
	bool insert_state(int insert_before, const qdInterfaceElementState &st);

	//! Удаляет состояние номер state_num.
	bool erase_state(int state_num);

	//! Модифицирует состояние кнопки.
	/**
	Соответствующему состоянию присваивается значение st.
	*/
	bool modify_state(int state_num, const qdInterfaceElementState &st) {
		assert(state_num >= 0 && state_num < (int)_states.size());
#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
		*_states[state_num] = st;
		_states[state_num]->set_owner(this);
#else
		_states[state_num] = st;
		_states[state_num].set_owner(this);
#endif
		return true;
	}

	//! Переключает состояние кнопки.
	/**
	Если direction == true включается следующее состояние, если false - предыдущее.
	*/
	bool change_state(bool direction = true);

	//! Обработчик событий мыши.
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(Common::KeyCode vkey);

	//! Инициализация элемента.
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	//! Обсчет логики, параметр - время в секундах.
	bool quant(float dt);

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	bool hit_test(int x, int y) const;

	const qdInterfaceEvent *find_event(qdInterfaceEvent::event_t type) const;
	bool has_event(qdInterfaceEvent::event_t type, const char *ev_data = NULL) const;

protected:

	//! Запись данных в скрипт.
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag *p);

private:

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	typedef Std::vector<qdInterfaceElementState *> state_container_t;
#else
	typedef Std::vector<qdInterfaceElementState> state_container_t;
#endif

	//! Состояния кнопки.
	state_container_t _states;

	//! Текущее состояние кнопки.
	int _cur_state;

};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_BUTTON_H
