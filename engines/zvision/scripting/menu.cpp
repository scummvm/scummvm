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

#include "common/config-manager.h"
#include "zvision/detection.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/menu.h"

namespace ZVision {

enum {
	kFocusNone = -1,
	kFocusItems = 0,
	kFocusMagic = 1,
	kFocusMain = 2
};

MenuManager::MenuManager(ZVision *engine, const Common::Rect menuArea, const MenuParams params) :
	_engine(engine),
	_params(params),
	_menuBarFlag(0xFFFF),
	_menuArea(menuArea),
	_menuOrigin(menuArea.origin()),
	_menuTriggerArea(_menuOrigin, _menuArea.width(), _params.triggerHeight),
	_mainScroller(params.activePos, params.idlePos, params.period) {

	_enableFlags.set_size(6);
	for (int8 i = 0; i < 4; i++) {
		// Generate button hotspot areas
		_menuHotspots[i] = Common::Rect(_params.wxButs[i][1], _menuArea.top, _params.wxButs[i][1] + _params.wxButs[i][0], _menuArea.bottom);
		// Initialise button animation frames
		_mainFrames[i] = _params.idleFrame;
	}
	for (int i = 0; i < 4; i++)
		_buttonAnim[i] = new LinearScroller(_params.activeFrame, _params.idleFrame, _params.buttonPeriod);
	setFocus(kFocusNone); // Ensure focus list is initialised
	_mainArea = Common::Rect(_params.wMain, _hMainMenu);
	_mainArea.moveTo(_menuOrigin + _mainScroller._pos);
}

MenuManager::~MenuManager() {
	for (int i = 0; i < 4; i++)
		delete _buttonAnim[i];
	_mainBack.free();
}

void MenuManager::setEnable(uint16 flags) {
	static const uint16 flagMasks[6] = {0x8, 0x4, 0x2, 0x1, 0x100, 0x200}; // Enum order: save,restore,prefs,quit,items,magic
	_menuBarFlag = flags;
	for (uint i = 0; i <= 5; i++) {
		if (_menuBarFlag & flagMasks[i])
			_enableFlags.set(i);
		else
			_enableFlags.unset(i);
	}
}

void MenuManager::onMouseUp(const Common::Point &pos) {
	if (_menuFocus.front() == kFocusMain) {
		_mouseOnItem = mouseOverMain(pos);
		if (_mouseOnItem == _mainClicked)
			// Activate clicked action from main menu
			switch (_mouseOnItem) {
			case kMainMenuSave:
				_engine->getScriptManager()->changeLocation('g', 'j', 's', 'e', 0);
				setFocus(kFocusNone);
				_mainScroller.reset();
				_redraw = true;
				break;
			case kMainMenuLoad:
				_engine->getScriptManager()->changeLocation('g', 'j', 'r', 'e', 0);
				setFocus(kFocusNone);
				_mainScroller.reset();
				_redraw = true;
				break;
			case kMainMenuPrefs:
				_engine->getScriptManager()->changeLocation('g', 'j', 'p', 'e', 0);
				setFocus(kFocusNone);
				_mainScroller.reset();
				_redraw = true;
				break;
			case kMainMenuExit:
				_engine->quit(true);
				break;
			default:
				break;
			}
	}
	_mainClicked = -1;
}

void MenuManager::onMouseDown(const Common::Point &pos) {
	if (_menuFocus.front() == kFocusMain) {
		_mouseOnItem = mouseOverMain(pos);
		// Show clicked graphic
		if ((_mouseOnItem >= 0) && (_mouseOnItem < 4))
			if (_enableFlags.get(_mouseOnItem)) {
				_mainClicked = _mouseOnItem;
				_redraw = true;
			}
	}
	debugC(kDebugMouse, "mouse position %d %d", pos.x, pos.y);
	debugC(kDebugMouse, "panorama position %d %d", pos.x, pos.y);
}

void MenuManager::onMouseMove(const Common::Point &pos) {
	bool nowInMenu = inMenu(pos);
	if (nowInMenu != _prevInMenu)
		_redraw = true;
	_prevInMenu = nowInMenu;

	int lastItem = _mouseOnItem;

	switch (_menuFocus.front()) {
	case kFocusMain:
		// Inform game scripting engine that mouse is in main menu
		if (_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 2)
			_engine->getScriptManager()->setStateValue(StateKey_MenuState, 2);
		_mouseOnItem = mouseOverMain(pos);
		break;
	case kFocusNone:
		// Inform game scripting engine that mouse is not in any menu
		if (_engine->getScriptManager()->getStateValue(StateKey_MenuState) != 0)
			_engine->getScriptManager()->setStateValue(StateKey_MenuState, 0);
		_mouseOnItem = -1;
		break;
	}
	_mainScroller.setActive(_menuFocus.front() == kFocusMain);
	// Update button animation status
	for (int i = 0; i < 4; i++)
		if (_menuFocus[0] == kFocusMain && _mouseOnItem == i)
			_buttonAnim[i]->setActive(true);
		else
			_buttonAnim[i]->setActive(false);
	if (lastItem != _mouseOnItem)
		_redraw = true;
}

int MenuManager::mouseOverMain(const Common::Point &pos) {
	// Common::Rect mainHotspot(28,_hSideMenu);
	// mainHotspot.moveTo(mainOrigin + _mainScroller._pos);
	for (int8 i = 0; i < 4; i++) {
		if (_enableFlags.get(i) && _menuHotspots[i].contains(pos))
			return i;
	}
	return -1;
}

void MenuManager::process(uint32 deltatime) {
	if (_mainScroller.update(deltatime)) {
		_mainArea.moveTo(_menuOrigin + _mainScroller._pos);
		for (int i = 0; i < 4; i++)
			_menuHotspots[i].moveTo(_menuOrigin + Common::Point(_params.wxButs[i][1], _mainScroller._pos.y));
		_redraw = true;
	}
	// Update button highlight animation frame
	for (int i = 0; i < 4; i++)
		if (_buttonAnim[i]->update(deltatime)) {
			_mainFrames[i] = _buttonAnim[i]->_pos;
			_redraw = true;
		}
	if (_redraw) {
		_engine->getRenderManager()->clearMenuSurface();
		redrawAll();
		_redraw = false;
	}
}

void MenuNemesis::redrawAll() {
	redrawMain();
}

void MenuManager::redrawMain() {
	// Draw menu background
	_engine->getRenderManager()->blitSurfaceToMenu(_mainBack, _mainScroller._pos.x, _mainScroller._pos.y, 0);
	// Draw buttons
	if (_menuFocus.front() == kFocusMain)
		for (int8 i = 0; i < 4; i++) {
			if (_enableFlags.get(i) && (_mainFrames[i] >= 0)) {
				if (_mainClicked == i)
					_engine->getRenderManager()->blitSurfaceToMenu(_mainButtons[i][_params.clickedFrame], _params.wxButs[i][1], _mainScroller._pos.y, 0);
				else
					_engine->getRenderManager()->blitSurfaceToMenu(_mainButtons[i][_mainFrames[i]], _params.wxButs[i][1], _mainScroller._pos.y, 0);
			}
		}
	_clean = false;
}

void MenuManager::setFocus(int8 currentFocus) {
	_menuFocus.set(currentFocus);
	assert(_menuFocus.size() <= 4);
}

MenuZGI::MenuZGI(ZVision *engine, const Common::Rect menuArea) :
	MenuManager(engine, menuArea, zgiParams),
	_itemsScroller(Common::Point(0, 0), Common::Point(_wSideMenuTab - _wSideMenu, 0), _sideMenuPeriod),
	_magicScroller(Common::Point(-_wSideMenu, 0), Common::Point(-_wSideMenuTab, 0), _sideMenuPeriod),
	_itemsOrigin(menuArea.left, menuArea.top),
	_magicOrigin(menuArea.right, menuArea.top) {

	_magicArea = Common::Rect(_magicOrigin + _magicScroller._pos, _wSideMenu, _hSideMenu);
	_itemsArea = Common::Rect(_itemsOrigin + _itemsScroller._pos, _wSideMenu, _hSideMenu);

	// Buffer main menu background
	_engine->getRenderManager()->readImageToSurface("gmzau031.tga", _mainBack, false);

	char buf[24];
	for (int i = 0; i < 4; i++) {
		// Buffer menu buttons
		Common::sprintf_s(buf, "gmzmu%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, _mainButtons[i][0], false);
		Common::sprintf_s(buf, "gmznu%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, _mainButtons[i][1], false);
	}
	for (int i = 1; i < 4; i++) {
		// Buffer full menu backgrounds
		Common::sprintf_s(buf, "gmzau%2.2x1.tga", i);
		_engine->getRenderManager()->readImageToSurface(buf, _menuBack[i - 1], false);
	}
	for (int i = 0; i < 50; i++) {
		_items[i][0] = NULL;
		_items[i][1] = NULL;
		_itemId[i] = 0;
	}
	for (int i = 0; i < 12; i++) {
		_magic[i][0] = NULL;
		_magic[i][1] = NULL;
		_magicId[i] = 0;
	}
	// Initialise focus sequence
	setFocus(kFocusMain);
	setFocus(kFocusMagic);
	setFocus(kFocusItems);
	setFocus(kFocusNone);
}

MenuZGI::~MenuZGI() {
	for (int i = 0; i < 3; i++) {
		_menuBack[i].free();
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 2; j++)
			_mainButtons[i][j].free();
	}
	for (int i = 0; i < 50; i++) {
		if (_items[i][0]) {
			_items[i][0]->free();
			delete _items[i][0];
		}
		if (_items[i][1]) {
			_items[i][1]->free();
			delete _items[i][1];
		}
	}
	for (int i = 0; i < 12; i++) {
		if (_magic[i][0]) {
			_magic[i][0]->free();
			delete _magic[i][0];
		}
		if (_magic[i][1]) {
			_magic[i][1]->free();
			delete _magic[i][1];
		}
	}
}

bool MenuZGI::inMenu(const Common::Point &pos) const {
	return _menuTriggerArea.contains(pos) || (_menuFocus.front() != kFocusNone);
}

void MenuZGI::onMouseUp(const Common::Point &pos) {
	if (inMenu(pos))
		// _redraw = true;
		switch (_menuFocus.front()) {
		case kFocusItems:
			if (_enableFlags.get(kItemsMenu)) {
				int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
				if (itemCount == 0)
					itemCount = 20;
				int i = mouseOverItem(pos, itemCount);
				if (i != -1) {
					int32 mouseItem = _engine->getScriptManager()->getStateValue(StateKey_InventoryItem);
					if (mouseItem >= 0  && mouseItem < 0xE0) {
						_engine->getScriptManager()->inventoryDrop(mouseItem);
						_engine->getScriptManager()->inventoryAdd(_engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + i));
						_engine->getScriptManager()->setStateValue(StateKey_Inv_StartSlot + i, mouseItem);
						_redraw = true;
					}
				}
			}
			break;

		case kFocusMagic:
			if (_enableFlags.get(kMagicMenu)) {
				int i = mouseOverMagic(pos);
				if (i != -1) {

					uint itemnum = _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + i);
					if (itemnum != 0) {
						if (_engine->getScriptManager()->getStateValue(StateKey_Reversed_Spellbooc) == 1)
							itemnum = 0xEE + i;
						else
							itemnum = 0xE0 + i;
					}
					if (itemnum)
						if (_engine->getScriptManager()->getStateValue(StateKey_InventoryItem) == 0 || _engine->getScriptManager()->getStateValue(StateKey_InventoryItem) >= 0xE0)
							_engine->getScriptManager()->setStateValue(StateKey_Active_Spell, itemnum);
				}

			}
			break;

		case kFocusMain:
			MenuManager::onMouseUp(pos);
			break;

		default:
			break;
		}
}

void MenuZGI::onMouseMove(const Common::Point &pos) {
	if (!inMenu(pos)) {
		_mainScroller.reset();
		_magicScroller.reset();
		_itemsScroller.reset();
	}

	// Set focus to topmost layer of menus that mouse is currently over
	for (uint8 i = 0; i < _menuFocus.size(); i++) {
		switch (_menuFocus[i]) {
		case kFocusItems:
			if (_itemsArea.contains(pos)) {
				setFocus(kFocusItems);
				i = _menuFocus.size() + 1;
			}
			break;
		case kFocusMagic:
			if (_magicArea.contains(pos)) {
				setFocus(kFocusMagic);
				i = _menuFocus.size() + 1;
			}
			break;
		case kFocusMain:
			if (_mainArea.contains(pos)) {
				setFocus(kFocusMain);
				i = _menuFocus.size() + 1;
			}
			break;
		default:
			setFocus(kFocusNone);
			break;
		}
	}
	_itemsScroller.setActive(_menuFocus.front() == kFocusItems);
	_magicScroller.setActive(_menuFocus.front() == kFocusMagic);
	if (_menuFocus.front() != kFocusNone) {
		switch (_menuFocus.front()) {
		case kFocusItems:
			if (_enableFlags.get(kItemsMenu)) {
				int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
				if (itemCount == 0)
					itemCount = 20;
				else if (itemCount > 50)
					itemCount = 50;
				int lastItem = _mouseOnItem;
				_mouseOnItem = mouseOverItem(pos, itemCount);
				if (lastItem != _mouseOnItem)
					if (_engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + _mouseOnItem) || _engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + lastItem))
						_redraw = true;
			}
			break;

		case kFocusMagic:
			if (_enableFlags.get(kMagicMenu)) {
				int lastItem = _mouseOnItem;
				_mouseOnItem = mouseOverMagic(pos);

				if (lastItem != _mouseOnItem)
					if (_engine->getScriptManager()->getStateValue(StateKey_Spell_1 + _mouseOnItem) || _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + lastItem))
						_redraw = true;

			}
			break;
		case kFocusMain:
			break;
		}
	}
	MenuManager::onMouseMove(pos);
}

int MenuZGI::mouseOverItem(const Common::Point &pos, int itemCount) {
	int itemWidth = (_wSideMenu - 28) / itemCount;
	Common::Rect itemHotspot = Common::Rect(28, _hSideMenu);
	itemHotspot.moveTo(_itemsOrigin + _itemsScroller._pos);
	for (int i = 0; i < itemCount; i++) {
		if (itemHotspot.contains(pos))
			return i;
		itemHotspot.translate(itemWidth, 0);
	}
	return -1;
}

int MenuZGI::mouseOverMagic(const Common::Point &pos) {
	Common::Rect magicHotspot(28, _hSideMenu);
	magicHotspot.moveTo(_magicOrigin + _magicScroller._pos);
	magicHotspot.translate(28, 0); // Offset from end of menu
	for (int i = 0; i < 12; i++) {
		if (magicHotspot.contains(pos))
			return i;
		magicHotspot.translate(_magicWidth, 0);
	}
	return -1;
}

void MenuZGI::process(uint32 deltatime) {
	if (_itemsScroller.update(deltatime)) {
		_itemsArea.moveTo(_itemsOrigin + _itemsScroller._pos);
		_redraw = true;
	}
	if (_magicScroller.update(deltatime)) {
		_magicArea.moveTo(_magicOrigin + _magicScroller._pos);
		_redraw = true;
	}
	MenuManager::process(deltatime);
}

void MenuZGI::redrawAll() {
	if (MenuManager::inMenu())
		for (int8 i = _menuFocus.size() - 1; i >= 0; i--)
			switch (_menuFocus[i]) {
			case kFocusItems:
				if (_enableFlags.get(kItemsMenu)) {
					redrawItems();
				}
				break;
			case kFocusMagic:
				if (_enableFlags.get(kMagicMenu)) {
					redrawMagic();
				}
				break;
			case kFocusMain:
				redrawMain();
				break;
			default:
				break;
			}
}

void MenuZGI::redrawMagic() {
	const int16 yOrigin = _menuArea.width();
	_engine->getRenderManager()->blitSurfaceToMenu(_menuBack[kFocusMagic], yOrigin + _magicScroller._pos.x, 0, 0);
	for (int i = 0; i < 12; i++) {
		bool inrect = false;
		if (_mouseOnItem == i)
			inrect = true;
		uint curItemId = _engine->getScriptManager()->getStateValue(StateKey_Spell_1 + i);
		if (curItemId) {
			if (_engine->getScriptManager()->getStateValue(StateKey_Reversed_Spellbooc) == 1)
				curItemId = 0xEE + i;
			else
				curItemId = 0xE0 + i;
		}
		if (curItemId != 0) {
			if (_itemId[i] != curItemId) {
				char buf[16];
				Common::sprintf_s(buf, "gmzwu%2.2x1.tga", curItemId);
				_magic[i][0] = _engine->getRenderManager()->loadImage(buf, false);
				Common::sprintf_s(buf, "gmzxu%2.2x1.tga", curItemId);
				_magic[i][1] = _engine->getRenderManager()->loadImage(buf, false);
				_magicId[i] = curItemId;
			}
			if (inrect)
				_engine->getRenderManager()->blitSurfaceToMenu(*_magic[i][1], yOrigin + _magicScroller._pos.x + 28 + _magicWidth * i, 0, 0);
			else
				_engine->getRenderManager()->blitSurfaceToMenu(*_magic[i][0], yOrigin + _magicScroller._pos.x + 28 + _magicWidth * i, 0, 0);
		} else {
			if (_magic[i][0]) {
				_magic[i][0]->free();
				delete _magic[i][0];
				_magic[i][0] = NULL;
			}
			if (_magic[i][1]) {
				_magic[i][1]->free();
				delete _magic[i][1];
				_magic[i][1] = NULL;
			}
			_magicId[i] = 0;
		}
	}
	_clean = false;
}

void MenuZGI::redrawItems() {
	_engine->getRenderManager()->blitSurfaceToMenu(_menuBack[kFocusItems], _itemsScroller._pos.x, 0, 0);
	int itemCount = _engine->getScriptManager()->getStateValue(StateKey_Inv_TotalSlots);
	if (itemCount == 0)
		itemCount = 20;
	else if (itemCount > 50)
		itemCount = 50;
	int itemWidth = (_wSideMenu - 28) / itemCount;

	for (int i = 0; i < itemCount; i++) {
		bool inrect = false;
		if (_mouseOnItem == i)
			inrect = true;
		uint curItemId = _engine->getScriptManager()->getStateValue(StateKey_Inv_StartSlot + i);

		if (curItemId != 0) {
			if (_itemId[i] != curItemId) {
				char buf[16];
				Common::sprintf_s(buf, "gmzwu%2.2x1.tga", curItemId);
				_items[i][0] = _engine->getRenderManager()->loadImage(buf, false);
				Common::sprintf_s(buf, "gmzxu%2.2x1.tga", curItemId);
				_items[i][1] = _engine->getRenderManager()->loadImage(buf, false);
				_itemId[i] = curItemId;
			}
			if (inrect)
				_engine->getRenderManager()->blitSurfaceToMenu(*_items[i][1], _itemsScroller._pos.x + itemWidth * i, 0, 0);
			else
				_engine->getRenderManager()->blitSurfaceToMenu(*_items[i][0], _itemsScroller._pos.x + itemWidth * i, 0, 0);
		} else {
			if (_items[i][0]) {
				_items[i][0]->free();
				delete _items[i][0];
				_items[i][0] = NULL;
			}
			if (_items[i][1]) {
				_items[i][1]->free();
				delete _items[i][1];
				_items[i][1] = NULL;
			}
			_itemId[i] = 0;
		}
	}
	_clean = false;
}

MenuNemesis::MenuNemesis(ZVision *engine, const Common::Rect menuArea) :
	MenuManager(engine, menuArea, nemesisParams) {

	// Buffer menu background image
	_engine->getRenderManager()->readImageToSurface("bar.tga", _mainBack, false);

	char buf[24];
	for (int i = 0; i < 4; i++)
		// Buffer menu buttons
		for (int j = 0; j < 6; j++) {
			Common::sprintf_s(buf, "butfrm%d%d.tga", i + 1, j);
			_engine->getRenderManager()->readImageToSurface(buf, _mainButtons[i][j], false);
		}

}

MenuNemesis::~MenuNemesis() {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 6; j++)
			_mainButtons[i][j].free();

}

bool MenuNemesis::inMenu(const Common::Point &pos) const {
	return _menuTriggerArea.contains(pos) || (_menuFocus.front() != kFocusNone);
}

void MenuNemesis::onMouseMove(const Common::Point &pos) {
	// Trigger main menu scrolldown to get mouse over main trigger area
	// Set focus to topmost layer of menus that mouse is currently over
	if (_mainArea.contains(pos) || _menuTriggerArea.contains(pos))
		setFocus(kFocusMain);
	else
		setFocus(kFocusNone);
	MenuManager::onMouseMove(pos);
}

} // End of namespace ZVision
