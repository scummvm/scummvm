#ifndef __QD_GAME_OBJECT_STATIC_H__
#define __QD_GAME_OBJECT_STATIC_H__

#include "qd_sprite.h"
#include "qd_game_object.h"

class XStream;

class qdGameObjectStatic : public qdGameObject
{
public:
	qdGameObjectStatic();
	~qdGameObjectStatic();

	int named_object_type() const { return QD_NAMED_OBJECT_STATIC_OBJ; }

	const qdSprite* get_sprite() const { return &sprite_; }
	qdSprite* get_sprite() { return &sprite_; }

	void redraw(int offs_x = 0,int offs_y = 0) const;
	void debug_redraw() const { }
	void draw_contour(unsigned color) const;
	void draw_shadow(int offs_x,int offs_y,unsigned color,int alpha) const { }

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev){ return false; }

	bool load_resources();
	void free_resources();

	Vect2s screen_size() const { return Vect2s(sprite_.size_x(),sprite_.size_y()); }

	bool hit(int x,int y) const;

#ifdef _QUEST_EDITOR
	bool remove_sprite_edges();
#endif //_QUEST_EDITOR
protected:

	bool load_script_body(const xml::tag* p);
	bool save_script_body(XStream& fh,int indent = 0) const;

private:
	qdSprite sprite_;
};

#endif /* __QD_GAME_OBJECT_STATIC_H__ */