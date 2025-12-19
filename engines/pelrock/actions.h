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
#ifndef PELROCK_ACTIONS_H
#define PELROCK_ACTIONS_H

#include "pelrock/types.h"
#include "pelrock/pelrock.h"

namespace Pelrock {

const int WILDCARD = -1;

struct ActionEntry {
    int hotspotExtra;
    VerbIcon action;
    void (PelrockEngine::*handler)(HotSpot *);
};

// Action table for all rooms
extern const ActionEntry actionTable[];

} // End of namespace Pelrock
#endif // PELROCK_ACTIONS_H
