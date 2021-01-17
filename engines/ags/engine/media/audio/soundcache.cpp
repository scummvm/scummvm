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

#include "ags/engine/ac/file.h"
#include "ags/shared/util/wgt2allg.h"
#include "ags/engine/media/audio/soundcache.h"
#include "ags/engine/media/audio/audiointernaldefs.h"
#include "ags/engine/util/mutex.h"
#include "ags/engine/util/mutex_lock.h"
#include "ags/shared/util/string.h"
#include "ags/shared/debugging/out.h"
#include "ags/ags.h"
#include "common/memstream.h"

namespace AGS3 {

using namespace Shared;

sound_cache_entry_t *sound_cache_entries = nullptr;
unsigned int sound_cache_counter = 0;

void clear_sound_cache() {
	AGS::Engine::MutexLock _lock(::AGS::g_vm->_soundCacheMutex);

	if (sound_cache_entries) {
		int i;
		for (i = 0; i < psp_audio_cachesize; i++) {
			if (sound_cache_entries[i].data) {
				free(sound_cache_entries[i].data);
				sound_cache_entries[i].data = nullptr;
				free(sound_cache_entries[i].file_name);
				sound_cache_entries[i].file_name = nullptr;
				sound_cache_entries[i].reference = 0;
			}
		}
	} else {
		sound_cache_entries = (sound_cache_entry_t *)malloc(psp_audio_cachesize * sizeof(sound_cache_entry_t));
		memset(sound_cache_entries, 0, psp_audio_cachesize * sizeof(sound_cache_entry_t));
	}
}

void sound_cache_free(char *buffer, bool is_wave) {
	AGS::Engine::MutexLock _lock(::AGS::g_vm->_soundCacheMutex);

#ifdef SOUND_CACHE_DEBUG
	Debug::Printf("sound_cache_free(%p %d)\n", buffer, (unsigned int)is_wave);
#endif
	int i;
	for (i = 0; i < psp_audio_cachesize; i++) {
		if (sound_cache_entries[i].data == buffer) {
			if (sound_cache_entries[i].reference > 0)
				sound_cache_entries[i].reference--;

#ifdef SOUND_CACHE_DEBUG
			Debug::Printf("..decreased reference count of slot %d to %d\n", i, sound_cache_entries[i].reference);
#endif
			return;
		}
	}

#ifdef SOUND_CACHE_DEBUG
	Debug::Printf("..freeing uncached sound\n");
#endif

	// Sound is uncached
	if (i == psp_audio_cachesize) {
		if (is_wave)
			destroy_sample((SAMPLE *)buffer);
		else
			free(buffer);
	}
}


char *get_cached_sound(const AssetPath &asset_name, bool is_wave, size_t &size) {
	AGS::Engine::MutexLock _lock(::AGS::g_vm->_soundCacheMutex);

#ifdef SOUND_CACHE_DEBUG
	Debug::Printf("get_cached_sound(%s %d)\n", asset_name.first.GetCStr(), (unsigned int)is_wave);
#endif

	size = 0;

	int i;
	for (i = 0; i < psp_audio_cachesize; i++) {
		if (sound_cache_entries[i].data == nullptr)
			continue;

		if (strcmp(asset_name.second, sound_cache_entries[i].file_name) == 0) {
#ifdef SOUND_CACHE_DEBUG
			Debug::Printf("..found in slot %d\n", i);
#endif
			sound_cache_entries[i].reference++;
			sound_cache_entries[i].last_used = sound_cache_counter++;
			size = sound_cache_entries[i].size;

			return sound_cache_entries[i].data;
		}
	}

	// Not found
	PACKFILE *mp3in = nullptr;
	SAMPLE *wave = nullptr;

	if (is_wave) {
		PACKFILE *wavin = PackfileFromAsset(asset_name, size);
		if (wavin != nullptr) {
			wave = load_wav_pf(wavin);
			pack_fclose(wavin);
		}
	} else {
		mp3in = PackfileFromAsset(asset_name, size);
		if (mp3in == nullptr) {
			return nullptr;
		}
	}

	// Find free slot
	for (i = 0; i < psp_audio_cachesize; i++) {
		if (sound_cache_entries[i].data == nullptr)
			break;
	}

	// No free slot?
	if (i == psp_audio_cachesize) {
		unsigned int oldest = sound_cache_counter;
		int index = -1;

		for (i = 0; i < psp_audio_cachesize; i++) {
			if (sound_cache_entries[i].reference == 0) {
				if (sound_cache_entries[i].last_used < oldest) {
					oldest = sound_cache_entries[i].last_used;
					index = i;
				}
			}
		}

		i = index;
	}

	// Load new file
	char *newdata;

	if (is_wave) {
		size = 0; // ??? CHECKME
		newdata = (char *)wave;
	} else {
		newdata = (char *)malloc(size);

		if (newdata == nullptr) {
			pack_fclose(mp3in);
			return nullptr;
		}

		pack_fread(newdata, size, mp3in);
		pack_fclose(mp3in);
	}

	if (i == -1) {
		// No cache slot empty, return uncached data
#ifdef SOUND_CACHE_DEBUG
		Debug::Printf("..loading uncached\n");
#endif
		return newdata;
	} else {
		// Add to cache, free old sound first
#ifdef SOUND_CACHE_DEBUG
		Debug::Printf("..loading cached in slot %d\n", i);
#endif

		if (sound_cache_entries[i].data) {
			if (sound_cache_entries[i].is_wave)
				destroy_sample((SAMPLE *)sound_cache_entries[i].data);
			else
				free(sound_cache_entries[i].data);
		}

		sound_cache_entries[i].size = size;
		sound_cache_entries[i].data = newdata;

		if (sound_cache_entries[i].file_name)
			free(sound_cache_entries[i].file_name);
		sound_cache_entries[i].file_name = (char *)malloc(strlen(asset_name.second) + 1);
		strcpy(sound_cache_entries[i].file_name, asset_name.second);
		sound_cache_entries[i].reference = 1;
		sound_cache_entries[i].last_used = sound_cache_counter++;
		sound_cache_entries[i].is_wave = is_wave;

		return sound_cache_entries[i].data;
	}
}

Common::SeekableReadStream *get_cached_sound(const AssetPath &asset_name) {
	size_t muslen = 0;
	const byte *data = (byte *)get_cached_sound(asset_name, false, muslen);
	if (data == nullptr)
		return nullptr;

	// Create a read stream for the sound
	return new Common::MemoryReadStream(data, muslen, DisposeAfterUse::YES);
}

} // namespace AGS3
