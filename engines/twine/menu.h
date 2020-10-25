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

#ifndef TWINE_MENU_H
#define TWINE_MENU_H

#include "twine/actor.h"

namespace TwinE {

enum MenuSettingsType {
	// button number
	MenuSettings_CurrentLoadedButton = 0,
	// is used to calc the height where the first button will appear
	MenuSettings_NumberOfButtons = 1,
	MenuSettings_ButtonsBoxHeight = 2,
	MenuSettings_HeaderEnd = 3, // TODO: unknown
	MenuSettings_FirstButtonState = 4,
	MenuSettings_FirstButton = 5
};

class Menu {
private:
	TwinEEngine *_engine;
	/** Hero behaviour menu entity */
	uint8 *behaviourEntity = 0;
	/** Behaviour menu anim state */
	int16 behaviourAnimState[4]; // winTab
	/** Behaviour menu anim data pointer */
	AnimTimerDataStruct behaviourAnimData[4];

	int32 inventorySelectedColor = 0;
	int32 inventorySelectedItem = 0; // currentSelectedObjectInInventory

	/**
	 * Draws main menu button
	 * @param width menu button width
	 * @param topheight is the height between the top of the screen and the first button
	 * @param buttonId current button identification from menu settings
	 * @param textId
	 * @param hover flag to know if should draw as a hover button or not
	 */
	void drawButtonGfx(int32 width, int32 topheight, int32 buttonId, int32 textId, bool hover);
	void plasmaEffectRenderFrame();
	/**
	 * Process the menu button draw
	 * @param data menu settings array
	 * @param mode flag to know if should draw as a hover button or not
	 */
	void drawButton(const int16 *menuSettings, bool hover);
	/** Used to run the advanced options menu */
	int32 advoptionsMenu();
	/** Used to run the volume menu */
	int32 volumeMenu();
	/** Used to run the save game management menu */
	int32 savemanageMenu();
	void drawInfoMenu(int16 left, int16 top);
	void drawBehaviour(HeroBehaviourType behaviour, int32 angle, int16 cantDrawBox);
	void drawInventoryItems();
	void drawBehaviourMenu(int32 angle);
	void drawItem(int32 item);
	/**
	 * Draw the entire button box
	 * @param left start width to draw the button
	 * @param top start height to draw the button
	 * @param right end width to draw the button
	 * @param bottom end height to draw the button
	 */
	void drawMagicItemsBox(int32 left, int32 top, int32 right, int32 bottom, int32 color);

	int16 *GiveUpMenuWithSaveState;
	int16 *VolumeMenuState;
	int16 *SaveManageMenuState;
	int16 *GiveUpMenuState;
	int16 *MainMenuState;
	int16 *AdvOptionsMenuState;

public:
	Menu(TwinEEngine *engine);
	~Menu();
	int16 *OptionsMenuState;

	int32 currMenuTextIndex = -1;
	int32 currMenuTextBank = -1;
	char currMenuTextBuffer[256];

	int16 itemAngle[255]; // objectRotation

	/** Behaviour menu move pointer */
	ActorMoveStruct moveMenu;

	/** Plasma Effect pointer to file content: RESS.HQR:51 */
	uint8 *plasmaEffectPtr = nullptr;

	/**
	 * Process the plasma effect
	 * @param top top height where the effect will be draw in the front buffer
	 * @param color plasma effect start color
	 */
	void processPlasmaEffect(int32 left, int32 top, int32 right, int32 color);

	/**
	 * Draw the entire button box
	 * @param left start width to draw the button
	 * @param top start height to draw the button
	 * @param right end width to draw the button
	 * @param bottom end height to draw the button
	 */
	void drawBox(int32 left, int32 top, int32 right, int32 bottom);

	/**
	 * Where the main menu options are processed
	 * @param menuSettings menu settings array with the information to build the menu options
	 * @return pressed menu button identification
	 */
	int32 processMenu(int16 *menuSettings);

	bool init();

	/** Used to run the main menu */
	void run();

	/** Used to run the in-game give-up menu */
	int32 giveupMenu();

	/** Used to run the options menu */
	int32 optionsMenu();

	/** Process hero behaviour menu */
	void processBehaviourMenu();

	/** Process in-game inventory menu */
	void processInventoryMenu();
};

} // namespace TwinE

#endif
