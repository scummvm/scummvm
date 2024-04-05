/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag.h"
#include "qdscr_parser.h"

#include "qd_game_object_static.h"
#include "qd_game_scene.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdGameObjectStatic::qdGameObjectStatic()
{
}

qdGameObjectStatic::~qdGameObjectStatic()
{
}

void qdGameObjectStatic::redraw(int offs_x,int offs_y) const
{
	Vect2i scrCoord = screen_pos() + Vect2i(offs_x,offs_y);
	sprite_.redraw(scrCoord.x,scrCoord.y,screen_depth(),0);
}

bool qdGameObjectStatic::load_script(const xml::tag* p)
{
	return load_script_body(p);
}

bool qdGameObjectStatic::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<static_object name=\"" < qdscr_XML_string(name()) < "\">\r\n";

	save_script_body(fh,indent);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</static_object>\r\n";

	return true;
}

bool qdGameObjectStatic::load_script_body(const xml::tag* p)
{
	qdGameObject::load_script_body(p);

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_FILE:
				sprite_.set_file(it -> data());
				break;
		}
	}

	return true;
}

bool qdGameObjectStatic::save_script_body(XStream& fh,int indent) const
{
	qdGameObject::save_script_body(fh,indent);

	for(int i = 0; i <= indent; i ++) fh < "\t";
	if(sprite_.file()) fh < "<file>" < qdscr_XML_string(sprite_.file()) < "</file>\r\n";

	return true;
}

bool qdGameObjectStatic::load_resources()
{
	return sprite_.load();
}

void qdGameObjectStatic::free_resources()
{ 
	sprite_.free();
}

bool qdGameObjectStatic::hit(int x,int y) const
{
#ifdef _QUEST_EDITOR
	Vect2s pos = screen_pos();
	return sprite_.hit(x - pos.x,y - pos.y);
#endif
	return false;
}

void qdGameObjectStatic::draw_contour(unsigned color) const
{
	Vect2s pos = screen_pos();
	sprite_.draw_contour(pos.x,pos.y,color);
}

#ifdef _QUEST_EDITOR
bool qdGameObjectStatic::remove_sprite_edges()
{
	if(!qdCamera::current_camera()) return false;

	qdCamera const&  camera = *qdCamera::current_camera();
	int sx = sprite_.size_x();
	int sy = sprite_.size_y();

	Vect2i offs = sprite_.remove_edges();
	sprite_.save();

	offs.x = offs.x + sprite_.size_x()/2 - sx/2;
	offs.y = offs.y + sprite_.size_y()/2 - sy/2;

	Vect2s pos = screen_pos();
//	float d = screen_depth();
	Vect3f camPos = camera.global2camera_coord(R());
	pos = camera.scr2rscr(pos + offs);
	set_pos(camera.rscr2global(pos,camPos.z));

	return true;
}
#endif //_QUEST_EDITOR

