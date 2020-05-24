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

#ifndef LASTEXPRESS_COOKS_H
#define LASTEXPRESS_COOKS_H

#include "lastexpress/entities/entity.h"

namespace LastExpress {

class LastExpressEngine;

class Cooks : public Entity {
public:
	Cooks(LastExpressEngine *engine);
	~Cooks() override {}

	/**
	 * Draws the entity
	 *
	 * @param sequence The sequence to draw
	 */
	DECLARE_FUNCTION_1(draw, const char *sequence)

	/**
	 * Plays sound
	 *
	 * @param filename The sound filename
	 */
	DECLARE_VFUNCTION_1(playSound, const char *filename)

	DECLARE_FUNCTION(uptrainVersion)

	DECLARE_FUNCTION(downtrainVersion)

	/**
	 * Setup Chapter 1
	 */
	DECLARE_VFUNCTION(chapter1)

	/**
	 * Chapter 1: Prepare dinner in kitchen
	 */
	DECLARE_FUNCTION(inKitchenDinner)

	DECLARE_FUNCTION(lockUp)

	/**
	 * Setup Chapter 2
	 */
	DECLARE_VFUNCTION(chapter2)

	/*
	 * Chapter 2: Prepare breakfast in kitchen
	 */
	DECLARE_FUNCTION(inKitchenBreakfast)

	/**
	 * Setup Chapter 3
	 */
	DECLARE_VFUNCTION(chapter3)

	/**
	 * Chapter 3: Prepare lunch in kitchen
	 */
	DECLARE_FUNCTION(inKitchenLunch)

	/**
	 * Setup Chapter 4
	 */
	DECLARE_VFUNCTION(chapter4)

	/**
	 * Chapter 4: Prepare second dinner in kitchen
	 */
	DECLARE_FUNCTION(inKitchenDinner2)

	/**
	 * Setup Chapter 5
	 */
	DECLARE_VFUNCTION(chapter5)
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_COOKS_H
