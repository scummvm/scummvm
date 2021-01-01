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
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/audio/remorse_music_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/audio/music_flex.h"
#include "ultima/ultima8/audio/midi_player.h"
#include "ultima/ultima8/audio/audio_mixer.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "audio/mods/mod_xm_s3m.h"

namespace Ultima {
namespace Ultima8 {

static const int MAX_TRACK_REMORSE = 21;
static const int MAX_TRACK_REGRET = 22;

// NOTE: This order is chosen to match the list in Crusader: No Remorse.
static const char *TRACK_FILE_NAMES_REMORSE[] = {
	nullptr,
	"M01",
	"M02",
	"M03",
	"M04",
	"M05",
	"M06",
	"M07",
	"M08",
	"M09",
	"M10",
	"M11",
	"M12",
	"M13",
	"M14",
	"M15",
	"M16A",
	"M16B",
	"M16C",
	"cred",
	"menu",
	"buyme" // for demo
};

static const char *TRACK_FILE_NAMES_REGRET[] = {
	nullptr,
	"ninth",
	"phil",
	"straight",
	"party",
	"demo",
	"stint",
	"mk",
	"space2",
	"d3",
	"space",
	"rhythm",
	"intent",
	"m03",
	"silver",
	"m01",
	"techno",
	"cred",
	"regret",
	"m13",
	"retro",
	"metal",
	"xmas" // for demo
};


// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(RemorseMusicProcess)

RemorseMusicProcess::RemorseMusicProcess() : MusicProcess(), _currentTrack(0), _savedTrack(0), _m16offset(0), _combatMusicActive(false) {
	_maxTrack = (GAME_IS_REMORSE ? MAX_TRACK_REMORSE : MAX_TRACK_REGRET);
	_trackNames = (GAME_IS_REMORSE ? TRACK_FILE_NAMES_REMORSE
				   : TRACK_FILE_NAMES_REGRET);
}

RemorseMusicProcess::~RemorseMusicProcess() {
	// We shouldn't need to do anything here - the mixer will
	// clean up the stream for us.
}

void RemorseMusicProcess::playMusic(int track) {
	playMusic_internal(track);
}

void RemorseMusicProcess::playCombatMusic(int track) {
	// Only U8 has combat music.. ignore it.
}

void RemorseMusicProcess::queueMusic(int track) {
	playMusic_internal(track);
}

void RemorseMusicProcess::unqueueMusic() {

}

void RemorseMusicProcess::restoreMusic() {

}

void RemorseMusicProcess::saveTrackState() {
	assert(!_savedTrack);
	_savedTrack = _currentTrack;
}

void RemorseMusicProcess::restoreTrackState() {
	_currentTrack = _savedTrack;
	_savedTrack = 0;
	playMusic_internal(_currentTrack);
}

void RemorseMusicProcess::playMusic_internal(int track) {
	if (track < 0 || track > _maxTrack) {
		playMusic_internal(0);
		return;
	}

	if (GAME_IS_REMORSE && track == 16) {
		// Loop through m16a / m16b / m16c
		track += _m16offset;
		_m16offset = (_m16offset + 1) % 4;
	}

	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	assert(mixer);

	if (track == _currentTrack && (track == 0 || mixer->isSoundHandleActive(_soundHandle)))
		// Already playing what we want.
		return;

	mixer->stopHandle(_soundHandle);
	_soundHandle = Audio::SoundHandle();
	_currentTrack = track;

	if (track > 0) {
		// TODO: It's a bit ugly having this here.  Should be in GameData.
		const Std::string fname = Std::string::format("@game/sound/%s.amf", _trackNames[track]);
		FileSystem *filesystem = FileSystem::get_instance();
		assert(filesystem);
		Common::SeekableReadStream *rs = filesystem->ReadFile(fname);
		if (!rs) {
			error("Couldn't load AMF file: %s", fname.c_str());
			return;
		}

		Audio::AudioStream *stream = Audio::makeModXmS3mStream(rs, DisposeAfterUse::NO);
		if (!stream) {
			error("Couldn't create stream from AMF file: %s", fname.c_str());
			return;
		}
		mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	}
}

void RemorseMusicProcess::run() {
	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	assert(mixer);
	if (mixer->isSoundHandleActive(_soundHandle)) {
		return;
	}

	// hit end of stream, play it again.
	// TODO: This doesn't loop to the correct spot, should do something a bit nicer..
	playMusic_internal(_currentTrack);
}

void RemorseMusicProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_currentTrack));
}

bool RemorseMusicProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_currentTrack = static_cast<int32>(rs->readUint32LE());

	return true;
}

bool RemorseMusicProcess::isPlaying() {
	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	return _currentTrack != 0 && mixer && mixer->isSoundHandleActive(_soundHandle);
}

void RemorseMusicProcess::pauseMusic() {
	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	assert(mixer);
	if (mixer->isSoundHandleActive(_soundHandle))
		mixer->pauseHandle(_soundHandle, true);
}

void RemorseMusicProcess::unpauseMusic() {
	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	assert(mixer);
	if (mixer->isSoundHandleActive(_soundHandle))
		mixer->pauseHandle(_soundHandle, false);
}



} // End of namespace Ultima8
} // End of namespace Ultima
