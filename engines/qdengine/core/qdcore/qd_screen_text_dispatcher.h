#ifndef __QD_SCREEN_TEXT_DISPATCHER__
#define __QD_SCREEN_TEXT_DISPATCHER__

#include "mouse_input.h"
#include "qd_screen_text_set.h"

//! Диспетчер экранных текстов.
class qdScreenTextDispatcher
{
public:
	qdScreenTextDispatcher();
	~qdScreenTextDispatcher();

	//! Очистка всех текстов.
	void clear_texts();
	//! Очистка всех текстов с владельцем owner.
	void clear_texts(qdNamedObject* owner);

	//! Добавление текста в определенный набор.
	qdScreenText* add_text(int set_ID,const qdScreenText& txt);
	//! Добавление набора текстов.
	qdScreenTextSet* add_text_set(const qdScreenTextSet& set){ text_sets_.push_back(set); return &text_sets_.back(); }
	//! Поиск набора текстов.
	const qdScreenTextSet* get_text_set(int id) const;
	//! Поиск набора текстов.
	qdScreenTextSet* get_text_set(int id);

	//! Отрисовка текстов.
	void redraw() const;
	void pre_redraw() const;
	void post_redraw();

	bool save_script(XStream& fh,int indent = 0) const;

private:

	typedef std::vector<qdScreenTextSet> text_sets_container_t;
	//! Наборы текстов.
	text_sets_container_t text_sets_;
};

#endif /* __QD_SCREEN_TEXT_DISPATCHER__ */






















