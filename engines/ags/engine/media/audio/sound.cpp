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

#include "ags/engine/media/audio/audiodefines.h"
#include "ags/engine/media/audio/sound.h"
#include "ags/engine/media/audio/soundcache.h"
#include "ags/engine/media/audio/soundclip.h"
#include "ags/engine/media/audio/clip_mymidi.h"
#include "audio/mods/mod_xm_s3m.h"
#include "audio/mods/protracker.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"

namespace AGS3 {

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
#ifdef USE_MAD
	Common::SeekableReadStream *data = get_cached_sound(asset_name);
	if (data) {
		Audio::AudioStream *audioStream = Audio::makeMP3Stream(data, DisposeAfterUse::YES);
		return new SoundClipWave<MUS_MP3>(audioStream, voll, false);
	} else {
		return nullptr;
	}
#else
	return nullptr;
#endif
}

SOUNDCLIP *my_load_mp3(const AssetPath &asset_name, int voll) {
	return my_load_static_mp3(asset_name, voll, false);
}

SOUNDCLIP *my_load_static_ogg(const AssetPath &asset_name, int voll, bool loop) {
#ifdef USE_VORBIS
	Common::SeekableReadStream *data = get_cached_sound(asset_name);
	if (data) {
		Audio::AudioStream *audioStream = Audio::makeVorbisStream(data, DisposeAfterUse::YES);
		return new SoundClipWave<MUS_OGG>(audioStream, voll, loop);
	} else {
		return nullptr;
	}
#else
	return nullptr;
#endif
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
		// determine the file extension
		size_t lastDot = asset_name.second.FindCharReverse('.');
		if (lastDot == AGS::Shared::String::npos || lastDot == asset_name.second.GetLength() - 1) {
			delete data;
			return nullptr;
		}
		// get the first char of the extensin
		char charAfterDot = toupper(asset_name.second[lastDot + 1]);

		// use the appropriate loader
		Audio::AudioStream *audioStream = nullptr;
		if (charAfterDot == 'I') {
			// Impulse Tracker
			warning("Impulse Tracker MOD files not yet supported");
			delete data;
			return nullptr;
		} else if (charAfterDot == 'X') {
			audioStream = Audio::makeModXmS3mStream(data, DisposeAfterUse::YES);
		} else if (charAfterDot == 'S') {
			audioStream = Audio::makeModXmS3mStream(data, DisposeAfterUse::YES);
		} else if (charAfterDot == 'M') {
			audioStream = Audio::makeProtrackerStream(data, DisposeAfterUse::YES);
		} else {
			warning("MOD file format not recognized");
			delete data;
			return nullptr;
		}

		return new SoundClipWave<MUS_MOD>(audioStream, 255, repeat);
	} else {
		return nullptr;
	}
}

} // namespace AGS3
