/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"

#include "qdscr_parser.h"
#include "qd_game_end.h"
#include "qd_game_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdGameEnd::qdGameEnd()
{
}

qdGameEnd::qdGameEnd(const qdGameEnd& end) : qdConditionalObject(end),
	interface_screen_(end.interface_screen_)
{
}

qdGameEnd::~qdGameEnd()
{
}

qdGameEnd& qdGameEnd::operator = (const qdGameEnd& end)
{
	if(this == &end) return *this;

	*static_cast<qdConditionalObject*>(this) = end;

	interface_screen_ = end.interface_screen_;

	return *this;
}

qdConditionalObject::trigger_start_mode qdGameEnd::trigger_start()
{
	if(qdGameDispatcher* p = qdGameDispatcher::get_dispatcher()){
		p -> set_game_end(this);
		return qdConditionalObject::TRIGGER_START_ACTIVATE;
	}

	return qdConditionalObject::TRIGGER_START_FAILED;
}

bool qdGameEnd::load_script(const xml::tag* p)
{
	load_conditions_script(p);

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_GAME_END_SCREEN:
			set_interface_screen(it -> data());
			break;
		}
	}

	return true;
}

bool qdGameEnd::save_script(class XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "<game_end";

	if(name())
		fh < " name=\"" < qdscr_XML_string(name()) < "\"";

	if(flags())
		fh < " flags=\"" <= flags() < "\"";

	if(!interface_screen_.empty())
		fh < " end_screen=\"" < qdscr_XML_string(interface_screen_.c_str()) < "\"";

	if(has_conditions()){
		fh < ">\r\n";

		save_conditions_script(fh,indent);
		for(int i = 0; i < indent; i++) fh < "\t";
		fh < "</game_end>\r\n";
	}
	else
		fh < "/>\r\n";

	return true;
}
