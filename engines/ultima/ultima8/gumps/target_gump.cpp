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
#include "ultima/ultima8/gumps/target_gump.h"

#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TargetGump)

TargetGump::TargetGump() : ModalGump(), _targetTracing(false) {

}


TargetGump::TargetGump(int x, int y)
	: ModalGump(x, y, 0, 0), _targetTracing(false) {

}

TargetGump::~TargetGump() {
}

void TargetGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// we're invisible
}

void TargetGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	// we need a notifier process
	CreateNotifier();

	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor();
	mouse->setMouseCursor(Mouse::MOUSE_TARGET);
}

void TargetGump::Close(bool no_del) {
	Mouse *mouse = Mouse::get_instance();
	mouse->popMouseCursor();

	ModalGump::Close(no_del);
}

bool TargetGump::PointOnGump(int mx, int my) {
	// HACK alert: if we're currently tracing from TargetGump::onMouseUp,
	//  then we do NOT want to intercept the trace
	if (_targetTracing) return false;

	return ModalGump::PointOnGump(mx, my);
}

void TargetGump::onMouseUp(int button, int32 mx, int32 my) {
	_targetTracing = true;

	_parent->GumpToScreenSpace(mx, my);

	Gump *desktopgump = _parent;
	ObjId objId = desktopgump->TraceObjId(mx, my);
	Item *item = getItem(objId);

	if (item) {
		// done
		pout << "Target result: ";
		item->dumpInfo();

		_processResult = objId;
		Close();
	}

	_targetTracing = false;
}

uint32 TargetGump::I_target(const uint8 * /*args*/, unsigned int /*argsize*/) {
	TargetGump *targetgump = new TargetGump(0, 0);
	targetgump->InitGump(0);

	return targetgump->GetNotifyProcess()->getPid();
}



void TargetGump::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

bool TargetGump::loadData(Common::ReadStream *rs, uint32 versin) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
