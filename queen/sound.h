/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENSOUND_H
#define QUEENSOUND_H

#include "sound/mixer.h"
#include "queen/defs.h"
#include "common/str.h"

namespace Queen {

class Input;
class Resource;

struct songData { 
	int16 tuneList[5];
	int16 volume;
	int16 tempo;
	int16 reverb;
	int16 override;
	int16 ignore;
};

struct tuneData { 
	int16 tuneNum[9];
	int16 sfx[2];
	int16 mode;
	int16 delay;
};

class Sound {
public:
	Sound(SoundMixer *mixer, Input *input, Resource *resource);
	virtual ~Sound(); 
	virtual void sfxPlay(const char *base) = 0;
	static Sound *giveSound(SoundMixer *mixer, Input *input, Resource *resource, uint8 compression);
	void waitSfxFinished();
	void playSong(int16 songNum);

protected:
	SoundMixer *_mixer;
	Input *_input;
	Resource *_resource;

	static const songData _song[];
	static const tuneData _tune[];
	static const char *_sfxName[];
	
	int16 _lastOverride;
	int16 _lastMerge;
	int16 _altMrgPri;
	int16 _currentSong;
	int16 _previousSong;
	int16 _previousSongNum;
	PlayingSoundHandle _sfxHandle;
};

class SilentSound : public Sound {
public:
	SilentSound(SoundMixer *mixer, Input *input, Resource *resource) : Sound(mixer, input, resource) {};
	void sfxPlay(const char *base) { }
};

class SBSound : public Sound {
public:
	SBSound(SoundMixer *mixer, Input *input, Resource *resource) : Sound(mixer, input, resource) {};
	void sfxPlay(const char *base);
protected:
	int playSound(byte *sound, uint32 size);
};

#ifdef USE_MAD
class MP3Sound : public Sound {
public:
	MP3Sound(SoundMixer *mixer, Input *input, Resource *resource) : Sound(mixer, input, resource) {};
	void sfxPlay(const char *base);
};
#endif

#ifdef USE_VORBIS
class OGGSound : public Sound {
public:
	OGGSound(SoundMixer *mixer, Input *input, Resource *resource) : Sound(mixer, input, resource) {};
	void sfxPlay(const char *base);
};
#endif
} // End of namespace Queen

#endif
