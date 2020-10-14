/** @file sound.cpp
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

#include <SDL/SDL.h>
#ifndef MACOSX
#include <SDL/SDL_mixer.h>
#else
#include <SDL_mixer/SDL_mixer.h>
#endif

#include "sound.h"
#include "flamovies.h"
#include "main.h"
#include "resources.h"
#include "hqrdepack.h"
#include "movements.h"
#include "grid.h"
#include "collision.h"
#include "text.h"

/** SDL_Mixer channels */
int32 channel;
/** Samples chunk variable */
Mix_Chunk *sample;

int32 channelIdx = -1;

/** Sample volume
	@param channel sample channel
	@param volume sample volume number */
void sampleVolume(int32 channel, int32 volume) {
	Mix_Volume(channel, volume / 2);
}

/** Play FLA movie samples
	@param index sample index under flasamp.hqr file
	@param frequency frequency used to play the sample
	@param repeat number of times to repeat the sample
	@param x unknown x variable
	@param y unknown y variable */
void playFlaSample(int32 index, int32 frequency, int32 repeat, int32 x, int32 y) {
	if (cfgfile.Sound) {
		int32 sampSize = 0;
		int8 sampfile[256];
		SDL_RWops *rw;
		uint8* sampPtr;

		sprintf(sampfile, FLA_DIR "%s",HQR_FLASAMP_FILE);

		sampSize = hqrGetallocEntry(&sampPtr, sampfile, index);

		// Fix incorrect sample files first byte
		if (*sampPtr != 'C')
			*sampPtr = 'C';

		rw = SDL_RWFromMem(sampPtr, sampSize);
		sample = Mix_LoadWAV_RW(rw, 1);

		channelIdx = getFreeSampleChannelIndex();
		if (channelIdx != -1) {
			samplesPlaying[channelIdx] = index;
		}

		sampleVolume(channelIdx, cfgfile.WaveVolume);

		if (Mix_PlayChannel(channelIdx, sample, repeat - 1) == -1)
			printf("Error while playing VOC: Sample %d \n", index);

		/*if (cfgfile.Debug)
			printf("Playing VOC: Sample %d\n", index);*/

		free(sampPtr);
	}
}

void setSamplePosition(int32 channelIdx, int32 x, int32 y, int32 z) {
	int32 distance;
	distance = Abs(getDistance3D(newCameraX << 9, newCameraY << 8, newCameraZ << 9, x, y, z));
	distance = getAverageValue(0, distance, 10000, 255);
	if (distance > 255) { // don't play it if its to far away
		distance = 255;
	}

	Mix_SetDistance(channelIdx, distance);
}

/** Play samples
	@param index sample index under flasamp.hqr file
	@param frequency frequency used to play the sample
	@param repeat number of times to repeat the sample
	@param x unknown x variable
	@param y unknown y variable
	@param z unknown z variable */
void playSample(int32 index, int32 frequency, int32 repeat, int32 x, int32 y, int32 z, int32 actorIdx) {
	if (cfgfile.Sound) {
		int32 sampSize = 0;
		SDL_RWops *rw;
		uint8* sampPtr;

		sampSize = hqrGetallocEntry(&sampPtr, HQR_SAMPLES_FILE, index);

		// Fix incorrect sample files first byte
		if (*sampPtr != 'C')
			*sampPtr = 'C';

		rw = SDL_RWFromMem(sampPtr, sampSize);
		sample = Mix_LoadWAV_RW(rw, 1);

		channelIdx = getFreeSampleChannelIndex();

		// only play if we have a free channel, otherwise we won't be able to control the sample
		if (channelIdx != -1) {
			samplesPlaying[channelIdx] = index;
			sampleVolume(channelIdx, cfgfile.WaveVolume);

			if (actorIdx != -1) {
				setSamplePosition(channelIdx, x, y, z);

				// save the actor index for the channel so we can check the position
				samplesPlayingActors[channelIdx] = actorIdx;
			}

			if (Mix_PlayChannel(channelIdx, sample, repeat - 1) == -1)
				printf("Error while playing VOC: Sample %d \n", index);

			/*if (cfgfile.Debug)
				printf("Playing VOC: Sample %d\n", index);*/
		}

		free(sampPtr);
	}
}

/** Resume samples */
void resumeSamples() {
	if (cfgfile.Sound) {
		Mix_Resume(-1);
		/*if (cfgfile.Debug)
			printf("Resume VOC samples\n");*/
	}
}

/** Pause samples */
void pauseSamples() {
	if (cfgfile.Sound) {
		Mix_HaltChannel(-1);
		/*if (cfgfile.Debug)
			printf("Pause VOC samples\n");*/
	}
}

/** Stop samples */
void stopSamples() {
	if (cfgfile.Sound) {
		memset(samplesPlaying, -1, sizeof(int32) * NUM_CHANNELS);
		Mix_HaltChannel(-1);
		//clean up
		Mix_FreeChunk(sample);
		sample = NULL; //make sure we free it
		/*if (cfgfile.Debug)
			printf("Stop VOC samples\n");*/
	}
}

int32 getActorChannel(int32 index) {
	int32 c = 0;
	for (c = 0; c < NUM_CHANNELS; c++) {
		if (samplesPlayingActors[c] == index) {
			return c;
		}
	}
	return -1;
}

int32 getSampleChannel(int32 index) {
	int32 c = 0;
	for (c = 0; c < NUM_CHANNELS; c++) {
		if (samplesPlaying[c] == index) {
			return c;
		}
	}
	return -1;
}

void removeSampleChannel(int32 c) {
	samplesPlaying[c] = -1;
	samplesPlayingActors[c] = -1;
}

/** Stop samples */
void stopSample(int32 index) {
	if (cfgfile.Sound) {
		int32 stopChannel = getSampleChannel(index);
		if (stopChannel != -1) {
			removeSampleChannel(stopChannel);
			Mix_HaltChannel(stopChannel);
			//clean up
			Mix_FreeChunk(sample);
			sample = NULL; //make sure we free it
			/*if (cfgfile.Debug)
				printf("Stop VOC samples\n");*/
		}
	}
}

int32 isChannelPlaying(int32 channel) {
	if (channel != -1) {
		if(Mix_Playing(channel)) {
			return 1;
		} else {
			removeSampleChannel(channel);
		}
	}
	return 0;
}

int32 isSamplePlaying(int32 index) {
	int32 channel = getSampleChannel(index);
	return isChannelPlaying(channel);
}

int32 getFreeSampleChannelIndex() {
	int i = 0;
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (samplesPlaying[i] == -1) {
			return i;
		}
	}
	//FIXME if didn't find any, lets free what is not in use
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (samplesPlaying[i] != -1) {
			isChannelPlaying(i);
		}
	}
	return -1;
}

void playVoxSample(int32 index) {
	if (cfgfile.Sound) {
		int32 sampSize = 0;
		SDL_RWops *rw;
		uint8* sampPtr = 0;

		sampSize = hqrGetallocVoxEntry(&sampPtr, currentVoxBankFile, index, voxHiddenIndex);

		// Fix incorrect sample files first byte
		if (*sampPtr != 'C') {
			hasHiddenVox = *sampPtr;
			voxHiddenIndex++;
			*sampPtr = 'C';
		}

		rw = SDL_RWFromMem(sampPtr, sampSize);
		sample = Mix_LoadWAV_RW(rw, 1);

		channelIdx = getFreeSampleChannelIndex();

		// only play if we have a free channel, otherwise we won't be able to control the sample
		if (channelIdx != -1) {
			samplesPlaying[channelIdx] = index;

			sampleVolume(channelIdx, cfgfile.VoiceVolume - 1);

			if (Mix_PlayChannel(channelIdx, sample, 0) == -1)
				printf("Error while playing VOC: Sample %d \n", index);

			/*if (cfgfile.Debug)
				printf("Playing VOC: Sample %d\n", index);*/
		}

		free(sampPtr);
	}
}
