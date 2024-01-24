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

#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "dgds/includes.h"
#include "dgds/font.h"
#include "dgds/menu.h"
#include "dgds/request.h"

namespace Dgds {

enum MenuIds {
	kMenuNone = -1,
	kMenuMain = 0,
	kMenuControls = 1,
	kMenuOptions = 2,
	kMenuCalibrate = 3,
	kMenuRestart = 4,
	// 5: you cannot save your game right now
	// 6: game over
	kMenuFiles = 7,
	// 8: save game not saved because disk is full
	// 9: all game entries are full
	kMenuSave = 10,
	// 11: change directory - create directory
	// 12: change directory - invalid directory specified
	kMenuChangeDirectory = 13,
	kMenuJoystick = 14,
	kMenuMouse = 15,
	kMenuQuit = 16
	// 17: I'm frustrated - keep trying / win arcade
	// 18: skip introduction / play introduction
	// 19: save game before arcade
	// 20: replay arcade
};

enum MenuButtonIds {
	kMenuMainPlay = 120,
	kMenuMainControls = 20,
	kMenuMainOptions = 121,
	kMenuMainCalibrate = 118,
	kMenuMainFiles = 119,
	kMenuMainQuit = 122,

	kMenuControlsVCR = 127,
	kMenuControlsPlay = 128,

	kMenuOptionsJoystickOnOff = 139,
	kMenuOptionsMouseOnOff = 138,
	kMenuOptionsSoundsOnOff = 137,
	kMenuOptionsMusicOnOff = 140,
	kMenuOptionsVCR = 135,
	kMenuOptionsPlay = 136,

	kMenuCalibrateJoystick = 145,
	kMenuCalibrateMouse = 146,
	kMenuCalibrateVCR = 144,
	kMenuCalibratePlay = 147,
	kMenuCalibrateVCRHoC = 159,
	kMenuCalibratePlayHoC = 158,

	kMenuFilesSave = 107,
	kMenuFilesRestore = 106,
	kMenuFilesRestart = 105,
	kMenuFilesVCR = 103,
	kMenuFilesPlay = 130,

	kMenuSavePrevious = 58,
	kMenuSaveNext = 59,
	kMenuSaveSave = 53,
	kMenuSaveCancel = 54,
	kMenuSaveChangeDirectory = 55,

	kMenuChangeDirectoryOK = 95,
	kMenuChangeDirectoryCancel = 96,

	kMenuMouseCalibrationCalibrate = 157,
	kMenuMouseCalibrationPlay = 155,

	kMenuJoystickCalibrationOK = 132,

	kMenuQuitYes = 134,
	kMenuQuitNo = 133,

	kMenuRestartYes = 163,
	kMenuRestartNo = 164
};

Menu::Menu() {
	_screenBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
}

Menu::~Menu() {
	_screenBuffer.free();
}

void Menu::setScreenBuffer() {
	Graphics::Surface *dst = g_system->lockScreen();
	_screenBuffer.copyFrom(*dst);
	g_system->unlockScreen();
}

void Menu::drawMenu(REQFileData &vcrRequestData, int16 menu) {
	_curMenu = menu;

	Common::Array<Common::SharedPtr<Gadget> > gadgets = vcrRequestData._requests[_curMenu]._gadgets;

	// Restore background when drawing submenus
	g_system->copyRectToScreen(_screenBuffer.getPixels(), _screenBuffer.pitch, 0, 0, _screenBuffer.w, _screenBuffer.h);

	Graphics::Surface *dst = g_system->lockScreen();

	vcrRequestData._requests[_curMenu].draw(dst);

	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetType == kGadgetButton || gadget->_gadgetType == kGadgetSlider)
			gadget->draw(dst);
	}

	drawMenuText(vcrRequestData, dst);

	g_system->unlockScreen();
	g_system->updateScreen();
}

void Menu::drawMenuText(REQFileData &vcrRequestData, Graphics::Surface *dst) {
	Common::Array<Common::SharedPtr<Gadget> > gadgets = vcrRequestData._requests[_curMenu]._gadgets;
	Common::Array<TextItem> textItems = vcrRequestData._requests[_curMenu]._textItemList;

	// TODO: Get the parent coordinates properly
	uint16 parentX = gadgets[0].get()->_parentX;
	uint16 parentY = gadgets[0].get()->_parentY;
	uint16 pos = 0;

	for (TextItem &textItem : textItems) {
		// HACK: Skip the first entry, which corresponds to the header
		if (pos == 0) {
			pos++;
			continue;
		}

		const Font *font = RequestData::getMenuFont();
		int w = font->getStringWidth(textItem._txt);
		font->drawString(dst, textItem._txt, parentX + textItem._x, parentY + textItem._y, w, 0);
		pos++;
	}
}

int16 Menu::getClickedMenuItem(REQFileData &vcrRequestData, Common::Point mouseClick) {
	if (_curMenu < 0)
		return -1;

	Common::Array<Common::SharedPtr<Gadget> > gadgets = vcrRequestData._requests[_curMenu]._gadgets;

	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetType == kGadgetButton || gadget->_gadgetType == kGadgetSlider) {
			int16 x = gadget->_x + gadget->_parentX;
			int16 y = gadget->_y + gadget->_parentY;
			int16 right = x + gadget->_width;
			int16 bottom = (y + gadget->_height) - 1;
			Common::Rect gadgetRect(x, y, right, bottom);
			if (gadgetRect.contains(mouseClick))
				return (int16)gadget->_gadgetNo;
		}
	}

	return -1;
}

void Menu::handleMenu(REQFileData &vcrRequestData, Common::Point &mouse) {
	const int16 clickedMenuItem = getClickedMenuItem(vcrRequestData, mouse);

	// Click animation
	// TODO: Handle on/off buttons
	if (clickedMenuItem >= 0) {
		toggleGadget(vcrRequestData, clickedMenuItem, false);
		drawMenu(vcrRequestData, _curMenu);
		g_system->delayMillis(500);
		toggleGadget(vcrRequestData, clickedMenuItem, true);
	}

	switch (clickedMenuItem) {
	case kMenuMainPlay:
	case kMenuControlsPlay:
	case kMenuOptionsPlay:
	case kMenuCalibratePlay:
	case kMenuCalibratePlayHoC:
	case kMenuFilesPlay:
	case kMenuMouseCalibrationPlay:
		_curMenu = kMenuNone;
		CursorMan.showMouse(false);
		break;
	case kMenuMainControls:
		drawMenu(vcrRequestData, kMenuControls);
		break;
	case kMenuMainOptions:
		drawMenu(vcrRequestData, kMenuOptions);
		break;
	case kMenuMainCalibrate:
	case kMenuJoystickCalibrationOK:
	case kMenuMouseCalibrationCalibrate:
		drawMenu(vcrRequestData, kMenuCalibrate);
		break;
	case kMenuMainFiles:
	case kMenuSaveCancel:
		drawMenu(vcrRequestData, kMenuFiles);
		break;
	case kMenuMainQuit:
		drawMenu(vcrRequestData, kMenuQuit);
		break;
	case kMenuControlsVCR:
	case kMenuOptionsVCR:
	case kMenuCalibrateVCR:
	case kMenuCalibrateVCRHoC:
	case kMenuFilesVCR:
	case kMenuQuitNo:
	case kMenuRestartNo:
		drawMenu(vcrRequestData, kMenuMain);
		break;
	case kMenuOptionsJoystickOnOff:
	case kMenuOptionsMouseOnOff:
	case kMenuOptionsSoundsOnOff:
	case kMenuOptionsMusicOnOff:
		// TODO
		debug("Clicked option with ID %d", clickedMenuItem);
		break;
	case kMenuCalibrateJoystick:
		drawMenu(vcrRequestData, kMenuJoystick);
		break;
	case kMenuCalibrateMouse:
		drawMenu(vcrRequestData, kMenuMouse);
		break;
	case kMenuFilesSave:
	case kMenuChangeDirectoryCancel:
		drawMenu(vcrRequestData, kMenuSave);
		break;
	case kMenuFilesRestore:
		// TODO
		debug("Clicked Files - Restore %d", clickedMenuItem);
		break;
	case kMenuFilesRestart:
		drawMenu(vcrRequestData, kMenuRestart);
		break;
	case kMenuSavePrevious:
	case kMenuSaveNext:
	case kMenuSaveSave:
		// TODO
		debug("Clicked Save - %d", clickedMenuItem);
		break;
	case kMenuSaveChangeDirectory:
		drawMenu(vcrRequestData, kMenuChangeDirectory);
		break;
	case kMenuChangeDirectoryOK:
		// TODO
		debug("Clicked change directory - %d", clickedMenuItem);
		break;
	case kMenuQuitYes:
		g_engine->quitGame();
		break;
	case kMenuRestartYes:
		// TODO
		debug("Clicked Restart - Yes %d", clickedMenuItem);
		break;
	default:
		debug("Clicked ID %d", clickedMenuItem);
		break;
	}
}

void Menu::toggleGadget(REQFileData &vcrRequestData, int16 gadgetId, bool enable) {
	Common::Array<Common::SharedPtr<Gadget> > gadgets = vcrRequestData._requests[_curMenu]._gadgets;

	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetNo == gadgetId) {
			gadget->toggle(enable);
			return;
		}
	}
}

} // End of namespace Dgds
