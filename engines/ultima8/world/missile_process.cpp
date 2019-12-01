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


// This class is DEPRECATED.
// It only exists to provide backward compatibility for savegames



#include "ultima8/misc/pent_include.h"

#include "missile_process.h"
#include "ultima8/world/Item.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MissileProcess, Process);

MissileProcess::MissileProcess()
	: Process() {

}

void MissileProcess::run() {
	// some values to make items use a nice arc :-)
	// (No, this isn't permanent, but it makes the fish fly nicely ;-) )
	static const int sinvals[21] = {
		0, 156, 309, 454, 588, 705, 809, 891, 952, 987, 1000,
		987, 952, 891, 809, 705, 588, 454, 309, 156, 0
	};

	Item *it = getItem(item_num);

	if (!it) {
		// item gone
		terminate();
		return;
	}

	int32 x, y, z;

	currentpos += 1;
	if (currentpos > (2500 / speed)) currentpos = (2500 / speed);

	//!! speed doesn't work the way it (probably) should...
	x = from_x + (((to_x - from_x) * currentpos) * speed) / 2500;
	y = from_y + (((to_y - from_y) * currentpos) * speed) / 2500;
	z = from_z + (((to_z - from_z) * currentpos) * speed) / 2500;
	if (curve) z += sinvals[(20 * currentpos * speed) / 2500] / 25;

	it->collideMove(x, y, z, false, false);

	if (currentpos >= (2500 / speed)) {
		it->fall();
		terminate();
	}
}

void MissileProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(from_x));
	ods->write4(static_cast<uint32>(from_y));
	ods->write4(static_cast<uint32>(from_z));
	ods->write4(static_cast<uint32>(to_x));
	ods->write4(static_cast<uint32>(to_y));
	ods->write4(static_cast<uint32>(to_z));
	ods->write4(static_cast<uint32>(speed));
	ods->write4(static_cast<uint32>(currentpos));

	uint8 c = (curve ? 1 : 0);
	ods->write1(c);

}

bool MissileProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	from_x = static_cast<int>(ids->read4());
	from_y = static_cast<int>(ids->read4());
	from_z = static_cast<int>(ids->read4());
	to_x = static_cast<int>(ids->read4());
	to_y = static_cast<int>(ids->read4());
	to_z = static_cast<int>(ids->read4());
	speed = static_cast<int>(ids->read4());
	currentpos = static_cast<int>(ids->read4());

	curve = (ids->read1() != 0);

	return true;
}
