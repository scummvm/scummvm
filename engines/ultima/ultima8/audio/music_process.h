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

#ifndef ULTIMA8_AUDIO_MUSICPROCESS_H
#define ULTIMA8_AUDIO_MUSICPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "audio/mididrv.h"

namespace Ultima8 {

class MusicProcess : public Process {
public:
	MusicProcess();
	MusicProcess(::MidiDriver *); // Note that this does NOT delete the driver
	virtual ~MusicProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	//! Get the current instance of the Music Processes
	static MusicProcess *get_instance() {
		return the_music_process;
	}

	void playMusic(int track);
	void playCombatMusic(int track);
	void queueMusic(int track);
	void unqueueMusic();
	void restoreMusic();

	INTRINSIC(I_playMusic);
	INTRINSIC(I_musicStop);


	//! Get the number of the current or wanted track
	int getTrack() const {
		return wanted_track;
	}

	virtual void run();

	bool loadData(IDataSource *ids, uint32 version);

	// MusicProcess::playMusic console command
	static void ConCmd_playMusic(const Console::ArgvType &argv);

private:
	virtual void saveData(ODataSource *ods);

	//! Play a music track
	//! \param track The track number to play. Pass 0 to stop music
	void playMusic_internal(int track);

	static MusicProcess    *the_music_process;

	::MidiDriver *driver;
	int         state;
	int         current_track;      // Currently playing track (don't save)
	int         wanted_track;       // Track we want to play (save this)
	int         song_branches[128];

	int         last_request;       // Last requested track
	int         queued_track;       // Track queued to start after current

	enum MusicStates {
		MUSIC_NORMAL                = 1,
		MUSIC_TRANSITION            = 2,
		MUSIC_PLAY_WANTED           = 3
	};
};

} // End of namespace Ultima8

#endif
