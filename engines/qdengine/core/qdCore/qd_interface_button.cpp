/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qdscr_parser.h"
#include "xml_tag.h"

#include "qd_game_dispatcher.h"
#include "qd_interface_button.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceButton::qdInterfaceButton() : cur_state_(-1)
{
}

qdInterfaceButton::qdInterfaceButton(const qdInterfaceButton& bt) : qdInterfaceElement(bt),
#ifndef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	states_(bt.states_),
#endif
	cur_state_(-1)
{

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	for(int i = 0; i < bt.num_states(); i++)
		states_.push_back(new qdInterfaceElementState(*bt.get_state(i)));
#endif

	for(int i = 0; i < num_states(); i++){
		get_state(i) -> set_owner(this);
		get_state(i) -> register_resources();
	}
}

qdInterfaceButton::~qdInterfaceButton()
{
	states_.clear();
}

qdInterfaceButton& qdInterfaceButton::operator = (const qdInterfaceButton& bt)
{
	if(this == &bt) return *this;

	*static_cast<qdInterfaceElement*>(this) = bt;

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	for(state_container_t::iterator it = states_.begin(); it != states_.end(); ++it)
		delete *it;

	states_.clear();

	for(int i = 0; i < bt.num_states(); i++)
		states_.push_back(new qdInterfaceElementState(*bt.get_state(i)));
#else
	states_.clear();
	states_ = bt.states_;
#endif

	for(int i = 0; i < num_states(); i++){
		get_state(i) -> set_owner(this);
		get_state(i) -> register_resources();
	}

	cur_state_ = -1;

	return *this;
}

bool qdInterfaceButton::activate_state(int state_num)
{
	if(state_num >= states_.size())
		state_num = states_.size() - 1;

	if(!states_.size() || cur_state_ == state_num) return false;

	if(state_num < 0)
		state_num = 0;

	cur_state_ = state_num;

	if(state_num >= 0 && state_num < states_.size())
		return set_state(get_state(state_num));

	return true;
}

bool qdInterfaceButton::activate_state(const char* state_name)
{
	for(int i = 0; i < num_states(); i++){
		qdInterfaceElementState* p = get_state(i);
		if(!strcmp(p -> name(),state_name))
			return activate_state(i);
	}

	return false;
}

bool qdInterfaceButton::set_option_value(int value)
{
	if(value < 0) value = 0;
	if(value >= states_.size()) value = states_.size() - 1;

	activate_state(value);
	return true;
}

bool qdInterfaceButton::add_state(const qdInterfaceElementState& st)
{
#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	states_.push_back(new qdInterfaceElementState(st));
#else
	states_.push_back(st);
#endif
	
	get_state(states_.size() - 1) -> set_owner(this);
	get_state(states_.size() - 1) -> register_resources();

	return true;
}

bool qdInterfaceButton::insert_state(int insert_before,const qdInterfaceElementState& st)
{
	assert(insert_before >= 0 && insert_before < states_.size());

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
	states_.insert(states_.begin() + insert_before,new qdInterfaceElementState(st));
#else
	states_.insert(states_.begin() + insert_before,st);
#endif

	get_state(insert_before) -> set_owner(this);
	get_state(insert_before) -> register_resources();

	return true;
}

bool qdInterfaceButton::erase_state(int state_num)
{
	assert(state_num >= 0 && state_num < states_.size());

#ifdef _QD_INTERFACE_BUTTON_PTR_CONTAINER
//	delete *(states_.begin() + state_num);
#endif

	states_.erase(states_.begin() + state_num);

	if(cur_state_ == state_num)
		activate_state(--cur_state_);

	return true;
}

bool qdInterfaceButton::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	if(cur_state_ == -1) return false;
	
	return get_state(cur_state_) -> mouse_handler(x,y,ev);
}

bool qdInterfaceButton::keyboard_handler(int vkey)
{
	return false;
}

bool qdInterfaceButton::init(bool is_game_active)
{
	set_lock(false);

	if(!is_game_active){
		for(int i = 0; i < num_states(); i++){
			if(get_state(i) -> need_active_game()){
				set_lock(true);
				break;
			}
		}
	}

	for(int i = 0; i < num_states(); i++){
		qdInterfaceElementState* p = get_state(i);
		p -> set_state_mode(qdInterfaceElementState::DEFAULT_MODE);
	}

	if(cur_state_ != -1)
		return set_state(get_state(cur_state_));

	return true;
}

bool qdInterfaceButton::save_script_body(XStream& fh,int indent) const
{
	for(int i = 0; i < num_states(); i++)
		get_state(i) -> save_script(fh,indent + 1);

	return true;
}

bool qdInterfaceButton::load_script_body(const xml::tag* p)
{
	int num_states = 0;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_INTERFACE_ELEMENT_STATE:
			num_states++;
			break;
		}
	}

	if(num_states){
		states_.reserve(num_states);
		cur_state_ = 0;
	}

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_INTERFACE_ELEMENT_STATE: {
				qdInterfaceElementState st;
				st.load_script(&*it);
				add_state(st);
			}
			break;
		}
	}

	return true;
}

bool qdInterfaceButton::quant(float dt)
{
	qdInterfaceElement::quant(dt);

	if(find_event(qdInterfaceEvent::EVENT_CLEAR_MOUSE)){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
			if(dp -> is_on_mouse(NULL))
				activate_state(1);
			else
				activate_state(0);
		}
	}

	if(cur_state_ != -1)
		get_state(cur_state_) -> quant(dt);

	return true;
}

bool qdInterfaceButton::hit_test(int x,int y) const
{
	if(cur_state_ != -1){
		if(get_state(cur_state_) -> has_contour(get_state(cur_state_) -> state_mode()))
			return get_state(cur_state_) -> hit_test(x - r().x,y - r().y,get_state(cur_state_) -> state_mode());
	}

	return qdInterfaceElement::hit_test(x,y);
}

bool qdInterfaceButton::change_state(bool direction)
{
	if(num_states()){
		if(cur_state_ != -1){
			cur_state_ += (direction) ? 1 : -1;
			if(cur_state_ < 0) cur_state_ = num_states() - 1;
			if(cur_state_ >= num_states()) cur_state_ = 0;
		}
		else
			cur_state_ = 0;

		return activate_state(cur_state_);
	}

	return false;
}

const qdInterfaceEvent* qdInterfaceButton::find_event(qdInterfaceEvent::event_t type) const
{
	for(int i = 0; i < num_states(); i++){
		const qdInterfaceElementState* p = get_state(i);
		if(const qdInterfaceEvent* ev = p -> find_event(type))
			return ev;
	}

	return NULL;
}

bool qdInterfaceButton::has_event(qdInterfaceEvent::event_t type,const char* ev_data) const
{
	for(int i = 0; i < num_states(); i++){
		const qdInterfaceElementState* p = get_state(i);
		if(p -> has_event(type,ev_data))
			return true;
	}

	return false;
}
