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
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/sound.h $
 * $Id: sound.h 15 2010-06-27 06:13:42Z clone2727 $
 *
 */

#ifndef STARTREK_SOUND_H
#define STARTREK_SOUND_H

#include "startrek/startrek.h"

#include "audio/midiparser.h"
#include "audio/mididrv.h"
#include "audio/mixer.h"

namespace Common {
	class MacResManager;
}

namespace StarTrek {

class StarTrekEngine;

class Sound {
public:
	Sound(StarTrekEngine *vm);
	~Sound();
	
	void playSound(const char *baseSoundName);	
	void playSoundEffect(const char *baseSoundName);	
	
private:
	StarTrekEngine *_vm;
	Audio::SoundHandle *_soundHandle;
	
	// PC Sound Functions
	void playXMIDISound(const char *baseSoundName);
	void playSMFSound(const char *baseSoundName);
	
	// Macintosh Sound Functions
	void playMacSMFSound(const char *baseSoundName);
	void playMacSoundEffect(const char *baseSoundName);
	Common::MacResManager *_macAudioResFork;
	
	// Amiga Sound Functions
	void playAmigaSound(const char *baseSoundName);
	void playAmigaSoundEffect(const char *baseSoundName);
	
	// MIDI-Related Variables
	MidiParser *_midiParser;
	MidiDriver *_midiDriver;
	uint32 _midiDevice;	
};

}

#endif
