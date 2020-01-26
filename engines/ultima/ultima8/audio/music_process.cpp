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
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MusicProcess, Process)

MusicProcess *MusicProcess::the_music_process = 0;

MusicProcess::MusicProcess() : _midiPlayer(0), _state(MUSIC_NORMAL),
		_currentTrack(0), _wantedTrack(0), _lastRequest(0), _queuedTrack(0) {
	std::memset(_songBranches, (byte)-1, 128 * sizeof(int));
}

MusicProcess::MusicProcess(MidiPlayer *player) : _midiPlayer(player),
		_state(MUSIC_NORMAL), _currentTrack(0), _wantedTrack(0),
		_lastRequest(0), _queuedTrack(0) {
	std::memset(_songBranches, (byte)-1, 128 * sizeof(int));

	the_music_process = this;
	flags |= PROC_RUNPAUSED;
	type = 1; // persistent
}

MusicProcess::~MusicProcess() {
	the_music_process = 0;
}

void MusicProcess::playMusic(int track) {
	_lastRequest = track;

	ObjectManager *om = ObjectManager::get_instance();
	if (om && getMainActor()) {
		MainActor *av = getMainActor();
		if (av->isInCombat() || (av->getActorFlags() & Actor::ACT_COMBATRUN)) {
			// combat music active
			return;
		}
	}

	if (_queuedTrack) {
		_queuedTrack = track;
		return;
	}

	playMusic_internal(track);
}

void MusicProcess::playCombatMusic(int track) {
	playMusic_internal(track);
}

void MusicProcess::queueMusic(int track) {
	if (_wantedTrack != track) {
		_queuedTrack = track;
	}
}

void MusicProcess::unqueueMusic() {
	_queuedTrack = 0;
}

void MusicProcess::restoreMusic() {
	_queuedTrack = 0;
	playMusic_internal(_lastRequest);
}

void MusicProcess::playMusic_internal(int track) {
	if (track < 0 || track > 128) {
		playMusic_internal(0);
		return;
	}

	// No current track if not playing
	if (_midiPlayer && !_midiPlayer->isPlaying())
		_wantedTrack = _currentTrack = 0;

	// It's already playing and we are not transitioning
	if (_currentTrack == track && _state == MUSIC_NORMAL) {
		return;
	} else if (_currentTrack == 0 || _state != MUSIC_NORMAL || !_midiPlayer) {
		_wantedTrack = track;
		_state = MUSIC_PLAY_WANTED;

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
				_wantedTrack = 0;
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

		if (xmidi) list = xmidi->GetEventList(trans);
		else list = 0;

		if (list) {
			_midiPlayer->startSequence(1, list, false, 255, _songBranches[track]);
			if (speed_hack) _midiPlayer->setSequenceSpeed(1, 200);
		} else _midiPlayer->finishSequence(1);
#endif
		_wantedTrack = track;
		_state = MUSIC_TRANSITION;
	}
}

void MusicProcess::run() {
	switch (_state) {
	case MUSIC_NORMAL:
		if (_midiPlayer && !_midiPlayer->isPlaying() && _queuedTrack) {
			_wantedTrack = _queuedTrack;
			_state = MUSIC_PLAY_WANTED;
			_queuedTrack = 0;
		}

		break;

	case MUSIC_TRANSITION:
		if (!_midiPlayer) {
			_state = MUSIC_PLAY_WANTED;
		} else if (!_midiPlayer->isPlaying()) {
			_state = MUSIC_PLAY_WANTED;
			_midiPlayer->stop();
		}
		break;

	case MUSIC_PLAY_WANTED: {
		if (_midiPlayer)
			_midiPlayer->stop();

		byte *data = nullptr;
		uint32 size = 0;

		if (_wantedTrack) {
			int xmidi_index = _wantedTrack;
			if (_midiPlayer && _midiPlayer->isFMSynth())
				xmidi_index += 128;

			data = GameData::get_instance()->getMusic()->getRawObject(xmidi_index, &size);
		}

		if (data) {
			if (_midiPlayer) {
				// if there's a track queued, only play this one once
				bool repeat = (_queuedTrack == 0);
				_midiPlayer->play(data, size);
				_midiPlayer->setLooping(repeat);
			}

			_currentTrack = _wantedTrack;
			_songBranches[_wantedTrack]++;
		} else {
			_currentTrack = _wantedTrack = 0;
		}
		_state = MUSIC_NORMAL;
	}
	break;
	}
}

void MusicProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(_wantedTrack));
	ods->write4(static_cast<uint32>(_lastRequest));
	ods->write4(static_cast<uint32>(_queuedTrack));
}

bool MusicProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	_wantedTrack = static_cast<int32>(ids->read4());

	if (version >= 4) {
		_lastRequest = static_cast<int32>(ids->read4());
		_queuedTrack = static_cast<int32>(ids->read4());
	} else {
		_lastRequest = _wantedTrack;
		_queuedTrack = 0;
	}

	_state = MUSIC_PLAY_WANTED;

	the_music_process = this;

	_midiPlayer = Pentagram::AudioMixer::get_instance()->getMidiPlayer();

	return true;
}

uint32 MusicProcess::I_musicStop(const uint8 * /*args*/,
                                 unsigned int /*argsize*/) {
	if (the_music_process) the_music_process->playMusic_internal(0);
	return 0;
}

uint32 MusicProcess::I_playMusic(const uint8 *args,
                                 unsigned int /*argsize*/) {
	ARG_UINT8(song);
	if (the_music_process) the_music_process->playMusic(song & 0x7F);
	return 0;
}


void MusicProcess::ConCmd_playMusic(const Console::ArgvType &argv) {
	if (the_music_process) {
		if (argv.size() != 2) {
			pout << "MusicProcess::playMusic (tracknum)" << std::endl;
		} else {
			pout << "Playing track " << argv[1] << std::endl;
			the_music_process->playMusic_internal(atoi(argv[1].c_str()));
		}
	} else {
		pout << "No Music Process" << std::endl;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
