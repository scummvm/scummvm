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

//include <ctype.h> // for toupper

#include "ags/shared/core/platform.h"
#include "ags/shared/util/wgt2allg.h"
#include "ags/engine/ac/file.h"
#include "ags/engine/media/audio/audiodefines.h"
#include "ags/engine/media/audio/sound.h"
#include "ags/engine/media/audio/audiointernaldefs.h"
#include "ags/engine/media/audio/clip_mywave.h"
#ifndef NO_MP3_PLAYER
#include "ags/engine/media/audio/clip_mymp3.h"
#include "ags/engine/media/audio/clip_mystaticmp3.h"
#endif
#include "ags/engine/media/audio/clip_myogg.h"
#include "ags/engine/media/audio/clip_mystaticogg.h"
#include "ags/engine/media/audio/clip_mymidi.h"
#ifdef JGMOD_MOD_PLAYER
#include "ags/engine/media/audio/clip_myjgmod.h"
#endif
#ifdef DUMB_MOD_PLAYER
#include "ags/engine/media/audio/clip_mydumbmod.h"
#endif
#include "ags/engine/media/audio/soundcache.h"
#include "ags/engine/util/mutex_lock.h"
#include "ags/ags.h"

namespace AGS3 {

#if defined JGMOD_MOD_PLAYER && defined DUMB_MOD_PLAYER
#error JGMOD_MOD_PLAYER and DUMB_MOD_PLAYER macros cannot be defined at the same time.
#endif

#if !defined PSP_NO_MOD_PLAYBACK && !defined JGMOD_MOD_PLAYER && !defined DUMB_MOD_PLAYER
#error Either JGMOD_MOD_PLAYER or DUMB_MOD_PLAYER should be defined.
#endif

// Load MIDI from PACKFILE stream
extern MIDI *load_midi_pf(PACKFILE *pf);


int use_extra_sound_offset = 0;



MYWAVE *thiswave;
SOUNDCLIP *my_load_wave(const AssetPath &asset_name, int voll, int loop) {
	// Load via soundcache.
	size_t dummy;
	SAMPLE *new_sample = (SAMPLE *)get_cached_sound(asset_name, true, dummy);

	if (new_sample == nullptr)
		return nullptr;

	thiswave = new MYWAVE();
	thiswave->wave = new_sample;
	thiswave->vol = voll;
	thiswave->repeat = (loop != 0);

	return thiswave;
}

PACKFILE *mp3in;

#ifndef NO_MP3_PLAYER

MYMP3 *thistune;

SOUNDCLIP *my_load_mp3(const AssetPath &asset_name, int voll) {
	size_t asset_size;
	mp3in = PackfileFromAsset(asset_name, asset_size);
	if (mp3in == nullptr)
		return nullptr;

	char *tmpbuffer = (char *)malloc(MP3CHUNKSIZE);
	if (tmpbuffer == nullptr) {
		pack_fclose(mp3in);
		return nullptr;
	}
	thistune = new MYMP3();
	thistune->in = mp3in;
	thistune->chunksize = MP3CHUNKSIZE;
	thistune->filesize = asset_size;
	thistune->vol = voll;

	if (thistune->chunksize > (int)thistune->filesize)
		thistune->chunksize = thistune->filesize;

	pack_fread(tmpbuffer, thistune->chunksize, mp3in);

	thistune->buffer = (char *)tmpbuffer;

	{
		AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
		thistune->stream = almp3_create_mp3stream(tmpbuffer, thistune->chunksize, (thistune->filesize < 1));
	}

	if (thistune->stream == nullptr) {
		free(tmpbuffer);
		pack_fclose(mp3in);
		delete thistune;
		return nullptr;
	}

	return thistune;
}



MYSTATICMP3 *thismp3;
SOUNDCLIP *my_load_static_mp3(const AssetPath &asset_name, int voll, bool loop) {
	// Load via soundcache.
	size_t muslen = 0;
	char *mp3buffer = get_cached_sound(asset_name, false, muslen);
	if (mp3buffer == nullptr)
		return nullptr;

	// now, create an MP3 structure for it
	thismp3 = new MYSTATICMP3();
	if (thismp3 == nullptr) {
		free(mp3buffer);
		return nullptr;
	}
	thismp3->vol = voll;
	thismp3->mp3buffer = nullptr;
	thismp3->repeat = loop;

	{
		AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
		thismp3->tune = almp3_create_mp3(mp3buffer, muslen);
	}

	if (thismp3->tune == nullptr) {
		free(mp3buffer);
		delete thismp3;
		return nullptr;
	}

	thismp3->mp3buffer = mp3buffer;

	return thismp3;
}

#else // NO_MP3_PLAYER

SOUNDCLIP *my_load_mp3(const AssetPath &asset_name, int voll) {
	return NULL;
}

SOUNDCLIP *my_load_static_mp3(const AssetPath &asset_name, int voll, bool loop) {
	return NULL;
}

#endif // NO_MP3_PLAYER



MYSTATICOGG *thissogg;
SOUNDCLIP *my_load_static_ogg(const AssetPath &asset_name, int voll, bool loop) {
	// Load via soundcache.
	size_t muslen = 0;
	char *mp3buffer = get_cached_sound(asset_name, false, muslen);
	if (mp3buffer == nullptr)
		return nullptr;

	// now, create an OGG structure for it
	thissogg = new MYSTATICOGG();
	thissogg->vol = voll;
	thissogg->repeat = loop;
	thissogg->mp3buffer = mp3buffer;
	thissogg->mp3buffersize = muslen;

	thissogg->tune = alogg_create_ogg_from_buffer(mp3buffer, muslen);

	if (thissogg->tune == nullptr) {
		thissogg->destroy();
		delete thissogg;
		return nullptr;
	}

	return thissogg;
}

MYOGG *thisogg;
SOUNDCLIP *my_load_ogg(const AssetPath &asset_name, int voll) {
	size_t asset_size;
	mp3in = PackfileFromAsset(asset_name, asset_size);
	if (mp3in == nullptr)
		return nullptr;

	char *tmpbuffer = (char *)malloc(MP3CHUNKSIZE);
	if (tmpbuffer == nullptr) {
		pack_fclose(mp3in);
		return nullptr;
	}

	thisogg = new MYOGG();
	thisogg->in = mp3in;
	thisogg->vol = voll;
	thisogg->chunksize = MP3CHUNKSIZE;
	thisogg->last_but_one = 0;
	thisogg->last_ms_offs = 0;
	thisogg->last_but_one_but_one = 0;

	if (thisogg->chunksize > (int)asset_size)
		thisogg->chunksize = asset_size;

	pack_fread(tmpbuffer, thisogg->chunksize, mp3in);

	thisogg->buffer = (char *)tmpbuffer;
	thisogg->stream = alogg_create_oggstream(tmpbuffer, thisogg->chunksize, (asset_size < 1));

	if (thisogg->stream == nullptr) {
		free(tmpbuffer);
		pack_fclose(mp3in);
		delete thisogg;
		return nullptr;
	}

	return thisogg;
}



MYMIDI *thismidi;
SOUNDCLIP *my_load_midi(const AssetPath &asset_name, int repet) {
	// The first a midi is played, preload all patches.
	if (!thismidi && psp_midi_preload_patches)
		load_midi_patches();

	size_t asset_size;
	PACKFILE *pf = PackfileFromAsset(asset_name, asset_size);
	if (!pf)
		return nullptr;

	MIDI *midiPtr = load_midi_pf(pf);
	pack_fclose(pf);

	if (midiPtr == nullptr)
		return nullptr;

	thismidi = new MYMIDI();
	thismidi->tune = midiPtr;
	thismidi->repeat = (repet != 0);

	return thismidi;
}


#ifdef JGMOD_MOD_PLAYER

MYMOD *thismod = NULL;
SOUNDCLIP *my_load_mod(const char *filname, int repet) {

	JGMOD *modPtr = load_mod((char *)filname);
	if (modPtr == NULL)
		return NULL;

	thismod = new MYMOD();
	thismod->tune = modPtr;
	thismod->repeat = (repet != 0);

	return thismod;
}

int init_mod_player(int numVoices) {
	return install_mod(numVoices);
}

void remove_mod_player() {
	remove_mod();
}

//#endif   // JGMOD_MOD_PLAYER
#elif defined DUMB_MOD_PLAYER

MYMOD *thismod = nullptr;
SOUNDCLIP *my_load_mod(const AssetPath &asset_name, int repet) {
	size_t asset_size;
	DUMBFILE *df = DUMBfileFromAsset(asset_name, asset_size);
	if (!df)
		return nullptr;

	DUH *modPtr = nullptr;
	// determine the file extension
	const char *lastDot = strrchr(asset_name.second, '.');
	if (lastDot == nullptr) {
		dumbfile_close(df);
		return nullptr;
	}
	// get the first char of the extensin
	int charAfterDot = toupper(lastDot[1]);

	// use the appropriate loader
	if (charAfterDot == 'I') {
		modPtr = dumb_read_it(df);
	} else if (charAfterDot == 'X') {
		modPtr = dumb_read_xm(df);
	} else if (charAfterDot == 'S') {
		modPtr = dumb_read_s3m(df);
	} else if (charAfterDot == 'M') {
		modPtr = dumb_read_mod(df);
	}

	dumbfile_close(df);
	if (modPtr == nullptr)
		return nullptr;

	thismod = new MYMOD();
	thismod->tune = modPtr;
	thismod->vol = 255;
	thismod->repeat = (repet != 0);

	return thismod;
}

int init_mod_player(int numVoices) {
	dumb_register_packfiles();
	return 0;
}

void remove_mod_player() {
	dumb_exit();
}

} // namespace AGS3

#endif
