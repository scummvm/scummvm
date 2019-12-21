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

#ifndef ULTIMA6_SOUND_ADLIB_SFX_MANAGER_H
#define ULTIMA6_SOUND_ADLIB_SFX_MANAGER_H

#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/conf/configuration.h"
#include "audio/mixer.h"
#include "ultima/ultima6/sound/sfx_manager.h"
#include "audio/audiostream.h"

namespace Ultima {
namespace Ultima6 {

class AdLibSfxManager : public SfxManager {
private:


public:
	AdLibSfxManager(Configuration *cfg, Audio::Mixer *m);
	~AdLibSfxManager();



	bool playSfx(SfxIdType sfx_id, uint8 volume);
	bool playSfxLooping(SfxIdType sfx_id, Audio::SoundHandle *handle, uint8 volume);

private:
	void playSoundSample(Audio::AudioStream *stream, Audio::SoundHandle *looping_handle, uint8 volume);

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
