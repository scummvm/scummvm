#ifndef __QD_INTERFACE_TEXT_WINDOW_H__
#define __QD_INTERFACE_TEXT_WINDOW_H__

#include "UI_TextParser.h"

#include "qd_interface_element.h"
#include "qd_interface_element_state.h"

class qdScreenTextSet;
class qdInterfaceSlider;

//! Интерфейсный элемент - окно для текстов.
class qdInterfaceTextWindow : public qdInterfaceElement
{
public:
	qdInterfaceTextWindow();
	qdInterfaceTextWindow(const qdInterfaceTextWindow& wnd);
	~qdInterfaceTextWindow();

	enum WindowType {
		WINDOW_DIALOGS,
		WINDOW_EDIT,
		WINDOW_TEXT
	};

	enum TextVAlign
	{
		VALIGN_TOP,
		VALIGN_CENTER,
		VALIGN_BOTTOM
	};

	enum InfoType {
		INFO_NONE,
		INFO_PLAYER_NAME,
		INFO_PLAYER_SCORE
	};

	qdInterfaceTextWindow& operator = (const qdInterfaceTextWindow& wnd);

	qdInterfaceElement::element_type get_element_type() const { return qdInterfaceElement::EL_TEXT_WINDOW; }

	//! Обработчик событий мыши.
	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);
	//! Обработчик ввода с клавиатуры.
	bool keyboard_handler(int vkey);
	bool char_input_handler(int input);

	void hover_clear();

	//! Инициализация элемента. 
	/**
	Вызывается каждый раз при заходе на экран, которому принадлежит элемент.
	*/
	bool init(bool is_game_active = true);

	bool quant(float dt);

	const char* border_background_file() const { return border_background_.animation_file(); }
	void set_border_background_file(const char* file_name){ border_background_.set_animation_file(file_name); }

	unsigned background_color() const { return background_color_; }
	void set_background_color(unsigned color){ background_color_ = color; }

	int background_alpha() const { return background_alpha_; }
	void set_background_alpha(int alpha){ background_alpha_ = alpha; }

	bool has_background_color() const { return has_background_color_; }
	void toggle_background_color(bool state){ has_background_color_ = state; }

#ifndef _QUEST_EDITOR
	//! Возвращает экранные координаты центра окна.
	virtual const Vect2i r() const { return qdInterfaceElement::r() - qdInterfaceElement::screen_offset(); }
#endif

	//! Отрисовка окна.
	bool redraw() const;
	bool need_redraw() const;

	grScreenRegion screen_region() const;

	//! Возвращает размер окна вместе с рамкой по горизонтали в пикселах.
	int size_x() const;
	//! Возвращает размер окна вместе с рамкой по вертикали в пикселах.
	int size_y() const;

	const Vect2i& text_size() const { return text_size_; }
	void set_text_size(const Vect2i& sz){ text_size_ = sz; }

	const char* slider_name() const { return slider_name_.c_str(); }
	void set_slider_name(const char* name){ slider_name_ = name; }

	//! Возвращает true, если точка с экранными координатами (x,у) попадает в элемент.
	bool hit_test(int x,int y) const;

	WindowType windowType() const { return windowType_; }
	void setWindowType(WindowType type){ windowType_ = type; }

	InfoType infoType() const { return infoType_; }
	void setInfoType(InfoType type){ infoType_ = type; }

	int playerID() const { return playerID_; }
	void setPlayerID(int id){ playerID_ = id; }

	const char* input_string() const { return inputString_.c_str(); }
	void set_input_string(const char* str);
	bool edit_start();
	bool edit_done(bool cancel = false);

	TextVAlign text_valign() const { return textVAlign_; }
	void set_text_valign(TextVAlign align) { textVAlign_ = align; }

	int inputStringLimit() const { return inputStringLimit_; }
	void setInputStringLimit(int length){ inputStringLimit_ = length; }

	const qdScreenTextFormat& textFormat() const { return textFormat_; }
	void setTextFormat(const qdScreenTextFormat& format){ textFormat_ = format; }

protected:

	//! Запись данных в скрипт.
	bool save_script_body(XStream& fh,int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag* p);

private:

	WindowType windowType_;

	InfoType infoType_;
	int playerID_;

	//! Размер текстовой области окна.
	Vect2i text_size_;

	TextVAlign textVAlign_;

	//! Скорость выезжания текста, если нулевая появляется мгновенно.
	float scrolling_speed_;

	float scrolling_position_;
	int text_set_position_;

	qdInterfaceElementState border_background_;
	unsigned background_color_;
	bool has_background_color_;
	int background_alpha_;

	int text_set_id_;
	qdScreenTextSet* text_set_;

	std::string slider_name_;
	qdInterfaceSlider* slider_;

	std::string inputString_;
	std::string inputStringBackup_;
	qdScreenTextFormat textFormat_;
	int inputStringLimit_;
	bool isEditing_;
	int caretPose_;

	UI_TextParser parser_;

	static bool caretVisible_;
	static float caretTimer_;

	void update_text_position();
	void set_scrolling(int y_delta);

	void text_redraw() const;
	bool edit_input(int vkey);
};

#endif // __QD_INTERFACE_TEXT_WINDOW_H__

