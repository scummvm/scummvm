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

#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(DestroyItemProcess)

DestroyItemProcess::DestroyItemProcess() : Process() {

}

DestroyItemProcess::DestroyItemProcess(Item *item) {
	if (item)
		_itemNum = item->getObjId();
	else
		_itemNum = 0;

	_type = 0x232;
}

void DestroyItemProcess::run() {
	if (_itemNum == 0) {
		// need to get ObjId to use from process result. (We were apparently
		// waiting for a process which returned the ObjId to delete.)
		_itemNum = static_cast<ObjId>(_result);
	}

	Item *it = getItem(_itemNum);

	if (!it) {
		// somebody did our work for us...
		terminate();
		return;
	}

	// FIXME: should probably prevent player from opening gump in the
	// first place...
	if (it->hasFlags(Item::FLG_GUMP_OPEN)) {
		// first close gump in case player is still rummaging through us
		it->closeGump();
	}

	// bye bye
	// (note that Container::destroy() calls removeContents())
	it->destroy(true);

	// NOTE: we're terminated here because this process belongs to the item
}

void DestroyItemProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
}

bool DestroyItemProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
