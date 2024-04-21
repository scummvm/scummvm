/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef ENABLE_MADSV2

#ifndef MADS_GLOBALS_FOREST_H
#define MADS_GLOBALS_FOREST_H

#include "mads/game.h"
#include "mads/resources.h"

namespace MADS {

namespace Forest {

enum GlobalId {
	// Global variables

	kWalkerTiming           = 0,
	kWalkerTiming2			= 1
	};

class ForestGlobals : public Globals {
public:
	SynchronizedList _spriteIndexes;
	SynchronizedList _sequenceIndexes;
	SynchronizedList _animationIndexes;
public:
	/**
	 * Constructor
	 */
	ForestGlobals();

	/**
	* Synchronize the globals data
	*/
	virtual void synchronize(Common::Serializer &s);
};

} // End of namespace Forest

} // End of namespace MADS

#endif /* MADS_GLOBALS_FOREST_H */

#endif
