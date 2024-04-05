/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <stdlib.h>

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_game_dispatcher.h"
#include "qd_inventory_cell.h"
#include "qd_interface_element.h"
#include "qd_named_object_reference.h"
#include "qd_game_object_animated.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

// class qdInventoryCellType

bool qdInventoryCellType::load_resources() const
{
	return sprite_.load();
}

void qdInventoryCellType::free_resources() const
{
	sprite_.free();
}

bool qdInventoryCellType::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_TYPE:
			set_type(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_FILE:
			sprite_.set_file(it -> data());
			break;
		}
	}

	return true;
}

bool qdInventoryCellType::save_script(class XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<inventory_cell_type type=\"" <= type_ < "\"";
		
	if(strlen(sprite_.file()))
		fh < " file=\"" < qdscr_XML_string(sprite_.file()) < "\"";

	fh < "/>\r\n";

	return true;
}

// class qdInventoryCell

Vect2i qdInventoryCell::screen_offset_ = Vect2i(0,0);
unsigned qdInventoryCell::shadow_color_ = 0;
int qdInventoryCell::shadow_alpha_ = -1;

qdInventoryCell::qdInventoryCell() : type_(0),
#ifndef _QUEST_EDITOR
	sprite_(NULL),
#endif
	object_(NULL)
{ 
}

qdInventoryCell::qdInventoryCell(const qdInventoryCellType& tp) : type_(tp.type()),
#ifndef _QUEST_EDITOR
	sprite_(tp.sprite()),
#endif // _QUEST_EDITOR
	object_(NULL)
{ 
}

qdInventoryCell::qdInventoryCell(const qdInventoryCell& cl) : type_(cl.type_), 
#ifndef _QUEST_EDITOR
	sprite_(cl.sprite_),
#endif // _QUEST_EDITOR
	object_(cl.object_)
{ 
}

qdInventoryCell& qdInventoryCell::operator = (const qdInventoryCell& cl)
{
	if(this == &cl) return *this;

	type_ = cl.type_;
#ifndef _QUEST_EDITOR
	sprite_ = cl.sprite_;
#endif // _QUEST_EDITOR
	object_ = cl.object_;

	return *this;
}

#ifdef _QUEST_EDITOR
const qdSprite* qdInventoryCell::sprite() const
{
	qdGameDispatcher* p = qd_get_game_dispatcher();
	if(p){
		const qdInventoryCellType* ct = p -> get_inventory_cell_type(type_);
		if(ct) return ct -> sprite();
	}

	return NULL;
}
#endif

void qdInventoryCell::redraw(int x,int y,bool inactive_mode) const
{
	if(sprite())
		sprite() -> redraw(x,y,0);

	if(object_){
		object_ -> set_pos(Vect3f(x,y,0));
		object_ -> set_flag(QD_OBJ_SCREEN_COORDS_FLAG);
		object_ -> update_screen_pos();
		object_ -> redraw();

		if(inactive_mode)
			object_->draw_shadow(0,0,shadow_color_,shadow_alpha_);
	}
}

void qdInventoryCell::set_object(qdGameObjectAnimated* obj)
{
	object_ = obj; 
	if(object_) object_ -> set_flag(QD_OBJ_SCREEN_COORDS_FLAG);
}

bool qdInventoryCell::load_resources()
{
	if(object_) 
		return object_ -> load_resources();
	return true;
}

bool qdInventoryCell::free_resources()
{
	if(object_) object_ -> free_resources();
	return true;
}

bool qdInventoryCell::load_data(qdSaveStream& fh,int save_version)
{
	char flag;
	fh > flag;

	if(flag){
		qdNamedObjectReference ref;
		if(!ref.load_data(fh,save_version)) return false;

		if(qdGameDispatcher* p = qdGameDispatcher::get_dispatcher())
			object_ = static_cast<qdGameObjectAnimated*>(p -> get_named_object(&ref));
	}
	else
		object_ = NULL;
	
	return true;
}

bool qdInventoryCell::save_data(qdSaveStream& fh) const
{
	if(object_){
		fh < char(1);

		qdNamedObjectReference ref(object_);
		if(!ref.save_data(fh)) return false;
	}
	else 
		fh < char(0);

	return true;
}


// class qdInventoryCellSet

qdInventoryCellSet::qdInventoryCellSet() : size_(0,0), 
	screen_pos_(0,0),
	additional_cells_(0,0),
	cells_shift_(0,0)
{
}

qdInventoryCellSet::qdInventoryCellSet(int x,int y,int sx,int sy,short addit_sx, short addit_sy,const qdInventoryCellType& tp) : size_(sx,sy),
	screen_pos_(x,y),
	additional_cells_(addit_sx,addit_sy),
	cells_shift_(0,0),
	cells_((sx+addit_sx) * (sy+addit_sy),tp)
{
}

qdInventoryCellSet::qdInventoryCellSet(const qdInventoryCellSet& set) : size_(set.size_),
	cells_(set.cells_),
	screen_pos_(set.screen_pos_),
	additional_cells_(set.additional_cells_),
	cells_shift_(set.cells_shift_)
{
}

qdInventoryCellSet::~qdInventoryCellSet()
{
	cells_.clear(); 
}

qdInventoryCellSet& qdInventoryCellSet::operator = (const qdInventoryCellSet& set)
{
	if(this == &set) return *this;

	size_ = set.size_;
	cells_ = set.cells_;
	screen_pos_ = set.screen_pos_;
	additional_cells_ = set.additional_cells_;
	cells_shift_ = set.cells_shift_;

	return *this;
}

void qdInventoryCellSet::redraw(int offs_x,int offs_y,bool inactive_mode) const
{
	Vect2s pos = screen_pos();
	
	int idx;
	for(int i = cells_shift_.y; i < size_.y+cells_shift_.y; i ++)
	{
		idx = i * (size_.x + additional_cells_.x) + cells_shift_.x;
		for(int j = cells_shift_.x; j < size_.x+cells_shift_.x; j ++)
		{
			xassert(idx >= 0 && idx < cells_.size());
			cells_[idx].redraw(offs_x + pos.x,offs_y + pos.y,inactive_mode);
			pos.x += cells_[idx].size_x();
			idx++;
		}
		pos.x = screen_pos().x;
		pos.y += cells_.front().size_y();
	}
}

bool qdInventoryCellSet::put_object(qdGameObjectAnimated* p)
{
	qdInventoryCellVector::iterator it;
	FOR_EACH(cells_,it){
		if(it -> is_empty() && it -> type() == p -> inventory_type()){
			it -> set_object(p);
			return true;
		}
	}

	return false;
}

bool qdInventoryCellSet::hit(const Vect2s& pos) const 
{
	if(cells_.empty() || !cells_.front().size_x() || !cells_.front().size_y()) 
		return false;

	Vect2s v = pos - screen_pos() + cells_.front().size()/2;

	if(v.x >= 0 && v.x < size_.x * cells_.front().size_x() && v.y >= 0 && v.y < size_.y * cells_.front().size_y())
		return true;

	return false;
}

grScreenRegion qdInventoryCellSet::screen_region() const
{
	if(cells_.empty() || !cells_.front().size_x() || !cells_.front().size_y()) 
		return grScreenRegion::EMPTY;

	int sx = size_.x * cells_.front().size_x();
	int sy = size_.y * cells_.front().size_y();

	int x = screen_pos().x - cells_.front().size_x()/2 + sx/2;
	int y = screen_pos().y - cells_.front().size_y()/2 + sy/2;

	return grScreenRegion(x,y,sx,sy);
}

bool qdInventoryCellSet::put_object(qdGameObjectAnimated* p,const Vect2s& pos)
{
	if(!hit(pos)) return false;

	Vect2s v = pos - screen_pos() + cells_.front().size()/2;
	int idx = v.x / cells_.front().size_x() + (v.y / cells_.front().size_y()) * size_.x;
	// ƒвигаем индекс на текущее смещение €чеек
	idx += cells_shift_.y * (size_.x + additional_cells_.x) + cells_shift_.x;

	if(idx >= 0 && idx < cells_.size() && cells_[idx].is_empty() && cells_[idx].type() == p -> inventory_type()){
		cells_[idx].set_object(p);
		return true;
	}

	return false;
}

qdGameObjectAnimated* qdInventoryCellSet::get_object(const Vect2s& pos) const
{
	if(!hit(pos)) return NULL;

	Vect2s v = pos - screen_pos() + cells_.front().size()/2;

	int idx = v.x / cells_.front().size_x() + (v.y / cells_.front().size_y()) * size_.x;
	// ƒвигаем индекс на текущее смещение €чеек
	idx += cells_shift_.y * (size_.x + additional_cells_.x) + cells_shift_.x;

	if(idx >= 0 && idx < cells_.size())
		return cells_[idx].object();

	return NULL;
}

bool qdInventoryCellSet::remove_object(qdGameObjectAnimated* p)
{
	qdInventoryCellVector::iterator it;
	FOR_EACH(cells_,it){
		if(it -> object() == p){
			it -> set_object(NULL);
			return true;
		}
	}
	
	return false;
}

bool qdInventoryCellSet::is_object_in_list(const qdGameObjectAnimated* p) const
{
	qdInventoryCellVector::const_iterator it;
	FOR_EACH(cells_,it){
		if(it -> object() == p)
			return true;
	}

	return false;
}

bool qdInventoryCellSet::load_script(const xml::tag* p)
{
	Vect2s v;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_INVENTORY_CELL_SET_POS:
				xml::tag_buffer(*it) > v.x > v.y;
				set_screen_pos(v);
				break;
			case QDSCR_INVENTORY_CELL_SET_SIZE:
				xml::tag_buffer(*it) > v.x > v.y;
				set_size(v);
				break;
			case QDSCR_INVENTORY_CELL_SET_ADDITIONAL_CELLS:
				xml::tag_buffer(*it) > additional_cells_.x > additional_cells_.y;
				break;
		}
	}

	if(const xml::tag* tp = p -> search_subtag(QDSCR_TYPE))
		set_cell_type(qdInventoryCellType(xml::tag_buffer(*tp).get_int()));

	return true;
}

bool qdInventoryCellSet::save_script(class XStream& fh,int indent) const
{
	int tp = 0;
	if(!cells_.empty())
		tp = cells_.front().type();

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<inventory_cell_set type=\"" <= tp < "\">\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<inventory_cell_set_size>" <= size_.x < " " <= size_.y < "</inventory_cell_set_size>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<inventory_cell_set_pos>" <= screen_pos_.x < " " <= screen_pos_.y < "</inventory_cell_set_pos>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<inventory_cell_set_additional_cells>" <= additional_cells_.x < " " <= additional_cells_.y < "</inventory_cell_set_additional_cells>\r\n";
		   
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</inventory_cell_set>\r\n";
	
	return true;
}

bool qdInventoryCellSet::init(const qdInventoryCellTypeVector& tp)
{
	if(cells_.empty()) return false;

	qdInventoryCellTypeVector::const_iterator it;
	FOR_EACH(tp,it){
		if(it -> type() == cells_.front().type()){
			set_cell_type(*it);
			return true;
		}
	}

	return false;
}

#ifdef _QUEST_EDITOR
const Vect2i qdInventoryCellSet::screen_size() const
{
	if (cells_.empty()) return Vect2i(0,0);
	const qdInventoryCell& cell = cells_.front();
	return Vect2i(size_.x * cell.size_x(), size_.y * cell.size_y());
}

const Vect2i qdInventoryCellSet::cell_size() const
{
	if (cells_.empty()) return Vect2i(0, 0);
	return cells_.front().size();
}
#endif // _QUEST_EDITOR

bool qdInventoryCellSet::load_resources()
{
	qdInventoryCellVector::iterator it;
	FOR_EACH(cells_,it)
		it -> load_resources();

	return true;
}

bool qdInventoryCellSet::free_resources()
{
	qdInventoryCellVector::iterator it;
	FOR_EACH(cells_,it)
		it -> free_resources();

	return true;
}

bool qdInventoryCellSet::load_data(qdSaveStream& fh,int save_version)
{
	if (save_version >= 102)
		fh > additional_cells_.x > additional_cells_.y;
	qdInventoryCellVector::iterator it;
	FOR_EACH(cells_,it){
		if(!it -> load_data(fh,save_version))
			return false;
	}

	return true;
}

bool qdInventoryCellSet::save_data(qdSaveStream& fh) const
{
	fh < additional_cells_.x < additional_cells_.y;
	qdInventoryCellVector::const_iterator it;
	FOR_EACH(cells_,it){
		if(!it -> save_data(fh))
			return false;
	}

	return true;
}

int qdInventoryCellSet::cell_index(const qdGameObjectAnimated* obj) const
{
	qdInventoryCellVector::const_iterator it;
	FOR_EACH(cells_,it){
		if(it -> object() == obj)
			return (it - cells_.begin());
	}

	return -1;
}

Vect2s qdInventoryCellSet::cell_position(int cell_idx) const
{
	if(cell_idx >= 0 && cell_idx < cells_.size()){
		int x = (cell_idx % size_.x) * cells_.front().size_x() + screen_pos().x;
		int y = (cell_idx / size_.x) * cells_.front().size_y() + screen_pos().y;
		// ƒелаем поправку на смещение €чеек
		x -= cells_.front().size_x() * cells_shift_.x;
		y -= cells_.front().size_y() * cells_shift_.y;
		return Vect2s(x,y);
	}

	return Vect2s(0,0);
}

void qdInventoryCellSet::set_mouse_hover_object(qdGameObjectAnimated* obj)
{
	qdInventoryCellVector::const_iterator it;
	FOR_EACH(cells_,it){
		if(it -> object() && it -> object() != obj && it -> object() -> get_cur_state()){
			if(it -> object() -> get_cur_state() -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE)){
				if(qdGameObjectState* sp = it -> object() -> get_inventory_state())
					it -> object() -> set_state(sp);
			}
		}
	}

	if(obj){
		if(qdGameObjectState* p = obj -> get_mouse_hover_state()){
			if(obj -> get_cur_state() != p)
				p -> set_prev_state(obj -> get_cur_state());
			obj -> set_state(p);
		}
	}
}

void qdInventoryCellSet::objects_quant(float dt)
{
	qdInventoryCellVector::iterator it;
	FOR_EACH(cells_,it){
		if(!it -> is_empty())
			it -> object() -> quant(dt);
	}
}

void qdInventoryCellSet::pre_redraw() const
{
	qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();
	if(!dp) return;

	int idx;
	for (int i = cells_shift_.y; i < size().y + cells_shift_.y; i++)
	{
		idx = i * (size_.x + additional_cells_.x) + cells_shift_.x;
		for (int j = cells_shift_.x; j < size().x + cells_shift_.x; j++)
		{
			xassert(idx >= 0 && idx < cells_.size());
			if (!cells_[idx].is_empty() && cells_[idx].object()->need_redraw())
			{
				dp -> add_redraw_region(cells_[idx].object()->last_screen_region());
				dp -> add_redraw_region(cells_[idx].object()->screen_region());
			}
			idx++;
		}
	}

	grScreenRegion reg = screen_region();
	if(reg != last_screen_region_){
		dp -> add_redraw_region(last_screen_region_);
		dp -> add_redraw_region(reg);
	}
}

void qdInventoryCellSet::post_redraw()
{
	int idx;
	for (int i = cells_shift_.y; i < size().y + cells_shift_.y; i++)
	{
		idx = i * (size_.x + additional_cells_.x) + cells_shift_.x;
		for (int j = cells_shift_.x; j < size().x + cells_shift_.x; j++)
		{
			xassert(idx >= 0 && idx < cells_.size());
			if (!cells_[idx].is_empty())
				cells_[idx].object()->post_redraw();
			idx++;
		}
	}

	last_screen_region_ = screen_region();
}

bool qdInventoryCellSet::has_rect_objects(int left, int top, int right, int bottom) const
{
	int idx;
	for (int i = top; i <= bottom; i++)
		for (int j = left; j <= right; j++)
		{
			idx = i * (size_.x + additional_cells_.x) + j;
			xassert(idx >= 0 && idx < cells_.size());
			// Ќашли объект вне видимой области - значит скроллинг нужен
			if (!cells_[idx].is_empty())
				return true;
		}
	return false;
}

void qdInventoryCellSet::scroll_left()
{ 
	if (!has_rect_objects(0, 0, cells_shift_.x-1, size_.y+additional_cells_.y-1)) 
		return;
	cells_shift_.x--;
	if (cells_shift_.x < 0) cells_shift_.x = additional_cells_.x;
}

void qdInventoryCellSet::scroll_right()
{ 
	if (!has_rect_objects(cells_shift_.x+size_.x, 0, size_.x+additional_cells_.x-1, size_.y+additional_cells_.y-1))
		return;
	cells_shift_.x++;
	if (cells_shift_.x > additional_cells_.x) cells_shift_.x = 0;
}

void qdInventoryCellSet::scroll_up()
{ 
	if (!has_rect_objects(0, 0, size_.x+additional_cells_.x-1, cells_shift_.y-1))
		return;
	cells_shift_.y--;
	if (cells_shift_.y < 0) cells_shift_.y = additional_cells_.y;
}

void qdInventoryCellSet::scroll_down()
{ 
	if (!has_rect_objects(0, cells_shift_.y+size_.y, size_.x+additional_cells_.x-1, size_.y+additional_cells_.y-1))
		return;
	cells_shift_.y++;
	if (cells_shift_.y > additional_cells_.y) cells_shift_.y = 0;
}

void qdInventoryCellSet::debug_log() const
{
#ifdef _DEBUG
	for(int i = cells_shift_.y; i < size().y + cells_shift_.y; i++){
		int idx = i * (size_.x + additional_cells_.x) + cells_shift_.x;
		for(int j = cells_shift_.x; j < size().x + cells_shift_.x; j++){
			if(!cells_[idx].is_empty()){
				appLog::default_log() << i << " " << j << " " << cells_[idx].object()->name() << "\r\n";
			}
		}
	}
#endif
}
