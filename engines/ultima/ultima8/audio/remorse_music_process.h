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

#ifndef ULTIMA8_AUDIO_REMORSEMUSICPROCESS_H
#define ULTIMA8_AUDIO_REMORSEMUSICPROCESS_H

#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Ultima {
namespace Ultima8 {

class Debugger;
class MidiPlayer;

class RemorseMusicProcess : public MusicProcess {
	friend class Debugger;

protected:
	//! Play a music track
	//! \param track The track number to play. Pass 0 to stop music
	void playMusic_internal(int track) override;

private:
	int _currentTrack;      //! Currently playing track (don't save)

	//! Is the current music "combat" music
	bool _combatMusicActive;

	int _savedTrack;

	uint8 _m16offset;

	Audio::SoundHandle _soundHandle;

	int _maxTrack;
	const char **_trackNames;

public:
	RemorseMusicProcess();
	~RemorseMusicProcess() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	//! Play some background music. Does not change the current track if combat music is active.  If another track is currently queued, just queues this track for play.
	void playMusic(int track) override;
	//! Play some combat music - the last played track will be remembered
	void playCombatMusic(int track) override;
	//! Queue a track to start once the current one finishes
	void queueMusic(int track) override;
	//! Clear any queued track (does not affect currently playing track)
	void unqueueMusic() override;
	//! Restore the last requested non-combat track (eg, at the end of combat)
	void restoreMusic() override;

	//! Save the current track state - used when the menu is opened
	void saveTrackState() override;
	//! Bring back the track state from before it was put on hold
	void restoreTrackState() override;

	//! Is a track currently playing?
	bool isPlaying() override;

	//! Pause the currently playing track
	void pauseMusic() override;
	//! Resume the current track after pausing
	void unpauseMusic() override;

	void run() override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
