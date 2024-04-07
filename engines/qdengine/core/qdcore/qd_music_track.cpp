/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_game_dispatcher.h"
#include "qd_music_track.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdMusicTrack::qdMusicTrack() : volume_(256)
{
	toggle_cycle(true);
}

qdMusicTrack::qdMusicTrack(const qdMusicTrack& trk) : qdConditionalObject(trk),
	file_name_(trk.file_name_),
	volume_(trk.volume_)
{
}

qdMusicTrack::~qdMusicTrack()
{
}

qdMusicTrack& qdMusicTrack::operator = (const qdMusicTrack& trk)
{
	if(this == &trk) return *this;

	*static_cast<qdConditionalObject*>(this) = trk;

	file_name_ = trk.file_name_;
	volume_ = trk.volume_;

	return *this;
}

bool qdMusicTrack::load_script(const xml::tag* p)
{
	load_conditions_script(p);

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_FLAG:
			clear_flags();
            set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_MUSIC_TRACK_CYCLED:
			if(xml::tag_buffer(*it).get_int())
				toggle_cycle(true);
			else
				toggle_cycle(false);
			break;
		case QDSCR_FILE:
			set_file_name(it -> data());
			break;
		case QDSCR_SOUND_VOLUME:
			set_volume(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}

bool qdMusicTrack::save_script(class XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<music_track";

	if(name())
		fh < " name=\"" < qdscr_XML_string(name()) < "\"";

	fh < " flags=\"" <= flags() < "\"";

	if(!file_name_.empty())
		fh < " file=\"" < qdscr_XML_string(file_name_.c_str()) < "\"";

	if(is_cycled())
		fh < " cycled=\"1\"";

	if(volume_ != 256)
		fh < " volume=\"" <= volume_ < "\"";

	if(has_conditions()){
		fh < ">\r\n";
		save_conditions_script(fh,indent);
		for(int i = 0; i < indent; i ++) fh < "\t";
		fh < "</music_track>";
	}
	else 
		fh < "/>\r\n";

	return true;
}

qdConditionalObject::trigger_start_mode qdMusicTrack::trigger_start()
{
	if(qdGameDispatcher* dp = qd_get_game_dispatcher()){
		dp -> play_music_track(this);
		return qdConditionalObject::TRIGGER_START_ACTIVATE;
	}

	return qdConditionalObject::TRIGGER_START_FAILED;
}
