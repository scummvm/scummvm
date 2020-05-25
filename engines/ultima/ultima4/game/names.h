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

#ifndef ULTIMA4_GAME_NAMES_H
#define ULTIMA4_GAME_NAMES_H

#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/map/direction.h"

namespace Ultima {
namespace Ultima4 {

/*
 * These routines convert the various enumerations for classes, reagents,
 * etc. into the textual representations used in the game.
 */
const char *getClassName(ClassType klass);
const char *getReagentName(Reagent reagent);
const char *getVirtueName(Virtue virtue);
const char *getBaseVirtueName(int virtueMask);
int getBaseVirtues(Virtue virtue);
const char *getVirtueAdjective(Virtue virtue);
const char *getStoneName(Virtue virtue);
const char *getItemName(Item item);
const char *getDirectionName(Direction dir);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
