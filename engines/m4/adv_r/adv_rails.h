
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

#ifndef M4_ADV_R_ADV_RAILS_H
#define M4_ADV_R_ADV_RAILS_H

#include "m4/m4_types.h"
#include "m4/adv_r/adv.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

struct Rails_Globals {
	railNode **myNodes = nullptr;
	int16 *myEdges = nullptr;

	railNode **stackBottom = nullptr, **stackTop = nullptr;
	noWalkRect *noWalkRectList = nullptr;

	int32 memtypePATHN = 0;
};

extern bool InitRails();
extern void rail_system_shutdown();
extern void ClearRails();
noWalkRect *intr_add_no_walk_rect(int32 x1, int32 y1, int32 x2, int32 y2, int32 altX, int32 altY, Buffer *walkCodes);
extern void intr_move_no_walk_rect(noWalkRect *myRect, int32 new_x1, int32 new_y1,
	int32 new_x2, int32 new_y2, int32 new_altX, int32 new_altY, Buffer *walkCodes);
extern void intr_remove_no_walk_rect(noWalkRect *myRect, Buffer *walkCodes);
extern void CreateEdge(int32 node1, int32 node2, Buffer *walkCodes);
extern void RestoreNodeEdges(int32 nodeID, Buffer *walkCodes);
extern void RestoreEdgeList(Buffer *walkCodes);
extern int32 AddRailNode(int32 x, int32 y, Buffer *walkCodes, bool restoreEdges);
extern void MoveRailNode(int32 nodeID, int32 x, int32 y, Buffer *walkCodes, bool restoreEdges);
extern bool RemoveRailNode(int32 nodeID, Buffer *walkCodes, bool restoreEdges);
extern bool RailNodeExists(int32 nodeID, int32 *nodeX, int32 *nodeY);
extern int16 GetEdgeLength(int32 node1, int32 node2);
extern bool GetShortestPath(int32 origID, int32 destID, railNode **shortPath);
extern railNode *CreateCustomPath(int32 coord, ...);
extern void DisposePath(railNode *pathStart);
extern bool intr_LineCrossesRect(int32 line_x1, int32 line_y1, int32 line_x2, int32 line_y2,
	int32 rect_x1, int32 rect_y1, int32 rect_x2, int32 rect_y2);
extern bool intr_LinesCross(int32 line1_x1, int32 line1_y1, int32 line1_x2, int32 line1_y2,
	int32 line2_x1, int32 line2_y1, int32 line2_x2, int32 line2_y2);
extern bool intr_PathCrossesLine(int32 startX, int32 startY, railNode *pathStart,
	int32 line_x1, int32 line_y1, int32 line_x2, int32 line_y2);

} // End of namespace M4

#endif
