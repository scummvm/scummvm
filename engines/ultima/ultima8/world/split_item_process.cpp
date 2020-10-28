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
#include "ultima/ultima8/world/split_item_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(SplitItemProcess)

SplitItemProcess::SplitItemProcess() : Process(), _target(0) {

}

SplitItemProcess::SplitItemProcess(Item *original, Item *target) {
	assert(original);
	assert(target);

	assert(original->getShapeInfo()->hasQuantity());
	assert(target->getShapeInfo()->hasQuantity());

	_itemNum = original->getObjId();
	_target = target->getObjId();

	// type = TODO
}

void SplitItemProcess::run() {
	Item *original = getItem(_itemNum);
	Item *targetitem = getItem(_target);

	assert(original);
	assert(targetitem);
	assert(original->getShapeInfo()->hasQuantity());
	assert(targetitem->getShapeInfo()->hasQuantity());

	uint16 movecount = static_cast<uint16>(_result);

	assert(movecount <= original->getQuality());

	uint16 origcount = original->getQuality() - movecount;
	uint16 targetcount = targetitem->getQuality() + movecount;

	pout << "SplitItemProcess splitting: " << movecount << ": "
	     << origcount << "-" << targetcount << Std::endl;

	if (targetcount > 0) {
		targetitem->setQuality(targetcount);
		targetitem->callUsecodeEvent_combine();
	} else {
		targetitem->destroy();
	}

	if (origcount > 0) {
		original->setQuality(origcount);
		original->callUsecodeEvent_combine();
	} else {
		original->destroy(); // note: this terminates us
	}

	_result = 0;

	if (!is_terminated())
		terminate();
}

void SplitItemProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint16LE(_target);
}

bool SplitItemProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_target = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
