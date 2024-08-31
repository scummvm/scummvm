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

#ifndef QDENGINE_SYSTEM_INPUT_MOUSE_INPUT_H
#define QDENGINE_SYSTEM_INPUT_MOUSE_INPUT_H


namespace QDEngine {

//! Обработчик мыши.
class mouseDispatcher {
public:
	mouseDispatcher();
	~mouseDispatcher();

	//! Обработчик событий.
	/**
	Возвращает true при успешной обработке события.
	*/
	typedef bool (*event_handler_t)(int x, int y, int flags);

	//! События.
	enum mouseEvent {
		//! Нажатие левой кнопки.
		EV_LEFT_DOWN = 0,
		//! Нажатие правой кнопки.
		EV_RIGHT_DOWN,
		//! Двойное нажатие левой кнопки.
		EV_LEFT_DBLCLICK,
		//! Двойное нажатие правой кнопки.
		EV_RIGHT_DBLCLICK,
		//! Отжатие левой кнопки.
		EV_LEFT_UP,
		//! Отжатие правой кнопки.
		EV_RIGHT_UP,
		//! Перемещение мыши.
		EV_MOUSE_MOVE
	};

	//! Идентификаторы кнопок.
	enum mouseButtonID {
		ID_BUTTON_LEFT,
		ID_BUTTON_MIDDLE,
		ID_BUTTON_RIGHT
	};

	//! Установка обработчика события.
	event_handler_t set_event_handler(mouseEvent ev, event_handler_t h) {
		event_handler_t old_h = _event_handlers[ev];
		_event_handlers[ev] = h;
		return old_h;
	}

	//! Обработка события.
	bool handle_event(mouseEvent ev, int x, int y, int flags);

	//! Возвращает true, если событие происходило и пока не обработано.
	bool check_event(mouseEvent ev) const {
		if (_events & (1 << ev)) return true;
		else return false;
	}
	//! Возвращает true, если событие происходило с момента вызова clear_events().
	bool is_event_active(mouseEvent ev) const {
		if (_active_events & (1 << ev)) return true;
		else return false;
	}
	//! Очищает информацию о событиях.
	bool clear_events() {
		_events = _active_events = 0;
		return true;
	}
	//! Очищает информацию о событии ev.
	bool clear_event(mouseEvent ev) {
		_events &= ~(1 << ev);
		return true;
	}
	//! Помечает событие, как непроисходившее.
	bool deactivate_event(mouseEvent ev) {
		_active_events &= ~(1 << ev);
		return true;
	}
	//! Помечает событие как произошедшее.
	void toggle_event(mouseEvent ev) {
		_events |= (1 << ev);
		_active_events |= (1 << ev);
	}

	//! Возвращает горизонтальную координату мышиного курсора.
	int mouse_x() const {
		return _mouse_x;
	}
	//! Возвращает вертикальную координату мышиного курсора.
	int mouse_y() const {
		return _mouse_y;
	}

	//! Возвращает true, если кнопка bt_id нажата.
	bool is_pressed(mouseButtonID bt_id) {
		return (_button_status & (1 << bt_id));
	}

	//! Возвращает обработчик по-умолчанию.
	static mouseDispatcher *instance();

	//! Возвращает идентификатор первого события.
	static mouseEvent first_event_ID() {
		return EV_LEFT_DOWN;
	}
	//! Возвращает идентификатор последнего события.
	static mouseEvent last_event_ID() {
		return EV_MOUSE_MOVE;
	}

private:
	//! События - при успешной обработке события клиентом он скидывает соответсвующий флаг.
	int _events;
	//! События - для проверки, происходило ли событие с момента вызова clear_events()
	int _active_events;

	//! Статус кнопок - нажаты или нет.
	int _button_status;

	//! Горизонтальная координата мышиного курсора.
	int _mouse_x;
	//! Вертикальная координата мышиного курсора.
	int _mouse_y;

	//! Обработчики событий.
	event_handler_t _event_handlers[EV_MOUSE_MOVE + 1];
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_INPUT_MOUSE_INPUT_H
