#ifndef __QD_GAME_OBJECT_MOUSE_H__
#define __QD_GAME_OBJECT_MOUSE_H__

#include "qd_game_object_animated.h"

//! Мышь.
class qdGameObjectMouse : public qdGameObjectAnimated
{
public:
	qdGameObjectMouse();
	qdGameObjectMouse(const qdGameObjectMouse& obj);
	~qdGameObjectMouse();

	qdGameObjectMouse& operator = (const qdGameObjectMouse& obj);

	int named_object_type() const { return QD_NAMED_OBJECT_MOUSE_OBJ; }

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	bool load_data(qdSaveStream& fh,int save_version);
	bool save_data(qdSaveStream& fh) const;

	void take_object(qdGameObjectAnimated* p);
	qdGameObjectAnimated* object(){ return object_; }

	//! Идентификаторы для курсоров по-умолчанию.
	enum cursor_ID_t {
		//! общий курсор по-умолчанию
		DEFAULT_CURSOR,
		//! курсор для анимированных объектов
		OBJECT_CURSOR,
		//! курсор для зон на сетке
		ZONE_CURSOR,
		//! курсор для main menu
		MAIN_MENU_CURSOR,
		//! курсор для внутриигрового интерфейса
		INGAME_INTERFACE_CURSOR,
		//! курсор для объектов в инвентори
		INVENTORY_OBJECT_CURSOR,

		MAX_CURSOR_ID
	};

	//! Установка курсора по-умолчанию. 
	/**
	Номер состояния отсчитывается с нуля.
	*/
	void set_default_cursor(cursor_ID_t id,int state_index){ default_cursors_[id] = state_index; }
	//! Возвращает курсор по-умолчанию.
	int default_cursor(cursor_ID_t id) const { return default_cursors_[id]; }

	void set_cursor(cursor_ID_t id);
	void set_cursor_state(int state_idx){ if(cur_state() != state_idx) set_state(state_idx); }

	void pre_redraw();
	void redraw(int offs_x = 0,int offs_y = 0) const;
	void post_redraw();

	void quant(float dt);

	bool update_screen_pos();

	const Vect2f& screen_pos_offset() const { return screen_pos_offset_; }

protected:
	
	bool load_script_body(const xml::tag* p);
	bool save_script_body(XStream& fh,int indent = 0) const;

private:
	
	//! Объект, который прицеплен к курсору.
	qdGameObjectAnimated* object_;

	//! Номера состояний, соответствующие курсорам по-умолчанию.
	int default_cursors_[MAX_CURSOR_ID];

	grScreenRegion object_screen_region_;

	Vect2f screen_pos_offset_;
	Vect2f screen_pos_offset_delta_;

	void update_object_position() const;
};

#endif /* __QD_GAME_OBJECT_MOUSE_H__ */
