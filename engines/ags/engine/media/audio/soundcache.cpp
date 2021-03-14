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
#include "ags/engine/util/mutex.h"
#include "ags/engine/util/mutex_lock.h"
#include "ags/shared/util/string.h"
#include "ags/shared/debugging/out.h"
#include "ags/globals.h"
#include "ags/ags.h"
#include "common/memstream.h"

namespace AGS3 {

using namespace Shared;

void clear_sound_cache() {
	AGS::Engine::MutexLock _lock(::AGS::g_vm->_soundCacheMutex);

	if (_G(sound_cache_entries)) {
		int i;
		for (i = 0; i < _G(psp_audio_cachesize); i++) {
			if (_G(sound_cache_entries)[i].data) {
				free(_G(sound_cache_entries)[i].data);
				_G(sound_cache_entries)[i].data = nullptr;
				free(_G(sound_cache_entries)[i].file_name);
				_G(sound_cache_entries)[i].file_name = nullptr;
				_G(sound_cache_entries)[i].reference = 0;
			}
		}
	} else {
		_G(sound_cache_entries) = (sound_cache_entry_t *)malloc(_G(psp_audio_cachesize) * sizeof(sound_cache_entry_t));
		memset(_G(sound_cache_entries), 0, _G(psp_audio_cachesize) * sizeof(sound_cache_entry_t));
	}
}

void sound_cache_free(char *buffer) {
	AGS::Engine::MutexLock _lock(::AGS::g_vm->_soundCacheMutex);

#ifdef SOUND_CACHE_DEBUG
	Debug::Printf("sound_cache_free(%p)\n", buffer);
#endif
	int i;
	for (i = 0; i < _G(psp_audio_cachesize); i++) {
		if (_G(sound_cache_entries)[i].data == buffer) {
			if (_G(sound_cache_entries)[i].reference > 0)
				_G(sound_cache_entries)[i].reference--;

#ifdef SOUND_CACHE_DEBUG
			Debug::Printf("..decreased reference count of slot %d to %d\n", i, _G(sound_cache_entries)[i].reference);
#endif
			return;
		}
	}

#ifdef SOUND_CACHE_DEBUG
	Debug::Printf("..freeing uncached sound\n");
#endif

	// Sound is uncached
	if (i == _G(psp_audio_cachesize)) {
		free(buffer);
	}
}


char *get_cached_sound(const AssetPath &asset_name, size_t &size) {
	AGS::Engine::MutexLock _lock(::AGS::g_vm->_soundCacheMutex);

#ifdef SOUND_CACHE_DEBUG
	Debug::Printf("get_cached_sound(%s)\n", asset_name.first.GetCStr());
#endif

	size = 0;

	int i;
	for (i = 0; i < _G(psp_audio_cachesize); i++) {
		if (_G(sound_cache_entries)[i].data == nullptr)
			continue;

		if (strcmp(asset_name.second, _G(sound_cache_entries)[i].file_name) == 0) {
#ifdef SOUND_CACHE_DEBUG
			Debug::Printf("..found in slot %d\n", i);
#endif
			_G(sound_cache_entries)[i].reference++;
			_G(sound_cache_entries)[i].last_used = _G(sound_cache_counter)++;
			size = _G(sound_cache_entries)[i].size;

			return _G(sound_cache_entries)[i].data;
		}
	}

	// Not found
	PACKFILE *mp3in = nullptr;

	mp3in = PackfileFromAsset(asset_name, size);
	if (mp3in == nullptr) {
		return nullptr;
	}

	// Find free slot
	for (i = 0; i < _G(psp_audio_cachesize); i++) {
		if (_G(sound_cache_entries)[i].data == nullptr)
			break;
	}

	// No free slot?
	if (i == _G(psp_audio_cachesize)) {
		unsigned int oldest = _G(sound_cache_counter);
		int index = -1;

		for (i = 0; i < _G(psp_audio_cachesize); i++) {
			if (_G(sound_cache_entries)[i].reference == 0) {
				if (_G(sound_cache_entries)[i].last_used < oldest) {
					oldest = _G(sound_cache_entries)[i].last_used;
					index = i;
				}
			}
		}

		i = index;
	}

	// Load new file
	char *newdata;

	newdata = (char *)malloc(size);

	if (newdata == nullptr) {
		pack_fclose(mp3in);
		return nullptr;
	}

	pack_fread(newdata, size, mp3in);
	pack_fclose(mp3in);

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

		if (_G(sound_cache_entries)[i].data) {
			free(_G(sound_cache_entries)[i].data);
		}

		_G(sound_cache_entries)[i].size = size;
		_G(sound_cache_entries)[i].data = newdata;

		if (_G(sound_cache_entries)[i].file_name)
			free(_G(sound_cache_entries)[i].file_name);
		_G(sound_cache_entries)[i].file_name = (char *)malloc(strlen(asset_name.second) + 1);
		strcpy(_G(sound_cache_entries)[i].file_name, asset_name.second);
		_G(sound_cache_entries)[i].reference = 1;
		_G(sound_cache_entries)[i].last_used = _G(sound_cache_counter)++;

		return _G(sound_cache_entries)[i].data;
	}
}

Common::SeekableReadStream *get_cached_sound(const AssetPath &asset_name) {
	size_t muslen = 0;
	const byte *data = (byte *)get_cached_sound(asset_name, muslen);
	if (data == nullptr)
		return nullptr;

	// Create a read stream for the sound
	return new Common::MemoryReadStream(data, muslen, DisposeAfterUse::NO);
}

} // namespace AGS3
