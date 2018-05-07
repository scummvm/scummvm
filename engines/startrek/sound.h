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


const int MAX_LOADED_SFX_FILES = 8;
const int MAX_SFX_PLAYING = 4;

struct MidiPlaybackSlot {
	int slot;
	int track;
	MidiParser *midiParser;
};


class Sound {
public:
	Sound(StarTrekEngine *vm);
	~Sound();
	
	void playMidiTrack(int track);

	void loadMusicFile(const char *baseSoundName);
	void playSoundEffect(const char *baseSoundName);
	void playSpeech(const Common::String &basename);
	void stopPlayingSpeech();
	
private:
	StarTrekEngine *_vm;
	Audio::SoundHandle *_soundHandle;
	
	void loadPCMusicFile(const char *baseSoundName);
	void clearMidiSlot(int slot);
	void clearAllMidiSlots();
	
	// MIDI-Related Variables
	MidiDriver *_midiDriver;
	MidiPlaybackSlot _midiSlots[8]; // 0 is for music; 1-7 are for sfx
	Common::List<MidiPlaybackSlot*> _sfxSlotList; // Sorts midi slots by most recently used

	byte *loadedSoundData;
	uint32 _midiDevice;	

	// VOC-related variables
	Common::String _loopingAudioName;
	Audio::SoundHandle _sfxHandles[MAX_SFX_PLAYING];
	Audio::SoundHandle _speechHandle;
	bool _playingSpeech;

	// Driver callback
	static void midiDriverCallback(void *data);
};

}

#endif
