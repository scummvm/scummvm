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

#ifndef PARALLACTION_MUSIC_H
#define PARALLACTION_MUSIC_H

#include "common/util.h"
#include "common/mutex.h"

#include "sound/audiostream.h"
#include "sound/iff.h"
#include "sound/mixer.h"
#include "sound/mididrv.h"

#define PATH_LEN 200

class MidiParser;

namespace Parallaction {

class Parallaction_ns;
class MidiPlayer;

typedef void* SoundManCommandParameter;

class SoundManImpl {
public:
	virtual void execute(int command, SoundManCommandParameter parm) = 0;
	virtual ~SoundManImpl() { }
};

class SoundMan {
	SoundManImpl *_impl;
public:
	SoundMan(SoundManImpl *impl) : _impl(impl) { }
	virtual ~SoundMan() { delete _impl; }
	void execute(int command, SoundManCommandParameter parm = 0) {
		if (_impl) {
			_impl->execute(command, parm);
		}
	}
};

enum {
	// soundMan commands
	SC_PLAYMUSIC,
	SC_STOPMUSIC,
	SC_SETMUSICTYPE,
	SC_SETMUSICFILE,
	SC_PLAYSFX,
	SC_STOPSFX,
	SC_SETSFXCHANNEL,
	SC_SETSFXLOOPING,
	SC_SETSFXVOLUME,
	SC_SETSFXRATE,
	SC_PAUSE
};

class SoundMan_ns : public SoundManImpl {
public:
	enum {
		MUSIC_ANY,
		MUSIC_CHARACTER,
		MUSIC_LOCATION
	};

protected:
	Parallaction_ns	*_vm;
	Audio::Mixer	*_mixer;
	char			_musicFile[PATH_LEN];

	bool 	_sfxLooping;
	int		_sfxVolume;
	int		_sfxRate;
	uint	_sfxChannel;
	
	int		_musicType;

public:
	SoundMan_ns(Parallaction_ns *vm);
	virtual ~SoundMan_ns() {}

	virtual void playSfx(const char *filename, uint channel, bool looping, int volume = -1) { }
	virtual void stopSfx(uint channel) { }

	void setMusicFile(const char *filename);
	virtual void playMusic() = 0;
	virtual void stopMusic() = 0;
	virtual void playCharacterMusic(const char *character) = 0;
	virtual void playLocationMusic(const char *location) = 0;
	virtual void pause(bool p) { }
	virtual void execute(int command, SoundManCommandParameter parm = 0);

	void setMusicVolume(int value);
};

class DosSoundMan_ns : public SoundMan_ns {

	MidiPlayer	*_midiPlayer;
	int			_musicData1;

	bool isLocationSilent(const char *locationName);

public:
	DosSoundMan_ns(Parallaction_ns *vm, MidiDriver *midiDriver);
	~DosSoundMan_ns();
	void playMusic();
	void stopMusic();

	void playCharacterMusic(const char *character);
	void playLocationMusic(const char *location);

	void pause(bool p);
};

#define NUM_AMIGA_CHANNELS 4

class AmigaSoundMan_ns : public SoundMan_ns {

	Audio::AudioStream *_musicStream;
	Audio::SoundHandle	_musicHandle;

	struct Channel {
		Audio::Voice8Header	header;
		int8				*data;
		uint32				dataSize;
		bool				dispose;
		Audio::SoundHandle	handle;
		uint32				flags;
	} _channels[NUM_AMIGA_CHANNELS];

	void loadChannelData(const char *filename, Channel *ch);

public:
	AmigaSoundMan_ns(Parallaction_ns *vm);
	~AmigaSoundMan_ns();
	void playMusic();
	void stopMusic();

	void playSfx(const char *filename, uint channel, bool looping, int volume);
	void stopSfx(uint channel);

	void playCharacterMusic(const char *character);
	void playLocationMusic(const char *location);
};

class DummySoundMan : public SoundManImpl {
public:	
	void execute(int command, SoundManCommandParameter parm) { }
};

} // namespace Parallaction

#endif
