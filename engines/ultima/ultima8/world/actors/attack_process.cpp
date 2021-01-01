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

#include "ultima/ultima8/world/actors/attack_process.h"

#include "common/memstream.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/weapon_info.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/world/actors/combat_dat.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AttackProcess)

static const int16 ATTACK_SFX_1[] = {0x15, 0x78, 0x80, 0x83, 0xDC, 0xDD};
static const int16 ATTACK_SFX_2[] = {0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xE7};
static const int16 ATTACK_SFX_3[] = {0xFC, 0xFD, 0xFE, 0xC8};
static const int16 ATTACK_SFX_4[] = {0xCC, 0xCD, 0xCE, 0xCF};
static const int16 ATTACK_SFX_5[] = {0xC7, 0xCA, 0xC9};
static const int16 ATTACK_SFX_6[] = {0x82, 0x84, 0x85};
static const int16 ATTACK_SFX_7[] = {0x9B, 0x9C, 0x9D, 0x9E, 0x9F};

#define RANDOM_ELEM(array) (array[getRandom() % ARRAYSIZE(array)])

// If data is referenced in the metalang with an offset of this or greater,
// read from the data array.
static const int MAGIC_DATA_OFF = 33000;


static uint16 someSleepGlobal = 0;

// TODO: Implement me. Set timer for some avatar moves.
static bool World_FinishedAvatarMoveTimeout() {
	return true;
}


static inline int32 randomOf(int32 max) {
	return (max > 0 ? getRandom() % max : 0);
}

AttackProcess::AttackProcess() : Process(), _block(0), _target(1), _tactic(0), _tacticDat(nullptr),
_tacticDatReadStream(nullptr), _tacticDatStartOffset(0), _soundNo(-1), _playedStartSound(false),
_npcInitialDir(dir_invalid), _field57(0), _field59(0), _field7f(false), _field96(false), _field97(false),
_isActivity9orB(false), _isActivityAorB(false), _timer3set(false), _timer2set(false),
_doubleDelay(false), _wpnField8(1), _wpnBasedTimeout(0), _difficultyBasedTimeout(0), _timer2(0),
_timer3(0), _timer4(0), _timer5(0), _soundTimestamp(0), _fireTimestamp(0) {
	for (int i = 0; i < ARRAYSIZE(_dataArray); i++) {
		_dataArray[i] = 0;
	}
}

AttackProcess::AttackProcess(Actor *actor) : _block(0), _target(1), _tactic(0), _tacticDat(nullptr),
_tacticDatReadStream(nullptr), _tacticDatStartOffset(0), _soundNo(-1), _playedStartSound(false),
_field57(0), _field59(0), _field7f(false), _field96(false), _field97(false), _isActivity9orB(false),
_isActivityAorB(false), _timer3set(false), _timer2set(false), _doubleDelay(false), _wpnField8(1),
_wpnBasedTimeout(0), _difficultyBasedTimeout(0), _timer2(0), _timer3(0), _timer4(0), _timer5(0),
_soundTimestamp(0), _fireTimestamp(0) {
	assert(actor);
	_itemNum = actor->getObjId();
	_npcInitialDir = actor->getDir();

	// Note: this isn't actually initialized in the original which
	// suggests it can't ever get used before setting, but to make
	// coverity etc happy clear it anyway.
	for (int i = 0; i < ARRAYSIZE(_dataArray); i++) {
		_dataArray[i] = 0;
	}

	const Item *wpn = getItem(actor->getActiveWeapon());
	if (wpn) {
		const uint32 wpnshape = wpn->getShape();
		const uint32 npcshape = actor->getShape();
		const uint8 difficulty = World::get_instance()->getGameDifficulty();
		if (wpnshape == 0x386 || wpnshape == 0x388 || wpnshape == 0x38e) {
			_wpnBasedTimeout = 0x3c;
			switch (difficulty) {
				case 1:
					_difficultyBasedTimeout = 0x78;
					break;
				case 2:
					_difficultyBasedTimeout = 0x5a;
					break;
				case 3:
				case 4:
				default:
					if (npcshape == 0x3ac)
						_difficultyBasedTimeout = 0xf;
					else
						_difficultyBasedTimeout = 0x3c;
					break;
			}
		} else {
			_wpnBasedTimeout = 0x1e;
			switch (difficulty) {
				case 1:
					_difficultyBasedTimeout = _wpnBasedTimeout;
					break;
				case 2:
					_difficultyBasedTimeout = 0x14;
					break;
				case 3:
					_difficultyBasedTimeout = 0xf;
					break;
				case 4:
				default:
					_difficultyBasedTimeout = 0;
			}
		}
	}

	_type = 0x0259; // CONSTANT !

	setTacticNo(actor->getCombatTactic());
}

AttackProcess::~AttackProcess() {
	delete _tacticDatReadStream;
}

void AttackProcess::terminate() {
	Actor *a = getActor(_itemNum);
	if (a)
		a->clearActorFlag(Actor::ACT_INCOMBAT);

	Process::terminate();
}

void AttackProcess::run() {
	Actor *a = getActor(_itemNum);
	Actor *target = getActor(_target);

	if (!a || !a->hasFlags(Item::FLG_FASTAREA) || a->isDead() || !_tacticDatReadStream) {
		terminate();
		return;
	}

	if (_tactic == 0) {
		genericAttack();
		return;
	}

	if (!target || target->isDead()) {
		warning("got into attack process with invalid target");
		terminate();
		return;
	}

	const Direction curdir = a->getDir();

	const uint8 opcode = _tacticDatReadStream->readByte();
	switch (opcode) {
		case 0x81:
			// Seems like field 0x53 is never used anywhere?
			/*_field53 = */readNextWordWithData();
			return;
		case 0x82:
			/*_field53 = 0*/;
			return;
		case 0x84:
			_target = readNextWordWithData();
			// This is called in the original, but basically redundant.
			// a->setActivity(5);
			return;
		case 0x85:
			a->doAnim(Animation::walk, dir_current);
			return;
		case 0x86:
			a->doAnim(Animation::run, dir_current);
		  return;
		case 0x87:
			a->doAnim(Animation::retreat, dir_current);
			return;
		case 0x88:
		{
			// Turn 90 degrees left
			Direction newdir = Direction_TurnByDelta(curdir, -2, dirmode_8dirs);
			a->turnTowardDir(newdir);
			return;
		}
		case 0x89:
		{
			// Turn 90 degrees right
			Direction newdir = Direction_TurnByDelta(curdir, 2, dirmode_8dirs);
			a->turnTowardDir(newdir);
			return;
		}
		case 0x8a:
		{
			bool result = a->fireDistance(target, curdir, 0, 0, 0);
			// Fire small weapon
			if (result)
				a->doAnim(Animation::attack, dir_current);
			return;
		}
		case 0x8b:
		{
			bool result = a->fireDistance(target, curdir, 0, 0, 0);
			// Fire large weapon
			if (result)
				a->doAnim(Animation::attack, dir_current);
			return;
		}
		case 0x8c:
			a->doAnim(Animation::stand, dir_current);
			return;
		case 0x8d:
		{
			// Pathfind to home
			int32 x, y, z;
			a->getHomePosition(x, y, z);
			ProcId pid = Kernel::get_instance()->addProcess(
					   new PathfinderProcess(a, x, y, z));
			waitFor(pid);
			return;
		}
		case 0x8e:
		{
			// Pathfind to target
			int32 x, y, z;
			target->getLocation(x, y, z);
			ProcId pid = Kernel::get_instance()->addProcess(
					   new PathfinderProcess(a, x, y, z));
			waitFor(pid);
			return;
		}
		case 0x8f:
		{
			// Pathfind to a point between npc and the target
			int32 tx, ty, tz;
			target->getLocation(tx, ty, tz);
			int32 ax, ay, az;
			target->getLocation(ax, ay, az);
			int32 x = (tx + ax) / 2;
			int32 y = (ty + ay) / 2;
			int32 z = (tz + az) / 2;
			ProcId pid = Kernel::get_instance()->addProcess(
					   new PathfinderProcess(a, x, y, z));
			waitFor(pid);
			return;
		}
		case 0x92:
			a->doAnim(Animation::kneelAndFire, dir_current);
			return;
		case 0x93:
		{
			// Sleep for a random value scaled by difficult level
			int ticks = readNextWordWithData();
			if (ticks == someSleepGlobal) {
				ticks = randomOf(0x32) + 0x14;
			}
			ticks /= World::get_instance()->getGameDifficulty();
			sleep(ticks);
			return;
		}
		case 0x94:
		{
			// Loiter a bit..
			uint16 data = readNextWordWithData();
			ProcId pid = Kernel::get_instance()->addProcess(new LoiterProcess(a, data));
			waitFor(pid);
			return;
		}
		case 0x95:
		{
			Direction dir = a->getDirToItemCentre(*target);
			a->turnTowardDir(dir);
			return;
		}
		case 0x96:
			// do activity specified by next word
			a->setActivity(readNextWordWithData());
			return;
		case 0x97:
			// switch to tactic no specified by next word
			setTacticNo(readNextWordWithData());
			return;
		case 0x98:
		{
			a->setDir(_npcInitialDir);
			a->moveToEtherealVoid();
			int32 hx, hy, hz;
			a->getHomePosition(hx, hy, hz);
			a->move(hx, hy, hz);
			return;
		}
		case 0x99:
			terminate();
			return;
		case 0x9a:
		{
			// get next word and jump to that offset if distance < 481
			Point3 apt, tpt;
			a->getLocation(apt);
			target->getLocation(tpt);
			int maxdiff = apt.maxDistXYZ(tpt);
			int16 data = readNextWordWithData();
			if (maxdiff < 481) {
				_tacticDatReadStream->seek(data, SEEK_SET);
			}
			return;
		}
		case 0x9b:
		{
			// get next word and jump to that offset if distance > 160
			Point3 apt, tpt;
			a->getLocation(apt);
			target->getLocation(tpt);
			int maxdiff = apt.maxDistXYZ(tpt);
			int16 data = readNextWordWithData();
			if (maxdiff > 160) {
				_tacticDatReadStream->seek(data, SEEK_SET);
			}
			return;
		}
		case 0x9c:
		{
			bool result = a->fireDistance(target, curdir, 0, 0, 0);
			uint16 data = readNextWordWithData();
			if (!result) {
				_tacticDatReadStream->seek(data, SEEK_SET);
			}
			return;
		}
		case 0x9d:
		{
			bool result = a->fireDistance(target, curdir, 0, 0, 0);
			uint16 data = readNextWordWithData();
			if (result) {
				_tacticDatReadStream->seek(data, SEEK_SET);
			}
			return;
		}
		case 0x9e:
		{
			uint16 maxval = readNextWordWithData();
			uint16 randval = randomOf(maxval);
			uint16 offset = readNextWordWithData();
			if (randval != 0) {
				_tacticDatReadStream->seek(offset);
			}
			return;
		}
		case 0x9f:
			_field57 = readNextWordWithData();
			_field59 = _tacticDatReadStream->pos();
			return;
		case 0xa6:
		{
			const uint16 targetFrame = readNextWordWithData();
			const uint16 targetQ = a->getUnkByte();

			UCList uclist(2);
			// loopscript to find shape = 0x33A (826), the numbers that NPCs wander between
			LOOPSCRIPT(script, LS_SHAPE_EQUAL(0x33a));
			CurrentMap *currentmap = World::get_instance()->getCurrentMap();
			currentmap->areaSearch(&uclist, script, sizeof(script), nullptr,
									0x200 * 16, true);
			for (unsigned int i = 0; i < uclist.getSize(); ++i) {
				Item *founditem = getItem(uclist.getuint16(i));
				uint16 itemQlo = founditem->getQuality() & 0xff;
				uint32 itemFrame = founditem->getFrame();

				if (itemFrame == targetFrame && (targetQ == 0 || itemQlo == targetQ)) {
					ProcId pid = Kernel::get_instance()->addProcess(
						 new PathfinderProcess(a, founditem->getObjId()));
					waitFor(pid);
					break;
				}
			}
			return;
		}
		case 0xa7:
			a->turnTowardDir(dir_north);
			return;
		case 0xa8:
			a->turnTowardDir(dir_south);
			return;
		case 0xa9:
			a->turnTowardDir(dir_east);
			return;
		case 0xaa:
			a->turnTowardDir(dir_west);
			return;
		case 0xab:
			a->turnTowardDir(dir_northeast);
			return;
		case 0xac:
			a->turnTowardDir(dir_southwest);
			return;
		case 0xad:
			a->turnTowardDir(dir_southeast);
			return;
		case 0xae:
			a->turnTowardDir(dir_northwest);
			return;
		case 0xaf:
		{
			uint16 next = readNextWordWithData();
			uint16 offset = readNextWordRaw();
			setAttackData(offset, next);
			return;
		}
		case 0xb0:
		{
			uint16 offset = readNextWordRaw();
			uint16 val = getAttackData(offset);
			setAttackData(opcode, val + readNextWordWithData());
			return;
		}
		case 0xb1:
		{
			uint16 offset = readNextWordRaw();
			uint16 val = getAttackData(offset);
			setAttackData(offset, val - readNextWordWithData());
			return;
		}
		case 0xb2:
		{
			uint16 offset = readNextWordRaw();
			uint16 val = getAttackData(offset);
			setAttackData(offset, val * readNextWordWithData());
			return;
		}
		case 0xb3:
		{
			uint16 offset = readNextWordRaw();
			uint16 val = getAttackData(offset);
			uint16 divisor = readNextWordWithData();
			if (!divisor)
				divisor = 1; // shouldn't happen in real data, but just to be sure..
			setAttackData(offset, val / divisor);
			return;
		}
		case 0xb4:
		{
			uint16 dir = Direction_ToUsecodeDir(curdir);
			uint16 offset = readNextWordRaw();
			setAttackData(offset, dir);
			return;
		}
		case 0xb5:
		{
			uint16 dir = readNextWordWithData();
			a->setDir(Direction_FromUsecodeDir(dir));
			return;
		}
		case 0xb6:
		{
			uint16 offset = readNextWordRaw();
			uint16 dir = Direction_ToUsecodeDir(curdir);
			setAttackData(offset, dir);
			return;
		}
		case 0xb7:
			a->doAnim(Animation::kneelingRetreat, dir_current);
			return;
		case 0xb8:
			a->doAnim(Animation::kneelingAdvance, dir_current);
			return;
		case 0xb9:
			a->doAnim(Animation::kneelingSlowRetreat, dir_current);
			return;
		case 0xc0:
			_tacticDatReadStream->seek(readNextWordWithData(), SEEK_SET);
			return;
		case 0xc1:
			_field57--;
			if (_field57 > 0) {
				_tacticDatReadStream->seek(_field59, SEEK_SET);
			}
			return;
		case 0xff:
			// flip to block 1 and restart
			if (_block == 0) {
				setBlockNo(1);
			}
			_tacticDatReadStream->seek(_tacticDatStartOffset, SEEK_SET);
			return;
	}
}

void AttackProcess::genericAttack() {
	Actor *a = getActor(_itemNum);
	assert(a);

	if (Kernel::get_instance()->getNumProcesses(_itemNum, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE)
		|| a->hasActorFlags(Actor::ACT_PATHFINDING)) {
		return;
	}

	const Item *wpn = getItem(a->getActiveWeapon());
	/*if (!wpn) {
		warning("started attack for NPC %d with no weapon", _itemNum);
	}*/

	AudioProcess *audio = AudioProcess::get_instance();
	const Direction curdir = a->getDir();
	const int32 now = Kernel::get_instance()->getTickNum();
	int wpnField8 = wpn ? wpn->getShapeInfo()->_weaponInfo->_field8 : 1;
	const uint16 controlledNPC = World::get_instance()->getControlledNPCNum();
	Direction targetdir = dir_invalid;

	if (_target != controlledNPC) {
		if (controlledNPC <= 1)
			_target = 1;
		else
			_target = controlledNPC;
	}

	Actor *target = getActor(_target);
	if (!target || !target->isOnScreen() || target->isDead()) {
		// Walk around randomly in hope of finding target
		_target = 0;
		if (!_isActivity9orB) {
			int32 x, y, z;
			a->getLocation(x, y, z);
			x += -0x1ff + randomOf(0x400);
			y += -0x1ff + randomOf(0x400);
			_field96 = true;
			const ProcId pid = Kernel::get_instance()->addProcess(
								new PathfinderProcess(a, x, y, z));
			waitFor(pid);
			return;
		}
	} else {
		Animation::Sequence anim;
		if (a->isInCombat()) {
			anim = Animation::combatStand;
		} else {
			anim = Animation::stand;
		}
		DirectionMode standDirMode = a->animDirMode(anim);
		if (_timer3set) {
			if (_timer3 >= now) {
				if (a->isInCombat()) {
					if (randomOf(3) != 0) {
						const Animation::Sequence lastanim = a->getLastAnim();
						if ((lastanim != Animation::unreadyWeapon)) // TODO: && (lastanim != Animation::unreadyLargeWeapon))
							a->doAnim(Animation::unreadyWeapon, dir_current);
						else
							a->doAnim(Animation::readyWeapon, dir_current);
						return;
					}

					if (randomOf(3) == 0) {
						// TODO: this should be a random dir based on some
						// NPC field and stuff.
						a->turnTowardDir(Direction_TurnByDelta(curdir, getRandom() % 8, dirmode_8dirs));
						return;
					}

					if (_target == 0)
						return;

					if (curdir != a->getDirToItemCentre(*target))
						return;

					if (_soundNo != -1) {
						if (audio->isSFXPlayingForObject(_soundNo, _itemNum))
							return;
						_soundNo = -1;
					}

					_wpnField8 = wpnField8;
					if (_wpnField8 < 3) {
						_wpnField8 = 1;
					} else if ((_doubleDelay && (getRandom() % 2 == 0)) || (getRandom() % 5 == 0)) {
						// TODO: a->setField0x68(1);
						_wpnField8 *= 4;
					}
					_fireTimestamp = now;
					if (_timer4 == 0)
						_timer4 = now;

					const ProcId animpid = a->doAnim(Animation::attack, dir_current); // fire small weapon.
					if (animpid == 0) {
						return;
					}
					waitFor(animpid);
					_wpnField8--;
					if (_wpnField8 == 0)
						return;

					// TODO: this is not correct - should be Process_11e0_15ab(animpid); (not waitFor)..
					waitFor(animpid);
					return;
				}
			} else {
				_timer3set = false;
				a->setActivity(5);
			}
		}
		if (targetdir == dir_invalid) {
			targetdir = a->getDirToItemCentre(*target);
		}

		Point3 apt, tpt;
		a->getLocation(apt);
		target->getLocation(tpt);
		const int32 dist = apt.maxDistXYZ(tpt);
		const int32 zdiff = abs(a->getZ() - target->getZ());
		// FIXME: is this the right function??
		const bool onscreen = a->isOnScreen();
		if ((!_isActivity9orB && !onscreen) || (dist <= zdiff)) {
			pathfindToItemInNPCData();
			return;
		}
		if (targetdir == curdir) {
			const uint16 rnd = randomOf(10);
			if (!onscreen ||
				(!_field96 && !timer4and5Update(now) && !World_FinishedAvatarMoveTimeout()
				 && rnd > 2 && (!_isActivityAorB || rnd > 3))) {
				sleep(0x14);
				return;
			}

			_field96 = false;
			bool ready;
			if (now - a->getLastTimeWasHit() < 120)
				ready = checkReady(now, targetdir);
			else
				ready = true;

			if (_timer2set && (randomOf(5) == 0 || checkTimer2PlusDelayElapsed(now))) {
				_timer2set = false;
			}

			if (!ready) {
				if (_isActivity9orB == 0)
					pathfindToItemInNPCData();
				else
					sleep(0xf);
				return;
			}

			checkRandomAttackSound(now, a->getShape());

			if (!a->hasActorFlags(Actor::ACT_WEAPONREADY)) {
				_timer4 = now;
				a->doAnim(Animation::readyWeapon, dir_current); // ready small wpn
				return;
			}

			// Wait until sound is finished playing.
			if (_soundNo != -1) {
				if (audio->isSFXPlayingForObject(_soundNo, _itemNum))
					return;
				_soundNo = -1;
			}

			const int32 t5elapsed = now - _timer5;
			if (t5elapsed > _wpnBasedTimeout) {
				const int32 fireelapsed = now - _fireTimestamp;
				if (fireelapsed <= _difficultyBasedTimeout) {
					sleep(_difficultyBasedTimeout - fireelapsed);
					return;
				}

				if (wpn) {
					_wpnField8 = wpnField8;
					if (_wpnField8 > 2 && ((_doubleDelay && randomOf(2) == 0) || randomOf(5) == 0)) {
						// TODO: a->setField0x68(1);
						_wpnField8 *= 4;
					}
				}

				_fireTimestamp = now;
				if (_timer4 == 0) {
					_timer4 = now;
				}

				const ProcId firepid = a->doAnim(Animation::attack, dir_current); // Fire SmallWpn
				if (firepid != 0) {
					waitFor(firepid);
					_wpnField8--;
					if (_wpnField8 != 0) {
						// TODO: this is not correct - should be Process_11e0_15ab(firepid); (not waitFor)..
						waitFor(firepid);
						return;
					}
				}
			} else if (t5elapsed != 0) {
				sleep(_wpnBasedTimeout - t5elapsed);
				return;
			}
		} else {
			bool local_1b;
			if (!timer4and5Update(now) && !_field7f) {
				if (standDirMode != dirmode_16dirs) {
					targetdir = a->getDirToItemCentre(*target);
				}
				local_1b = a->fireDistance(target, targetdir, 0, 0, 0);
				if (local_1b)
					timeNowToTimerVal2(now);
			} else {
				timeNowToTimerVal2(now);
				local_1b = true;
				_field7f = false;
			}

			// 5a flag 1 set?
			if (!a->hasActorFlags(Actor::ACT_WEAPONREADY) && local_1b) {
				_timer4 = now;
				a->doAnim(Animation::readyWeapon, dir_current); // ready SmallWpn
				return;
			}
			if (local_1b || _isActivity9orB) {
				a->turnTowardDir(targetdir);
				return;
			}
			pathfindToItemInNPCData();
		}
	}
}

void AttackProcess::checkRandomAttackSound(int now, uint32 shapeno) {
	AudioProcess *audio = AudioProcess::get_instance();
	int16 attacksound = -1;
	if (!_playedStartSound) {
		_playedStartSound = true;
		if (randomOf(3) == 0) {
			switch(shapeno) {
				case 0x371:
					attacksound = RANDOM_ELEM(ATTACK_SFX_3);
					break;
				case 0x1b4:
					attacksound = RANDOM_ELEM(ATTACK_SFX_5);
					break;
				case 0x2fd:
				case 0x319:
					attacksound = RANDOM_ELEM(ATTACK_SFX_1);
					break;
				case 900:
					attacksound = RANDOM_ELEM(ATTACK_SFX_2);
					break;
				case 0x4d1:
				case 0x528:
					attacksound = RANDOM_ELEM(ATTACK_SFX_4);
					break;
				default:
					break;
			}
		}
	} else {
		if (readyForNextSound(now)) {
			if (shapeno == 0x2df)
				attacksound = RANDOM_ELEM(ATTACK_SFX_6);
			else if (shapeno == 899)
				attacksound = RANDOM_ELEM(ATTACK_SFX_7);
		}
	}

	if (attacksound != -1) {
		_soundNo = attacksound;
		audio->playSFX(attacksound, 0x80, _itemNum, 1);
	}
}

bool AttackProcess::readyForNextSound(int now) {
	if (_soundTimestamp == 0 || _soundTimestamp - now >= 480) {
		_soundTimestamp = now;
		return true;
	}
	return false;
}

bool AttackProcess::checkTimer2PlusDelayElapsed(int now) {
	int delay = 60;
	if (_doubleDelay)
		delay *= 2;
	return (now > _timer2 + delay);
}

void AttackProcess::setAttackData(uint16 off, uint16 val) {
	if (off >= MAGIC_DATA_OFF && off < MAGIC_DATA_OFF + ARRAYSIZE(_dataArray) - 1)
		_dataArray[off - MAGIC_DATA_OFF] = val;

	warning("Invalid offset to setAttackDataArray %d %d", off, val);
}

uint16 AttackProcess::getAttackData(uint16 off) const {
	if (off >= MAGIC_DATA_OFF && off < MAGIC_DATA_OFF + ARRAYSIZE(_dataArray) - 1)
		return _dataArray[off - MAGIC_DATA_OFF];

	warning("Invalid offset to getAttackDataArray: %d", off);
	return 0;
}

void AttackProcess::pathfindToItemInNPCData() {
	_doubleDelay = false;
	_timer2set = false;
	_field96 = true;

	Actor *a = getActor(_itemNum);
	Actor *target = getActor(_target);

	Process *pathproc = new PathfinderProcess(a, target->getObjId());
	// In case pathfinding fails delay for a bit to ensure we don't get
	// stuck in a tight loop using all the cpu
	Process *delayproc = new DelayProcess(10);
	Kernel::get_instance()->addProcess(pathproc);
	Kernel::get_instance()->addProcess(delayproc);
	delayproc->waitFor(pathproc);
	waitFor(delayproc);
}

bool AttackProcess::timer4and5Update(int now) {
	int32 delay = 120;
	if (_doubleDelay) {
		delay = 240;
	}

	if (_timer4) {
		_timer5 = _timer4;
		if (_timer4 + delay >= now) {
			return true;
		}
	}

	_timer4 = 0;
	_doubleDelay = false;
	return false;
}

bool AttackProcess::checkReady(int now, Direction targetdir) {
	if (timer4and5Update(now) || _timer2set) {
		return true;
	}

	Actor *a = getActor(_itemNum);
	Actor *target = getActor(_target);
	if (!a || !target)
		return false;
	return a->fireDistance(target, targetdir, 0, 0, 0) > 0;
}

void AttackProcess::timeNowToTimerVal2(int now) {
	_timer2 = now;
	_timer2set = true;
}

void AttackProcess::setTimer3() {
	const int32 now = Kernel::get_instance()->getTickNum();
	_timer3set = true;
	_timer3 = randomOf(10) * 60 + now;
	return;
}

void AttackProcess::sleep(int ticks) {
	Process *delayProc = new DelayProcess(ticks);
	ProcId pid = Kernel::get_instance()->addProcess(delayProc);
	waitFor(pid);
}

void AttackProcess::setTacticNo(int tactic) {
	assert(tactic < 32);
	_tactic = tactic;
	_tacticDat = GameData::get_instance()->getCombatDat(tactic);
	delete _tacticDatReadStream;
	_tacticDatReadStream = new Common::MemoryReadStream(_tacticDat->getData(), _tacticDat->getDataLen());
	setBlockNo(0);
}

void AttackProcess::setBlockNo(int block) {
	_block = block;

	if (!_tacticDat)
		return;

	_tacticDatStartOffset = _tacticDat->getOffset(block);
	_tacticDatReadStream->seek(_tacticDatStartOffset, SEEK_SET);
}

uint16 AttackProcess::readNextWordWithData() {
	uint16 data = _tacticDatReadStream->readUint16LE();
	if (data >= MAGIC_DATA_OFF) {
		data = getAttackData(data);
	}

	return data;
}

uint16 AttackProcess::readNextWordRaw() {
	assert(_tacticDatReadStream);
	return _tacticDatReadStream->readUint16LE();
}

void AttackProcess::dumpInfo() const {
	Process::dumpInfo();

}

void AttackProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint16LE(_target);
	ws->writeUint16LE(_tactic);
	ws->writeUint16LE(_block);
	ws->writeUint16LE(_tacticDatStartOffset);

	ws->writeUint16LE(_soundNo);
	ws->writeByte(_playedStartSound ? 1 : 0);
	ws->writeByte(Direction_ToUsecodeDir(_npcInitialDir));

	ws->writeSint16LE(_field57);
	ws->writeUint16LE(_field59);
	ws->writeByte(_field7f ? 1 : 0);
	ws->writeByte(_field96 ? 1 : 0);
	ws->writeByte(_field97 ? 1 : 0);

	ws->writeByte(_isActivity9orB ? 1 : 0);
	ws->writeByte(_isActivityAorB ? 1 : 0);
	ws->writeByte(_timer2set ? 1 : 0);
	ws->writeByte(_timer3set ? 1 : 0);
	ws->writeByte(_doubleDelay ? 1 : 0);

	ws->writeUint16LE(_wpnField8);

	for (int i = 0; i < ARRAYSIZE(_dataArray); i++) {
		ws->writeUint16LE(_dataArray[i]);
	}

	ws->writeSint32LE(_wpnBasedTimeout);
	ws->writeSint32LE(_difficultyBasedTimeout);

	ws->writeSint32LE(_timer2);
	ws->writeSint32LE(_timer3);
	ws->writeSint32LE(_timer4);
	ws->writeSint32LE(_timer5);
	ws->writeSint32LE(_soundTimestamp);
	ws->writeSint32LE(_fireTimestamp);
}

bool AttackProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_target = rs->readUint16LE();
	setTacticNo(rs->readUint16LE());
	setBlockNo(rs->readUint16LE());
	_tacticDatStartOffset = rs->readUint16LE();

	_soundNo = rs->readUint16LE();
	_playedStartSound = rs->readByte();
	_npcInitialDir = Direction_FromUsecodeDir(rs->readByte());

	_field57 = rs->readSint16LE();
	_field59 = rs->readUint16LE();
	_field7f = rs->readByte();
	_field96 = rs->readByte();
	_field97 = rs->readByte();

	_isActivity9orB= rs->readByte();
	_isActivityAorB = rs->readByte();
	_timer2set = rs->readByte();
	_timer3set = rs->readByte();
	_doubleDelay = rs->readByte();

	_wpnField8 = rs->readUint16LE();

	for (int i = 0; i < ARRAYSIZE(_dataArray); i++) {
		_dataArray[i] = rs->readUint16LE();
	}

	_wpnBasedTimeout = rs->readSint32LE();
	_difficultyBasedTimeout = rs->readSint32LE();

	_timer2 = rs->readSint32LE();
	_timer3 = rs->readSint32LE();
	_timer4 = rs->readSint32LE();
	_timer5 = rs->readSint32LE();
	_soundTimestamp = rs->readSint32LE();
	_fireTimestamp = rs->readSint32LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
