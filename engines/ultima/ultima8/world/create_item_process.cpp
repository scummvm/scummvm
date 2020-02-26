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
#include "ultima/ultima8/world/create_item_process.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/item.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CreateItemProcess, Process)

CreateItemProcess::CreateItemProcess()
	: Process() {

}

CreateItemProcess::CreateItemProcess(uint32 shape_, uint32 frame_,
                                     uint16 quality_, uint16 flags_,
                                     uint16 npcnum_, uint16 mapnum_,
                                     uint32 extendedflags_,
                                     int32 x, int32 y, int32 z)
	: _shape(shape_), _frame(frame_), _quality(quality_), _flags(flags_),
	  _npcNum(npcnum_), _mapNum(mapnum_), _extendedFlags(extendedflags_),
	  _x(x), _y(y), _z(z) {

}

CreateItemProcess::~CreateItemProcess(void) {
}

void CreateItemProcess::run() {
	Item *item = ItemFactory::createItem(_shape, _frame, _quality, _flags,
	                                     _npcNum, _mapNum, _extendedFlags, true);
	item->move(_x, _y, _z);

	_result = item->getObjId();

	terminate();
}

void CreateItemProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(_shape);
	ods->write4(_frame);
	ods->write2(_quality);
	ods->write2(_flags);
	ods->write2(_npcNum);
	ods->write2(_mapNum);
	ods->write4(_extendedFlags);
	ods->write4(static_cast<uint32>(_x));
	ods->write4(static_cast<uint32>(_y));
	ods->write4(static_cast<uint32>(_z));
}

bool CreateItemProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	_shape = ids->read4();
	_frame = ids->read4();
	_quality = ids->read2();
	_flags = ids->read2();
	_npcNum = ids->read2();
	_mapNum = ids->read2();
	_extendedFlags = ids->read4();
	_x = static_cast<int32>(ids->read4());
	_y = static_cast<int32>(ids->read4());
	_z = static_cast<int32>(ids->read4());
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
