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

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "audio/mixer.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/sound/towns_sfx_manager.h"

namespace Ultima {
namespace Nuvie {

typedef struct { // sfx lookup
	uint16 sfx_id;
	uint8 towns_sample_num;
} TownsSfxLookup;

#define TOWNS_SFX_TBL_SIZE 12
//15 hail effect
//16 explosion
//17 level not high enough, no effect etc.
//18 cast magic sound
//19 resurrection tune
static const TownsSfxLookup sfx_lookup_tbl[] = {
	{NUVIE_SFX_BLOCKED, 0},
	{NUVIE_SFX_HIT, 4},
	{NUVIE_SFX_BROKEN_GLASS, 12},
	{NUVIE_SFX_BELL, 13},
	{NUVIE_SFX_FOUNTAIN, 46},
	{NUVIE_SFX_PROTECTION_FIELD, 47},
	//FIXME {NUVIE_SFX_CLOCK, 1},
	{NUVIE_SFX_FIRE, 6},
	{NUVIE_SFX_RUBBER_DUCK, 3},
	{NUVIE_SFX_WATER_WHEEL, 48},
	{NUVIE_SFX_MISSLE, 9},
	{NUVIE_SFX_EXPLOSION, 16},
	{NUVIE_SFX_ATTACK_SWING, 2}
};

TownsSfxManager::TownsSfxManager(Configuration *cfg, Audio::Mixer *m) : SfxManager(cfg, m),
		fireStream(nullptr) {
	config->pathFromValue("config/townsdir", "sounds2.dat", sounds2dat_filepath);
	loadSound1Dat();
}

TownsSfxManager::~TownsSfxManager() {
	//FIXME how do we make sure no sfxs are playing before deleting our resources?
	//delete fireStream;
	//free sounds1_dat[] buffers
}

void TownsSfxManager::loadSound1Dat() {
	Std::string filename;
	U6Lzw decompressor;
	U6Lib_n lib;
	NuvieIOBuffer iobuf;
	uint32 slib32_len = 0;

	config->pathFromValue("config/townsdir", "sounds1.dat", filename);
	unsigned char *slib32_data = decompressor.decompress_file(filename, slib32_len);

	if (slib32_len == 0)
		return;

	iobuf.open(slib32_data, slib32_len);
	free(slib32_data);

	if (!lib.open(&iobuf, 4))
		return;

	uint8 i;
	for (i = 0; i < TOWNS_SFX_SOUNDS1_SIZE; i++) {
		sounds1_dat[i].buf = lib.get_item(i);
		sounds1_dat[i].len = lib.get_item_size(i);
	}

	// Fire SFX is made up of three individual samples played in a random sequence
	Std::vector<Audio::RewindableAudioStream *> streams;
	streams.push_back(new FMtownsDecoderStream(sounds1_dat[6].buf, sounds1_dat[6].len));
	streams.push_back(new FMtownsDecoderStream(sounds1_dat[7].buf, sounds1_dat[7].len));
	streams.push_back(new FMtownsDecoderStream(sounds1_dat[8].buf, sounds1_dat[8].len));

	fireStream = U6Audio::makeRandomCollectionAudioStream(mixer->getOutputRate(), false, streams, DisposeAfterUse::NO);
}

bool TownsSfxManager::playSfx(SfxIdType sfx_id, uint8 volume) {
	return playSfxLooping(sfx_id, NULL, volume);
}


bool TownsSfxManager::playSfxLooping(SfxIdType sfx_id, Audio::SoundHandle *handle, uint8 volume) {
	uint16 i = 0;
	for (i = 0; i < TOWNS_SFX_TBL_SIZE; i++) {
		if (sfx_lookup_tbl[i].sfx_id == sfx_id) {
			playSoundSample(sfx_lookup_tbl[i].towns_sample_num, handle, volume);
			return true;
		}
	}
	return false;
}

void TownsSfxManager::playSoundSample(uint8 sample_num, Audio::SoundHandle *looping_handle, uint8 volume) {
	Audio::AudioStream *stream = NULL;
	Audio::SoundHandle handle;

	if (sample_num > 5 && sample_num < 9) {
		// Fire samples
		mixer->playStream(Audio::Mixer::kPlainSoundType, looping_handle ? looping_handle : &handle, fireStream, -1, volume, 0, DisposeAfterUse::NO);
		return;
	}

	if (sample_num < TOWNS_SFX_SOUNDS1_SIZE) {
		stream = new FMtownsDecoderStream(sounds1_dat[sample_num].buf, sounds1_dat[sample_num].len);
	} else {
		stream = new FMtownsDecoderStream(sounds2dat_filepath, sample_num - TOWNS_SFX_SOUNDS1_SIZE, false); //not compressed
	}

	if (looping_handle) {
		Audio::RewindableAudioStream *rwStream = dynamic_cast<Audio::RewindableAudioStream *>(stream);
		Audio::LoopingAudioStream *looping_stream = new Audio::LoopingAudioStream(rwStream, 0);
		mixer->playStream(Audio::Mixer::kPlainSoundType, looping_handle, looping_stream, -1, volume);
	} else {
		mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, stream, -1, volume);
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
