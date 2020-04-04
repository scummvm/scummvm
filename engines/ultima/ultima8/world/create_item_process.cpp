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

	ods->writeUint32LE(_shape);
	ods->writeUint32LE(_frame);
	ods->writeUint16LE(_quality);
	ods->writeUint16LE(_flags);
	ods->writeUint16LE(_npcNum);
	ods->writeUint16LE(_mapNum);
	ods->writeUint32LE(_extendedFlags);
	ods->writeUint32LE(static_cast<uint32>(_x));
	ods->writeUint32LE(static_cast<uint32>(_y));
	ods->writeUint32LE(static_cast<uint32>(_z));
}

bool CreateItemProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	_shape = ids->readUint32LE();
	_frame = ids->readUint32LE();
	_quality = ids->readUint16LE();
	_flags = ids->readUint16LE();
	_npcNum = ids->readUint16LE();
	_mapNum = ids->readUint16LE();
	_extendedFlags = ids->readUint32LE();
	_x = static_cast<int32>(ids->readUint32LE());
	_y = static_cast<int32>(ids->readUint32LE());
	_z = static_cast<int32>(ids->readUint32LE());
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
