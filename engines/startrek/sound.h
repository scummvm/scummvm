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


// Sound effects for "playSoundEffectIndex" function
enum SoundEffects {
	SND_TRICORDER = 4,
	SND_DOOR1 = 5,
	SND_PHASSHOT = 6,
	SND_07 = 7,
	SND_TRANSDEM = 8,
	SND_TRANSMAT = 9,
	SND_TRANSENE = 0x0a,
	SND_BLANK_0b = 0x0b,
	SND_SELECTION = 0x10,
	SND_BLANK_14 = 0x14,
	SND_HAILING = 0x22,
	SND_PHASSHOT_2 = 0x24,
	SND_PHOTSHOT = 0x25,
	SND_HITSHIEL = 0x26,
	SND_27 = 0x27,
	SND_REDALERT = 0x28,
	SND_WARP = 0x29
};

// Max # of VOC files that can play at once
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
	
	void clearAllMidiSlots();
	void playMidiTrack(int track);
	void playMidiTrackInSlot(int slot, int track);
	void loadMusicFile(const Common::String &baseSoundName);
	void playMidiMusicTracks(int startTrack, int loopTrack);
	void playVoc(const Common::String &baseSoundName);
	void playSpeech(const Common::String &basename);
	void stopAllVocSounds();
	void stopPlayingSpeech();
	void setMusicEnabled(bool enable);
	void setSfxEnabled(bool enable);
	void checkLoopMusic();
	
private:
	StarTrekEngine *_vm;
	Audio::SoundHandle *_soundHandle;
	
	void loadPCMusicFile(const Common::String &baseSoundName);
	void clearMidiSlot(int slot);
	
	// MIDI-Related Variables
	MidiDriver *_midiDriver;
	MidiPlaybackSlot _midiSlots[8]; // 0 is for music; 1-7 are for sfx
	Common::List<MidiPlaybackSlot*> _midiSlotList; // Sorts midi slots by most recently used

	byte *loadedSoundData;
	uint32 _midiDevice;	

	// VOC-related variables
	Audio::SoundHandle _sfxHandles[MAX_SFX_PLAYING];
	Audio::SoundHandle _speechHandle;
	bool _playingSpeech;

public:
	Common::String _loopingAudioName;
	Common::String _loadedMidiFilename;
	int _loopingMidiTrack;

private:
	// Driver callback
	static void midiDriverCallback(void *data);
};

}

#endif
