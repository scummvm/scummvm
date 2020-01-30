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

#ifndef NUVIE_SOUND_SFX_MANAGER_H
#define NUVIE_SOUND_SFX_MANAGER_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "audio/mixer.h"
#include "ultima/nuvie/sound/sfx.h"

namespace Ultima {
namespace Nuvie {

class SfxManager {
public:
	SfxManager(Configuration *cfg, Audio::Mixer *m) : config(cfg), mixer(m) {
		sfx_duration = 0;
	};
	virtual ~SfxManager() {};


	virtual bool playSfx(SfxIdType sfx_id, uint8 volume) = 0;
	virtual bool playSfxLooping(SfxIdType sfx_id, Audio::SoundHandle *handle, uint8 volume) = 0;

	uint32 getLastSfxDuration() {
		return sfx_duration;
	}

protected:
	Configuration *config;
	Audio::Mixer *mixer;
	uint32 sfx_duration; //duration of the last sfx played in milliseconds.
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
