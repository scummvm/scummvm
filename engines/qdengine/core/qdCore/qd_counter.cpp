/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_counter.h"
#include "qd_game_object_state.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdCounterElement::qdCounterElement() : state_(NULL),
	last_state_status_(false),
	increment_value_(true)
{
}

qdCounterElement::~qdCounterElement()
{
}

qdCounterElement::qdCounterElement(const qdGameObjectState* p,bool inc_value) : state_(p),
	state_reference_(p),
	last_state_status_(false),
	increment_value_(inc_value)
{
}

bool qdCounterElement::init()
{
//	if(!state_){
		state_ = dynamic_cast<const qdGameObjectState*>(state_reference_.object());
		if(!state_){
			appLog::default_log() << "qdCounterElement::init() failed\r\n";
			return false;
		}
//	}

	last_state_status_ = false;

	return true;
}

bool qdCounterElement::quant()
{
	if(state_){
		bool result = false;

		bool status = state_ -> is_active();
		if(status && !last_state_status_)
			result = true;

		last_state_status_ = status;

		return result;
	}

	return false;
}

bool qdCounterElement::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		xml::tag_buffer buf(*it);
		switch(it -> ID()){
		case QDSCR_NAMED_OBJECT:
			state_reference_.load_script(&*it);
			break;
		case QDSCR_COUNTER_INC_VALUE:
			increment_value_ = (xml::tag_buffer(*it).get_int()) ? true : false;
			break;
		}
	}

	return true;
}

bool qdCounterElement::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "<counter_element";

	if(!increment_value_)
		fh < " inc_value=\"0\"";
	
	fh < ">\r\n";

	if(state_){
		qdNamedObjectReference ref(state_);
		ref.save_script(fh,indent + 1);
	}

	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "</counter_element>\r\n";

	return true;
}

bool qdCounterElement::load_data(qdSaveStream& fh,int save_version)
{
	char v;
	fh > v;
	last_state_status_ = v;
	return true;
}

bool qdCounterElement::save_data(qdSaveStream& fh) const
{
	char v = last_state_status_;
	fh < v;
	return true;
}

qdCounter::qdCounter() : value_(0),
	value_limit_(0)
{
}

qdCounter::~qdCounter()
{
}

void qdCounter::set_value(int value)
{
	value_ = value;

	if(value_limit_ > 0 && value_ >= value_limit_)
		value_ = 0;

	if(check_flag(POSITIVE_VALUE) && value_ < 0)
		value_ = 0;
}

void qdCounter::add_value(int value_delta)
{
	value_ += value_delta;

	if(value_limit_ > 0 && value_ >= value_limit_)
		value_ = 0;

	if(check_flag(POSITIVE_VALUE) && value_ < 0)
		value_ = 0;
}

bool qdCounter::add_element(const qdGameObjectState* p,bool inc_value)
{
	element_container_t::const_iterator it = std::find(elements_.begin(),elements_.end(),p);
	if(it != elements_.end())
		return false;

	elements_.push_back(qdCounterElement(p,inc_value));
	return true;
}

bool qdCounter::remove_element(const qdGameObjectState* p)
{
	element_container_t::iterator it = std::find(elements_.begin(),elements_.end(),p);
	if(it != elements_.end()){
		elements_.erase(it);
		return true;
	}

	return false;
}

bool qdCounter::remove_element(int idx)
{
	assert(idx >= 0 && idx < elements_.size());

	elements_.erase(elements_.begin() + idx);
	return true;
}

#ifdef _QUEST_EDITOR
void qdCounter::remove_all_elements()
{
	elements_.clear();
}
#endif // _QUEST_EDITOR

void qdCounter::quant()
{
	int value_change = 0;
	for(element_container_t::iterator it = elements_.begin(); it != elements_.end(); ++it){
		if(it -> quant()){
			if(it -> increment_value())
				value_change++;
			else
				value_change--;
		}
	}

	value_ += value_change;

	if(value_limit_ > 0 && value_ >= value_limit_)
		value_ = 0;

	if(check_flag(POSITIVE_VALUE) && value_ < 0)
		value_ = 0;
}

bool qdCounter::load_script(const xml::tag* p)
{
	int num_elements = 0;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		xml::tag_buffer buf(*it);
		switch(it -> ID()){
		case QDSCR_COUNTER_ELEMENT:
			num_elements++;
			break;
		}
	}

	elements_.reserve(num_elements);

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		xml::tag_buffer buf(*it);
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_COUNTER_ELEMENT: {
				qdCounterElement el;
				el.load_script(&*it);
				elements_.push_back(el);
			}
			break;
		case QDSCR_COUNTER_LIMIT:
			xml::tag_buffer(*it) > value_limit_;
			break;
		}
	}

	return true;
}

bool qdCounter::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "<counter";

	fh < " name=\"" < qdscr_XML_string(name()) < "\"";

	if(value_limit_)
		fh < " limit=\"" <= value_limit_ < "\"";

	if(flags())
		fh < " flags=\"" <= flags() < "\"";

	fh < ">\r\n";

	for(element_container_t::const_iterator it = elements_.begin(); it != elements_.end(); ++it)
		it -> save_script(fh,indent + 1);

	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "</counter>\r\n";
	return true;
}

bool qdCounter::load_data(qdSaveStream& fh,int save_version)
{
	int sz;
	fh > value_ > sz;

	if(sz != elements_.size()) return false;

	for(element_container_t::iterator it = elements_.begin(); it != elements_.end(); ++it)
		it -> load_data(fh,save_version);

	return true;
}

bool qdCounter::save_data(qdSaveStream& fh) const
{
	fh < value_ < elements_.size();

	for(element_container_t::const_iterator it = elements_.begin(); it != elements_.end(); ++it)
		it -> save_data(fh);

	return true;
}

void qdCounter::init()
{
	for(element_container_t::iterator it = elements_.begin(); it != elements_.end(); ++it)
		it -> init();

	value_ = 0;
}

