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
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_SOUND_H
#define MORTEVIELLE_SOUND_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/mutex.h"
#include "common/queue.h"

namespace Mortevielle {

typedef int tablint[256];

/**
 * Structure used to store pending notes to play
 */
struct SpeakerNote {
	int freq;
	uint32 length;

	SpeakerNote(int noteFreq, uint32 noteLength) {
		freq = noteFreq;
		length = noteLength;
	}
};

/**
 * This is a modified PC Speaker class that allows the queueing of an entire song
 * sequence one note at a time.
 */
class PCSpeaker : public Audio::AudioStream {
private:
	Common::Queue<SpeakerNote> _pendingNotes;
	Common::Mutex _mutex;

	int _rate;
	uint32 _oscLength;
	uint32 _oscSamples;
	uint32 _remainingSamples;
	uint32 _mixedSamples;
	byte _volume;

	void dequeueNote();
protected:
	static int8 generateSquare(uint32 x, uint32 oscLength);
public:
	PCSpeaker(int rate = 44100);
	~PCSpeaker();

	/** Play a note for length microseconds.
	 */
	void play(int freq, uint32 length);
	/** Stop the currently playing sequence */
	void stop();
	/** Adjust the volume. */
	void setVolume(byte volume);

	bool isPlaying() const;

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const	{ return false; }
	bool endOfData() const	{ return false; }
	bool endOfStream() const { return false; }
	int getRate() const	{ return _rate; }
};

class SoundManager {
private:
	Audio::Mixer *_mixer;
	PCSpeaker *_speakerStream;
	Audio::SoundHandle _speakerHandle;
public:
	SoundManager(Audio::Mixer *mixer);
	~SoundManager();

	void playNote(int frequency, int32 length);

	void decodeMusic(const byte *PSrc, byte *PDest, int NbreSeg);
	void litph(tablint &t, int typ, int tempo);
	void musyc(tablint &tb, int nbseg, int att);
};

} // End of namespace Mortevielle

#endif
