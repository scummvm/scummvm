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

#include "ultima/ultima.h"
#include "ultima/ultima8/gumps/target_gump.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TargetGump)

TargetGump::TargetGump() : ModalGump(), _targetTracing(false) {

}

// Skip pause as usecode processes need to complete & matches original game
TargetGump::TargetGump(int x, int y)
	: ModalGump(x, y, 0, 0, 0, FLAG_DONT_SAVE | FLAG_PREVENT_SAVE, LAYER_MODAL, false),
	_targetTracing(false) {

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
	mouse->pushMouseCursor(Mouse::MOUSE_TARGET);
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
	if (button == Mouse::BUTTON_LEFT) {
		_targetTracing = true;

		_parent->GumpToScreenSpace(mx, my);

		Gump *desktopgump = _parent;
		ObjId objId = desktopgump->TraceObjId(mx, my);
		Item *item = getItem(objId);

		if (item) {
			// done
			debugC(kDebugObject, "Target result: %s", item->dumpInfo().c_str());

			_processResult = objId;
			Close();
		}

		_targetTracing = false;
	}
}

bool TargetGump::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_ESCAPE: {
		Close();
	} break;
	default:
		break;
	}

	return true;
}

uint32 TargetGump::I_target(const uint8 * /*args*/, unsigned int /*argsize*/) {
	TargetGump *targetgump = new TargetGump(0, 0);
	targetgump->InitGump(0);

	return targetgump->GetNotifyProcess()->getPid();
}

void TargetGump::saveData(Common::WriteStream *ws) {
	warning("Trying to save ModalGump");
}

bool TargetGump::loadData(Common::ReadStream *rs, uint32 versin) {
	warning("Trying to load ModalGump");
	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
