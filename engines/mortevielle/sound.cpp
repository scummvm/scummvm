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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/scummsys.h"
#include "mortevielle/sound.h"
#include "mortevielle/mortevielle.h"

namespace Mortevielle {

/**
 * Constructor
 */
PCSpeaker::PCSpeaker(int rate) {
	_rate = rate;
	_oscLength = 0;
	_oscSamples = 0;
	_remainingSamples = 0;
	_volume = 255;
}

/**
 * Destructor
 */
PCSpeaker::~PCSpeaker() {
}

/**
 * Adds a new note to the queue of notes to be played.
 */
void PCSpeaker::play(int freq, uint32 length) {
	assert((freq > 0) && (length > 0));
	Common::StackLock lock(_mutex);

	_pendingNotes.push(SpeakerNote(freq, length));
}

/**
 * Stops the currently playing song
 */
void PCSpeaker::stop() {
	Common::StackLock lock(_mutex);

	_remainingSamples = 0;
	_pendingNotes.clear();
}

void PCSpeaker::setVolume(byte volume) {
	_volume = volume;
}

/**
 * Return true if a song is currently playing
 */
bool PCSpeaker::isPlaying() const {
	return !_pendingNotes.empty() || (_remainingSamples != 0);
}

/**
 * Method used by the mixer to pull off pending samples to play
 */
int PCSpeaker::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	int i;

	for (i = 0; (_remainingSamples || !_pendingNotes.empty())  && (i < numSamples); ++i) {
		if (!_remainingSamples)
			// Used up the current note, so queue the next one
			dequeueNote();

		buffer[i] = generateSquare(_oscSamples, _oscLength) * _volume;
		if (_oscSamples++ >= _oscLength)
			_oscSamples = 0;

		_remainingSamples--;
	}

	// Clear the rest of the buffer
	if (i < numSamples)
		memset(buffer + i, 0, (numSamples - i) * sizeof(int16));

	return numSamples;
}

/**
 * Dequeues a note from the pending note list
 */
void PCSpeaker::dequeueNote() {
	SpeakerNote note = _pendingNotes.pop();

	_oscLength = _rate / note.freq;
	_oscSamples = 0;
	_remainingSamples = (_rate * note.length) / 1000000;
	assert((_oscLength > 0) && (_remainingSamples > 0));
}

/**
 * Support method for generating a square wave
 */
int8 PCSpeaker::generateSquare(uint32 x, uint32 oscLength) {
	return (x < (oscLength / 2)) ? 127 : -128;
}

/*-------------------------------------------------------------------------*/

const int tab[16] = { -96, -72, -48, -32, -20, -12, -8, -4, 0, 4, 8, 12, 20, 32, 48, 72 };

// The PC timer chip works at a frequency of 1.19318Mhz
#define TIMER_FREQUENCY 1193180

SoundManager::SoundManager(Audio::Mixer *mixer) {
	_mixer = mixer;
	_speakerStream = new PCSpeaker(mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
							_speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

SoundManager::~SoundManager() {
	_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;

}

/**
 * Decode music data
 */
void SoundManager::decodeMusic(const byte *PSrc, byte *PDest, int NbreSeg) {
	int seed = 128;
	int v;

	for (int idx1 = 0; idx1 < (NbreSeg * 2); ++idx1) {
		for (int idx2 = 0; idx2 < 64; ++idx2) {
			byte srcByte = *PSrc++;
			v = tab[srcByte >> 4];
			seed += v;
			*PDest++ = seed & 0xff;

			v = tab[srcByte & 0xf];
			seed += v;
			*PDest++ = seed & 0xff;
		}
	}
}

void SoundManager::litph(tablint &t, int typ, int tempo) {
	return;
}

void SoundManager::playNote(int frequency, int32 length) {
	_speakerStream->play(frequency, length);
}


void SoundManager::musyc(tablint &tb, int nbseg, int att) {
#ifdef DEBUG
	const byte *pSrc = &mem[0x5000 * 16];

	// Convert the countdown amount to a tempo rate, and then to note length in microseconds
	int tempo = TIMER_FREQUENCY / att;
	int length = 1000000 / tempo;

	for (int noteIndex = 0; noteIndex < (nbseg * 16); ++noteIndex) {
		int lookupValue = *pSrc++;
		int noteCountdown = tb[lookupValue];
		int noteFrequency = TIMER_FREQUENCY / noteCountdown;

		playNote(noteFrequency, length);
	}

	// Keep waiting until the song has been finished
	while (_speakerStream->isPlaying() && !g_vm->shouldQuit()) {
		g_vm->delay(10);
	}
#endif
}

} // End of namespace Mortevielle
