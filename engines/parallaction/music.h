/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
#include "sound/mixer.h"
#include "sound/mididrv.h"

#include "parallaction/defs.h"

class MidiParser;

namespace Parallaction {

class Parallaction;
class MidiPlayer;

class SoundMan {

protected:
	Parallaction 	*_vm;
	Audio::Mixer 	*_mixer;
	char			_musicFile[PATH_LEN];

public:
	SoundMan(Parallaction *vm);
	virtual ~SoundMan() {}

	void setMusicFile(const char *filename);

	virtual void playMusic() = 0;
	virtual void stopMusic() = 0;

	virtual void playCharacterMusic(const char *character) = 0;
	virtual void playLocationMusic(const char *location) = 0;

	void setMusicVolume(int value);
};

class DosSoundMan : public SoundMan {

	MidiPlayer 	*_midiPlayer;
	int			_musicData1;

public:
	DosSoundMan(Parallaction *vm, MidiDriver *midiDriver);
	~DosSoundMan();
	void playMusic();
	void stopMusic();

	void playCharacterMusic(const char *character);
	void playLocationMusic(const char *location);
};

class AmigaSoundMan : public SoundMan {

	Audio::AudioStream *_musicStream;
	Audio::SoundHandle	_musicHandle;

public:
	AmigaSoundMan(Parallaction *vm);
	~AmigaSoundMan();
	void playMusic();
	void stopMusic();

	void playCharacterMusic(const char *character);
	void playLocationMusic(const char *location);
};

} // namespace Parallaction

#endif
