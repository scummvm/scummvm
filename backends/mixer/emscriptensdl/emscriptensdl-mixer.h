/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef BACKENDS_MIXER_EMSCRIPTENSDL_H
#define BACKENDS_MIXER_EMSCRIPTENSDL_H

#include "backends/mixer/sdl/sdl-mixer.h"
#include "common/queue.h"

/**
 * Emscripten SDL mixer manager. Extends the SDL mixer to decouple
 * I/O operations from the audio callback.
 */
class EmscriptenSdlMixerManager : public SdlMixerManager {
public:
#if SDL_VERSION_ATLEAST(3, 0, 0)
	EmscriptenSdlMixerManager();
	virtual ~EmscriptenSdlMixerManager();
#endif

protected:
	/**
	 * Starts SDL audio
	 */
	void startAudio() override;

	/**
	 * Pauses the audio system
	 */
	void suspendAudio() override;

	/**
	 * Resumes the audio system
	 */
	int resumeAudio() override;

#if SDL_VERSION_ATLEAST(3, 0, 0)
	void startTimer(int interval = 10 * 1000);
	void stopTimer();
	/**
	 * Fills the data buffers with new audio data for playback
	 */
	static void updateMixer(void *refCon);

	/**
	 * Emscripten-specific SDL3 callback with buffering support
	 */
	static void emscriptenSdl3Callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);

	int _lastRequestedAmount = -1;
	uint32 _requestTimestamp = 0;
	bool _timerStarted = false;
	Common::Queue<Common::Pair<int, Uint8 *> > *_dataBuffers;
#endif
};

#endif
