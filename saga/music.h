/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Music class

#ifndef SAGA_MUSIC_H_
#define SAGA_MUSIC_H_

#include "saga/rscfile_mod.h"
#include "saga/game_mod.h"
#include "sound/mixer.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"

namespace Saga {
enum MUSIC_FLAGS {
	R_MUSIC_LOOP = 0x0001,
	R_MUSIC_DEFAULT = 0xffff
};

struct MUSIC_MIDITABLE {
	const char *filename;
	int flags;
};

struct MUSIC_DIGITABLE {
	uint32 start;
	uint32 length;
};

class MusicPlayer : public MidiDriver {
public:
	MusicPlayer(MidiDriver *driver);
	~MusicPlayer();
	void setVolume(int volume);
	int getVolume() { return _masterVolume; }

	void hasNativeMT32(bool b) { _nativeMT32 = b; }
	void playMusic();
	void stopMusic();
	void setLoop(bool loop) { _looping = loop; }

	void setGM(bool isGM) { _isGM = isGM; }

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);

	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo(void)	{ return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	MidiParser *_parser;

protected:

	static void onTimer(void *data);

	MidiChannel *_channel[16];
	MidiDriver *_driver;
	byte _channelVolume[16];
	bool _nativeMT32;
	bool _isGM;

	bool _isPlaying;
	bool _looping;
	bool _randomLoop;
	byte _masterVolume;

	byte *_musicData;
	uint16 *_buf;
	uint32 _musicDataSize;
};

class Music {
public:

	Music(SoundMixer *mixer, MidiDriver *driver, int enabled);
	~Music(void);
	void hasNativeMT32(bool b)		{ _player->hasNativeMT32(b); }

	int play(uint32 music_rn, uint16 flags = R_MUSIC_DEFAULT);
	int pause(void);
	int resume(void);
	int stop(void);

private:

	SoundMixer *_mixer;

	MusicPlayer *_player;
	PlayingSoundHandle _musicHandle;

	static const MUSIC_MIDITABLE _midiTableITECD[26];
	MUSIC_DIGITABLE _digiTableITECD[27];

	int _musicInitialized;
	int _enabled;
	bool _hasDigiMusic;
};

} // End of namespace Saga

#endif
