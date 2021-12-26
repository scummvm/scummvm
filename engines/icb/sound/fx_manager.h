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

#ifndef ICB_FXMANAGER_H__INCLUDED_
#define ICB_FXMANAGER_H__INCLUDED_

#include "engines/icb/sound/direct_sound.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace ICB {

extern bool8 noSoundEngine;

#define SAMPLE_NAME_LENGTH 64

typedef struct Effect {
	enum FxFlags { EMPTY, DELAYED, QUEUED, PLAYING, READY };

	char name[SAMPLE_NAME_LENGTH]; // Sample name
	int32 delay;                     // The delay until playing
	int32 looped;                    // Loop this effect

	int32 pitch;  // Sampling rate (Hz)
	int32 pan;    // Pan (DirectSound scale)
	int32 volume; // Volume (DirectSound scale)

	int32 rate;      // Original buffer sample rate
	FxFlags flags; // Status of sample
	int32 length;    // Length of sample in millisecs at base rate...
	Audio::SeekableAudioStream *_stream;
	Audio::SoundHandle _handle;
} Effect;

class FxManager {
private:
	Effect m_effects[MAX_FX];

public:
	FxManager();
	~FxManager();

public:
	// Register wavs and unregister (high level load and unload)
	int32 Register(const int32 id, const char *name, const int32 delay = 0, uint32 byteOffsetInCluster = 0);
	void Unregister(int32 id);
	void UnregisterAll();

	// Called on a timer 10 times a second
	bool8 Poll();

	// Pretty important really
	void Play(int32 id);
	void Stop(int32 id);

	void StopAll(void);

	// All realtime tweakers
	void SetVolume(int32 id, int32 vol);
	void SetPitch(int32 id, int32 pitch);

	// A DirectSound buffer can support panning OR 3D position but not both
	// A parameter for Register() lets you decide which to support
	void SetPan(int32 id, int32 pan);

	// Helpers and debug shat
	void SetLooping(int32 id, int32 loop = 1) { m_effects[id].looped = loop; }
	bool8 IsPlaying(int32 id) { return (m_effects[id].flags == Effect::PLAYING) ? TRUE8 : FALSE8; }

	int32 GetDefaultRate(const char *name, uint32 byteOffsetInCluster = 0);

	int32 GetDefaultLength(int32 id) { return m_effects[id].length; }

private:
	bool8 Load(int32 id, const char *name, uint32 byteOffsetInCluster = 0);

	// Can get original sampling rate by channel or examine a wav file
	int32 GetDefaultRateByID(int32 id) { return m_effects[id].rate; }
	int32 GetDefaultRateByName(const char *name, uint32 byteOffsetInCluster = 0);
};

} // End of namespace ICB

#endif
