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

#ifndef ULTIMA8_GUMPS_WIDGETS_SLIDINGWIDGET_H
#define ULTIMA8_GUMPS_WIDGETS_SLIDINGWIDGET_H

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

class SlidingWidget : public Gump {
protected:
	Common::Rect32 _dragBounds;

public:
	ENABLE_RUNTIME_CLASSTYPE()

	SlidingWidget();
	SlidingWidget(int x, int y, FrameID frame, const Common::Rect32 &dragBounds);
	~SlidingWidget() override;

	int getValueForRange(int min, int max);
	void setValueForRange(int value, int min, int max);

	void InitGump(Gump *newparent, bool take_focus = true) override;
	uint16 TraceObjId(int32 mx, int32 my) override;

	void Move(int32 x, int32 y) override;
	void onDrag(int32 mx, int32 my) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	enum Message {
		DRAGGING = 0
	};
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
