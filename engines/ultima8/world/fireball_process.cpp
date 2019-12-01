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

#include "ultima8/misc/pent_include.h"

#include "ultima8/world/fireball_process.h"
#include "ultima8/world/Item.h"
#include "ultima8/world/current_map.h"
#include "ultima8/world/actors/main_actor.h"
#include "ultima8/kernel/kernel.h"
#include "ultima8/world/item_factory.h"
#include "ultima8/misc/direction.h"
#include "ultima8/world/weapon_info.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(FireballProcess, Process);

FireballProcess::FireballProcess()
	: Process() {

}

FireballProcess::FireballProcess(Item *item, Item *target_)
	: xspeed(0), yspeed(0), age(0) {
	assert(item);
	assert(target_);

	tail[0] = 0;
	tail[1] = 0;
	tail[2] = 0;

	item_num = item->getObjId();

	target = target_->getObjId();

	type = 0x218; // CONSTANT!
}

void FireballProcess::run() {
	age++;

	Item *item = getItem(item_num);
	if (!item) {
		terminate();
		return;
	}

	Item *t = getItem(target);
	if (!t) {
		terminate();
		return;
	}

	if (age > 300 && (std::rand() % 20 == 0)) {
		// chance of 5% to disappear every frame after 10 seconds
		terminate();
		return;
	}

	// * accelerate a bit towards target
	// * try to move
	// * if succesful:
	//   * move
	//   * shift tail, enlarging if smaller than 3 flames
	// * if failed
	//   * deal damage if hit Actor
	//   * turn around if hit non-Actor

	int32 x, y, z;
	int32 tx, ty, tz;
	int32 dx, dy;
	item->getLocation(x, y, z);
	t->getLocationAbsolute(tx, ty, tz);

	dx = tx - x;
	dy = ty - y;

	int targetdir = item->getDirToItemCentre(*t);

	if (xspeed == 0 && yspeed == 0 && dx / 64 == 0 && dy / 64 == 0) {
		xspeed += 2 * x_fact[targetdir];
		yspeed += 2 * y_fact[targetdir];
	} else {
		xspeed += (dx / 64);
		yspeed += (dy / 64);
	}

	// limit speed
	int speed = static_cast<int>(sqrt(static_cast<float>(xspeed * xspeed + yspeed * yspeed)));
	if (speed > 32) {
		xspeed = (xspeed * 32) / speed;
		yspeed = (yspeed * 32) / speed;
	}

	ObjId hititem = 0;
	item->collideMove(x + xspeed, y + yspeed, z, false, false, &hititem);

	// handle tail
	// tail is shape 261, frame 0-7 (0 = to top-right, 7 = to top)
	if (tail[2] == 0) {
		// enlarge tail
		Item *newtail = ItemFactory::createItem(261, 0, 0,
		                                        Item::FLG_DISPOSABLE, 0, 0,
		                                        Item::EXT_SPRITE, true);
		tail[2] = newtail->getObjId();
	}

	Item *tailitem = getItem(tail[2]);
	tailitem->setFrame(Get_WorldDirection(yspeed, xspeed));
	tailitem->move(x, y, z);

	tail[2] = tail[1];
	tail[1] = tail[0];
	tail[0] = tailitem->getObjId();

	if (hititem) {
		Actor *hit = getActor(hititem);
		if (hit) {
			// hit an actor: deal damage and explode
			hit->receiveHit(0, 8 - targetdir, 5 + (std::rand() % 5),
			                WeaponInfo::DMG_FIRE);
			terminate();
			return;

		} else {
			// hit an object: invert direction

			xspeed = -xspeed;
			yspeed = -yspeed;
		}
	}
}

void FireballProcess::terminate() {
	// terminate first to prevent item->destroy() from terminating us again
	Process::terminate();

	explode();
}

void FireballProcess::explode() {
	Item *item = getItem(item_num);
	if (item) item->destroy();

	for (unsigned int i = 0; i < 3; ++i) {
		item = getItem(tail[i]);
		if (item) item->destroy();
	}
}

uint32 FireballProcess::I_TonysBalls(const uint8 *args,
                                     unsigned int /*argsize*/) {
	ARG_NULL16(); // unknown
	ARG_NULL16(); // unknown
	ARG_SINT16(x);
	ARG_SINT16(y);
	ARG_UINT16(z);

	Item *ball = ItemFactory::createItem(260, 4, 0, Item::FLG_FAST_ONLY,
	                                     0, 0, 0, true);
	if (!ball) {
		perr << "I_TonysBalls failed to create item (260, 4)." << std::endl;
		return 0;
	}
	if (!ball->canExistAt(x, y, z)) {
		perr << "I_TonysBalls: failed to create fireball." << std::endl;
		ball->destroy();
		return 0;
	}
	ball->move(x, y, z);

	MainActor *avatar = getMainActor();

	FireballProcess *fbp = new FireballProcess(ball, avatar);
	Kernel::get_instance()->addProcess(fbp);

	return 0;
}

void FireballProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(xspeed));
	ods->write4(static_cast<uint32>(yspeed));
	ods->write2(target);
	ods->write2(tail[0]);
	ods->write2(tail[1]);
	ods->write2(tail[2]);
	ods->write2(age);
}

bool FireballProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	xspeed = static_cast<int>(ids->read4());
	yspeed = static_cast<int>(ids->read4());
	target = ids->read2();
	tail[0] = ids->read2();
	tail[1] = ids->read2();
	tail[2] = ids->read2();
	age = ids->read2();

	return true;
}
