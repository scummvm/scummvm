/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TWINE_MUSIC_H
#define TWINE_MUSIC_H

#include "audio/midiplayer.h"
#include "audio/mixer.h"
#include "common/scummsys.h"

namespace TwinE {

class TwinEEngine;

class TwinEMidiPlayer : public Audio::MidiPlayer {
private:
	TwinEEngine *_engine;
public:
	TwinEMidiPlayer(TwinEEngine *engine);
	void play(byte *buf, int size, bool loop);
};

class Music {
private:
	TwinEEngine *_engine;
	TwinEMidiPlayer _midiPlayer;

	void fadeMusicMidi(uint32 time = 1);

	/** Auxiliar midi pointer to  */
	uint8 *midiPtr = nullptr;
	Audio::SoundHandle _midiHandle;
	/** Track number of the current playing music */
	int32 numXmi = -1;
	int32 currentMusicCD = -1;
	// int32 endMusicCD = -1;
	const bool _flagVoiceCD = false;
public:
	// TODO: implement the handling
	int32 _nextMusic = -1;       // lba2: NextMusic
	int32 _nextMusicTimer;       // lba2: NextMusicTimer
	bool _stopLastMusic = false; // lba2: StopLastMusic
private:
	/** Stop CD music */
	void stopMusicCD();
	bool playMidi(int32 midiIdx);
	int32 getLengthTrackCDR(int track) const;
	bool playTrackCDR(int32 track);
public:
	Music(TwinEEngine *engine);

	/**
	 * Music volume
	 * @param current volume number
	 */
	void musicVolume(int32 volume);

	/**
	 * Play CD music
	 * @param track track number to play
	 */
	bool playCdTrack(int32 track);
	/**
	 * Generic play music, according with settings it plays CD or high quality sounds instead
	 * @param track track number to play
	 */
	bool playMusic(int32 track);
	/**
	 * Play MIDI music
	 * @param midiIdx music index under mini_mi_win.hqr
	 * @note valid indices for lba1 are [1-32]
	 */
	bool playMidiFile(int32 midiIdx);

	void playAllMusic(int track);

	/** Stop MIDI music */
	void stopMusicMidi();

	/** Initialize CD-Rom */
	bool initCdrom();

	/** Stop MIDI and Track music */
	void stopMusic();

	bool isMidiPlaying() const;
	int32 getMusicCD();
};

} // namespace TwinE

#endif
