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

#ifndef SCUMM_PLAYERS_PLAYER_V2A_H
#define SCUMM_PLAYERS_PLAYER_V2A_H

#include "common/scummsys.h"
#include "scumm/music.h"
#include "scumm/players/player_mod.h"

class Mixer;

namespace Scumm {

class ScummEngine;
class V2A_Sound;

/**
 * Scumm V2 Amiga sound/music driver.
 */
class Player_V2A : public MusicEngine {
public:
	Player_V2A(ScummEngine *scumm, Audio::Mixer *mixer);
	~Player_V2A() override;

	void setMusicVolume(int vol) override;
	void startSound(int sound) override;
	void stopSound(int sound) override;
	void stopAllSounds() override;
	int  getMusicTimer() override;
	int  getSoundStatus(int sound) const override;

private:
	enum {
		V2A_MAXSLOTS = 8
	};

	struct soundSlot {
		int id;
		V2A_Sound *sound;
	};

	ScummEngine *_vm;
	Player_MOD *_mod;
	soundSlot _slot[V2A_MAXSLOTS];

	int getSoundSlot(int id = 0) const;
	static void update_proc(void *param);
	void updateSound();
};

} // End of namespace Scumm

#endif
