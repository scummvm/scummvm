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

#ifndef NUVIE_SOUND_PC_SPEAKER_SFX_MANAGER_H
#define NUVIE_SOUND_PC_SPEAKER_SFX_MANAGER_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "audio/mixer.h"
#include "ultima/nuvie/sound/sfx_manager.h"
#include "audio/audiostream.h"

namespace Ultima {
namespace Nuvie {

class PCSpeakerSfxManager : public SfxManager {
public:
	PCSpeakerSfxManager(Configuration *cfg, Audio::Mixer *m);
	~PCSpeakerSfxManager() override;

	bool playSfx(SfxIdType sfx_id, uint8 volume) override;
	bool playSfxLooping(SfxIdType sfx_id, Audio::SoundHandle *handle, uint8 volume) override;

private:
	void playSoundSample(Audio::AudioStream *stream, Audio::SoundHandle *looping_handle, uint8 volume);

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
