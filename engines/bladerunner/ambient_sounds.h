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
	BladeRunnerEngine *_vm;

	struct NonLoopingSound {
		bool   isActive;
		char   name[13];
		int32  hash;
		int32  audio_player_track;
		int32  time1;
		int32  time2;
		uint32 nextPlayTime;
		int32  volume1;
		int32  volume2;
		int32  volume;
		int32  pan1begin;
		int32  pan1end;
		int32  pan2begin;
		int32  pan2end;
		int32  priority;
	};

	struct LoopingSound {
		bool  isActive;
		char  name[13];
		int32 hash;
		int32 volume;
	};

	NonLoopingSound *_nonLoopingSounds;
	LoopingSound    *_loopingSounds;
	int              _ambientVolume;

public:
	AmbientSounds(BladeRunnerEngine *vm);
	~AmbientSounds();

	void addSound(
		int id,
		int timeRangeBegin, int timeRangeEnd,
		int volumeRangeBegin, int volumeRangeEnd,
		int pan1begin, int pan1end,
		int pan2begin, int pan2end,
		int priority, int unk3
	);
	// removeSound
	// addSpeechSound
	// removeSpeechSound
	// playSound
	// playSpeech
	// removeAllNonLoopingSounds

	// addLoopingSound
	void addLoopingSound(int sfx_id, int volume, int unk, int fadeInTime);
	// adjustLoopingSound
	// removeLoopingSound
	// removeAllLoopingSounds

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
		int timeRangeBegin, int timeRangeEnd,
		int volumeRangeBegin, int volumeRangeEnd,
		int unk1RangeBegin, int unk1RangeEnd,
		int unk2RangeBegin, int unk2RangeEnd,
		int priority, int unk3);
};

} // End of namespace BladeRunner

#endif
