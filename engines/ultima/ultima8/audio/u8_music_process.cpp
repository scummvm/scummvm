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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/audio/u8_music_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/audio/music_flex.h"
#include "ultima/ultima8/audio/midi_player.h"
#include "ultima/ultima8/audio/audio_mixer.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(U8MusicProcess)

U8MusicProcess::U8MusicProcess() : _midiPlayer(nullptr), _state(PLAYBACK_NORMAL),
		_currentTrack(0), _combatMusicActive(false),
		_savedTrackState(nullptr) {
	Std::memset(_songBranches, (byte)-1, 128 * sizeof(int));
}

U8MusicProcess::U8MusicProcess(MidiPlayer *player) : _midiPlayer(player),
		_state(PLAYBACK_NORMAL), _currentTrack(0), _combatMusicActive(false),
		_savedTrackState(nullptr) {
	Std::memset(_songBranches, (byte)-1, 128 * sizeof(int));

	_theMusicProcess = this;
	_type = 1; // persistent
	setRunPaused();
}

U8MusicProcess::~U8MusicProcess() {
	if (_savedTrackState)
		delete _savedTrackState;
	if (_midiPlayer)
		_midiPlayer->stop();
	_theMusicProcess = nullptr;
}

void U8MusicProcess::playMusic(int track) {
	_trackState._lastRequest = track;

	if (_combatMusicActive)
		return;

	if (_trackState._queued) {
		_trackState._queued = track;
		return;
	}

	playMusic_internal(track);
}

void U8MusicProcess::playCombatMusic(int track) {
	_combatMusicActive = (track != 0);
	playMusic_internal(track);
}

void U8MusicProcess::queueMusic(int track) {
	if (_trackState._wanted != track) {
		_trackState._queued = track;
	}
}

void U8MusicProcess::unqueueMusic() {
	_trackState._queued = 0;
}

void U8MusicProcess::restoreMusic() {
	_trackState._queued = 0;
	_combatMusicActive = false;
	playMusic_internal(_trackState._lastRequest);
}

void U8MusicProcess::getTrackState(TrackState &trackState) const {
	trackState = _trackState;
}

void U8MusicProcess::setTrackState(const TrackState &trackState) {
	_trackState = trackState;
	_state = PLAYBACK_PLAY_WANTED;
}

void U8MusicProcess::saveTrackState() {
	assert(!_savedTrackState);
	_savedTrackState = new TrackState(_trackState);
}

void U8MusicProcess::restoreTrackState() {
	if (_savedTrackState == nullptr)
		return;

	_trackState = *_savedTrackState;
	_state = PLAYBACK_PLAY_WANTED;
	delete _savedTrackState;
	_savedTrackState = nullptr;
}

void U8MusicProcess::playMusic_internal(int track) {
	if (track < 0 || track >= 128) {
		playMusic_internal(0);
		return;
	}

	MusicFlex *musicflex = GameData::get_instance()->getMusic();

	// No current track if not playing
	if (_midiPlayer && !_midiPlayer->isPlaying())
		_trackState._wanted = _currentTrack = 0;

	// It's already playing and we are not transitioning
	if (_currentTrack == track && _state == PLAYBACK_NORMAL) {
		return;
	} else if (_currentTrack == 0 || _state != PLAYBACK_NORMAL || !_midiPlayer) {
		_trackState._wanted = track;
		_state = PLAYBACK_PLAY_WANTED;

	} else {
		// We want to do a transition
		const MusicFlex::SongInfo *info = musicflex->getSongInfo(_currentTrack);

		uint32 measure = _midiPlayer->getSequenceCallbackData(0);

		// No transition info, or invalid measure, so fast change
		if (!info || (measure >= (uint32)info->_numMeasures) ||
		        !info->_transitions[track] || !info->_transitions[track][measure]) {
			_currentTrack = 0;
			if (track == 0) {
				_trackState._wanted = 0;
				_state = PLAYBACK_PLAY_WANTED;
			} else {
				playMusic_internal(track);
			}
			return;
		}

		// Get transition info
		int trans = info->_transitions[track][measure];
		bool speed_hack = false;

		if (trans < 0) {
			trans = (-trans) - 1;
			speed_hack = true;
		} else {
			_midiPlayer->stop();
			trans = trans - 1;
		}

		// Now get the transition midi
		int xmidi_index = _midiPlayer->isFMSynth() ? 260 : 258;
		MusicFlex::XMidiData *xmidi = musicflex->getXMidi(xmidi_index);

		warning("Doing a MIDI transition! trans: %d xmidi: %d speedhack: %d", trans, xmidi_index, speed_hack);

		if (xmidi && xmidi->_data) {
			_midiPlayer->load(xmidi->_data, xmidi->_size, 1, speed_hack);
			_midiPlayer->play(trans, -1);
		} else {
			_midiPlayer->stop();
		}

		_trackState._wanted = track;
		_state = PLAYBACK_TRANSITION;
	}
}

void U8MusicProcess::run() {
	switch (_state) {
	case PLAYBACK_NORMAL:
		if (_midiPlayer && !_midiPlayer->isPlaying() && _trackState._queued) {
			_trackState._wanted = _trackState._queued;
			_state = PLAYBACK_PLAY_WANTED;
			_trackState._queued = 0;
		}

		break;

	case PLAYBACK_TRANSITION:
		if (!_midiPlayer) {
			_state = PLAYBACK_PLAY_WANTED;
		} else {
			_state = PLAYBACK_PLAY_WANTED;
			_midiPlayer->stop();
		}
		break;

	case PLAYBACK_PLAY_WANTED: {
		if (_midiPlayer)
			_midiPlayer->stop();

		MusicFlex::XMidiData *xmidi = nullptr;

		if (_trackState._wanted) {
			int xmidi_index = _trackState._wanted;
			if (_midiPlayer && _midiPlayer->isFMSynth())
				xmidi_index += 128;

			xmidi = GameData::get_instance()->getMusic()->getXMidi(xmidi_index);
		}

		if (xmidi && xmidi->_data) {

			if (_midiPlayer) {
				// if there's a track queued, only play this one once
				bool repeat = (_trackState._queued == 0);
				_midiPlayer->load(xmidi->_data, xmidi->_size, 0, false);
				_midiPlayer->setLooping(repeat);
				if (_songBranches[_trackState._wanted] >= 0 && !_midiPlayer->hasBranchIndex(_songBranches[_trackState._wanted])) {
					if (_songBranches[_trackState._wanted] == 0) {
						// This track does not have any branches.
						_songBranches[_trackState._wanted] = -1;
					} else {
						// Current branch is past the end of the list of branches. Reset to 0.
						_songBranches[_trackState._wanted] = 0;
					}
				}
				_midiPlayer->play(0, _songBranches[_trackState._wanted]);
			}

			_currentTrack = _trackState._wanted;
			// Start this track at a different point (branch) next time
			_songBranches[_trackState._wanted]++;
		} else {
			_currentTrack = _trackState._wanted = 0;
		}
		_state = PLAYBACK_NORMAL;
	}
	break;
	}
}

void U8MusicProcess::saveData(Common::WriteStream *ws) {
	MusicProcess::saveData(ws);

	// When saving the game we want to remember the track state
	// from before the menu was opened
	const TrackState *stateToSave = _savedTrackState;
	if (stateToSave == nullptr)
		stateToSave = &_trackState;

	ws->writeUint32LE(static_cast<uint32>(stateToSave->_wanted));
	ws->writeUint32LE(static_cast<uint32>(stateToSave->_lastRequest));
	ws->writeUint32LE(static_cast<uint32>(stateToSave->_queued));
}

bool U8MusicProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!MusicProcess::loadData(rs, version)) return false;

	_trackState._wanted = static_cast<int32>(rs->readUint32LE());

	if (version >= 4) {
		_trackState._lastRequest = static_cast<int32>(rs->readUint32LE());
		_trackState._queued = static_cast<int32>(rs->readUint32LE());
	} else {
		_trackState._lastRequest = _trackState._wanted;
		_trackState._queued = 0;
	}

	_state = PLAYBACK_PLAY_WANTED;

	_theMusicProcess = this;

	_midiPlayer = AudioMixer::get_instance()->getMidiPlayer();

	return true;
}

bool U8MusicProcess::isPlaying() {
	return _currentTrack != 0;
}

void U8MusicProcess::pauseMusic() {
	// probably no real use for this?
	warning("TODO: U8MusicProcess::pauseMusic Implement me.");
}

void U8MusicProcess::unpauseMusic() {
	// probably no real use for this?
	warning("TODO: U8MusicProcess::unpauseMusic Implement me.");
}


} // End of namespace Ultima8
} // End of namespace Ultima
