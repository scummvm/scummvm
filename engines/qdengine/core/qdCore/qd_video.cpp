/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "qd_video.h"
#include "qd_setup.h"

#include "qd_game_dispatcher.h"
#include "qd_file_manager.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdVideo::qdVideo() : position_(0,0)
{
}

qdVideo::qdVideo(const qdVideo& v):
	qdConditionalObject(v),
	position_(v.position_),
	file_name_(v.file_name_),
	background_(v.background_)
{
}

qdVideo::~qdVideo()
{
}

qdVideo& qdVideo::operator = (const qdVideo& v)
{
	if(this == &v) return *this;

	*static_cast<qdConditionalObject*>(this) = v;

	position_ = v.position_;
	file_name_ = v.file_name_;
	background_ = v.background_;

	return *this;
}

bool qdVideo::load_script(const xml::tag* p)
{
	load_conditions_script(p);

	Vect2s v;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_CD:
			set_CD_info(xml::tag_buffer(*it).get_uint());
			break;
		case QDSCR_FILE:
			set_file_name(it -> data());
			break;
		case QDSCR_VIDEO_POSITION:
			xml::tag_buffer(*it) > v.x > v.y;
			set_position(v);
			break;
		case QDSCR_VIDEO_BACKGROUND:
			if(strlen(it -> data()))
				set_background_file_name(it -> data());
			break;
		}
	}

	return true;
}

bool qdVideo::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";

	fh < "<video name=";

	if(name()) fh < "\"" < qdscr_XML_string(name()) < "\"";
	else fh < "\" \"";

	if(CD_info())
		fh < " cd=\"" <= CD_info() < "\"";
	
	if(flags())
		fh < " flags=\"" <= flags() < "\"";

	if(!check_flag(VID_CENTER_FLAG | VID_FULLSCREEN_FLAG) && (position_.x || position_.y))
		fh < " video_position=\"" <= position_.x < " " <= position_.y < "\"";

	fh < ">\r\n";

	if(background_.has_file()){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<video_background>" < qdscr_XML_string(background_file_name()) < "</video_background>\r\n";
	}

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<file>" < qdscr_XML_string(file_name_.c_str()) < "</file>\r\n";

	save_conditions_script(fh,indent);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</video>\r\n";

	return true;
}

bool qdVideo::draw_background()
{
	if(background_.has_file()){
		background_.load();

		grDispatcher::instance() -> Fill(0);
		int x = qdGameConfig::get_config().screen_sx() >> 1;
		int y = qdGameConfig::get_config().screen_sy() >> 1;
		background_.redraw(x,y,0);
		grDispatcher::instance() -> Flush();

		background_.free();

		return true;
	}

	grDispatcher::instance() -> Fill(0);
	grDispatcher::instance() -> Flush();

	return false;
}

qdConditionalObject::trigger_start_mode qdVideo::trigger_start()
{
	appLog::default_log() << appLog::default_log().time_string() << " Video start -> " << name() << "\r\n";

	if(qdGameDispatcher* dp = qd_get_game_dispatcher()){
		if(dp -> play_video(this))
			dp -> pause();
	}

	return qdConditionalObject::TRIGGER_START_ACTIVATE;
}

bool qdVideo::adjust_files_paths(const char* copy_dir, const char* pack_dir, bool can_overwrite)
{
	std::string copy_corr_dir = copy_dir;
	app_io::adjust_dir_end_slash(copy_corr_dir);
	std::string pack_corr_dir = pack_dir;
	app_io::adjust_dir_end_slash(pack_corr_dir);

	bool all_ok = true;
	if (file_name_.size() != 0)
		QD_ADJUST_TO_REL_FILE_MEMBER(copy_corr_dir, file_name, set_file_name, can_overwrite, all_ok);

	if (NULL != background_file_name())
		QD_ADJUST_TO_REL_FILE_MEMBER(pack_corr_dir, background_file_name, set_background_file_name, can_overwrite, all_ok);

	return all_ok;
}

bool qdVideo::get_files_list(qdFileNameList& files_to_copy,qdFileNameList& files_to_pack) const
{
	if(!file_name_.empty())
		files_to_copy.push_back(file_name_);

	if(background_file_name())
		files_to_pack.push_back(background_file_name());

	return true;
}