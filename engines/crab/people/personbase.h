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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_PERSONBASE_H
#define CRAB_PERSONBASE_H

#include "crab/stat/Stat.h"

namespace Crab {

namespace pyrodactyl {
namespace people {
// What each type is, and what it does
// neutral - peaceful person who will fight back on being attacked
// hostile - person who will attack you on sight
// coward - person who will flee on being attacked
// immobile - person who cannot move but can be killed
enum PersonType {
	PE_NEUTRAL,
	PE_HOSTILE,
	PE_COWARD,
	PE_IMMOBILE
};

// What each state is, and what it does
// normal - person doing his default movement
// fight - person fighting you
// flee - person running away from you
// ko - person is dead/knocked out
// dying - play the dying animation
enum PersonState {
	PST_NORMAL,
	PST_FIGHT,
	PST_FLEE,
	PST_KO,
	PST_DYING
};

PersonType stringToPersonType(const Common::String &val);
PersonState stringToPersonState(const Common::String &val);
} // End of namespace people
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_PERSONBASE_H
