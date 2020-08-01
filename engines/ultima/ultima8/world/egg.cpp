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

#include "ultima/ultima8/world/egg.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/usecode/uc_machine.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(Egg)

Egg::Egg() : _hatched(false) {
}


Egg::~Egg() {
}

uint16 Egg::hatch() {
	if (_hatched) return 0;
	_hatched = true;
	return callUsecodeEvent_hatch();
}

uint16 Egg::unhatch() {
	if (GAME_IS_CRUSADER) {
		if (!_hatched) return 0;
		_hatched = false;
		return callUsecodeEvent_unhatch();
	}
	return 0;
}

void Egg::dumpInfo() const {
	Item::dumpInfo();
	pout << "range: " << getXRange() << "," << getYRange()
	     << ", hatched=" << _hatched << Std::endl;
}

void Egg::leaveFastArea() {
	reset();
	Item::leaveFastArea();
}

void Egg::saveData(Common::WriteStream *ws) {
	Item::saveData(ws);

	uint8 h = _hatched ? 1 :  0;
	ws->writeByte(h);
}

bool Egg::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Item::loadData(rs, version)) return false;

	_hatched = (rs->readByte() != 0);

	return true;
}

uint32 Egg::I_getEggXRange(const uint8 *args, unsigned int /*argsize*/) {
	ARG_EGG_FROM_PTR(egg);
	if (!egg) return 0;

	return static_cast<uint32>(egg->getXRange());
}

uint32 Egg::I_getEggYRange(const uint8 *args, unsigned int /*argsize*/) {
	ARG_EGG_FROM_PTR(egg);
	if (!egg) return 0;

	return static_cast<uint32>(egg->getYRange());
}

uint32 Egg::I_setEggXRange(const uint8 *args, unsigned int /*argsize*/) {
	ARG_EGG_FROM_PTR(egg);
	ARG_UINT16(xr);
	if (!egg) return 0;

	egg->setXRange(xr);
	return 0;
}

uint32 Egg::I_setEggYRange(const uint8 *args, unsigned int /*argsize*/) {
	ARG_EGG_FROM_PTR(egg);
	ARG_UINT16(yr);
	if (!egg) return 0;

	egg->setYRange(yr);
	return 0;
}

uint32 Egg::I_getEggId(const uint8 *args, unsigned int /*argsize*/) {
	ARG_EGG_FROM_PTR(egg);
	if (!egg) return 0;

	return egg->getMapNum();
}

uint32 Egg::I_setEggId(const uint8 *args, unsigned int /*argsize*/) {
	ARG_EGG_FROM_PTR(egg);
	ARG_UINT16(eggid);
	if (!egg) return 0;

	egg->setMapNum(eggid);

	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
