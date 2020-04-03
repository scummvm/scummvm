/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/filesys/u4file.h"

namespace Ultima {
namespace Ultima4 {

void Music::create_sys() {
#ifdef TODO
	/*
	 * initialize sound subsystem
	 */
	TRACE_LOCAL(*logger, "Initializing SDL sound subsystem");

	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_S16LSB; /* 16-bit stereo */
	int audio_channels = 2;
	int audio_buffers = 1024;

	if (u4_SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		errorWarning("unable to init SDL audio subsystem: %s",
		             SDL_GetError());
		this->functional = false;
		return;
	}

	TRACE_LOCAL(*logger, "Opening audio");

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels,
	                  audio_buffers)) {
		fprintf(stderr, "Unable to open audio!\n");
		this->functional = false;
		return;
	}
	this->functional = true;
	TRACE_LOCAL(*logger, "Allocating channels");

	Mix_AllocateChannels(16);
#endif
}

void Music::destroy_sys() {
#ifdef TODO
	if (playing) {
		TRACE_LOCAL(*logger, "Stopping currently playing music");
		Mix_FreeMusic(playing);
		playing = NULL;
	}

	TRACE_LOCAL(*logger, "Closing audio");
	Mix_CloseAudio();

	TRACE_LOCAL(*logger, "Quitting SDL audio subsystem");
	u4_SDL_QuitSubSystem(SDL_INIT_AUDIO);
#endif
}

bool Music::load_sys(const Common::String &pathname) {
#ifdef TODO
	if (playing) {
		Mix_FreeMusic(playing);
		playing = NULL;
	}

	playing = Mix_LoadMUS(pathname.c_str());
	if (!playing) {
		errorWarning("unable to load music file %s: %s", pathname.c_str(),
		             Mix_GetError());
		return false;
	}
#endif
	return true;
}

void Music::playMid(Type music) {
#ifdef TODO
	if (!functional || !on)
		return;

	/* loaded a new piece of music */
	if (load(music)) {
		Mix_PlayMusic(playing, NLOOPS);
		//Mix_SetMusicPosition(0.0);  //Could be useful if music was stored on different 'it/mod' patterns
	}
#endif
}

void Music::stopMid() {
#ifdef TODO
	Mix_HaltMusic();
#endif
}

void Music::setSoundVolume_sys(int volume) {
#ifdef TODO
	/**
	 * Use Channel 1 for sound effects
	 */
	Mix_Volume(1, int((float)MIX_MAX_VOLUME / MAX_VOLUME * volume));
#endif
}

bool Music::isPlaying_sys() {
#ifdef TODO
	return Mix_PlayingMusic();
#else
	return false;
#endif
} /**< Returns true if the mixer is playing any audio */

void Music::setMusicVolume_sys(int volume) {
#ifdef TODO
	Mix_VolumeMusic(int((float)MIX_MAX_VOLUME / MAX_VOLUME * volume));
#endif
}

void Music::fadeIn_sys(int msecs, bool loadFromMap) {
#ifdef TODO
	if (Mix_FadeInMusic(playing, NLOOPS, msecs) == -1)
		errorWarning("Mix_FadeInMusic: %s\n", Mix_GetError());
#endif
}

void Music::fadeOut_sys(int msecs) {
#ifdef TODO
	if (Mix_FadeOutMusic(msecs) == -1)
		errorWarning("Mix_FadeOutMusic: %s\n", Mix_GetError());
#endif
}

} // End of namespace Ultima4
} // End of namespace Ultima
