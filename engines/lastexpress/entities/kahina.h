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

#ifndef LASTEXPRESS_KAHINA_H
#define LASTEXPRESS_KAHINA_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Kahina : public Entity {
public:
	Kahina(LastExpressEngine *engine);
	~Kahina() override {}

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
	 * Saves the game
	 *
	 * @param savegameType The type of the savegame
	 * @param param        The param for the savegame (EventIndex or TimeValue)
	 */
	DECLARE_VFUNCTION_2(savegame, SavegameType savegameType, uint32 param)

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param time The time to add
	 */
	DECLARE_FUNCTION_1(updateFromTime, uint32 time)

	/**
	 * Updates parameter 2 using ticks value
	 *
	 * @param savepoint The savepoint
	 *                    - ticks to add
	 */
	DECLARE_FUNCTION_NOSETUP(updateFromTicks)

	DECLARE_FUNCTION_1(lookingForCath, TimeValue timeValue)

	/**
	 * Updates the entity
	 *
	 * @param car            The car
	 * @param entityPosition The entity position
	 */
	DECLARE_FUNCTION_2(updateEntity2, CarIndex car, EntityPosition entityPosition)

	/**
	 * Updates the entity
	 *
	 * @param car            The car
	 * @param entityPosition The entity position
	 */
	DECLARE_VFUNCTION_2(updateEntity, CarIndex car, EntityPosition entityPosition)

	/**
	 * Handles entering/exiting a compartment.
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_VFUNCTION_2(enterExitCompartment, const char *sequence, ObjectIndex compartment)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_VFUNCTION(chapter1)

	/**
	 * Handle Chapter 1 events
	 */
	DECLARE_FUNCTION(chapter1Handler)

	DECLARE_FUNCTION(awaitingCath)
	DECLARE_FUNCTION(cathDone)
	DECLARE_FUNCTION(function14)
	DECLARE_FUNCTION(searchTrain)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_VFUNCTION(chapter2)
	DECLARE_FUNCTION(inSeclusionPart2)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_VFUNCTION(chapter3)

	/**
	 * Update the entity, handling excuse me events and resetting the entity state after the argument with Anna in the baggage car
	 *
	 * @param car            The car index
	 * @param entityPosition The entity position
	 */
	DECLARE_FUNCTION_2(function19, CarIndex car, EntityPosition entityPosition)

	DECLARE_FUNCTION(beforeConcert)
	DECLARE_FUNCTION(concert)
	DECLARE_FUNCTION(finished)
	DECLARE_FUNCTION(findFirebird)
	DECLARE_FUNCTION(seekCath)
	DECLARE_FUNCTION(searchCath)
	DECLARE_FUNCTION(searchTatiana)
	DECLARE_FUNCTION(killCathAnywhere)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_VFUNCTION(chapter4)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_VFUNCTION(chapter5)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_KAHINA_H
