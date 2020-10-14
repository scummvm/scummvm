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

#include "common/scummsys.h"

namespace TwinE {

class TwinEEngine;

class Music {
private:
	TwinEEngine *_engine;

	void musicFadeIn(int32 loops, int32 ms);
	void musicFadeOut(int32 ms);

	/** Auxiliar midi pointer to  */
	uint8 *midiPtr = nullptr;

public:
	Music(TwinEEngine *engine) : _engine(engine) {}
	/** Track number of the current playing music */
	int32 currentMusic = 0;

	/**
	 * Music volume
	 * @param current volume number
	 */
	void musicVolume(int32 volume);
	/**
	 * Play CD music
	 * @param track track number to play
	 */
	void playTrackMusicCd(int32 track);
	/** Stop CD music */
	void stopTrackMusicCd();
	/**
	 * Generic play music, according with settings it plays CD or high quality sounds instead
	 * @param track track number to play
	 */
	void playTrackMusic(int32 track);
	/** Generic stop music according with settings */
	void stopTrackMusic();
	/**
	 * Play MIDI music
	 * @param midiIdx music index under mini_mi_win.hqr
	 */
	void playMidiMusic(int32 midiIdx, int32 loop);
	/** Stop MIDI music */
	void stopMidiMusic();

	/** Initialize CD-Rom */
	int32 initCdrom();

	/** Stop MIDI and Track music */
	void stopMusic();
};

} // namespace TwinE

#endif
