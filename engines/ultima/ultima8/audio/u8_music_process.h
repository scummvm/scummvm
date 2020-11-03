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

#ifndef ULTIMA8_AUDIO_U8MUSICPROCESS_H
#define ULTIMA8_AUDIO_U8MUSICPROCESS_H

#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "audio/mididrv.h"

namespace Ultima {
namespace Ultima8 {

class Debugger;
class MidiPlayer;

class U8MusicProcess : public MusicProcess {
	friend class Debugger;

	enum PlaybackStates {
		PLAYBACK_NORMAL = 1,
		PLAYBACK_TRANSITION = 2,
		PLAYBACK_PLAY_WANTED = 3
	};

public:
	//! The saveable part of track state
	struct TrackState {
		//! Track we want to play
		int _wanted;
		//! Last requested track that was not a temporary (ie, combat) track
		int _lastRequest;
		//! Track queued to start after current
		int _queued;

		TrackState() : _wanted(0), _lastRequest(0), _queued(0) { }
		TrackState(int wanted, int lastRequest, int queued) :
			_wanted(wanted), _lastRequest(lastRequest), _queued(queued) { }
	};

private:
	//! Play a music track
	//! \param track The track number to play. Pass 0 to stop music
	void playMusic_internal(int track) override;

	MidiPlayer *_midiPlayer;
	PlaybackStates _state;
	//! The branch (starting point) to use for each music track
	int _songBranches[128];

	int _currentTrack;      //! Currently playing track (don't save)

	TrackState _trackState;

	//! The track state temporarily saved when using the menu etc
	TrackState *_savedTrackState;

	//! Is the current music "combat" music
	bool _combatMusicActive;

public:
	U8MusicProcess();
	U8MusicProcess(MidiPlayer *player); // Note that this does NOT delete the driver
	~U8MusicProcess() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	//! Get the current instance of the Music Processes
	static MusicProcess *get_instance() {
		return _theMusicProcess;
	}

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

	//! Get the state of tracks (wanted, requested, queued)
	void getTrackState(TrackState &trackState) const;

	void setTrackState(const TrackState &state);

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
