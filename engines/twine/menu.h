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

class Menu {
private:
	TwinEEngine *_engine;

	/** Hero behaviour menu entity */
	uint8 *behaviourEntity = 0;
	/** Behaviour menu anim state */
	int16 behaviourAnimState[4]{0}; // winTab
	/** Behaviour menu anim data pointer */
	AnimTimerDataStruct behaviourAnimData[4];

	int32 inventorySelectedColor = 0;
	int32 inventorySelectedItem = 0; // currentSelectedObjectInInventory

	/**
	 * Draws main menu button
	 * @param width menu button width
	 * @param topheight is the height between the top of the screen and the first button
	 * @param id current button identification from menu settings
	 * @param value current button key pressed value
	 * @param mode flag to know if should draw as a hover button or not
	 */
	void drawButtonGfx(int32 width, int32 topheight, int32 id, int32 value, int32 mode);
	void plasmaEffectRenderFrame();
	/**
	 * Process the menu button draw
	 * @param data menu settings array
	 * @param mode flag to know if should draw as a hover button or not
	 */
	void drawButton(const int16 *menuSettings, int32 mode);
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

public:
	Menu(TwinEEngine *engine);

	int32 currMenuTextIndex = 0;
	int32 currMenuTextBank = 0;
	char currMenuTextBuffer[256] = "";

	int16 itemAngle[255]{0}; // objectRotation

	// TODO: these should be const - or the state might not get reset on an engine restart
	static int16 OptionsMenuSettings[];
	static int16 GiveUpMenuSettingsWithSave[];
	static int16 VolumeMenuSettings[];
	static int16 SaveManageMenuSettings[];
	static int16 GiveUpMenuSettings[];
	static int16 MainMenuSettings[];
	static int16 AdvOptionsMenuSettings[];

	/** Behaviour menu move pointer */
	ActorMoveStruct moveMenu;

	/** Plasma Effect pointer to file content: RESS.HQR:51 */
	uint8 *plasmaEffectPtr = nullptr;

	/**
	 * Process the plasma effect
	 * @param top top height where the effect will be draw in the front buffer
	 * @param color plasma effect start color
	 */
	void processPlasmaEffect(int32 top, int32 color);

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

	/** Used to run the main menu */
	void mainMenu();

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
