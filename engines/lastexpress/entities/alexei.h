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

#ifndef LASTEXPRESS_ALEXEI_H
#define LASTEXPRESS_ALEXEI_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Alexei : public Entity {
public:
	Alexei(LastExpressEngine *engine);
	~Alexei() override {}

	/**
	 * Resets the entity
	 */
	DECLARE_FUNCTION(reset)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_VFUNCTION_1(playSound, const char *filename)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	/**
	 * Draws the entity
	 *
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(draw, const char *sequence)

	/**
	 * Updates the position
	 *
	 * @param sequence1      The sequence to draw
	 * @param car            The car
	 * @param position       The position
	 */
	DECLARE_FUNCTION_3(updatePosition, const char *sequence1, CarIndex car, Position position)

	/**
	 * Handles entering/exiting a compartment.
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_VFUNCTION_2(enterExitCompartment, const char *sequence, ObjectIndex compartment)

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

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
	 * Draws the entity along with another one
	 *
	 * @param savepoint   The savepoint
	 *                      - The sequence to draw
	 *                      - The sequence to draw for the second entity
	 *                      - The EntityIndex of the second entity
	 */
	DECLARE_FUNCTION_NOSETUP(draw2)

	/**
	 * Process callback action when somebody is standing in the restaurant or salon.
	 */
	DECLARE_FUNCTION(callbackActionRestaurantOrSalon)

	DECLARE_FUNCTION(enterComparment)
	DECLARE_FUNCTION(exitCompartment)
	DECLARE_FUNCTION(pacingAtWindow)

	/**
	 * ???
	 *
	 * @param timeValue          The time value
	 * @param sequence           The sequence to draw
	 */
	DECLARE_FUNCTION_2(compartmentLogic, TimeValue timeValue, const char *sequence)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_VFUNCTION(chapter1)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(atDinner)
	DECLARE_FUNCTION(returnCompartment)
	DECLARE_FUNCTION(goSalon)
	DECLARE_FUNCTION(sitting)
	DECLARE_FUNCTION(standingAtWindow)
	DECLARE_FUNCTION(waitingForTatiana)
	DECLARE_FUNCTION(upset)
	DECLARE_FUNCTION(returnCompartmentNight)
	DECLARE_FUNCTION(function26)
	DECLARE_FUNCTION(function27)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_VFUNCTION(chapter2)

	/**
	 * Handle Chapter 2 events
	 */
	DECLARE_FUNCTION(inCompartment2)
	DECLARE_FUNCTION(atBreakfast)
	DECLARE_FUNCTION(returnCompartment2)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_VFUNCTION(chapter3)

	/**
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION(playingChess)
	DECLARE_FUNCTION(inPart3)
	DECLARE_FUNCTION(pacing3)
	DECLARE_FUNCTION(goSalon3)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_VFUNCTION(chapter4)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(inCompartment4)
	DECLARE_FUNCTION(meetTatiana)
	DECLARE_FUNCTION(leavePlatform)
	DECLARE_FUNCTION(inCompartmentAgain)
	DECLARE_FUNCTION(goSalon4)
	DECLARE_FUNCTION(pacing)
	DECLARE_FUNCTION(goToPlatform)
	DECLARE_FUNCTION(returnCompartment4)
	DECLARE_FUNCTION(bombPlanB)
	DECLARE_FUNCTION(function47)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_VFUNCTION(chapter5)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ALEXEI_H
