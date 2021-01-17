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

//=============================================================================
//
// ACSOUND - AGS sound system wrapper
//
//=============================================================================

#include "ags/shared/core/platform.h"
#include "ags/shared/util/wgt2allg.h"
#include "ags/engine/ac/file.h"
#include "ags/engine/media/audio/audiodefines.h"
#include "ags/engine/media/audio/sound.h"
#include "ags/engine/media/audio/audiointernaldefs.h"
#include "ags/engine/media/audio/clip_mywave.h"
#include "ags/engine/media/audio/soundcache.h"
#include "ags/engine/media/audio/clip_mymidi.h"
#include "ags/engine/util/mutex_lock.h"
#include "ags/ags.h"
#include "common/memstream.h"
#include "audio/mods/mod_xm_s3m.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"

namespace AGS3 {

#if defined JGMOD_MOD_PLAYER && defined DUMB_MOD_PLAYER
#error JGMOD_MOD_PLAYER and DUMB_MOD_PLAYER macros cannot be defined at the same time.
#endif

#if !defined PSP_NO_MOD_PLAYBACK && !defined JGMOD_MOD_PLAYER && !defined DUMB_MOD_PLAYER
#error Either JGMOD_MOD_PLAYER or DUMB_MOD_PLAYER should be defined.
#endif

int use_extra_sound_offset = 0;


SOUNDCLIP *my_load_wave(const AssetPath &asset_name, int voll, int loop) {
	Common::SeekableReadStream *data = get_cached_sound(asset_name);
	if (data) {
		Audio::AudioStream *audioStream = Audio::makeWAVStream(data, DisposeAfterUse::YES);
		return new SoundClipWave<MUS_WAVE>(audioStream, voll, loop);
	} else {
		return nullptr;
	}
}

SOUNDCLIP *my_load_static_mp3(const AssetPath &asset_name, int voll, bool loop) {
	Common::SeekableReadStream *data = get_cached_sound(asset_name);
	if (data) {
		Audio::AudioStream *audioStream = Audio::makeMP3Stream(data, DisposeAfterUse::YES);
		return new SoundClipWave<MUS_MP3>(audioStream, voll, false);
	} else {
		return nullptr;
	}
}

SOUNDCLIP *my_load_mp3(const AssetPath &asset_name, int voll) {
	return my_load_static_mp3(asset_name, voll, false);
}

SOUNDCLIP *my_load_static_ogg(const AssetPath &asset_name, int voll, bool loop) {
	Common::SeekableReadStream *data = get_cached_sound(asset_name);
	if (data) {
		Audio::AudioStream *audioStream = Audio::makeVorbisStream(data, DisposeAfterUse::YES);
		return new SoundClipWave<MUS_OGG>(audioStream, voll, loop);
	} else {
		return nullptr;
	}
}

SOUNDCLIP *my_load_ogg(const AssetPath &asset_name, int voll) {
	return my_load_static_ogg(asset_name, voll, false);
}

SOUNDCLIP *my_load_midi(const AssetPath &asset_name, bool repeat) {
	Common::SeekableReadStream *data = get_cached_sound(asset_name);
	if (data) {
		return new MYMIDI(data, repeat);
	} else {
		return nullptr;
	}
}

SOUNDCLIP *my_load_mod(const AssetPath &asset_name, bool repeat) {
	Common::SeekableReadStream *data = get_cached_sound(asset_name);
	if (data) {
		Audio::AudioStream *audioStream = Audio::makeModXmS3mStream(data, DisposeAfterUse::YES);
		return new SoundClipWave<MUS_OGG>(audioStream, 255, repeat);
	} else {
		return nullptr;
	}
}

int init_mod_player(int numVoices) {
	return 0;
}

void remove_mod_player() {
}

} // namespace AGS3
