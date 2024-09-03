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


#ifndef QDENGINE_QDCORE_QD_INTERFACE_BACKGROUND_H
#define QDENGINE_QDCORE_QD_INTERFACE_BACKGROUND_H

#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_interface_element_state.h"


namespace QDEngine {

//! Интерфейсный элемент - фон.
class qdInterfaceBackground : public qdInterfaceElement {
public:
	qdInterfaceBackground();
	qdInterfaceBackground(const qdInterfaceBackground &bk);
	~qdInterfaceBackground();

	qdInterfaceBackground &operator = (const qdInterfaceBackground &bk);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const {
		return qdInterfaceElement::EL_BACKGROUND;
	}

	//! Обработчик событий мыши.
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(Common::KeyCode vkey);

	//! Инициализация элемента.
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	//! Устанавливает имя файла для анимации.
	/**
	Если надо убрать анимацию - передать NULL в качестве имени файла.
	*/
	void set_animation_file(const Common::Path name) {
		_state.set_animation_file(name);
	}
	//! Возвращает имя файла для анимации.
	const Common::Path animation_file() const {
		return _state.animation_file();
	}
	//! Возвращает флаги анимации.
	int animation_flags() const {
		return _state.animation_flags();
	}

	//! Устанавливает флаг анимации.
	void set_animation_flag(int fl) {
		_state.set_animation_flag(fl);
	}
	//! Скидывает флаг анимации.
	void drop_animation_flag(int fl) {
		_state.drop_animation_flag(fl);
	}
	//! Возвращает true, если для анимации установлен флаг fl.
	bool check_animation_flag(int fl) const {
		return _state.check_animation_flag(fl);
	}

	//! Возвращает true, если к фону привязана анимация.
	bool has_animation() const {
		return _state.has_animation();
	}

protected:

	//! Запись данных в скрипт.
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag *p);

private:

	//! Состояние, в котором хранятся все необходимые настройки.
	qdInterfaceElementState _state;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_BACKGROUND_H
