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

#ifndef LASTEXPRESS_VESNA_H
#define LASTEXPRESS_VESNA_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Vesna : public Entity {
public:
	Vesna(LastExpressEngine *engine);
	~Vesna() override {}

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
	 * Handles entering/exiting a compartment.
	 *
	 * @param sequence    The sequence to draw
	 * @param compartment The compartment
	 */
	DECLARE_VFUNCTION_2(enterExitCompartment, const char *sequence, ObjectIndex compartment)

	/**
	 * Draws the entity
	 *
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(draw, const char *sequence)

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

	/**
	 * Updates the entity
	 *
	 * @param car            The car
	 * @param entityPosition The entity position
	 */
	DECLARE_FUNCTION_2(updateEntity2, CarIndex car, EntityPosition entityPosition)

	/**
	 * Process callback action when somebody is standing in the restaurant or salon.
	 */
	DECLARE_FUNCTION(callbackActionRestaurantOrSalon)

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 */
	DECLARE_FUNCTION(callbackActionOnDirection)

	/**
	 * Saves the game
	 *
	 * @param savegameType The type of the savegame
	 * @param param        The param for the savegame (EventIndex or TimeValue)
	 */
	DECLARE_VFUNCTION_2(savegame, SavegameType savegameType, uint32 param)

	DECLARE_FUNCTION(homeAlone)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_VFUNCTION(chapter1)

	DECLARE_FUNCTION(withMilos)
	DECLARE_FUNCTION(homeTogether)
	DECLARE_FUNCTION(function15)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_VFUNCTION(chapter2)

	/**
	 * Handle Chapter 2 events
	 */
	DECLARE_FUNCTION(chapter2Handler)

	DECLARE_FUNCTION(checkTrain)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_VFUNCTION(chapter3)

	DECLARE_FUNCTION(inCompartment)
	DECLARE_FUNCTION(takeAWalk)
	DECLARE_FUNCTION(killAnna)
	DECLARE_FUNCTION(killedAnna)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_VFUNCTION(chapter4)

	DECLARE_FUNCTION(exitLocation)
	DECLARE_FUNCTION(done)
	DECLARE_FUNCTION(function27)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_VFUNCTION(chapter5)

	DECLARE_FUNCTION(guarding)
	DECLARE_FUNCTION(climbing)
	DECLARE_NULL_FUNCTION()
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_VESNA_H
