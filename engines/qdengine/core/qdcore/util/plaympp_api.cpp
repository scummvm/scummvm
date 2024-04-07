
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "plaympp_api.h"

#include "PlayMpeg.h"
#include "PlayOgg.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

bool mpegPlayer::is_library_initialized_ = false;

mpegPlayer::mpegPlayer() : is_enabled_(true),
	volume_(255),
	cur_track_volume_(255),
	isOGG_(false)
{
	oggPlayer_ = new MpegSound;
}

mpegPlayer::~mpegPlayer()
{
	delete oggPlayer_;
}
	
bool mpegPlayer::play(const char* file, bool loop, int vol)
{
	bool last_ogg_state = isOGG_;

	isOGG_ = !stricmp(app_io::get_ext(file), ".ogg");

	if(isOGG_){
		if(!last_ogg_state)
			mpp::MpegStop();

		oggPlayer_->OpenToPlay(file,loop);

		if(!is_enabled())
			pause();

		cur_track_volume_ = vol;
		oggPlayer_->SetVolume(volume_ * cur_track_volume_ / 256);
	}
	else {
		if(last_ogg_state)
			oggPlayer_->Stop();

		mpp::MpegOpenToPlay(file, loop);

		if(!is_enabled())
			pause();

		cur_track_volume_ = vol;
		mpp::MpegSetVolume(volume_ * cur_track_volume_ / 256);
	}

	return true;
}

bool mpegPlayer::stop()
{
	if(isOGG_)
		oggPlayer_->Stop();
	else
		mpp::MpegStop();
	return true;
}

bool mpegPlayer::pause()
{
	if(isOGG_)
		oggPlayer_->Pause();
	else
		mpp::MpegPause();
	return true;
}

bool mpegPlayer::resume()
{
	if(isOGG_){
		oggPlayer_->Resume();
		oggPlayer_->SetVolume(volume_ * cur_track_volume_ / 256);
	}
	else {
		mpp::MpegResume();
		mpp::MpegSetVolume(volume_ * cur_track_volume_ / 256);
	}
	return true;
}

mpegPlayer::mpeg_status_t mpegPlayer::status() const
{
	if(isOGG_){
		switch(oggPlayer_->IsPlay()){
			case MPEG_PLAY:
				return MPEG_PLAYING;
			case MPEG_PAUSE:
				return MPEG_PAUSED;
			default:
				return MPEG_STOPPED;
		}
	}
	else {
		switch(mpp::MpegIsPlay()){
			case mpp::MPEG_PLAY:
				return MPEG_PLAYING;
			case mpp::MPEG_PAUSE:
				return MPEG_PAUSED;
			default:
				return MPEG_STOPPED;
		}
	}

	return MPEG_STOPPED;
}

void mpegPlayer::set_volume(unsigned int vol)
{
	volume_ = vol;

	if(status() != MPEG_STOPPED){
		if(isOGG_)
			oggPlayer_->SetVolume(volume_ * cur_track_volume_ / 256);
		else
			mpp::MpegSetVolume(volume_ * cur_track_volume_ / 256);
	}
}

bool mpegPlayer::init_library(void* dsound_device)
{
	if(!is_library_initialized_){
		MpegInitLibrary(dsound_device);
		mpp::MpegInitLibrary(dsound_device);

		is_library_initialized_ = true;
		return true;
	}

	return false;
}

void mpegPlayer::deinit_library()
{
	if(is_library_initialized_){
		if(instance().status() != MPEG_STOPPED)
			instance().stop();

		MpegDeinitLibrary();
		mpp::MpegDeinitLibrary();
		is_library_initialized_ = false;
	}
}

mpegPlayer& mpegPlayer::instance()
{
	static mpegPlayer player;
	return player;
}
