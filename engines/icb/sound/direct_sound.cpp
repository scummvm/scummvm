/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/icb/sound/direct_sound.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/sound/speech_manager.h"
#include "engines/icb/sound/fx_manager.h"
#include "engines/icb/sound.h"
#include "engines/icb/icb.h"

#include "audio/mixer.h"

#include "common/system.h"
#include "common/timer.h"

namespace ICB {

MusicManager *g_theMusicManager = nullptr;
SpeechManager *g_theSpeechManager = nullptr;
FxManager *g_theFxManager = nullptr;
bool8 g_TimerOn = TRUE8;

Common::TimerManager::TimerProc g_timer_id;

void SoundEngineTimer(void *ignored) {
	if (g_TimerOn) {
		UpdateSounds10Hz();
		Poll_Sound_Engine();
	}
}

bool8 Init_Sound_Engine() {
	// Create a timer to poll the sound engine at 100 millisecond intervals
	g_system->getTimerManager()->installTimerProc(SoundEngineTimer, 100 * 1000, nullptr, "SoundEngineTimer");

	// Initalize the other sections (ie music, fx and speech)
	g_theMusicManager = new MusicManager();
	g_theSpeechManager = new SpeechManager();
	g_theFxManager = new FxManager;

	return TRUE8;
}

bool8 Close_Sound_Engine() {
	// Kill the sound engine timer
	g_system->getTimerManager()->removeTimerProc(g_timer_id);

	// Destroy the fx manager
	if (g_theFxManager) {
		delete g_theFxManager;
		g_theFxManager = nullptr;
	}

	// Destroy the music manager
	if (g_theMusicManager) {
		delete g_theMusicManager;
		g_theMusicManager = nullptr;
	}

	// Destroy the speech manager
	if (g_theSpeechManager) {
		delete g_theSpeechManager;
		g_theSpeechManager = nullptr;
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
