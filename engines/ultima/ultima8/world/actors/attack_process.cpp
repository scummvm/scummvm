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


#include "ultima/ultima8/world/actors/attack_process.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/actors/combat_dat.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/cru_pathfinder_process.h"
#include "ultima/ultima8/misc/direction_util.h"

namespace Ultima {
namespace Ultima8 {

//#define WATCHACTOR 3

DEFINE_RUNTIME_CLASSTYPE_CODE(AttackProcess)

// These sound number arrays are in the order they appear in the original exes

static const int16 REM_SFX_1[] = {0x15, 0x78, 0x80, 0x83, 0xDC, 0xDD};
static const int16 REM_SFX_2[] = {0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xE7};
static const int16 REM_SFX_3[] = {0xFC, 0xFD, 0xFE, 0xC8};
static const int16 REM_SFX_4[] = {0xCC, 0xCD, 0xCE, 0xCF};
static const int16 REM_SFX_5[] = {0xC7, 0xCA, 0xC9};
static const int16 REM_SFX_6[] = {0x82, 0x84, 0x85};
static const int16 REM_SFX_7[] = {0x9B, 0x9C, 0x9D, 0x9E, 0x9F};

static const int16 REG_SFX_1[] = { 0xD2, 0xD3, 0xD4, 0xD5, 0xE5, 0x100 };
static const int16 REG_SFX_2[] = { 0x9, 0x79, 0x7A, 0x7B, 0x7C, 0x7D };
static const int16 REG_SFX_3[] = { 0x7E, 0x7F, 0x90, 0xB6, 0xC2, 0xD0 };
static const int16 REG_SFX_4[] = { 0x101, 0x102, 0x103, 0x104, 0x105, 0x106 };
static const int16 REG_SFX_5[] = { 0x108, 0x109, 0x1AB, 0x1AC, 0x1AD, 0x1AF, 0x1AE };
static const int16 REG_SFX_6[] = { 0x1B0, 0x1B1, 0x1B2, 0x1B3, 0x1B4 };
static const int16 REG_SFX_7[] = { 0x1B5, 0x1B6, 0x1B7, 0x1B8, 0x1B9, 0x1BA, 0x1BB };
static const int16 REG_SFX_8[] = { 0x1C1, 0x1C0, 0x1BF, 0x1BE, 0x1BD, 0x1BC };
static const int16 REG_SFX_9[] = { 0x1C2, 0x1C3, 0x1C4, 0x1C5, 0x1C6, 0x1C7 };
static const int16 REG_SFX_10[] = { 0x1C8, 0x1C9, 0x1CA, 0x1CB, 0x1CC, 0x1CD };
static const int16 REG_SFX_11[] = { 0x1D0, 0x1D1, 0x1D2, 0x1D3, 0x1D4, 0x1D5 };
static const int16 REG_SFX_12[] = { 0x1D7, 0x1D8, 0x1D9, 0x1DA, 0x1DB, 0x1DC };
static const int16 REG_SFX_13[] = { 0x1DD, 0x1DE, 0x1DF, 0x1E0, 0x1E1, 0x1E2, 0x1E3 };
static const int16 REG_SFX_14[] = { 0x9B, 0x9C, 0x9D, 0x9E, 0x9F };
static const int16 REG_SFX_15[] = { 0x1E7, 0x1E8, 0x1E9, 0x1EA, 0x1ED };

#define RANDOM_ELEM(array) (array[rs.getRandomNumber(ARRAYSIZE(array) - 1)])

// If data is referenced in the metalang with an offset of this or greater,
// read from the data array.
static const int MAGIC_DATA_OFF = 33000;

int16 AttackProcess::_lastAttackSound = -1;
int16 AttackProcess::_lastLastAttackSound = -1;

static uint16 someSleepGlobal = 0;

AttackProcess::AttackProcess() : Process(), _block(0), _target(1), _tactic(0), _tacticDat(nullptr),
_tacticDatReadStream(nullptr), _tacticDatStartOffset(0), _soundNo(-1), _playedStartSound(false),
_npcInitialDir(dir_invalid), _field57(0), _field59(0), _field7f(false), _field96(false), _field97(false),
_isActivity9orB(false), _isActivityAorB(false), _timer3set(false), _timer2set(false),
_doubleDelay(false), _wpnField8(1), _wpnBasedTimeout(0), _difficultyBasedTimeout(0), _timer2(0),
_timer3(0), _timer4(0), _timer5(0), _soundTimestamp(0), _soundDelayTicks(480), _fireTimestamp(0) {
	for (int i = 0; i < ARRAYSIZE(_dataArray); i++) {
		_dataArray[i] = 0;
	}
	if (GAME_IS_REGRET) {
		Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
		_soundDelayTicks = rs.getRandomNumberRng(10, 24) * 60;
		if (rs.getRandomNumber(2) == 0)
			_soundTimestamp = Kernel::get_instance()->getTickNum();
	}
}

AttackProcess::AttackProcess(Actor *actor) : _block(0), _target(1), _tactic(0), _tacticDat(nullptr),
_tacticDatReadStream(nullptr), _tacticDatStartOffset(0), _soundNo(-1), _playedStartSound(false),
_field57(0), _field59(0), _field7f(false), _field96(false), _field97(false), _isActivity9orB(false),
_isActivityAorB(false), _timer3set(false), _timer2set(false), _doubleDelay(false), _wpnField8(1),
_wpnBasedTimeout(0), _difficultyBasedTimeout(0), _timer2(0), _timer3(0), _timer4(0), _timer5(0),
_soundTimestamp(0), _soundDelayTicks(480), _fireTimestamp(0) {
	assert(actor);
	_itemNum = actor->getObjId();
	_npcInitialDir = actor->getDir();

	// Note: this isn't actually initialized in the original which
	// suggests it can't ever get used before setting, but to make
	// coverity etc happy clear it anyway.
	for (int i = 0; i < ARRAYSIZE(_dataArray); i++) {
		_dataArray[i] = 0;
	}

	if (GAME_IS_REGRET) {
		Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
		_soundDelayTicks = rs.getRandomNumberRng(10, 24) * 60;
		if (rs.getRandomNumber(2) == 0)
			_soundTimestamp = Kernel::get_instance()->getTickNum();
	}

	actor->setAttackAimFlag(false);

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

	_type = ATTACK_PROC_TYPE;

	setTacticNo(actor->getCombatTactic());
	actor->setToStartOfAnim(Animation::stand);
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

	if (!a || a->isDead() || !_tacticDatReadStream) {
		terminate();
		return;
	}

	if (!a->hasFlags(Item::FLG_FASTAREA))
		return;

	if (_tactic == 0) {
		genericAttack();
		return;
	}

	if (!target || target->isDead()) {
		warning("got into attack process with invalid target");
		terminate();
		return;
	}

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
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
					   new CruPathfinderProcess(a, Point3(x, y, z), 100, 0x80, true));
			waitFor(pid);
			return;
		}
		case 0x8e:
		{
			// Pathfind to target
			Point3 pt = target->getLocation();
			ProcId pid = Kernel::get_instance()->addProcess(
					   new CruPathfinderProcess(a, pt, 12, 0x80, true));
			waitFor(pid);
			return;
		}
		case 0x8f:
		{
			// Pathfind to a point between npc and the target
			Point3 apt = a->getLocation();
			Point3 tpt = target->getLocation();
			int32 x = (tpt.x + apt.x) / 2;
			int32 y = (tpt.y + apt.y) / 2;
			int32 z = (tpt.z + apt.z) / 2;
			ProcId pid = Kernel::get_instance()->addProcess(
					   new CruPathfinderProcess(a, Point3(x, y, z), 12, 0x80, true));
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
				ticks = rs.getRandomNumberRng(0x31, 0x45);
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
			Point3 apt = a->getLocation();
			Point3 tpt = target->getLocation();
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
			Point3 apt = a->getLocation();
			Point3 tpt = target->getLocation();
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
			uint16 offset = readNextWordWithData();
			if (maxval != 0) {
				uint16 randval = rs.getRandomNumber(maxval - 1);
				if (randval != 0) {
					_tacticDatReadStream->seek(offset);
				}
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
						 new CruPathfinderProcess(a, founditem, 100, 0x80, true));
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

	if (a->isBusy() || a->hasActorFlags(Actor::ACT_PATHFINDING)) {
		return;
	}

#ifdef WATCHACTOR
	if (_itemNum == WATCHACTOR)
		debug("Attack: actor %d genericAttack (not busy or pathfinding)", _itemNum);
#endif

	// This should never be running on the controlled npc.
	if (_itemNum == World::get_instance()->getControlledNPCNum()) {
		terminate();
		return;
	}

	const Item *wpn = getItem(a->getActiveWeapon());
	/*if (!wpn) {
		warning("started attack for NPC %d with no weapon", _itemNum);
	}*/

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	AudioProcess *audio = AudioProcess::get_instance();
	const Direction curdir = a->getDir();
	const int32 ticknow = Kernel::get_instance()->getTickNum();
	int wpnField8 = wpn ? wpn->getShapeInfo()->_weaponInfo->_field8 : 1;
	const uint16 controlledNPC = World::get_instance()->getControlledNPCNum();
	Direction targetdir = dir_invalid;

	if (_target != controlledNPC) {
		if (controlledNPC <= 1)
			_target = 1;
		else
			_target = controlledNPC;
	}

#ifdef WATCHACTOR
	if (_itemNum == WATCHACTOR)
		debug("Attack: genericAttack chose target %d", _target);
#endif

	Actor *target = getActor(_target);
	if (!target || !target->isOnScreen() || target->isDead()) {
		// Walk around randomly in hope of finding target
		_target = 0;
		if (!_isActivity9orB) {
#ifdef WATCHACTOR
			if (_itemNum == WATCHACTOR)
				debug("Attack: genericAttack walking around looking for target %d", _target);
#endif
			Point3 pt = a->getLocation();
			pt.x += rs.getRandomNumberRngSigned(-0x1ff, 0x1ff);
			pt.y += rs.getRandomNumberRngSigned(-0x1ff, 0x1ff);
			_field96 = true;
			const ProcId pid = Kernel::get_instance()->addProcess(
								new CruPathfinderProcess(a, pt, 12, 0x80, true));
			// add a tiny delay to avoid tight loops
			Process *delayproc = new DelayProcess(2);
			Kernel::get_instance()->addProcess(delayproc);
			delayproc->waitFor(pid);
			waitFor(delayproc);
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
#ifdef WATCHACTOR
			if (_itemNum == WATCHACTOR)
				debug("Attack: _timer3set");
#endif
			if (_timer3 >= ticknow) {
				if (a->isInCombat()) {
					if (rs.getRandomNumber(2) != 0) {
#ifdef WATCHACTOR
						if (_itemNum == WATCHACTOR)
							debug("Attack: toggle weapon state");
#endif
						const Animation::Sequence lastanim = a->getLastAnim();
						if ((lastanim != Animation::unreadyWeapon) && (lastanim != Animation::unreadyLargeWeapon))
							a->doAnim(Animation::unreadyWeapon, dir_current);
						else
							a->doAnim(Animation::readyWeapon, dir_current);
						return;
					}

					if (rs.getRandomNumber(2) == 0) {
						a->turnTowardDir(Direction_TurnByDelta(curdir, rs.getRandomNumber(7), dirmode_8dirs));
						return;
					}

					if (!a->hasActorFlags(Actor::ACT_WEAPONREADY))
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
					} else if ((_doubleDelay && rs.getRandomNumber(1) == 0) || (rs.getRandomNumber(4) == 0)) {
						a->setAttackAimFlag(true);
						_wpnField8 *= 4;
					}
					_fireTimestamp = ticknow;
					if (_timer4 == 0)
						_timer4 = ticknow;

#ifdef WATCHACTOR
					if (_itemNum == WATCHACTOR)
						debug("Attack: firing weapon at tick %d!", ticknow);
#endif

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

		Point3 apt = a->getLocation();
		Point3 tpt = target->getLocation();
		const int32 dist = apt.maxDistXYZ(tpt);
		const int32 zdiff = abs(a->getZ() - target->getZ());
		const bool onscreen = a->isPartlyOnScreen(); // note: original uses "isMajorityOnScreen", this is close enough.
		if ((!_isActivity9orB && !onscreen) || (dist <= zdiff)) {
#ifdef WATCHACTOR
			if (_itemNum == WATCHACTOR)
				debug("Attack: Not 9/B and actor not onscreen or dist %d < zdiff %d, pathfinding", dist, zdiff);
#endif
			pathfindToItemInNPCData();
			return;
		}
		if (targetdir == curdir) {
#ifdef WATCHACTOR
			if (_itemNum == WATCHACTOR)
				debug("Attack: targetdir == currentdir");
#endif
			const uint16 rnd = rs.getRandomNumber(9);
			const uint32 frameno = Kernel::get_instance()->getFrameNum();
			const uint32 timeoutfinish = target->getAttackMoveTimeoutFinishFrame();

			if (!onscreen ||
				(!_field96 && !timer4and5Update(ticknow) && frameno < timeoutfinish
				 && rnd > 2 && (!_isActivityAorB || rnd > 3))) {
				sleep(0x14);
				return;
			}

			_field96 = false;
			bool ready;
			if (ticknow - a->getLastTickWasHit() <= 120)
				ready = true;
			else
				ready = checkReady(ticknow, targetdir);

			if (_timer2set && (rs.getRandomNumber(4) == 0 || checkTimer2PlusDelayElapsed(ticknow))) {
				_timer2set = false;
			}

			if (!ready) {
				if (!_isActivity9orB)
					pathfindToItemInNPCData();
				else
					sleep(0xf);
				return;
			}

			checkRandomAttackSound(ticknow, a->getShape());

			if (!a->hasActorFlags(Actor::ACT_WEAPONREADY)) {
				_timer4 = ticknow;
				a->doAnim(Animation::readyWeapon, dir_current); // ready small wpn
				return;
			}

			// Wait until sound is finished playing.
			if (_soundNo != -1) {
				if (audio->isSFXPlayingForObject(_soundNo, _itemNum))
					return;
				_soundNo = -1;
			}

			const int32 t5elapsed = ticknow - _timer5;
			if (t5elapsed > _wpnBasedTimeout) {
				const int32 fireelapsed = ticknow - _fireTimestamp;
				if (fireelapsed <= _difficultyBasedTimeout) {
					sleep(_difficultyBasedTimeout - fireelapsed);
					return;
				}

				if (!wpn) {
					_wpnField8 = 1;
				} else {
					_wpnField8 = wpnField8;
					if (_wpnField8 > 2 && ((_doubleDelay && rs.getRandomNumber(1) == 0) || rs.getRandomNumber(4) == 0)) {
						a->setAttackAimFlag(true);
						_wpnField8 *= 4;
					}
				}

				_fireTimestamp = ticknow;
				if (_timer4 == 0) {
					_timer4 = ticknow;
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
#ifdef WATCHACTOR
			if (_itemNum == WATCHACTOR)
				debug("Attack: targetdir != currentdir");
#endif
			bool ready;
			if (!timer4and5Update(ticknow) && !_field7f) {
				if (standDirMode != dirmode_16dirs) {
					targetdir = a->getDirToItemCentre(*target);
				}
				ready = a->fireDistance(target, targetdir, 0, 0, 0);
				if (ready)
					timeNowToTimerVal2(ticknow);
			} else {
				timeNowToTimerVal2(ticknow);
				ready = true;
				_field7f = false;
			}

			// 5a flag 1 set?
			if (!a->hasActorFlags(Actor::ACT_WEAPONREADY) && ready) {
				_timer4 = ticknow;
				a->doAnim(Animation::readyWeapon, dir_current); // ready SmallWpn
				return;
			}
			if (ready || _isActivity9orB) {
				a->turnTowardDir(targetdir);
				return;
			}
			pathfindToItemInNPCData();
		}
	}
}

void AttackProcess::checkRandomAttackSoundRegret(const Actor *actor) {
	if (!readyForNextSound(Kernel::get_instance()->getTickNum()))
		return;

	AudioProcess *audio = AudioProcess::get_instance();
	if (audio->isSFXPlayingForObject(-1, actor->getObjId()))
		return;

	int16 sndno = getRandomAttackSoundRegret(actor);

	if (sndno != -1 && _lastAttackSound != sndno && _lastLastAttackSound != sndno) {
		_lastLastAttackSound = _lastAttackSound;
		_lastAttackSound = sndno;
		_soundNo = sndno;
		audio->playSFX(sndno, 0x80, actor->getObjId(), 1);
	}
}

/* static */
int16 AttackProcess::getRandomAttackSoundRegret(const Actor *actor) {
	if (World::get_instance()->getControlledNPCNum() != kMainActorId)
		return -1;

	if (actor->isDead())
		return -1;

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	uint32 shapeno = actor->getShape();

	int16 sndno = -1;
	// The order here is pretty random, how it comes out of the disasm.
	switch (shapeno) {
	  case 0x4e0:
		  sndno = RANDOM_ELEM(REG_SFX_8);
		  break;
	  case 899:
		  sndno = RANDOM_ELEM(REG_SFX_14);
		  break;
	  case 900:
		  sndno = RANDOM_ELEM(REG_SFX_7);
		  break;
	  case 0x4d1:
	  case 0x528:
		  sndno = RANDOM_ELEM(REG_SFX_3);
		  break;
	  case 0x344:
		  sndno = RANDOM_ELEM(REG_SFX_13);
		  break;
	  case 0x371:
	  case 0x62f:
	  case 0x630:
		  sndno = RANDOM_ELEM(REG_SFX_2);
		  break;
	  case 0x2f5:
		  sndno = RANDOM_ELEM(REG_SFX_9);
		  break;
	  case 0x2f6:
		  sndno = RANDOM_ELEM(REG_SFX_12);
		  break;
	  case 0x2f7:
	  case 0x595:
		  sndno = RANDOM_ELEM(REG_SFX_11);
		  break;
	  case 0x2df:
		  sndno = RANDOM_ELEM(REG_SFX_6);
		  break;
	  case 0x597:
		  sndno = RANDOM_ELEM(REG_SFX_10);
		  break;
	  case 0x5b1:
		  sndno = RANDOM_ELEM(REG_SFX_15);
		  break;
	  case 0x5ff:
	  case 0x5d7:
		  sndno = RANDOM_ELEM(REG_SFX_5);
		  break;
	  case 0x1b4:
	  case 0x625:
		  sndno = RANDOM_ELEM(REG_SFX_4);
		  break;
	  case 0x5f0:
	  case 0x308:
		  sndno = RANDOM_ELEM(REG_SFX_1);
		  break;
	  default:
		  break;
	}

	return sndno;
}

void AttackProcess::checkRandomAttackSound(int now, uint32 shapeno) {
	if (GAME_IS_REGRET) {
		checkRandomAttackSoundRegret(getActor(_itemNum));
		return;
	}

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	AudioProcess *audio = AudioProcess::get_instance();
	int16 attacksound = -1;
	if (!_playedStartSound) {
		_playedStartSound = true;
		if (rs.getRandomNumber(2) == 0) {
			switch(shapeno) {
				case 0x371:
					attacksound = RANDOM_ELEM(REM_SFX_3);
					break;
				case 0x1b4:
					attacksound = RANDOM_ELEM(REM_SFX_5);
					break;
				case 0x2fd:
				case 0x319:
					attacksound = RANDOM_ELEM(REM_SFX_1);
					break;
				case 900:
					attacksound = RANDOM_ELEM(REM_SFX_2);
					break;
				case 0x4d1:
				case 0x528:
					attacksound = RANDOM_ELEM(REM_SFX_4);
					break;
				default:
					break;
			}
		}
	} else {
		if (readyForNextSound(now)) {
			if (shapeno == 0x2df)
				attacksound = RANDOM_ELEM(REM_SFX_6);
			else if (shapeno == 899)
				attacksound = RANDOM_ELEM(REM_SFX_7);
		}
	}

	if (attacksound != -1) {
		_soundNo = attacksound;
		audio->playSFX(attacksound, 0x80, _itemNum, 1);
	}
}

bool AttackProcess::readyForNextSound(uint32 now) {
	if (_soundTimestamp == 0 || now - _soundTimestamp >= _soundDelayTicks) {
		_soundTimestamp = now;
		return true;
	}
	return false;
}

bool AttackProcess::checkTimer2PlusDelayElapsed(int now) {
	int delay = 60;
	if (_doubleDelay)
		delay *= 2;
	return (now >= _timer2 + delay);
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
#ifdef WATCHACTOR
	if (_itemNum == WATCHACTOR)
		debug("Attack: pathfindToItemInNPCData");
#endif
	_doubleDelay = false;
	_timer2set = false;
	_field96 = true;

	Actor *a = getActor(_itemNum);
	Actor *target = getActor(_target);

	Process *pathproc = new CruPathfinderProcess(a, target, 12, 0x80, false);
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

#ifdef WATCHACTOR
	if (_itemNum == WATCHACTOR)
		debug("Attack: timer4and5Update (doubledelay=%d, timer4=%d, timer5=%d)",
			  _doubleDelay, _timer4, _timer5);
#endif

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
	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	const int32 now = Kernel::get_instance()->getTickNum();
	_timer3set = true;
	_timer3 = rs.getRandomNumber(9) * 60 + now;
	return;
}

void AttackProcess::sleep(int ticks) {
	// waiting less than 2 ticks can cause a tight loop
#ifdef WATCHACTOR
	if (_itemNum == WATCHACTOR)
		debug("Attack: sleeping for %d", ticks);
#endif
	ticks = MAX(ticks, 2);
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

Common::String AttackProcess::dumpInfo() const {
	return Process::dumpInfo();
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
	ws->writeSint32LE(_soundTimestamp); // bug: this should ideally be unsigned, probably won't make any difference.
	// Don't write the sound delay because it only affects
	// No Regret, adding it now would need a version bump, and
	// and it doesn't make much difference to re-randomize it.
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
