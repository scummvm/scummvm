/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * /scummvm/scummvm/scumm/player_v3a.h
 *
 */

#ifndef SCUMM_MUSIC_H
#define SCUMM_MUSIC_H

#include "common/scummsys.h"
#include "common/system.h"

class MusicEngine {
public:
	virtual ~MusicEngine() {}

	virtual void setMasterVolume(int vol) = 0;

	virtual void startSound(int sound) = 0;
	virtual void stopSound(int sound) = 0;
	virtual void stopAllSounds() = 0;
	virtual int  getSoundStatus(int sound) const = 0;

	virtual int  getMusicTimer() const { return 0; }

	virtual void terminate() {}
};

#endif
