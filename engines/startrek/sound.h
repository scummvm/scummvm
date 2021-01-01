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

#include "startrek/soundeffects.h"
#include "startrek/startrek.h"

#include "audio/midiparser.h"
#include "audio/mididrv.h"
#include "audio/mixer.h"

namespace Common {
class MacResManager;
}

namespace StarTrek {

class StarTrekEngine;


// Max # of VOC files that can play at once
const int MAX_SFX_PLAYING = 4;

const int NUM_MIDI_SLOTS = 8;

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
	/**
	 * Plays a midi track as a sound effect (one of midi slots 1-7)
	 */
	void playMidiTrack(int track);
	void playMidiTrackInSlot(int slot, int track);
	bool isMidiPlaying();
	void loadMusicFile(const Common::String &baseSoundName);
	void playMidiMusicTracks(int startTrack, int loopTrack);
	void playVoc(const Common::String &baseSoundName);
	void playSpeech(const Common::String &basename);
	void stopAllVocSounds();
	void stopPlayingSpeech();
	void playSoundEffectIndex(int index);
	void setMusicEnabled(bool enable);
	void setSfxEnabled(bool enable);
	void toggleMusic();
	void toggleSfx();
	void checkLoopMusic();

private:
	StarTrekEngine *_vm;
	Audio::SoundHandle *_soundHandle;

	void loadPCMusicFile(const Common::String &baseSoundName);
	void clearMidiSlot(int slot);

	// MIDI-Related Variables
	MidiDriver *_midiDriver;
	MidiPlaybackSlot _midiSlots[NUM_MIDI_SLOTS]; // 0 is for music; 1-7 are for sfx
	Common::List<MidiPlaybackSlot *> _midiSlotList; // Sorts midi slots by most recently used

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

} // End of namespace StarTrek

#endif
