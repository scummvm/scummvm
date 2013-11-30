/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

#include "graphics/font.h"

namespace Buried {

class KitchenUnitTurnOn : public SceneBase {
public:
	KitchenUnitTurnOn(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _powerButton;
};

KitchenUnitTurnOn::KitchenUnitTurnOn(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_powerButton = Common::Rect(49, 125, 121, 147);
}

int KitchenUnitTurnOn::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_powerButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 1;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int KitchenUnitTurnOn::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_powerButton.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class KitchenUnitTitleScreen : public SceneBase {
public:
	KitchenUnitTitleScreen(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _menuButton;
	Common::Rect _powerButton;
};

KitchenUnitTitleScreen::KitchenUnitTitleScreen(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_menuButton = Common::Rect(49, 96, 121, 118);
	_powerButton = Common::Rect(49, 125, 121, 147);
}

int KitchenUnitTitleScreen::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_menuButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 2;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_powerButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 0;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int KitchenUnitTitleScreen::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_powerButton.contains(pointLocation) || _menuButton.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class KitchenUnitMainMenu : public SceneBase {
public:
	KitchenUnitMainMenu(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _menuButton;
	Common::Rect _powerButton;
	Common::Rect _autoChef;
	Common::Rect _shopNet;
	Common::Rect _postBox;
};

KitchenUnitMainMenu::KitchenUnitMainMenu(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_menuButton = Common::Rect(49, 96, 121, 118);
	_powerButton = Common::Rect(49, 125, 121, 147);
	_autoChef = Common::Rect(159, 65, 251, 82);
	_shopNet = Common::Rect(159, 94, 239, 111);
	_postBox = Common::Rect(159, 123, 243, 140);
}

int KitchenUnitMainMenu::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_powerButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 0;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_autoChef.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 5;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_shopNet.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 4;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_postBox.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 3;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int KitchenUnitMainMenu::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_powerButton.contains(pointLocation) || _menuButton.contains(pointLocation)
			|| _autoChef.contains(pointLocation) || _shopNet.contains(pointLocation) || _postBox.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class KitchenUnitAutoChef : public SceneBase {
public:
	KitchenUnitAutoChef(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	~KitchenUnitAutoChef();
	void preDestructor();
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int gdiPaint(Window *viewWindow);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _menuButton;
	Common::Rect _powerButton;
	Common::Rect _autoChefButtons;
	int _status;
	Graphics::Font *_textFont;
	int _lineHeight;
};

KitchenUnitAutoChef::KitchenUnitAutoChef(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_menuButton = Common::Rect(49, 96, 121, 118);
	_powerButton = Common::Rect(49, 125, 121, 147);
	_autoChefButtons = Common::Rect(135, 54, 273, 137);
	_status = 0;
	_lineHeight = (_vm->getLanguage() == Common::JA_JPN) ? 11 : 14;
	_textFont = _vm->_gfx->createFont(_lineHeight);
}

KitchenUnitAutoChef::~KitchenUnitAutoChef() {
	preDestructor();
}

void KitchenUnitAutoChef::preDestructor() {
	delete _textFont;
	_textFont = 0;
}

int KitchenUnitAutoChef::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_menuButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 2;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_powerButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 0;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_autoChefButtons.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		_status = 1;
		_staticData.navFrameIndex = 58;
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int KitchenUnitAutoChef::gdiPaint(Window *viewWindow) {
	if (_status == 1) {
		uint32 textColor = _vm->_gfx->getColor(144, 200, 248);
		Common::String text = _vm->getString(IDFAKI_AC_ORDER_FOOD_TEXT);
		Common::Rect rect = Common::Rect(80, 26, 294, 92);
		rect.translate(64, 128);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, text, rect.left, rect.top, rect.width(), rect.height(), textColor, _lineHeight, kTextAlignLeft, true);
	}

	return SC_FALSE;
}

int KitchenUnitAutoChef::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_powerButton.contains(pointLocation) || _menuButton.contains(pointLocation) || _autoChefButtons.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class KitchenUnitShopNet : public SceneBase {
public:
	KitchenUnitShopNet(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	~KitchenUnitShopNet();
	void preDestructor();
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int onCharacter(Window *viewWindow, const Common::KeyState &character);
	int gdiPaint(Window *viewWindow);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);
	int preExitRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _status;
	Common::String _shopNetCode;
	Common::Rect _menuButton;
	Common::Rect _powerButton;
	Common::Rect _transmitButton;
	Common::Rect _acceptButton;
	Graphics::Font *_textFont;
	Common::Rect _numberButtons[10];
	int _lineHeight;
};

KitchenUnitShopNet::KitchenUnitShopNet(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_status = 0;
	_menuButton = Common::Rect(49, 96, 121, 118);
	_powerButton = Common::Rect(49, 125, 121, 147);
	_transmitButton = Common::Rect(176, 121, 259, 138);
	_acceptButton = Common::Rect(140, 128, 293, 145);
	_numberButtons[0] = Common::Rect(324, 105, 342, 122);
	_numberButtons[1] = Common::Rect(0, 0, 0, 0); // Number 1 is obscured
	_numberButtons[2] = Common::Rect(323, 4, 341, 21);
	_numberButtons[3] = Common::Rect(303, 13, 321, 30);
	_numberButtons[4] = Common::Rect(324, 29, 342, 46);
	_numberButtons[5] = Common::Rect(303, 39, 321, 56);
	_numberButtons[6] = Common::Rect(324, 54, 342, 71);
	_numberButtons[7] = Common::Rect(303, 64, 321, 81);
	_numberButtons[8] = Common::Rect(324, 79, 342, 96);
	_numberButtons[9] = Common::Rect(303, 91, 321, 108);
	_lineHeight = (_vm->getLanguage() == Common::JA_JPN) ? 10 : 14;
	_textFont = _vm->_gfx->createFont(_lineHeight);
}

KitchenUnitShopNet::~KitchenUnitShopNet() {
	preDestructor();
}

void KitchenUnitShopNet::preDestructor() {
	delete _textFont;
	_textFont = 0;
}

int KitchenUnitShopNet::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_menuButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 2;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_powerButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 0;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_transmitButton.contains(pointLocation) && _status == 1) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		if (_shopNetCode == _vm->getString(IDFAKI_SN_CHEESE_GIRL_CODE_TEXT)) {
			_status = 3;
			_staticData.navFrameIndex = 54;
		} else if (_shopNetCode == _vm->getString(IDFAKI_SN_TRANSLATE_CHIP_CODE_TEXT)) {
			_status = 2;
			_staticData.navFrameIndex = 54;
		} else if (_shopNetCode == _vm->getString(IDFAKI_SN_GENO_SINGLE_CODE_TEXT)) {
			_status = 4;
			_staticData.navFrameIndex = 54;
		}

		viewWindow->invalidateWindow();

		switch (_status) {
		case 2:
			((SceneViewWindow *)viewWindow)->startPlacedAsynchronousAnimation(206, 30, 84, 84, 6, true);
			break;
		case 3:
			((SceneViewWindow *)viewWindow)->startPlacedAsynchronousAnimation(206, 30, 84, 84, 7, true);
			break;
		case 4:
			((SceneViewWindow *)viewWindow)->startPlacedAsynchronousAnimation(206, 30, 84, 84, 8, true);
			break;
		}

		return SC_TRUE;
	} else if (_acceptButton.contains(pointLocation) && (_status >= 2 && _status <= 4)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();

		GlobalFlags &flags = ((SceneViewWindow *)viewWindow)->getGlobalFlags();

		if (flags.faKIPostBoxSlotA == _status || flags.faKIPostBoxSlotB == _status || flags.faKIPostBoxSlotC == _status) {
			// Already in the post box
			_status = 6;
			_staticData.navFrameIndex = 55;
		} else {
			// Add this item to the end of the post box list
			if (flags.faKIPostBoxSlotA == 0)
				flags.faKIPostBoxSlotA = _status;
			else if (flags.faKIPostBoxSlotB == 0)
				flags.faKIPostBoxSlotB = _status;
			else if (flags.faKIPostBoxSlotC == 0)
				flags.faKIPostBoxSlotC = _status;

			_status = 5;
			_staticData.navFrameIndex = 55;
		}

		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	}

	for (int i = 0; i < 10; i++) {
		if (_numberButtons[i].contains(pointLocation)) {
			// Generate a key state
			Common::KeyState state;
			state.keycode = (Common::KeyCode)(Common::KEYCODE_0 + i);
			state.ascii = '0' + i;
			state.flags = 0;
			onCharacter(viewWindow, state);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int KitchenUnitShopNet::onCharacter(Window *viewWindow, const Common::KeyState &character) {
	if (_status == 0 && ((character.keycode >= Common::KEYCODE_0 && character.keycode <= Common::KEYCODE_9)
			|| character.keycode == Common::KEYCODE_BACKSPACE || character.keycode == Common::KEYCODE_DELETE)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 9));

		if (character.keycode == Common::KEYCODE_BACKSPACE || character.keycode == Common::KEYCODE_DELETE) {
			if (_shopNetCode.empty()) {
				// clone2727 asks why the sound effect is being played again
				_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 9));

				if (_shopNetCode.size() == 6 || _shopNetCode.size() == 11) {
					_shopNetCode.deleteLastChar();
					_shopNetCode.deleteLastChar();
					_shopNetCode.deleteLastChar();
					_shopNetCode.deleteLastChar();
				} else {
					_shopNetCode.deleteLastChar();
				}

				viewWindow->invalidateWindow(false);

				// NOTE: Original returned here instead, but that resulted in a bad character
				// being printed if the string was empty.
			}

			return SC_TRUE;
		}

		// Add the character to the code string
		static const char *dashString = " - ";

		switch (_shopNetCode.size()) {
		case 0:
		case 1:
		case 6:
		case 11:
		case 12:
			_shopNetCode += (char)(character.keycode - Common::KEYCODE_0 + '0');
			break;
		case 2:
		case 7:
			_shopNetCode += (char)(character.keycode - Common::KEYCODE_0 + '0');
			_shopNetCode += dashString;
			break;
		case 13:
			_shopNetCode += (char)(character.keycode - Common::KEYCODE_0 + '0');

			if (_shopNetCode == _vm->getString(IDFAKI_SN_CHEESE_GIRL_CODE_TEXT)
					|| _shopNetCode == _vm->getString(IDFAKI_SN_TRANSLATE_CHIP_CODE_TEXT)
					|| _shopNetCode == _vm->getString(IDFAKI_SN_GENO_SINGLE_CODE_TEXT)) {
				_status = 1;
				_staticData.navFrameIndex = 53;
			} else {
				_shopNetCode.clear();
			}
			break;
		}

		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int KitchenUnitShopNet::gdiPaint(Window *viewWindow) {
	uint32 textColor = _vm->_gfx->getColor(104, 40, 168);
	Common::String text;
	Common::Rect rect;
	bool vtCenter = false;

	switch (_status) {
	case 0: // Entering code
	case 1: // Valid code
		text = _shopNetCode;
		rect = Common::Rect(117, 71, 244, 88);
		vtCenter = true;
		break;
	case 2: // Translate BioChip
		text = _vm->getString(IDFAKI_SN_TRANSLATE_CHIP_ORDER_TEXT);
		rect = Common::Rect(87, 33, 179, 87);
		break;
	case 3: // Cheese Girl
		text = _vm->getString(IDFAKI_SN_CHEESE_GIRL_ORDER_TEXT);
		rect = Common::Rect(87, 33, 179, 87);
		break;
	case 4: // Geno Environ Cartridge
		text = _vm->getString(IDFAKI_SN_GENO_SINGLE_ORDER_TEXT);
		rect = Common::Rect(87, 33, 179, 87);
		break;
	case 5: // Ordered the item successfully
		text = _vm->getString(IDFAKI_SN_SUCCESSFUL_ORDER_TEXT);
		rect = Common::Rect(80, 26, 294, 92);
		break;
	case 6: // Ordered the item unsuccessfully
		text = _vm->getString(IDFAKI_SN_UNSUCCESSFUL_ORDER_TEXT);
		rect = Common::Rect(80, 26, 294, 92);
		break;
	}

	if (!text.empty()) {
		rect.translate(64, 128);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, text, rect.left, rect.top, rect.width(), rect.height(), textColor, _lineHeight, kTextAlignLeft, true);
	}

	return SC_FALSE;
}

int KitchenUnitShopNet::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_powerButton.contains(pointLocation) || _menuButton.contains(pointLocation))
		return kCursorFinger;

	if (_transmitButton.contains(pointLocation) && _status == 1)
		return kCursorFinger;

	if (_acceptButton.contains(pointLocation) && (_status >= 2 && _status <= 4))
		return kCursorFinger;

	for (int i = 0; i < 10; i++)
		if (_numberButtons[i].contains(pointLocation))
			return kCursorFinger;

	return kCursorArrow;
}

int KitchenUnitShopNet::preExitRoom(Window *viewWindow, const Location &priorLocation) {
	((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
	return SC_TRUE;
}

class KitchenUnitPostBox : public SceneBase {
public:
	KitchenUnitPostBox(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	~KitchenUnitPostBox();
	void preDestructor();
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int gdiPaint(Window *viewWindow);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _packageCount;
	int _selectedPackage;
	Common::Rect _menuButton;
	Common::Rect _powerButton;
	Common::Rect _items[3];
	Common::Rect _replicateButton;
	Graphics::Font *_textFont;
	int _lineHeight;

	void changeBackgroundBitmap();
};

KitchenUnitPostBox::KitchenUnitPostBox(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_packageCount = 0;
	_selectedPackage = -1;
	_menuButton = Common::Rect(49, 96, 121, 118);
	_powerButton = Common::Rect(49, 125, 121, 147);
	_items[0] = Common::Rect(137, 62, 285, 79);
	_items[1] = Common::Rect(137, 91, 285, 108);
	_items[2] = Common::Rect(137, 120, 285, 137);
	_replicateButton = Common::Rect(200, 150, 283, 167);

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().faKIPostBoxSlotA != 0)
		_packageCount++;
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().faKIPostBoxSlotB != 0)
		_packageCount++;
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().faKIPostBoxSlotC != 0)
		_packageCount++;

	changeBackgroundBitmap();

	_lineHeight = (_vm->getLanguage() == Common::JA_JPN) ? 10 : 14;
	_textFont = _vm->_gfx->createFont(_lineHeight);
}

KitchenUnitPostBox::~KitchenUnitPostBox() {
	preDestructor();
}

void KitchenUnitPostBox::preDestructor() {
	delete _textFont;
	_textFont = 0;
}

int KitchenUnitPostBox::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_menuButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 2;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_powerButton.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		Location newLocation = _staticData.location;
		newLocation.depth = 0;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	} else if (_replicateButton.contains(pointLocation) && _selectedPackage >= 0) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().faKITakenPostboxItem = 0;
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;

		int item;
		switch (_selectedPackage) {
		case 0:
			item = ((SceneViewWindow *)viewWindow)->getGlobalFlags().faKIPostBoxSlotA;
			break;
		case 1:
			item = ((SceneViewWindow *)viewWindow)->getGlobalFlags().faKIPostBoxSlotB;
			break;
		case 2:
			item = ((SceneViewWindow *)viewWindow)->getGlobalFlags().faKIPostBoxSlotC;
			break;
		}

		switch (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(offsetof(GlobalFlags, faKIPostBoxSlotA) + _selectedPackage)) {
		case 2:
			newScene.destinationScene.depth = 6;
			newScene.transitionData = 9;
			break;
		case 3:
			newScene.destinationScene.depth = 7;
			newScene.transitionData = 10;
			break;
		case 4:
			newScene.destinationScene.depth = 8;
			newScene.transitionData = 11;
			break;
		}

		// Remove the item from the post box
		for (int i = _selectedPackage; i < _packageCount - 1; i++) {
			byte nextPackage = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(offsetof(GlobalFlags, faKIPostBoxSlotA) + i + 1);
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(offsetof(GlobalFlags, faKIPostBoxSlotA) + i, nextPackage);
		}

		// Reset the last entry to 0
		((SceneViewWindow *)viewWindow)->setGlobalFlagByte(offsetof(GlobalFlags, faKIPostBoxSlotA) + _packageCount - 1, 0);

		// Move to the destination scene
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);

		return SC_TRUE;
	}

	for (int i = 0; i < _packageCount; i++) {
		if (_items[i].contains(pointLocation)) {
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));
			_selectedPackage = i;
			changeBackgroundBitmap();
			viewWindow->invalidateWindow(false);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int KitchenUnitPostBox::gdiPaint(Window *viewWindow) {
	uint32 textColor = _vm->_gfx->getColor(144, 200, 248);

	for (int i = 0; i < _packageCount; i++) {
		Common::String text;

		switch (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(offsetof(GlobalFlags, faKIPostBoxSlotA) + i)) {
		case 2:
			text = _vm->getString(IDFAKI_SN_TRANSLATE_CHIP_CODE_TITLE);
			break;
		case 3:
			text = _vm->getString(IDFAKI_SN_CHEESE_GIRL_CODE_TITLE);
			break;
		case 4:
			text = _vm->getString(IDFAKI_SN_GENO_SINGLE_CODE_TITLE);
			break;
		}

		Common::Rect rect(_items[i]);
		rect.translate(64, 128);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, text, rect.left, rect.top, rect.width(), rect.height(), textColor, _lineHeight, kTextAlignCenter, true);
	}

	return SC_FALSE;
}

int KitchenUnitPostBox::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_powerButton.contains(pointLocation) || _menuButton.contains(pointLocation))
		return kCursorFinger;

	if (_items[0].contains(pointLocation) && _packageCount >= 0)
		return kCursorFinger;

	if (_items[1].contains(pointLocation) && _packageCount >= 1)
		return kCursorFinger;

	if (_items[2].contains(pointLocation) && _packageCount >= 2)
		return kCursorFinger;

	if (_replicateButton.contains(pointLocation) && _selectedPackage >= 0)
		return kCursorFinger;

	return kCursorArrow;
}

void KitchenUnitPostBox::changeBackgroundBitmap() {
	switch (_packageCount) {
	case 0:
		_staticData.navFrameIndex = 41;
		break;
	case 1:
		switch (_selectedPackage) {
		case -1:
			_staticData.navFrameIndex = 42;
			break;
		case 0:
			_staticData.navFrameIndex = 45;
			break;
		}
		break;
	case 2:
		switch (_selectedPackage) {
		case -1:
			_staticData.navFrameIndex = 43;
			break;
		case 0:
			_staticData.navFrameIndex = 46;
			break;
		case 1:
			_staticData.navFrameIndex = 48;
			break;
		}
		break;
	case 3:
		switch (_selectedPackage) {
		case -1:
			_staticData.navFrameIndex = 44;
			break;
		case 0:
			_staticData.navFrameIndex = 47;
			break;
		case 1:
			_staticData.navFrameIndex = 49;
			break;
		case 2:
			_staticData.navFrameIndex = 50;
			break;
		}
		break;
	}
}

class ClickZoomToyShelf : public SceneBase {
public:
	ClickZoomToyShelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);

	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _toyZooms[4];
};

ClickZoomToyShelf::ClickZoomToyShelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) : SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_toyZooms[0] = Common::Rect(102, 8, 170, 108);
	_toyZooms[1] = Common::Rect(188, 24, 228, 108);
	_toyZooms[2] = Common::Rect(282, 24, 330, 108);
	_toyZooms[3] = Common::Rect(330, 28, 366, 108);
}

int ClickZoomToyShelf::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 4; i++) {
		if (_toyZooms[i].contains(pointLocation)) {
			DestinationScene newScene;
			newScene.destinationScene = _staticData.location;
			newScene.destinationScene.depth = i + 1;
			newScene.transitionType = TRANSITION_VIDEO;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;

			static const int transitionData[4] = { 13, 16, 19, 22 };
			newScene.transitionData = transitionData[i];

			((SceneViewWindow *)viewWindow)->moveToDestination(newScene);

			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int ClickZoomToyShelf::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 4; i++)
		if (_toyZooms[i].contains(pointLocation))
			return kCursorMagnifyingGlass;

	return kCursorArrow;
}

class ToyClick : public SceneBase {
public:
	ToyClick(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int returnDepth = -1, int clickAnimation = -1, int returnAnimation = -1); 

	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _clickRect;
	int _returnDepth;
	int _clickAnimation;
	int _returnAnimation;
};

ToyClick::ToyClick(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int returnDepth, int clickAnimation, int returnAnimation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRect = Common::Rect(left, top, right, bottom);
	_returnDepth = returnDepth;
	_clickAnimation = clickAnimation;
	_returnAnimation = returnAnimation;
}

int ToyClick::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRect.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_clickAnimation);

		if (_clickAnimation == 17) {
			((SceneViewWindow *)viewWindow)->getGlobalFlags().faHeardAgentFigure = 1;

			if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1 && !((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_WM_AI_LAB_TEXT));
		}

		return SC_TRUE;
	}

	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = _returnDepth;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = _returnAnimation;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);

	return SC_TRUE;
}

int ToyClick::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRect.contains(pointLocation))
		return kCursorFinger;

	return kCursorPutDown;
}

class MainEnvironDoorDown : public SceneBase {
public:
	MainEnvironDoorDown(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	bool _doorOpen;
	Common::Rect _doorRect;
};

MainEnvironDoorDown::MainEnvironDoorDown(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_doorOpen = false;

	if (priorLocation.timeZone == _staticData.location.timeZone &&
			priorLocation.environment == _staticData.location.environment &&
			priorLocation.node == _staticData.location.node &&
			priorLocation.facing == _staticData.location.facing &&
			priorLocation.orientation == 1) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().faMNEnvironDoor == 1) {
			_doorOpen = true;
			_staticData.navFrameIndex = 220;
		}
	} else {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().faMNEnvironDoor = 0;
	}

	_doorRect = Common::Rect(0, 0, 432, 189);
}

int MainEnvironDoorDown::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Play the door open movie
	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 1;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = 0;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_FALSE;
}

int MainEnvironDoorDown::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_doorRect.contains(pointLocation)) {
		_staticData.navFrameIndex = 220;
		_doorOpen = true;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().faMNEnvironDoor = 1;

		// Play the door open movie
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 1;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 0;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);

		return SC_TRUE;
	}

	return SC_FALSE;
}

int MainEnvironDoorDown::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_doorRect.contains(pointLocation))
		return kCursorFinger;

	return kCursorPutDown;
}

class MainEnvironDoorExit : public SceneBase {
public:
	MainEnvironDoorExit(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
};

MainEnvironDoorExit::MainEnvironDoorExit(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int MainEnvironDoorExit::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Play the door open movie
	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 1;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = 14;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_FALSE;
}

class EnvironDoorExitSound : public SceneBase {
public:
	EnvironDoorExitSound(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation);
};

EnvironDoorExitSound::EnvironDoorExitSound(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int EnvironDoorExitSound::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_staticData.location.timeZone == newLocation.timeZone)
		_vm->_sound->playSoundEffect(_vm->getFilePath(IDS_FUTAPT_ENVIRON_DOOR_CLOSE));

	return SC_TRUE;
}

bool SceneViewWindow::startFutureApartmentAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(4, environment, SF_AMBIENT), fade, 64);
	return true;
}

SceneBase *SceneViewWindow::constructFutureApartmentSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 5:
		return new KitchenUnitTurnOn(_vm, viewWindow, sceneStaticData, priorLocation);
	case 6:
		return new KitchenUnitTitleScreen(_vm, viewWindow, sceneStaticData, priorLocation);
	case 7:
		return new KitchenUnitMainMenu(_vm, viewWindow, sceneStaticData, priorLocation);
	case 8:
		return new KitchenUnitPostBox(_vm, viewWindow, sceneStaticData, priorLocation);
	case 9:
		return new KitchenUnitShopNet(_vm, viewWindow, sceneStaticData, priorLocation);
	case 10:
		return new KitchenUnitAutoChef(_vm, viewWindow, sceneStaticData, priorLocation);
	case 11:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 200, 83, 230, 116, kItemBioChipTranslate, 61, offsetof(GlobalFlags, faKITakenPostboxItem));
	case 12:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 202, 80, 227, 155, kItemCheeseGirl, 59, offsetof(GlobalFlags, faKITakenPostboxItem));
	case 13:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 203, 111, 225, 129, kItemGenoSingleCart, 63, offsetof(GlobalFlags, faKITakenPostboxItem));
	case 15:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 134, 0, 300, 189, kCursorFinger, 4, 2, 2, 0, 1, 1, TRANSITION_VIDEO, 0, -1, -1);
	case 16:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 163, 25, 273, 145, kCursorMagnifyingGlass, 4, 2, 2, 0, 1, 2, TRANSITION_VIDEO, 1, -1, -1);
	case 21:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 4, 2, 2, 0, 1, 1, TRANSITION_VIDEO, 4, -1, -1);
	case 23:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 81, 146, 134, 189, kItemRemoteControl, 45, offsetof(GlobalFlags, faERTakenRemoteControl));
	case 30:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 128, offsetof(GlobalFlags, faStingerID), offsetof(GlobalFlags, faStingerChannelID), 10, 14);
	case 32:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 4, 3, 9, 0, 1, 0, TRANSITION_VIDEO, 10, -1, -1);
	case 35:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 4, 3, 9, 0, 0, 0, TRANSITION_VIDEO, 8, -1, -1);
	case 37:
		return new ClickZoomToyShelf(_vm, viewWindow, sceneStaticData, priorLocation);
	case 38:
		return new ToyClick(_vm, viewWindow, sceneStaticData, priorLocation, 82, 0, 358, 189, 0, 14, 15);
	case 39:
		return new ToyClick(_vm, viewWindow, sceneStaticData, priorLocation, 104, 0, 320, 189, 0, 17, 18);
	case 40:
		return new ToyClick(_vm, viewWindow, sceneStaticData, priorLocation, 104, 10, 270, 189, 0, 20, 21);
	case 41:
		return new ToyClick(_vm, viewWindow, sceneStaticData, priorLocation, 128, 0, 332, 189, 0, 23, 24);
	case 42:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 168, 38, 268, 108, kCursorMagnifyingGlass, 4, 3, 5, 0, 0, 1, TRANSITION_VIDEO, 28, -1, -1);
	case 43:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 4, 3, 5, 0, 0, 0, TRANSITION_VIDEO, 29, -1, -1);
	case 46:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 44, 26, 254, 144, kCursorMagnifyingGlass, 4, 3, 0, 2, 0, 1, TRANSITION_VIDEO, 30, -1, -1);
	case 50:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 82, 38, 346, 138, kCursorMagnifyingGlass, 4, 3, 9, 2, 0, 1, TRANSITION_VIDEO, 38, -1, -1);
	case 54:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 36, kCursorFinger, 0, 0, 432, 189);
	case 56:
		return new MainEnvironDoorDown(_vm, viewWindow, sceneStaticData, priorLocation);
	case 57:
		return new MainEnvironDoorExit(_vm, viewWindow, sceneStaticData, priorLocation);
	case 59:
		return new EnvironDoorExitSound(_vm, viewWindow, sceneStaticData, priorLocation);
	}

	warning("TODO: Future apartment scene object %d", sceneStaticData.classID);

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
