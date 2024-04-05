#include "qd_precomp.h"

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "gr_dispatcher.h"

#include "qd_game_dispatcher.h"
#include "qd_counter.h"
#include "qd_interface_counter.h"

qdInterfaceCounter::qdInterfaceCounter() : counter_(0)
{
	digits_ = 4;

	lastValue_ = -1;

	textFormat_.toggle_global_depend(false);
}

qdInterfaceCounter::qdInterfaceCounter(const qdInterfaceCounter& counter) : qdInterfaceElement(counter),
	textFormat_(counter.textFormat_),
	digits_(counter.digits_),
	counterName_(counter.counterName_),
	counter_(counter.counter_)
{
	lastValue_ = -1;
	textFormat_.toggle_global_depend(false);
}

qdInterfaceCounter::~qdInterfaceCounter()
{
}

qdInterfaceCounter& qdInterfaceCounter::operator = (const qdInterfaceCounter& counter)
{
	if(this == &counter) return *this;

	*static_cast<qdInterfaceElement*>(this) = counter;

	textFormat_ = counter.textFormat_;

	digits_ = counter.digits_;

	counterName_ = counter.counterName_;
	counter_ = counter.counter_;

	lastValue_ = -1;

	return *this;
}

bool qdInterfaceCounter::save_script_body(XStream& fh,int indent) const
{
	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<counter_name>" < qdscr_XML_string(counterName()) < "</counter_name>\r\n";

	textFormat_.save_script(fh, indent + 1);

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<digits>" <= digits_ < "</digits>\r\n";

	return true;
}

bool qdInterfaceCounter::load_script_body(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_COUNTER_NAME:
				setCounterName(it->data());
				break;
			case QDSCR_SCREEN_TEXT_FORMAT:
				textFormat_.load_script(&*it);
				break;
			case QDSCR_COUNTER_DIGITS:
				digits_ = xml::tag_buffer(*it).get_int();
				break;
		}
	}

	return true;
}

const char* qdInterfaceCounter::data() const
{
	static XBuffer str;
	str.init();

	int val = 0;
	if(counter_)
		val = counter_->value();

	if(val < 0)
		str < "-";

	int delta = 10;

	for(int i = 1; i < digits_; i++){
		if(abs(val) < abs(delta))
			str < "0";

		delta *= 10;
	}

	str <= abs(val);

	return str;
}

bool qdInterfaceCounter::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	return true;
}

bool qdInterfaceCounter::keyboard_handler(int vkey)
{
	return false;
}

grScreenRegion qdInterfaceCounter::screen_region() const
{
	Vect2i sz = Vect2i(size_x(), size_y());
	Vect2i pos = r();

	return grScreenRegion(pos.x, pos.y, sz.x, sz.y);
}

bool qdInterfaceCounter::init(bool is_game_active)
{
	counter_ = qdGameDispatcher::get_dispatcher()->get_counter(counterName());
	return true;
}

bool qdInterfaceCounter::redraw() const
{
	qdInterfaceElement::redraw();

	if(counter_){
		const grFont* font = qdGameDispatcher::get_dispatcher()->
							find_font(textFormat_.font_type());

		Vect2i sz = Vect2i(size_x(), size_y());
		Vect2i pos = r() - sz/2;
		grDispatcher::instance() -> DrawAlignedText(pos.x, pos.y, sz.x, sz.y,
			textFormat_.color(), data(), GR_ALIGN_LEFT, 0, 0, font);
	}

	return true;
}

bool qdInterfaceCounter::need_redraw() const
{
	if(qdInterfaceElement::need_redraw())
		return true;

	if(counter_)
		return lastValue_ != counter_->value();

	return false;
}

bool qdInterfaceCounter::post_redraw()
{
	qdInterfaceElement::post_redraw();

	if(counter_)
		lastValue_ = counter_->value();

	return true;
}

int qdInterfaceCounter::size_x() const
{
	const grFont* font = qdGameDispatcher::get_dispatcher()->
		                   find_font(textFormat_.font_type());
	return grDispatcher::instance() -> TextWidth(data(),0,font);
}

int qdInterfaceCounter::size_y() const
{
	const grFont* font = qdGameDispatcher::get_dispatcher()->
		                   find_font(textFormat_.font_type());
	return grDispatcher::instance() -> TextHeight(data(),0,font);
}

void qdInterfaceCounter::setCounter(const qdCounter* counter)
{
	counter_ = counter;
	counterName_ = (counter) ? counter->name() : "";
}

const char* qdInterfaceCounter::counterName() const
{
	return counterName_.c_str();
}

void qdInterfaceCounter::setCounterName(const char* name)
{
	counterName_ = name;
}

