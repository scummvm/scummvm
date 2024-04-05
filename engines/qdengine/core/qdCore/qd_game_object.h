#ifndef __QD_GAME_OBJECT_H__
#define __QD_GAME_OBJECT_H__

#include "xml_fwd.h"

#include "mouse_input.h"
#include "gr_screen_region.h"

#include "qd_named_object.h"

const int QD_OBJ_MOVING_FLAG			= 0x01;
const int QD_OBJ_HIDDEN_FLAG			= 0x02;
const int QD_OBJ_INVERSE_PERSPECTIVE_FLAG	= 0x04;
const int QD_OBJ_SHOW_NAME_FLAG			= 0x08;
const int QD_OBJ_NO_SCALE_FLAG			= 0x10;
const int QD_OBJ_SCREEN_COORDS_FLAG		= 0x20;
const int QD_OBJ_NON_PLAYER_PERSONAGE_FLAG	= 0x40;
const int QD_OBJ_HAS_BOUND_FLAG			= 0x80;
const int QD_OBJ_DISABLE_MOVEMENT_FLAG		= 0x100;
const int QD_OBJ_DISABLE_MOUSE_FLAG		= 0x200;
const int QD_OBJ_IS_IN_TRIGGER_FLAG		= 0x400;
const int QD_OBJ_STATE_CHANGE_FLAG		= 0x800;
const int QD_OBJ_IS_IN_INVENTORY_FLAG		= 0x1000;
const int QD_OBJ_KEYBOARD_CONTROL_FLAG		= 0x2000;
const int QD_OBJ_FIXED_SCREEN_COORDS_FLAG	= 0x4000;

//! Базовый класс для игровых объектов.
class qdGameObject : public qdNamedObject
{
public:
	qdGameObject();
	qdGameObject(const qdGameObject& obj);
	~qdGameObject();

	qdGameObject& operator = (const qdGameObject& obj);

	void set_parallax_offset(int offs_x,int offs_y){ parallax_offset_.x = offs_x; parallax_offset_.y = offs_y; }
	const Vect2i& get_parallax_offset() const { return parallax_offset_; }

	virtual void redraw(int offs_x = 0,int offs_y = 0) const = 0;
	virtual void debug_redraw() const = 0;
	virtual void draw_contour(unsigned color) const = 0;
	virtual void draw_shadow(int offs_x,int offs_y,unsigned color,int alpha) const = 0;

	virtual bool need_redraw() const { return false; }
	virtual void post_redraw(){ }

	virtual const grScreenRegion& last_screen_region() const { return grScreenRegion::EMPTY; }
	virtual grScreenRegion screen_region() const { return grScreenRegion(grScreenRegion::EMPTY); }

	virtual bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev) = 0;
	virtual bool hit(int x,int y) const = 0;

#ifdef _QUEST_EDITOR
	const Vect2i& screen_pos();
#endif
	const Vect2i& screen_pos() const { return screen_r_; }
	virtual bool update_screen_pos();
	virtual Vect2s screen_size() const = 0;

	float screen_depth() const { return screen_depth_; }

	virtual bool is_visible() const { return !check_flag(QD_OBJ_HIDDEN_FLAG); }

	const Vect3f& R() const { return r_; }
	void set_pos(const Vect3f& rr){ r_ = rr; }

	virtual bool load_script(const xml::tag* p) = 0;
	virtual bool save_script(XStream& fh,int indent = 0) const = 0;

	virtual bool load_resources(){ return true; };
	virtual void free_resources(){ };

	//! Инициализация объекта, вызывается при старте и перезапуске игры.
	virtual bool init();

	virtual void quant(float dt);

	virtual bool init_grid_zone(){ return false; }
	virtual bool toggle_grid_zone(bool make_walkable = false){ return false; }
	virtual bool save_grid_zone(){ return false; }
	virtual bool restore_grid_zone(){ return false; }
	virtual bool set_grid_zone_attributes(int attr) const { return false; }
	virtual bool drop_grid_zone_attributes(int attr) const { return false; }

	//! Копирует в буфер отладочную информацию для вывода на экран в отладочном режиме.
	virtual bool get_debug_info(XBuffer& buf) const { return true; }
	
	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	const Vect2i& get_screen_R() const { return screen_r_; }

protected:

	virtual bool load_script_body(const xml::tag* p);
	virtual bool save_script_body(XStream& fh,int indent = 0) const;

	void set_screen_R(const Vect2i& v){ screen_r_ = v; }

private:

	Vect3f r_;
	Vect2i parallax_offset_;

	Vect2i screen_r_;
	float screen_depth_;
};

#ifdef __QD_DEBUG_ENABLE__
inline bool qdbg_is_object(const qdNamedObject* obj,const char* scene_name,const char* object_name)
{
	if(dynamic_cast<const qdGameObject*>(obj)){
		if(obj -> name() && !strcmp(object_name,obj -> name())){
			if(!scene_name || (obj -> owner() && obj -> owner() -> name() && !strcmp(obj -> owner() -> name(),scene_name)))
				return true;
		}
	}

	return false;
}
#else
inline bool qdbg_is_object(const qdNamedObject*,const char*,const char*){ return false; }
#endif

#endif /* __QD_GAME_OBJECT_H__ */

