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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_AUDIO_H
#define SAGA2_AUDIO_H

#include "audio/mixer.h"

namespace Saga2 {

class Music;
class hResContext;

#define Here Point32(0,0)

enum VolumeTarget {
	kVolSfx,
	kVolVoice,
	kVolMusic
};

struct SoundInstance {
	uint32 seg;
	bool loop;
	Point32 loc;
};

class AudioInterface {
private:
	enum {
		kClickSounds = 3
	};

	SoundInstance _currentSpeech;
	SoundInstance _currentLoop;
	SoundInstance _currentMusic;

public:
	Audio::SoundHandle _speechSoundHandle;
	Audio::SoundHandle _sfxSoundHandle;
	Audio::SoundHandle _bgmSoundHandle;
	Audio::SoundHandle _clickSoundHandle;
	Audio::SoundHandle _loopSoundHandle;

	Common::List<SoundInstance> _speechQueue;
	Common::Queue<SoundInstance> _sfxQueue;

	Audio::Mixer *_mixer;
	Music *_music;

	int32 _clickSizes[kClickSounds];
	uint8 *_clickData[kClickSounds];

public:
	// ctor, dtor, initialization
	AudioInterface();
	~AudioInterface();

	// init, cleanup
	void initAudioInterface(hResContext *musicContext);

	// event loop calls
	bool playFlag();
	void playMe();

	// music calls
	void playMusic(uint32 s, int16 loopFactor = 1, Point32 where = Here);
	void stopMusic();

	// sound calls
	void queueSound(uint32 s, int16 loopFactor = 1, Point32 where = Here);

	// loop calls
	void playLoop(uint32 s, int16 loopFactor = 0, Point32 where = Here);
	void stopLoop();
	void setLoopPosition(Point32 newLoc);
	uint32 currentLoop() {
		return _currentLoop.seg;
	}

	// voice calls
	void queueVoice(uint32 s, Point32 where = Here);
	void queueVoice(uint32 s[], Point32 where = Here);
	void stopVoice();
	bool talking();
	bool saying(uint32 s);

	byte getVolume(VolumeTarget src);
	void suspend();
	void resume();
};

} // end of namespace Saga2

#endif
