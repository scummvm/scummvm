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

#ifndef ULTIMA8_GUMPS_WIDGETS_BUTTONWIDGET_H
#define ULTIMA8_GUMPS_WIDGETS_BUTTONWIDGET_H

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/graphics/frame_id.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ButtonWidget : public Gump {
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()

	ButtonWidget();
	ButtonWidget(int x, int y, Std::string txt, bool gamefont, int font,
	             uint32 mouseOverBlendCol = 0, int width = 0, int height = 0,
	             int32 layer = LAYER_NORMAL);
	ButtonWidget(int x, int y, FrameID frame_up, FrameID frame_down,
	             bool mouseOver = false, int32 layer = LAYER_NORMAL);
	~ButtonWidget() override;

	void InitGump(Gump *newparent, bool take_focus = true) override;
	uint16 TraceObjId(int32 mx, int32 my) override;
	bool PointOnGump(int mx, int my) override;

	Gump *onMouseDown(int button, int32 mx, int32 my) override;
	void onMouseUp(int button, int32 mx, int32 my) override;
	void onMouseClick(int button, int32 mx, int32 my) override;
	void onMouseDouble(int button, int32 mx, int32 my) override;

	void onMouseOver() override;
	void onMouseLeft() override;

	//! return the textwidget's vlead, or 0 for an image button
	int getVlead();

	//void SetShapeDown(Shape *_shape, uint32 frameNum);
	//void SetShapeUp(Shape *_shape, uint32 frameNum);

	enum Message {
		BUTTON_CLICK        = 0,
		BUTTON_UP           = 1,
		BUTTON_DOUBLE       = 2
	};

protected:
	Shape *_shapeUp;
	uint32 _frameNumUp;
	Shape *_shapeDown;
	uint32 _frameNumDown;
	uint16 _textWidget;
	uint32 _mouseOverBlendCol;
	bool _mouseOver;
	int _origW, _origH;

public:
	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
