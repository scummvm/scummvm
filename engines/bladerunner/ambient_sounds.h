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

#ifndef BLADERUNNER_AMBIENT_SOUNDS_H
#define BLADERUNNER_AMBIENT_SOUNDS_H

#include "audio/audiostream.h"

namespace BladeRunner {

class BladeRunnerEngine;

class AmbientSounds {
	struct NonLoopingSound {
		bool   isActive;
		char   name[13];
		int32  hash;
		int32  audioPlayerTrack;
		int32  timeMin;
		int32  timeMax;
		uint32 nextPlayTime;
		int32  volumeMin;
		int32  volumeMax;
		int32  volume;
		int32  panStartMin;
		int32  panStartMax;
		int32  panEndMin;
		int32  panEndMax;
		int32  priority;
	};

	struct LoopingSound {
		bool  isActive;
		char  name[13];
		int32 hash;
		int   audioPlayerTrack;
		int32 volume;
		int   pan;
	};

	BladeRunnerEngine *_vm;

	NonLoopingSound *_nonLoopingSounds;
	LoopingSound    *_loopingSounds;
	int              _ambientVolume;

public:
	AmbientSounds(BladeRunnerEngine *vm);
	~AmbientSounds();

	void addSound(
		int sfxId,
		int timeMin, int timeMax,
		int volumeMin, int volumeMax,
		int panStartMin, int panStartMax,
		int panEndMin, int panEndMax,
		int priority, int unk
	);
	void removeNonLoopingSound(int sfxId, bool stopPlaying);
	void removeAllNonLoopingSounds(bool stopPlaying);

	void addSpeech(
		int actorId, int sentenceId,
		int timeMin, int timeMax,
		int volumeMin, int volumeMax,
		int panStartMin, int panStartMax,
		int panEndMin, int panEndMax,
		int priority, int unk);
	void playSound(int sfxId, int volume, int panStart, int panEnd, int priority);

	void addLoopingSound(int sfxId, int volume, int pan, int delay);
	void adjustLoopingSound(int sfxId, int volume, int pan, int delay);
	// it seems there is little confusion in original code about delay parameter,
	// sometimes it is used as boolean in same way as stopPlaying from non looping
	void removeLoopingSound(int sfxId, int delay);
	void removeAllLoopingSounds(int delay);

	void tick();

	// setVolume
	// getVolume

private:
	int findAvailableNonLoopingTrack();
	int findNonLoopingTrackByHash(int32 hash);

	int findAvailableLoopingTrack();
	int findLoopingTrackByHash(int32 hash);

	// stopNonLoopingTrack
	// stopLoopingTrack

	// saveToSaveGame
	// initFromSaveGame
	// addSoundByName
	// playVolumeAdjustSound

	void addSoundByName(
		const char *name,
		int timeMin, int timeMax,
		int volumeMin, int volumeMax,
		int panStartMin, int panStartMax,
		int panEndMin, int panEndMax,
		int priority, int unk);

	void removeNonLoopingSoundByIndex(int index, bool stopPlaying);
	void removeLoopingSoundByIndex(int index, int delay);
};

} // End of namespace BladeRunner

#endif
