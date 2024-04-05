
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_interface_element_state_mode.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceElementStateMode::qdInterfaceElementStateMode() : sound_(NULL),
	animation_flags_(0),
	animation_(NULL),
	contour_(qdContour::CONTOUR_POLYGON)
{
}

qdInterfaceElementStateMode::qdInterfaceElementStateMode(const qdInterfaceElementStateMode& ev) : sound_file_(ev.sound_file_),
	sound_(ev.sound_),
	animation_file_(ev.animation_file_),
	animation_flags_(ev.animation_flags_),
	animation_(ev.animation_),
	contour_(ev.contour_)
{
}

qdInterfaceElementStateMode::~qdInterfaceElementStateMode()
{
}

qdInterfaceElementStateMode& qdInterfaceElementStateMode::operator = (const qdInterfaceElementStateMode& ev)
{
	if(this == &ev) return *this;

	sound_file_ = ev.sound_file_;
	sound_ = ev.sound_;

	animation_file_ = ev.animation_file_;
	animation_flags_ = ev.animation_flags_;
	animation_ = ev.animation_;

	contour_ = ev.contour_;

	return *this;
}

void qdInterfaceElementStateMode::set_sound_file(const char* name)
{
	if(name)
		sound_file_ = name;
	else
		sound_file_.clear();
}

void qdInterfaceElementStateMode::set_animation_file(const char* name)
{
	if(name)
		animation_file_ = name;
	else
		animation_file_.clear();
}

bool qdInterfaceElementStateMode::save_script(XStream& fh,int type_id,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < '\t';
	fh < "<state_mode type=\"" <= type_id < "\"";

	if(has_animation())
		fh < " animation=\"" < qdscr_XML_string(animation_file()) < "\"";

	if(animation_flags_)
		fh < " animation_flags=\"" <= animation_flags_ < "\"";

	if(has_sound())
		fh < " sound=\"" < qdscr_XML_string(sound_file()) < "\"";

	if(has_contour()){
		fh < ">\r\n";
		contour_.save_script(fh,indent + 1);
		for(int i = 0; i < indent; i ++) fh < '\t';
		fh < "</state_mode>\r\n";
	}
	else
		fh < "/>\r\n";

	return true;
}

bool qdInterfaceElementStateMode::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_SOUND:
			set_sound_file(it -> data());
			break;
		case QDSCR_INTERFACE_ANIMATION_FLAGS:
			xml::tag_buffer(*it) > animation_flags_;
			break;
		case QDSCR_ANIMATION:
			set_animation_file(it -> data());
			break;
		case QDSCR_CONTOUR_CIRCLE:
			contour_.set_contour_type(qdContour::CONTOUR_CIRCLE);
			contour_.load_script(&*it);
			break;
		case QDSCR_CONTOUR_RECTANGLE:
			contour_.set_contour_type(qdContour::CONTOUR_RECTANGLE);
			contour_.load_script(&*it);
			break;
		case QDSCR_CONTOUR_POLYGON:
			contour_.set_contour_type(qdContour::CONTOUR_POLYGON);
			contour_.load_script(&*it);
			break;
		}
	}

	return true;
}
