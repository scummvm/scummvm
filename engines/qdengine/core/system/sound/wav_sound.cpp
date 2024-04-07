/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "wav_sound.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

wavSound::wavSound() : data_(NULL)
{
	data_length_ = 0;
	bits_per_sample_ = 0;
	channels_ = 0;
	samples_per_sec_ = 0;
}

wavSound::~wavSound()
{
	free_data();
}

bool wavSound::init(int data_len,int bits,int chn,int samples)
{
	free_data();
	
	data_length_ = data_len;
	data_ = new char[data_length_];

	channels_ = chn;
	bits_per_sample_ = bits;
	samples_per_sec_ = samples;

	return true;
}

void wavSound::free_data()
{
	if(data_){
		delete [] data_;
		data_ = NULL;
	}

	data_length_ = 0;
	bits_per_sample_ = 0;
	channels_ = 0;
	samples_per_sec_ = 0;
}
