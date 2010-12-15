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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_SOUND_H
#define HUGO_SOUND_H

#include "sound/mixer.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"

namespace Hugo {

class MidiPlayer : public MidiDriver {
public:

	enum {
		NUM_CHANNELS = 16
	};

	MidiPlayer(MidiDriver *driver);
	~MidiPlayer();

	void play(uint8 *stream, uint16 size);
	void stop();
	void pause(bool p);
	void updateTimer();
	void adjustVolume(int diff);
	void setVolume(int volume);
	int getVolume() const { return _masterVolume; }
	void setLooping(bool loop) { _isLooping = loop; }

	// MidiDriver interface
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length);
	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo() { return _driver ? _driver->getBaseTempo() : 0; }
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

private:

	static void timerCallback(void *p);

	MidiDriver *_driver;
	MidiParser *_parser;
	uint8 *_midiData;
	bool _isLooping;
	bool _isPlaying;
	bool _paused;
	int _masterVolume;
	MidiChannel *_channelsTable[NUM_CHANNELS];
	uint8 _channelsVolume[NUM_CHANNELS];
	Common::Mutex _mutex;
};

class SoundHandler {
public:
	SoundHandler(HugoEngine *vm);
	~SoundHandler();

	void toggleMusic();
	void toggleSound();
	void setMusicVolume();
	void playMusic(int16 tune);
	void playSound(int16 sound, stereo_t channel, byte priority);
	void initSound();

private:
	HugoEngine *_vm;
	Audio::SoundHandle _soundHandle;
	MidiPlayer *_midiPlayer;

	void stopSound();
	void stopMusic();
	void playMIDI(sound_pt seq_p, uint16 size);
};

} // End of namespace Hugo

#endif //HUGO_SOUND_H
