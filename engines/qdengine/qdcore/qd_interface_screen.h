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


#ifndef QDENGINE_QDCORE_QD_INTERFACE_SCREEN_H
#define QDENGINE_QDCORE_QD_INTERFACE_SCREEN_H

#include "common/keyboard.h"

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/xmath.h"
#include "qdengine/qdcore/qd_interface_object_base.h"
#include "qdengine/qdcore/qd_object_list_container.h"
#include "qdengine/qdcore/qd_resource_dispatcher.h"
#include "qdengine/qdcore/qd_music_track.h"
#include "qdengine/system/graphics/gr_screen_region.h"
#include "qdengine/system/input/mouse_input.h"


namespace QDEngine {

class qdInterfaceElement;
class qdInterfaceElementState;

//! Интерфейсный экран.
class qdInterfaceScreen : public qdInterfaceObjectBase {
public:
	qdInterfaceScreen();
	~qdInterfaceScreen();

	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);

	//! Отрисовка экрана.
	bool redraw(int dx = 0, int dy = 0) const;

	bool pre_redraw(bool force_full_redraw = false);
	bool post_redraw();

	//! Обсчет логики, параметр - время в секундах.
	bool quant(float dt);

	//! Добавление элемента.
	bool add_element(qdInterfaceElement *p);
	//! Изменение имени элемента.
	bool rename_element(qdInterfaceElement *p, const char *name);
	//! Удаление элемента из списка.
	bool remove_element(qdInterfaceElement *p);
	//! Поиск элемента по имени.
	qdInterfaceElement *get_element(const char *el_name);
	//! Возвращает true, если элемент есть в списке.
	bool is_element_in_list(const qdInterfaceElement *el) const;

	typedef Std::list<qdInterfaceElement *> element_list_t;
	//! Возвращает список элементов экрана.
	const element_list_t &element_list() const {
		return _elements.get_list();
	}

	//! Обработчик событий мыши.
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(Common::KeyCode vkey);
	bool char_input_handler(int vkey);

	//! Добавляет ресурс file_name с владельцем owner.
	qdResource *add_resource(const Common::Path file_name, const qdInterfaceElementState *res_owner);
	//! Удаляет ресурс file_name с владельцем owner.
	bool remove_resource(const Common::Path file_name, const qdInterfaceElementState *res_owner);
	//! Возвращает true, если на ресурс есть ссылки.
	bool has_references(const qdResource *p) const {
		return _resources.is_registered(p);
	}

	//! Прячет элемент.
	bool hide_element(const char *element_name, bool temporary_hide = true);
	//! Прячет элемент.
	bool hide_element(qdInterfaceElement *p, bool temporary_hide = true);
	//! Показывает элемент.
	bool show_element(const char *element_name);
	//! Показывает элемент.
	bool show_element(qdInterfaceElement *p);

	//! Инициализация экрана при заходе на него.
	bool init(bool is_game_active = true);

	//! Строит сортированный список видимых элементов.
	bool build_visible_elements_list();

	//! Устанавливает, что ресурсы экрана не надо выгружать при выходе с него.
	void lock_resources() {
		_is_locked = true;
	}
	//! Устанавливает, что ресурсы экрана надо выгружать при выходе с него.
	void unlock_resources() {
		_is_locked = false;
	}
	//! Возвращает true, если ресурсы экрана не надо выгружать при выходе с него.
	bool is_locked() const {
		return _is_locked;
	}

	//! Включает кнопки, связанные с персонажем p, выключает кнопки остальных персонажей.
	void activate_personage_buttons(const qdNamedObject *p);

	void update_personage_buttons();

	float autohide_time() const {
		return _autohide_time;
	}
	void set_autohide_time(float time) {
		_autohide_time = time;
	}

	const Vect2i &autohide_offset() const {
		return _autohide_offset;
	}
	void set_autohide_offset(const Vect2i &offset) {
		_autohide_offset = offset;
	}

	//! Фоновая музыка.
	const qdMusicTrack &music_track() const {
		return _music_track;
	}
	//! Устанавливает фоновую музыку.
	void set_music_track(const qdMusicTrack &track) {
		_music_track = track;
	}
	//! Возвращает true, если экрану задана фоновая музыка.
	bool has_music_track() const {
		return _music_track.has_file_name();
	}

	void set_autohide_phase(float ph) {
		_autohide_phase = ph;
	}

	// Указатель на объект, последним вызвавший данный как модальный экран
	qdInterfaceObjectBase *modal_caller() {
		return _modal_caller;
	}
	void set_modal_caller(qdInterfaceObjectBase *caller) {
		_modal_caller = caller;
	}

private:

	//! Список интерфейсных элементов экрана.
	qdObjectListContainer<qdInterfaceElement> _elements;

	//! Ресурсы, на которые ссылается экран.
	qdResourceDispatcher<qdInterfaceElementState> _resources;

	typedef Std::vector<qdInterfaceElement *> sorted_element_list_t;
	//! Отсортированный по глубине список элементов экрана.
	sorted_element_list_t _sorted_elements;

	//! Сортирует элементы по глубине.
	bool sort_elements();

	//! true, если ресурсы экрана не надо выгружать при выходе с него.
	bool _is_locked;

	//! Время всплывания экрана в секундах.
	float _autohide_time;
	//! Смещение экрана, когда он минимизирован.
	Vect2i _autohide_offset;

	//! Текущая фаза всплывания экрана.
	float _autohide_phase;

	//! Фоновая музыка.
	qdMusicTrack _music_track;

	//! Указатель на объект, который последним вызвал данный экран в качестве модального
	qdInterfaceObjectBase *_modal_caller;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_SCREEN_H
