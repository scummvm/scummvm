/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/biochip_right.h"
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

class OvenDoor : public SceneBase {
public:
	OvenDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int openAnimID = 0, int closeAnimID = 0, int openFrame = 0, int closedFrame = 0,
			int left = 0, int top = 0, int right = 0, int bottom = 0);
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _openAnimationID;
	int _closeAnimationID;
	int _openFrame;
	int _closedFrame;
	Common::Rect _clickableRegion;
};

OvenDoor::OvenDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int openAnimID, int closeAnimID, int openFrame, int closedFrame,
		int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	_openAnimationID = openAnimID;
	_closeAnimationID = closeAnimID;
	_openFrame = openFrame;
	_closedFrame = closedFrame;
	_clickableRegion = Common::Rect(left, top, right, bottom);

	if (globalFlags.faKIOvenStatus == 1)
		_staticData.navFrameIndex = _openFrame;
	else
		_staticData.navFrameIndex = _closedFrame;
}

int OvenDoor::postExitRoom(Window *viewWindow, const Location &newLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if ((newLocation.orientation == 0 ||
		newLocation.facing != _staticData.location.facing ||
		newLocation.node != _staticData.location.node) && globalFlags.faKIOvenStatus == 1) {
		if (_staticData.location.timeZone == newLocation.timeZone)
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 7));

		globalFlags.faKIOvenStatus = 0;
	}

	return SC_TRUE;
}

int OvenDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_clickableRegion.contains(pointLocation)) {
		if (globalFlags.faKIOvenStatus == 1) {
			// Change the flag status
			globalFlags.faKIOvenStatus = 0;

			// Play the specified animation
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_closeAnimationID);
			_staticData.navFrameIndex = _closedFrame;

			return SC_TRUE;
		} else {
			// Change the flag status
			globalFlags.faKIOvenStatus = 1;

			// Play the specified animation
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_openAnimationID);
			_staticData.navFrameIndex = _openFrame;
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int OvenDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableRegion.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class KitchenUnitTurnOn : public SceneBase {
public:
	KitchenUnitTurnOn(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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
	void preDestructor() override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int gdiPaint(Window *viewWindow) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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
	_textFont = nullptr;
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
	void preDestructor() override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int onCharacter(Window *viewWindow, const Common::KeyState &character) override;
	int gdiPaint(Window *viewWindow) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;
	int preExitRoom(Window *viewWindow, const Location &priorLocation) override;

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
	_textFont = nullptr;
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
			if (!_shopNetCode.empty()) {
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
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, text, rect.left, rect.top, rect.width(), rect.height(), textColor, _lineHeight, kTextAlignLeft, vtCenter);
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
	void preDestructor() override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int gdiPaint(Window *viewWindow) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _packageCount;
	int _selectedPackage;
	Common::Rect _menuButton;
	Common::Rect _powerButton;
	Common::Rect _items[3];
	Common::Rect _replicateButton;
	Graphics::Font *_textFont;
	int _lineHeight;
	GlobalFlags &_globalFlags;

	void changeBackgroundBitmap();
	byte *postBoxSlot(byte slot);
	byte getPostBoxSlot(byte slot);
	void setPostBoxSlot(byte slot, byte value);
};

KitchenUnitPostBox::KitchenUnitPostBox(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation),
		_globalFlags(((SceneViewWindow *)viewWindow)->getGlobalFlags()) {
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
	_textFont = nullptr;
}

byte *KitchenUnitPostBox::postBoxSlot(byte slot) {
	switch (slot) {
	case 0:
		return &_globalFlags.faKIPostBoxSlotA;
	case 1:
		return &_globalFlags.faKIPostBoxSlotB;
	case 2:
		return &_globalFlags.faKIPostBoxSlotC;
	default:
		return nullptr;
	}
}

byte KitchenUnitPostBox::getPostBoxSlot(byte slot) {
	byte *s = postBoxSlot(slot);
	return s ? *s : 0;
}

void KitchenUnitPostBox::setPostBoxSlot(byte slot, byte value) {
	byte *s = postBoxSlot(slot);
	if (s)
		*s = value;
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

		switch (getPostBoxSlot(_selectedPackage)) {
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
			byte nextPackage = getPostBoxSlot(i + 1);
			setPostBoxSlot(i, nextPackage);
		}

		// Reset the last entry to 0
		setPostBoxSlot(_packageCount - 1, 0);

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

		switch (getPostBoxSlot(i)) {
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

class EnvironSystemControls : public SceneBase {
public:
	EnvironSystemControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _environCart;
	Common::Rect _environButton;
	Common::Rect _allSatButton;
};

EnvironSystemControls::EnvironSystemControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_environCart = Common::Rect(176, 70, 256, 136);
	_environButton = Common::Rect(102, 85, 152, 143);
	_allSatButton = Common::Rect(278, 85, 326, 143);
}

int EnvironSystemControls::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	switch (((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge) {
	case 0: // No cartridge inserted
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(11);
		_staticData.navFrameIndex = 57;
		break;
	case 1: // Geno single inserted
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(7);
		_staticData.navFrameIndex = 59;
		break;
	case 2: // Agent 3's cartridge inserted
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(9);
		_staticData.navFrameIndex = 56;
		break;
	}

	return SC_TRUE;
}

int EnvironSystemControls::preExitRoom(Window *viewWindow, const Location &newLocation) {
	_staticData.navFrameIndex = 50;

	switch (((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge) {
	case 0: // No cartridge inserted
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(12);
		break;
	case 1: // Geno single inserted
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(8);
		_staticData.navFrameIndex = 59;
		break;
	case 2: // Agent 3's cartridge inserted
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(10);
		_staticData.navFrameIndex = 56;
		break;
	}

	return SC_TRUE;
}

int EnvironSystemControls::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_environCart.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge > 0) {
		int itemID = 0;
		switch (((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge) {
		case 1:
			itemID = kItemGenoSingleCart;
			break;
		case 2:
			itemID = kItemEnvironCart;
			break;
		case 3:
			itemID = kItemClassicGamesCart; // Alas, the only time this is used in the code
			break;
		}

		// Reset the flag and change the frame index
		_staticData.navFrameIndex = 57;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge = 0;

		// Start dragging
		Common::Point ptInventoryWindow = viewWindow->convertPointToGlobal(pointLocation);
		ptInventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->convertPointToLocal(ptInventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(itemID, ptInventoryWindow);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int EnvironSystemControls::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 1;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = 2;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;

	// If there is something in the slot, return here
	if (_environCart.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge > 0)
		return SC_FALSE;

	if (_environButton.contains(pointLocation)) {
		switch (((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge) {
		case 0:
			newScene.destinationScene.depth = 3;
			newScene.transitionData = 16;
			break;
		case 1:
			newScene.destinationScene.depth = 4;
			newScene.transitionData = 16;
			break;
		case 2:
			newScene.destinationScene.depth = 5;
			newScene.transitionData = 16;
			break;
		}
	} else if (_allSatButton.contains(pointLocation)) {
		newScene.destinationScene.depth = 7;
		newScene.transitionData = 16;
	}

	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_TRUE;
}

int EnvironSystemControls::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if ((itemID == kItemGenoSingleCart || itemID == kItemEnvironCart || itemID == kItemClassicGamesCart) &&
			_environCart.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge == 0)
		return 1;

	return 0;
}

int EnvironSystemControls::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if ((itemID == kItemGenoSingleCart || itemID == kItemEnvironCart || itemID == kItemClassicGamesCart) &&
			((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge == 0) {
		switch (itemID) {
		case kItemGenoSingleCart: {
			_staticData.navFrameIndex = 59;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge = 1;
			viewWindow->invalidateWindow(false);
			DestinationScene newScene;
			newScene.destinationScene = _staticData.location;
			newScene.destinationScene.depth = 4;
			newScene.transitionType = TRANSITION_VIDEO;
			newScene.transitionData = 16;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
			return SIC_ACCEPT;
		}
		case kItemEnvironCart: {
			_staticData.navFrameIndex = 56;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge = 2;
			viewWindow->invalidateWindow(false);
			DestinationScene newScene;
			newScene.destinationScene = _staticData.location;
			newScene.destinationScene.depth = 5;
			newScene.transitionType = TRANSITION_VIDEO;
			newScene.transitionData = 16;
			newScene.transitionStartFrame = -1;
			newScene.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
			return SIC_ACCEPT;
		}
		}

		viewWindow->invalidateWindow(false);
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int EnvironSystemControls::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_environCart.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().faERCurrentCartridge > 0)
		return kCursorOpenHand;

	if (_environButton.contains(pointLocation) || _allSatButton.contains(pointLocation))
		return kCursorFinger;

	return kCursorPutDown;
}

class EnvironGenoVideo : public SceneBase {
public:
	EnvironGenoVideo(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _returnRegion;
};

EnvironGenoVideo::EnvironGenoVideo(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_returnRegion = Common::Rect(136, 150, 292, 189);
}

int EnvironGenoVideo::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_vm->_sound->stop();
	((SceneViewWindow *)viewWindow)->startAsynchronousAnimation(13, false);
	return SC_TRUE;
}

int EnvironGenoVideo::preExitRoom(Window *viewWindow, const Location &newLocation) {
	((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
	_vm->_sound->restart();
	return SC_TRUE;
}

int EnvironGenoVideo::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_returnRegion.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 1;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 4;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int EnvironGenoVideo::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_returnRegion.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class FlagChangeBackground : public SceneBase {
public:
	FlagChangeBackground(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			byte minFlagValue = 1, int newStillFrame = 0);
};

FlagChangeBackground::FlagChangeBackground(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		byte minFlagValue, int newStillFrame) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	
	if (globalFlags.faERTakenRemoteControl >= minFlagValue)
		_staticData.navFrameIndex = newStillFrame;
}

class ClickZoomInTopOfBookshelf : public SceneBase {
public:
	ClickZoomInTopOfBookshelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _awardZoom, _bookZoom;
};

ClickZoomInTopOfBookshelf::ClickZoomInTopOfBookshelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_awardZoom = Common::Rect(66, 20, 168, 64);
	_bookZoom = Common::Rect(206, 0, 370, 84);
}

int ClickZoomInTopOfBookshelf::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_awardZoom.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 1;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 9;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	if (_bookZoom.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 2;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 11;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickZoomInTopOfBookshelf::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_awardZoom.contains(pointLocation) || _bookZoom.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

class ClickZoomToyShelf : public SceneBase {
public:
	ClickZoomToyShelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);

	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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

	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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

class ClickOnCoffeeTable : public SceneBase {
public:
	ClickOnCoffeeTable(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _toyClickRect;
	Common::Rect _tazClickRect;
};

ClickOnCoffeeTable::ClickOnCoffeeTable(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_toyClickRect = Common::Rect(242, 56, 358, 138);
	_tazClickRect = Common::Rect(174, 0, 234, 56);
}

int ClickOnCoffeeTable::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_toyClickRect.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(31);
		return SC_TRUE;
	}

	if (_tazClickRect.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(26);
		((SceneViewWindow *)viewWindow)->getGlobalFlags().faMNTazClicked = 1;

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 0;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = 32;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_TRUE;
}

int ClickOnCoffeeTable::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_toyClickRect.contains(pointLocation) || _tazClickRect.contains(pointLocation))
		return kCursorFinger;

	return kCursorPutDown;
}

class ClickZoomInBottomOfBookshelf : public SceneBase {
public:
	ClickZoomInBottomOfBookshelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _leftClocks;
	Common::Rect _rightClocks;
};

ClickZoomInBottomOfBookshelf::ClickZoomInBottomOfBookshelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_leftClocks = Common::Rect(72, 32, 186, 110);
	_rightClocks = Common::Rect(194, 10, 370, 100);
}

int ClickZoomInBottomOfBookshelf::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_leftClocks.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 1;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 7;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	if (_rightClocks.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 2;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 3;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickZoomInBottomOfBookshelf::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_leftClocks.contains(pointLocation) || _rightClocks.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

class RightClockShelf : public SceneBase {
public:
	RightClockShelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _alienClock;
	Common::Rect _alarmClock;
	Common::Rect _pendulum;
};

RightClockShelf::RightClockShelf(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_alienClock = Common::Rect(0, 152, 80, 189);
	_alarmClock = Common::Rect(82, 102, 148, 189);
	_pendulum = Common::Rect(274, 0, 384, 189);
}

int RightClockShelf::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_alienClock.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(4);
		return SC_TRUE;
	}

	if (_alarmClock.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(2);
		((SceneViewWindow *)viewWindow)->getGlobalFlags().faMNClockClicked = 1;

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	if (_pendulum.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(5);
		return SC_TRUE;
	}

	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 0;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = 6;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_TRUE;
}

int RightClockShelf::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_alienClock.contains(pointLocation) || _alarmClock.contains(pointLocation) || _pendulum.contains(pointLocation))
		return kCursorFinger;

	return kCursorPutDown;
}

class MainDeskView : public SceneBase {
public:
	MainDeskView(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _papers;
	Common::Rect _terminal;
	Common::Rect _vidPhone;
	Common::Rect _deskLight;
};

MainDeskView::MainDeskView(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_papers = Common::Rect(38, 126, 132, 154);
	_terminal = Common::Rect(138, 118, 274, 166);
	_vidPhone = Common::Rect(334, 46, 418, 142);
	_deskLight = Common::Rect(20, 62, 82, 122);
}

int MainDeskView::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_papers.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 0) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 2;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 40;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	if (_terminal.contains(pointLocation)) {
		// Play the terminal access movie
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(44);
		return SC_TRUE;
	}

	if (_vidPhone.contains(pointLocation)) {
		// Move to the vidphone
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 3;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 42;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	if (_deskLight.contains(pointLocation)) {
		// Play the desk light animation
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(39);
		return SC_TRUE;
	}

	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 0;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = 45;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_TRUE;
}

int MainDeskView::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_papers.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 0)
		return kCursorMagnifyingGlass;

	if (_terminal.contains(pointLocation) || _deskLight.contains(pointLocation))
		return kCursorFinger;

	if (_vidPhone.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorPutDown;
}

class ViewVidPhone : public SceneBase {
public:
	ViewVidPhone(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;
	int movieCallback(Window *viewWindow, VideoWindow *movie, int animationID, int status) override;

private:
	Common::Rect _playButton;
	Common::Rect _pauseButton;
	Common::Rect _prevButton;
	bool _playingMovie;
	int _curMovie;
};

ViewVidPhone::ViewVidPhone(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_playButton = Common::Rect(102, 22, 120, 40);
	_pauseButton = Common::Rect(98, 49, 112, 63);
	_prevButton = Common::Rect(91, 81, 105, 95);
	_playingMovie = false;
	_curMovie = -1;
}

int ViewVidPhone::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Play messages sound effect here
	_vm->_sound->playSoundEffect("BITDATA/FUTAPT/FAMN_ANS.BTA");
	return SC_TRUE;
}

int ViewVidPhone::preExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_playingMovie) {
		((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
		_playingMovie = false;
		_vm->_sound->restart();
	}

	return SC_TRUE;
}

int ViewVidPhone::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_playButton.contains(pointLocation)) {
		if (_curMovie == 0) {
			((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
			_vm->_sound->restart();
			_curMovie = -1;
		} else {
			if (_playingMovie)
				((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();

			_curMovie = 0;
			_vm->_sound->stop();
			((SceneViewWindow *)viewWindow)->startAsynchronousAnimation(33, false);
			_playingMovie = true;
		}

		return SC_TRUE;
	}

	if (_pauseButton.contains(pointLocation)) {
		if (_curMovie == 1) {
			((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
			_vm->_sound->restart();
			_curMovie = -1;
		} else {
			if (_playingMovie)
				((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();

			_curMovie = 1;
			_vm->_sound->stop();
			((SceneViewWindow *)viewWindow)->startAsynchronousAnimation(34, false);
			_playingMovie = true;
		}

		return SC_TRUE;
	}

	if (_prevButton.contains(pointLocation)) {
		if (_curMovie == 2) {
			((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();
			_vm->_sound->restart();
			_curMovie = -1;
		} else {
			if (_playingMovie)
				((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();

			_curMovie = 2;
			_vm->_sound->stop();
			((SceneViewWindow *)viewWindow)->startAsynchronousAnimation(35, false);
			_playingMovie = true;
		}

		return SC_TRUE;
	}

	if (_playingMovie)
		((SceneViewWindow *)viewWindow)->stopAsynchronousAnimation();

	_playingMovie = false;
	_vm->_sound->restart();

	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 1;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = 43;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_TRUE;
}

int ViewVidPhone::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_playButton.contains(pointLocation) || _pauseButton.contains(pointLocation) || _prevButton.contains(pointLocation))
		return kCursorFinger;

	return kCursorPutDown;
}

int ViewVidPhone::movieCallback(Window *viewWindow, VideoWindow *movie, int animationID, int status) {
	if (animationID == -1 && status == MOVIE_STOPPED) {
		_vm->_sound->restart();
		_playingMovie = false;
		_curMovie = -1;
	}

	return SC_TRUE;
}

class MainEnvironDoorDown : public SceneBase {
public:
	MainEnvironDoorDown(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
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

class ClickOnBooks : public SceneBase {
public:
	ClickOnBooks(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int cursorID = 0,
			int timeZone = -1, int environment = -1, int node = -1, int facing = -1, int orientation = -1, int depth = -1,
			int transitionType = -1, int transitionData = -1, int transitionStartFrame = -1, int transitionLength = -1,
			int soundFileNameID = -1, int soundLeft = -1, int soundTop = -1, int soundRight = -1, int soundBottom = -1);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _cursorID;
	Common::Rect _clickRegion;
	DestinationScene _clickDestination;
	int _soundFileNameID;
	Common::Rect _soundRegion;
};

ClickOnBooks::ClickOnBooks(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int cursorID,
		int timeZone, int environment, int node, int facing, int orientation, int depth,
		int transitionType, int transitionData, int transitionStartFrame, int transitionLength,
		int soundFileNameID, int soundLeft, int soundTop, int soundRight, int soundBottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(left, top, right, bottom);
	_cursorID = cursorID;
	_clickDestination.destinationScene = Location(timeZone, environment, node, facing, orientation, depth);
	_clickDestination.transitionType = transitionType;
	_clickDestination.transitionData = transitionData;
	_clickDestination.transitionStartFrame = transitionStartFrame;
	_clickDestination.transitionLength = transitionLength;
	_soundFileNameID = soundFileNameID;
	_soundRegion = Common::Rect(soundLeft, soundTop, soundRight, soundBottom);
}

int ClickOnBooks::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_soundRegion.contains(pointLocation)) {
		_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_soundFileNameID), 128);
		((SceneViewWindow *)viewWindow)->getGlobalFlags().faMNBooksClicked = 1;

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		return SC_TRUE;
	}

	if (_clickRegion.contains(pointLocation))
		((SceneViewWindow *)viewWindow)->moveToDestination(_clickDestination);

	return SC_FALSE;
}

int ClickOnBooks::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_soundRegion.contains(pointLocation))
		return kCursorFinger;

	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

class ClickEnvironNatureScenes : public SceneBase {
public:
	ClickEnvironNatureScenes(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _controls;
};

ClickEnvironNatureScenes::ClickEnvironNatureScenes(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_controls = Common::Rect(0, 160, 432, 189);
	_staticData.navFrameIndex = 52;
}

int ClickEnvironNatureScenes::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_controls.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.depth = 1;
		newScene.transitionType = TRANSITION_VIDEO;
		newScene.transitionData = 4;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	// Update the image
	_staticData.navFrameIndex++;
	if (_staticData.navFrameIndex > 54)
		_staticData.navFrameIndex = 52;
	viewWindow->invalidateWindow(false);
	return SC_TRUE;
}

int ClickEnvironNatureScenes::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_controls.contains(pointLocation))
		return kCursorPutDown;

	return kCursorArrow;
}

class ViewEnvironCart : public SceneBase {
public:
	ViewEnvironCart(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int timerCallback(Window *viewWindow) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;
};

ViewEnvironCart::ViewEnvironCart(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().lensFilterActivated == 0)
		_staticData.navFrameIndex = 66;
}

int ViewEnvironCart::timerCallback(Window *viewWindow) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().lensFilterActivated == 1) {
		_staticData.navFrameIndex = 64;

		// Kill the ambient sound
		_vm->_sound->setAmbientSound();

		// Set the research scoring flag
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreResearchEnvironCart = 1;

		// Sit back and relax as you get abducted
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(15);
		_staticData.navFrameIndex = 55;

		// Empty the input queue
		BuriedEngine *vm = _vm;
		vm->removeMouseMessages(viewWindow);
		vm->removeKeyboardMessages(viewWindow);

		// Make the jump to Agent 3's lair
		DestinationScene newScene;
		newScene.destinationScene = Location(3, 2, 6, 0, 0, 0);
		newScene.transitionType = TRANSITION_NONE;
		newScene.transitionData = -1;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);

		vm->removeMouseMessages(viewWindow);
		vm->removeKeyboardMessages(viewWindow);
	}

	return SC_TRUE;
}

int ViewEnvironCart::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	// Move back
	DestinationScene newScene;
	newScene.destinationScene = _staticData.location;
	newScene.destinationScene.depth = 1;
	newScene.transitionType = TRANSITION_VIDEO;
	newScene.transitionData = 4;
	newScene.transitionStartFrame = -1;
	newScene.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
	return SC_TRUE;
}

int ViewEnvironCart::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	return kCursorFinger;
}

class MainEnvironSitDownClick : public SceneBase {
public:
	MainEnvironSitDownClick(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _environRect;
};

MainEnvironSitDownClick::MainEnvironSitDownClick(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_environRect = Common::Rect(120, 0, 302, 189);
}

int MainEnvironSitDownClick::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_environRect.contains(pointLocation)) {
		DestinationScene newScene;
		newScene.destinationScene = _staticData.location;
		newScene.destinationScene.orientation = 1;
		newScene.destinationScene.depth = 1;
		newScene.transitionType = TRANSITION_NONE;
		newScene.transitionData = -1;
		newScene.transitionStartFrame = -1;
		newScene.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newScene);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int MainEnvironSitDownClick::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_environRect.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class EnvironDoorExitSound : public SceneBase {
public:
	EnvironDoorExitSound(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
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
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	byte dummyFlag = 0; // a dummy flag, used as a placeholder for writing (but not reading)

	switch (sceneStaticData.classID) {
	case 0:
		// Default scene
		break;
	case 1:
		return new ClickPlayVideoSwitchAI(_vm, viewWindow, sceneStaticData, priorLocation, 0, kCursorFinger, globalFlags.faKICoffeeSpilled, 212, 114, 246, 160);
	case 2:
		return new ClickPlayVideoSwitchAI(_vm, viewWindow, sceneStaticData, priorLocation, 1, kCursorFinger, globalFlags.faKIBirdsBobbed, 150, 40, 260, 164);
	case 3:
		return new OvenDoor(_vm, viewWindow, sceneStaticData, priorLocation, 2, 3, 37, 25, 0, 0, 270, 80);
	case 4:
		return new OvenDoor(_vm, viewWindow, sceneStaticData, priorLocation, 4, 5, 38, 26, 0, 50, 300, 189);
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
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 200, 83, 230, 116, kItemBioChipTranslate, 61, globalFlags.faKITakenPostboxItem);
	case 12:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 202, 80, 227, 155, kItemCheeseGirl, 59, globalFlags.faKITakenPostboxItem);
	case 13:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 203, 111, 225, 129, kItemGenoSingleCart, 63, globalFlags.faKITakenPostboxItem);
	case 15:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 134, 0, 300, 189, kCursorFinger, 4, 2, 2, 0, 1, 1, TRANSITION_VIDEO, 0, -1, -1);
	case 16:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 163, 25, 273, 145, kCursorMagnifyingGlass, 4, 2, 2, 0, 1, 2, TRANSITION_VIDEO, 1, -1, -1);
	case 17:
		return new EnvironSystemControls(_vm, viewWindow, sceneStaticData, priorLocation);
	case 18:
		return new ClickEnvironNatureScenes(_vm, viewWindow, sceneStaticData, priorLocation);
	case 19:
		return new EnvironGenoVideo(_vm, viewWindow, sceneStaticData, priorLocation);
	case 20:
		return new ViewEnvironCart(_vm, viewWindow, sceneStaticData, priorLocation);
	case 21:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 4, 2, 2, 0, 1, 1, TRANSITION_VIDEO, 4, -1, -1);
	case 22:
		return new InteractiveNewsNetwork(_vm, viewWindow, sceneStaticData, priorLocation, -1, 4, 2, 2, 0, 1, 1, TRANSITION_VIDEO, 4, -1, -1);
	case 23:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 81, 146, 134, 189, kItemRemoteControl, 45, globalFlags.faERTakenRemoteControl);
	case 24:
		return new FlagChangeBackground(_vm, viewWindow, sceneStaticData, priorLocation, 1, 33);
	case 25:
		return new FlagChangeBackground(_vm, viewWindow, sceneStaticData, priorLocation, 1, 21);
	case 26:
		return new FlagChangeBackground(_vm, viewWindow, sceneStaticData, priorLocation, 1, 9);
	case 30:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 128, globalFlags.faStingerID, globalFlags.faStingerChannelID, 10, 14);
	case 31:
		return new ClickZoomInTopOfBookshelf(_vm, viewWindow, sceneStaticData, priorLocation);
	case 32:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 4, 3, 9, 0, 1, 0, TRANSITION_VIDEO, 10, -1, -1);
	case 33:
		return new ClickOnBooks(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 4, 3, 9, 0, 1, 0, TRANSITION_VIDEO, 12, -1, -1, IDS_FUTAPT_BOOK_AUDIO_FILENAME, 182, 8, 396, 156);
	case 34:
		return new ClickZoomInBottomOfBookshelf(_vm, viewWindow, sceneStaticData, priorLocation);
	case 35:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 4, 3, 9, 0, 0, 0, TRANSITION_VIDEO, 8, -1, -1);
	case 36:
		return new RightClockShelf(_vm, viewWindow, sceneStaticData, priorLocation);
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
	case 44:
		return new ClickPlayLoopingVideoClip(_vm, viewWindow, sceneStaticData, priorLocation, kCursorFinger, 25, 120, 0, 299, 132, globalFlags.faMNPongClicked, 1);
	case 45:
		return new ClickPlayLoopingVideoClip(_vm, viewWindow, sceneStaticData, priorLocation, kCursorFinger, 27, 0, 0, 432, 189, dummyFlag, 0);
	case 46:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 44, 26, 254, 144, kCursorMagnifyingGlass, 4, 3, 0, 2, 0, 1, TRANSITION_VIDEO, 30, -1, -1);
	case 47:
		return new ClickOnCoffeeTable(_vm, viewWindow, sceneStaticData, priorLocation);
	case 50:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 82, 38, 346, 138, kCursorMagnifyingGlass, 4, 3, 9, 2, 0, 1, TRANSITION_VIDEO, 38, -1, -1);
	case 51:
		return new MainDeskView(_vm, viewWindow, sceneStaticData, priorLocation);
	case 52:
		return new BrowseBook(_vm, viewWindow, sceneStaticData, priorLocation, IDBD_LETTERS_BOOK_DATA, -1, 0, 4, 3, 9, 2, 0, 1, TRANSITION_VIDEO, 41, -1, -1);
	case 53:
		return new ViewVidPhone(_vm, viewWindow, sceneStaticData, priorLocation);
	case 54:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 36, kCursorFinger, 0, 0, 432, 189);
	case 56:
		return new MainEnvironDoorDown(_vm, viewWindow, sceneStaticData, priorLocation);
	case 57:
		return new MainEnvironDoorExit(_vm, viewWindow, sceneStaticData, priorLocation);
	case 58:
		return new MainEnvironSitDownClick(_vm, viewWindow, sceneStaticData, priorLocation);
	case 59:
		return new EnvironDoorExitSound(_vm, viewWindow, sceneStaticData, priorLocation);
	default:
		warning("Unknown Future apartment scene object %d", sceneStaticData.classID);
	}

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
