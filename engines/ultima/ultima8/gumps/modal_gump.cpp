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
#include "ultima/ultima8/gumps/modal_gump.h"

#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/audio/audio_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ModalGump, Gump)

ModalGump::ModalGump() : Gump() {

}


ModalGump::ModalGump(int x, int y, int width, int height, uint16 owner,
                     uint32 flags, int32 layer)
	: Gump(x, y, width, height, owner, flags, layer) {

}

ModalGump::~ModalGump() {
}

void ModalGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	// lock keyboard
	Ultima8Engine::get_instance()->enterTextMode(this);

	Kernel::get_instance()->pause();

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->pauseAllSamples();
}

Gump *ModalGump::FindGump(int mx, int my) {
	Gump *ret = Gump::FindGump(mx, my);
	if (!ret) ret = this; // we take all mouse input

	return ret;
}

bool ModalGump::PointOnGump(int mx, int my) {
	return true; // we take all mouse input
}

uint16 ModalGump::TraceObjId(int32 mx, int32 my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);
	if (!objId_) objId_ = getObjId();

	return objId_;
}

void ModalGump::Close(bool no_del) {
	// free keyboard
	Ultima8Engine::get_instance()->leaveTextMode(this);

	Kernel::get_instance()->unpause();

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->unpauseAllSamples();

	Gump::Close(no_del);
}

Gump *ModalGump::OnMouseDown(int button, int32 mx, int32 my) {
	Gump *handled = Gump::OnMouseDown(button, mx, my);
	if (!handled) handled = this;
	return handled;
}


void ModalGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

bool ModalGump::loadData(IDataSource *ids, uint32 version) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
