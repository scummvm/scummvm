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
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/audio/music_flex.h"
#include "ultima/ultima8/audio/midi_player.h"
#include "ultima/ultima8/audio/audio_mixer.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MusicProcess, Process)

MusicProcess *MusicProcess::_theMusicProcess = nullptr;

MusicProcess::MusicProcess() : _midiPlayer(nullptr), _state(PLAYBACK_NORMAL),
		_currentTrack(0), _combatMusicActive(false) {
	Std::memset(_songBranches, (byte)-1, 128 * sizeof(int));
}

MusicProcess::MusicProcess(MidiPlayer *player) : _midiPlayer(player),
		_state(PLAYBACK_NORMAL), _currentTrack(0), _combatMusicActive(false)  {
	Std::memset(_songBranches, (byte)-1, 128 * sizeof(int));

	_theMusicProcess = this;
	_flags |= PROC_RUNPAUSED;
	_type = 1; // persistent
}

MusicProcess::~MusicProcess() {
	_midiPlayer->stop();
	_theMusicProcess = nullptr;
}

void MusicProcess::playMusic(int track) {
	_trackState._lastRequest = track;

	if (_combatMusicActive)
		return;

	if (_trackState._queued) {
		_trackState._queued = track;
		return;
	}

	playMusic_internal(track);
}

void MusicProcess::playCombatMusic(int track) {
	_combatMusicActive = (track != 0);
	playMusic_internal(track);
}

void MusicProcess::queueMusic(int track) {
	if (_trackState._wanted != track) {
		_trackState._queued = track;
	}
}

void MusicProcess::unqueueMusic() {
	_trackState._queued = 0;
}

void MusicProcess::restoreMusic() {
	_trackState._queued = 0;
	_combatMusicActive = false;
	playMusic_internal(_trackState._lastRequest);
}

void MusicProcess::getTrackState(TrackState &trackState) const {
	trackState = _trackState;
}

void MusicProcess::setTrackState(const TrackState &trackState) {
	_trackState = trackState;
	_state = PLAYBACK_PLAY_WANTED;
}

void MusicProcess::playMusic_internal(int track) {
	if (track < 0 || track > 128) {
		playMusic_internal(0);
		return;
	}

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
		// TODO: Properly handle transitions under ScummVM
#ifdef TODO
		const MusicFlex::SongInfo *info = GameData::get_instance()->getMusic()->getSongInfo(_currentTrack);

		uint32 measure = _midiPlayer->getSequenceCallbackData(0);

		// No transition info, or invalid measure, so fast change
		if (!info || (measure >= (uint32)info->num_measures) ||
		        !info->transitions[track] || !info->transitions[track][measure]) {
			_currentTrack = 0;
			if (track == 0) {
				_trackState._wanted = 0;
				_state = MUSIC_PLAY_WANTED;
			} else {
				playMusic_internal(track);
			}
			return;
		}

		// Get transition info
		int trans = info->transitions[track][measure];
		bool speed_hack = false;

		if (trans < 0) {
			trans = (-trans) - 1;
			speed_hack = true;
		} else {
			_midiPlayer->finishSequence(0);
			trans = trans - 1;
		}

		// Now get the transition midi
		int xmidi_index = _midiPlayer->isFMSynth() ? 260 : 258;
		XMidiFile *xmidi = GameData::get_instance()->getMusic()->getXMidi(xmidi_index);
		XMidiEventList *list;

		if (xmidi)
			list = xmidi->GetEventList(trans);
		else
			list = 0;

		if (list) {
			_midiPlayer->startSequence(1, list, false, 255, _songBranches[track]);
			if (speed_hack)
				_midiPlayer->setSequenceSpeed(1, 200);
		} else {
			_midiPlayer->finishSequence(1);
		}
#endif
		_trackState._wanted = track;
		_state = PLAYBACK_TRANSITION;
	}
}

void MusicProcess::run() {
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

		byte *data = nullptr;
		uint32 size = 0;

		if (_trackState._wanted) {
			int xmidi_index = _trackState._wanted;
			if (_midiPlayer && _midiPlayer->isFMSynth())
				xmidi_index += 128;

			data = GameData::get_instance()->getMusic()->getRawObject(xmidi_index, &size);
		}

		if (data) {
			if (_midiPlayer) {
				// if there's a track queued, only play this one once
				bool repeat = (_trackState._queued == 0);
				_midiPlayer->play(data, size);
				_midiPlayer->setLooping(repeat);
			}

			_currentTrack = _trackState._wanted;
			_songBranches[_trackState._wanted]++;
		} else {
			_currentTrack = _trackState._wanted = 0;
		}
		_state = PLAYBACK_NORMAL;
	}
	break;
	}
}

void MusicProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(_trackState._wanted));
	ods->write4(static_cast<uint32>(_trackState._lastRequest));
	ods->write4(static_cast<uint32>(_trackState._queued));
}

bool MusicProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	_trackState._wanted = static_cast<int32>(ids->read4());

	if (version >= 4) {
		_trackState._lastRequest = static_cast<int32>(ids->read4());
		_trackState._queued = static_cast<int32>(ids->read4());
	} else {
		_trackState._lastRequest = _trackState._wanted;
		_trackState._queued = 0;
	}

	_state = PLAYBACK_PLAY_WANTED;

	_theMusicProcess = this;

	_midiPlayer = AudioMixer::get_instance()->getMidiPlayer();

	return true;
}

uint32 MusicProcess::I_musicStop(const uint8 * /*args*/,
                                 unsigned int /*argsize*/) {
	if (_theMusicProcess) _theMusicProcess->playMusic_internal(0);
	return 0;
}

uint32 MusicProcess::I_playMusic(const uint8 *args,
                                 unsigned int /*argsize*/) {
	ARG_UINT8(song);
	if (_theMusicProcess) _theMusicProcess->playMusic(song & 0x7F);
	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
