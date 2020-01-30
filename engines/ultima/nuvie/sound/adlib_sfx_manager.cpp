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
#include "ultima/nuvie/sound/decoder/adlib_sfx_stream.h"
#include "ultima/nuvie/sound/adlib_sfx_manager.h"
#include "audio/mixer.h"

namespace Ultima {
namespace Nuvie {

AdLibSfxManager::AdLibSfxManager(Configuration *cfg, Audio::Mixer *m) : SfxManager(cfg, m) {

}

AdLibSfxManager::~AdLibSfxManager() {

}

bool AdLibSfxManager::playSfx(SfxIdType sfx_id, uint8 volume) {
	return playSfxLooping(sfx_id, NULL, volume);
}


bool AdLibSfxManager::playSfxLooping(SfxIdType sfx_id, Audio::SoundHandle *handle, uint8 volume) {
	AdLibSfxStream *stream = NULL;

	if (sfx_id == NUVIE_SFX_SE_TICK) {
		stream = new AdLibSfxStream(config, mixer->getOutputRate(), 17, 0x30, 0x60, 0xff, 22050);
	} else if (sfx_id == NUVIE_SFX_EXPLOSION) {
		stream = new AdLibSfxStream(config, mixer->getOutputRate(), 8, 0x40, 0x40, 0x7f, 22050);
	}

	if (stream) {
		sfx_duration = stream->getLengthInMsec();
		playSoundSample(stream, handle, volume);
		return true;
	}

	return false;
}

void AdLibSfxManager::playSoundSample(Audio::AudioStream *stream, Audio::SoundHandle *looping_handle, uint8 volume) {
	Audio::SoundHandle handle;

	if (looping_handle) {
		Audio::RewindableAudioStream *audioStream = dynamic_cast<Audio::RewindableAudioStream *>(stream);
		assert(audioStream);
		Audio::LoopingAudioStream *looping_stream = new Audio::LoopingAudioStream(audioStream, 0);
		mixer->playStream(Audio::Mixer::kPlainSoundType, looping_handle, looping_stream, -1, volume);
	} else {
		mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, stream, -1, volume);
	}

}

} // End of namespace Nuvie
} // End of namespace Ultima
