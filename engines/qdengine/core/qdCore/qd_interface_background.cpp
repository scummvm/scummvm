/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qdscr_parser.h"
#include "xml_tag.h"

#include "qd_interface_background.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceBackground::qdInterfaceBackground()
{
	state_.set_owner(this);
}

qdInterfaceBackground::qdInterfaceBackground(const qdInterfaceBackground& bk) : qdInterfaceElement(bk)
{
	state_.set_owner(this);
	state_ = bk.state_;
}

qdInterfaceBackground::~qdInterfaceBackground()
{
	state_.unregister_resources();
}

qdInterfaceBackground& qdInterfaceBackground::operator = (const qdInterfaceBackground& bk)
{
	if(this == &bk) return *this;

	*static_cast<qdInterfaceElement*>(this) = bk;

	state_ = bk.state_;

	return *this;
}

bool qdInterfaceBackground::save_script_body(XStream& fh,int indent) const
{
	if(!state_.save_script(fh,indent)) return false;

	return true;
}

bool qdInterfaceBackground::load_script_body(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_INTERFACE_ELEMENT_STATE:
				if(!state_.load_script(&*it)) return false;
				break;
		}
	}

	return true;
}

bool qdInterfaceBackground::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	return true;
}

bool qdInterfaceBackground::keyboard_handler(int vkey)
{
	return false;
}

bool qdInterfaceBackground::init(bool is_game_active)
{
	return set_state(&state_);
}
