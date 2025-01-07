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
#include "common/array.h"
#include "common/bitarray.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/common/scroller.h"
#include "zvision/common/focus_list.h"

namespace ZVision {

enum {
	kMainMenuSave = 0,
	kMainMenuLoad = 1,
	kMainMenuPrefs = 2,
	kMainMenuExit = 3,
	kItemsMenu = 4,
	kMagicMenu = 5
};

struct MenuParams {
  int16 wxButs[4][2];   //Widths & X positions of main menu buttons; {Save, Restore, Prefs, Quit}
  int16 wMain;  //Width of main menu background
  int8 idleFrame; //Frame to display of unselected main menu button
  int8 activeFrame; //Frame to display of selected main menu button when mouse is down
  int8 clickedFrame; //Frame to display of selected main menu button when mouse is down
  Common::Point activePos;  //Fully scrolled main menu position, relative to origin of menu area
  Common::Point idlePos;  //Fully retracted main menu position, relative to origin of menu area
  int16 period; //Duration of main menu scrolldown
  int16 triggerHeight;  //Height of menu trigger area when inactive
  int16 buttonPeriod; //Duration of main menu button animation
};

//NB - menu area is same width as working window.

static const MenuParams nemesisParams {
  { {120 , -1}, {144, 120}, {128, 264}, {120, 392} },
  512,
  -1,
  4,
  5,
  Common::Point(0,0),
  Common::Point(0,-32),
  500,
  2,
  500
};

static const MenuParams zgiParams {
  { {135 , 50}, {135, 185}, {135, 320}, {135, 455} },
  580,
  0,
  1,
  1,
  Common::Point(30,0),
  Common::Point(30,-20),
  250,
  32,
  0
};

class MenuManager {
public:
	MenuManager(ZVision *engine, const Common::Rect menuArea, const MenuParams params);
	virtual ~MenuManager();
	virtual void onMouseMove(const Common::Point &Pos);
	virtual void onMouseDown(const Common::Point &Pos);
	virtual void onMouseUp(const Common::Point &Pos);
	virtual void process(uint32 deltaTimeInMillis);
  bool inMenu() {return prevInMenu;};
  virtual bool inMenu(const Common::Point &Pos) {return false;}; //For widescreen mod; used to suspend panning, tilting & scripting triggers when the mouse is within the working window but also in the menu.

	void mainMouseDown(const Common::Point &Pos); //Show clicked graphic under selected button
	bool mainMouseMove(const Common::Point &Pos); //return true if selected button has changed

	void setEnable(uint16 flags);
	uint16 getEnable() {
		return menuBarFlag;
	}
	bool getEnable(uint8 flag) {
	  return enableFlags.get(flag);
	}
	
protected:
  virtual void redrawAll() {};
  void redrawMain();
	int mouseOverMain(const Common::Point &Pos);
	void setFocus(int8 currentFocus);
	
//	bool inMenu = false;  //True if menus are currently visible
	bool prevInMenu = false;
	bool redraw = true;
	int mouseOnItem = -1;
	static const uint8 hMainMenu = 32;
  int8 mainClicked = -1;

	ZVision *_engine;
  const MenuParams _params;
	uint16 menuBarFlag;
  const Common::Rect _menuArea;
	const Common::Point menuOrigin;
  const Common::Rect menuTriggerArea;
	Graphics::Surface mainBack;
	Graphics::Surface mainButtons[4][6];
	Common::BitArray enableFlags;
  Common::Rect mainArea;
	Common::Rect menuHotspots[4];
  int8 mainFrames[4]; //Frame to display of each main menu button; first row is currently displayed, 2nd row is backbuffer for idle animations
  Scroller mainScroller;
	FocusList<int8> menuFocus;  //Order in which menus have most recently had focus; determines current mouse focus & order in which to redraw them.
	bool clean = false; //Whether or not to blank
  LinearScroller* buttonAnim[4];
};
 
class MenuZGI: public MenuManager {
public:
	MenuZGI(ZVision *engine, Common::Rect menuArea);
	~MenuZGI() override;
	void onMouseMove(const Common::Point &Pos) override;
	void onMouseUp(const Common::Point &Pos) override;
	void process(uint32 deltaTimeInMillis) override;
  bool inMenu(const Common::Point &Pos) override;
private:
  void redrawAll() override;
	Graphics::Surface menuBack[3];
	Graphics::Surface *items[50][2];
	uint itemId[50];

	Graphics::Surface *magic[12][2];
	uint magicId[12];
//  void redrawMain(bool hasFocus) override;
	void redrawItems();
	void redrawMagic();
	int mouseOverItem(const Common::Point &Pos, int itemCount);
	int mouseOverMagic(const Common::Point &Pos);
	
	static const uint16 hSideMenu = 32;
	static const uint16 wSideMenu = 600;
	static const uint16 wSideMenuTab = 20;
  static const int16 magicWidth = 47;

  Scroller itemsScroller, magicScroller;

  const Common::Point magicOrigin;
  const Common::Point itemsOrigin;

  Common::Rect magicArea;
  Common::Rect itemsArea;
};

class MenuNemesis: public MenuManager {
public:
	MenuNemesis(ZVision *engine, Common::Rect menuArea);
	~MenuNemesis() override;
	void onMouseMove(const Common::Point &Pos) override;
  bool inMenu(const Common::Point &Pos) override;
private:
  void redrawAll() override;
};

} // End of namespace ZVision

#endif
