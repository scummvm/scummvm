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

#ifndef LASTEXPRESS_TATIANA_H
#define LASTEXPRESS_TATIANA_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Tatiana : public Entity {
public:
	Tatiana(LastExpressEngine *engine);
	~Tatiana() override {}

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
	 * Handles entering/exiting a compartment and updates position/play animation
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_FUNCTION_2(enterExitCompartment2, const char *sequence, ObjectIndex compartment)

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
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

	/**
	 * Updates parameter 2 using ticks value
	 *
	 * @param savepoint The savepoint
	 *                    - ticks to add
	 */
	DECLARE_FUNCTION_NOSETUP(updateFromTicks)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	/**
	 * Process callback action when somebody is standing in the restaurant or salon.
	 */
	DECLARE_FUNCTION(callbackActionRestaurantOrSalon)

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

	DECLARE_FUNCTION(enterCompartment)
	DECLARE_FUNCTION(exitCompartment)
	DECLARE_FUNCTION_1(handleCompartment, uint32)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_VFUNCTION(chapter1)

	DECLARE_FUNCTION(function18)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(exitDining)
	DECLARE_FUNCTION(returnToCompartment)
	DECLARE_FUNCTION(getSomeAir)
	DECLARE_FUNCTION(returnToCompartmentAgain)
	DECLARE_FUNCTION(function24)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_VFUNCTION(chapter2)

	/**
	 * Handle Chapter 2 events
	 */
	DECLARE_FUNCTION(chapter2Handler)

	DECLARE_FUNCTION(breakfastClick)
	DECLARE_FUNCTION(joinAlexei)
	DECLARE_FUNCTION(leaveBreakfast)
	DECLARE_FUNCTION(returnToCompartment2)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_VFUNCTION(chapter3)

	DECLARE_FUNCTION(playChess)
	DECLARE_FUNCTION(returnToCompartment3)
	DECLARE_FUNCTION(beforeConcert)
	DECLARE_FUNCTION(concert)
	DECLARE_FUNCTION(leaveConcert)
	DECLARE_FUNCTION(afterConcert)
	DECLARE_FUNCTION(cryAnna)
	DECLARE_FUNCTION(function39)
	DECLARE_FUNCTION(function40)
	DECLARE_FUNCTION(trapCath)

	/**
	 * ???
	 *
	 * @param car            The car
	 * @param entityPosition The entity position
	 */
	DECLARE_FUNCTION_2(function42, CarIndex car, EntityPosition entityPosition)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_VFUNCTION(chapter4)

	DECLARE_FUNCTION(inCompartment4)
	DECLARE_FUNCTION(meetAlexei)
	DECLARE_FUNCTION(withAlexei)
	DECLARE_FUNCTION(thinking)
	DECLARE_FUNCTION(seekCath)
	DECLARE_FUNCTION(function49)
	DECLARE_FUNCTION(alexeiDead)
	DECLARE_FUNCTION(function51)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_VFUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(chapter5Handler)

	DECLARE_FUNCTION(autistic)
	DECLARE_FUNCTION(function55)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_TATIANA_H
