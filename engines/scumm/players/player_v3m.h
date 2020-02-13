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

#ifndef SCUMM_PLAYERS_PLAYER_V3M_H
#define SCUMM_PLAYERS_PLAYER_V3M_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/mutex.h"
#include "scumm/music.h"
#include "scumm/players/player_mac.h"

namespace Audio {
class Mixer;
}

namespace Scumm {

class ScummEngine;

/**
 * Scumm V3 Macintosh music driver.
 */
class Player_V3M : public Player_Mac {
public:
	Player_V3M(ScummEngine *scumm, Audio::Mixer *mixer);

	bool checkMusicAvailable() override;
	bool loadMusic(const byte *ptr) override;
	bool getNextNote(int ch, uint32 &samples, int &pitchModifier, byte &velocity) override;
};

} // End of namespace Scumm

#endif
