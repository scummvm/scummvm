/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "SymbianOS.h"

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Fullscreen", GFX_NORMAL},
	{0, 0, 0}
};

OSystem *OSystem_SymbianOS_create() {
	return new OSystem_SDL_Symbian();
}


#include "config-manager.h"

extern Common::ConfigManager *g_config;

OSystem_SDL_Symbian::OSystem_SDL_Symbian() :_channels(0),_stereo_mix_buffer(0){
	ConfMan.set("FM_high_quality", false);
	ConfMan.set("FM_medium_quality", true);
//	ConfMan.set("joystick_num",0); // S60 should have joystick_num set to 0 in the ini file
	ConfMan.flushToDisk();
}

OSystem_SDL_Symbian::~OSystem_SDL_Symbian() {
	delete []_stereo_mix_buffer;
}

int OSystem_SDL_Symbian::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

const OSystem::GraphicsMode *OSystem_SDL_Symbian::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

// make sure we always go to normal, even if the string might be set wrong!
bool OSystem_SDL_Symbian::setGraphicsMode(const char *name) {
	// let parent OSystem_SDL handle it
	return OSystem_SDL::setGraphicsMode(getDefaultGraphicsMode());
}

/*
 * SumthinWicked says: the stuff below is copied from common/scaler.cpp, 
 * so we can skip compiling the scalers. ESDL still needs 1x and the scaler
 * architecture because we inherit from OSystem_SDL.
 */
int gBitFormat = 565;
void InitScalers(uint32 BitFormat) {} // called by OSystem_SDL functions, not relevant for ESDL
	
/** 
 * Trivial 'scaler' - in fact it doesn't do any scaling but just copies the
 * source to the destination.
 */
void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	while (height--) {
		memcpy(dstPtr, srcPtr, 2 * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

bool OSystem_SDL_Symbian::setSoundCallback(SoundProc proc, void *param) {

	// First save the proc and param
	_sound_proc_param = param; 
	_sound_proc = proc;
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	memset(&desired, 0, sizeof(desired));

	_samplesPerSec = 0;

	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");

	if (_samplesPerSec <= 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	// Originally, we always used 2048 samples. This loop will produce the
	// same result at 22050 Hz, and should hopefully produce something
	// sensible for other frequencies. Note that it must be a power of two.

	uint32 samples = 0x8000;

	for (;;) {
		if ((1000 * samples) / _samplesPerSec < 100)
			break;
		samples >>= 1;
	}

	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = (uint16)samples;
#ifdef S60
	desired.callback = symbianMixCallback;
	desired.userdata = this;
#else
	desired.callback = proc;
	desired.userdata = param;
#endif
	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		return false;
	}
	// Note: This should be the obtained output rate, but it seems that at
	// least on some platforms SDL will lie and claim it did get the rate
	// even if it didn't. Probably only happens for "weird" rates, though.
	_samplesPerSec = obtained.freq;
	_channels = obtained.channels;

	// Need to create mixbuffer for stereo mix to downmix
	if(_channels != 2)
	{
		_stereo_mix_buffer = new byte [obtained.size*2];//*2 for stereo values
	}

	SDL_PauseAudio(0);
	return true;
}

/**
 * The mixer callback function, passed on to OSystem::setSoundCallback().
 * This simply calls the mix() method.
 */
void OSystem_SDL_Symbian::symbianMixCallback(void *s, byte *samples, int len){
	static_cast <OSystem_SDL_Symbian*>(s)->symbianMix(samples,len);
}


/**
 * Actual mixing implementation
 */
void OSystem_SDL_Symbian::symbianMix(byte *samples, int len){


	// If not stereo then we need to downmix
	if(_channels != 2){
	  	_sound_proc(_sound_proc_param,_stereo_mix_buffer,len*2);
		int16* bitmixDst=(int16*)samples;
		int16* bitmixSrc=(int16*)_stereo_mix_buffer;

		for(int loop=len/2;loop>=0;loop--)
		{
			*bitmixDst=(*bitmixSrc+*(bitmixSrc+1))>>1;
			bitmixDst++;
			bitmixSrc+=2;
		}
	}
	else
		_sound_proc(_sound_proc_param,samples,len);

}

/*
// probably don't need this anymore: will remove later
#define EMPTY_SCALER_IMPLEMENTATION(x)	\
	void x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, \
					uint32 dstPitch, int width, int height) {}

//EMPTY_SCALER_IMPLEMENTATION(_2xSaI);
//EMPTY_SCALER_IMPLEMENTATION(Super2xSaI);
//EMPTY_SCALER_IMPLEMENTATION(SuperEagle);
EMPTY_SCALER_IMPLEMENTATION(AdvMame2x);
EMPTY_SCALER_IMPLEMENTATION(AdvMame3x);
//EMPTY_SCALER_IMPLEMENTATION(Normal1x);
//EMPTY_SCALER_IMPLEMENTATION(Normal2x);
//EMPTY_SCALER_IMPLEMENTATION(Normal3x);
EMPTY_SCALER_IMPLEMENTATION(Normal1o5x);
//EMPTY_SCALER_IMPLEMENTATION(TV2x);
//EMPTY_SCALER_IMPLEMENTATION(DotMatrix);
//EMPTY_SCALER_IMPLEMENTATION(HQ2x);
//EMPTY_SCALER_IMPLEMENTATION(HQ3x);
*/