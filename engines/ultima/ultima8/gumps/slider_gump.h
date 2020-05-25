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

#ifndef ULTIMA8_GUMPS_SLIDERGUMP_H
#define ULTIMA8_GUMPS_SLIDERGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class UCProcess;
class RenderedText;

/**
 * A slider that lets you choose how many things to move (eg, when moving stacked items in the backpack)
 */
class SliderGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	SliderGump();
	SliderGump(int x, int y, int16 min, int16 max,
	           int16 value, int16 delta = 1);
	~SliderGump() override;

	void InitGump(Gump *newparent, bool take_focus = true) override;
	void PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) override;
	void Close(bool no_del = false) override;
	void ChildNotify(Gump *child, uint32 message) override;

	void setUsecodeNotify(UCProcess *ucp);

	// Dragging
	bool StartDraggingChild(Gump *gump, int32 mx, int32 my) override;
	void DraggingChild(Gump *gump, int mx, int my) override;
	void StopDraggingChild(Gump *gump) override;

	bool OnKeyDown(int key, int mod) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	int16 _min;
	int16 _max;
	int16 _delta;
	int16 _value;

	uint16 _usecodeNotifyPID;

	int16 _renderedValue;
	RenderedText *_renderedText;

	int getSliderPos();
	void setSliderPos();
	void setValueFromSlider(int sliderx);
	void drawText(RenderSurface *surf);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
