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


#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

FireType::FireType(uint16 typeNo, uint16 minDamage, uint16 maxDamage, uint8 range,
				 uint8 numShots, uint16 shieldCost, uint8 shieldMask, bool accurate,
				 uint16 cellsPerRound, uint16 roundDuration, bool nearSprite) :
	_typeNo(typeNo), _minDamage(minDamage), _maxDamage(maxDamage),
	_range(range), _numShots(numShots), _shieldCost(shieldCost),
	_shieldMask(shieldMask), _accurate(accurate),
	_cellsPerRound(cellsPerRound), _roundDuration(roundDuration),
	_nearSprite(nearSprite) {
	assert(maxDamage >= minDamage);
}

uint16 FireType::getRandomDamage() const {
	if (_minDamage == _maxDamage)
		return _minDamage;
	return _minDamage + (getRandom() % (_maxDamage - _minDamage));
}


// The first 3 arrays are valid for No Remorse and No Regret.
static const int16 FIRESOUND_1[] = { 0x26, 0x27, 0x41, 0x42, 0x45, 0x46 };
static const int16 FIRESOUND_3[] = { 0x1c, 0x6c, 0x3e };
static const int16 FIRESOUND_7[] = { 0x48, 0x5 };
// These ones are No Regret only.
static const int16 FIRESOUND_0x10_REG[] = { 0x8, 0x202 };
static const int16 FIRESOUND_0xE_REG[] = { 0x205, 0x204 };
static const int16 FIRESOUND_0x14_REG[] = { 0x207, 0x208 };

// Shape arrays are very similar but slightly different between games
static const int16 FIRESHAPE_3_REM[] = { 0x326, 0x320, 0x321 };
static const int16 FIRESHAPE_3_REG[] = { 0x326, 0x320, 0x321, 0x323 };
static const int16 FIRESHAPE_10_REM[] = { 0x31c, 0x31f, 0x322 };
static const int16 FIRESHAPE_10_REG[] = { 0x31c, 0x31f, 0x321 };

#define RANDOM_ELEM(array) (array[getRandom() % ARRAYSIZE(array)])

void FireType::makeBulletSplashShapeAndPlaySound(int32 x, int32 y, int32 z) const {
	int16 sfxno = 0;
	int16 shape = 0;

	// First randomize the sprite and sound
	switch (_typeNo) {
		case 1:
		case 0xb:
			shape = 0x1d8;
			sfxno = RANDOM_ELEM(FIRESOUND_1);
			break;
		case 2:
			shape = 0x1d8;
			if (GAME_IS_REGRET && (getRandom() % 3 == 0)) {
				sfxno = RANDOM_ELEM(FIRESOUND_1);
			}
			break;
		case 3:
		case 4:
			if (GAME_IS_REMORSE)
				shape = RANDOM_ELEM(FIRESHAPE_3_REM);
			else
				shape = RANDOM_ELEM(FIRESHAPE_3_REG);
			sfxno = RANDOM_ELEM(FIRESOUND_3);
			break;
		case 5:
			shape = 0x537;
			if (GAME_IS_REGRET) {
				if (getRandom() % 2)
					sfxno = 0x164;
				else
					sfxno = 0x71;
			}
			break;
		case 6:
			shape = 0x578;
			if (GAME_IS_REGRET)
				sfxno = 0x206;
			break;
		case 7:
			shape = 0x537;
			sfxno = RANDOM_ELEM(FIRESOUND_7);
			break;
		case 10:
			if (GAME_IS_REMORSE)
				shape = RANDOM_ELEM(FIRESHAPE_10_REM);
			else
				shape = RANDOM_ELEM(FIRESHAPE_10_REG);
			sfxno = RANDOM_ELEM(FIRESOUND_3);
			break;
		case 0xd:
			shape = 0x1d8;
			if (GAME_IS_REMORSE || (getRandom() % 4 == 0))
				sfxno = RANDOM_ELEM(FIRESOUND_1);
			break;
		case 0xe:
			shape = 0x56b;
			if (GAME_IS_REGRET)
				sfxno = RANDOM_ELEM(FIRESOUND_0xE_REG);
			break;
		case 0xf:
			shape = 0x59b;
			sfxno = RANDOM_ELEM(FIRESOUND_7);
			if (GAME_IS_REGRET)
				sfxno = RANDOM_ELEM(FIRESOUND_7);
			break;
		case 0x10: // No Regret only
			shape = 0x643;
			sfxno = RANDOM_ELEM(FIRESOUND_0x10_REG);
			break;
		case 0x11: // No Regret only
			shape = 0x642;
			sfxno = 0x203;
			break;
		case 0x12: // No Regret only
			shape = 0x59b;
			sfxno = RANDOM_ELEM(FIRESOUND_0x10_REG);
			break;
		case 0x13: // No Regret only
			shape = 0x59b;
			sfxno = RANDOM_ELEM(FIRESOUND_7);
			break;
		case 0x14: // No Regret only
			shape = 0x641;
			sfxno = RANDOM_ELEM(FIRESOUND_0x14_REG);
			break;
		case 0x15: // No Regret only
			shape = 0x641;
			sfxno = 0x20a;
			break;
		case 0x16: // No Regret only
			shape = 0x31f;
			sfxno = RANDOM_ELEM(FIRESOUND_3);
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
	case 0x642:
		firstframe = (getRandom() % 3) * 5;
		lastframe = firstframe + 4;
		break;
	case 0x59b:
		firstframe = (getRandom() % 2) * 4;
		lastframe = firstframe + 3;
		break;
	case 0x641: // No Regret only
	case 0x643: // No Regret only
		lastframe = 3;
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

void FireType::applySplashDamageAround(const Point3 &pt, int damage, int rangediv, const Item *exclude, const Item *src) const {
	assert(rangediv > 0);
	if (!getRange())
		return;
	static const uint32 BULLET_SPLASH_SHAPE = 0x1d9;

	CurrentMap *currentmap = World::get_instance()->getCurrentMap();

	//
	// Find items in range and apply splash damage.  Coordinates here are 2x the
	// original game code (in line with our other x2 multipliers for game coords)
	//
	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	currentmap->areaSearch(&uclist, script, sizeof(script), nullptr,
						   getRange() * 32 / rangediv, false, pt.x, pt.y);
	for (unsigned int i = 0; i < uclist.getSize(); ++i) {
		Item *splashitem = getItem(uclist.getuint16(i));
		if (!splashitem) {
			// already gone - probably got destroyed by some chain-reaction?
			continue;
		}

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
			splashrange = (splashrange / 32) / 3;
			if (splashrange)
				splashitemdamage /= splashrange;
		}
		if (!splashitemdamage)
			continue;

		Direction splashdir = src->getDirToItemCentre(pt);
		splashitem->receiveHit(0, splashdir, splashitemdamage, _typeNo);
	}
}

}
}
