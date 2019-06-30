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

#pragma once

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "cryo/cryolib.h"

namespace Cryo {

class CryoEngine;

class CSoundChannel {
private:
	Audio::Mixer *_mixer;
	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _soundHandle;
	unsigned int _sampleRate;
	bool _stereo;
	unsigned int _bufferFlags;

	void applyVolumeChange();

public:
	CSoundChannel(Audio::Mixer *mixer, unsigned int sampleRate, bool stereo, bool is16bits = false);
	~CSoundChannel();

	// Queue a new buffer, cancel any previously queued buffers if playNow is set
	void queueBuffer(byte *buffer, unsigned int size, bool playNow = false, bool playQueue = true, bool buffering = true);

	// Play any queued buffers
	void play();

	// Stop playing and purge play queue
	void stop();

	// How many buffers in queue (including currently playing one)
	unsigned int numQueued();

	// Volume control
	int _volumeLeft, _volumeRight;
	unsigned int getVolume();
	void setVolume(unsigned int volumeLeft, unsigned int volumeRight);
	void setVolumeLeft(unsigned int volume);
	void setVolumeRight(unsigned int volume);
};

}
