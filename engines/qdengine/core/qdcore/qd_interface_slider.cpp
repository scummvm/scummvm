/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "qd_interface_slider.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceSlider::qdInterfaceSlider() : active_rectangle_(0,0),
	 phase_(0.5f),
	 orientation_(SL_HORIZONTAL),
	 track_mouse_(false),
	 background_offset_(Vect2i(0,0))
{
	inverse_direction_ = false;
	background_.set_owner(this);
	slider_.set_owner(this);
}

qdInterfaceSlider::qdInterfaceSlider(const qdInterfaceSlider& sl) : qdInterfaceElement(sl),
	active_rectangle_(sl.active_rectangle_),
	phase_(sl.phase_),
	orientation_(sl.orientation_),
	inverse_direction_(sl.inverse_direction_),
	track_mouse_(false)
{
	background_.set_owner(this);
	slider_.set_owner(this);

	background_ = sl.background_;
	background_offset_ = sl.background_offset_;
	slider_ = sl.slider_;
}

qdInterfaceSlider::~qdInterfaceSlider()
{
	background_.unregister_resources();
	slider_.unregister_resources();
}

qdInterfaceSlider& qdInterfaceSlider::operator = (const qdInterfaceSlider& sl)
{
	if(this == &sl) return *this;

	*static_cast<qdInterfaceElement*>(this) = sl;

	background_ = sl.background_;
	background_offset_ = sl.background_offset_;
	slider_ = sl.slider_;

	active_rectangle_ = sl.active_rectangle_;

	phase_ = sl.phase_;
	orientation_ = sl.orientation_;
	inverse_direction_ = sl.inverse_direction_;

	track_mouse_ = false;

	return *this;
}

bool qdInterfaceSlider::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	x -= r().x;
	y -= r().y;

	Vect2i rect(active_rectangle_);
	if(!slider_animation_.is_empty()){
		if(rect.x < slider_animation_.size_x())
			rect.x = slider_animation_.size_x();
		if(rect.y < slider_animation_.size_y())
			rect.y = slider_animation_.size_y();
	}

	switch(ev){
	case mouseDispatcher::EV_LEFT_DOWN:
		if(x >= -rect.x/2 && x < rect.x/2 && y >= -rect.y/2 && y < rect.y/2){
			set_phase(offset2phase(Vect2i(x,y)));
			track_mouse_ = true;
			return true;
		}
		break;
	case mouseDispatcher::EV_MOUSE_MOVE:
		if(track_mouse_){
			if(mouseDispatcher::instance() -> is_pressed(mouseDispatcher::ID_BUTTON_LEFT)){
				set_phase(offset2phase(Vect2i(x,y)));
				return true;
			}
			else
				track_mouse_ = false;
		}
		break;
	}

	return false;
}

bool qdInterfaceSlider::keyboard_handler(int vkey)
{
	return false;
}

int qdInterfaceSlider::option_value() const
{
	return round(phase_ * 255.0f);
}

bool qdInterfaceSlider::set_option_value(int value)
{
	set_phase(float(value) / 255.0f);
	return true;
}

bool qdInterfaceSlider::init(bool is_game_active)
{
	set_state(&background_);
	set_slider_animation(slider_.animation());
	
	track_mouse_ = false;

	return true;
}

bool qdInterfaceSlider::save_script_body(XStream& fh,int indent) const
{
	if(!background_.save_script(fh,indent)) return false;
	if(!slider_.save_script(fh,indent)) return false;

	if(active_rectangle_.x || active_rectangle_.y){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<slider_rect>" <= active_rectangle_.x < " " <= active_rectangle_.y < "</slider_rect>\r\n";
	}

	if(background_offset_.x || background_offset_.y){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<background_offset>" <= background_offset_.x < " " <= background_offset_.y < "</background_offset>\r\n";
	}

	if(orientation_ != SL_HORIZONTAL){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<slider_orientation>" <= int(orientation_) < "</slider_orientation>\r\n";
	}

	if(inverse_direction_){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<inverse_direction>1</inverse_direction>\r\n";
	}
	
	return true;
}

bool qdInterfaceSlider::load_script_body(const xml::tag* p)
{
	bool background_flag = false;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_INTERFACE_ELEMENT_STATE:
			if(!background_flag){
				if(!background_.load_script(&*it)) return false;
				background_flag = true;
			}
			else {
				if(!slider_.load_script(&*it)) return false;
			}
			break;
		case QDSCR_INTERFACE_SLIDER_RECTANGLE:
			xml::tag_buffer(*it) > active_rectangle_.x > active_rectangle_.y;
			break;
		case QDSCR_INTERFACE_SLIDER_ORIENTATION: {
				int v;
				xml::tag_buffer(*it) > v;
				orientation_ = orientation_t(v);
			}
			break;
		case QDSCR_INVERSE_DIRECTION:
			inverse_direction_ = xml::tag_buffer(*it).get_int() != 0;
			break;
		case QDSCR_INTERFACE_BACKGROUND_OFFSET:
			xml::tag_buffer(*it) > background_offset_.x > background_offset_.y;
			break;
		}
	}

	return true;
}

bool qdInterfaceSlider::redraw() const
{
	Vect2i rr = r() + background_offset_;
	animation().redraw(rr.x,rr.y,0);

	if(!slider_animation_.is_empty()){
		Vect2i rr = r() + phase2offset(phase_);
		slider_animation_.redraw(rr.x,rr.y,0);
	}

	return true;
}

int qdInterfaceSlider::size_x() const
{
	int x = active_rectangle_.x;

	if(!slider_animation_.is_empty()){
		if(x < slider_animation_.size_x())
			x = slider_animation_.size_x();
	}

	return x;
}

int qdInterfaceSlider::size_y() const
{
	int y = active_rectangle_.y;

	if(!slider_animation_.is_empty()){
		if(y < slider_animation_.size_y())
			y = slider_animation_.size_y();
	}

	return y;
}

grScreenRegion qdInterfaceSlider::screen_region() const
{
	if(!slider_animation_.is_empty()){
		grScreenRegion reg = qdInterfaceElement::screen_region();
		reg.move(background_offset_.x, background_offset_.y);

		Vect2i rr = r() + phase2offset(phase_);
		grScreenRegion reg1 = slider_animation_.screen_region();
		reg1.move(rr.x,rr.y);

		reg += reg1;
		
		return reg;
	}
	else
		return qdInterfaceElement::screen_region();
}

bool qdInterfaceSlider::set_slider_animation(const qdAnimation* anm,int anm_flags)
{
	if(anm){
		anm -> create_reference(&slider_animation_);
		
		if(anm_flags & QD_ANIMATION_FLAG_LOOP)
			slider_animation_.set_flag(QD_ANIMATION_FLAG_LOOP);
		
		if(anm_flags & QD_ANIMATION_FLAG_FLIP_HORIZONTAL)
			slider_animation_.set_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);
		
		if(anm_flags & QD_ANIMATION_FLAG_FLIP_VERTICAL)
			slider_animation_.set_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);
		
		slider_animation_.start();
	}
	else
		slider_animation_.clear();
	
	return true;
}

Vect2i qdInterfaceSlider::phase2offset(float ph) const
{
	if(ph < 0.0f) ph = 0.0f;
	if(ph > 1.0f) ph = 1.0f;

	Vect2i offs(0,0);

	if(inverse_direction_)
		ph = 1.0f - ph;

	switch(orientation_){
	case SL_HORIZONTAL:
		offs.x = int(ph * active_rectangle_.x) - active_rectangle_.x/2;
		break;
	case SL_VERTICAL:
		offs.y = active_rectangle_.y/2 - int(ph * active_rectangle_.y);
		break;
	}

	return offs;
}

float qdInterfaceSlider::offset2phase(const Vect2i& offs) const
{
	float ph = 0.0f;

	switch(orientation_){
	case SL_HORIZONTAL:
		if(!active_rectangle_.x) return 0.0f;
		ph = float(offs.x + active_rectangle_.x/2)/float(active_rectangle_.x);
		break;
	case SL_VERTICAL:
		if(!active_rectangle_.y) return 0.0f;
		ph = float(-offs.y + active_rectangle_.y/2)/float(active_rectangle_.y);
		break;
	}

	if(ph < 0.0f) ph = 0.0f;
	if(ph > 1.0f) ph = 1.0f;
	
	if(inverse_direction_)
		ph = 1.0f - ph;

	return ph;
}

bool qdInterfaceSlider::hit_test(int x,int y) const
{
	x -= r().x;
	y -= r().y;

	Vect2i rect(active_rectangle_);
	if(!slider_animation_.is_empty()){
		if(rect.x < slider_animation_.size_x())
			rect.x = slider_animation_.size_x();
		if(rect.y < slider_animation_.size_y())
			rect.y = slider_animation_.size_y();
	}

	if(x >= -rect.x/2 && x < rect.x/2 && y >= -rect.y/2 && y < rect.y/2)
		return true;

	return false;
}
