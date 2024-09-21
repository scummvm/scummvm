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

#ifndef QDENGINE_QDCORE_QD_INTERFACE_TEXT_WINDOW_H
#define QDENGINE_QDCORE_QD_INTERFACE_TEXT_WINDOW_H

#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_interface_element_state.h"
#include "qdengine/system/graphics/UI_TextParser.h"


namespace QDEngine {

class qdScreenTextSet;
class qdInterfaceSlider;

//! Интерфейсный элемент - окно для текстов.
class qdInterfaceTextWindow : public qdInterfaceElement {
public:
	qdInterfaceTextWindow();
	qdInterfaceTextWindow(const qdInterfaceTextWindow &wnd);
	~qdInterfaceTextWindow();

	enum WindowType {
		WINDOW_DIALOGS,
		WINDOW_EDIT,
		WINDOW_TEXT
	};

	enum TextVAlign {
		VALIGN_TOP,
		VALIGN_CENTER,
		VALIGN_BOTTOM
	};

	enum InfoType {
		INFO_NONE,
		INFO_PLAYER_NAME,
		INFO_PLAYER_SCORE
	};

	qdInterfaceTextWindow &operator = (const qdInterfaceTextWindow &wnd);

	qdInterfaceElement::element_type get_element_type() const {
		return qdInterfaceElement::EL_TEXT_WINDOW;
	}

	//! Обработчик событий мыши.
	bool mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(Common::KeyCode vkey);
	bool char_input_handler(int input);

	void hover_clear();

	//! Инициализация элемента.
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	bool quant(float dt);

	const Common::Path border_background_file() const {
		return _border_background.animation_file();
	}
	void set_border_background_file(const Common::Path file_name) {
		_border_background.set_animation_file(file_name);
	}

	uint32 background_color() const {
		return _background_color;
	}
	void set_background_color(uint32 color) {
		_background_color = color;
	}

	int background_alpha() const {
		return _background_alpha;
	}
	void set_background_alpha(int alpha) {
		_background_alpha = alpha;
	}

	bool has_background_color() const {
		return _has_background_color;
	}
	void toggle_background_color(bool state) {
		_has_background_color = state;
	}

	//! Возвращает экранные координаты центра окна.
	virtual const Vect2i r() const {
		return qdInterfaceElement::r() - g_engine->screen_offset();
	}

	//! Отрисовка окна.
	bool redraw() const;
	bool need_redraw() const;

	grScreenRegion screen_region() const;

	//! Возвращает размер окна вместе с рамкой по горизонтали в пикселах.
	int size_x() const;
	//! Возвращает размер окна вместе с рамкой по вертикали в пикселах.
	int size_y() const;

	const Vect2i &text_size() const {
		return _text_size;
	}
	void set_text_size(const Vect2i &sz) {
		_text_size = sz;
	}

	const char *slider_name() const {
		return _slider_name.c_str();
	}
	void set_slider_name(const char *name) {
		_slider_name = name;
	}

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	bool hit_test(int x, int y) const;

	WindowType windowType() const {
		return _windowType;
	}
	void setWindowType(WindowType type) {
		_windowType = type;
	}

	InfoType infoType() const {
		return _infoType;
	}
	void setInfoType(InfoType type) {
		_infoType = type;
	}

	int playerID() const {
		return _playerID;
	}
	void setPlayerID(int id) {
		_playerID = id;
	}

	const char *input_string() const {
		return _inputString.c_str();
	}
	void set_input_string(const char *str);
	bool edit_start();
	bool edit_done(bool cancel = false);

	TextVAlign text_valign() const {
		return _textVAlign;
	}
	void set_text_valign(TextVAlign align) {
		_textVAlign = align;
	}

	int inputStringLimit() const {
		return _inputStringLimit;
	}
	void setInputStringLimit(int length) {
		_inputStringLimit = length;
	}

	const qdScreenTextFormat &textFormat() const {
		return _textFormat;
	}
	void setTextFormat(const qdScreenTextFormat &format) {
		_textFormat = format;
	}

protected:

	//! Запись данных в скрипт.
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag *p);

private:

	WindowType _windowType;

	InfoType _infoType;
	int _playerID;

	//! Размер текстовой области окна.
	Vect2i _text_size;

	TextVAlign _textVAlign;

	//! Скорость выезжания текста, если нулевая появляется мгновенно.
	float _scrolling_speed;

	float _scrolling_position;
	int _text_set_position;

	qdInterfaceElementState _border_background;
	uint32 _background_color;
	bool _has_background_color;
	int _background_alpha;

	int _text_set_id;
	qdScreenTextSet *_text_set;

	Common::String _slider_name;
	qdInterfaceSlider *_slider;

	Common::String _inputString;
	Common::String _inputStringBackup;
	qdScreenTextFormat _textFormat;
	int _inputStringLimit;
	bool _isEditing;
	int _caretPose;

	UI_TextParser _parser;

	static bool _caretVisible;
	static float _caretTimer;

	void update_text_position();
	void set_scrolling(int y_delta);

	void text_redraw() const;
	bool edit_input(Common::KeyCode vkey);
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_TEXT_WINDOW_H
