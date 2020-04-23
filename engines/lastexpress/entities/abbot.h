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

#ifndef LASTEXPRESS_ABBOT_H
#define LASTEXPRESS_ABBOT_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Abbot : public Entity {
public:
	Abbot(LastExpressEngine *engine);
	~Abbot() override {}

	/**
	 * Resets the entity
	 */
	DECLARE_FUNCTION(reset)

	/**
	 * Draws the entity
	 *
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(draw, const char *sequence)

	/**
	 * Handles entering/exiting a compartment.
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_VFUNCTION_2(enterExitCompartment, const char *sequence, ObjectIndex compartment)

	/**
	 * Handles entering/exiting a compartment and updates position/play animation
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_FUNCTION_2(enterExitCompartment2, const char *sequence, ObjectIndex compartment)

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

	/**
	 * Draws the entity along with another one
	 *
	 * @param sequence1   The sequence to draw
	 * @param sequence2   The sequence to draw for the second entity
	 * @param entity      The EntityIndex of the second entity
	 */
	DECLARE_FUNCTION_3(draw2, const char *sequence1, const char *sequence2, EntityIndex entity)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	/**
	 * Updates parameter 2 using ticks value
	 *
	 * @param ticks The number of ticks to add
	 */
	DECLARE_FUNCTION_1(updateFromTicks, uint32 ticks)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_VFUNCTION_1(playSound, const char *filename)

	/**
	 * Saves the game
	 *
	 * @param savegameType The type of the savegame
	 * @param param        The param for the savegame (EventIndex or TimeValue)
	 */
	DECLARE_VFUNCTION_2(savegame, SavegameType savegameType, uint32 param)

	/**
	 * Updates the entity
	 *
	 * @param car            The car
	 * @param entityPosition The entity position
	 */
	DECLARE_VFUNCTION_2(updateEntity, CarIndex car, EntityPosition entityPosition)

	/**
	 * Call a savepoint (or draw sequence in default case)
	 *
	 * @param sequence1   The sequence to draw in the default case
	 * @param entity      The entity
	 * @param action      The action
	 * @param sequence2   The sequence name for the savepoint
	 */
	DECLARE_FUNCTION_4(callSavepoint, const char *sequence1, EntityIndex entity, ActionIndex action, const char *sequence2)

	/**
	 * Updates the position
	 *
	 * @param sequence1      The sequence to draw
	 * @param car            The car
	 * @param position       The position
	 */
	DECLARE_FUNCTION_3(updatePosition, const char *sequence1, CarIndex car, Position position)

	/**
	 * Process callback action when somebody is standing in the restaurant or salon.
	 */
	DECLARE_FUNCTION(callbackActionRestaurantOrSalon)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_VFUNCTION(chapter1)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_VFUNCTION(chapter2)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_VFUNCTION(chapter3)

	/**
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION(inKitchen)
	DECLARE_FUNCTION(openCompartment)
	DECLARE_FUNCTION(readPaper)
	DECLARE_FUNCTION(goToLunch)
	DECLARE_FUNCTION(haveLunch)
	DECLARE_FUNCTION(leaveLunch)
	DECLARE_FUNCTION(closedCompartment)
	DECLARE_FUNCTION(goSalon1)
	DECLARE_FUNCTION(inSalon1)
	DECLARE_FUNCTION(goCompartment)
	DECLARE_FUNCTION(openCompartment2)
	DECLARE_FUNCTION(goWander)
	DECLARE_FUNCTION(goSalon2)
	DECLARE_FUNCTION(inSalon2)
	DECLARE_FUNCTION(goCompartment3)
	DECLARE_FUNCTION(openCompartment3)
	DECLARE_FUNCTION(goSalon3)
	DECLARE_FUNCTION(inSalon3)
	DECLARE_FUNCTION(withAugust)
	DECLARE_FUNCTION(goCompartment4)
	DECLARE_FUNCTION(inCompartment4)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_VFUNCTION(chapter4)

	/**
	 * Search Cath by walking around
	 *
	 * @param car            The car
	 * @param entityPosition The entity position
	 */
	DECLARE_FUNCTION_2(doWalkSearchingForCath, CarIndex car, EntityPosition position)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(chapter4Handler)
	DECLARE_FUNCTION(leaveDinner)
	DECLARE_FUNCTION(inCompartment)
	DECLARE_FUNCTION(function44)
	DECLARE_FUNCTION(conferring)
	DECLARE_FUNCTION(goSalon4)
	DECLARE_FUNCTION(beforeBomb)
	DECLARE_FUNCTION(afterBomb)
	DECLARE_FUNCTION(catchCath)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_VFUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)
	DECLARE_FUNCTION(function52)
	DECLARE_FUNCTION(runningTrain)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ABBOT_H
