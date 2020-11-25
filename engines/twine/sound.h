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

#ifndef TWINE_SOUND_H
#define TWINE_SOUND_H

#include "audio/mixer.h"
#include "common/scummsys.h"
#include "common/types.h"

namespace TwinE {

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
	WalkFloorBegin = 126
};
}

class TwinEEngine;
class Sound {
private:
	TwinEEngine *_engine;

	/** Get the channel where the sample is playing */
	int32 getSampleChannel(int32 index);

	/** Samples playing at the same time */
	Audio::SoundHandle samplesPlaying[NUM_CHANNELS];

	/** Samples playing at a actors position */
	int32 samplesPlayingActors[NUM_CHANNELS]{0};

	bool playSample(int channelIdx, int index, uint8 *sampPtr, int32 sampSize, int32 loop, const char *name, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType, DisposeAfterUse::Flag disposeFlag = DisposeAfterUse::YES);

	bool isChannelPlaying(int32 channel);

	/** Find a free channel slot to use */
	int32 getFreeSampleChannelIndex();

	/** Remove a sample from the channel usage list */
	void removeSampleChannel(int32 index);

public:
	Sound(TwinEEngine *engine);
	~Sound();

	/**
	 * Play FLA movie samples
	 * @param index sample index under flasamp.hqr file
	 * @param repeat number of times to repeat the sample
	 * @param x unknown x variable
	 * @param y unknown y variable
	 */
	void playFlaSample(int32 index, int32 repeat, int32 x, int32 y);

	/** Update sample position in channel */
	void setSamplePosition(int32 channelIdx, int32 x, int32 y, int32 z);

	/**
	 * Play samples
	 * @param index sample index under flasamp.hqr file
	 * @param repeat number of times to repeat the sample
	 * @param x sound generating entity x position
	 * @param y sound generating entity y position
	 * @param z sound generating entity z position
	 * @param actorIdx
	 */
	void playSample(int32 index, int32 repeat = 1, int32 x = 128, int32 y = 128, int32 z = 128, int32 actorIdx = -1);

	/** Pause samples */
	void pauseSamples();

	/** Resume samples */
	void resumeSamples();

	/** Stop samples */
	void stopSamples();

	/** Get the channel where the actor sample is playing */
	int32 getActorChannel(int32 index);

	/** Stops a specific sample */
	void stopSample(int32 index);

	/** Check if a sample is playing */
	int32 isSamplePlaying(int32 index);

	/** Play VOX sample */
	void playVoxSample(int32 index);
};

} // namespace TwinE

#endif
