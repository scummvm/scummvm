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

#include "ultima/ultima8/world/bobo_boomer_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(BoboBoomerProcess)

BoboBoomerProcess::BoboBoomerProcess() : Process(),
_counter(0), _x(0), _y(0), _z(0)
{}

BoboBoomerProcess::BoboBoomerProcess(const Item *item) : Process(), _counter(0)
{
	assert(item);
	Point3 pt = item->getLocation();
	_x = pt.x;
	_y = pt.y;
	_z = pt.z;
	_type = 0x264;
}

void BoboBoomerProcess::run() {
	const FireType *firetype = GameData::get_instance()->getFireType(4);
	assert(firetype);

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	int32 randx = rs.getRandomNumberRngSigned(-7, 7);
	int32 randy = rs.getRandomNumberRngSigned(-7, 7);
	Point3 pt(_x + randx * 32, _y + randy * 32, _z);
	firetype->makeBulletSplashShapeAndPlaySound(pt.x, pt.y, pt.z);

	if (firetype->getRange() > 0) {
		uint16 damage = firetype->getRandomDamage();
		firetype->applySplashDamageAround(pt, damage, 1, nullptr, nullptr);
	}

	_counter++;
	if (_counter > 9) {
		terminate();
		return;
	}

	int sleep = rs.getRandomNumberRng(5, 20);
	Process *wait = new DelayProcess(sleep);
	Kernel::get_instance()->addProcess(wait);
	waitFor(wait);
}


void BoboBoomerProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeSint32LE(_counter);
	ws->writeSint32LE(_x);
	ws->writeSint32LE(_y);
	ws->writeSint32LE(_z);
}

bool BoboBoomerProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_counter = rs->readSint32LE();
	_x = rs->readSint32LE();
	_y = rs->readSint32LE();
	_z = rs->readSint32LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
