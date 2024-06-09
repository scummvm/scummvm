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

#include "ultima/ultima8/world/super_sprite_process.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"
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
			Actor *srcnpc = dynamic_cast<Actor *>(srcitem);
			Actor *controlled = getControlledActor();
			const uint32 frameno = Kernel::get_instance()->getFrameNum();
			const uint32 timeoutfinish = controlled ? controlled->getAttackMoveTimeoutFinishFrame() : 0;
			if (!srcnpc || !srcnpc->getAttackAimFlag()) {
				if (!srcnpc || frameno < timeoutfinish) {
					if (!srcnpc && (controlled && controlled->isKneeling())) {
						rng = rng / 5;
					} else {
						const uint16 dodgefactor = controlled ? controlled->getAttackMoveDodgeFactor() : 2;
						if (!srcnpc) {
							rng = rng / (dodgefactor * 3);
						} else {
							rng = rng / dodgefactor;
						}
					}
				} else {
					rng = rng / 8;
				}
			} else {
				rng = rng / 2;
			}
		}

		if (rng > 0x50)
			rng = 0x50;

		Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
		int xoff = rs.getRandomNumberRngSigned(-rng, rng);
		int yoff = rs.getRandomNumberRngSigned(-rng, rng);
		rng /= 3;
		if (rng > 0x18)
			rng = 0x18;
		int zoff = rs.getRandomNumberRngSigned(-rng, rng);

		_destpt.translate(xoff, yoff, zoff);
		if (_destpt.z > 0xfa)
			_destpt.z = 0xfa;
		else if (_destpt.z < 0)
			_destpt.z = 0;
	}

	float travel = _destpt.maxDistXYZ(_nextpt);
	float speed = firetypedat->getCellsPerRound() * 32.0f;
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

static inline int _sign(int x) {
	if (x < 0)
		return -1;
	else if (x == 0)
		return 0;
	return 1;
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
		if (_counter < firetypedat->getRoundDuration()) {
			if (!_expired) {
				Direction dir8 = dir_current;
				if (_target == 0) {
					targetz = _nowpt.z;
				} else {
					const Item *target = getItem(_target);
					if (target) {
						Point3 ptt = target->getLocation();
						Point3 ptc = target->getCentre();
						targetz = ptc.z + 8;
						dir8 = Direction_GetWorldDir(ptt.y - _nowpt.y, ptt.x - _nowpt.x, dirmode_8dirs);
					}
				}

				int xoff = Direction_XFactor(dir8);
				if (_sign(xoff) != _sign(_xstep))
					_xstep *= 2;
				int yoff = Direction_YFactor(dir8);
				if (_sign(yoff) != _sign(_ystep))
					_ystep *= 2;
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
	if (_pt3.z < 0)
		_pt3.z = 0;
	if (_pt3.z > 0xfa)
		_pt3.z = 0xfa;

	_counter++;

	if (!_spriteNo && _counter > 1) {
		Item *sprite = ItemFactory::createItem(_shape, _frame, 0, Item::FLG_DISPOSABLE,
			0, 0, Item::EXT_SPRITE, true);
		_spriteNo = sprite->getObjId();
		sprite->move(_nowpt);
	}

	if (_pt3.z > 0 && _pt3.z < 0xfa) {
		int32 duration = firetypedat->getRoundDuration() + 25;

		if (_counter < duration) {
			// disasm ~line 305
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
		firetypedat->applySplashDamageAround(pt, _damage, 1, item, src);
	}
	firetypedat->makeBulletSplashShapeAndPlaySound(pt.x, pt.y, pt.z);
}

static bool _pointOutOfMap(const Point3 &pt, int32 maxxy) {
	return (pt.x < 0     || pt.y < 0     || pt.z < 0 ||
			pt.x > maxxy || pt.y > maxxy || pt.z > 255);
}

void SuperSpriteProcess::hitAndFinish() {
	Point3 pt(_nowpt);
	//int dist = _nowpt.maxDistXYZ(_pt3);

	int xstep = _pt3.x - _nowpt.x;
	int ystep = _pt3.y - _nowpt.y;
	int zstep = _pt3.z - _nowpt.z;
	// We add a slight hack - our sweep test is off-by-one on Z??
	Point3 start(_nowpt.x, _nowpt.y, _nowpt.z + 1);
	Point3 end(_pt3.x, _pt3.y, _pt3.z + 1);
	int32 dims[3] = {1, 1, 1};
	// will never get a collision if not stepping at all..
	bool collision = !(xstep || ystep || zstep);
	Std::list<CurrentMap::SweepItem> hits;

	while (!collision) {
		CurrentMap *map = World::get_instance()->getCurrentMap();
		collision = map->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
								   _source, true, &hits);
		if (collision)
			break;
		start.x += xstep;
		start.y += ystep;
		start.z += zstep;
		end.x += xstep;
		end.y += ystep;
		end.z += zstep;
		const int32 mapmax = map->getChunkSize() * MAP_NUM_CHUNKS;
		if (_pointOutOfMap(start, mapmax) || _pointOutOfMap(end, mapmax))
			break;
	}

	if (collision && hits.size()) {
		const CurrentMap::SweepItem &firsthit = hits.front();
		_item0x77 = firsthit._item;
		pt = firsthit.GetInterpolatedCoords(start, end);
	}

	Item *item = getItem(_item0x77);
	if (item) {
		int32 ifx, ify, ifz;
		item->getFootpadData(ifx, ify, ifz);
		Point3 pti = item->getLocation();

		if (ifx > 2 && ify > 2 && ifz > 2) {
			int32 ixsize = (ifx - 2) * 16;
			int32 iysize = (ify - 2) * 16;
			if (pt.x < pti.x - ixsize)
				pt.x = pti.x - ixsize;
			if (pt.y < pti.y - iysize)
				pt.y = pti.y - iysize;
		}

		//Actor *actor = dynamic_cast<Actor *>(item);
		// CHECKME: This is not exactly the same as the game, but
		// it should work? See disasm 1138:1384, lines 142 ~ 172
		// There is some random factor added for non-actor items
		// which needs checking
		Direction dir = Direction_GetWorldDir(pti.y - _nowpt.y, pti.x - _nowpt.x, dirmode_8dirs);
		item->receiveHit(_itemNum, dir, _damage, _fireType);
	}
	makeBulletSplash(pt);
	destroyItemOrTerminate();
}

void SuperSpriteProcess::terminate() {
	if (_spriteNo) {
		Item *sprite = getItem(_spriteNo);
		if (sprite)
			sprite->destroy();
		_spriteNo = 0;
	}
	Process::terminate();
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

	Item *item = getItem(_itemNum);
	if (item) {
		item->collideMove(_pt3.x, _pt3.y, _pt3.z, false, false);
	}

	if (_spriteNo) {
		Item *sprite = getItem(_spriteNo);
		assert(sprite);
		sprite->move(_nextpt);
		uint32 frame = sprite->getFrame() + 1;
		if (_fireType == 0xe) {
			if (frame > 0x4b)
				frame = 0x47;
			sprite->setFrame(frame);
		} else if (_fireType == 0x11) { // No Regret only
			if (frame % 6 == 0)
				frame = frame - 5;
			sprite->setFrame(frame);
		} else if (_fireType == 0x14) { // No Regret only
			if ((frame - 0xdb) % 3 == 0)
				frame = frame - 2;
			sprite->setFrame(frame);
		}
	}

	if (_fireType == 3) {
		if (_pt5.x != -1) {
			// Create a little sparkly thing
			Process *p = new SpriteProcess(0x426, 0, 9, 1, 3, _pt5.x, _pt5.y, _pt5.z);
			Kernel::get_instance()->addProcess(p);
		}
		_pt5 = _nextpt;
	}
}

bool SuperSpriteProcess::areaSearch() {
	CurrentMap *map = World::get_instance()->getCurrentMap();

	Point3 start(_nowpt.x, _nowpt.y, _nowpt.z + 1);
	Point3 end(_pt3.x, _pt3.y, _pt3.z + 1);
	int32 dims[3] = {1, 1, 1};

	Item *item = getItem(_itemNum);
	if (item) {
		start = item->getLocation();
	}

	Std::list<CurrentMap::SweepItem> hits;
	map->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
							   _source, true, &hits);

	if (hits.size() > 0) {
		_item0x77 = hits.front()._item;
	}

	return hits.size() == 0;
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
	ws->writeUint16LE(_frame);
	ws->writeUint16LE(_fireType);
	ws->writeUint16LE(_damage);
	ws->writeUint16LE(_source);
	ws->writeUint16LE(_target);
	ws->writeUint16LE(_counter);
	ws->writeUint16LE(_item0x77);
	ws->writeUint16LE(_spriteNo);
	ws->writeFloatLE(_xstep);
	ws->writeFloatLE(_ystep);
	ws->writeFloatLE(_zstep);
	ws->writeByte(_startedAsFiretype9);
	ws->writeByte(_expired);
}

bool SuperSpriteProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_shape = static_cast<int>(rs->readUint32LE());
	_frame = rs->readUint32LE();
	_nowpt.loadData(rs, version);
	_nextpt.loadData(rs, version);
	_pt3.loadData(rs, version);
	_startpt.loadData(rs, version);
	_pt5.loadData(rs, version);
	_destpt.loadData(rs, version);
	_frame = rs->readUint16LE();
	_fireType = rs->readUint16LE();
	_damage = rs->readUint16LE();
	_source = rs->readUint16LE();
	_target = rs->readUint16LE();
	_counter = rs->readUint16LE();
	_item0x77 = rs->readUint16LE();
	_spriteNo = rs->readUint16LE();
	_xstep = rs->readFloatLE();
	_ystep = rs->readFloatLE();
	_zstep = rs->readFloatLE();
	_startedAsFiretype9 = (rs->readByte() != 0);
	_expired = (rs->readByte() != 0);

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
