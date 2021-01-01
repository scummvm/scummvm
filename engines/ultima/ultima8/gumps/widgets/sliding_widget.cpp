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
#include "ultima/ultima8/gumps/widgets/sliding_widget.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/shape.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SlidingWidget)

SlidingWidget::SlidingWidget()
	: Gump() {
}

SlidingWidget::SlidingWidget(int x, int y, const Shape *shape, uint32 frameNum)
	: Gump(x, y, 5, 5, 0, FLAG_DRAGGABLE) {
	_shape = shape;
	_frameNum = frameNum;
}

SlidingWidget::~SlidingWidget() {
}

void SlidingWidget::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	UpdateDimsFromShape();
}

uint16 SlidingWidget::TraceObjId(int32 mx, int32 my) {
	if (PointOnGump(mx, my))
		return getObjId();
	else
		return 0;
}

void SlidingWidget::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);
}

bool SlidingWidget::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Gump::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
