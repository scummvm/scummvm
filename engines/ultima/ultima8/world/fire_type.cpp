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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/misc/point3.h"
#include "ultima/ultima8/audio/audio_process.h"

namespace Ultima {
namespace Ultima8 {

FireType::FireType(uint16 typeNo, uint16 minDamage, uint16 maxDamage, uint8 range,
				 uint8 numShots, uint16 shieldCost, uint8 shieldMask, uint8 accurate,
				 uint16 cellsPerRound, uint16 roundDuration, uint8 nearSprite) :
	_typeNo(typeNo), _minDamage(minDamage), _maxDamage(maxDamage),
	_range(range), _numShots(numShots), _shieldCost(shieldCost),
	_shieldMask(shieldMask), _accurate(accurate),
	_cellsPerRound(cellsPerRound), _roundDuration(roundDuration),
	_nearSprite(nearSprite) {
}

uint16 FireType::getRandomDamage() const {
	return _minDamage + (getRandom() % (_maxDamage - _minDamage));
}


// TODO: This is all hard coded values for No Remorse. These should probably
// be moved into ini file settings so that it can be swapped out for No Regret.
static const int16 FIRESOUND_1[] = { 0x26, 0x27, 0x41, 0x42, 0x45, 0x46 };
static const int16 FIRESOUND_3[] = { 0x1c, 0x6c, 0x3e };
static const int16 FIRESOUND_7[] = { 0x48, 0x5 };

static const int16 FIRESHAPE_3[] = { 0x326, 0x320, 0x321 };
static const int16 FIRESHAPE_10[] = { 0x31c, 0x31f, 0x322 };

void FireType::makeBulletSplashShapeAndPlaySound(int32 x, int32 y, int32 z) const {
	int16 sfxno = 0;
	int16 shape = 0;

	// First randomize the sprite and sound
	switch (_typeNo) {
		case 1:
		case 0xb:
			shape = 0x1d8;
			sfxno = FIRESOUND_1[getRandom() % 6];
			break;
		case 2:
			shape = 0x1d8;
			break;
		case 3:
		case 4:
			shape = FIRESHAPE_3[getRandom() % 3];
			sfxno = FIRESOUND_3[getRandom() % 3];
			break;
		case 5:
			shape = 0x573;
			break;
		case 6:
			shape = 0x578;
			break;
		case 7:
			shape = 0x537;
			sfxno = FIRESOUND_7[getRandom() % 2];
			break;
		case 10:
			shape = FIRESHAPE_10[getRandom() % 3];
			sfxno = FIRESOUND_3[getRandom() % 3];
			break;
		case 0xd:
			shape = 0x1d8;
			sfxno = FIRESOUND_1[getRandom() % 6];
			break;
		case 0xe:
			shape = 0x56b;
			break;
		case 0xf:
			shape = 0x59b;
			sfxno = FIRESOUND_7[getRandom() % 2];
			break;
		case 9:
		default:
			shape = 0x537;
			break;
	}

	int16 firstframe = 0;
	int16 lastframe = 0x27;

	// now randomize frames
	switch (shape) {
	case 0x56b:
		firstframe = (getRandom() % 3) * 6;
		lastframe = firstframe + 5;
		break;
	case 0x537:
		lastframe = 10;
		break;
	case 0x578:
		firstframe = (getRandom() % 3) * 6;
		lastframe = firstframe + 4;
		break;
	case 0x59b:
		firstframe = (getRandom() % 2) * 4;
		lastframe = firstframe + 3;
		break;
	case 0x1d8: {
		switch (getRandom() % 4) {
			case 0:
				lastframe = 4;
				break;
			case 1:
				firstframe = 5;
				lastframe = 8;
				break;
			case 2:
				firstframe = 9;
				lastframe = 0xc;
				break;
			case 3:
				firstframe = 0xd;
				lastframe = 0x10;
				break;
		}
		break;
	}
	}

	SpriteProcess *sprite = new SpriteProcess(shape, firstframe, lastframe, 1, 3, x, y, z);
	Kernel::get_instance()->addProcess(sprite);

	AudioProcess *audio = AudioProcess::get_instance();
	if (sfxno && audio) {
		audio->playSFX(sfxno, 0x10, 0, 1, false);
	}
}

void FireType::applySplashDamageAround(const Point3 &pt, int damage, const Item *exclude, const Item *src) const {
	if (!getRange())
		return;
	static const uint32 BULLET_SPLASH_SHAPE = 0x1d9;

	CurrentMap *currentmap = World::get_instance()->getCurrentMap();

	//
	// Find items in range and apply splash damage
	//
	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	currentmap->areaSearch(&uclist, script, sizeof(script), nullptr,
						   getRange() * 16, true);
	for (unsigned int i = 0; i < uclist.getSize(); ++i) {
		Item *splashitem = getItem(uclist.getuint16(i));
		assert(splashitem);
		//
		// Other items don't get splash damage from their own fire.. but the
		// player does.  Life is not fair..
		//
		if (splashitem == exclude || (splashitem == src && src != getControlledActor()) ||
			splashitem->getShape() == BULLET_SPLASH_SHAPE)
			continue;
		int splashitemdamage = damage;
		if (_typeNo == 3 || _typeNo == 4 || _typeNo == 10) {
			Point3 pt2;
			splashitem->getLocation(pt2);
			int splashrange = pt.maxDistXYZ(pt2);
			splashrange = (splashrange / 16) / 3;
			if (splashrange)
				splashitemdamage /= splashrange;
		}
		Direction splashdir = src->getDirToItemCentre(pt);
		splashitem->receiveHit(0, splashdir, splashitemdamage, _typeNo);
	}
}

}
}
