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

#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/menu.h"

//FOR BUG TEST ONLY
//#include "common/debug.h"

namespace ZVision {

enum {
  kFocusNone = -1,
	kFocusItems = 0,
	kFocusMagic = 1,
	kFocusMain = 2
};

MenuHandler::MenuHandler(ZVision *engine, const Common::Rect menuArea, const MenuParams params) :
	_engine{engine},
  _params{params},
	menuBarFlag{0xFFFF},
	_menuArea{menuArea},
  menuOrigin{menuArea.left,menuArea.top},
  menuTriggerArea{menuOrigin,_menuArea.width(),_params.triggerHeight},
  mainScroller{params.activePos, params.idlePos, params.period} {
	
	enableFlags.set_size(6);
  for(int8 i = 0; i < 4; i++) {
	  //Generate button hotspot areas
	  menuHotspots[i] = Common::Rect(_params.wxButs[i][1], _menuArea.top, _params.wxButs[i][1]+_params.wxButs[i][0], _menuArea.bottom);
    //Initialise button animation frames
    mainFrames[i] = _params.idleFrame;
	}
	for (int i = 0; i < 4; i++)
    buttonAnim[i] = new LinearScroller(_params.activeFrame,_params.idleFrame,_params.buttonPeriod);
	menuFocus.push_back(kFocusNone);
	mainArea = Common::Rect(_params.wMain,hMainMenu);
	mainArea.moveTo(menuOrigin+mainScroller.Pos);
}

MenuHandler::~MenuHandler() {
	for (int i = 0; i < 4; i++)
	  delete buttonAnim[i];
	mainBack.free();
}

void MenuHandler::setEnable(uint16 flags) {
	  static const uint16 flagMasks[6] = {0x8,0x4,0x2,0x1,0x100,0x200};  //Enum order: save,restore,prefs,quit,items,magic
		menuBarFlag = flags;
		for(uint i=0; i<=5; i++) {
      if(menuBarFlag & flagMasks[i])
        enableFlags.set(i);
      else
        enableFlags.unset(i);
    }		  
	}

void MenuHandler::onMouseUp(const Common::Point &Pos) {
  if(menuFocus.front() == kFocusMain) {
    mouseOnItem = mouseOverMain(Pos);
    if(mouseOnItem == mainClicked)
      //Activate clicked action from main menu
      switch(mouseOnItem) {
        case kMainMenuSave:
		      _engine->getScriptManager()->changeLocation('g', 'j', 's', 'e', 0);
		      setFocus(kFocusNone);
		      mainScroller.reset();
		      redraw = true;
		      break;
	      case kMainMenuLoad:
		      _engine->getScriptManager()->changeLocation('g', 'j', 'r', 'e', 0);
		      setFocus(kFocusNone);
		      mainScroller.reset();
		      redraw = true;
		      break;
        case kMainMenuPrefs:
		      _engine->getScriptManager()->changeLocation('g', 'j', 'p', 'e', 0);
		      setFocus(kFocusNone);
		      mainScroller.reset();
		      redraw = true;
	        break;
        case kMainMenuExit:
		      _engine->ifQuit();
		      break;
	      default:
	        break;
      }
  }
  mainClicked = -1;
}

void MenuHandler::onMouseDown(const Common::Point &Pos) {
  if(menuFocus.front() == kFocusMain) {
    mouseOnItem = mouseOverMain(Pos);
    //Show clicked graphic
    if((mouseOnItem>=0) && (mouseOnItem<4))
      if(enableFlags.get(mouseOnItem)) {
        mainClicked = mouseOnItem;
        redraw = true;
      }
  }
  debug("mouse position %d %d", Pos.x, Pos.y);
}

void MenuHandler::onMouseMove(const Common::Point &Pos) {
  bool nowInMenu = inMenu(Pos);
  if(nowInMenu != prevInMenu)
    redraw = true;
  prevInMenu = nowInMenu;
    
	int lastItem = mouseOnItem;
  
  switch(menuFocus.front()) {
    case kFocusMain:
      //Inform game scripting engine that mouse is in main menu
	    if(_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 2)
		    _engine->getScriptManager()->setStateValue(StateKey_MenuState, 2);
      mouseOnItem = mouseOverMain(Pos);
	    break;
	  case kFocusNone:
		  //Inform game scripting engine that mouse is not in any menu
		  if(_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 0)
			  _engine->getScriptManager()->setStateValue(StateKey_MenuState, 0);
		  mouseOnItem = -1;
		  break;
	}
  mainScroller.setActive(menuFocus.front()==kFocusMain);
  //Update button animation status
  for (int i = 0; i < 4; i++)
    if(menuFocus[0] == kFocusMain && mouseOnItem == i)
      buttonAnim[i]->setActive(true);
    else
 	    buttonAnim[i]->setActive(false);
  if(lastItem != mouseOnItem)
    redraw = true;
}

int MenuHandler::mouseOverMain(const Common::Point &Pos) {
  //Common::Rect mainHotspot(28,hSideMenu);
  //mainHotspot.moveTo(mainOrigin + mainScroller.Pos);
  for(int8 i = 0; i < 4; i++) {
    if(enableFlags.get(i) && menuHotspots[i].contains(Pos))
      return i;
  }
  return -1;
}

void MenuHandler::process(uint32 deltatime) {
	if(mainScroller.update(deltatime)) {
	  mainArea.moveTo(menuOrigin+mainScroller.Pos);
    for (int i = 0; i < 4; i++)
      menuHotspots[i].moveTo(_menuArea.left + _params.wxButs[i][1], _menuArea.top + mainScroller.Pos.y);
		redraw = true;
	}
  //Update button highlight animation frame
  for(int i = 0; i < 4; i++)
    if(buttonAnim[i]->update(deltatime)) {
      mainFrames[i] = buttonAnim[i]->Pos;
      redraw = true;
    }
	if(redraw) {
	  _engine->getRenderManager()->clearMenuSurface(colorkey);
  	redrawAll();
  	redraw = false;
  }
};

void MenuNemesis::redrawAll() {
  redrawMain();
};

void MenuHandler::redrawMain() {
  //Draw menu background
  _engine->getRenderManager()->blitSurfaceToMenu(mainBack, mainScroller.Pos.x, mainScroller.Pos.y, 0);
  //Draw buttons
  if(menuFocus.front() == kFocusMain)
    for(int8 i = 0; i < 4; i++) {
      if(enableFlags.get(i) && (mainFrames[i]>=0)) {
        if(mainClicked == i)
	        _engine->getRenderManager()->blitSurfaceToMenu(mainButtons[i][_params.clickedFrame], _params.wxButs[i][1], mainScroller.Pos.y, 0);        
        else
	        _engine->getRenderManager()->blitSurfaceToMenu(mainButtons[i][mainFrames[i]], _params.wxButs[i][1], mainScroller.Pos.y, 0);
      }
    }
  clean = false;
}

void MenuHandler::setFocus(int8 currentFocus) {
  if(menuFocus.front() != currentFocus) {
    Common::Array<int8> _menuFocus;
    while(menuFocus.size() > 0) {
      if(menuFocus.back() != currentFocus)
        _menuFocus.push_back(menuFocus.back());
      menuFocus.pop_back();
    }
    menuFocus.push_back(currentFocus);
    while(_menuFocus.size() > 0) {
      menuFocus.push_back(_menuFocus.back());
      _menuFocus.pop_back();
    }
  }
  assert(menuFocus.size() <= 4);
}

MenuZGI::MenuZGI(ZVision *engine, const Common::Rect menuArea) :
	MenuHandler(engine, menuArea, zgiParams),
	itemsScroller{Common::Point(0,0), Common::Point(wSideMenuTab-wSideMenu,0), 1000},
	magicScroller{Common::Point(-wSideMenu,0), Common::Point(-wSideMenuTab,0), 1000},
	itemsOrigin{menuArea.left, menuArea.top},
	magicOrigin{menuArea.right, menuArea.top} {

  magicArea = Common::Rect(magicOrigin + magicScroller.Pos, wSideMenu, hSideMenu);
  itemsArea = Common::Rect(itemsOrigin + itemsScroller.Pos, wSideMenu, hSideMenu);
	
  //Buffer main menu background
	_engine->getRenderManager()->readImageToSurface("gmzau031.tga", mainBack, false);
	
	char buf[24];
	for (int i = 0; i < 4; i++) {
	  //Buffer menu buttons
		Common::sprintf_s(buf, "gmzmu%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, mainButtons[i][0], false);
		Common::sprintf_s(buf, "gmznu%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, mainButtons[i][1], false);
	}
	for (int i = 1; i < 4; i++) {
	  //Buffer full menu backgrounds
		Common::sprintf_s(buf, "gmzau%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, menuBack[i - 1], false);
	}
	for (int i = 0; i < 50; i++) {
		items[i][0] = NULL;
		items[i][1] = NULL;
		itemId[i] = 0;
	}
	for (int i = 0; i < 12; i++) {
		magic[i][0] = NULL;
		magic[i][1] = NULL;
		magicId[i] = 0;
	}
	//Initialise focus sequence
  setFocus(kFocusMain);
  setFocus(kFocusMagic);
  setFocus(kFocusItems);
  setFocus(kFocusNone);
}

MenuZGI::~MenuZGI() {
	for (int i = 0; i < 3; i++)
		menuBack[i].free();
		
	for (int i = 0; i < 4; i++)
    for(int j = 0; j < 2; j++)
		  mainButtons[i][j].free();
		  
	for (int i = 0; i < 50; i++) {
		if(items[i][0]) {
			items[i][0]->free();
			delete items[i][0];
		}
		if(items[i][1]) {
			items[i][1]->free();
			delete items[i][1];
		}
	}
	for (int i = 0; i < 12; i++) {
		if(magic[i][0]) {
			magic[i][0]->free();
			delete magic[i][0];
		}
		if(magic[i][1]) {
			magic[i][1]->free();
			delete magic[i][1];
		}
	}
}

bool MenuZGI::inMenu(const Common::Point &Pos) {
  return menuTriggerArea.contains(Pos) || (menuFocus.front() != kFocusNone);
}

void MenuZGI::onMouseUp(const Common::Point &Pos) {
	if(inMenu(Pos))
	  //redraw = true;
		switch (menuFocus.front()) {
		  case kFocusItems:
			  if(enableFlags.get(kItemsMenu)) {
				  int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
				  if(itemCount == 0)
					  itemCount = 20;
          int i = mouseOverItem(Pos,itemCount);
          if(i != -1) {
				    int32 mouseItem = _engine->getScriptManager()->getStateValue(StateKey_InventoryItem);
				    if(mouseItem >= 0  && mouseItem < 0xE0) {
					    _engine->getScriptManager()->inventoryDrop(mouseItem);
					    _engine->getScriptManager()->inventoryAdd(_engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + i));
					    _engine->getScriptManager()->setStateValue(StateKey_Inv_StartSlot + i, mouseItem);
					    redraw = true;         
            }
          }
		    }
			  break;
		  case kFocusMagic:
			  if(enableFlags.get(kMagicMenu)) {
			    int i = mouseOverMagic(Pos);
			    if(i != -1) {
				    uint itemnum = _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + i);
				    if(itemnum != 0) {
					    if(_engine->getScriptManager()->getStateValue(StateKey_Reversed_Spellbooc) == 1)
						    itemnum = 0xEE + i;
					    else
						    itemnum = 0xE0 + i;
					    }
				    if(itemnum)
					    if(_engine->getScriptManager()->getStateValue(StateKey_InventoryItem) == 0 || _engine->getScriptManager()->getStateValue(StateKey_InventoryItem) >= 0xE0)
					      _engine->getScriptManager()->setStateValue(StateKey_Active_Spell, itemnum);
			    }
			  }
			  break;
		  case kFocusMain:
        MenuHandler::onMouseUp(Pos);
			  break;
		  default:
			  break;
		  }
}

void MenuZGI::onMouseMove(const Common::Point &Pos) {
  if(!inMenu(Pos)) {
    mainScroller.reset();
    magicScroller.reset();
    itemsScroller.reset();
  }

  //Set focus to topmost layer of menus that mouse is currently over
  for(uint8 i=0; i<menuFocus.size(); i++) {
    switch(menuFocus[i]) {
      case kFocusItems:
        if(itemsArea.contains(Pos)) {
          setFocus(kFocusItems);
          i = menuFocus.size()+1;
        }
        break;
      case kFocusMagic:
        if(magicArea.contains(Pos)) {
          setFocus(kFocusMagic);
          i = menuFocus.size()+1;
        }
        break;
      case kFocusMain:
        if(mainArea.contains(Pos)) {
          setFocus(kFocusMain);
          i = menuFocus.size()+1;
        }
        break;
      default:
        setFocus(kFocusNone);
        break;
    }
  }    
  itemsScroller.setActive(menuFocus.front()==kFocusItems);
  magicScroller.setActive(menuFocus.front()==kFocusMagic);  
	if(menuFocus.front() != kFocusNone) {
		switch (menuFocus.front()) {
		  case kFocusItems:
			  if(enableFlags.get(kItemsMenu)) {
				  int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
				  if(itemCount == 0)
					  itemCount = 20;
				  else if(itemCount > 50)
					  itemCount = 50;					  
				  int lastItem = mouseOnItem;
			    mouseOnItem = mouseOverItem(Pos, itemCount);
				  if(lastItem != mouseOnItem)
					  if(_engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + mouseOnItem) || _engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + lastItem))
						  redraw = true;
			  }
			  break;
		  case kFocusMagic:
			  if(enableFlags.get(kMagicMenu)) {
				  int lastItem = mouseOnItem;
			    mouseOnItem = mouseOverMagic(Pos);
				  if(lastItem != mouseOnItem)
					  if(_engine->getScriptManager()->getStateValue(StateKey_Spell_1 + mouseOnItem) || _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + lastItem))
						  redraw = true;
			  }
			  break;
		  case kFocusMain:
		    break;
    }
	} 
  MenuHandler::onMouseMove(Pos);
}

int MenuZGI::mouseOverItem(const Common::Point &Pos, int itemCount) {
  int itemWidth = (wSideMenu - 28) / itemCount;
  Common::Rect itemHotspot = Common::Rect(28,hSideMenu);
  itemHotspot.moveTo(itemsOrigin + itemsScroller.Pos);
  for (int i = 0; i < itemCount; i++) {
	  if(itemHotspot.contains(Pos))
      return i;
	  itemHotspot.translate(itemWidth,0);
  }
  return -1;
};

int MenuZGI::mouseOverMagic(const Common::Point &Pos) {
  Common::Rect magicHotspot(28,hSideMenu);
  magicHotspot.moveTo(magicOrigin + magicScroller.Pos);
  magicHotspot.translate(28,0); //Offset from end of menu
  for (int i = 0; i < 12; i++) {
    if(magicHotspot.contains(Pos))
      return i;
  magicHotspot.translate(magicWidth,0);
  }
  return -1;
};

void MenuZGI::process(uint32 deltatime) {
	if(itemsScroller.update(deltatime)) {
	  itemsArea.moveTo(itemsOrigin+itemsScroller.Pos);
	  redraw = true;
  }
  if(magicScroller.update(deltatime)) {
	  magicArea.moveTo(magicOrigin+magicScroller.Pos);
	  redraw = true;
  }
  MenuHandler::process(deltatime);
}

void MenuZGI::redrawAll() {
  if(MenuHandler::inMenu())
    for(int8 i=menuFocus.size()-1; i>=0; i--)
      switch (menuFocus[i]) {
        case kFocusItems:
          if(enableFlags.get(kItemsMenu)) {
            redrawItems();
          }
    	    break;
        case kFocusMagic:
          if(enableFlags.get(kMagicMenu)) {
            redrawMagic();
          }
	        break;
        case kFocusMain:
          redrawMain(); 
	        break;
        default:
	        break;
      }
};

void MenuZGI::redrawMagic() {
  const int16 yOrigin = _menuArea.width();
  _engine->getRenderManager()->blitSurfaceToMenu(menuBack[kFocusMagic], yOrigin+magicScroller.Pos.x, 0, 0);
  for (int i = 0; i < 12; i++) {
    bool inrect = false;
    if(mouseOnItem == i)
	    inrect = true;
    uint curItemId = _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + i);
    if(curItemId) {
	    if(_engine->getScriptManager()->getStateValue(StateKey_Reversed_Spellbooc) == 1)
		    curItemId = 0xEE + i;
	    else
		    curItemId = 0xE0 + i;
    }
    if(curItemId != 0) {
	    if(itemId[i] != curItemId) {
		    char buf[16];
		    Common::sprintf_s(buf, "gmzwu%2.2x1.tga", curItemId);
		    magic[i][0] = _engine->getRenderManager()->loadImage(buf, false);
		    Common::sprintf_s(buf, "gmzxu%2.2x1.tga", curItemId);
		    magic[i][1] = _engine->getRenderManager()->loadImage(buf, false);
		    magicId[i] = curItemId;
	    }
	    if(inrect)
		    _engine->getRenderManager()->blitSurfaceToMenu(*magic[i][1], yOrigin + magicScroller.Pos.x + 28 + magicWidth*i, 0, 0);
	    else
		    _engine->getRenderManager()->blitSurfaceToMenu(*magic[i][0], yOrigin + magicScroller.Pos.x + 28 + magicWidth*i, 0, 0);
    } 
    else {
	    if(magic[i][0]) {
		    magic[i][0]->free();
		    delete magic[i][0];
		    magic[i][0] = NULL;
	    }
	    if(magic[i][1]) {
		    magic[i][1]->free();
		    delete magic[i][1];
		    magic[i][1] = NULL;
	    }
	    magicId[i] = 0;
    }
  }
  clean = false;
}

void MenuZGI::redrawItems() {
  _engine->getRenderManager()->blitSurfaceToMenu(menuBack[kFocusItems], itemsScroller.Pos.x, 0, 0);
  int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
  if(itemCount == 0)
    itemCount = 20;
  else if(itemCount > 50)
    itemCount = 50;
  int itemWidth = (wSideMenu - 28) / itemCount;

  for (int i = 0; i < itemCount; i++) {
    bool inrect = false;
    if(mouseOnItem == i)
	    inrect = true;
    uint curItemId = _engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + i);

    if(curItemId != 0) {
	    if(itemId[i] != curItemId) {
		    char buf[16];
		    Common::sprintf_s(buf, "gmzwu%2.2x1.tga", curItemId);
		    items[i][0] = _engine->getRenderManager()->loadImage(buf, false);
		    Common::sprintf_s(buf, "gmzxu%2.2x1.tga", curItemId);
		    items[i][1] = _engine->getRenderManager()->loadImage(buf, false);
		    itemId[i] = curItemId;
	    }
	    if(inrect)
		    _engine->getRenderManager()->blitSurfaceToMenu(*items[i][1], itemsScroller.Pos.x + itemWidth*i, 0, 0);
	    else
		    _engine->getRenderManager()->blitSurfaceToMenu(*items[i][0], itemsScroller.Pos.x + itemWidth*i, 0, 0);
    } else {
	    if(items[i][0]) {
		    items[i][0]->free();
		    delete items[i][0];
		    items[i][0] = NULL;
	    }
	    if(items[i][1]) {
		    items[i][1]->free();
		    delete items[i][1];
		    items[i][1] = NULL;
	    }
	    itemId[i] = 0;
    }
  }
  clean = false;
}

MenuNemesis::MenuNemesis(ZVision *engine, const Common::Rect menuArea) :
	MenuHandler(engine, menuArea, nemesisParams) {

  //Buffer menu background image
	_engine->getRenderManager()->readImageToSurface("bar.tga", mainBack, false);
	
	char buf[24];
	for (int i = 0; i < 4; i++) {
    //Buffer menu buttons
		for (int j = 0; j < 6; j++) {
			Common::sprintf_s(buf, "butfrm%d%d.tga", i + 1, j);
			_engine->getRenderManager()->readImageToSurface(buf, mainButtons[i][j], false);
		}
	}
}

MenuNemesis::~MenuNemesis() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 6; j++)
			mainButtons[i][j].free();
	};
}

bool MenuNemesis::inMenu(const Common::Point &Pos) {
  return menuTriggerArea.contains(Pos) || (menuFocus.front() != kFocusNone);
}

void MenuNemesis::onMouseMove(const Common::Point &Pos) {
  //Trigger main menu scrolldown to get mouse over main trigger area
  //Set focus to topmost layer of menus that mouse is currently over
  if(mainArea.contains(Pos) || menuTriggerArea.contains(Pos))
    setFocus(kFocusMain);
  else
    setFocus(kFocusNone);
  MenuHandler::onMouseMove(Pos);
}

} // End of namespace ZVision
