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

namespace Ultima {
namespace Ultima8 {

class Debugger;
class MidiPlayer;

class MusicProcess : public Process {
	friend class Debugger;

protected:
	//! Play a music track
	//! \param track The track number to play. Pass 0 to stop music
	virtual void playMusic_internal(int track) = 0;

	static MusicProcess *_theMusicProcess;

public:
	MusicProcess();
	~MusicProcess() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	//! Get the current instance of the Music Processes
	static MusicProcess *get_instance() {
		return _theMusicProcess;
	}

	//! Play some background music. Does not change the current track if combat music is active.  If another track is currently queued, just queues this track for play.
	virtual void playMusic(int track) = 0;
	//! Play some combat music - the last played track will be remembered
	virtual void playCombatMusic(int track) = 0;
	//! Queue a track to start once the current one finishes
	virtual void queueMusic(int track) = 0;
	//! Clear any queued track (does not affect currently playing track)
	virtual void unqueueMusic() = 0;
	//! Restore the last requested non-combat track (eg, at the end of combat)
	virtual void restoreMusic() = 0;

	//! Save the current track state - used when the menu is opened
	virtual void saveTrackState() = 0;
	//! Bring back the track state from before it was put on hold
	virtual void restoreTrackState() = 0;

	//! Is a track currently playing?
	virtual bool isPlaying() = 0;

	//! Pause the currently playing track
	virtual void pauseMusic() = 0;
	//! Resume the current track after pausing
	virtual void unpauseMusic() = 0;

	INTRINSIC(I_playMusic);
	INTRINSIC(I_stopMusic);
	INTRINSIC(I_pauseMusic);
	INTRINSIC(I_unpauseMusic);

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
