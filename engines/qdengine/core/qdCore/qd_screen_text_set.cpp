/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "qdscr_parser.h"
#include "qd_screen_text_set.h"
#include "qd_game_dispatcher.h"

#include "xml_tag_buffer.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdScreenTextSet::qdScreenTextSet() : ID_(0),
	pos_(0,0),
	size_(0,0),
	space_(5),
	was_changed_(false),
	need_redraw_(false)
{
	max_text_width_ = 0;
	new_texts_height_ = 0;
	texts_.reserve(16);
}

qdScreenTextSet::~qdScreenTextSet()
{
}

void qdScreenTextSet::redraw() const
{
	texts_container_t::const_iterator it;
	FOR_EACH(texts_,it)
		it -> redraw(pos_);

//	grDispatcher::instance()->Rectangle(pos_.x - size_.x/2,pos_.y - size_.y/2,size_.x,size_.x,0xFFFFFF,0,GR_OUTLINED);
}

bool qdScreenTextSet::arrange_texts()
{
	if(texts_.empty()) return false;

	int row_sx = texts_[0].size_x() + space_;
	int row_sy = texts_[0].size_y();

	texts_[0].set_screen_pos(Vect2i(space_,0));

	Vect2i text_pos(0,0);

	for(int i = 1; i < texts_.size(); i ++){
		if(texts_[i].arrangement() == qdScreenTextFormat::ARRANGE_HORIZONTAL){
			if(row_sy < texts_[i].size_y())
				row_sy = texts_[i].size_y();

			text_pos.x += texts_[i - 1].size_x() + space_;
			row_sx += texts_[i - 1].size_x() + space_;
		}
		else {
			text_pos.x = 0;
			text_pos.y += row_sy + space_;

			row_sx = texts_[i].size_x();
			row_sy = texts_[i].size_y();
		}

		texts_[i].set_screen_pos(text_pos);
	}

	int row_start = 0;
	int row_size = 1;

	for(int i = 0; i < texts_.size(); i ++){
		if(texts_[i].arrangement() == qdScreenTextFormat::ARRANGE_VERTICAL || i == texts_.size() - 1){
			int sx = 0;
			for(int j = 0; j < row_size; j++)
				sx += texts_[row_start + j].size_x() + space_;

			int dx = 0;
			switch(texts_[row_start].alignment()){
				case qdScreenTextFormat::ALIGN_CENTER:
					dx = (max_text_width_ - sx - space_)/2;
					break;
				case qdScreenTextFormat::ALIGN_RIGHT:
					dx = max_text_width_ - sx - space_;
					break;
			}

			for(int j = 0; j < row_size; j++){
				Vect2i pos = texts_[row_start + j].screen_pos();
				pos.x += dx;
				texts_[row_start + j].set_screen_pos(pos);
			}

			row_size = 1;
			row_start = i;
		}
		else
			row_size++;
	}

	int sx = 0;
	int sy = 0;

	for(int i = 0; i < texts_.size(); i ++){
		Vect2i sz = Vect2i(texts_[i].size_x(),texts_[i].size_y());
		Vect2i ps = texts_[i].screen_pos();

		if(texts_[i].screen_pos().x + texts_[i].size_x() > sx)
			sx = texts_[i].screen_pos().x + texts_[i].size_x();
		if(texts_[i].screen_pos().y + texts_[i].size_y() > sy)
			sy = texts_[i].screen_pos().y + texts_[i].size_y();
	}

	size_ = Vect2i(sx,sy);
	for(int i = 0; i < texts_.size(); i ++){
		Vect2i pos = texts_[i].screen_pos();
		pos -= size_/2;
		texts_[i].set_screen_pos(pos);
	}

	need_redraw_ = true;
	return true;
}

qdScreenText* qdScreenTextSet::get_text(int x,int y)
{
	texts_container_t::iterator it;
	FOR_EACH(texts_,it)
		if(it -> hit(x - pos_.x,y - pos_.y)) return &*it;

	return NULL;
}

void qdScreenTextSet::clear_texts(qdNamedObject* owner)
{
	bool ret = false;
	texts_container_t::iterator it = std::remove_if(texts_.begin(),texts_.end(),std::bind2nd(std::mem_fun_ref(qdScreenText::is_owned_by),owner));
	if(it != texts_.end()){
		texts_.erase(it,texts_.end());
		ret = true;
	}

	if(ret){
		need_redraw_ = true;
		arrange_texts();
		toggle_changed(true);
	}
}

bool qdScreenTextSet::pre_redraw() const
{
	if(need_redraw_){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
			dp -> add_redraw_region(screen_region());
			dp -> add_redraw_region(last_screen_region_);
		}
	}

	return true;
}

bool qdScreenTextSet::post_redraw()
{
	need_redraw_ = false;
	last_screen_region_ = screen_region();
	return true;
}

grScreenRegion qdScreenTextSet::screen_region() const
{
	grScreenRegion reg;
	for(texts_container_t::const_iterator it = texts_.begin(); it != texts_.end(); ++it)
		reg += it -> screen_region();
	return reg;
}

void qdScreenTextSet::load_script(const xml::tag* p)
{
	Vect2f r;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_ID:
			xml::tag_buffer(*it) > ID_;
			break;
		case QDSCR_POS2D:
			xml::tag_buffer(*it) > r.x > r.y;
			pos_ = r;
			break;
		case QDSCR_SCREEN_SIZE:
			xml::tag_buffer(*it) > size_.x > size_.y;
			break;
		}
	}
}

bool qdScreenTextSet::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<text_set ID=\"" <= ID_ < "\"";

	if(pos_.x || pos_.y)
		fh < " pos_2d=\"" <= pos_.x < " " <= pos_.y < "\"";

	if(size_.x || size_.y)
		fh < " screen_size=\"" <= size_.x < " " <= size_.y < "\"";

	fh < "/>\r\n";

	return true;
}

qdScreenText* qdScreenTextSet::add_text(const qdScreenText& txt)
{
	int sy = size_.y;

	texts_.push_back(txt); 

	if(max_text_width_)
		texts_.back().format_text(max_text_width_ - space_*2);

	arrange_texts(); 
	toggle_changed(true);

	new_texts_height_ += size_.y - sy;

	return &texts_.back();
}

void qdScreenTextSet::clear_hover_mode()
{
	for(texts_container_t::iterator it = texts_.begin(); it != texts_.end(); ++it)
		it -> set_hover_mode(false);
}


void qdScreenTextSet::format_texts()
{
	if(!max_text_width_) return;

	for(texts_container_t::iterator it = texts_.begin(); it != texts_.end(); ++it)
		it -> format_text(max_text_width_);
}
   
