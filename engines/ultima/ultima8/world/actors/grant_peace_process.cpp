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
#include "ultima/ultima8/world/actors/grant_peace_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/gumps/target_gump.h"
#include "ultima/ultima8/world/weapon_info.h"
#include "ultima/ultima8/graphics/palette_fader_process.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(GrantPeaceProcess)

GrantPeaceProcess::GrantPeaceProcess() : Process(), _haveTarget(false) {
}

GrantPeaceProcess::GrantPeaceProcess(Actor *caster) {
	assert(caster);
	_itemNum = caster->getObjId();

	_type = 0x21d; // CONSTANT !

	_haveTarget = false;
}

void GrantPeaceProcess::run() {
	Actor *caster = getActor(_itemNum);
	if (!caster) {
		terminate();
		return;
	}

	if (!_haveTarget) {
		TargetGump *targetgump = new TargetGump(0, 0);
		targetgump->InitGump(0);

		waitFor(targetgump->GetNotifyProcess()->getPid());

		_haveTarget = true;

		return;
	}

	// get target _result
	ObjId targetid = static_cast<ObjId>(_result);
	Actor *target = getActor(targetid);

	if (targetid == 1 || !target) {
		// targeting the avatar, no target or not an Actor
		terminate();
		return;
	}

	bool hit = false;

	if (target->getDefenseType() & WeaponInfo::DMG_UNDEAD) {
		// undead

		// first see if we're near Khumash-Gor
		CurrentMap *currentmap = World::get_instance()->getCurrentMap();
		UCList KGlist(2);
		LOOPSCRIPT(script, LS_SHAPE_EQUAL(289));
		currentmap->areaSearch(&KGlist, script, sizeof(script),
		                       caster, 2048, false);
		bool khumash = (KGlist.getSize() > 0);

		// then find all the undead in the area
		UCList itemlist(2);
		LOOPSCRIPT(script2, LS_TOKEN_TRUE);
		currentmap->areaSearch(&itemlist, script2, sizeof(script2),
		                       caster, 768, false);

		for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
			Actor *t = getActor(itemlist.getuint16(i));
			if (!t) continue;
			if (t == caster) continue;

			if (t->isDead()) continue;

			// undead?
			if (t->getDefenseType() & WeaponInfo::DMG_UNDEAD) {
				t->receiveHit(_itemNum, dir_current, target->getHP(),
				              (WeaponInfo::DMG_MAGIC |
				               WeaponInfo::DMG_PIERCE |
				               WeaponInfo::DMG_FIRE));
				hit = true;

				if (t->getShape() == 411 && khumash) { // CONSTANT!
					int32 tx, ty, tz;
					t->getLocation(tx, ty, tz);

					// CONSTANT! (shape 480, frame 0-9, repeat 1, delay 1)
					Process *sp = new SpriteProcess(480, 0, 9, 1, 1, tx, ty, tz);
					Kernel::get_instance()->addProcess(sp);

					Item *throne = getItem(KGlist.getuint16(0));
					if (throne) {
						throne->setFrame(1); // CONSTANT!
					}
				}

#if 0
				// FIXME: this seems to screw up the death animation; why?

				int dir = caster->getDirToItemCentre(*t);

				t->hurl(((getRandom() % 5) + 5) * x_fact[dir],
				        ((getRandom() % 5) + 5) * y_fact[dir],
				        ((getRandom() % 5) + 5),
				        4);
#endif
			}

		}

	} else {
		// not undead

		if (!target->hasActorFlags(Actor::ACT_DEAD |
								   Actor::ACT_IMMORTAL |
								   Actor::ACT_INVINCIBLE)) {
			if (getRandom() % 10 == 0) {
				target->receiveHit(_itemNum, dir_current, target->getHP(),
				                   (WeaponInfo::DMG_MAGIC |
				                    WeaponInfo::DMG_PIERCE |
				                    WeaponInfo::DMG_FIRE));
				hit = true;
			}
		}

	}


	if (hit) {
		// lightning

		// calling intrinsic...
		PaletteFaderProcess::I_lightningBolt(0, 0);
		int sfx;
		switch (getRandom() % 3) {
		case 0:
			sfx = 91;
			break;
		case 1:
			sfx = 94;
			break;
		default:
			sfx = 96;
			break;
		}

		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(sfx, 0x60, 1, 0); //constants!!
	}


	// done
	terminate();
}

uint32 GrantPeaceProcess::I_castGrantPeace(const uint8 *args,
        unsigned int /*argsize*/) {
	MainActor *avatar = getMainActor();

	GrantPeaceProcess *gpp = new GrantPeaceProcess(avatar);
	Kernel::get_instance()->addProcess(gpp);

	// start casting
	ProcId anim1 = avatar->doAnim(Animation::cast1, dir_current);

	// cast
	ProcId anim2 = avatar->doAnim(Animation::cast3, dir_current);
	Process *anim2p = Kernel::get_instance()->getProcess(anim2);

	// end casting
	ProcId anim3 = avatar->doAnim(Animation::cast2, dir_current);
	Process *anim3p = Kernel::get_instance()->getProcess(anim3);

	anim2p->waitFor(anim1);
	anim3p->waitFor(anim2);
	gpp->waitFor(anim2);

	return 0;
}

void GrantPeaceProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	uint8 ht = _haveTarget ? 1 : 0;
	ws->writeByte(ht);
}

bool GrantPeaceProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_haveTarget = (rs->readByte() != 0);

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
