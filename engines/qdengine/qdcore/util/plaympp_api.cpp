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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/stream.h"

#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/mpc.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_file_manager.h"
#include "qdengine/qdcore/util/plaympp_api.h"

namespace QDEngine {

mpegPlayer::mpegPlayer() : _is_enabled(true),
	_volume(255),
	_cur_track_volume(255),
	_paused(false) {
}

mpegPlayer::~mpegPlayer() {
}

bool mpegPlayer::play(const Common::Path file, bool loop, int vol) {
	bool isOGG = file.baseName().hasSuffixIgnoreCase(".ogg");

	debugC(1, kDebugSound, "mpegPlayer::play(%s, %d, %d)", file.toString().c_str(), loop, vol);

	_file = file;

	stop();

	Common::SeekableReadStream *stream;

	if (qdFileManager::instance().open_file(&stream, file, false)) {
		Audio::SeekableAudioStream *audiostream;

		if (isOGG) {
#ifdef USE_VORBIS
			audiostream = Audio::makeVorbisStream(stream, DisposeAfterUse::YES);
#else
			warning("mpegPlayer::play(%s, %d, %d): Vorbis support not compiled", file.toString().c_str(), loop, vol);
			return false;
#endif
		} else {
#ifdef USE_MPCDEC
			audiostream = Audio::makeMPCStream(stream, DisposeAfterUse::YES);
#else
			warning("mpegPlayer::play(%s, %d, %d): MPC support not compiled", file.toString().c_str(), loop, vol);
			return false;
#endif
		}

		if (!loop) {
			g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, audiostream);
		} else {
			Audio::LoopingAudioStream *looped = new Audio::LoopingAudioStream(audiostream, 0, DisposeAfterUse::YES);

			g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, looped);
		}
	}

	set_volume(vol);

	_paused = false;

	debugC(1, kDebugSound, "mpegPlayer::play(%s)", _file.toString().c_str());

	return true;
}

bool mpegPlayer::stop() {
	debugC(1, kDebugSound, "mpegPlayer::stop(%s)", _file.toString().c_str());

	g_system->getMixer()->stopHandle(_soundHandle);
	return true;
}

bool mpegPlayer::pause() {
	if (_paused)
		return true;

	debugC(1, kDebugSound, "mpegPlayer::pause(%s)", _file.toString().c_str());

	g_system->getMixer()->pauseHandle(_soundHandle, true);
	_paused = true;
	return true;
}

bool mpegPlayer::resume() {
	if (!_paused)
		return true;

	debugC(1, kDebugSound, "mpegPlayer::resume(%s)", _file.toString().c_str());

	g_system->getMixer()->pauseHandle(_soundHandle, false);
	_paused = false;

	g_system->getMixer()->setChannelVolume(_soundHandle, _volume * _cur_track_volume / 256);

	return true;
}

mpegPlayer::mpeg_status_t mpegPlayer::status() const {
	mpeg_status_t res;

	if (!g_system->getMixer()->isReady())
		res = MPEG_STOPPED;
	else if (_paused)
		res = MPEG_PAUSED;
	else if (g_system->getMixer()->isSoundHandleActive(_soundHandle))
		res = MPEG_PLAYING;
	else
		res = MPEG_STOPPED;

	debugC(7, kDebugSound, "mpegPlayer::status(%s), status: %d", _file.toString().c_str(), res);

	return res;
}

void mpegPlayer::set_volume(uint32 vol) {
	debugC(1, kDebugSound, "mpegPlayer::set_volume(%s), vol: %d", _file.toString().c_str(), vol);

	_volume = vol;

	g_system->getMixer()->setChannelVolume(_soundHandle, _volume * _cur_track_volume / 256);
}

bool mpegPlayer::init_library(void *dsound_device) {
	return true;
}

void mpegPlayer::deinit_library() {
}

mpegPlayer *g_mpegPlayer = nullptr;

mpegPlayer &mpegPlayer::instance() {
	if (!g_mpegPlayer)
		g_mpegPlayer = new mpegPlayer;

	return *g_mpegPlayer;
}

void mpegPlayer::syncMusicSettings() {
	set_volume(ConfMan.getInt("music_volume"));

	if (ConfMan.getBool("enable_music"))
		enable();
	else
		disable();
}

} // namespace QDEngine
