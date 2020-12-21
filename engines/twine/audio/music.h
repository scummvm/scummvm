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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TWINE_MUSIC_H
#define TWINE_MUSIC_H

#include "audio/midiplayer.h"
#include "common/scummsys.h"

namespace TwinE {

class TwinEEngine;

class TwinEMidiPlayer : public Audio::MidiPlayer {
private:
	TwinEEngine *_engine;
public:
	TwinEMidiPlayer(TwinEEngine *engine);
	void play(byte *buf, int size);
};

class Music {
private:
	TwinEEngine *_engine;
	TwinEMidiPlayer _midiPlayer;

	void musicFadeIn();
	void musicFadeOut();

	/** Auxiliar midi pointer to  */
	uint8 *midiPtr = nullptr;
	/** Track number of the current playing music */
	int32 currentMusic = -1;
	/**
	 * Play CD music
	 * @param track track number to play
	 */
	bool playTrackMusicCd(int32 track);
	/** Stop CD music */
	void stopTrackMusicCd();
public:
	Music(TwinEEngine *engine);

	/**
	 * Music volume
	 * @param current volume number
	 */
	void musicVolume(int32 volume);

	/**
	 * Generic play music, according with settings it plays CD or high quality sounds instead
	 * @param track track number to play
	 */
	bool playTrackMusic(int32 track);
	/** Generic stop music according with settings */
	void stopTrackMusic();
	/**
	 * Play MIDI music
	 * @param midiIdx music index under mini_mi_win.hqr
	 */
	bool playMidiMusic(int32 midiIdx, int32 loop = 0);
	/** Stop MIDI music */
	void stopMidiMusic();

	/** Initialize CD-Rom */
	bool initCdrom();

	/** Stop MIDI and Track music */
	void stopMusic();
};

} // namespace TwinE

#endif
