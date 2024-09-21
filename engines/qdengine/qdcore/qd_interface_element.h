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

#ifndef QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_H
#define QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_H

#include "common/keyboard.h"

#include "qdengine/system/sound/snd_sound.h"
#include "qdengine/system/input/mouse_input.h"
#include "qdengine/system/graphics/gr_screen_region.h"
#include "qdengine/qdcore/qd_animation.h"
#include "qdengine/qdcore/qd_interface_object_base.h"
#include "qdengine/qdcore/qd_sound_handle.h"


namespace QDEngine {

//class qdInterfaceScreen;
class qdInterfaceElementState;

//! Базовый класс для элементов GUI.
class qdInterfaceElement : public qdInterfaceObjectBase {
public:
	qdInterfaceElement();
	qdInterfaceElement(const qdInterfaceElement &el);
	virtual ~qdInterfaceElement() = 0;

	qdInterfaceElement &operator = (const qdInterfaceElement &el);

	//! Идентификаторы настроек игры.
	enum option_ID_t {
		OPTION_NONE = 0,
		/// звук вкл/выкл, 1/0
		OPTION_SOUND,
		/// громкость звука, [0, 255]
		OPTION_SOUND_VOLUME,
		/// музыка вкл/выкл, 1/0
		OPTION_MUSIC,
		/// громкость музыки, [0, 255]
		OPTION_MUSIC_VOLUME,
		/// определенный персонаж персонаж активен/неактивен, 1/0
		OPTION_ACTIVE_PERSONAGE
	};

	//! Типы элементов интерфейса.
	enum element_type {
		//! фон
		EL_BACKGROUND,
		//! кнопка
		EL_BUTTON,
		//! слайдер
		EL_SLIDER,
		//! сэйв
		EL_SAVE,
		//! окно с текстами
		EL_TEXT_WINDOW,
		//! счётчик
		EL_COUNTER
	};

	//! Статус состояния.
	enum state_status_t {
		//! состояние неактивно
		STATE_INACTIVE,
		//! состояние активно
		STATE_ACTIVE,
		//! работа состояния закончилась
		STATE_DONE
	};

	//! Создание элемента интерфейса заданного типа.
	static qdInterfaceElement *create_element(element_type tp);
	//! Уничтожение элемента интерфейса.
	static void destroy_element(qdInterfaceElement *p);
	//! Возвращает тип элемента.
	virtual element_type get_element_type() const = 0;

	//! Возвращает идентификатор настройки игры, связанной с элементом.
	option_ID_t option_ID() const {
		return _option_ID;
	}
	//! Устанавливает идентификатор настройки игры, связанной с элементом.
	void set_option_ID(option_ID_t id) {
		_option_ID = id;
	}
	//! Возвращает true, если с элементом связана настройка игры.
	bool linked_to_option() const {
		return (_option_ID != OPTION_NONE);
	}
	//! Возвращает значение настройки игры, связанной с элементом.
	virtual int option_value() const {
		return -1;
	}
	//! Устанавливает значение настройки игры, связанной с элементом.
	virtual bool set_option_value(int value) {
		return false;
	}

	//! Возвращает экранные координаты центра элемента.
	virtual const Vect2i r() const {
		return _r + g_engine->screen_offset();
	}
	//! Устанавливает экранные координаты центра элемента.
	void set_r(const Vect2i &v) {
		_r = v;
	}

	//! Возвращает размер элемента по горизонтали в пикселах.
	virtual int size_x() const {
		return _animation.size_x();
	}
	//! Возвращает размер элемента по вертикали в пикселах.
	virtual int size_y() const {
		return _animation.size_y();
	}

	virtual grScreenRegion screen_region() const;
	const grScreenRegion &last_screen_region() const {
		return _last_screen_region;
	}

	//! Устанавливает экранную глубину элемента.
	void set_screen_depth(int v) {
		_screen_depth = v;
	}
	//! Возвращает экранную глубину эелмента.
	int screen_depth() const {
		return _screen_depth;
	}

	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);

	//! Отрисовка элемента.
	virtual bool redraw() const;
	virtual bool need_redraw() const;
	virtual bool post_redraw();

	//! Обработчик событий мыши.
	virtual bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) = 0;
	//! Обработчик ввода с клавиатуры.
	virtual bool keyboard_handler(Common::KeyCode vkey) = 0;
	//! Обработчик ввода символов с клавиатуры.
	virtual bool char_input_handler(int vkey) {
		return false;
	}

	virtual void hover_clear() { }

	//! Инициализация элемента.
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	virtual bool init(bool is_game_active = true) = 0;

	//! Обсчет логики, параметр - время в секундах.
	virtual bool quant(float dt);

	const qdAnimation &animation() const {
		return _animation;
	}

	//! Устанавливает анимацию для элемента.
	bool set_animation(const qdAnimation *anm, int anm_flags = 0);
	//! Включает состояние элемента.
	bool set_state(const qdInterfaceElementState *p);

	//! Добавляет ресурс file_name с владельцем owner.
	qdResource *add_resource(const Common::Path file_name, const qdInterfaceElementState *res_owner);
	//! Удаляет ресурс file_name с владельцем owner.
	bool remove_resource(const Common::Path file_name, const qdInterfaceElementState *res_owner);

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	virtual bool hit_test(int x, int y) const;

	//! Возвращает статус состояния.
	state_status_t state_status(const qdInterfaceElementState *p) const;

	//! Прячет элемент.
	void hide() {
		_is_visible = false;
	}
	//! Показывает элемент.
	void show() {
		_is_visible = true;
	}
	//! Возвращает true, если элемент не спрятан.
	bool is_visible() const {
		return _is_visible;
	}

	//! Возвращает true, если элемент заблокирован.
	bool is_locked() const {
		return _is_locked;
	}
	//! Блокировка/разблокировка элемента.
	void set_lock(bool state) {
		_is_locked = state;
	}

protected:

	//! Запись данных в скрипт.
	virtual bool save_script_body(Common::WriteStream &fh, int indent = 0) const = 0;

	//! Загрузка данных из скрипта.
	virtual bool load_script_body(const xml::tag *p) = 0;

	void clear_screen_region() {
		_last_screen_region = grScreenRegion_EMPTY;
	}

private:

	//! Идентификатор настройки игры, связанной с элементом.
	option_ID_t _option_ID;
	//! Данные для настройки игры, связанной с элементом.
	Common::String _option_data;

	//! Экранные координаты центра элемента.
	Vect2i _r;

	//! Экранная глубина. Чем больше значение - тем дальше элемент.
	int _screen_depth;

	//! Анимация элемента.
	qdAnimation _animation;

	//! Хэндл для управления звуками.
	qdSoundHandle _sound_handle;

	//! Равно false, если элемент спрятан.
	bool _is_visible;

	//! Равно true, если элемент заблокирован (не реагирует на клавиатуру/мышь).
	bool _is_locked;

	grScreenRegion _last_screen_region;
	const qdAnimationFrame *_last_animation_frame;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_H
