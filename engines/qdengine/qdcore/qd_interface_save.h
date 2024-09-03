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


#ifndef QDENGINE_QDCORE_QD_INTERFACE_SAVE_H
#define QDENGINE_QDCORE_QD_INTERFACE_SAVE_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_interface_element_state.h"


namespace QDEngine {


//! Интерфейсный элемент - сэйв.
class qdInterfaceSave : public qdInterfaceElement {
public:
	qdInterfaceSave();
	qdInterfaceSave(const qdInterfaceSave &sv);
	~qdInterfaceSave();

	qdInterfaceSave &operator = (const qdInterfaceSave &sv);

	//! Возвращает тип элемента.
	qdInterfaceElement::element_type get_element_type() const {
		return qdInterfaceElement::EL_SAVE;
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

	//! Отрисовка элемента.
	bool redraw() const;

	grScreenRegion screen_region() const;

	//! Возвращает размер элемента по горизонтали в пикселах.
	int size_x() const;
	//! Возвращает размер элемента по вертикали в пикселах.
	int size_y() const;

	//! Возвращает размер картики-превью по горизонтали в пикселах.
	int thumbnail_size_x() const {
		return _thumbnail_size_x;
	}
	//! Возвращает размер картики-превью по вертикали в пикселах.
	int thumbnail_size_y() const {
		return _thumbnail_size_y;
	}

	//! Назначает размер картики-превью по горизонтали в пикселах.
	void set_thumbnail_size_x(int sx) {
		_thumbnail_size_x = sx;
	}
	//! Назначает размер картики-превью по вертикали в пикселах.
	void set_thumbnail_size_y(int sy) {
		_thumbnail_size_y = sy;
	}

	//! Смещение текста, выводимого в сэйве, относительно левого-верхнего угла
	int text_dx() const {
		return _text_dx;
	}
	int text_dy() const {
		return _text_dy;
	}
	void set_test_dx(int val) {
		_text_dx = val;
	}
	void set_test_dy(int val) {
		_text_dy = val;
	}

	//! Обсчет логики, параметр - время в секундах.
	bool quant(float dt);

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	bool hit_test(int x, int y) const;

	//! Возвращает номер сэйва.
	int save_ID() const {
		return _save_ID;
	}
	//! Устанавливает номер сэйва.
	void set_save_ID(int id) {
		_save_ID = id;
	}

	//! Устанавливает режим работы - записывать (state == true) или загружать (state == false) сэйвы.
	static void set_save_mode(bool state) {
		_save_mode = state;
	}

	//! Устанавливает имя файла для анимации.
	/**
	Если надо убрать анимацию - передать NULL в качестве имени файла.
	*/
	void set_frame_animation_file(const Common::Path name, qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) {
		_frame.set_animation_file(name, mode);
	}
	//! Возвращает имя файла для анимации.
	const Common::Path frame_animation_file(qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const {
		return _frame.animation_file(mode);
	}
	//! Возвращает флаги анимации.
	int frame_animation_flags(qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const {
		return _frame.animation_flags(mode);
	}

	//! Устанавливает флаг анимации.
	void set_frame_animation_flag(int fl, qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) {
		_frame.set_animation_flag(fl, mode);
	}
	//! Скидывает флаг анимации.
	void drop_frame_animation_flag(int fl, qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) {
		_frame.drop_animation_flag(fl, mode);
	}
	//! Возвращает true, если для анимации установлен флаг fl.
	bool check_frame_animation_flag(int fl, qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const {
		return _frame.check_animation_flag(fl, mode);
	}

	//! Возвращает true, если к сейву привязана анимация рамки.
	bool frame_has_animation(qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const {
		return _frame.has_animation(mode);
	}

	void set_frame_sound_file(const char *name, qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) {
		_frame.set_sound_file(name, mode);
	}
	const Common::Path frame_sound_file(qdInterfaceElementState::state_mode_t mode = qdInterfaceElementState::MOUSE_HOVER_MODE) const {
		return _frame.sound_file(mode);
	}

	bool isAutosaveSlot() const {
		return _isAutosaveSlot;
	}
	void setAutosaveSlot(bool state) {
		_isAutosaveSlot = state;
	}

	void set_title(const char *title) {
		_save_title = title;
	}
	const char *title() const {
		return _save_title.c_str();
	}

	//! Осуществить сохранение текущего состояния игры в ячейку сэйва.
	bool perform_save();

protected:

	//! Запись данных в скрипт.
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag *p);

private:

	//! Горизонтальный размер картинки-превью сэйва.
	int _thumbnail_size_x;
	//! Вертикальный размер картинки-превью сэйва.
	int _thumbnail_size_y;

	//! Смещение текста, выводимого в сэйве, относительно левого-верхнего угла
	int _text_dx;
	int _text_dy;

	//! Номер сэйва, имя файла имеет вид saveNNN.dat, где NNN - save_ID_.
	int _save_ID;

	/// если true, то игрок сам не может записать игру в этот слот
	bool _isAutosaveSlot;

	//! Превью сэйва.
	qdInterfaceElementState _thumbnail;

	Common::String _save_title;

	//! Опциональная рамка вокруг картинки-превью сэйва.
	qdInterfaceElementState _frame;

	//! Режим работы сэйвов - чтение/запись (== false/true).
	static bool _save_mode;

	//! Текущий номер сэйва.
	static int _current_save_ID;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_SAVE_H
