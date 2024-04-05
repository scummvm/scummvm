/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <math.h>

#include "snd_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

sndDispatcher* sndDispatcher::dispatcher_ptr_;

#ifdef _QUEST_EDITOR
sndDispatcher::SoundDisabler::SoundDisabler(){
	if(sndDispatcher* p = sndDispatcher::get_dispatcher())
	{
		soundEnabled_ = p->is_enabled();
		p -> disable();
	}
}
sndDispatcher::SoundDisabler::~SoundDisabler(){
	if(sndDispatcher* p = sndDispatcher::get_dispatcher())
		if (soundEnabled_) p ->enable();
}
#endif // _QUEST_EDITOR

sndDispatcher::sndDispatcher() : is_enabled_(true),
	is_paused_(false),
	volume_(255),
	volume_dB_(0),
	frequency_coeff_(1.0f)
{
	if(!dispatcher_ptr_) dispatcher_ptr_ = this;
}

sndDispatcher::~sndDispatcher()
{
	if(dispatcher_ptr_ == this) dispatcher_ptr_ = NULL;
}

void sndDispatcher::set_volume(unsigned int vol)
{
	volume_ = vol & 0xFF;

	volume_dB_ = convert_volume_to_dB(volume_);

	update_volume();
}

int sndDispatcher::convert_volume_to_dB(int vol)
{
	if(vol > 255) vol = 255;
	if(vol < 0) vol = 0;

	if(vol != 255){
		const int DB_MIN = -10000;
		const int DB_MAX = 0;
		const int DB_SIZE = DB_MAX - DB_MIN;

		return (DB_MIN + round(log10(9.0*log(double(vol + 1))/(log(2.0)*8) + 1.0)*DB_SIZE));
	}
	else
		return 0;
}
