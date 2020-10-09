/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#if defined (SDL_BACKEND)
#include <SDL.h>
#endif

#include "engines/icb/sound/direct_sound.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/sound/speech_manager.h"
#include "engines/icb/sound/fx_manager.h"
#include "engines/icb/icb.h"

#include "audio/mixer.h"

namespace ICB {

MusicManager *g_theMusicManager = NULL;
SpeechManager *g_theSpeechManager = NULL;
FxManager *g_theFxManager = NULL;
bool8 g_TimerOn = TRUE8;

#if defined (SDL_BACKEND)
SDL_TimerID g_timer_id = 0;

Uint32 SoundEngineTimer(Uint32 interval, void *) {
	SDL_Event event;
	SDL_UserEvent ue;

	ue.type = SDL_USEREVENT;
	ue.code = 0;
	ue.data1 = NULL;
	ue.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = ue;

	SDL_PushEvent(&event);

	return interval;
}
#endif

bool8 Init_Sound_Engine() {
	// DO THIS FIRST INCASE REST OF INIT FAILS
	// Create a timer to poll the sound engine at 100 millisecond intervals
#if defined (SDL_BACKEND)
	g_timer_id = SDL_AddTimer(100, SoundEngineTimer, NULL);
#endif

	// Initalize the other sections (ie music, fx and speech)
	g_theMusicManager = new MusicManager();
	g_theSpeechManager = new SpeechManager();
	g_theFxManager = new FxManager;

	return TRUE8;
}

bool8 Close_Sound_Engine() {
#if defined (SDL_BACKEND)
	// Kill the sound engine timer
	SDL_RemoveTimer(g_timer_id);
#endif

	// Destroy the fx manager
	if (g_theFxManager) {
		delete g_theFxManager;
		g_theFxManager = NULL;
	}

	// Destroy the music manager
	if (g_theMusicManager) {
		delete g_theMusicManager;
		g_theMusicManager = NULL;
	}

	// Destroy the speech manager
	if (g_theSpeechManager) {
		delete g_theSpeechManager;
		g_theSpeechManager = NULL;
	}
	g_icb->_mixer->stopAll();

	return TRUE8;
}

bool8 Poll_Sound_Engine() {
	// Refresh the music streams
	if (g_theMusicManager) {
		if (!g_theMusicManager->UpdateMusic()) {
			// Shutdown if we fail - not elegant but safe
			Close_Sound_Engine();
		}
	}

	// Refresh the speech streams
	if (g_theSpeechManager) {
		if (!g_theSpeechManager->UpdateSpeech()) {
			// Shutdown if we fail - not elegant but safe
			Close_Sound_Engine();
		}
	}

	// Poll the sound fx manager
	if (g_theFxManager) {
		if (!g_theFxManager->Poll()) {
			// Shutdown if we fail - not elegant but safe
			Close_Sound_Engine();
		}
	}

	return TRUE8;
}

} // End of namespace ICB
