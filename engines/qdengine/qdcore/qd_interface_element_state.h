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

#ifndef QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_STATE_H
#define QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_STATE_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_animation_info.h"
#include "qdengine/qdcore/qd_interface_object_base.h"
#include "qdengine/qdcore/qd_interface_element_state_mode.h"


namespace QDEngine {


//! Интерфейсное событие.
class qdInterfaceEvent {
public:
	//! События, производимые элементами интерфейса.
	enum event_t {
		EVENT_NONE = 0,
		//! выйти из игры
		EVENT_EXIT,             // 1
		//! перейти на сцену, параметр - имя сцены
		EVENT_LOAD_SCENE,           // 2
		//! записать игру [не используется]
		EVENT_SAVE_GAME,            // 3
		//! новая игра
		EVENT_NEW_GAME,             // 4
		//! перейти на другой интерфейсный экран (парметер имя экрана)
		EVENT_CHANGE_INTERFACE_SCREEN,      // 5
		//! сменить активного персонажа
		EVENT_CHANGE_PERSONAGE,         // 6
		//! временно спрятать элемент активного экрана (до перехода на другой экран)
		// (парметер имя элемента)
		EVENT_TMP_HIDE_ELEMENT,         // 7
		//! спрятать элемент активного экрана (насовсем)
		// (парметер имя элемента)
		EVENT_HIDE_ELEMENT,         // 8
		//! показать элемент активного экрана
		// (парметер имя элемента)
		EVENT_SHOW_ELEMENT,         // 9
		//! продолжить игру
		EVENT_RESUME_GAME,          // 10
		//! включить режим записи игры
		/**
		при заходе на экран с сэйвами будет происходить
		запись игры при клике по сэйву
		*/
		EVENT_SET_SAVE_MODE,            // 11
		//! включить режим загрузки игры
		/**
		при заходе на экран с сэйвами будет происходить
		загрузка сохраненной игры при клике по сэйву
		*/
		EVENT_SET_LOAD_MODE,            // 12
		//! сделать персонажа активным, имя персонажа - в данных события
		EVENT_ACTIVATE_PERSONAGE,       // 13
		//! включить предыдущее состояние кнопки (парметер имя кнопки)
		EVENT_PREV_ELEMENT_STATE,       // 14
		//! включить следующее состояние кнопки (парметер имя кнопки)
		EVENT_NEXT_ELEMENT_STATE,       // 15
		//! перейти в main menu
		EVENT_MAIN_MENU,            // 16
		//! играть видеоролик (параметр - имя видеоролика из списка в qdGameDispatcher)
		EVENT_PLAY_VIDEO,           // 17
		//! включить определенное состояние кнопки, параметр - имя кнопки
		EVENT_BUTTON_STATE,         // 18
		//! скинуть с мыши объект в инвентори
		EVENT_CLEAR_MOUSE,          // 19
		//! скроллинг
		EVENT_SCROLL_LEFT,          // 20
		EVENT_SCROLL_RIGHT,         // 21
		EVENT_SCROLL_UP,            // 22
		EVENT_SCROLL_DOWN,          // 23
		//! показать экран интерфейса, как модальный
		EVENT_SHOW_INTERFACE_SCREEN_AS_MODAL,   // 24
		//! пользователь завершил работу с модальным окном и ответил Ок
		EVENT_MODAL_OK,             // 25
		//! пользователь завершил работу с модальным окном и ответил Отмена
		EVENT_MODAL_CANCEL,         // 26
		/// Имя игрока в таблице рекордов, параметр - номер места
		EVENT_HALL_OF_FAME_PLAYER,  // 27
		/// Очки игрока в таблице рекордов, параметр - номер места
		EVENT_HALL_OF_FAME_SCORE,   // 28
		/// Текущие очки игрока на экране таблицы рекордов
		EVENT_HALL_OF_FAME_CUR_SCORE    // 29
	};

	/// Способы запуска события
	enum activation_t {
		/// левый клик по кнопке
		EVENT_ACTIVATION_CLICK = 0,
		/// наведение мышиного курсора
		EVENT_ACTIVATION_HOVER
	};

	qdInterfaceEvent(event_t id, const char *data, bool anm_flag, activation_t activation = EVENT_ACTIVATION_CLICK) : _event(id), _event_data(data), _is_before_animation(anm_flag), _activation(activation) { }
	~qdInterfaceEvent() {}

	bool operator == (event_t id) const {
		return (_event == id);
	}

	//! Код события.
	event_t event() const {
		return _event;
	}
	//! Возвращает true, если у события есть данные.
	bool has_data() const {
		return !_event_data.empty();
	}
	//! Возвращает данные для события.
	const char *event_data() const {
		return _event_data.c_str();
	}
	//! Возвращает true, если событие должно происходить до включения анимации.
	bool is_before_animation() const {
		return _is_before_animation;
	}
	activation_t activation() const {
		return _activation;
	}

private:

	//! Код события.
	event_t _event;
	//! Данные для события.
	Common::String _event_data;
	//! Равно true, если событие должно происходить до включения анимации.
	bool _is_before_animation;
	//! Как событие активируется
	activation_t _activation;
};

//! Состояние элемента интерфейса.
class qdInterfaceElementState : public qdInterfaceObjectBase {
public:
	//! Идентификаторы для режимов работы состояния.
	enum state_mode_t {
		//! режим по умолчанию
		DEFAULT_MODE = 0,
		//! режим, который включается когда мышиный курсор находится над элементом-владельцем
		MOUSE_HOVER_MODE,
		//! режим, который включается при клике по элементу-владельцу
		EVENT_MODE
	};

	enum {
		//! количество режимов работы состояния
		NUM_MODES = 3
	};

	qdInterfaceElementState();
	qdInterfaceElementState(const qdInterfaceElementState &st);
	~qdInterfaceElementState();

	qdInterfaceElementState &operator = (const qdInterfaceElementState &st);

	//! Устанавливает имя файла звукового эффекта, привязанного к состоянию.
	/**
	Если надо убрать звук - передать NULL в качестве имени файла.
	*/
	void set_sound_file(const Common::Path str, state_mode_t snd_id = DEFAULT_MODE);
	//! Возвращает имя файла звукового эффекта, привязанного к состоянию.
	const Common::Path sound_file(state_mode_t snd_id = DEFAULT_MODE) const {
		return _modes[snd_id].sound_file();
	}
	//! Возвращает указатель на звуковой эффект, привязанный к состоянию.
	const qdSound *sound(state_mode_t snd_id = DEFAULT_MODE) const {
		return _modes[snd_id].sound();
	}
	//! Возвращает true, если к состоянию привязан звук.
	bool has_sound(state_mode_t snd_id = DEFAULT_MODE) const {
		return _modes[snd_id].has_sound();
	}

	//! Устанавливает имя файла для анимации.
	/**
	Если надо убрать анимацию - передать NULL в качестве имени файла.
	*/
	void set_animation_file(const Common::Path name, state_mode_t anm_id = DEFAULT_MODE);
	//! Возвращает имя файла для анимации.
	const Common::Path animation_file(state_mode_t anm_id = DEFAULT_MODE) const {
		return _modes[anm_id].animation_file();
	}
	//! Возвращает флаги анимации.
	int animation_flags(state_mode_t anm_id = DEFAULT_MODE) const {
		return _modes[anm_id].animation_flags();
	}

	//! Устанавливает флаг анимации.
	void set_animation_flag(int fl, state_mode_t anm_id = DEFAULT_MODE) {
		_modes[anm_id].set_animation_flag(fl);
	}
	//! Скидывает флаг анимации.
	void drop_animation_flag(int fl, state_mode_t anm_id = DEFAULT_MODE) {
		_modes[anm_id].drop_animation_flag(fl);
	}
	//! Возвращает true, если для анимации установлен флаг fl.
	bool check_animation_flag(int fl, state_mode_t anm_id = DEFAULT_MODE) const {
		return _modes[anm_id].check_animation_flag(fl);
	}

	//! Возвращает указатель на анимацию, привязанную к состоянию.
	const qdAnimation *animation(state_mode_t anm_id = DEFAULT_MODE) const {
		return _modes[anm_id].animation();
	}
	//! Возвращает true, если к состоянию привязана анимация.
	bool has_animation(state_mode_t anm_id = DEFAULT_MODE) const {
		return _modes[anm_id].has_animation();
	}

	//! Регистрирует ресурсы, связанные с состоянием (анимацию и звук).
	bool register_resources();
	//! Высвобождает ресурсы, связанные с состоянием (анимацию и звук).
	bool unregister_resources();

	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);

	//! Обсчет логики, параметр - время в секундах.
	/**
	Вызывается владельцем для активного состояния.
	*/
	bool quant(float dt);

	//! Устанавливает режим работы состояния.
	void set_state_mode(state_mode_t mode) {
		_state_mode = mode;
	}
	//! Возвращает режим работы состояния.
	state_mode_t state_mode() const {
		return _state_mode;
	}
	//! Возвращает true, если у состояния может быть включен режим работы mode.
	bool has_state_mode(state_mode_t mode) const;

	//! Возвращает true, если у состояния задан контур для режима mode.
	bool has_contour(state_mode_t mode) const {
		return _modes[mode].has_contour();
	}
	//! Записывает контур режима mode в cnt.
	bool get_contour(state_mode_t mode, qdContour &cnt) const;
	//! Устанавливает контур для режима mode.
	bool set_contour(state_mode_t mode, const qdContour &cnt);

	//! Возвращает true, если точка с экранными координатами (x,у) попадает внутрь контура режима mode.
	bool hit_test(int x, int y, state_mode_t mode) const {
		return _modes[mode].hit_test(x, y);
	}

	//! Обработчик событий мыши.
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(Common::KeyCode vkey);

	typedef Std::vector<qdInterfaceEvent> events_container_t;
	//! Список событий.
	const events_container_t &events() const {
		return _events;
	}
	//! Добавление события.
	void add_event(const qdInterfaceEvent &ev) {
		_events.push_back(ev);
	}
	//! Уделение события
	void erase_event(int iNum) {
		_events.erase(_events.begin() + iNum);
	}
	//! Удаление всех событий
	void erase_all_events() {
		_events.clear();
	}

	/**
	Возвращает true, если хотя бы для одного из событий состояния необходимо
	наличие загруженной сцены (т.е. оно происходит только во время игры).
	*/
	bool need_active_game() const;

	const qdInterfaceEvent *find_event(qdInterfaceEvent::event_t type) const;
	bool has_event(qdInterfaceEvent::event_t type, const char *ev_data = NULL) const;

private:

	//! События, происходящие при активации состояния.
	events_container_t _events;

	//! События состояния.
	qdInterfaceElementStateMode _modes[NUM_MODES];

	//! Текущий режим работы состояния.
	state_mode_t _state_mode;
	//! Режим работы состояния в прошлый квант времени.
	state_mode_t _prev_state_mode;

	//! Обработка событий состояния.
	/**
	Параметром выбирается, какие события выполнять - которые должны
	происхолить до анимации (параметр true) или после нее (параметр false).
	*/
	bool handle_events(qdInterfaceEvent::activation_t activation_type, bool before_animation);
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_STATE_H
