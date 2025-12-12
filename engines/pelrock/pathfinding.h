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
#ifndef PELROCK_PATHFINDING_H
#define PELROCK_PATHFINDING_H

#include "common/scummsys.h"
#include "graphics/screen.h"

#include "pelrock/types.h"

namespace Pelrock {
bool findPath(int sourceX, int sourceY, int targetX, int targetY, Common::Array<WalkBox> &walkboxes, PathContext *context);

Common::Point calculateWalkTarget(Common::Array<WalkBox> &walkboxes, int x, int y);
uint8_t findWalkboxForPoint(Common::Array<WalkBox> &walkboxes, uint16_t x, uint16_t y);
uint8_t getAdjacentWalkbox(Common::Array<WalkBox> &walkboxes, uint8_t current_box_index);
uint16_t buildWalkboxPath(Common::Array<WalkBox> &walkboxes, uint8_t start_box, uint8_t dest_box, uint8_t *path_buffer);
uint16_t generateMovementSteps(Common::Array<WalkBox> &walkboxes, uint8_t *path_buffer, uint16_t path_length, uint16_t start_x, uint16_t start_y, uint16_t dest_x, uint16_t dest_y, MovementStep *movement_buffer);
bool isPointInWalkbox(WalkBox *box, uint16_t x, uint16_t y);
void clearVisitedFlags(Common::Array<WalkBox> &walkboxes);

} // End of namespace Pelrock

#endif // PELROCK_PATHFINDING_H
