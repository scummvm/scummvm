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

#ifndef ZVISION_MENU_H
#define ZVISION_MENU_H

#include "graphics/surface.h"
#include "common/rect.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/scroller.h"

namespace ZVision {

enum menuBar {
	kMenubarExit = 0x1,
	kMenubarSettings = 0x2,
	kMenubarRestore = 0x4,
	kMenubarSave = 0x8,
	kMenubarItems = 0x100,
	kMenubarMagic = 0x200
};

class MenuHandler {
public:
	MenuHandler(ZVision *engine, Common::Rect menuArea);
	virtual ~MenuHandler() {};
	virtual void onMouseMove(const Common::Point &Pos) {};
	virtual void onMouseDown(const Common::Point &Pos) {};
	virtual void onMouseUp(const Common::Point &Pos) {};
	virtual void process(uint32 deltaTimeInMillis) {};
  bool isInMenu() {return inMenu;}; //For widescreen mod; used to suspend panning, tilting & scripting triggers when the mouse is within the working window but also in the menu.
  //NB former boolean flag inMenu has been renamed to menuActive.

	void setEnable(uint16 flags) {
		menuBarFlag = flags;
	}
	uint16 getEnable() {
		return menuBarFlag;
	}
protected:
	uint16 menuBarFlag;
	ZVision *_engine;
  Common::Rect _menuArea;	
  Common::Rect _menuTriggerArea;
	bool menuActive;  //True if mouse is in menu area
	bool inMenu = false;  //True if mouse is over scrolled menu graphics, regardless of menuActive
	bool redraw = true;
	int mouseOnItem;
	int32 colorkey = -1;  //Transparency color for compositing menu over playfield
};

class MenuZGI: public MenuHandler {
public:
	MenuZGI(ZVision *engine, Common::Rect menuArea);
	~MenuZGI() override;
	void onMouseMove(const Common::Point &Pos) override;
	void onMouseUp(const Common::Point &Pos) override;
	void process(uint32 deltaTimeInMillis) override;
private:
	Graphics::Surface menuBack[3][2];
	Graphics::Surface menuBar[4][2];
	Graphics::Surface *items[50][2];
	uint itemId[50];

	Graphics::Surface *magic[12][2];
	uint magicId[12];

	int menuMouseFocus;

	bool scrolled[3];
	int16 scrollPos[3];

	bool clean;
};

class MenuNemesis: public MenuHandler {
public:
	MenuNemesis(ZVision *engine, Common::Rect menuArea);
	~MenuNemesis() override;
	void onMouseMove(const Common::Point &Pos) override;
	void onMouseUp(const Common::Point &Pos) override;
	void process(uint32 deltaTimeInMillis) override;
private:
	Graphics::Surface but[4][6];
	Graphics::Surface menuBar;
	Common::Rect hotspots[4];
	
  //Widths & X positions of buttons; {Save, Restore, Prefs, Exit}
  //X positions relative to left of menu area
  //const int16 wxButs[4][2] = { {120 , 64}, {144, 184}, {128, 328}, {120, 456} };  //Originals
  const int16 wxButs[4][2] = { {120 , 0}, {144, 120}, {128, 264}, {120, 392} };
	
  Scroller menuScroller;

	int frm;
	int16 delay;
};

} // End of namespace ZVision

#endif
