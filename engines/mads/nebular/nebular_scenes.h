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
 */

#ifndef MADS_NEBULAR_SCENES_H
#define MADS_NEBULAR_SCENES_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"

namespace MADS {

namespace Nebular {

enum {
	SEX_MALE = 0, SEX_FEMALE = 2
};

enum Noun {
	NOUN_BLOWGUN		= 0x29,
	NOUN_BURGER			= 0x35,
	NOUN_CHAIR			= 0x47,
	NOUN_DEAD_FISH		= 0x65,
	NOUN_DOOR			= 0x6E,
	NOUN_EAT			= 0x75,
	NOUN_EXAMINE		= 0x7D,
	NOUN_FRONT_WINDOW	= 0x8E,
	NOUN_FUZZY_DICE		= 0x91,
	NOUN_HOSE_DOWN		= 0x0A6,
	NOUN_HOTPANTS		= 0x0A7,
	NOUN_HULL			= 0x0A8,
	NOUN_HURL			= 0x0A9,
	NOUN_IGNITE			= 0x0B4,
	NOUN_INFLATE		= 0x0B5,
	NOUN_INSERT			= 0x0B6,
	NOUN_INSPECT		= 0x0B7,
	NOUN_JUNGLE			= 0x0B8,
	NOUN_LIFE_SUPPORT_SECTION  = 0x0CC,
	NOUN_LOG			= 0x0D0,
	NOUN_LOOK_AT		= 0x0D1,
	NOUN_LOOK_IN		= 0x0D2,
	NOUN_LOOK_THROUGH	= 0x0D3,
	NOUN_MONKEY			= 0x0E3,
	NOUN_OUTER_HULL		= 0x0F8,
	NOUN_OUTSIDE		= 0x0F9,
	NOUN_PEER_THROUGH	= 0x103,
	NOUN_PLANT_STALK	= 0x10F,
	NOUN_READ			= 0x11F,
	NOUN_REFRIDGERATOR	= 0x122,
	NOUN_ROBO_KITCHEN	= 0x127,
	NOUN_SHIELD_ACCESS_PANEL  = 0x135,
	NOUN_SHIELD_MODULATOR	= 0x137,
	NOUN_SHOOT			= 0x13A,
	NOUN_SIT_IN			= 0x13F,
	NOUN_SMELL			= 0x147,
	NOUN_STUFFED_FISH	= 0x157,
	NOUN_VIEW_SCREEN	= 0x180,
	NOUN_CAPTIVE_CREATURE	= 0x1C3,
	NOUN_NATIVE_WOMAN	= 0x1DC,
	NOUN_ALCOHOL		= 0x310,
	NOUN_DOLLOP			= 0x3AC,
	NOUN_DROP			= 0x3AD,
	NOUN_DASH			= 0x3AE,
	NOUN_SPLASH			= 0x3AF
};

class SceneFactory {
public:
	static SceneLogic *createScene(MADSEngine *vm);
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES_H */
