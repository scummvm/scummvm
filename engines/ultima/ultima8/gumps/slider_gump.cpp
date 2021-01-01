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
#include "ultima/ultima8/gumps/slider_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/gumps/widgets/sliding_widget.h"
#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/usecode/uc_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SliderGump)

SliderGump::SliderGump() : ModalGump(), _renderedText(nullptr), _min(0), _max(0),
		_delta(0), _value(0), _usecodeNotifyPID(0), _renderedValue(-1) {
}


SliderGump::SliderGump(int x, int y, int16 min, int16 max,
                       int16 value, int16 delta)
	: ModalGump(x, y, 5, 5), _min(min), _max(max), _delta(delta), _value(value),
	  _usecodeNotifyPID(0), _renderedText(nullptr), _renderedValue(-1) {
}

SliderGump::~SliderGump() {
}

/*
  41:0 = slider gump
  42:0,1 = ok button
  43:0,1 = left button
  44:0,1 = right button
  45:0 = slider
*/

static const int gumpshape = 41;
static const int okshape = 42;
static const int leftshape = 43;
static const int rightshape = 44;
static const int slidershape = 45;
static const int sliderframe = 0;
static const int slidery = 17;
static const int sliderminx = 55;
static const int slidermaxx = 130;
static const int labelx = 161;
static const int labely = 26;
static const int labelfont = 0;

static const int OK_INDEX = 1;
static const int LEFT_INDEX = 2;
static const int RIGHT_INDEX = 3;
static const int SLIDER_INDEX = 4;

int SliderGump::getSliderPos() {
	return sliderminx + (_value - _min) * (slidermaxx - sliderminx) / (_max - _min);
}

void SliderGump::setValueFromSlider(int sliderx) {
	int val = (sliderx - sliderminx) * (_max - _min) / (slidermaxx - sliderminx) + _min;
	if (val < _min) val = _min;
	if (val > _max) val = _max;
	_value = _min + _delta * (static_cast<int16>(val / _delta));
}

void SliderGump::setSliderPos() {
	Gump *slider = Gump::FindGump<SlidingWidget>();
	assert(slider);
	slider->Move(getSliderPos(), slidery);
}

void SliderGump::drawText(RenderSurface *surf) {
	if (!_renderedText || _value != _renderedValue) {
		Font *font;
		font = FontManager::get_instance()->getGameFont(labelfont);
		char buf[10]; // more than enough for a int16
		sprintf(buf, "%d", _value);

		unsigned int remaining;
		delete _renderedText;
		_renderedText = font->renderText(buf, remaining);
		_renderedValue = _value;
	}

	_renderedText->draw(surf, labelx, labely);
}

void SliderGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);

	drawText(surf);
}


void SliderGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	_shape = GameData::get_instance()->getGumps()->getShape(gumpshape);
	UpdateDimsFromShape();

	const Shape *childshape = GameData::get_instance()->
	                    getGumps()->getShape(slidershape);

	// Create the SlidingWidget
	Gump *widget = new SlidingWidget(getSliderPos(), slidery,
	                                 childshape, sliderframe);
	widget->SetIndex(SLIDER_INDEX);
	widget->InitGump(this);

	FrameID button_up(GameData::GUMPS, okshape, 0);
	FrameID button_down(GameData::GUMPS, okshape, 1);

	widget = new ButtonWidget(14, 17, button_up, button_down);
	widget->SetIndex(OK_INDEX);
	widget->InitGump(this);

	FrameID buttonleft_up(GameData::GUMPS, leftshape, 0);
	FrameID buttonleft_down(GameData::GUMPS, leftshape, 1);

	widget = new ButtonWidget(36, 17, buttonleft_up, buttonleft_down);
	widget->SetIndex(LEFT_INDEX);
	widget->InitGump(this);


	FrameID buttonright_up(GameData::GUMPS, rightshape, 0);
	FrameID buttonright_down(GameData::GUMPS, rightshape, 1);

	widget = new ButtonWidget(141, 17, buttonright_up, buttonright_down);
	widget->SetIndex(RIGHT_INDEX);
	widget->InitGump(this);
}

void SliderGump::ChildNotify(Gump *child, uint32 message) {
	if (message == ButtonWidget::BUTTON_CLICK) {
		switch (child->GetIndex()) {
		case OK_INDEX:
			Close();
			break;
		case LEFT_INDEX:
			_value -= _delta;
			if (_value < _min) _value = _min;
			setSliderPos();
			break;
		case RIGHT_INDEX:
			_value += _delta;
			if (_value > _max) _value = _max;
			setSliderPos();
			break;
		}
	}
}


void SliderGump::Close(bool no_del) {
	_processResult = _value;

	if (_usecodeNotifyPID) {
		UCProcess *ucp = dynamic_cast<UCProcess *>(Kernel::get_instance()->getProcess(_usecodeNotifyPID));
		assert(ucp);
		ucp->setReturnValue(_value);
		ucp->wakeUp(_value);
	}

	ModalGump::Close(no_del);
}

bool SliderGump::StartDraggingChild(Gump *gump, int32 mx, int32 my) {
	if (gump->GetIndex() == SLIDER_INDEX) {
		gump->ParentToGump(mx, my);
		Mouse::get_instance()->setDraggingOffset(mx, 0);
		return true;
	}

	return false;
}

void SliderGump::DraggingChild(Gump *gump, int mx, int my) {
	if (gump->GetIndex() == SLIDER_INDEX) {
		int32 dox, doy;
		Mouse::get_instance()->getDraggingOffset(dox, doy);
		setValueFromSlider(mx - dox);
		gump->Move(getSliderPos(), slidery);
	}
}

void SliderGump::StopDraggingChild(Gump *gump) {
}

bool SliderGump::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_LEFT:
		_value -= _delta;
		if (_value < _min) _value = _min;
		setSliderPos();
		break;
	case Common::KEYCODE_RIGHT:
		_value += _delta;
		if (_value > _max) _value = _max;
		setSliderPos();
		break;
	case Common::KEYCODE_RETURN:
		Close();
		break;
	default:
		break;
	}

	return true;
}

void SliderGump::setUsecodeNotify(UCProcess *ucp) {
	assert(ucp);
	_usecodeNotifyPID = ucp->getPid();
}

void SliderGump::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

bool SliderGump::loadData(Common::ReadStream *rs, uint32 version) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
