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
 */

#include "illusions/menusystem.h"
#include "illusions/illusions.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/screen.h"
#include "illusions/screentext.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "gui/saveload.h"

namespace Illusions {

// MenuItem

MenuItem::MenuItem(const Common::String text, BaseMenuAction *action)
	: _text(text), _action(action) {
}

MenuItem::~MenuItem() {
	delete _action;
}

void MenuItem::executeAction(const Common::Point &point) {
	_mouseClickPoint = point;
	_action->execute();
}

// BaseMenu

BaseMenu::BaseMenu(BaseMenuSystem *menuSystem, uint32 fontId, byte backgroundColor, byte borderColor, byte textColor, byte fieldE,
	uint defaultMenuItemIndex)
	: _menuSystem(menuSystem), _fontId(fontId), _backgroundColor(backgroundColor), _borderColor(borderColor), _textColor(textColor), _fieldE(fieldE),
	_defaultMenuItemIndex(defaultMenuItemIndex)
{
}

BaseMenu::~BaseMenu() {
	for (MenuItems::iterator it = _menuItems.begin(); it != _menuItems.end(); ++it) {
		delete *it;
	}
}

void BaseMenu::addText(const Common::String text) {
	_text.push_back(text);
}

void BaseMenu::addMenuItem(MenuItem *menuItem) {
	_menuItems.push_back(menuItem);
}

uint BaseMenu::getHeaderLinesCount() {
	return _text.size();
}

const Common::String& BaseMenu::getHeaderLine(uint index) {
	return _text[index];
}

uint BaseMenu::getMenuItemsCount() {
	return _menuItems.size();
}

MenuItem *BaseMenu::getMenuItem(uint index) {
	return _menuItems[index];
}

void BaseMenu::enterMenu() {
	// Empty, implemented if neccessary by the inherited class when the menu is entered
}

// BaseMenuSystem

BaseMenuSystem::BaseMenuSystem(IllusionsEngine *vm)
	: _vm(vm), _isTimeOutEnabled(false), _menuChoiceOffset(0) {
}

BaseMenuSystem::~BaseMenuSystem() {
}

void BaseMenuSystem::playSoundEffect13() {
	playSoundEffect(13);
}

void BaseMenuSystem::playSoundEffect14() {
	playSoundEffect(14);
}

void BaseMenuSystem::selectMenuChoiceIndex(uint choiceIndex) {
	debug(0, "choiceIndex: %d", choiceIndex);
	debug(0, "_menuChoiceOffset: %p", (void*)_menuChoiceOffset);
	if (choiceIndex > 0 && _menuChoiceOffset) {
		*_menuChoiceOffset = _menuChoiceOffsets[choiceIndex - 1];
		debug(0, "*_menuChoiceOffset: %04X", *_menuChoiceOffset);
	}
	_vm->_threads->notifyId(_menuCallerThreadId);
	_menuCallerThreadId = 0;
	closeMenu();
}

void BaseMenuSystem::leaveMenu() {
	playSoundEffect13();
	if (!_menuStack.empty())
		leaveSubMenu();
	else
		closeMenu();
}

void BaseMenuSystem::enterSubMenu(BaseMenu *menu) {
	_menuStack.push(_activeMenu);
	activateMenu(menu);
	_hoveredMenuItemIndex = _hoveredMenuItemIndex3;
	_hoveredMenuItemIndex2 = _hoveredMenuItemIndex3;
	setMouseCursorToMenuItem(_hoveredMenuItemIndex);
	placeActorHoverBackground();
	placeActorTextColorRect();
}

void BaseMenuSystem::leaveSubMenu() {
	_activeMenu = _menuStack.pop();
	_field54 = _activeMenu->_field2C18;
	_menuLinesCount = _activeMenu->getHeaderLinesCount();
	_hoveredMenuItemIndex = 1;
	_vm->_screenText->removeText();
	_vm->_screenText->removeText();
	activateMenu(_activeMenu);
	_hoveredMenuItemIndex = _hoveredMenuItemIndex3;
	_hoveredMenuItemIndex2 = _hoveredMenuItemIndex3;
	setMouseCursorToMenuItem(_hoveredMenuItemIndex);
	initActorHoverBackground();
	placeActorTextColorRect();
}

void BaseMenuSystem::enterSubMenuById(int menuId) {
	BaseMenu *menu = getMenuById(menuId);
	enterSubMenu(menu);
}

uint BaseMenuSystem::getQueryConfirmationChoiceIndex() const {
	return _queryConfirmationChoiceIndex;
}

void BaseMenuSystem::setQueryConfirmationChoiceIndex(uint queryConfirmationChoiceIndex) {
	_queryConfirmationChoiceIndex = queryConfirmationChoiceIndex;
}

void BaseMenuSystem::setMouseCursorToMenuItem(int menuItemIndex) {
	Common::Point mousePos;
	if (calcMenuItemMousePos(menuItemIndex, mousePos))
		setMousePos(mousePos);
}

void BaseMenuSystem::calcMenuItemRect(uint menuItemIndex, WRect &rect) {
	FontResource *font = _vm->_dict->findFont(_activeMenu->_fontId);
	int charHeight = font->getCharHeight() + font->getLineIncr();

	_vm->_screenText->getTextInfoPosition(rect._topLeft);
	if (_activeMenu->_backgroundColor) {
		rect._topLeft.y += 4;
		rect._topLeft.x += 4;
	}
	rect._topLeft.y += charHeight * (menuItemIndex + _menuLinesCount - 1);

	WidthHeight textInfoDimensions;
	_vm->_screenText->getTextInfoDimensions(textInfoDimensions);
	rect._bottomRight.x = rect._topLeft.x + textInfoDimensions._width;
	rect._bottomRight.y = rect._topLeft.y + charHeight;
}

bool BaseMenuSystem::calcMenuItemMousePos(uint menuItemIndex, Common::Point &pt) {
	if (menuItemIndex < _hoveredMenuItemIndex3 || menuItemIndex >= _hoveredMenuItemIndex3 + _menuItemCount)
		return false;

	WRect rect;
	calcMenuItemRect(menuItemIndex - _hoveredMenuItemIndex3 + 1, rect);
	pt.x = rect._topLeft.x;
	pt.y = rect._topLeft.y + (rect._bottomRight.y - rect._topLeft.y) / 2;
	return true;
}

bool BaseMenuSystem::calcMenuItemIndexAtPoint(Common::Point pt, uint &menuItemIndex) {
	WRect rect;
	calcMenuItemRect(1, rect);

	uint index = _hoveredMenuItemIndex3 + (pt.y - rect._topLeft.y) / (rect._bottomRight.y - rect._topLeft.y);

	if (pt.y < rect._topLeft.y || pt.x < rect._topLeft.x || pt.x > rect._bottomRight.x ||
		index > _field54 || index > _hoveredMenuItemIndex3 + _menuItemCount - 1)
		return false;

	menuItemIndex = index;
	return true;
}

void BaseMenuSystem::setMousePos(Common::Point &mousePos) {
	_vm->_input->setCursorPosition(mousePos);
	Control *mouseCursor = _vm->getObjectControl(Illusions::CURSOR_OBJECT_ID);
	mouseCursor->_actor->_position = mousePos;
}

void BaseMenuSystem::activateMenu(BaseMenu *menu) {
	_activeMenu = menu;
	// TODO Run menu enter callback if neccessary
	_menuLinesCount = menu->getHeaderLinesCount();
	menu->_field2C18 = menu->getMenuItemsCount();
	_hoveredMenuItemIndex3 = 1;
	_field54 = menu->_field2C18;

	uint v2 = drawMenuText(menu);
	if (menu->_field2C18 <= v2)
		_menuItemCount = menu->_field2C18;
	else
		_menuItemCount = v2;

}

void BaseMenuSystem::initActorHoverBackground() {
	Control *v0 = _vm->getObjectControl(0x4013E);
	if (!v0) {
		WidthHeight dimensions;
		dimensions._width = 300;
		dimensions._height = 15;
		if (_vm->getGameId() == kGameIdBBDOU) {
			_vm->_controls->placeSequenceLessActor(0x4013E, Common::Point(0, 0), dimensions, 91);
		} else {
			_vm->_controls->placeSequenceLessActor(0x4013E, Common::Point(0, 0), dimensions, 18);
		}
		v0 = _vm->getObjectControl(0x4013E);
		v0->_flags |= 8;
	}
	placeActorHoverBackground();
	v0->appearActor();
}

void BaseMenuSystem::placeActorHoverBackground() {
	Control *v0 = _vm->getObjectControl(0x4013E);
	v0->fillActor(0);

	WidthHeight textInfoDimensions;
	_vm->_screenText->getTextInfoDimensions(textInfoDimensions);

	if (_activeMenu->_backgroundColor && _activeMenu->_borderColor != _activeMenu->_backgroundColor)
		textInfoDimensions._width -= 6;

	WidthHeight frameDimensions;
	v0->getActorFrameDimensions(frameDimensions);

	FontResource *font = _vm->_dict->findFont(_activeMenu->_fontId);
	int charHeight = font->getCharHeight() + font->getLineIncr();
	if (frameDimensions._height < charHeight)
		charHeight = frameDimensions._height;

	v0->drawActorRect(Common::Rect(textInfoDimensions._width - 1, charHeight), _activeMenu->_fieldE);

	updateActorHoverBackground();
}

void BaseMenuSystem::updateActorHoverBackground() {
	Control *v0 = _vm->getObjectControl(0x4013E);
	WRect rect;
	calcMenuItemRect(_hoveredMenuItemIndex2 - _hoveredMenuItemIndex3 + 1, rect);
  	v0->setActorPosition(rect._topLeft);
}

void BaseMenuSystem::hideActorHoverBackground() {
	Control *v0 = _vm->getObjectControl(0x4013E);
	if (v0)
		v0->disappearActor();
}

void BaseMenuSystem::initActorTextColorRect() {
	Control *v0 = _vm->getObjectControl(0x40143);
	if (!v0) {
		WidthHeight dimensions;
		if (_vm->getGameId() == kGameIdBBDOU) {
			dimensions._width = 420;
			dimensions._height = 180;
			_vm->_controls->placeSequenceLessActor(0x40143, Common::Point(0, 0), dimensions, 90);
		} else {
			dimensions._width = 300;
			dimensions._height = 180;
			_vm->_controls->placeSequenceLessActor(0x40143, Common::Point(0, 0), dimensions, 17);
		}
		v0 = _vm->getObjectControl(0x40143);
		v0->_flags |= 8;
	}
	placeActorTextColorRect();
	v0->appearActor();
}

void BaseMenuSystem::placeActorTextColorRect() {
	Control *v0 = _vm->getObjectControl(0x40143);
	v0->fillActor(0);

	Common::Point textInfoPosition;
	WidthHeight textInfoDimensions;
	_vm->_screenText->getTextInfoPosition(textInfoPosition);
	_vm->_screenText->getTextInfoDimensions(textInfoDimensions);

	if (_activeMenu->_backgroundColor && _activeMenu->_borderColor != _activeMenu->_backgroundColor) {
		textInfoDimensions._width -= 2;
		textInfoDimensions._height -= 6;
	}

	v0->setActorPosition(textInfoPosition);
	v0->drawActorRect(Common::Rect(textInfoDimensions._width - 1, textInfoDimensions._height - 1), _activeMenu->_textColor);
}

void BaseMenuSystem::hideActorTextColorRect() {
	Control *v0 = _vm->getObjectControl(0x40143);
	if (v0)
		v0->disappearActor();
}

void BaseMenuSystem::openMenu(BaseMenu *menu) {

	_isActive = true;
	_menuStack.clear();

	_cursorInitialVisibleFlag = initMenuCursor();
	_savedCursorPos = _vm->_input->getCursorPosition();
	_savedGameState = getGameState();
	Control *cursorControl = _vm->getObjectControl(Illusions::CURSOR_OBJECT_ID);
	_savedCursorActorIndex = cursorControl->_actor->_actorIndex;
	_savedCursorSequenceId = cursorControl->_actor->_sequenceId;

	setMenuCursorNum(1);

	if (_vm->getGameId() == kGameIdDuckman) {
		setGameState(4);
	} else if (_vm->getGameId() == kGameIdBBDOU) {
		setGameState(3);
	}

	activateMenu(menu);

	_hoveredMenuItemIndex = _hoveredMenuItemIndex3;
	_hoveredMenuItemIndex2 = _hoveredMenuItemIndex3;
	setMouseCursorToMenuItem(_hoveredMenuItemIndex);
	initActorHoverBackground();
	initActorTextColorRect();
	_vm->_input->discardAllEvents();
}

void BaseMenuSystem::closeMenu() {
	while (!_menuStack.empty()) {
		_vm->_screenText->removeText();
		_menuStack.pop();
	}
	_vm->_screenText->removeText();
	hideActorHoverBackground();
	hideActorTextColorRect();
	Control *mouseCursor = _vm->getObjectControl(Illusions::CURSOR_OBJECT_ID);
	setGameState(_savedGameState);
	mouseCursor->_actor->_actorIndex = _savedCursorActorIndex;
	mouseCursor->_actor->_position = _savedCursorPos;
	setMousePos(_savedCursorPos);
	mouseCursor->startSequenceActor(_savedCursorSequenceId, 2, 0);
	if (_cursorInitialVisibleFlag)
		mouseCursor->disappearActor();
	_vm->_input->discardAllEvents();
	_isActive = false;
}

void BaseMenuSystem::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	debug(0, "BaseMenuSystem::handleClick() menuItemIndex: %d click point: (%d, %d)", menuItemIndex, mousePos.x, mousePos.y);

	if (menuItemIndex == 0) {
		playSoundEffect14();
		return;
	}

	MenuItem *menuItem = _activeMenu->getMenuItem(menuItemIndex - 1);
	menuItem->executeAction(mousePos);

}

uint BaseMenuSystem::drawMenuText(BaseMenu *menu) {
	MenuTextBuilder *menuTextBuilder = new MenuTextBuilder();
	uint lineCount = 0;

	for (uint i = 0; i < menu->getHeaderLinesCount(); ++i) {
		menuTextBuilder->appendString(menu->getHeaderLine(i));
		menuTextBuilder->appendNewLine();
	}

	for (uint i = _hoveredMenuItemIndex3; i <= _field54; ++i) {
		menuTextBuilder->appendString(menu->getMenuItem(i - 1)->getText());
		if (i + 1 <= menu->getMenuItemsCount())
			menuTextBuilder->appendNewLine();
		++lineCount;
	}

	menuTextBuilder->finalize();

	uint16 *text = menuTextBuilder->getText();

	Common::Point textPt;
	int16 v9 = 0;
	if (menu->_backgroundColor)
		v9 = 4;
	textPt.x = v9;
	textPt.y = v9;

	uint flags = TEXT_FLAG_LEFT_ALIGN;
	if (menu->_backgroundColor != menu->_borderColor)
		flags |= TEXT_FLAG_BORDER_DECORATION;

	WidthHeight dimensions;

	if (_vm->getGameId() == kGameIdDuckman) {
		dimensions._width = 300;
		dimensions._height = 180;
	} else if (_vm->getGameId() == kGameIdBBDOU) {
		dimensions._width = 580;
		dimensions._height = 420;
	}

	uint16 *outTextPtr;
	if (!_vm->_screenText->insertText(text, menu->_fontId, dimensions, textPt, flags, menu->_backgroundColor, menu->_borderColor, 0xFF, 0xFF, 0xFF, outTextPtr)) {
		--lineCount;
		for ( ; *outTextPtr; ++outTextPtr) {
			if (*outTextPtr == 13)
				--lineCount;
		}
	}

	delete menuTextBuilder;

	return lineCount;
}

void BaseMenuSystem::update(Control *cursorControl) {
	Common::Point mousePos = _vm->_input->getCursorPosition();
	setMousePos(mousePos);

	uint newHoveredMenuItemIndex;
	bool resetTimeOut = false;

	if (calcMenuItemIndexAtPoint(mousePos, newHoveredMenuItemIndex)) {
		if (newHoveredMenuItemIndex != _hoveredMenuItemIndex) {
			if (_hoveredMenuItemIndex == 0)
				initActorHoverBackground();
			_hoveredMenuItemIndex = newHoveredMenuItemIndex;
			_hoveredMenuItemIndex2 = newHoveredMenuItemIndex;
			setMenuCursorNum(2);
			updateActorHoverBackground();
			resetTimeOut = true;
		}
	} else if (_hoveredMenuItemIndex != 0) {
		setMenuCursorNum(1);
		hideActorHoverBackground();
		_hoveredMenuItemIndex = 0;
		resetTimeOut = true;
	}

	if (_vm->_input->hasNewEvents())
		resetTimeOut = true;

	if (_vm->_input->pollEvent(kEventLeftClick)) {
		handleClick(_hoveredMenuItemIndex, mousePos);
	} else if (_vm->_input->pollEvent(kEventAbort) && _activeMenu->_defaultMenuItemIndex) {
		handleClick(_activeMenu->_defaultMenuItemIndex, mousePos);
	} else if (_vm->_input->pollEvent(kEventUp)) {
		// TODO handleUpKey();
		if (_hoveredMenuItemIndex == 1) {
			_hoveredMenuItemIndex = _activeMenu->getMenuItemsCount();
		} else {
			_hoveredMenuItemIndex--;
		}
		setMouseCursorToMenuItem(_hoveredMenuItemIndex);
		_hoveredMenuItemIndex2 = _hoveredMenuItemIndex;
		updateActorHoverBackground();
		playSoundEffect(0xC);
	} else if (_vm->_input->pollEvent(kEventDown)) {
		// TODO handleDownKey();
		if (_hoveredMenuItemIndex == _activeMenu->getMenuItemsCount()) {
			_hoveredMenuItemIndex = 1;
		} else {
			_hoveredMenuItemIndex++;
		}
		setMouseCursorToMenuItem(_hoveredMenuItemIndex);
		_hoveredMenuItemIndex2 = _hoveredMenuItemIndex;
		updateActorHoverBackground();
		playSoundEffect(0xC);
	}

	updateTimeOut(resetTimeOut);
}

void BaseMenuSystem::setTimeOutDuration(uint32 duration, uint timeOutMenuChoiceIndex) {
	if (duration > 0) {
		_isTimeOutEnabled = true;
		_isTimeOutReached = false;
		_timeOutDuration = duration;
		_timeOutMenuChoiceIndex = timeOutMenuChoiceIndex;
		_timeOutStartTime = getCurrentTime();
		_timeOutEndTime = duration + _timeOutStartTime;
	} else {
		_isTimeOutEnabled = false;
	}
}

void BaseMenuSystem::setMenuCallerThreadId(uint32 menuCallerThreadId) {
	_menuCallerThreadId = menuCallerThreadId;
}

void BaseMenuSystem::setMenuChoiceOffsets(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset) {
	_menuChoiceOffsets = menuChoiceOffsets;
	_menuChoiceOffset = menuChoiceOffset;
}

void BaseMenuSystem::setSavegameSlotNum(int slotNum) {
	_vm->_savegameSlotNum = slotNum;
}

void BaseMenuSystem::setSavegameDescription(Common::String desc) {
	_vm->_savegameDescription = desc;
}

void BaseMenuSystem::updateTimeOut(bool resetTimeOut) {

	if (!_isTimeOutEnabled)
		return;

	if (_menuStack.empty()) {
		if (_isTimeOutReached) {
			resetTimeOut = true;
			_isTimeOutReached = false;
		}
	} else if (!_isTimeOutReached) {
		_isTimeOutReached = true;
	}

	if (!_isTimeOutReached) {
		if (resetTimeOut) {
			_timeOutStartTime = getCurrentTime();
			_timeOutEndTime = _timeOutDuration + _timeOutStartTime;
		} else if (isTimerExpired(_timeOutStartTime, _timeOutEndTime)) {
			_isTimeOutEnabled = false;
			selectMenuChoiceIndex(_timeOutMenuChoiceIndex);
		}
	}

}

void BaseMenuSystem::redrawMenuText(BaseMenu *menu) {
	_vm->_screenText->removeText();
	drawMenuText(menu);
}

bool BaseMenuSystem::calcMenuItemTextPositionAtPoint(Common::Point pt, int &offset) {
	uint menuItemIndex;
	if (!calcMenuItemIndexAtPoint(pt, menuItemIndex)) {
		return false;
	}

	WRect rect;

	MenuItem *menuItem = _activeMenu->getMenuItem(menuItemIndex - 1);
	calcMenuItemRect(menuItemIndex, rect);
	int x = pt.x - rect._topLeft.x;
	Common::String text = menuItem->getText();
	FontResource *font = _vm->_dict->findFont(_activeMenu->_fontId);

	int curX = 0;
	for (uint i = 0; i < text.size(); i++) {
		int16 w = font->getCharInfo(text[i])->_width;
		if (x >= curX && x <= curX + w) {
			offset = i;
			return true;
		}
		curX = curX + w;
	}

	return false;
}

// MenuTextBuilder

MenuTextBuilder::MenuTextBuilder() : _pos(0) {
}

void MenuTextBuilder::appendString(const Common::String &value) {
	for (uint i = 0; i < value.size(); ++i) {
		_text[_pos++] = value[i];
	}
}

void MenuTextBuilder::appendNewLine() {
	_text[_pos++] = '\r';
}

void MenuTextBuilder::finalize() {
	_text[_pos] = '\0';
}

// BaseMenuAction

BaseMenuAction::BaseMenuAction(BaseMenuSystem *menuSystem)
	: _menuSystem(menuSystem) {
}

// MenuActionEnterMenu

MenuActionEnterMenu::MenuActionEnterMenu(BaseMenuSystem *menuSystem, int menuId)
	: BaseMenuAction(menuSystem), _menuId(menuId) {
}

void MenuActionEnterMenu::execute() {
	_menuSystem->enterSubMenuById(_menuId);
}

// MenuActionLeaveMenu

MenuActionLeaveMenu::MenuActionLeaveMenu(BaseMenuSystem *menuSystem)
	: BaseMenuAction(menuSystem) {
}

void MenuActionLeaveMenu::execute() {
	_menuSystem->leaveMenu();
}

// MenuActionReturnChoice

MenuActionReturnChoice::MenuActionReturnChoice(BaseMenuSystem *menuSystem, uint choiceIndex)
	: BaseMenuAction(menuSystem), _choiceIndex(choiceIndex) {
}

void MenuActionReturnChoice::execute() {
	_menuSystem->playSoundEffect13();
	_menuSystem->selectMenuChoiceIndex(_choiceIndex);
}

// MenuActionEnterQueryMenu

MenuActionEnterQueryMenu::MenuActionEnterQueryMenu(BaseMenuSystem *menuSystem, int menuId, uint confirmationChoiceIndex)
	: BaseMenuAction(menuSystem), _menuId(menuId), _confirmationChoiceIndex(confirmationChoiceIndex) {
}

void MenuActionEnterQueryMenu::execute() {
	_menuSystem->setQueryConfirmationChoiceIndex(_confirmationChoiceIndex);
	_menuSystem->enterSubMenuById(_menuId);
}

// MenuActionLoadGame

MenuActionLoadGame::MenuActionLoadGame(BaseMenuSystem *menuSystem, uint choiceIndex)
	: BaseMenuAction(menuSystem), _choiceIndex(choiceIndex) {
}

void MenuActionLoadGame::execute() {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	slot = dialog->runModalWithCurrentTarget();

	delete dialog;

	if (slot >= 0) {
		_menuSystem->setSavegameSlotNum(slot);
		_menuSystem->selectMenuChoiceIndex(_choiceIndex);
	}

}

// MenuActionSaveGame

MenuActionSaveGame::MenuActionSaveGame(BaseMenuSystem *menuSystem, uint choiceIndex)
		: BaseMenuAction(menuSystem), _choiceIndex(choiceIndex) {
}

void MenuActionSaveGame::execute() {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	slot = dialog->runModalWithCurrentTarget();
	desc = dialog->getResultString();

	delete dialog;

	if (slot >= 0) {
		_menuSystem->setSavegameSlotNum(slot);
		_menuSystem->setSavegameDescription(desc);
		_menuSystem->selectMenuChoiceIndex(_choiceIndex);
	}
}

} // End of namespace Illusions
