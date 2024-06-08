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

#include "ultima/ultima8/world/fireball_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/world/get_object.h"


namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(FireballProcess)

FireballProcess::FireballProcess()
	: Process(), _xSpeed(0), _ySpeed(0), _age(0), _target(0) {

}

FireballProcess::FireballProcess(Item *item, Item *target)
	: _xSpeed(0), _ySpeed(0), _age(0) {
	assert(item);
	assert(target);

	_tail[0] = 0;
	_tail[1] = 0;
	_tail[2] = 0;

	_itemNum = item->getObjId();

	_target = target->getObjId();

	_type = 0x218; // CONSTANT!
}

void FireballProcess::run() {
	_age++;

	Item *item = getItem(_itemNum);
	if (!item) {
		terminate();
		return;
	}

	Item *t = getItem(_target);
	if (!t) {
		terminate();
		return;
	}

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	if (_age > 300 && rs.getRandomNumber(19) == 0) {
		// chance of 5% to disappear every frame after 10 seconds
		terminate();
		return;
	}

	// * accelerate a bit towards _target
	// * try to move
	// * if successful:
	//   * move
	//   * shift _tail, enlarging if smaller than 3 flames
	// * if failed
	//   * deal damage if hit Actor
	//   * turn around if hit non-Actor

	int32 dx, dy;
	Point3 pt1 = item->getLocation();
	Point3 pt2 = t->getLocationAbsolute();

	dx = pt2.x - pt1.x;
	dy = pt2.y - pt1.y;

	Direction targetdir = item->getDirToItemCentre(*t);

	if (_xSpeed == 0 && _ySpeed == 0 && dx / 64 == 0 && dy / 64 == 0) {
		_xSpeed += 2 * Direction_XFactor(targetdir);
		_ySpeed += 2 * Direction_YFactor(targetdir);
	} else {
		_xSpeed += (dx / 64);
		_ySpeed += (dy / 64);
	}

	// limit speed
	int speed = static_cast<int>(sqrt(static_cast<float>(_xSpeed * _xSpeed + _ySpeed * _ySpeed)));
	if (speed > 32) {
		_xSpeed = (_xSpeed * 32) / speed;
		_ySpeed = (_ySpeed * 32) / speed;
	}

	ObjId hititem = 0;
	item->collideMove(pt1.x + _xSpeed, pt1.y + _ySpeed, pt1.z, false, false, &hititem);

	// handle _tail
	// _tail is shape 261, frame 0-7 (0 = to top-right, 7 = to top)
	if (_tail[2] == 0) {
		// enlarge _tail
		Item *newtail = ItemFactory::createItem(261, 0, 0,
		                                        Item::FLG_DISPOSABLE, 0, 0,
		                                        Item::EXT_SPRITE, true);
		_tail[2] = newtail->getObjId();
	}

	Item *tailitem = getItem(_tail[2]);
	Direction movedir = Direction_GetWorldDir(_ySpeed, _xSpeed, dirmode_8dirs);
	tailitem->setFrame(Direction_ToUsecodeDir(movedir));
	tailitem->move(pt1);

	_tail[2] = _tail[1];
	_tail[1] = _tail[0];
	_tail[0] = tailitem->getObjId();

	if (hititem) {
		Actor *hit = getActor(hititem);
		if (hit) {
			// hit an actor: deal damage and explode
			hit->receiveHit(0, Direction_Invert(targetdir), rs.getRandomNumberRng(5, 9),
			                WeaponInfo::DMG_FIRE);
			terminate();
			return;

		} else {
			// hit an object: invert direction

			_xSpeed = -_xSpeed;
			_ySpeed = -_ySpeed;
		}
	}
}

void FireballProcess::terminate() {
	// terminate first to prevent item->destroy() from terminating us again
	Process::terminate();

	explode();
}

void FireballProcess::explode() {
	Item *item = getItem(_itemNum);
	if (item) item->destroy();

	for (unsigned int i = 0; i < 3; ++i) {
		item = getItem(_tail[i]);
		if (item) item->destroy();
	}
}

uint32 FireballProcess::I_TonysBalls(const uint8 *args,
									 unsigned int /*argsize*/) {
	ARG_NULL16(); // unknown
	ARG_NULL16(); // unknown
	ARG_SINT16(x);
	ARG_SINT16(y);
	ARG_UINT8(z);

	Item *ball = ItemFactory::createItem(260, 4, 0, Item::FLG_FAST_ONLY,
	                                     0, 0, 0, true);
	if (!ball) {
		warning("I_TonysBalls failed to create item (260, 4).");
		return 0;
	}

	Point3 pt(x, y, z);
	if (!ball->canExistAt(pt)) {
		warning("I_TonysBalls: failed to create fireball.");
		ball->destroy();
		return 0;
	}
	ball->move(pt);

	MainActor *avatar = getMainActor();

	FireballProcess *fbp = new FireballProcess(ball, avatar);
	Kernel::get_instance()->addProcess(fbp);

	return 0;
}

void FireballProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_xSpeed));
	ws->writeUint32LE(static_cast<uint32>(_ySpeed));
	ws->writeUint16LE(_target);
	ws->writeUint16LE(_tail[0]);
	ws->writeUint16LE(_tail[1]);
	ws->writeUint16LE(_tail[2]);
	ws->writeUint16LE(_age);
}

bool FireballProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_xSpeed = static_cast<int>(rs->readUint32LE());
	_ySpeed = static_cast<int>(rs->readUint32LE());
	_target = rs->readUint16LE();
	_tail[0] = rs->readUint16LE();
	_tail[1] = rs->readUint16LE();
	_tail[2] = rs->readUint16LE();
	_age = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
