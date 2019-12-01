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
#include "ultima8/world/create_item_process.h"
#include "ultima8/world/item_factory.h"
#include "ultima8/world/item.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

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
                                     int32 x_, int32 y_, int32 z_)
	: shape(shape_), frame(frame_), quality(quality_), flags(flags_),
	  npcnum(npcnum_), mapnum(mapnum_), extendedflags(extendedflags_),
	  x(x_), y(y_), z(z_) {

}

CreateItemProcess::~CreateItemProcess(void) {

}

void CreateItemProcess::run() {
	Item *item = ItemFactory::createItem(shape, frame, quality, flags,
	                                     npcnum, mapnum, extendedflags, true);
	item->move(x, y, z);

	result = item->getObjId();

	terminate();
}

void CreateItemProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(shape);
	ods->write4(frame);
	ods->write2(quality);
	ods->write2(flags);
	ods->write2(npcnum);
	ods->write2(mapnum);
	ods->write4(extendedflags);
	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(z));
}

bool CreateItemProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	shape = ids->read4();
	frame = ids->read4();
	quality = ids->read2();
	flags = ids->read2();
	npcnum = ids->read2();
	mapnum = ids->read2();
	extendedflags = ids->read4();
	x = static_cast<int32>(ids->read4());
	y = static_cast<int32>(ids->read4());
	z = static_cast<int32>(ids->read4());
	return true;
}

} // End of namespace Ultima8
