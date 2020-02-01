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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "audio/mixer.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/sound/custom_sfx_manager.h"
#include "common/file.h"

namespace Ultima {
namespace Nuvie {

CustomSfxManager::CustomSfxManager(Configuration *cfg, Audio::Mixer *m) : SfxManager(cfg, m) {
	Std::string cfg_filename;

	sfx_map = new Std::map<uint16, uint16>();

	config->pathFromValue("config/ultima6/sfxdir", "", custom_filepath);

	build_path(custom_filepath, "sfx_map.cfg", cfg_filename);

	loadSfxMapFile(cfg_filename, sfx_map);
}

CustomSfxManager::~CustomSfxManager() {

}


bool CustomSfxManager::loadSfxMapFile(Std::string cfg_filename, Std::map<uint16, uint16> *m) {
	char seps[] = ";\r\n";
	char *token1;
	char *token2;
	NuvieIOFileRead niof;
	char *sz;

	if (niof.open(cfg_filename) == false) {
		DEBUG(0, LEVEL_ERROR, "Failed to open '%s'", cfg_filename.c_str());
		return false;
	}

	sz = (char *) niof.readAll();

	token1 = strtok(sz, seps);

	while ((token1 != NULL) && ((token2 = strtok(NULL, seps)) != NULL)) {
		SfxIdType sfx_id = (SfxIdType)atoi(token1);
		int custom_wave_id = atoi(token2);

		DEBUG(0, LEVEL_DEBUGGING, "%d : %d.wav\n", sfx_id, custom_wave_id);
		(*m)[sfx_id] = custom_wave_id;

		token1 = strtok(NULL, seps);
	}


	return true;
}

bool CustomSfxManager::playSfx(SfxIdType sfx_id, uint8 volume) {
	return playSfxLooping(sfx_id, NULL, volume);
}


bool CustomSfxManager::playSfxLooping(SfxIdType sfx_id, Audio::SoundHandle *handle, uint8 volume) {
	Std::map < uint16, uint16 >::iterator it;

	it = sfx_map->find((uint16)sfx_id);
	if (it != sfx_map->end()) {
		playSoundSample((*it)._value, handle, volume);
		return true;
	}

	return false;
}

void CustomSfxManager::playSoundSample(uint16 sample_num, Audio::SoundHandle *looping_handle, uint8 volume) {
	Audio::AudioStream *stream = NULL;
	Audio::SoundHandle handle;
	Std::string filename;
	char wavefile[10]; // "nnnnn.wav\0"

	sprintf(wavefile, "%d.wav", sample_num);

	build_path(custom_filepath, wavefile, filename);

	Common::File *readStream = new Common::File();
	if (!readStream->open(filename)) {
		DEBUG(0, LEVEL_ERROR, "Failed to open '%s'", filename.c_str());
		delete readStream;
		return;
	}

	stream = Audio::makeWAVStream(readStream, DisposeAfterUse::YES);

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
