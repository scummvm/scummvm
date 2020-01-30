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

#include "ultima/nuvie/sound/adplug/adplug_player.h"
#include "ultima/nuvie/sound/adplug/silent_opl.h"

namespace Ultima {
namespace Nuvie {

/***** CPlayer *****/

const unsigned short CPlayer::note_table[12] =
{363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647, 686};

const unsigned char CPlayer::op_table[9] =
{0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

CPlayer::CPlayer(Copl *newopl)
	: opl(newopl) {
}

CPlayer::~CPlayer() {
}

unsigned long CPlayer::songlength(int subsong) {
	CSilentopl    tempopl;
	Copl      *saveopl = opl;
	float     slength = 0.0f;

	// save original OPL from being overwritten
	opl = &tempopl;

	// get song length
	rewind(subsong);
	while (update() && slength < 600000)  // song length limit: 10 minutes
		slength += 1000.0f / getrefresh();
	rewind(subsong);

	// restore original OPL and return
	opl = saveopl;
	return (unsigned long)slength;
}

void CPlayer::seek(unsigned long ms) {
	float pos = 0.0f;

	rewind();
	while (pos < ms && update())      // seek to new position
		pos += 1000 / getrefresh();
}

} // End of namespace Nuvie
} // End of namespace Ultima
