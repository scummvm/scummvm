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

class Resource;

class Sound {
public:
	Sound(SoundMixer *mixer, Resource *resource);
	virtual ~Sound(); 
	virtual void sfxPlay(const char *base) = 0;
	static Sound *giveSound(SoundMixer *mixer, Resource *resource, uint8 compression);
	bool isPlaying();

protected:
	SoundMixer *_mixer;
	Resource *_resource;

	PlayingSoundHandle _sfxHandle;
};

class SilentSound : public Sound {
public:
	SilentSound(SoundMixer *mixer, Resource *resource) : Sound(mixer, resource) {};
	void sfxPlay(const char *base) { }
};

class SBSound : public Sound {
public:
	SBSound(SoundMixer *mixer, Resource *resource) : Sound(mixer, resource) {};
	int playSound(byte *sound, uint32 size);
	void sfxPlay(const char *base);
};

#ifdef USE_MAD
class MP3Sound : public Sound {
public:
	MP3Sound(SoundMixer *mixer, Resource *resource) : Sound(mixer, resource) {};
	void sfxPlay(const char *base);
};
#endif

} // End of namespace Queen

#endif
