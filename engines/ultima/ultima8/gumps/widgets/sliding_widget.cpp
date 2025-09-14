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

#include "ultima/ultima8/gumps/widgets/sliding_widget.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SlidingWidget)

SlidingWidget::SlidingWidget()
	: Gump(), _dragBounds() {
}

SlidingWidget::SlidingWidget(int x, int y, FrameID frame, const Common::Rect32 &dragBounds)
	: Gump(x, y, 5, 5, 0, FLAG_DRAGGABLE), _dragBounds(dragBounds) {
	SetShape(frame, true);
	if (_dragBounds.width() < _dims.width())
		_dragBounds.setWidth(_dims.width());
	if (_dragBounds.height() < _dims.height())
		_dragBounds.setHeight(_dims.height());
}

SlidingWidget::~SlidingWidget() {
}

int SlidingWidget::getValueForRange(int min, int max) {
	int val = min;
	if (_dragBounds.isValidRect()) {
		val = min + (_x - _dragBounds.left) * (max - min) / (_dragBounds.width() - _dims.width());
		if (val < min)
			val = min;
		if (val > max)
			val = max;
	}
	return val;
}

void SlidingWidget::setValueForRange(int value, int min, int max) {
	assert(_dragBounds.isValidRect());
	_x = _dragBounds.left + (value - min) * (_dragBounds.width() - _dims.width()) / (max - min);
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

void SlidingWidget::Move(int32 x, int32 y) {
	if (_dragBounds.isValidRect()) {
		if (x < _dragBounds.left)
			x = _dragBounds.left;
		if (x > _dragBounds.right - _dims.width())
			x = _dragBounds.right - _dims.width();
		if (y < _dragBounds.top)
			y = _dragBounds.top;
		if (y > _dragBounds.bottom - _dims.height())
			y = _dragBounds.bottom - _dims.height();
	}

	_x = x;
	_y = y;
}

void SlidingWidget::onDrag(int32 mx, int32 my) {
	Gump::onDrag(mx, my);
	_parent->ChildNotify(this, DRAGGING);
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
