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

#include "m4/adv_r/adv_rails.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/mem/mem.h"
#include "m4/vars.h"

namespace M4 {

#define TOP_EDGE    0x01
#define LEFT_EDGE   0x02
#define BOTTOM_EDGE 0x04
#define RIGHT_EDGE  0x08

bool InitRails() {
	int32 i, edgeTableSize;

	// Register with the stash the frequently used structs
	if (!mem_register_stash_type(&_G(rails).memtypePATHN, sizeof(pathNode), 32, "+PATHNODE")) {
		return false;
	}

	// Create the stack. Since any path through a series of nodes can have at most MAXRAILNODES...
	if ((_G(rails).stackBottom = (railNode **)mem_alloc(sizeof(railNode *) * MAXRAILNODES, STR_RAILNODE)) == nullptr) {
		return false;
	}

	// Allocate the array of railNode pointers and initialize...
	if ((_G(rails).myNodes = (railNode **)mem_alloc(sizeof(railNode *) * MAXRAILNODES, STR_RAILNODE)) == nullptr) {
		return false;
	}

	for (i = 0; i < MAXRAILNODES; i++) {
		_G(rails).myNodes[i] = nullptr;
	}

	// Calculate the size of the edge table, allocate, and initialize
	// The edge table stores the upper triangle of a square matrix.
	edgeTableSize = (MAXRAILNODES * (MAXRAILNODES - 1)) >> 1;
	if ((_G(rails).myEdges = (int16 *)mem_alloc(sizeof(int16) * edgeTableSize, "edge table")) == nullptr) {
		return false;
	}

	for (i = 0; i < edgeTableSize; i++) {
		_G(rails).myEdges[i] = 0;
	}

	// Set the parameters and return
	_G(rails).noWalkRectList = nullptr;

	return true;
}


void rail_system_shutdown(void) {
	if (_G(rails).stackBottom) {
		mem_free(_G(rails).stackBottom);
		_G(rails).stackBottom = nullptr;
	}

	ClearRails();

	if (_G(rails).myNodes) {
		mem_free(_G(rails).myNodes);
		_G(rails).myNodes = nullptr;
	}
	if (_G(rails).myEdges) {
		mem_free(_G(rails).myEdges);
		_G(rails).myEdges = nullptr;
	}
}


void ClearRails(void) {
	int32 i, edgeTableSize;
	noWalkRect *myRect;

	if (_G(rails).myNodes) {
		for (i = 0; i < MAXRAILNODES; i++) {
			if (_G(rails).myNodes[i]) {
				mem_free((void *)_G(rails).myNodes[i]);
				_G(rails).myNodes[i] = nullptr;
			}
		}
	}

	if (_G(rails).myEdges) {
		edgeTableSize = (MAXRAILNODES * (MAXRAILNODES - 1)) >> 1;
		for (i = 0; i < edgeTableSize; i++) {
			_G(rails).myEdges[i] = 0;
		}
	}

	// Now turf the noWalkRectList
	myRect = _G(rails).noWalkRectList;
	while (myRect) {
		_G(rails).noWalkRectList = _G(rails).noWalkRectList->next;
		mem_free((void *)myRect);
		myRect = _G(rails).noWalkRectList;
	}
}


noWalkRect *intr_add_no_walk_rect(int32 x1, int32 y1, int32 x2, int32 y2, int32 altX, int32 altY, Buffer *walkCodes) {
	noWalkRect *newRect;

	// Parameter verification
	if ((x2 < x1) || (y2 < y1)) {
		return nullptr;
	}

	// Create new noWalkRect structure
	if ((newRect = (noWalkRect *)mem_alloc(sizeof(noWalkRect), "intr noWalkRect")) == nullptr) {
		error_show(FL, 'IADN', "rect size: %d %d %d %d", x1, y1, x2, y2);
		return nullptr;
	}

	// Initialize the new rect
	newRect->x1 = x1;
	newRect->y1 = y1;
	newRect->x2 = x2;
	newRect->y2 = y2;

	// Add the alternate walkto node - this node must exist
	if ((newRect->alternateWalkToNode = AddRailNode(altX, altY, walkCodes, false)) < 0) {
		error_show(FL, 'IADN', "could not add node. coord: %d %d", altX, altY);
	}

	// Now add the corner nodes.  Not as important if these don't exist
	newRect->walkAroundNode1 = AddRailNode(x1 - 1, y1 - 1, walkCodes, false);
	newRect->walkAroundNode2 = AddRailNode(x2 + 1, y1 - 1, walkCodes, false);
	newRect->walkAroundNode3 = AddRailNode(x2 + 1, y2 + 1, walkCodes, false);
	newRect->walkAroundNode4 = AddRailNode(x1 - 1, y2 + 1, walkCodes, false);

	// Now link the rectangle into the list
	newRect->prev = nullptr;
	newRect->next = _G(rails).noWalkRectList;

	if (_G(rails).noWalkRectList) {
		_G(rails).noWalkRectList->prev = newRect;
	}
	_G(rails).noWalkRectList = newRect;

	// Now refresh all the edges in case the noWalkRect is blocking edges
	RestoreEdgeList(walkCodes);

	return newRect;
}

noWalkRect *intr_add_no_walk_rect(int32 x1, int32 y1, int32 x2, int32 y2, int32 altX, int32 altY) {
	return intr_add_no_walk_rect(x1, y1, x2, y2, altX, altY, _G(screenCodeBuff)->get_buffer());
}

void intr_move_no_walk_rect(noWalkRect *myRect, int32 new_x1, int32 new_y1,
	int32 new_x2, int32 new_y2, int32 new_altX, int32 new_altY, Buffer *walkCodes) {
	if (!myRect) {
		return;
	}

	// Set the new values into the rect
	myRect->x1 = new_x1;
	myRect->y1 = new_y1;
	myRect->x2 = new_x2;
	myRect->y2 = new_y2;

	// Now move the nodes
	MoveRailNode(myRect->alternateWalkToNode, new_altX, new_altY, walkCodes, false);
	MoveRailNode(myRect->walkAroundNode1, new_x1 - 1, new_y1 - 1, walkCodes, false);
	MoveRailNode(myRect->walkAroundNode2, new_x2 + 1, new_y1 - 1, walkCodes, false);
	MoveRailNode(myRect->walkAroundNode3, new_x2 + 1, new_y2 - 1, walkCodes, false);
	MoveRailNode(myRect->walkAroundNode4, new_x1 - 1, new_y2 - 1, walkCodes, false);

	// Now refresh all the edges
	RestoreEdgeList(walkCodes);
}


void intr_remove_no_walk_rect(noWalkRect *myRect, Buffer *walkCodes) {
	// Parameter verification
	if (!myRect) {
		return;
	}

	// Remove myRect from the list
	if (myRect->prev) {
		myRect->prev->next = myRect->next;
	} else {
		_G(rails).noWalkRectList = myRect->next;
	}
	if (myRect->next) {
		myRect->next->prev = myRect->prev;
	}

	// Remove the railNodes
	RemoveRailNode(myRect->alternateWalkToNode, walkCodes, false);
	RemoveRailNode(myRect->walkAroundNode1, walkCodes, false);
	RemoveRailNode(myRect->walkAroundNode2, walkCodes, false);
	RemoveRailNode(myRect->walkAroundNode3, walkCodes, false);
	RemoveRailNode(myRect->walkAroundNode4, walkCodes, false);

	// Turf myRect
	mem_free(myRect);

	// Now refresh all the edges
	RestoreEdgeList(walkCodes);
}

void intr_remove_no_walk_rect(noWalkRect *myRect) {
	intr_remove_no_walk_rect(myRect, _G(screenCodeBuff)->get_buffer());
}

bool intr_LineCrossesRect(int32 line_x1, int32 line_y1, int32 line_x2, int32 line_y2,
	int32 rect_x1, int32 rect_y1, int32 rect_x2, int32 rect_y2) {
	int32 p1X, p1Y, p2X, p2Y, mX, mY;
	uint8 endCode1, endCode2, midCode;
	bool finished;

	// Ensure we have a valid rectangle
	if ((rect_x1 > rect_x2) || (rect_y1 > rect_y2)) {
		return false;
	}

	// Make copies of x1, y1, x2, y2
	p1X = line_x1;
	p1Y = line_y1;
	p2X = line_x2;
	p2Y = line_y2;

	// Calculate the cohen sutherland codes for the endpoints of the line
	// For (p1X, p1Y)
	endCode1 = 0;
	if (p1X < rect_x1) {
		endCode1 = LEFT_EDGE;
	} else if (p1X > rect_x2) {
		endCode1 = RIGHT_EDGE;
	}
	if (p1Y < rect_y1) {
		endCode1 |= TOP_EDGE;
	} else if (p1Y > rect_y2) {
		endCode1 |= BOTTOM_EDGE;
	}

	// For (p2X, p2Y)
	endCode2 = 0;
	if (p2X < rect_x1) {
		endCode2 = LEFT_EDGE;
	} else if (p2X > rect_x2) {
		endCode2 = RIGHT_EDGE;
	}
	if (p2Y < rect_y1) {
		endCode2 |= TOP_EDGE;
	} else if (p2Y > rect_y2) {
		endCode2 |= BOTTOM_EDGE;
	}

	// If either endCode is 0, that point is inside the rect, therefore the line intersects
	if ((!endCode1) || (!endCode2)) {
		return true;
	}

	finished = false;
	while (!finished) {
		// If both have a bit set in common, then the line segment is completely off one edge
		if (endCode1 & endCode2) {
			finished = true;
		}

		// Calculate the mid point of the line segment
		mX = (p1X + p2X) >> 1;
		mY = (p1Y + p2Y) >> 1;

		// Because the midpoint is an integer (round-off err), make sure it isn't the same
		// as one of the two endpoints.
		if (((mX == p1X) && (mY == p1Y)) || ((mX == p2X) && (mY == p2Y))) {
			return false;
		}

		// Calculate the cohen sutherland codes for the midpoint of the line segment
		midCode = 0;
		if (mX < rect_x1) {
			midCode = LEFT_EDGE;
		} else if (mX > rect_x2) {
			midCode = RIGHT_EDGE;
		}
		if (mY < rect_y1) {
			midCode |= TOP_EDGE;
		} else if (mY > rect_y2) {
			midCode |= BOTTOM_EDGE;
		}

		if (!midCode) {
			return true;
		}

		// Else the midCode and one of the end points must form a line segment completely off one edge
		else if (midCode & endCode1) {
			// Setting endpoint1 to the midpoint throws away that half of the line segment
			p1X = mX;
			p1Y = mY;
			endCode1 = midCode;
		} else {
			// Ditto for endpoint 2
			p2X = mX;
			p2Y = mY;
			endCode2 = midCode;
		}
	}

	// Got through without intersecting, therefore...
	return false;
}


static bool intr_LinePassesThroughRect(int32 x1, int32 y1, int32 x2, int32 y2) {
	noWalkRect *tempRect;
	bool intersected;

	// If there aren't any no-walk rects, no problem, return false
	if (!_G(rails).noWalkRectList) {
		return false;
	}

	// Loop through the _G(rails).noWalkRectList
	tempRect = _G(rails).noWalkRectList;
	intersected = false;
	while (tempRect && (!intersected)) {
		// See if the line passes through tempRect
		intersected = intr_LineCrossesRect(x1, y1, x2, y2, tempRect->x1, tempRect->y1, tempRect->x2, tempRect->y2);

		tempRect = tempRect->next;
	}

	return intersected;
}


bool intr_LinesCross(int32 line1_x1, int32 line1_y1, int32 line1_x2, int32 line1_y2,
	int32 line2_x1, int32 line2_y1, int32 line2_x2, int32 line2_y2) {
	bool intersected;
	int32 rectX1, rectY1, rectX2, rectY2;

	// The theory is that either line1 intersects the rectangle created by line2, and/or line2
	// intersects the rectangle reacted by line1.

	// Make sure both lines are listed left to right, top to bottom when passing in the coords as a rectangle
	rectX1 = imath_min(line1_x1, line1_x2);
	rectY1 = imath_min(line1_y1, line1_y2);
	rectX2 = imath_max(line1_x1, line1_x2);
	rectY2 = imath_max(line1_y1, line1_y2);

	intersected = intr_LineCrossesRect(line2_x1, line2_y1, line2_x2, line2_y2, rectX1, rectY1, rectX2, rectY2);
	if (intersected) {
		rectX1 = imath_min(line2_x1, line2_x2);
		rectY1 = imath_min(line2_y1, line2_y2);
		rectX2 = imath_max(line2_x1, line2_x2);
		rectY2 = imath_max(line2_y1, line2_y2);

		intersected = intr_LineCrossesRect(line1_x1, line1_y1, line1_x2, line1_y2, rectX1, rectY1, rectX2, rectY2);
	}

	return intersected;
}


void CreateEdge(int32 node1, int32 node2, Buffer *walkCodes) {
	int32 i, temp;
	int32 index;
	int32 x1, y1, x2, y2;
	int32 y_unit, x_unit, xdiff, ydiff, scanX, scanY, width, height, error_term, stride;
	bool valid, finished;
	frac16 deltaX, deltaY, distance;
	uint8 *walkCodePtr;

	// Check for nodes and edges
	if ((!_G(rails).myNodes) || (!_G(rails).myEdges)) {
		return;
	}
	if ((node1 < 0) || (node1 >= MAXRAILNODES) || (node2 < 0) || (node2 >= MAXRAILNODES)) {
		return;
	}
	if (node1 == node2) {
		return;
	}

	// Ensure node1 < node2
	if (node2 < node1) {
		temp = node1;
		node1 = node2;
		node2 = temp;
	}

	// If node1 is y and node2 is x, first find table entry ie. tableWidth * y + x, the subtract
	// n(n+1)/2  since only the upper triangle of the table is stored...
	index = (MAXRAILNODES - 1) * node1 + node2 - 1 - (node1 * (node1 + 1) >> 1);
	_G(rails).myEdges[index] = 0;
	valid = true;
	walkCodePtr = nullptr;
	finished = false;

	if ((!_G(rails).myNodes[node1]) || (!_G(rails).myNodes[node2]))
		return;
	x1 = _G(rails).myNodes[node1]->x;
	y1 = _G(rails).myNodes[node1]->y;
	x2 = _G(rails).myNodes[node2]->x;
	y2 = _G(rails).myNodes[node2]->y;

	// Ensure the algorithm is symmetric...
	if (x2 < x1) {
		temp = x1;
		x1 = x2;
		x2 = temp;
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	if (walkCodes && walkCodes->data) {
		// Initialize the buffer data pointer, the maximum dimensions of the buffer, and the scan x and y
		width = walkCodes->w;
		stride = walkCodes->stride;
		height = walkCodes->h;
		scanX = x1;
		scanY = y1;

		// Calculate the difference along the y-axis
		ydiff = y2 - y1;

		// If we are scanning from bottom to top
		if (ydiff < 0) {

			//set ydiff to be the absolute, and set the y_unit direction negative
			ydiff = -ydiff;
			y_unit = -1;
		}

		//else set the y_unit direction positive
		else {
			y_unit = 1;
		}

		// Because of the symmetry check, xdiff is always positive
		xdiff = x2 - x1;
		x_unit = 1;

		// If the difference is bigger along the x axis
		if (xdiff > ydiff) {
			// Initialize the error term
			error_term = xdiff >> 1;

			// Loop along the x axis and adjust scanY as necessary
			scanX = x1;
			for (i = 0; ((i <= xdiff) && valid && (!finished)); i++) {

				// Check if we have scanned off the edge of the buffer
				if ((scanX >= width) || ((y_unit > 0) && (scanY >= height)) || ((y_unit < 0) && (scanY < 0))) {
					finished = true;
				} else {
					// Else we either haven't yet reached the buffer, or we are on it
					// Make sure we're on the buffer
					if ((scanX >= 0) && (scanY >= 0) && (scanY < height)) {
						// Check to see if this is a valid walking area
						if (!walkCodePtr) {
							walkCodePtr = (uint8 *) & ((walkCodes->data)[scanY * stride + scanX]);
						}
						if ((*walkCodePtr) & 0x10) {
							valid = false;
						}
					}

					// Update scanY if appropriate
					// Update the error term
					error_term += ydiff;

					// If the error_term has exceeded the xdiff, we need to move one unit along the y axis
					if (error_term >= xdiff) {
						// Reset the error term
						error_term -= xdiff;

						// Move along the y axis
						scanY += y_unit;

						// Update the walkCodePtr index if necessary
						if (walkCodePtr) {
							if (y_unit > 0) {
								walkCodePtr += stride;
							} else {
								walkCodePtr -= stride;
							}
						}
					}
				}
				scanX += x_unit;

				// Update the walkCodePtr index if necessary
				if (walkCodePtr) {
					if (x_unit > 0) {
						walkCodePtr++;
					} else {
						walkCodePtr--;
					}
				}
			}
		} else {
			// Else the difference is bigger along the y axis
			// Initialize the error term
			error_term = ydiff >> 1;

			// Loop along the y axis and adjust scanX as necessary
			scanY = y1;
			for (i = 0; ((i <= ydiff) && valid && (!finished)); i++) {
				// Check if we have scanned off the edge of the buffer
				if (((x_unit > 0) && (scanX >= width)) || ((x_unit < 0) && (scanX < 0)) ||
					((y_unit > 0) && (scanY >= height)) || ((y_unit < 0) && (scanY < 0))) {
					finished = true;
				} else {
					// Else we either haven't yet reached the buffer, or we are on it
					// Make sure we're on the buffer
					if ((scanX >= 0) && (scanX < width) && (scanY >= 0) && (scanY < height)) {
						// Check to see if this is a valid walking area
						if (!walkCodePtr) {
							walkCodePtr = (uint8 *) & ((walkCodes->data)[scanY * stride + scanX]);
						}
						if ((*walkCodePtr) & 0x10) {
							valid = false;
						}
					}

					// Update scanX if appropriate
					// Update the error term
					error_term += xdiff;

					// If the error_term has exceeded the xdiff, we need to move one unit along the y axis
					if (error_term >= ydiff) {
						// Reset the error term
						error_term -= ydiff;

						// Move along the x axis
						scanX += x_unit;

						// Update the walkCodePtr index if necessary
						if (walkCodePtr) {
							if (x_unit > 0) {
								walkCodePtr++;
							} else {
								walkCodePtr--;
							}
						}
					}
				}
				scanY += y_unit;

				// Update the walkCodePtr index if necessary
				if (walkCodePtr) {
					if (y_unit > 0) {
						walkCodePtr += stride;
					} else {
						walkCodePtr -= stride;
					}
				}
			}
		}
	}

	// Now that we've checked it against the walk codes, we check if the line passes through
	// any of the forbidden rectangles
	if (valid) {
		if (intr_LinePassesThroughRect(x1, y1, x2, y2)) {
			valid = false;
		}
	}

	// Finally, if the edge is still valid, fill in the edge table with the distance between the nodes.
	if (valid) {
		deltaX = imath_abs(((frac16)(x2 - x1)) << 16);
		deltaY = imath_abs(((frac16)(y2 - y1)) << 16);
		if ((deltaX >= 0x800000) || (deltaY >= 0x800000)) {
			deltaX >>= 16;
			deltaY >>= 16;
			distance = (frac16)(SqrtF16(deltaX * deltaX + deltaY * deltaY) << 16);
		} else {
			distance = SqrtF16(SquareSF16(deltaX) + SquareSF16(deltaY)) << 8;
		}

		_G(rails).myEdges[index] = (int16)(distance >> 16);
	}
}


void RestoreNodeEdges(int32 nodeID, Buffer *walkCodes) {
	int32 i;
	for (i = 0; i < MAXRAILNODES; i++) {
		CreateEdge(i, nodeID, walkCodes);
	}
}


void RestoreEdgeList(Buffer *walkCodes) {
	int32 i, j;
	for (i = 0; i < MAXRAILNODES; i++) {
		for (j = i + 1; j < MAXRAILNODES; j++) {
			CreateEdge(i, j, walkCodes);
		}
	}
}

int32 AddRailNode(int32 x, int32 y, Buffer *walkCodes, bool restoreEdges) {
	int32 i, j;
	railNode *newNode;

	if ((!_G(rails).myNodes) || (!_G(rails).myEdges)) {
		return -1;
	}
	for (i = 0; (i < MAXRAILNODES) && _G(rails).myNodes[i]; i++) {
	}

	if (i < MAXRAILNODES) {
		if ((newNode = (railNode *)mem_alloc(sizeof(railNode), "railNode")) == nullptr) {
			return -1;
		}
		newNode->nodeID = (Byte)i;
		newNode->x = (int16)x;
		newNode->y = (int16)y;
		_G(rails).myNodes[i] = newNode;

		if (restoreEdges) {
			for (j = 0; j < MAXRAILNODES; j++) {
				if (_G(rails).myNodes[j]) CreateEdge(i, j, walkCodes);
			}
		}

		return i;
	}
	return -1;
}

void MoveRailNode(int32 nodeID, int32 x, int32 y, Buffer *walkCodes, bool restoreEdges) {
	if ((!_G(rails).myNodes) || (!_G(rails).myEdges) || (nodeID < 0) || (nodeID >= MAXRAILNODES) || (!_G(rails).myNodes[nodeID])) {
		return;
	}
	_G(rails).myNodes[nodeID]->x = (int16)x;
	_G(rails).myNodes[nodeID]->y = (int16)y;

	if (restoreEdges) {
		RestoreNodeEdges(nodeID, walkCodes);
	}
}

bool RemoveRailNode(int32 nodeID, Buffer *walkCodes, bool restoreEdges) {
	if ((nodeID < 0) || (nodeID >= MAXRAILNODES)) {
		return false;
	}
	if ((!_G(rails).myNodes) || (!_G(rails).myNodes[nodeID]) || (!_G(rails).myEdges)) {
		return false;
	}
	mem_free((void *)_G(rails).myNodes[nodeID]);
	_G(rails).myNodes[nodeID] = nullptr;

	if (restoreEdges) {
		RestoreNodeEdges(nodeID, walkCodes);
	}

	return true;
}

bool RailNodeExists(int32 nodeID, int32 *nodeX, int32 *nodeY) {
	if ((nodeID < 0) || (nodeID >= MAXRAILNODES) || (!_G(rails).myNodes) || (!_G(rails).myNodes[nodeID])) {
		return false;
	}
	if (nodeX) {
		*nodeX = _G(rails).myNodes[nodeID]->x;
	}
	if (nodeY) {
		*nodeY = _G(rails).myNodes[nodeID]->y;
	}
	return true;
}

int16 GetEdgeLength(int32 node1, int32 node2) {
	int32 temp;
	int32 index;
	if (!_G(rails).myEdges) return 0;
	if (node1 == node2) return 0;
	if (node2 < node1) {
		temp = node1;
		node1 = node2;
		node2 = temp;
	}
	// If node1 is y and node2 is x, first find table entry ie. tableWidth * y + x, the subtract
	// n(n+1)/2  since only the upper triangle of the table is stored...
	index = (MAXRAILNODES - 1) * node1 + node2 - 1 - (node1 * (node1 + 1) >> 1);
	return _G(rails).myEdges[index];
}


void DisposePath(railNode *pathStart) {
	railNode *tempNode;

	tempNode = pathStart;
	while (tempNode) {
		pathStart = pathStart->shortPath;
		mem_free((void *)tempNode);
		tempNode = pathStart;
	}
}


static railNode *DuplicatePath(railNode *pathStart) {
	railNode *newNode, *firstNode, *prevNode, *pathNode;

	// Initialize pointers
	firstNode = prevNode = nullptr;

	// This routine assumes a valid path from _G(rails).myNodes[origID] following _G(rails).myNodes[]->shortPath until nullptr
	pathNode = pathStart;

	// Loop until nullptr - end of path
	while (pathNode) {

		// Create a new railNode, and duplicate values
		if ((newNode = (railNode *)mem_alloc(sizeof(railNode), "+RAIL")) == nullptr) {
			error_show(FL, 'OOM!', "Could not alloc railNode");
			return nullptr;
		}
		newNode->x = pathNode->x;
		newNode->y = pathNode->y;
		newNode->shortPath = nullptr;

		// Link into the new list
		if (!firstNode) {
			firstNode = newNode;
		} else {
			prevNode->shortPath = newNode;
		}
		prevNode = newNode;

		// Get the next in the list
		pathNode = pathNode->shortPath;
	}

	return firstNode;
}


railNode *CreateCustomPath(int coord, ...) {
	va_list argPtr;
	railNode *firstNode, *prevNode = nullptr, *newNode;
	int32 x, y;

	// Initialize firstNode
	firstNode = nullptr;

	// Set argPtr to point to the beginning of the variable arg list
	va_start(argPtr, coord);

	// Loop until coord == -1
	while (coord != -1) {
		// Set x
		x = coord;

		// Read the next arg off the arg list, and set y
		coord = va_arg(argPtr, int);
		y = coord;


		// Create a new node struct
		if ((newNode = (railNode *)mem_alloc(sizeof(railNode), "railNode")) == nullptr) {
			error_show(FL, 'OOM!', "could not alloc railNode");
			return nullptr;
		}

		// Set the new node values...
		newNode->x = x;
		newNode->y = y;
		newNode->shortPath = nullptr;

		// Link into path list
		if (!firstNode) {
			firstNode = newNode;
		} else {
			assert(prevNode);
			prevNode->shortPath = newNode;
		}
		prevNode = newNode;

		// Read another arg, should be the next "x" for the next pair of args
		if (coord != -1) {
			coord = va_arg(argPtr, int);
		}
	}

	// Restore the stack
	va_end(argPtr);

	return firstNode;
}


bool GetShortestPath(int32 origID, int32 destID, railNode **shortPath) {
	pathNode *thePath, *tempPathNode;
	railNode **checkStackTop, *currNode, *tempNode;
	int16 edgeDist, shortcutWeight;
	uint32 currPathNodes;
	int32 i, prevID, maxNodeID;

	*shortPath = nullptr;

	// Check that we have two valid and different nodes to walk between
	if ((!_G(rails).myNodes) || (!_G(rails).myEdges)) {
		return false;
	}
	if (origID == destID) {
		return true;
	}
	if ((!_G(rails).myNodes[origID]) || (!_G(rails).myNodes[destID])) {
		return false;
	}
	if ((_G(rails).myNodes[origID]->x == _G(rails).myNodes[destID]->x) && (_G(rails).myNodes[origID]->y == _G(rails).myNodes[destID]->y)) {
		return true;
	}

	// Set the end of the shortest path
	_G(rails).myNodes[destID]->shortPath = nullptr;

	// Check to see if we can walk directly from oridID to destID
	edgeDist = GetEdgeLength(origID, destID);
	if (edgeDist > 0) {
		_G(rails).myNodes[origID]->shortPath = _G(rails).myNodes[destID];
		_G(rails).myNodes[destID]->pathWeight = edgeDist;
		*shortPath = DuplicatePath(_G(rails).myNodes[origID]->shortPath);
		return true;
	}

	// Otherwise, run the algorithm to determine the shortest path

	// Initialize the railNodes and find the largest node ID to speed up for loops
	maxNodeID = 0;
	for (i = 0; i < MAXRAILNODES; i++) {
		tempNode = _G(rails).myNodes[i];
		if (tempNode) {
			maxNodeID = i;
			tempNode->shortPath = nullptr;
			tempNode->pathWeight = 32767;
		}
	}

	// Initialize the stack
	_G(rails).stackTop = _G(rails).stackBottom;

	// Initialize the bitmask of the nodes and the current path list
	currPathNodes = 0;
	thePath = nullptr;

	// Put the first node onto the stack (the address of, actually)
	_G(rails).myNodes[origID]->pathWeight = 0;
	*_G(rails).stackTop++ = _G(rails).myNodes[origID];

	// While there are still nodes on the stack keep processing
	while (_G(rails).stackTop > _G(rails).stackBottom) {

		// Take the first node off the stack
		currNode = *(--_G(rails).stackTop);

		// See if it is adjacent to the destination node - always 0 the first time through...
		edgeDist = GetEdgeLength(currNode->nodeID, destID);

		// Yes it is, therefore, we have a valid path, maybe the shortest...
		if (edgeDist > 0) {
			// Check whether the pathweight of the second last node + the edge to get to the last
			// is less than the pathweight of the previously found shortest path
			if (currNode->pathWeight + edgeDist < _G(rails).myNodes[destID]->pathWeight) {
				// If so, store the new shortest path weight, and complete the path link
				_G(rails).myNodes[destID]->pathWeight = (int16)(currNode->pathWeight + edgeDist);
				_G(rails).myNodes[currNode->nodeID]->shortPath = _G(rails).myNodes[destID];

				// Now we follow "thePath" back to the origID, updating the bitMask, and setting shortPath links
				// initialize variables used in the loop
				prevID = currNode->nodeID;
				currPathNodes = 0;
				tempPathNode = thePath;

				// Loop through to the end of the path
				while (tempPathNode) {

					// Link up the nodes in the shortPath
					_G(rails).myNodes[tempPathNode->nodeID]->shortPath = _G(rails).myNodes[prevID];

					// Or in the bitmask
					currPathNodes |= (1 << prevID);

					// Setup for the next link
					prevID = tempPathNode->nodeID;
					tempPathNode = tempPathNode->next;
				}
			}

			// Set the stackTop to point at the node preceding currNode, which precedes destID
			// we want to check the contents of the stack from the top, and stackTop always point to
			// the next available location, not directly at the top element.
			_G(rails).stackTop--;

			// Setup temp Ptr
			tempPathNode = thePath;

			// While the top of the stack point at a node directly in thePath -
			// we remove all the nodes in thePath from the top of the stack until we come across
			// one that is not in thePath.  This one that is not in thePath will not yet have been
			// checked.
			while ((tempPathNode) && (tempPathNode->nodeID == (*_G(rails).stackTop)->nodeID)) {
				_G(rails).stackTop--;
				thePath = thePath->next;
				mem_free_to_stash((void *)tempPathNode, _G(rails).memtypePATHN);
				tempPathNode = thePath;
			}

			// Since thePath cannot have more elements than are on the stack, we will never have
			// a stack underflow, but when the _G(rails).stackTop points to a node not in thePath, or
			// if the entire thePath was on the stack, thus the stack was completely emptied,
			// we must reset the _G(rails).stackTop to point to the next available location.
			_G(rails).stackTop++;
		} else {
			// Else the currNode is not adjacent to the dest node
			// Put currNode back onto the stack
			_G(rails).stackTop++;

			// Setup a temporary pointer, so we know whether any neighbors of the currNode were stacked...
			checkStackTop = _G(rails).stackTop;

			// Check to see whether the path leading to currNode is at least shorter than the current shortest path
			if (currNode->pathWeight < _G(rails).myNodes[destID]->pathWeight) {
				// If so, loop through all the nodes
				for (i = 0; i <= maxNodeID; i++) {
					if (_G(rails).myNodes[i]) {
						// For each different valid node, check to see if the currNode is adjacent to it
						edgeDist = GetEdgeLength(currNode->nodeID, i);

						// If it is a neighbor, and the pathweight reaching it through currNode is
						// less than the weight of any previous path that reached the same neighbor...
						if ((edgeDist > 0) && ((currNode->pathWeight + edgeDist) < _G(rails).myNodes[i]->pathWeight)) {
							// Now see if that neighbor is already part of the current shortest path
							if (currPathNodes & (1 << i)) {
								// If so, the path from node i to the destination node will already have been created
								// therefore the entire path will become shorter by the original weight to reach node i
								// minus (the weight to get to the currNode plus the weight of the edge to
								// get from the currNode to node i).
								shortcutWeight = (int16)(_G(rails).myNodes[i]->pathWeight - (currNode->pathWeight + edgeDist));

								// Loop from node i to the dest node, subtract the shortcutWeight, and or the bitmask
								currPathNodes = 0;
								tempNode = _G(rails).myNodes[i];
								while (tempNode) {
									currPathNodes |= (1 << tempNode->nodeID);
									tempNode->pathWeight -= shortcutWeight;
									tempNode = tempNode->shortPath;
								}

								// Link the currNode to node i
								_G(rails).myNodes[currNode->nodeID]->shortPath = _G(rails).myNodes[i];

								// ThePath is a linked list that lists the nodeIDs
								//		FROM: the node leading up to the currNode
								//		TO: the origID (ie. backwards)
								// Loop through backwards, linking up the new shortPath, and ORing the bitmask
								prevID = currNode->nodeID;
								tempPathNode = thePath;
								while (tempPathNode) {
									_G(rails).myNodes[tempPathNode->nodeID]->shortPath = _G(rails).myNodes[prevID];
									currPathNodes |= (1 << prevID);
									prevID = tempPathNode->nodeID;
									tempPathNode = tempPathNode->next;
								}
							} else {
								// Else we don't know whether we can reach the destID from node i
								// Set the pathweight of node i to the smaller value, and place it on the stack
								_G(rails).myNodes[i]->pathWeight = (int16)(currNode->pathWeight + edgeDist);
								*_G(rails).stackTop++ = _G(rails).myNodes[i];
							}
						}
					}
				}
			}

			// If we put neighbors of the currNode onto the stack...
			if (_G(rails).stackTop != checkStackTop) {
				// Create a pathNode, and place in the list.
				tempPathNode = (pathNode *)mem_get_from_stash(_G(rails).memtypePATHN, "+PATH");
				tempPathNode->nodeID = currNode->nodeID;
				tempPathNode->next = thePath;
				thePath = tempPathNode;
			} else {
				// Otherwise currNode had no neighbors such that it is not known whether traversing
				// through it's neighbor is shorter.  currNode is either a dead end, or is in the path

				// Take currNode off the stack top
				_G(rails).stackTop--;

				// Set the _G(rails).stackTop to point at the node preceding currNode
				// we want to check the contents of the stack from the top, and _G(rails).stackTop always point to
				// the next available location, not directly at the top element.
				_G(rails).stackTop--;

				//setup temp Ptr
				tempPathNode = thePath;

				// While the top of the stack point at a node directly in thePath -
				// we remove all the nodes in thePath from the top of the stack until we come across
				// one that is not in thePath.  This one that is not in thePath will not yet have been
				// checked.
				while ((tempPathNode) && (tempPathNode->nodeID == (*_G(rails).stackTop)->nodeID)) {
					_G(rails).stackTop--;
					thePath = thePath->next;
					mem_free_to_stash((void *)tempPathNode, _G(rails).memtypePATHN);
					tempPathNode = thePath;
				}

				// Since thePath cannot have more elements than are on the stack, we will never have
				// a stack underflow, but when the _G(rails).stackTop points to a node not in thePath, or
				// if the entire thePath was on the stack, thus the stack was completely emptied,
				// we must reset the _G(rails).stackTop to point to the next available location.
				_G(rails).stackTop++;

			}
		}
	}

	// We've completed the enhanced breadth-first search of the rail nodes.  return the result
	if (_G(rails).myNodes[destID]->pathWeight < 32767) {
		*shortPath = DuplicatePath(_G(rails).myNodes[origID]->shortPath);
		return true;
	} else {
		return false;
	}
}

bool intr_PathCrossesLine(int32 startX, int32 startY, railNode *pathStart,
	int32 line_x1, int32 line_y1, int32 line_x2, int32 line_y2) {
	railNode *tempNode;
	bool intersected;
	int32 prevX, prevY;

	intersected = false;
	prevX = startX;
	prevY = startY;

	// Loop through the path nodes. Each node is the end of line segment started at (prevX, prevY)
	tempNode = pathStart;
	while (tempNode && (!intersected)) {
		intersected = intr_LinesCross(line_x1, line_y1, line_x2, line_y2, prevX, prevY, tempNode->x, tempNode->y);
		prevX = tempNode->x;
		prevY = tempNode->y;
		tempNode = tempNode->shortPath;
	}

	return intersected;
}

} // End of namespace M4
