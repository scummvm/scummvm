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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "lab/lab.h"
#include "lab/stddefines.h"
#include "lab/labfun.h"

namespace Lab {

#define PLAYBUFSIZE     65536L

extern bool MusicOn;

//static sound_buff firstblock, tempblock;
static int bufnum;


bool EffectPlaying = false, ContMusic = false, DoMusic = false;
static char *CurMusic, *startMusic;
static uint32 StartMusicLen;
static Audio::SoundHandle g_musicHandle;
static Audio::SoundHandle g_sfxHandle;
static Audio::QueuingAudioStream *queuingAudioStream = NULL;

static byte *playBuffer;

void freeAudio() {
	if (!DoMusic)
		return;

	g_lab->_mixer->stopHandle(g_musicHandle);
	g_lab->_mixer->stopHandle(g_sfxHandle);

	delete queuingAudioStream;
	queuingAudioStream = NULL;
	delete[] playBuffer;
}



bool initAudio() {
	playBuffer = new byte[PLAYBUFSIZE];
	return true;
}


bool musicBufferEmpty() {
	return !g_lab->_mixer->isSoundHandleActive(g_sfxHandle);
}


uint16 getPlayingBufferCount() {
	return (queuingAudioStream) ? queuingAudioStream->numQueuedStreams() : 0;
}

void playMusicBlock(void *Ptr, uint32 Size, uint16 BufferNum, uint16 SampleSpeed) {
	bool startMusic = false;

	if (SampleSpeed < 4000)
		SampleSpeed = 4000;

	if (!queuingAudioStream) {
		queuingAudioStream = Audio::makeQueuingAudioStream(SampleSpeed, false);
		startMusic = true;
	}

	byte soundFlags = Audio::FLAG_LITTLE_ENDIAN;
	if (g_lab->getPlatform() == Common::kPlatformWindows)
		soundFlags |= Audio::FLAG_16BITS;
	else
		soundFlags |= Audio::FLAG_UNSIGNED;

	queuingAudioStream->queueBuffer((byte *)Ptr, Size, DisposeAfterUse::YES, soundFlags);

	if (startMusic)
		g_lab->_mixer->playStream(Audio::Mixer::kMusicSoundType, &g_musicHandle, queuingAudioStream);
}


void updateSoundBuffers() {
	if (!DoMusic)
		return;

	if (!EffectPlaying)
		return;

	// TODO
	// FIXME: Very crude implementation
	if (musicBufferEmpty()) {
		flushAudio();
		EffectPlaying = false;
	}

#if 0
	for (int i = 0; i < 2; i++) {
		if ((SDLSoundBufferStatus(i) == DAC_DONE) && firstblock.len) {
			// use extra memory for 16-bit samples
			tempblock.len = MIN(PLAYBUFSIZE, firstblock.len);
			firstblock.len -= tempblock.len;

			if (!(bufnum ^= 1)) {
				memcpy(buf1, CurMusic, (unsigned) tempblock.len);
				tempblock.sel_data = buf1;
			} else {
				memcpy(buf2, CurMusic, (unsigned) tempblock.len);
				tempblock.sel_data = buf2;
			}

			CurMusic += tempblock.len;

			SDLPlayBuffer(i, &tempblock);
		}
	}

	//
	// Playback ends when no bytes are left in the source data and
	// the status of both buffers equals DAC_DONE
	//

	if (!firstblock.len) {
		if (ContMusic) {
			CurMusic = startMusic;
			firstblock.len = StartMusicLen;
		} else if ((SDLSoundBufferStatus(0) == DAC_DONE) &&
		           (SDLSoundBufferStatus(1) == DAC_DONE)) {
			flushAudio();
			EffectPlaying = false;
		}
	}

#endif
}



void flushAudio() {
	if (!DoMusic)
		return;

	g_lab->_mixer->stopHandle(g_sfxHandle);
	EffectPlaying = false;
}




void playSoundEffect(uint16 SampleSpeed, uint16 Volume, uint32 Length, bool flush, void *Data) {
	if (!DoMusic)
		return;

	g_music->pauseBackMusic();

	if (flush)
		flushAudio();

	if (SampleSpeed < 4000)
		SampleSpeed = 4000;

	// TODO: 8-bit mono sample for DOS
	//firstblock.len = Length;
	bufnum = 0;

	//tempblock = firstblock;
	EffectPlaying = true;
	CurMusic = (char *)Data;
	startMusic = CurMusic;
	StartMusicLen = Length;

	byte soundFlags = Audio::FLAG_LITTLE_ENDIAN;
	if (g_lab->getPlatform() == Common::kPlatformWindows)
		soundFlags |= Audio::FLAG_16BITS;
	else
		soundFlags |= Audio::FLAG_UNSIGNED;

	Audio::SeekableAudioStream *audStream = Audio::makeRawStream((const byte *)Data, Length, SampleSpeed, soundFlags, DisposeAfterUse::NO);
	g_lab->_mixer->playStream(Audio::Mixer::kSFXSoundType, &g_sfxHandle, audStream);

	updateSoundBuffers();
}

} // End of namespace Lab
