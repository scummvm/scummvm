/** @file sound.h
	@brief
	This file contains music playing routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SOUND_H
#define SOUND_H

#include "sys.h"
#include "sys.h"

/** Total number of sprites allowed in the game */
#define NUM_SAMPLES 243
#define NUM_CHANNELS 32

/** Table with all loaded samples */
uint8* samplesTable[NUM_SAMPLES];
/** Table with all loaded samples sizes */
uint32 samplesSizeTable[NUM_SAMPLES];

/** Samples playing at the same time */
int32 samplesPlaying[NUM_CHANNELS];

/** Samples playing at a actors position */
int32 samplesPlayingActors[NUM_CHANNELS];

/** Sample volume
	@param channel sample channel
	@param volume sample volume number */
void sampleVolume(int32 channel, int32 volume);

/** Play FLA movie samples
	@param index sample index under flasamp.hqr file
	@param frequency frequency used to play the sample
	@param repeat number of times to repeat the sample
	@param x unknown x variable
	@param y unknown y variable */
void playFlaSample(int32 index, int32 frequency, int32 repeat, int32 x, int32 y);

/** Update sample position in channel */
void setSamplePosition(int32 channelIdx, int32 x, int32 y, int32 z);

/** Play samples
	@param index sample index under flasamp.hqr file
	@param frequency frequency used to play the sample
	@param repeat number of times to repeat the sample
	@param x unknown x variable
	@param y unknown y variable
	@param z unknown z variable */
void playSample(int32 index, int32 frequency, int32 repeat, int32 x, int32 y, int32 z, int32 actorIdx);

/** Pause samples */
void pauseSamples();

void resumeSamples();

/** Stop samples */
void stopSamples();

/** Get the channel where the actor sample is playing */
int32 getActorChannel(int32 index);

/** Get the channel where the sample is playing */
int32 getSampleChannel(int32 index);

/** Stops a specific sample */
void stopSample(int32 index);

/** Find a free channel slot to use */
int32 getFreeSampleChannelIndex();

/** Remove a sample from the channel usage list */
void removeSampleChannel(int32 index);

/** Check if a sample is playing */
int32 isSamplePlaying(int32 index);

/** Play VOX sample */
void playVoxSample(int32 index);

#endif
