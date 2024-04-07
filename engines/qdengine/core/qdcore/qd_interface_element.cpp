/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "snd_dispatcher.h"

#include "qd_sound.h"
#include "qd_interface_screen.h"
#include "qd_interface_element.h"
#include "qd_interface_element_state.h"
#include "qd_interface_background.h"
#include "qd_interface_counter.h"
#include "qd_interface_button.h"
#include "qd_interface_save.h"
#include "qd_interface_slider.h"
#include "qd_interface_text_window.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

Vect2i qdInterfaceElement::screen_offset_ = Vect2i(0,0);

qdInterfaceElement::qdInterfaceElement() : r_(0,0),
	option_ID_(OPTION_NONE),
	screen_depth_(0),
	is_visible_(true),
	is_locked_(false),
	last_animation_frame_(NULL)
{
}

qdInterfaceElement::qdInterfaceElement(const qdInterfaceElement& el) : qdInterfaceObjectBase(el),
	r_(el.r_),
	option_ID_(el.option_ID_),
	option_data_(el.option_data_),
	screen_depth_(el.screen_depth_),
	is_visible_(el.is_visible_),
	last_animation_frame_(NULL)
{
}

qdInterfaceElement::~qdInterfaceElement()
{
}

qdInterfaceElement& qdInterfaceElement::operator = (const qdInterfaceElement& el)
{
	if(this == &el) return *this;

	this -> qdInterfaceObjectBase::operator = (el);

	r_ = el.r_;
	option_ID_ = el.option_ID_;
	option_data_ = el.option_data_;
	screen_depth_ = el.screen_depth_;
	is_visible_ = el.is_visible_;

	return *this;
}

bool qdInterfaceElement::set_animation(const qdAnimation* anm,int anm_flags)
{
	if(anm){
		anm -> create_reference(&animation_);

		if(anm_flags & QD_ANIMATION_FLAG_LOOP)
			animation_.set_flag(QD_ANIMATION_FLAG_LOOP);

		if(anm_flags & QD_ANIMATION_FLAG_FLIP_HORIZONTAL)
			animation_.set_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);

		if(anm_flags & QD_ANIMATION_FLAG_FLIP_VERTICAL)
			animation_.set_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);

		animation_.start();
	}
	else
		animation_.clear();

	return true;
}

bool qdInterfaceElement::set_state(const qdInterfaceElementState* p)
{
	qdInterfaceElementState::state_mode_t mode = p -> state_mode();

	set_animation(p -> animation(mode),p -> animation_flags(mode));

	if(p -> sound(mode)){
		if(sndDispatcher* dp = sndDispatcher::get_dispatcher())
			dp -> stop_sound(&sound_handle_);

		p -> sound(mode) -> play(&sound_handle_);
	}

	return true;
}

bool qdInterfaceElement::redraw() const
{
	animation_.redraw(r().x,r().y,0);
	return true;
}

bool qdInterfaceElement::need_redraw() const
{
	if(last_animation_frame_ != animation_.get_cur_frame())
		return true;

	if(last_screen_region_ != screen_region())
		return true;

	return false;
}

bool qdInterfaceElement::post_redraw()
{
	last_screen_region_ = screen_region();
	last_animation_frame_ = animation_.get_cur_frame();
	
	return true;
}

bool qdInterfaceElement::quant(float dt)
{
	animation_.quant(dt);
	return true;
}

grScreenRegion qdInterfaceElement::screen_region() const
{
	if(!animation_.is_empty()){
		grScreenRegion reg = animation_.screen_region();
		reg.move(r().x,r().y);

		return reg;
	}
	else
		return grScreenRegion::EMPTY;
}

bool qdInterfaceElement::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < '\t';
	fh < "<interface_element ";
	fh < " type=\"" <= static_cast<int>(get_element_type()) < "\"";

	if(name()) fh < " name=\"" < qdscr_XML_string(name()) < "\"";
	fh < " pos=\"" <= r_.x < " " <= r_.y < " " <= screen_depth_ < "\"";

	if(option_ID_ != OPTION_NONE)
		fh < " option_id=\"" <= (int)option_ID_ < "\"";

	fh < ">\r\n";

	if(!save_script_body(fh,indent)) return false;

	for(int i = 0; i < indent; i ++) fh < '\t';
	fh < "</interface_element>\r\n";

	return true;
}

bool qdInterfaceElement::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_INTERFACE_ELEMENT_POS:
			xml::tag_buffer(*it) > r_.x > r_.y > screen_depth_;
			break;
		case QDSCR_INTERFACE_OPTION_ID:
			option_ID_ = option_ID_t(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return load_script_body(p);
}

qdResource* qdInterfaceElement::add_resource(const char* file_name,const qdInterfaceElementState* res_owner)
{
	if(qdInterfaceScreen* p = dynamic_cast<qdInterfaceScreen*>(owner()))
		return p -> add_resource(file_name,res_owner);

	return NULL;
}

bool qdInterfaceElement::remove_resource(const char* file_name,const qdInterfaceElementState* res_owner)
{
	if(qdInterfaceScreen* p = dynamic_cast<qdInterfaceScreen*>(owner()))
		return p -> remove_resource(file_name,res_owner);

	return false;
}

qdInterfaceElement* qdInterfaceElement::create_element(element_type tp)
{
	switch(tp){
	case EL_BACKGROUND:
		return new qdInterfaceBackground;
	case EL_BUTTON:
		return new qdInterfaceButton;
	case EL_SLIDER:
		return new qdInterfaceSlider;
	case EL_SAVE:
		return new qdInterfaceSave;
	case EL_TEXT_WINDOW:
		return new qdInterfaceTextWindow;
	case EL_COUNTER:
		return new qdInterfaceCounter;
	}

	return NULL;
}

void qdInterfaceElement::destroy_element(qdInterfaceElement* p)
{
	delete p;
}


bool qdInterfaceElement::hit_test(int x,int y) const
{
	if(!animation_.is_empty()){
		x -= r().x;
		y -= r().y;

		return animation_.hit(x,y);
	}

	return false;
}

qdInterfaceElement::state_status_t qdInterfaceElement::state_status(const qdInterfaceElementState* p) const
{
	qdInterfaceElementState::state_mode_t mode = p -> state_mode();

	if(p -> animation(mode)){
		if(p -> animation(mode) -> is_reference(&animation_)){
			if(!animation_.is_finished())
				return STATE_ACTIVE;
		}
		else
			return STATE_INACTIVE;
	}
	else {
		if(!animation_.is_empty())
			return STATE_INACTIVE;
	}

	if(p -> sound(mode)){
		if(!p -> sound(mode) -> is_stopped(&sound_handle_))
			return STATE_ACTIVE;
	}

	return STATE_DONE;
}
