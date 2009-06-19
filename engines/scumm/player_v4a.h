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

#ifndef SCUMM_PLAYER_V4A_H
#define SCUMM_PLAYER_V4A_H

#include "common/scummsys.h"
#include "scumm/music.h"
#include "sound/mixer.h"
#include "sound/mods/tfmx.h"

class Mixer;

namespace Scumm {

class ScummEngine;

/**
 * Scumm V4 Amiga sound/music driver.
 */
class Player_V4A : public MusicEngine {
public:
	Player_V4A(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_V4A();

	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer() const;
	virtual int  getSoundStatus(int sound) const;

private:
	ScummEngine *_vm;

	Audio::Tfmx *_tfmxPlay;
	Audio::Tfmx *_tfmxSfx;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _sfxHandle;

	int _musicId;

	bool init();
};

} // End of namespace Scumm

#endif
