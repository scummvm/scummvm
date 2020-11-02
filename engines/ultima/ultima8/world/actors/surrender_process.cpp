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
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/actors/surrender_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(SurrenderProcess)

static const uint16 SUIT_SUR_SNDS[] = {0xe9, 0xe0, 0xeb, 0xe1, 0xea};
static const uint16 CHEMSUIT_SUR_SNDS[] = {0xb4, 0xc5, 0xc6, 0xe8};
static const uint16 SCIENTIST_SUR_SNDS[] = {0xe3, 0xe4, 0xec, 0xf6};
static const uint16 HARDHAT_SUR_SNDS[] = {0xde, 0xdf, 0x8a, 0x8b};
static const uint16 FEMALE_SUR_SNDS[] = {0xd6, 0xff, 0xd7};

#define RANDOM_ELEM(array) (array[getRandom() % ARRAYSIZE(array)])

SurrenderProcess::SurrenderProcess() : Process(), _playedSound(false) {
}

SurrenderProcess::SurrenderProcess(Actor *actor) : _playedSound(false) {
	assert(actor);
	_itemNum = actor->getObjId();

	actor->doAnim(Animation::surrender, actor->getDir());

	_type = 0x25f; // CONSTANT!
}

void SurrenderProcess::run() {
	Actor *a = getActor(_itemNum);
	MainActor *main = getMainActor();
	if (!a || a->isDead() || !main) {
		// dead?
		terminate();
		return;
	}

	// do nothing while we are not in the fast area
	if (!a->hasFlags(Item::FLG_FASTAREA))
		return;

	int animating = Kernel::get_instance()->getNumProcesses(_itemNum, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE);
	if (animating) {
		// already busy.
		return;
	}

	a->setActorFlag(Actor::ACT_SURRENDERED);

	Direction curdir = a->getDir();
	Direction direction = a->getDirToItemCentre(*main);

	if (curdir != direction) {
		uint16 animpid = a->turnTowardDir(direction);
		if (animpid) {
			waitFor(animpid);
			return;
		}
	}

	if (_playedSound || a->getRangeIfVisible(*main) == 0)
		// Nothing to do.
		return;

	int16 soundno = -1;

	switch (a->getShape()) {
	case 0x2f7: // suit
		soundno = RANDOM_ELEM(SUIT_SUR_SNDS);
		break;
	case 0x2f5: // hardhat
		soundno = RANDOM_ELEM(HARDHAT_SUR_SNDS);
		break;
	case 0x2f6: // chemsuit
		soundno = RANDOM_ELEM(CHEMSUIT_SUR_SNDS);
		break;
	case 0x344: // chemsuit
		soundno = RANDOM_ELEM(SCIENTIST_SUR_SNDS);
		break;
	case 0x597: // female office worker
		soundno = RANDOM_ELEM(FEMALE_SUR_SNDS);
		break;
	}

	AudioProcess *audio = AudioProcess::get_instance();
	if (audio && soundno != -1) {
		audio->playSFX(soundno, 0x80, _itemNum, 1);
		_playedSound = true;
	}
}

void SurrenderProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	ws->writeByte(_playedSound ? 1 : 0);
}

bool SurrenderProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;
	_playedSound = rs->readByte() != 0;
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
