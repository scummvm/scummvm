#include "colony/colony.h"
#include "common/debug.h"

namespace Colony {

enum {
	kOpcodeLRotate = 1,
	kOpcodeRRotate = 2,
	kOpcodeForward = 3,
	kOpcodeFShoot = 4,
	kOpcodeSnoop = 20
};

static bool isBaseRobotType(int type) {
	return type >= kRobEye && type <= kRobUPyramid;
}

static bool isEggType(int type) {
	return type > kRobUPyramid && type < kRobQueen;
}

static uint8 trailCodeForDelta(int dx, int dy) {
	if (dx > 1)
		dx = 1;
	else if (dx < -1)
		dx = -1;

	if (dy > 1)
		dy = 1;
	else if (dy < -1)
		dy = -1;

	if (dx > 0 && dy > 0)
		return 1;
	if (dx > 0 && dy == 0)
		return 2;
	if (dx > 0 && dy < 0)
		return 3;
	if (dx == 0 && dy < 0)
		return 4;
	if (dx < 0 && dy < 0)
		return 5;
	if (dx < 0 && dy == 0)
		return 6;
	if (dx < 0 && dy > 0)
		return 7;
	if (dx == 0 && dy > 0)
		return 8;

	return 0;
}

static int trailTargetAngle(uint8 code) {
	switch (code) {
	case 2:
		return 0;
	case 1:
		return 32;
	case 8:
		return 64;
	case 7:
		return 96;
	case 6:
		return 128;
	case 5:
		return 160;
	case 4:
		return 192;
	case 3:
		return 224;
	default:
		return -1;
	}
}

void ColonyEngine::notePlayerTrail(int oldX, int oldY, int newX, int newY) {
	if (oldX < 0 || oldX >= 32 || oldY < 0 || oldY >= 32)
		return;
	if (newX < 0 || newX >= 32 || newY < 0 || newY >= 32)
		return;

	const uint8 code = trailCodeForDelta(newX - oldX, newY - oldY);
	if (code != 0)
		_dirXY[oldX][oldY] = code;
}

void ColonyEngine::respawnObject(int num, int type) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	Thing &obj = _objects[num - 1];
	if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
	    obj.where.yindex >= 0 && obj.where.yindex < 32) {
		if (isEggType(obj.type)) {
			if (_foodArray[obj.where.xindex][obj.where.yindex] == num)
				_foodArray[obj.where.xindex][obj.where.yindex] = 0;
		} else if (_robotArray[obj.where.xindex][obj.where.yindex] == num) {
			_robotArray[obj.where.xindex][obj.where.yindex] = 0;
		}
	}

	int xloc, yloc;
	do {
		if (_level == 7 && type == kRobQueen) {
			xloc = 27;
			yloc = 10;
		} else {
			xloc = 2 + _randomSource.getRandomNumber(26);
			yloc = 2 + _randomSource.getRandomNumber(26);
		}
	} while (_robotArray[xloc][yloc] || _foodArray[xloc][yloc]);

	resetObjectSlot(num - 1, type, (xloc << 8) + 128, (yloc << 8) + 128, _randomSource.getRandomNumber(255));
}

void ColonyEngine::cThink() {
	if (_gameMode != kModeColony)
		return;

	const int objectCount = (int)_objects.size();
	for (int num = 1; num <= objectCount; ++num) {
		if (num > (int)_objects.size())
			break;

		const Thing &obj = _objects[num - 1];
		if (!obj.alive || obj.type <= 0 || obj.type > kBaseObject)
			continue;

		switch (obj.type) {
		case kRobEye:
			eyeThink(num);
			break;
		case kRobPyramid:
			pyramidThink(num);
			break;
		case kRobCube:
			cubeThink(num);
			break;
		case kRobUPyramid:
			upyramidThink(num);
			break;
		case kRobQueen:
			queenThink(num);
			break;
		case kRobDrone:
		case kRobSoldier:
			droneThink(num);
			break;
		case kRobSnoop:
			snoopThink(num);
			break;
		default:
			eggThink(num);
			break;
		}
	}
}

void ColonyEngine::cubeThink(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	_objects[num - 1].where.look = (uint8)(_objects[num - 1].where.look + 15);
	moveThink(num);
	if (num > (int)_objects.size())
		return;

	Thing *obj = &_objects[num - 1];
	obj->time--;
	const bool laidEgg = (obj->time < 0) && layEgg(kRobMCube, obj->where.xindex, obj->where.yindex);
	obj = &_objects[num - 1];
	if (laidEgg)
		obj->time = 10 + (_randomSource.getRandomNumber(255) & 0xFF);
	if (obj->grow)
		bigGrow(num);
}

void ColonyEngine::pyramidThink(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	_objects[num - 1].where.look = (uint8)(_objects[num - 1].where.look + 15);
	moveThink(num);
	if (num > (int)_objects.size())
		return;

	Thing *obj = &_objects[num - 1];
	obj->time--;
	const bool laidEgg = (obj->time < 0) && layEgg(kRobMPyramid, obj->where.xindex, obj->where.yindex);
	obj = &_objects[num - 1];
	if (laidEgg)
		obj->time = 10 + (_randomSource.getRandomNumber(255) & 0xFF);
	if (obj->grow)
		bigGrow(num);
}

void ColonyEngine::upyramidThink(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	_objects[num - 1].where.look = (uint8)(_objects[num - 1].where.look - 15);
	moveThink(num);
	if (num > (int)_objects.size())
		return;

	Thing *obj = &_objects[num - 1];
	obj->time--;
	const bool laidEgg = (obj->time < 0) && layEgg(kRobMUPyramid, obj->where.xindex, obj->where.yindex);
	obj = &_objects[num - 1];
	if (laidEgg)
		obj->time = 10 + (_randomSource.getRandomNumber(255) & 0xFF);
	if (obj->grow)
		bigGrow(num);
}

void ColonyEngine::eyeThink(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	moveThink(num);
	if (num > (int)_objects.size())
		return;

	Thing *obj = &_objects[num - 1];
	obj->time--;
	const bool laidEgg = (obj->time < 0) && layEgg(kRobMEye, obj->where.xindex, obj->where.yindex);
	obj = &_objects[num - 1];
	if (laidEgg)
		obj->time = 10 + (_randomSource.getRandomNumber(255) & 0xFF);
	if (obj->grow)
		bigGrow(num);
}

void ColonyEngine::queenThink(int num) {
	Thing &obj = _objects[num - 1];
	moveThink(num);
	obj.time--;
	obj.where.lookx += obj.where.delta;
	if (obj.where.lookx < -24 || obj.where.lookx > 24)
		obj.where.delta = -obj.where.delta;
	obj.where.look = (uint8)(obj.where.ang + obj.where.lookx);

	if (obj.time < 0) {
		const int eggX = obj.where.xindex;
		const int eggY = obj.where.yindex;
		bool laidEgg = false;

		switch (_randomSource.getRandomNumber(3)) {
		case 0:
			laidEgg = layEgg(kRobMEye, eggX, eggY);
			break;
		case 1:
			laidEgg = layEgg(kRobMCube, eggX, eggY);
			break;
		case 2:
			laidEgg = layEgg(kRobMPyramid, eggX, eggY);
			break;
		case 3:
			laidEgg = layEgg(kRobMUPyramid, eggX, eggY);
			break;
		}

		if (laidEgg)
			_objects[num - 1].time = 10 + (_randomSource.getRandomNumber(255) & 0xFF);
	}

	Thing &updatedObj = _objects[num - 1];
	if (!updatedObj.grow)
		return;

	if (updatedObj.where.xindex >= 0 && updatedObj.where.xindex < 32 &&
	    updatedObj.where.yindex >= 0 && updatedObj.where.yindex < 32 &&
	    _robotArray[updatedObj.where.xindex][updatedObj.where.yindex] == num)
		_robotArray[updatedObj.where.xindex][updatedObj.where.yindex] = 0;

	updatedObj.alive = 0;
	_allGrow = false;
	_sound->play(Sound::kExplode);

	for (uint i = 0; i < _objects.size(); ++i) {
		Thing &other = _objects[i];
		if (other.alive && isBaseRobotType(other.type))
			other.grow = -1;
	}

	if (_level >= 1 && _level <= 7)
		_levelData[_level - 1].queen = 0;
}

void ColonyEngine::droneThink(int num) {
	Thing &obj = _objects[num - 1];
	moveThink(num);
	obj.time--;
	obj.where.lookx += obj.where.delta;
	if (obj.where.lookx < -24 || obj.where.lookx > 24)
		obj.where.delta = -obj.where.delta;
	obj.where.look = (uint8)(obj.where.ang + obj.where.lookx);

	if (!obj.grow)
		return;

	if (obj.type == kRobDrone) {
		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32 &&
		    _robotArray[obj.where.xindex][obj.where.yindex] == num)
			_robotArray[obj.where.xindex][obj.where.yindex] = 0;
		obj.alive = 0;
		_sound->play(Sound::kExplode);
	} else {
		obj.type = kRobDrone;
		obj.grow = 0;
		obj.where.power[1] = 10 + ((_randomSource.getRandomNumber(15) & 0x0F) << _level);
	}
}

void ColonyEngine::eggThink(int num) {
	Thing &obj = _objects[num - 1];
	if (!isEggType(obj.type))
		return;

	if (_allGrow) {
		obj.time--;
	} else if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
	           obj.where.yindex >= 0 && obj.where.yindex < 32 &&
	           _foodArray[obj.where.xindex][obj.where.yindex] == 0) {
		_foodArray[obj.where.xindex][obj.where.yindex] = (uint8)num;
		if (_robotArray[obj.where.xindex][obj.where.yindex] == num)
			_robotArray[obj.where.xindex][obj.where.yindex] = 0;
		obj.grow = -1;
	}

	if (obj.time <= 0)
		obj.grow = 1;
	if (obj.grow)
		growRobot(num);
}

bool ColonyEngine::layEgg(int type, int xindex, int yindex) {
	auto hasFood = [this](int x, int y) -> bool {
		if (x < 0 || x >= 32 || y < 0 || y >= 32)
			return false;
		return _foodArray[x][y] != 0;
	};

	if (hasFood(xindex, yindex) ||
	    hasFood(xindex + 1, yindex) ||
	    hasFood(xindex - 1, yindex) ||
	    hasFood(xindex, yindex + 1) ||
	    hasFood(xindex, yindex - 1))
		return false;

	createObject(type, (xindex << 8) + 128, (yindex << 8) + 128, _randomSource.getRandomNumber(255));
	return true;
}

void ColonyEngine::moveThink(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	Thing &obj = _objects[num - 1];
	if (!obj.alive)
		return;

	switch (obj.opcode) {
	case kOpcodeLRotate:
		obj.where.ang = (uint8)(obj.where.ang + 7);
		obj.where.look = (uint8)(obj.where.look + 7);
		obj.counter--;
		if (obj.counter <= 0) {
			obj.opcode = kOpcodeForward;
			obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
		}
		if (scanForPlayer(num) == kMeNum) {
			robotShoot(num);
			obj.opcode = kOpcodeFShoot;
			obj.counter = _randomSource.getRandomNumber(255) & 0x3F;
		}
		return;

	case kOpcodeRRotate:
		obj.where.ang = (uint8)(obj.where.ang - 7);
		obj.where.look = (uint8)(obj.where.look - 7);
		obj.counter--;
		if (obj.counter <= 0) {
			obj.opcode = kOpcodeForward;
			obj.counter = _randomSource.getRandomNumber(255) & 0x3F;
		}
		if (scanForPlayer(num) == kMeNum) {
			robotShoot(num);
			obj.opcode = kOpcodeFShoot;
			obj.counter = _randomSource.getRandomNumber(255) & 0x1F;
		}
		return;

	case kOpcodeForward: {
		obj.counter--;
		if (obj.counter <= 0) {
			obj.opcode = (_randomSource.getRandomNumber(1) ? kOpcodeLRotate : kOpcodeRRotate);
			obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
			return;
		}

		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32 &&
		    _robotArray[obj.where.xindex][obj.where.yindex] == num)
			_robotArray[obj.where.xindex][obj.where.yindex] = 0;

		_suppressCollisionSound = true;
		const int collide = checkwall(obj.where.xloc + (_cost[obj.where.ang] >> 2),
			obj.where.yloc + (_sint[obj.where.ang] >> 2), &obj.where);
		_suppressCollisionSound = false;

		if (collide) {
			obj.opcode = (_randomSource.getRandomNumber(1) ? kOpcodeLRotate : kOpcodeRRotate);
			obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
		}

		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32)
			_robotArray[obj.where.xindex][obj.where.yindex] = (uint8)num;
		return;
	}

	case kOpcodeFShoot: {
		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32 &&
		    _robotArray[obj.where.xindex][obj.where.yindex] == num)
			_robotArray[obj.where.xindex][obj.where.yindex] = 0;

		_suppressCollisionSound = true;
		const int collide = checkwall(obj.where.xloc + obj.where.dx + (_me.dx >> 2) + (_cost[obj.where.ang] >> 2),
			obj.where.yloc + obj.where.dy + (_me.dy >> 2) + (_sint[obj.where.ang] >> 2), &obj.where);
		_suppressCollisionSound = false;

		if (collide) {
			obj.opcode = (_randomSource.getRandomNumber(1) ? kOpcodeLRotate : kOpcodeRRotate);
			obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
		}

		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32)
			_robotArray[obj.where.xindex][obj.where.yindex] = (uint8)num;

		if (scanForPlayer(num) == kMeNum) {
			robotShoot(num);
		} else {
			obj.opcode = kOpcodeForward;
			obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
		}
		return;
	}

	default:
		obj.alive = 0;
		obj.opcode = kOpcodeForward;
		obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
		break;
	}
}

void ColonyEngine::snoopThink(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	Thing &obj = _objects[num - 1];
	if (!obj.alive)
		return;

	switch (obj.opcode) {
	case kOpcodeLRotate:
		obj.where.ang = (uint8)(obj.where.ang + 7);
		obj.where.look = obj.where.ang;
		obj.counter--;
		if (obj.counter <= 0) {
			obj.opcode = kOpcodeForward;
			obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
		}
		break;

	case kOpcodeRRotate:
		obj.where.ang = (uint8)(obj.where.ang - 7);
		obj.where.look = obj.where.ang;
		obj.counter--;
		if (obj.counter <= 0) {
			obj.opcode = kOpcodeForward;
			obj.counter = _randomSource.getRandomNumber(255) & 0x3F;
		}
		break;

	case kOpcodeForward: {
		obj.counter--;
		if (obj.counter <= 0) {
			obj.opcode = (_randomSource.getRandomNumber(1) ? kOpcodeLRotate : kOpcodeRRotate);
			obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
			break;
		}

		const int oldX = obj.where.xindex;
		const int oldY = obj.where.yindex;
		if (oldX >= 0 && oldX < 32 && oldY >= 0 && oldY < 32 && _robotArray[oldX][oldY] == num)
			_robotArray[oldX][oldY] = 0;

		const int fx = obj.where.xloc + (_cost[obj.where.ang] >> 2);
		const int fy = obj.where.yloc + (_sint[obj.where.ang] >> 2);
		_suppressCollisionSound = true;
		const int collide = checkwall(fx, fy, &obj.where);
		_suppressCollisionSound = false;

		if (collide == kMeNum) {
			respawnObject(num, kRobSnoop);
			_sound->play(Sound::kSlug);
			setPower(-(int)MIN<int32>((3 * _me.power[0]) / 4, 32000),
				-(int)MIN<int32>((3 * _me.power[1]) / 4, 32000),
				-(int)MIN<int32>((3 * _me.power[2]) / 4, 32000));
			return;
		}

		if (collide > 0 && collide <= (int)_objects.size()) {
			const Thing &other = _objects[collide - 1];
			if (other.alive && other.type >= kRobEye && other.type <= kBaseObject) {
				respawnObject(collide, other.type);
				obj.where.xloc = fx;
				obj.where.yloc = fy;
				obj.where.xindex = fx >> 8;
				obj.where.yindex = fy >> 8;
			} else {
				obj.opcode = (_randomSource.getRandomNumber(1) ? kOpcodeLRotate : kOpcodeRRotate);
				obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
			}
		} else if (collide) {
			obj.opcode = (_randomSource.getRandomNumber(1) ? kOpcodeLRotate : kOpcodeRRotate);
			obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
		}

		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32)
			_robotArray[obj.where.xindex][obj.where.yindex] = (uint8)num;

		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32 &&
		    _dirXY[obj.where.xindex][obj.where.yindex]) {
			obj.opcode = kOpcodeSnoop;
			obj.where.ang &= 0xF8;
			obj.where.look = obj.where.ang;
		}
		break;
	}

	case kOpcodeSnoop: {
		const int target = trailTargetAngle(_dirXY[obj.where.xindex][obj.where.yindex]);
		if (target >= 0 && obj.where.ang != target) {
			const int diff = (target - obj.where.ang) & 0xFF;
			if (diff < 128)
				obj.where.ang = (uint8)(obj.where.ang + 8);
			else
				obj.where.ang = (uint8)(obj.where.ang - 8);
			obj.where.look = obj.where.ang;
			return;
		}

		obj.opcode = kOpcodeForward;
		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32)
			_dirXY[obj.where.xindex][obj.where.yindex] = 0;
		obj.counter = 0x3F;
		break;
	}

	default:
		obj.alive = 0;
		obj.opcode = kOpcodeForward;
		obj.counter = _randomSource.getRandomNumber(255) & 0x0F;
		break;
	}
}

void ColonyEngine::bigGrow(int num) {
	Thing &obj = _objects[num - 1];
	const int xindex = obj.where.xindex;
	const int yindex = obj.where.yindex;

	obj.count++;
	if (xindex < 0 || xindex >= 32 || yindex < 0 || yindex >= 32)
		return;
	if (_foodArray[xindex][yindex] != 0)
		return;

	if (obj.where.xloc == (xindex << 8) + 128 && obj.where.yloc == (yindex << 8) + 128) {
		_foodArray[xindex][yindex] = (uint8)num;
		if (_robotArray[xindex][yindex] == num)
			_robotArray[xindex][yindex] = 0;
		obj.type += 4;
		obj.count = 0;
		obj.time = 10 + (_randomSource.getRandomNumber(63) & 0x3F);
	} else {
		obj.where.xloc = (xindex << 8) + 128;
		obj.where.yloc = (yindex << 8) + 128;
		obj.opcode = kOpcodeLRotate;
		obj.counter = 100;
		obj.count = 0;
	}
}

void ColonyEngine::growRobot(int num) {
	Thing &obj = _objects[num - 1];
	obj.count++;
	obj.time = 10 + (_randomSource.getRandomNumber(63) & 0x3F);

	switch (obj.type) {
	case kRobFEye:
	case kRobFPyramid:
	case kRobFCube:
	case kRobFUPyramid:
		if (obj.count > 3) {
			obj.count = 0;
			if (obj.grow == 1) {
				obj.grow = 0;
				obj.type -= 4;
			} else {
				obj.type += 4;
			}
		}
		break;

	case kRobSEye:
	case kRobSPyramid:
	case kRobSCube:
	case kRobSUPyramid:
		if (obj.count > 3) {
			obj.count = 0;
			if (obj.grow == 1) {
				obj.type -= 4;
			} else {
				obj.grow = 0;
				obj.type += 4;
			}
		}
		break;

	case kRobMEye:
	case kRobMPyramid:
	case kRobMCube:
	case kRobMUPyramid:
		if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
		    obj.where.yindex >= 0 && obj.where.yindex < 32 &&
		    _robotArray[obj.where.xindex][obj.where.yindex] == 0) {
			obj.count = 0;
			obj.type -= 4;
			_robotArray[obj.where.xindex][obj.where.yindex] = (uint8)num;
			if (_foodArray[obj.where.xindex][obj.where.yindex] == num)
				_foodArray[obj.where.xindex][obj.where.yindex] = 0;
		}
		break;

	default:
		break;
	}
}

int ColonyEngine::scanForPlayer(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return 0;

	Thing &obj = _objects[num - 1];
	Locate fire;
	memset(&fire, 0, sizeof(fire));
	fire.ang = obj.where.ang;
	fire.type = 2;

	int fireX = obj.where.xloc;
	int fireY = obj.where.yloc;
	int collide = 0;

	do {
		fire.xloc = fireX;
		fire.yloc = fireY;
		fire.xindex = fireX >> 8;
		fire.yindex = fireY >> 8;
		fireX += _cost[fire.ang] * 2;
		fireY += _sint[fire.ang] * 2;
		_suppressCollisionSound = true;
		collide = checkwall(fireX, fireY, &fire);
		_suppressCollisionSound = false;
	} while (!collide);

	if (collide == kMeNum) {
		obj.where.dx = (_me.xloc - fireX) >> 1;
		obj.where.dy = (_me.yloc - fireY) >> 1;
	}

	return collide;
}

void ColonyEngine::robotShoot(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	Thing &obj = _objects[num - 1];
	if (!obj.alive)
		return;

	auto qlog = [](int32 x) -> int {
		int i = 0;
		while (x > 0) {
			x >>= 1;
			i++;
		}
		return i;
	};

	const int epower2 = qlog(_me.power[2]);
	const int armor2 = _armor * _armor;
	int damage = 0;

	obj.opcode = kOpcodeFShoot;
	_sound->play(Sound::kShoot);

	switch (obj.type) {
	case kRobEye:
		damage = epower2 * armor2 - (1 << _level);
		if (damage > -_level)
			damage = -_level;
		setPower(damage, damage, damage);
		break;
	case kRobPyramid:
		damage = epower2 * armor2 - (1 << _level);
		if (damage > -_level)
			damage = -_level;
		setPower(damage, -_level, -_level);
		break;
	case kRobCube:
		damage = epower2 * armor2 - (1 << _level);
		if (damage > -_level)
			damage = -_level;
		setPower(-_level, damage, -_level);
		break;
	case kRobUPyramid:
		damage = epower2 * armor2 - (1 << _level);
		if (damage > -_level)
			damage = -_level;
		setPower(-_level, -_level, damage);
		break;
	case kRobQueen:
		damage = epower2 * armor2 - ((_level == 1 || _level == 7) ? (3 << 10) : (3 << _level));
		if (damage > -_level)
			damage = -(1 << _level);
		setPower(damage, damage, damage);
		break;
	case kRobDrone:
		damage = epower2 * armor2 - (4 << _level);
		if (damage > -_level)
			damage = -(_level << 1);
		setPower(damage, damage, damage);
		break;
	case kRobSoldier:
		damage = epower2 * armor2 - (5 << _level);
		if (damage > -_level)
			damage = -(_level << 1);
		setPower(damage, damage, damage);
		break;
	default:
		break;
	}
}

void ColonyEngine::meEat() {
	if (_me.xindex < 0 || _me.xindex >= 32 || _me.yindex < 0 || _me.yindex >= 32)
		return;

	const int foodNum = _foodArray[_me.xindex][_me.yindex];
	if (foodNum <= 0 || foodNum > (int)_objects.size())
		return;

	Thing &obj = _objects[foodNum - 1];
	_foodArray[_me.xindex][_me.yindex] = 0;
	obj.alive = 0;
	_sound->play(Sound::kEat);

	switch (obj.type) {
	case kRobMUPyramid:
	case kRobFUPyramid:
	case kRobSUPyramid:
		setPower(7 << _level, 7 << _level, 15 << _level);
		break;
	case kRobMEye:
	case kRobFEye:
	case kRobSEye:
		setPower(15 << _level, 15 << _level, 15 << _level);
		break;
	case kRobMPyramid:
	case kRobFPyramid:
	case kRobSPyramid:
		setPower(15 << _level, 7 << _level, 7 << _level);
		break;
	case kRobMCube:
	case kRobFCube:
	case kRobSCube:
		setPower(7 << _level, 15 << _level, 7 << _level);
		break;
	default:
		break;
	}
}

} // End of namespace Colony
