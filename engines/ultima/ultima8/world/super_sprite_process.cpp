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
#include "ultima/ultima8/world/super_sprite_process.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/sprite_process.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(SuperSpriteProcess)

SuperSpriteProcess::SuperSpriteProcess() : Process(),
	_shape(0), _frame(0), _fireType(0), _damage(0), _source(0),
	_target(0), _counter(1), _item0x77(0), _spriteNo(0),
	_startedAsFiretype9(false), _xstep(0), _ystep(0), _zstep(0),
	_expired(false) {
}

SuperSpriteProcess::SuperSpriteProcess(int shape, int frame, int sx, int sy, int sz,
									   int dx, int dy, int dz,
									   uint16 firetype, uint16 damage, uint16 source,
									   uint16 target, bool inexact) :
		_shape(shape), _frame(frame), _startpt(sx, sy, sz), _destpt(dx, dy, dz),
		_nextpt(sx, sy, sz), _fireType(firetype), _damage(damage), _source(source),
		_target(target), _counter(1), _item0x77(0), _spriteNo(0),
		_startedAsFiretype9(firetype == 9), _xstep(0), _ystep(0), _zstep(0),
		_expired(false) {

	const FireType *firetypedat = GameData::get_instance()->getFireType(firetype);
	assert(firetypedat);
	if (firetypedat->getAccurate()) {
		inexact = false;
	}
	if (inexact) {
		int rng = _startpt.maxDistXYZ(_destpt);
		Item *srcitem = getItem(source);
		if (srcitem == getControlledActor()) {
			if (firetype == 2 || firetype == 13)
				rng /= 8;
			else if (firetype == 5)
				rng /= 12;
			else if (firetype == 10)
				rng /= 5;
			else
				rng /= 10;
		} else {
			if (dynamic_cast<Actor *>(srcitem) != nullptr) {
				rng /= 2;
			} else {
				// TODO: various other flags are checked in the game (around 1138:0bd1)
				// such as World_FinishedAvatarMoveTimeout() -> 8
				//  to make it either 5 or 8.  For now just use 5.
				rng /= 5;
			}
		}

		if (rng > 0x50)
			rng = 0x50;
		int xoff = -rng + (getRandom() % (rng * 2 + 1));
		int yoff = -rng + (getRandom() % (rng * 2 + 1));
		rng /= 3;
		if (rng > 0x18)
			rng = 0x18;
		int zoff = -rng + (getRandom() % (rng * 2 + 1));

		_destpt.move(xoff, yoff, zoff);
		if (_destpt.z > 0xfa)
			_destpt.z = 0xfa;
		else if (_destpt.z < 0)
			_destpt.z = 0;
	}

	float travel = _destpt.maxDistXYZ(_nextpt);
	// FIXME: how to get this scaled correctly?
	float speed = firetypedat->getCellsPerRound() * 128.0f;
	float rounds = travel / speed;
	if (rounds < 1)
		rounds = 1;

	_xstep = (_destpt.x - sx) / rounds;
	_ystep = (_destpt.y - sy) / rounds;
	_zstep = (_destpt.z - sz) / rounds;

	if (_fireType == 2 || _fireType == 0xd) {
		_destpt.x += travel / 5;
		_destpt.y += travel / 5;
		_destpt.z += travel / 5;
	}

}

SuperSpriteProcess::~SuperSpriteProcess(void) {
	Item *item = getItem(_itemNum);
	if (item) item->destroy();
}

void SuperSpriteProcess::move(int x, int y, int z) {
	_nowpt.set(x, y, z);

	Item *item = getItem(_itemNum);
	if (item)
		item->move(_nowpt);
}

void SuperSpriteProcess::run() {
	CurrentMap *map = World::get_instance()->getCurrentMap();
	int mapChunkSize = map->getChunkSize();
	const FireType *firetypedat = GameData::get_instance()->getFireType(_fireType);

	if (!firetypedat || !map->isChunkFast(_nextpt.x / mapChunkSize, _nextpt.y / mapChunkSize)) {
		destroyItemOrTerminate();
		return;
	}

	_nowpt = _nextpt;

	Point3 newpt(_startpt);
	if (!_startedAsFiretype9) {
		newpt.x += _counter * _xstep;
		newpt.y += _counter * _ystep;
		newpt.z += _counter * _zstep;
	} else {
		int targetz = 0;
		if (_counter > firetypedat->getRoundDuration()) {
			if (!_expired) {
				if (_target == 0) {
					targetz = _nowpt.z;
				} else {
					Item *target = getItem(_target);
					if (target) {
						int32 tx, ty, tz;
						int32 cx, cy, cz;
						target->getLocation(tx, ty, tz);
						target->getCentre(cx, cy, cz);
						targetz = cz + 8;
					}
				}

				// TODO: Apply point adjustments for firetype 9 here
				// (lines 134~214 of disasm)
			}
		} else {
			_expired = true;
		}

		if (!_expired) {
			if (_nowpt.z < targetz) {
				_zstep++;
			} else if (targetz < _nowpt.z) {
				_zstep--;
			}
		} else {
			_zstep--;
		}

		_xstep = CLIP(_xstep, -32.0f, 32.0f);
		_ystep = CLIP(_ystep, -32.0f, 32.0f);
		_zstep = CLIP(_zstep, -16.0f, 16.0f);

		newpt.x += _counter * _xstep;
		newpt.y += _counter * _ystep;
		newpt.z += _counter * _zstep;

		if (_fireType == 9 && !_expired) {
			if (_nowpt.x != newpt.x || _nowpt.y != newpt.y) {
				Direction dir = Direction_GetWorldDir(_nowpt.y - newpt.y, _nowpt.x - newpt.x, dirmode_8dirs);
				Item *item;
				if (_itemNum == 0) {
					item = getItem(_spriteNo);
				} else {
					item = getItem(_itemNum);
				}
				if (item) {
					item->setFrame(Direction_ToUsecodeDir(dir) + 0x11);
				}
			}
		}
	}
	_pt3 = newpt;

	_counter++;

	if (!_itemNum && _counter > 1) {
		Item *sprite = ItemFactory::createItem(_shape, _frame, 0, Item::FLG_DISPOSABLE,
			0, 0, Item::EXT_SPRITE, true);
		_spriteNo = sprite->getObjId();
		sprite->move(_nowpt);
	}

	if (_pt3.z != 0 && _pt3.z != 0xfa) {
		int32 duration = firetypedat->getRoundDuration();

		if (_counter >= duration) {
			// disasm ~line 311
			if (!map->isChunkFast(_nowpt.x / mapChunkSize, _nowpt.y / mapChunkSize)) {
				destroyItemOrTerminate();
				return;
			}
			if (areaSearch()) {
				advanceFrame();
				Process *delay = new DelayProcess(_fireType == 9 ? 3 : 2);
				ProcId delayid = Kernel::get_instance()->addProcess(delay);
				waitFor(delayid);
				return;
			}
			if (_item0x77 && _fireType == 5) {
				Item *item = getItem(_item0x77);
				assert(item);
				const ShapeInfo *info = item->getShapeInfo();
				if (info->is_roof() && _fireType == 5) {
					makeBulletSplash(_pt3);
					// Finish disasm lines 347~381 here.
					warning("TODO: SuperSpriteProcess::run: Implement special case for firetype 5 hitting a roof");
					terminate();
					return;
				}
			}
		}
	}
	if (_source && (_item0x77 == _source && _counter < 5)) {
		Item *source = getItem(_source);
		assert(source);
		// CHECKME: This appears to be a hack in the original game
		// to avoid hitting the source item.. for now reproduce it.
		source->moveToEtherealVoid();
		run();
		source->returnFromEtherealVoid();
	}
	hitAndFinish();
}

void SuperSpriteProcess::makeBulletSplash(const Point3 &pt) {
	const FireType *firetypedat = GameData::get_instance()->getFireType(_fireType);
	if (!firetypedat)
		return;

	if (firetypedat->getRange()) {
		Item *item = getItem(_item0x77);
		Item *src = getItem(_source);
		firetypedat->applySplashDamageAround(pt, _damage, item, src);
	}
	firetypedat->makeBulletSplashShapeAndPlaySound(pt.x, pt.y, pt.z);
}

void SuperSpriteProcess::hitAndFinish() {
	Point3 pt(_nowpt);
	//int dist = _nowpt.maxDistXYZ(_pt3);

	CurrentMap *map = World::get_instance()->getCurrentMap();
	Std::list<CurrentMap::SweepItem> hits;
	int32 start[3] = {_nowpt.x, _nowpt.y, _nowpt.z};
	int32 end[3] = {_pt3.x, _pt3.y, _pt3.z};
	int32 dims[3] = {1, 1, 1};
	bool collision = map->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
									_source, true, &hits);

	if (collision && hits.size()) {
		const CurrentMap::SweepItem &firsthit = hits.front();
		_item0x77 = firsthit._item;
		int32 hitpt[3] = {pt.x, pt.y, pt.z};
		firsthit.GetInterpolatedCoords(hitpt, start, end);
		pt = Point3(hitpt[0], hitpt[1], hitpt[2]);
	}

	Item *item = getItem(_item0x77);
	if (item) {
		int32 ifx, ify, ifz;
		item->getFootpadData(ifx, ify, ifz);
		int32 ix, iy, iz;
		item->getLocation(ix, iy, iz);

		if (ifx > 2 && ify > 2 && ifz > 2) {
			int32 ixsize = (ifx - 2) * 16;
			int32 iysize = (ifx - 2) * 16;
			if (pt.x < ix - ixsize)
				pt.x = ix - ixsize;
			if (pt.y < iy - iysize)
				pt.y = iy - iysize;
		}

		//Actor *actor = dynamic_cast<Actor *>(item);
		// CHECKME: This is not exactly the same as the game, but
		// it should work? See disasm 1138:1384, lines 142 ~ 172
		// There is some random factor added for non-actor items
		// which needs checking
		Direction dir = Direction_GetWorldDir(iy - _nowpt.y, ix - _nowpt.x, dirmode_8dirs);
		item->receiveHit(_itemNum, dir, _damage, _fireType);
	}
	makeBulletSplash(pt);
	destroyItemOrTerminate();
}

void SuperSpriteProcess::destroyItemOrTerminate() {
	if (_itemNum) {
		Item *item = getItem(_itemNum);
		if (item)
			item->destroy();
		_itemNum = 0;
	} else {
		terminate();
	}
}

// Implementation of fn at 1138:0f9e
void SuperSpriteProcess::advanceFrame() {
	_nextpt = _pt3;

	if (_itemNum) {
		warning("TODO: SuperSpriteProcess::advanceFrame: Implement area search 10e0_123a");
		// TODO: do an area search for something here.
		// AreaSearch_10e0_123a
	}

	if (_spriteNo) {
		Item *sprite = getItem(_spriteNo);
		assert(sprite);
		sprite->move(_nextpt);
		uint32 frame = sprite->getFrame();
		frame++;
		if (frame > 0x4b)
			frame = 0x47;
		sprite->setFrame(frame);
	}

	if (_fireType == 3) {
		if (_pt5.x != -1) {
			// Create a little sparkly thing
			Process *p = new SpriteProcess(0x426, 0, 9, 0, 3, _pt5.x, _pt5.y, _pt5.z);
			Kernel::get_instance()->addProcess(p);
		}
		_pt5 = _nextpt;
	}
}

bool SuperSpriteProcess::areaSearch() {
	// int x1, int y1, int z1, int x2, int y2, int z2
	CurrentMap *map = World::get_instance()->getCurrentMap();

	warning("TODO: SuperSpriteProcess::areaSearch: Implement area search 1138_0ee8");
	// TODO: Implement SuperSprite_AreaSearch_1138_0ee8
	uint16 lsrange = _pt3.maxDistXYZ(_nextpt);
	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	map->areaSearch(&uclist, script, sizeof(script), nullptr,
					lsrange, true, _nextpt.x, _nextpt.y);
	for (unsigned int i = 0; i < uclist.getSize(); ++i) {
		Item *searchitem = getItem(uclist.getuint16(i));
		assert(searchitem);
	}

	return false;
}


void SuperSpriteProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_shape));
	ws->writeUint32LE(_frame);
	_nowpt.saveData(ws);
	_nextpt.saveData(ws);
	_pt3.saveData(ws);
	_startpt.saveData(ws);
	_pt5.saveData(ws);
	_destpt.saveData(ws);
	ws->writeUint16LE(_fireType);
	ws->writeUint16LE(_damage);
	ws->writeUint16LE(_source);
	ws->writeUint16LE(_target);
	ws->writeUint16LE(_item0x77);
	ws->writeUint16LE(_spriteNo);
	ws->writeFloatLE(_xstep);
	ws->writeFloatLE(_ystep);
	ws->writeFloatLE(_zstep);

	// TODO: Update save and load functions once this is finished.
}

bool SuperSpriteProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_shape = static_cast<int>(rs->readUint32LE());
	_frame = rs->readUint16LE();
	_nowpt.loadData(rs, version);
	_nextpt.loadData(rs, version);
	_pt3.loadData(rs, version);
	_startpt.loadData(rs, version);
	_pt5.loadData(rs, version);
	_destpt.loadData(rs, version);
	_fireType = rs->readUint16LE();
	_damage = rs->readUint16LE();
	_source = rs->readUint16LE();
	_target = rs->readUint16LE();
	_item0x77 = rs->readUint16LE();
	_spriteNo = rs->readUint16LE();
	_xstep = rs->readFloatLE();
	_ystep = rs->readFloatLE();
	_zstep = rs->readFloatLE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
