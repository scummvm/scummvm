/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/system.h"
#include "common/config-manager.h"

#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/audio/cru_music_process.h"
#include "audio/mods/mod_xm_s3m.h"

#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"

namespace Ultima {
namespace Ultima8 {

static const int MAX_TRACK_REMORSE = 21;
static const int MAX_TRACK_REGRET = 22;

// NOTE: The order of these lists has to be the same as the original games
// as they come as numbers from the usecode.
static const char *const TRACK_FILE_NAMES_REMORSE[] = {
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

static const char *const TRACK_FILE_NAMES_REGRET[] = {
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
	"xmas" // for christmas easter egg
};

static const int REGRET_MAP_TRACKS[] = {
	 0,  1, 10,  2,  0,  3, 11,  4,
	16,  5, 20,  6,  0,  7, 13,  8,
	15,  9, 12, 10, 19, 14, 21,  0};


DEFINE_RUNTIME_CLASSTYPE_CODE(CruMusicProcess)

CruMusicProcess::CruMusicProcess() : MusicProcess(), _currentTrack(0), _savedTrack(0), _m16offset(0) {
	_maxTrack = (GAME_IS_REMORSE ? MAX_TRACK_REMORSE : MAX_TRACK_REGRET);
	_trackNames = (GAME_IS_REMORSE ? TRACK_FILE_NAMES_REMORSE
				   : TRACK_FILE_NAMES_REGRET);
}

CruMusicProcess::~CruMusicProcess() {
	// We shouldn't need to do anything here - the mixer will
	// clean up the stream for us.
}

void CruMusicProcess::playMusic(int track) {
	if (GAME_IS_REGRET && track == 0x45) {
		// Play the default track for the current map
		uint32 curmap = World::get_instance()->getCurrentMap()->getNum();
		if (curmap < ARRAYSIZE(REGRET_MAP_TRACKS)) {
			track = REGRET_MAP_TRACKS[curmap];
		} else {
			track = 0;
		}

		// Regret has a Christmas music easter egg.
		if (!GAME_IS_DEMO) {
			TimeDate t;
			g_system->getTimeAndDate(t);
			if ((t.tm_mon == 11 && t.tm_mday >= 24) || ConfMan.getBool("always_christmas")) {
				track = 22;
			}
		}
	}
	playMusic_internal(track);
}

void CruMusicProcess::playCombatMusic(int track) {
	// Only U8 has combat music.. ignore it.
}

void CruMusicProcess::queueMusic(int track) {
	playMusic_internal(track);
}

void CruMusicProcess::unqueueMusic() {

}

void CruMusicProcess::restoreMusic() {

}

void CruMusicProcess::saveTrackState() {
	assert(!_savedTrack);
	_savedTrack = _currentTrack;
}

void CruMusicProcess::restoreTrackState() {
	int saved = _savedTrack;
	_savedTrack = 0;
	playMusic_internal(saved);
}

void CruMusicProcess::playMusic_internal(int track) {
	if (track < 0 || track > _maxTrack) {
		warning("Not playing track %d (max is %d)", track, _maxTrack);
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
		const Std::string fname = Std::string::format("sound/%s.amf", _trackNames[track]);
		auto *rs = new Common::File();
		if (!rs->open(Common::Path(fname))) {
			// This happens in No Regret demo.
			warning("Couldn't load AMF file: %s", fname.c_str());
			delete rs;
			return;
		}

		Audio::AudioStream *stream = Audio::makeModXmS3mStream(rs, DisposeAfterUse::YES);
		if (!stream) {
			error("Couldn't create stream from AMF file: %s", fname.c_str());
			return;
		}
		mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	}
}

void CruMusicProcess::run() {
	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	assert(mixer);
	if (mixer->isSoundHandleActive(_soundHandle)) {
		return;
	}

	// Hit end of stream, play it again.  This normally won't happen because
	// the mods should loop infinitely, but just in case.
	playMusic_internal(_currentTrack);
}

void CruMusicProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_currentTrack));
	ws->writeUint32LE(static_cast<uint32>(_savedTrack));
	ws->writeByte(_m16offset);
}

bool CruMusicProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_currentTrack = static_cast<int32>(rs->readUint32LE());
	_savedTrack = static_cast<int32>(rs->readUint32LE());
	_m16offset = rs->readByte();

	_theMusicProcess = this;

	// Slight hack - resuming from savegame we want to restore the game
	// track (not the menu track)
	if (_savedTrack)
		restoreTrackState();

	return true;
}

bool CruMusicProcess::isPlaying() {
	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	return _currentTrack != 0 && mixer && mixer->isSoundHandleActive(_soundHandle);
}

void CruMusicProcess::pauseMusic() {
	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	assert(mixer);
	if (mixer->isSoundHandleActive(_soundHandle))
		mixer->pauseHandle(_soundHandle, true);
}

void CruMusicProcess::unpauseMusic() {
	Audio::Mixer *mixer = Ultima8Engine::get_instance()->_mixer;
	assert(mixer);
	if (mixer->isSoundHandleActive(_soundHandle))
		mixer->pauseHandle(_soundHandle, false);
}



} // End of namespace Ultima8
} // End of namespace Ultima
