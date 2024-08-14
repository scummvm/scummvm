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

bool mpegPlayer::play(const char *file, bool loop, int vol) {
	bool isOGG = Common::String(file).hasSuffix(".ogg");

	debugC(1, kDebugSound, "mpegPlayer::play(%s, %d, %d)", file, loop, vol);

	_file = file;

	stop();

	if (qdFileManager::instance().open_file(&_stream, file, false)) {
		Audio::SeekableAudioStream *audiostream;

		if (isOGG) {
#ifdef USE_VORBIS
			audiostream = Audio::makeVorbisStream(_stream, DisposeAfterUse::YES);
#else
			warning("mpegPlayer::play(: Vorbis support not compiled", file, loop, vol);
			return false;
#endif
		} else {
#ifdef USE_MPCDEC
			audiostream = Audio::makeMPCStream(_stream, DisposeAfterUse::YES);
#else
			warning("mpegPlayer::play(%s, %d, %d): MPC support not compiled", file, loop, vol);
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

	return true;
}

bool mpegPlayer::stop() {
	debugC(1, kDebugSound, "mpegPlayer::stop(%s)", _file.c_str());
	g_system->getMixer()->stopHandle(_soundHandle);
	return true;
}

bool mpegPlayer::pause() {
	debugC(1, kDebugSound, "mpegPlayer::pause(%s)", _file.c_str());

	g_system->getMixer()->pauseHandle(_soundHandle, true);
	_paused = true;
	return true;
}

bool mpegPlayer::resume() {
	debugC(1, kDebugSound, "mpegPlayer::resume(%s)", _file.c_str());

	g_system->getMixer()->pauseHandle(_soundHandle, false);
	_paused = false;

	g_system->getMixer()->setChannelVolume(_soundHandle, _volume * _cur_track_volume / 256);

	return true;
}

mpegPlayer::mpeg_status_t mpegPlayer::status() const {
	if (!g_system->getMixer()->isReady())
		return MPEG_STOPPED;

	if (_paused)
		return MPEG_PAUSED;

	if (g_system->getMixer()->isSoundHandleActive(_soundHandle))
		return MPEG_PLAYING;
	else
		return MPEG_STOPPED;
}

void mpegPlayer::set_volume(uint32 vol) {
	_volume = vol;

	g_system->getMixer()->setChannelVolume(_soundHandle, _volume * _cur_track_volume / 256);
}

bool mpegPlayer::init_library(void *dsound_device) {
	return true;
}

void mpegPlayer::deinit_library() {
}

mpegPlayer &mpegPlayer::instance() {
	static mpegPlayer player;
	return player;
}

} // namespace QDEngine
