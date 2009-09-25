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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#ifndef TOLTECS_MENU_H
#define TOLTECS_MENU_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Toltecs {

const byte kFontColorMenuDefault = 229;
const byte kFontColorMenuActive  = 255;

class Widget {
public:
	Widget(ToltecsEngine *vm, int16 x, int16 y);
	virtual ~Widget();
	virtual void redraw();
	virtual Widget *getHoveredWidget(int mouseX, int mouseY);
	virtual void calcDimensions();
	void setRect(Common::Rect rect);
	//virtual void setHilighted(bool active);
	virtual void onMouseEnter();
	virtual void onMouseLeave();
	virtual void onMouseMove(int mouseX, int mouseY);
protected:
	ToltecsEngine *_vm;
	Common::Rect _rect;
	//bool _hilighted;
};

const int kLabelCentered	= 1 << 1;
const int kLabelHideOnMovie = 1 << 2;

class LabelWidget : public Widget {
public:
	LabelWidget(ToltecsEngine *vm, int x, int y, Common::String caption, uint flags);
	~LabelWidget();
	void redraw();
	void calcDimensions();
	void setCaption(Common::String caption);
	void setFontColor(byte fontColor);
	void onMouseEnter();
	void onMouseLeave();
protected:
	Common::String _caption;
	uint _flags;
	byte _fontColor;
};

class VolumeControlWidget : public Widget {
public:
	VolumeControlWidget(ToltecsEngine *vm, int x, int y, Common::String caption, uint flags);
	~VolumeControlWidget();
	void redraw();
	Widget *getHoveredWidget(int mouseX, int mouseY);
	void calcDimensions();
	//void setHilighted(bool active);
	void onMouseEnter();
	void onMouseLeave();
	void onMouseMove(int mouseX, int mouseY);
protected:
	uint _flags;
	LabelWidget *_label, *_up, *_down, *_indicator;
	Widget *_activeWidget;
};

class MenuPage {
public:
	MenuPage(Common::String caption);
	~MenuPage();
	void addWidget(Widget *widget);
	void redraw();
	Widget *getHoveredWidget(int mouseX, int mouseY);
protected:
	typedef Common::Array<Widget*> WidgetArray;
	Common::String _caption;
	WidgetArray _widgets;
};

class MenuSystem {

public:
	MenuSystem(ToltecsEngine *vm);
	~MenuSystem();

	void update();
	
protected:
	ToltecsEngine *_vm;

	//LabelWidget *label1, *label2;
	MenuPage *_page;

	Widget *_activeWidget;
	int16 _oldMouseX, _oldMouseY;

};

} // End of namespace Toltecs

#endif /* TOLTECS_MENU_H */
