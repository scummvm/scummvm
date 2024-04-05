/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_core.h"
#include "gr_dispatcher.h"
#include "ds_snd_dispatcher.h"

#include "plaympp_api.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

static bool operator == (const dsSound& snd0,const sndSound& snd1)
{
	return snd0.sound() == snd1.sound();
}

static bool operator == (const dsSound& snd,const sndHandle& h)
{
	return snd.handle() == &h;
}

ds_sndDispatcher::ds_sndDispatcher() : sound_device_(NULL)
{
	HRESULT res = DirectSoundCreate(NULL,&sound_device_,NULL);
  
	if(FAILED(res) || sound_device_ == NULL)
		return;

	HWND hWnd = static_cast<HWND>(appGetHandle());

	grDispatcher* gp = grDispatcher::instance();

	if(gp && gp -> is_in_fullscreen_mode()){
		res = sound_device_ -> SetCooperativeLevel(hWnd,DSSCL_EXCLUSIVE);
		if(FAILED(res))
			sound_device_ -> SetCooperativeLevel(hWnd,DSSCL_PRIORITY);
	}
	else 
		sound_device_ -> SetCooperativeLevel(hWnd,DSSCL_PRIORITY);

	mpegPlayer::init_library(sound_device_);
}

ds_sndDispatcher::~ds_sndDispatcher()
{
	sounds_.clear();

	mpegPlayer::deinit_library();

	if(sound_device_)
		sound_device_-> Release();
}

void ds_sndDispatcher::quant()
{
	sounds_.remove_if(std::mem_fun_ref(dsSound::is_stopped));
}

bool ds_sndDispatcher::play_sound(const sndSound* snd,bool loop,float start_position,int vol)
{
	if(!sound_device_) return false;

	if(is_enabled()){
		sounds_.push_back(dsSound(*snd,sound_device_));
		dsSound& p = sounds_.back();

		if(loop)
			p.toggle_looping();

		int snd_volume = (vol == 255) ? volume_dB() : convert_volume_to_dB((volume() * vol) >> 8);

		p.create_sound_buffer();
		p.set_volume(snd_volume);
		p.change_frequency(frequency_coeff());
		p.set_position(start_position);

		if(!is_paused()){
			if(!p.play()) return false;
		}
		else
			p.pause();
	}

	return true;
}

bool ds_sndDispatcher::stop_sound(const sndSound* snd)
{
	if(!sound_device_) return false;

	sound_list_t::iterator it = std::find(sounds_.begin(),sounds_.end(),*snd);

	if(it != sounds_.end()){
		it -> stop();
		sounds_.erase(it);

		return true;
	}

	return false;
}

bool ds_sndDispatcher::stop_sound(const sndHandle* handle)
{
	if(!sound_device_) return false;

	sound_list_t::iterator it = std::find(sounds_.begin(),sounds_.end(),*handle);

	if(it != sounds_.end()){
		it -> stop();
		sounds_.erase(it);

		return true;
	}

	return false;
}

sndSound::status_t ds_sndDispatcher::sound_status(const sndHandle* handle) const
{
	sound_list_t::const_iterator it = std::find(sounds_.begin(),sounds_.end(),*handle);

	if(it != sounds_.end()){
		if(is_paused())
			return sndSound::SOUND_PAUSED;

		return sndSound::SOUND_PLAYING;
	}

	return sndSound::SOUND_STOPPED;
}

sndSound::status_t ds_sndDispatcher::sound_status(const sndSound* snd) const
{
	sound_list_t::const_iterator it = std::find(sounds_.begin(),sounds_.end(),*snd);

	if(it != sounds_.end())
		return it -> status();

	return sndSound::SOUND_STOPPED;
}

bool ds_sndDispatcher::update_volume()
{
	for(sound_list_t::iterator it = sounds_.begin(); it != sounds_.end(); ++it)
		it -> set_volume(volume_dB());

	return true;
}

bool ds_sndDispatcher::update_frequency()
{
	for(sound_list_t::iterator it = sounds_.begin(); it != sounds_.end(); ++it)
		it -> change_frequency(frequency_coeff());

	return true;
}

void ds_sndDispatcher::stop_sounds()
{
	for(sound_list_t::iterator it = sounds_.begin(); it != sounds_.end(); ++it)
		it -> stop();

	sounds_.clear();
}

bool ds_sndDispatcher::set_sound_frequency(const sndHandle* snd,float coeff)
{
	sound_list_t::iterator it = std::find(sounds_.begin(),sounds_.end(),*snd);

	if(it != sounds_.end()){
		it -> change_frequency(frequency_coeff() * coeff);
		return true;
	}

	return false;
}

float ds_sndDispatcher::sound_position(const sndHandle* snd) const
{
	sound_list_t::const_iterator it = std::find(sounds_.begin(),sounds_.end(),*snd);

	if(it != sounds_.end())
		return it -> position();

	return 0.0f;
}

void ds_sndDispatcher::pause_sounds()
{
	for(sound_list_t::iterator it = sounds_.begin(); it != sounds_.end(); ++it)
		it -> pause();
}

void ds_sndDispatcher::resume_sounds()
{
	for(sound_list_t::iterator it = sounds_.begin(); it != sounds_.end(); ++it){
		if(it -> is_paused())
			it -> resume();
	}
}

