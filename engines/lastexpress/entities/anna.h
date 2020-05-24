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

#ifndef LASTEXPRESS_ANNA_H
#define LASTEXPRESS_ANNA_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Anna : public Entity {
public:
	Anna(LastExpressEngine *engine);
	~Anna() override {}

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
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_VFUNCTION_1(playSound, const char *filename)

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

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	DECLARE_FUNCTION(practiceMusic)

	/**
	 * Draws the entity along with another one
	 *
	 * @param sequence1   The sequence to draw
	 * @param sequence2   The sequence to draw for the second entity
	 * @param entity      The EntityIndex of the second entity
	 */
	DECLARE_FUNCTION_3(draw2, const char *sequence1, const char *sequence2, EntityIndex entity)

	/**
	 * Updates parameter 2 using ticks value
	 *
	 * @param ticks The number of ticks to add
	 */
	DECLARE_FUNCTION_1(updateFromTicks, uint32 ticks)

	DECLARE_FUNCTION_2(compartmentLogic, TimeValue timeValue, const char *sequence)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_VFUNCTION(chapter1)

	DECLARE_FUNCTION_2(doWalkP1, uint32, uint32)

	DECLARE_FUNCTION_1(diningLogic, TimeValue timeValue)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(fleeTyler)
	DECLARE_FUNCTION(waitDinner)
	DECLARE_FUNCTION(goDinner)
	DECLARE_FUNCTION(function22)
	DECLARE_FUNCTION(waitingDinner)
	DECLARE_FUNCTION(waitingDinner2)
	DECLARE_FUNCTION(eatingDinner)
	DECLARE_FUNCTION(leaveDinner)
	DECLARE_FUNCTION(freshenUp)
	DECLARE_FUNCTION(goSalon)
	DECLARE_FUNCTION(waitAugust)
	DECLARE_FUNCTION(function30)
	DECLARE_FUNCTION(leaveAugust)
	DECLARE_FUNCTION(returnCompartment)
	DECLARE_FUNCTION(readyForBed)
	DECLARE_FUNCTION(asleep)
	DECLARE_FUNCTION(wakeNight)
	DECLARE_FUNCTION(goVassili)
	DECLARE_FUNCTION(function37)
	DECLARE_FUNCTION(speakTatiana)
	DECLARE_FUNCTION_2(doWalk1019, CarIndex car, EntityPosition entityPosition)
	DECLARE_FUNCTION(leaveTatiana)
	DECLARE_FUNCTION(goBackToSleep)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_VFUNCTION(chapter2)

	/**
	 * Handle Chapter 2 events
	 */
	DECLARE_FUNCTION(inPart2)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_VFUNCTION(chapter3)

	/**
	 * Handle Chapter 3 events
	 */
	DECLARE_FUNCTION_1(exitCompartment, bool useAction1)
	DECLARE_FUNCTION(practicing)
	DECLARE_FUNCTION(goLunch)
	DECLARE_FUNCTION(lunch)
	DECLARE_FUNCTION(leaveTableWithAugust)
	DECLARE_FUNCTION(leaveLunch)
	DECLARE_FUNCTION(afterLunch)
	DECLARE_FUNCTION(returnCompartment3)
	DECLARE_FUNCTION(dressing)
	DECLARE_FUNCTION(giveMaxToConductor2)
	DECLARE_FUNCTION(goConcert)
	DECLARE_FUNCTION(concert)
	DECLARE_FUNCTION(leaveConcert)
	DECLARE_FUNCTION(leaveConcertCathInCompartment)
	DECLARE_FUNCTION(afterConcert)
	DECLARE_FUNCTION(giveMaxBack)
	DECLARE_FUNCTION(goBaggageCompartment)
	DECLARE_FUNCTION(function62)
	DECLARE_FUNCTION(deadBaggageCompartment)
	DECLARE_FUNCTION(baggageFight)
	DECLARE_FUNCTION(prepareVienna)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_VFUNCTION(chapter4)

	/**
	 * Handle Chapter 4 events
	 */
	DECLARE_FUNCTION(reading)
	DECLARE_FUNCTION(sulking)
	DECLARE_FUNCTION(goSalon4)
	DECLARE_FUNCTION(returnCompartment4)
	DECLARE_FUNCTION(enterCompartmentCathFollowsAnna)
	DECLARE_FUNCTION_2(doWalkCathFollowsAnna, CarIndex car, EntityPosition entityPosition)
	DECLARE_FUNCTION(letDownHair)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_VFUNCTION(chapter5)

	/**
	 * Handle Chapter 5 events
	 */
	DECLARE_FUNCTION(tiedUp)
	DECLARE_FUNCTION(function76)
	DECLARE_FUNCTION(readyToScore)
	DECLARE_FUNCTION(kidnapped)
	DECLARE_FUNCTION(waiting)
	DECLARE_FUNCTION(finalSequence)
	DECLARE_FUNCTION(openFirebird)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ANNA_H
