/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_error_handler.h"

#ifndef __QD_SYSLIB__
#include "xml_tag_buffer.h"
#include "qdscr_parser.h"
#endif

#include "qd_sound.h"
#include "snd_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#ifdef __QD_DEBUG_ENABLE__
//#define __QD_SOUND_LOG__
#endif

qdSound::qdSound() : volume_(255)
{
}

qdSound::~qdSound()
{
#ifndef __QD_SYSLIB__
	sound_.free_data();
#endif
}

bool qdSound::free_resource()
{
	toggle_resource_status(false);

#ifndef __QD_SYSLIB__
	sound_.free_data();
#endif
	return true;
}

bool qdSound::load_resource()
{
	if(file_name_.empty()) return false;

	toggle_resource_status(true);

#ifndef __QD_SYSLIB__
	return wav_file_load(file_name_.c_str(),&sound_);
#else
	return true;
#endif
}

void qdSound::load_script(const xml::tag* p)
{
#ifndef __QD_SYSLIB__
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_FILE:
			set_file_name(it -> data());
			break;
		case QDSCR_SOUND_VOLUME:
			xml::tag_buffer(*it) > volume_;
			break;
		}
	}
#endif
}

bool qdSound::save_script(XStream& fh,int indent) const
{
#ifndef __QD_SYSLIB__
	for(int i = 0; i < indent; i ++) fh < "\t";

	fh < "<sound name=";

	if(name())
		fh < "\"" < qdscr_XML_string(name()) < "\"";
	else
		fh < "\" \"";

	if(volume_ != 255)
		fh < " volume=\"" <= volume_ < "\"";

	if(!file_name_.empty())
		fh < " file=\"" < qdscr_XML_string(file_name_.c_str()) < "\"";

	fh < "/>\r\n";
#endif
	return true;
}

bool qdSound::play(const qdSoundHandle* handle,bool loop,float start_position) const
{
#ifdef __QD_SOUND_LOG__
	if(handle && handle -> owner()){
		appLog::default_log() << appLog::default_log().time_string() << " sound start " << this << " owner: " << handle -> owner();

		if(loop)
			appLog::default_log() << " cycled";

		appLog::default_log() << "\r\n";
	}
#endif

#ifndef __QD_SYSLIB__
	if(sndDispatcher* p = sndDispatcher::get_dispatcher())
		return p -> play_sound(&sndSound(&sound_,handle),loop,start_position,volume_);
#endif

	return false;
}

bool qdSound::stop(const qdSoundHandle* handle) const
{
#ifdef __QD_SOUND_LOG__
	if(handle && handle -> owner())
		appLog::default_log() << appLog::default_log().time_string() << " sound stop " << this << " owner: " << handle -> owner() << "\r\n";
#endif

#ifndef __QD_SYSLIB__
	if(sndDispatcher* p = sndDispatcher::get_dispatcher()){
		if(!handle)
			return p -> stop_sound(&sndSound(&sound_));
		else
			return p -> stop_sound(handle);
	}
#endif

	return false;
}

float qdSound::position(const qdSoundHandle* handle) const
{
	if(!handle) return 0.0f;

#ifndef __QD_SYSLIB__
	if(sndDispatcher* p = sndDispatcher::get_dispatcher())
		return p -> sound_position(handle);
#endif

	return 0.0f;
}

bool qdSound::is_stopped(const qdSoundHandle* handle) const
{
#ifndef __QD_SYSLIB__
	if(sndDispatcher* p = sndDispatcher::get_dispatcher()){
		if(handle)
			return (p -> sound_status(handle) == sndSound::SOUND_STOPPED);
		else
			return (p -> sound_status(&sndSound(&sound_)) == sndSound::SOUND_STOPPED);
	}
#endif

	return true;
}

bool qdSound::set_frequency(const qdSoundHandle* handle,float frequency_coeff) const
{
#ifndef __QD_SYSLIB__
	if(sndDispatcher* p = sndDispatcher::get_dispatcher())
		return p -> set_sound_frequency(handle,frequency_coeff);
#endif
	return false;
}
