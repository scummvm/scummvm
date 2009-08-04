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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SOUND_H_
#define ASYLUM_SOUND_H_

#include "sound/mixer.h"
#include "asylum/respack.h"

namespace Asylum {

class Sound {
public:
	Sound(Audio::Mixer *mixer);
	~Sound();

	void playSfx(byte *data, uint32 size);
	void playSfx(ResourcePack *resPack, uint32 resourceId) {
		ResourceEntry *resEntry = resPack->getResource(resourceId);
		playSfx(resEntry->data, resEntry->size);
	}
	bool isSfxActive() { return _mixer->isSoundHandleActive(_sfxHandle); }
	void pauseSfx() { _mixer->pauseHandle(_sfxHandle, true); }
	void resumeSfx() { _mixer->pauseHandle(_sfxHandle, false); }
	void stopSfx() { _mixer->stopHandle(_sfxHandle); }

	void playMusic(byte *data, uint32 size);
	void playMusic(ResourcePack *resPack, uint32 resourceId) {
		ResourceEntry *resEntry = resPack->getResource(resourceId);
		playMusic(resEntry->data, resEntry->size);
	}
	bool isMusicActive() { return _mixer->isSoundHandleActive(_musicHandle); }
	void pauseMusic() { _mixer->pauseHandle(_musicHandle, true); }
	void resumeMusic() { _mixer->pauseHandle(_musicHandle, false); }
	void stopMusic() { _mixer->stopHandle(_musicHandle); }

private:
	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _musicHandle;
	Audio::Mixer       *_mixer;
};

} // end of namespace Asylum

#endif
