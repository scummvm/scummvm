/** @file menu.h
	@brief
	This file contains main menu create and processing routines.

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MENU_H
#define MENU_H

#include "sdlengine.h"
#include "actor.h"

int32 currMenuTextIndex;
int32 currMenuTextBank;
int8  currMenuTextBuffer[256];

int16 itemAngle[255]; // objectRotation

extern int16 OptionsMenuSettings[];

/** Behaviour menu move pointer */
ActorMoveStruct moveMenu;

/** Plasma Effect pointer to file content: RESS.HQR:51 */
extern uint8 *plasmaEffectPtr;

/** Process the plasma effect
	@param top top height where the effect will be draw in the front buffer
	@param color plasma effect start color */
void processPlasmaEffect(int32 top, int32 color);

/** Draw the entire button box
	@param left start width to draw the button
	@param top start height to draw the button
	@param right end width to draw the button
	@param bottom end height to draw the button */
void drawBox(int32 left, int32 top, int32 right, int32 bottom);

/** Draws inside buttons transparent area
	@param left start width to draw the button
	@param top start height to draw the button
	@param right end width to draw the button
	@param bottom end height to draw the button
	@param colorAdj index to adjust the transparent box color */
void drawTransparentBox(int32 left, int32 top, int32 right, int32 bottom, int32 colorAdj);

/** Where the main menu options are processed
	@param menuSettings menu settings array with the information to build the menu options
	@return pressed menu button identification */
int32 processMenu(int16 * menuSettings);

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

#endif
