/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef PLAYER_V2A_H
#define PLAYER_V2A_H

#include "common/scummsys.h"
#include "common/system.h"
#include "scumm/music.h"
#include "scumm/player_mod.h"

class SoundMixer;

namespace Scumm {

class ScummEngine;
class V2A_Sound;

/**
 * Scumm V2 Amiga sound/music driver.
 */
class Player_V2A : public MusicEngine {
public:
	Player_V2A(ScummEngine *scumm);
	virtual ~Player_V2A();

	virtual void setMasterVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer() const;
	virtual int  getSoundStatus(int sound) const;

private:
	enum {
		V2A_MAXSLOTS = 8
	};

	struct soundSlot {
		int id;
		V2A_Sound *sound;
	};

	OSystem *_system;
	ScummEngine *_vm;
	Player_MOD *_mod;
	soundSlot _slot[V2A_MAXSLOTS];

	int getSoundSlot (int id = 0) const;
	static void update_proc(void *param);
	void updateSound();
};

} // End of namespace Scumm

#endif
