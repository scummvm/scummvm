#ifndef __QD_SCREEN_TEXT_SET_H__
#define __QD_SCREEN_TEXT_SET_H__

#include "qd_screen_text.h"
#include "xml_fwd.h"

//! Набор экранных текстов.
class qdScreenTextSet
{
public:
	qdScreenTextSet();
	~qdScreenTextSet();

	//! Возвращает идентификатор набора.
	int ID() const { return ID_; }
	//! Устанавливает идентификатор набора.
	void set_ID(int id){ ID_ = id; }

	//! Возвращает экранные координаты центра набора.
	const Vect2i& screen_pos() const { return pos_; }
	//! Устанавливает экранные координаты центра набора.
	void set_screen_pos(const Vect2i& pos){ pos_ = pos; }

	//! Возвращает размеры набора на экране.
	const Vect2i& screen_size() const { return size_; }
	//! Устанавливает размеры набора на экране.
	void set_screen_size(const Vect2i& sz){ size_ = sz; }

	//! Добавление текста в набор.
	qdScreenText* add_text(const qdScreenText& txt);
	//! Очистка всех текстов набора.
	void clear_texts(){ texts_.clear(); arrange_texts(); }
	//! Очистка всех текстов с владельцем owner.
	void clear_texts(qdNamedObject* owner);

	//! Отрисовка набора.
	void redraw() const;
	bool pre_redraw() const;
	bool post_redraw();

	grScreenRegion screen_region() const;

	//! Расставляет тексты в наборе.
	bool arrange_texts();

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	bool need_redraw() const { return need_redraw_; }

	bool is_empty() const { return texts_.empty(); }

	void set_max_text_width(int width){ max_text_width_ = width; format_texts(); arrange_texts(); }

	//! Возвращает текст, в который попадает точка с экранными координатами (x,y).
	qdScreenText* get_text(int x,int y);

	bool was_changed() const { return was_changed_; }
	void toggle_changed(bool state){ was_changed_ = state; }

	int new_texts_height() const { return new_texts_height_; }
	void clear_new_texts_height(){ new_texts_height_ = 0; }

	void clear_hover_mode();

private:

	//! Идентификатор набора.
	int ID_;

	//! Экранные координаты центра набора.
	Vect2i pos_;
	//! Размеры области, отведенной под набор на экране.
	Vect2i size_;

	//! Расстояние между соседними текстами в пикселах.
	int space_;

	typedef std::vector<qdScreenText> texts_container_t;
	//! Тексты.
	texts_container_t texts_;

	//! Устанавливается в true при добавлении/удалении текстов.
	bool was_changed_;

	//! Максимальная ширина текста в пикселах.
	/**
	Если нулевая - не учитывается.
	*/
	int max_text_width_;

	int new_texts_height_;

	bool need_redraw_;
	grScreenRegion last_screen_region_;

	//! Форматирует тексты по ширине, чтобы не вылезали за max_text_width_.
	void format_texts();
};

#endif /* __QD_SCREEN_TEXT_SET_H__ */
