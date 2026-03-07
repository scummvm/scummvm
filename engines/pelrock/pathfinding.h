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
bool findPath(int sourceX, int sourceY, int targetX, int targetY, Common::Array<WalkBox> &walkboxes, PathContext *context, HotSpot *hotspot = nullptr);

/**
 * Calculate the walk target point based on source coordinates and mouse hover state.
 * @param walkboxes         Array of walkboxes in the current room.
 * @param sourceX           X coordinate of the source point (e.g., mouse position).
 * @param sourceY           Y coordinate of the source point (e.g., mouse position).
 * @param mouseHoverState   State indicating what the mouse is hovering over (0 = nothing, 1 = hotspot hover, 2 = hotspot click).
 * @param hotspot           Pointer to the hotspot being hovered over (if applicable).
 * @return                  Calculated walk target point.
 */
Common::Point calculateWalkTarget(Common::Array<WalkBox> &walkboxes, int sourceX, int sourceY, bool mouseHoverState, HotSpot *hotspot);
byte findWalkboxForPoint(Common::Array<WalkBox> &walkboxes, uint16 x, uint16 y);
byte getAdjacentWalkbox(Common::Array<WalkBox> &walkboxes, byte current_box_index);
uint16 buildWalkboxPath(Common::Array<WalkBox> &walkboxes, byte start_box, byte dest_box, byte *path_buffer);
uint16 generateMovementSteps(Common::Array<WalkBox> &walkboxes, byte *path_buffer, uint16 path_length, uint16 start_x, uint16 start_y, uint16 dest_x, uint16 dest_y, MovementStep *movement_buffer);
bool isPointInWalkbox(WalkBox *box, uint16 x, uint16 y);
void clearVisitedFlags(Common::Array<WalkBox> &walkboxes);

} // End of namespace Pelrock

#endif // PELROCK_PATHFINDING_H
