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

namespace Ultima8 {

class UCProcess;
class RenderedText;

class SliderGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	SliderGump();
	SliderGump(int x, int y, int16 min, int16 max,
	           int16 value, int16 delta = 1);
	virtual ~SliderGump(void);

	virtual void InitGump(Gump *newparent, bool take_focus = true);
	virtual void PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled);
	virtual void Close(bool no_del = false);
	virtual void ChildNotify(Gump *child, uint32 message);

	void setUsecodeNotify(UCProcess *ucp);

	// Dragging
	virtual bool StartDraggingChild(Gump *gump, int mx, int my);
	virtual void DraggingChild(Gump *gump, int mx, int my);
	virtual void StopDraggingChild(Gump *gump);

	virtual bool OnKeyDown(int key, int mod);

	bool loadData(IDataSource *ids, uint32 version);
protected:
	virtual void saveData(ODataSource *ods);

	int16 min;
	int16 max;
	int16 delta;
	int16 value;

	uint16 usecodeNotifyPID;

	int16 renderedvalue;
	RenderedText *renderedtext;

	int getSliderPos();
	void setSliderPos();
	void setValueFromSlider(int sliderx);
	void drawText(RenderSurface *surf);
};

} // End of namespace Ultima8

#endif
