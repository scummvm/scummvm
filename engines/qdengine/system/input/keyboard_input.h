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

#ifndef QDENGINE_SYSTEM_INPUT_KEYBOARD_INPUT_H
#define QDENGINE_SYSTEM_INPUT_KEYBOARD_INPUT_H

#include "common/keyboard.h"

namespace QDEngine {

//! Обработчик ввода с клавиатуры.
class keyboardDispatcher {
public:
	//! Обработчик нажатий/отжатий кнопок.
	/**
	Возвращает true при успешной обработке события.
	*/
	typedef bool (*event_handler_t)(Common::KeyCode key_vcode, bool event);

	keyboardDispatcher();
	~keyboardDispatcher();

	//! Возвращает true, если кнопка с кодом vkey в данный момент нажата.
	bool is_pressed(int vkey) const {
		assert(vkey >= 0 && vkey < 256);
		return _key_states[vkey];
	}

	//! Устанавливает обработчик нажатий/отжатий кнопок.
	event_handler_t set_handler(event_handler_t h) {
		event_handler_t old_h = _handler;
		_handler = h;
		return old_h;
	}

	//! Обрабатывает нажатие (event == true) или отжатие (event == false) кнопки с кодом vkey.
	bool handle_event(Common::KeyCode vkey, bool event) {
		_key_states[vkey] = event;
		if (_handler) return (*_handler)(vkey, event);
		return false;
	}

	//! Возвращает диспетчер по-умолчанию.
	static keyboardDispatcher *instance();

private:

	//! Обработчик нажатий/отжатий кнопок.
	event_handler_t _handler;

	//! Состояния кнопок - _key_states[vkey] == true если кнопка с кодом vkey нажата.
	bool _key_states[Common::KEYCODE_LAST];
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_INPUT_KEYBOARD_INPUT_H
