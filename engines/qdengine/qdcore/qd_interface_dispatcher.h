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


#ifndef QDENGINE_QDCORE_QD_INTERFACE_DISPATCHER_H
#define QDENGINE_QDCORE_QD_INTERFACE_DISPATCHER_H

#include "common/keyboard.h"

#include "qdengine/qdcore/qd_resource_container.h"
#include "qdengine/qdcore/qd_interface_object_base.h"
#include "qdengine/qdcore/qd_object_list_container.h"


namespace QDEngine {

class qdInterfaceScreen;
class qdInterfaceElementState;

class qdInterfaceDispatcher : public qdInterfaceObjectBase {
public:
	qdInterfaceDispatcher();
	~qdInterfaceDispatcher();

	/// для чего вызван модальный экран
	enum ModalScreenMode {
		MODAL_SCREEN_SAVE_OVERWRITE,
		MODAL_SCREEN_SAVE_NAME_EDIT,
		MODAL_SCREEN_OTHER
	};

	//! Отрисовка интерфейса.
	bool redraw(int dx = 0, int dy = 0) const;
	bool pre_redraw();
	bool post_redraw();

	//! Обсчет логики, параметр - время в секундах.
	bool quant(float dt);

	//! Добавление экрана.
	bool add_screen(qdInterfaceScreen *scr);
	//! Изменение имени экрана.
	bool rename_screen(qdInterfaceScreen *scr, const char *name);
	//! Удаление экрана из списка.
	bool remove_screen(qdInterfaceScreen *scr);
	//! Поиск экрана по имени.
	qdInterfaceScreen *get_screen(const char *screen_name);
	//! Возвращает true, если экран есть в списке.
	bool is_screen_in_list(const qdInterfaceScreen *scr);

	typedef Std::list<qdInterfaceScreen *> screen_list_t;
	//! Возвращает список экранов.
	const screen_list_t &screen_list() const {
		return _screens.get_list();
	}

	//! Установка активного экрана.
	/**
	Имя можно передавать нулевое, для сброса активного экрана.
	*/
	bool select_screen(const char *screen_name, bool lock_resources = false);
	/// Установка фонового экрана.
	bool select_background_screen(qdInterfaceScreen *p);

	//! Включает внутриигровой интерфейс.
	bool select_ingame_screen(bool inventory_state = false);

	//! Включает кнопки, связанные с персонажем p, выключает кнопки остальных персонажей.
	void update_personage_buttons();

	qdInterfaceScreen *selected_screen() const {
		return _cur_screen;
	}

	//! Возвращает true, если экран p в данный момент активен.
	bool is_screen_active(const qdInterfaceScreen *p) const {
		return (_cur_screen == p);
	}

	//! Добавляет ресурс file_name с владельцем owner.
	qdResource *add_resource(const Common::Path file_name, const qdInterfaceElementState *owner);
	//! Удаляет ресурс file_name с владельцем owner.
	bool remove_resource(const Common::Path file_name, const qdInterfaceElementState *owner);
	//! Возвращает указатель на ресурс file_name.
	qdResource *get_resource(const Common::Path file_name) const {
		return _resources.get_resource(file_name);
	}

	//! Обработчик событий мыши.
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(Common::KeyCode vkey);
	bool char_input_handler(int vkey);

	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);

	//! Установка имени экрана с главным меню игры.
	void set_main_menu_screen(const char *name);
	//! Возвращает имя экрана с главным меню игры.
	const char *main_menu_screen_name() const {
		return _main_menu_screen_name.c_str();
	}
	//! Возвращает true, если установлено имя экрана с главным меню игры.
	bool has_main_menu() const {
		return !_main_menu_screen_name.empty();
	}

	//! Установка имени внутриигрового экрана.
	void set_ingame_screen(const char *name, bool inventory_state = false);
	//! Возвращает имя внутриигрового экрана.
	const char *ingame_screen_name(bool inventory_state = false) const {
		return _ingame_screen_names[inventory_state].c_str();
	}
	//! Возвращает true, если установлено имя внутриигрового экрана.
	bool has_ingame_screen(bool inventory_state = false) const {
		return !_ingame_screen_names[inventory_state].empty();
	}

	//! Имя модального экрана подсказки: перезаписывать сэйв или нет.
	const char *save_prompt_screen_name() const {
		return _save_prompt_screen_name.c_str();
	}
	void set_save_prompt_screen_name(const char *str) {
		if (str) _save_prompt_screen_name = str;
		else _save_prompt_screen_name = "";
	}
	bool has_save_prompt_screen() const {
		return !_save_prompt_screen_name.empty();
	}

	const char *save_title_screen_name() const {
		return _save_title_screen_name.c_str();
	}
	void set_save_title_screen_name(const char *name) {
		_save_title_screen_name = name;
	}
	bool has_save_title_screen() const {
		return !_save_title_screen_name.empty();
	}

	//! Нужно ли выводить скриншот к сохраненке
	bool need_save_screenshot() const {
		return _need_save_screenshot;
	}
	void toggle_save_screenshot(bool state) {
		_need_save_screenshot = state;
	}
	//! Нужно ли выводить дату и время при отображении сэйва
	bool need_show_save_time() const {
		return _need_show_save_time;
	}
	void toggle_show_save_time(bool state) {
		_need_show_save_time = state;
	}
	bool need_show_save_title() const {
		return _need_show_save_title;
	}
	void toggle_show_save_title(bool state) {
		_need_show_save_title = state;
	}
	//! Тип шрифт, которым выводится текст сэйва (в частности дата и время)
	int save_font_type() const {
		return _save_font_type;
	}
	void set_save_font_type(int type) {
		_save_font_type = type;
	}
	//! Цвет, которым выводится текст сэйва (в частности дата и время)
	int save_font_color() const {
		return _save_font_color;
	}
	void set_save_font_color(int clr) {
		_save_font_color = clr;
	}

	//! Возвращает true, если интерфейс отрисовывается поверх сцены.
	bool need_scene_redraw() const {
		return _need_scene_redraw;
	}
	//! Устанавливает, надо ли если интерфейсу отрисовываться поверх сцены.
	void set_scene_redraw(bool state) {
		_need_scene_redraw = state;
	}

	static void set_dispatcher(qdInterfaceDispatcher *p) {
		_dispatcher = p;
	}
	static qdInterfaceDispatcher *get_dispatcher() {
		return _dispatcher;
	}

	void activate() {
		_is_active = true;
	}
	void deactivate() {
		_is_active = false;
	}
	bool is_active() const {
		return _is_active;
	}

	bool is_mouse_hover() const {
		return _is_mouse_hover;
	}
	void toggle_mouse_hover() {
		_is_mouse_hover = true;
	}
	bool is_autohide_enabled() const {
		return !_autohide_disable;
	}
	void disable_autohide() {
		_autohide_disable = true;
	}

	ModalScreenMode modalScreenMode() const {
		return _modalScreenMode;
	}
	void setModalScreenMode(ModalScreenMode mode) {
		_modalScreenMode = mode;
	}

	bool set_save_title(const char *title);
	const char *get_save_title() const;

	/**
	Обработка события.
	sender - тот, кто послал событие на выполнение. NULL - sender не известен.
	*/
	bool handle_event(int event_code, const char *event_data, qdInterfaceObjectBase *sender = NULL);

	void toggle_end_game_mode(bool state) {
		_end_game_mode = state;
	}

	//! Устанавливает следующий экран.
	void set_next_screen(const char *screen_name) {
		_next_screen = screen_name;
	}

	static int option_value(int option_id, const char *option_data = NULL);
	static bool set_option_value(int option_id, int value, const char *oprtion_data = NULL);

#ifdef __QD_DEBUG_ENABLE__
	bool get_resources_info(qdResourceInfoContainer &infos) const;
#endif

private:

	//! Активный интерфейсный экран.
	qdInterfaceScreen *_cur_screen;

	/// Фоновый экран, рисуется под активным
	qdInterfaceScreen *_background_screen;
	/// true если ресурсы фонового экрана не надо выгружать
	bool _background_screen_lock;

	ModalScreenMode _modalScreenMode;

	//! Имя экрана с главным меню игры
	Common::String _main_menu_screen_name;

	//! Имена экраноы с внутриигровым интерфейсом.
	/**
	Первый экран показывается когда инвентори спрятано, вотрой - когда оно активно.
	*/
	Common::String _ingame_screen_names[2];

	/// Экран ввода имени сэйва
	Common::String _save_title_screen_name;
	Common::String _save_title;

	//! Экран, вызывающийся при подтверждении перезаписи файла сэйва
	Common::String _save_prompt_screen_name;

	//! Нужно ли выводить скриншот к сохраненке
	bool _need_save_screenshot;
	//! Нужно ли выводить дату и время при отображении сэйва
	bool _need_show_save_time;
	//! Нужно ли выводить имя при отображении сэйва
	bool _need_show_save_title;
	//! Тип шрифта, которым выводится текст сэйва (в частности дата и время)
	int _save_font_type;
	//! Цвет, которым выводится текст сэйва (в частности дата и время)
	int _save_font_color;

	//! Интерфейсные экраны.
	qdObjectListContainer<qdInterfaceScreen> _screens;

	typedef qdResourceContainer<qdInterfaceElementState> resource_container_t;
	//! Интерфейсные ресурсы.
	resource_container_t _resources;

	//! Равно true, если интерфейс активен.
	bool _is_active;

	//! Равно true, если курсор мыши попадает в интерфейс.
	bool _is_mouse_hover;
	/// true если надо показывать
	bool _autohide_disable;

	//! Равно true, если требуется полная отрисовка интерфейса, после отрисовки скидывается.
	bool _need_full_redraw;

	//! Равно true, если отрисовка сцены под интерфейсом.
	bool _need_scene_redraw;

	//! Режим окончания игры - после любого клика возврат с текущего экрана в главное меню.
	//! После загрузки меню скидывается в false.
	bool _end_game_mode;

	//! Имя экрана, на который надо перейти.
	const char *_next_screen;

	//! Текущий диспетчер интерфейса.
	static qdInterfaceDispatcher *_dispatcher;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_DISPATCHER_H
