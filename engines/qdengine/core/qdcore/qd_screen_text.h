#ifndef __QD_SCREEN_TEXT_H__
#define __QD_SCREEN_TEXT_H__

#include "xml_fwd.h"

#include "gr_screen_region.h"
#include "gr_font.h"

class qdGameObjectState;

const int QD_FONT_TYPE_NONE = -1; //! Обозначает отсутствие шрифта

//! Формат экранного текста
class qdScreenTextFormat
{
public:
	//! Расположение относительно других текстов.
	enum arrangement_t {
		//! Выравнивание по горизонтали.
		ARRANGE_HORIZONTAL,
		//! Выравнивание по вертикали.
		ARRANGE_VERTICAL
	};

	/// Выравнивание текста.
	enum alignment_t {
		/// по левому краю
		ALIGN_LEFT,
		/// по центру
		ALIGN_CENTER,
		/// по правому краю
		ALIGN_RIGHT
	};

	qdScreenTextFormat();
	~qdScreenTextFormat(){ };

	bool operator == (const qdScreenTextFormat& fmt) const 
	{ 
		return (color_ == fmt.color_ && 
			    arrangement_ == fmt.arrangement_ && 
				alignment_ == fmt.alignment_ &&
				hover_color_ == fmt.hover_color_ &&
				font_type_ == fmt.font_type() &&
				global_depend_ == fmt.is_global_depend());
	}

	bool operator != (const qdScreenTextFormat& fmt) const { return !(*this == fmt); }

	void set_arrangement(arrangement_t al){ arrangement_ = al; }
	arrangement_t arrangement() const { return arrangement_; }

	void set_alignment(alignment_t al){ alignment_ = al; }
	alignment_t alignment() const { return alignment_; }

	void set_color(int color){ color_ = color; }
	int color() const { return color_; }

	void set_hover_color(int color){ hover_color_ = color; }
	int hover_color() const { return hover_color_; }

	//! Возвращает формат текста по умолчанию.
	static const qdScreenTextFormat& default_format(){ return default_format_; }

	/// Глобальные настройки формата текста.
	static const qdScreenTextFormat& global_text_format() { return global_text_format_; }
	static void set_global_text_format(const qdScreenTextFormat& frmt) { global_text_format_ = frmt; }

	/// Глобальные настройки формата текста для тем диалогов.
	static const qdScreenTextFormat& global_topic_format() { return global_topic_format_; }
	static void set_global_topic_format(const qdScreenTextFormat& frmt) { global_topic_format_ = frmt; }

	void set_font_type(int tp) { font_type_ = tp; }
	int font_type() const { return font_type_; }

	bool is_global_depend() const { return global_depend_; }
	void toggle_global_depend(bool flag = true) { global_depend_ = flag; }

	bool load_script(const xml::tag* p);
	bool save_script(class XStream& fh,int indent = 0) const;

private:

	//! Расположение относительно других текстов.
	arrangement_t arrangement_;
	/// Выравнивание текста.
	alignment_t alignment_;
	//! Цвет текста, RGB().
	int color_;
	//! Цвет текста при наведении на него мыши, RGB().
	int hover_color_;

	//! Формат текста по умолчанию.
	static qdScreenTextFormat default_format_;

	/// Глобальный формат.
	static qdScreenTextFormat global_text_format_;
	/// Глобальный формат тем диалогов.
	static qdScreenTextFormat global_topic_format_;

	//! Тип шрифта
	int font_type_;

	/** Флаг, означающий, что данные должны браться из global_text_format_ или
	    global_topic_format_ вместо текущей переменной. */
	bool global_depend_;
};

//! Экранный текст.
class qdScreenText
{
public:
	qdScreenText(const char* p,const Vect2i& pos = Vect2i(0,0),qdGameObjectState* owner = NULL);
	qdScreenText(const char* p,const qdScreenTextFormat& fmt,const Vect2i& pos = Vect2i(0,0),qdGameObjectState* owner = NULL);
	~qdScreenText();

	//! Экранные координаты центра текста.
	Vect2i screen_pos(){ return pos_; }
	//! Устанавливает экранные координаты центра текста.
	void set_screen_pos(const Vect2i& pos){ pos_ = pos; }

	const char* data() const { return data_.c_str(); }
	//! Устанавливает текст.
	/**
	Текст копируется во внутренние данные.
	*/
	void set_data(const char* p);

	//! Горизонтальный размер текста в пикселах.
	int size_x() const { return size_.x; }
	//! Вертикальный размер текста в пикселах.
	int size_y() const { return size_.y; }

	qdScreenTextFormat::arrangement_t arrangement() const { return text_format_.arrangement(); }
	qdScreenTextFormat::alignment_t alignment() const { return text_format_.alignment(); }

	qdScreenTextFormat text_format() const { return text_format_; }
	void set_text_format(const qdScreenTextFormat& fmt){ text_format_ = fmt; }

	grScreenRegion screen_region() const { return grScreenRegion(pos_.x,pos_.y,size_.x,size_.y); }

	//! Отрисовка текста.
	void redraw(const Vect2i& owner_pos) const;

	unsigned color() const { return text_format_.color(); }
	void set_color(unsigned col){ text_format_.set_color(col); }

	//! Возвращает указатель на владельца текста.
	qdGameObjectState* owner() const { return owner_; }
	//! Устанавливает владельца текста.
	void set_owner(qdGameObjectState* p){ owner_ = p; }
	bool is_owned_by(const qdNamedObject* p) const;

	//! Проверка попадания точки в текст, параметры в экранных координатах.
	bool hit(int x,int y) const {
		if(x >= pos_.x && x < pos_.x + size_.x){
			if(y >= pos_.y && y < pos_.y + size_.y)
				return true;
		}
		return false;
	}

	void set_hover_mode(bool state){ hover_mode_ = state; }

	// форматирует текст так, чтобы его ширина не превышала max_width (размер в пикселах)
	bool format_text(int max_width);

private:

	//! Экранные координаты текста.
	Vect2i pos_;
	//! Размеры текста в пикселах.
	Vect2i size_;

	//! Текст (данные).
	std::string data_;
	
	//! true, если над текстом курсор мыши 
	bool hover_mode_;

	//! Cпособ выравнивания текста.
	qdScreenTextFormat text_format_;

	//! Владелец текста.
	mutable qdGameObjectState* owner_;
};

#endif /* __QD_SCREEN_TEXT_H__ */
