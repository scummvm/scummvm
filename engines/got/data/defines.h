/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GOT_DATA_DEFINES_H
#define GOT_DATA_DEFINES_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "got/data/actor.h"

namespace Got {

struct HEADER {
	long offset = 0;
	long length = 0;

	void load(Common::SeekableReadStream *src) {
		offset = src->readUint32LE();
		length = src->readUint32LE();
	}
};

//==========================================================================
// Defines

#define MAX_ACTORS 35
#define MAX_ENEMIES 16
#define STAMINA 20

#define _Z 44
#define _B 48
#define TMP_SIZE 5800

#define GAME1 (_G(area) == 1)
#define GAME2 (_G(area) == 2)
#define GAME3 (_G(area) == 3)

#define APPLE_MAGIC 1
#define LIGHTNING_MAGIC 2
#define BOOTS_MAGIC 4
#define WIND_MAGIC 8
#define SHIELD_MAGIC 16
#define THUNDER_MAGIC 32

#define BOSS_LEVEL1 59
#define BOSS_LEVEL2 60
#define BOSS_LEVEL3 95
#define ENDING_SCREEN 106

#define DEMO_LEN 3600

} // namespace Got

#endif
