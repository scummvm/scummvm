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

#ifndef TWINE_SOUND_H
#define TWINE_SOUND_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/scummsys.h"
#include "common/types.h"
#include "twine/shared.h"

namespace TwinE {

class TextEntry;

#define NUM_CHANNELS 32

namespace Samples {
enum _Samples {
	TwinsenHit = 0,
	SoldierHit = 4,
	ItemPopup = 11,
	Explode = 37,
	BigItemFound = 41,
	TaskCompleted = 41,
	Hit = 86,
	ItemFound = 97,
	WalkFloorBegin = 126, // BASE_STEP_SOUND
	WalkFloorRightBegin = 141
};
}

class TwinEEngine;
class Sound {
private:
	TwinEEngine *_engine;

	/** Samples playing at the same time */
	Audio::SoundHandle _samplesPlaying[NUM_CHANNELS];

	/** Samples playing at a actors position */
	int32 _samplesPlayingActors[NUM_CHANNELS]{0};

	bool playSample(int32 channelIdx, int32 index, Audio::SeekableAudioStream *audioStream, int32 loop, const char *name, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);

	bool isChannelPlaying(int32 channelIdx);

	/** Find a free channel slot to use */
	int32 getFreeSampleChannelIndex();

	/** Remove a sample from the channel usage list */
	void removeChannelWatch(int32 channelIdx);
public:
	int32 _parmSampleVolume = 127;
	int32 _parmSampleDecalage = 0;
	int32 _parmSampleFrequence = 0;

	Sound(TwinEEngine *engine);
	~Sound();

	/** Get the channel where the sample is playing */
	int32 getSampleChannel(int32 index);

	/**
	 * Play FLA movie samples
	 * @param index sample index under flasamp.hqr file
	 * @param repeat number of times to repeat the sample
	 */
	void playFlaSample(int32 index, int16 rate, int32 repeat, uint8 volumeLeft, uint8 volumeRight);

	void setChannelBalance(int32 channelIdx, uint8 volumeLeft, uint8 volumeRight);

	void setChannelRate(int32 channelIdx, uint32 rate);

	/** Update sample position in channel */
	void setChannelPosition(int32 channelIdx, int32 x, int32 y, int32 z);

	inline void setChannelPosition(int32 channelIdx, const IVec3 &pos) {
		setChannelPosition(channelIdx, pos.x, pos.y, pos.z);
	}

	/**
	 * Play samples
	 * @param index sample index under flasamp.hqr file
	 * @param repeat number of times to repeat the sample
	 * @param pos sound generating entity position
	 * @param actorIdx
	 */
	void mixSample3D(int32 index, uint16 pitchbend, int32 repeat, const IVec3 &pos, int32 actorIdx); // HQ_3D_MixSample

	void mixSample(int32 index, uint16 pitchbend, int32 repeat, uint8 volumeLeft, uint8 volumeRight); // HQ_MixSample

	/** Pause samples */
	void pauseSamples();

	/** Resume samples */
	void resumeSamples();

	void startRainSample();

	/** Stop samples */
	void stopSamples();

	/** Get the channel where the actor sample is playing */
	int32 getActorChannel(int32 actorIdx);

	/** Stops a specific sample */
	void stopSample(int32 index); // HQ_StopOneSample

	/** Check if a sample is playing */
	int32 isSamplePlaying(int32 index);

	/** Play VOX sample */
	bool playVoxSample(const TextEntry *text);
};

} // namespace TwinE

#endif
