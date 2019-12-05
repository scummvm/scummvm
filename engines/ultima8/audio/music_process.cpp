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

#include "ultima8/misc/pent_include.h"
#include "ultima8/audio/music_process.h"
#include "ultima8/games/game_data.h"
#include "ultima8/audio/music_flex.h"
#include "ultima8/audio/midi/midi_driver.h"
#include "ultima8/audio/midi/xmidi_file.h"
#include "ultima8/audio/midi/xmidi_event_list.h"
#include "ultima8/audio/audio_mixer.h"

#include "ultima8/kernel/object_manager.h"
#include "ultima8/world/get_object.h"
#include "ultima8/world/actors/main_actor.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MusicProcess, Process)

MusicProcess *MusicProcess::the_music_process = 0;

MusicProcess::MusicProcess()
	: current_track(0) {
	std::memset(song_branches, (byte)-1, 128 * sizeof(int));
}

MusicProcess::MusicProcess(MidiDriver *drv) :
	driver(drv), state(MUSIC_NORMAL), current_track(0),
	wanted_track(0), last_request(0), queued_track(0) {
	std::memset(song_branches, (byte)-1, 128 * sizeof(int));

	the_music_process = this;
	flags |= PROC_RUNPAUSED;
	type = 1; // persistent
}

MusicProcess::~MusicProcess() {
	the_music_process = 0;
}

void MusicProcess::playMusic(int track) {
	last_request = track;

	ObjectManager *om = ObjectManager::get_instance();
	if (om && getMainActor()) {
		MainActor *av = getMainActor();
		if (av->isInCombat() || (av->getActorFlags() & Actor::ACT_COMBATRUN)) {
			// combat music active
			return;
		}
	}

	if (queued_track) {
		queued_track = track;
		return;
	}

	playMusic_internal(track);
}

void MusicProcess::playCombatMusic(int track) {
	playMusic_internal(track);
}

void MusicProcess::queueMusic(int track) {
	if (wanted_track != track) {
		queued_track = track;
	}
}

void MusicProcess::unqueueMusic() {
	queued_track = 0;
}

void MusicProcess::restoreMusic() {
	queued_track = 0;
	playMusic_internal(last_request);
}



void MusicProcess::playMusic_internal(int track) {
	if (track < 0 || track > 128) {
		playMusic_internal(0);
		return;
	}

	// No current track if not playing
	if (driver && !driver->isSequencePlaying(0))
		wanted_track = current_track = 0;

	// It's already playing and we are not transitioning
	if (current_track == track && state == MUSIC_NORMAL) {
		return;
	} else if (current_track == 0 || state != MUSIC_NORMAL || !driver) {
		wanted_track = track;
		state = MUSIC_PLAY_WANTED;
	}
	// We want to do a transition
	else {
		const MusicFlex::SongInfo *info = GameData::get_instance()->getMusic()->getSongInfo(current_track);

		uint32 measure = driver->getSequenceCallbackData(0);

		// No transition info, or invalid measure, so fast change
		if (!info || (measure >= (uint32)info->num_measures) ||
		        !info->transitions[track] || !info->transitions[track][measure]) {
			current_track = 0;
			if (track == 0) {
				wanted_track = 0;
				state = MUSIC_PLAY_WANTED;
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
			driver->finishSequence(0);
			trans = trans - 1;
		}

		// Now get the transition midi
		int xmidi_index = driver->isFMSynth() ? 260 : 258;
		XMidiFile *xmidi = GameData::get_instance()->getMusic()->getXMidi(xmidi_index);
		XMidiEventList *list;

		if (xmidi) list = xmidi->GetEventList(trans);
		else list = 0;

		if (list) {
			driver->startSequence(1, list, false, 255, song_branches[track]);
			if (speed_hack) driver->setSequenceSpeed(1, 200);
		} else driver->finishSequence(1);
		wanted_track = track;

		state = MUSIC_TRANSITION;
	}
}

void MusicProcess::run() {
	switch (state) {
	case MUSIC_NORMAL:
		if (driver && !driver->isSequencePlaying(0) && queued_track) {
			wanted_track = queued_track;
			state = MUSIC_PLAY_WANTED;
			queued_track = 0;
		}

		break;

	case MUSIC_TRANSITION:
		if (!driver) {
			state = MUSIC_PLAY_WANTED;
		} else if (!driver->isSequencePlaying(1)) {
			state = MUSIC_PLAY_WANTED;
			driver->pauseSequence(0);
			driver->finishSequence(0);
		}
		break;

	case MUSIC_PLAY_WANTED: {
		if (driver) {
			driver->finishSequence(0);
			driver->finishSequence(1);
		}

		XMidiFile *xmidi = 0;
		if (wanted_track) {
			int xmidi_index = wanted_track;
			if (driver && driver->isFMSynth())
				xmidi_index += 128;

			xmidi = GameData::get_instance()->getMusic()->getXMidi(xmidi_index);
		}

		if (xmidi) {
			XMidiEventList *list = xmidi->GetEventList(0);
			if (song_branches[wanted_track] != -1) {
				XMidiEvent *event = list->findBranchEvent(song_branches[wanted_track]);
				if (!event) song_branches[wanted_track] = 0;
			}

			if (driver) {
				// if there's a track queued, only play this one once
				bool repeat = (queued_track == 0);
				driver->startSequence(0, list, repeat, 255, song_branches[wanted_track]);
			}
			current_track = wanted_track;
			song_branches[wanted_track]++;
		} else {
			current_track = wanted_track = 0;
		}
		state = MUSIC_NORMAL;
	}
	break;
	}
}

void MusicProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(wanted_track));
	ods->write4(static_cast<uint32>(last_request));
	ods->write4(static_cast<uint32>(queued_track));
}

bool MusicProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	wanted_track = static_cast<int32>(ids->read4());

	if (version >= 4) {
		last_request = static_cast<int32>(ids->read4());
		queued_track = static_cast<int32>(ids->read4());
	} else {
		last_request = wanted_track;
		queued_track = 0;
	}

	state = MUSIC_PLAY_WANTED;

	the_music_process = this;

	driver = Pentagram::AudioMixer::get_instance()->getMidiDriver();

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
