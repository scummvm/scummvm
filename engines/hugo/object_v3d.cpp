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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"
#include "common/random.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/object.h"
#include "hugo/global.h"
#include "hugo/display.h"
#include "hugo/file.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"

namespace Hugo {

ObjectHandler_v3d::ObjectHandler_v3d(HugoEngine *vm) : ObjectHandler_v2d(vm) {
}

ObjectHandler_v3d::~ObjectHandler_v3d() {
}

/**
* Update all object positions.  Process object 'local' events
* including boundary events and collisions
*/
void ObjectHandler_v3d::moveObjects() {
	debugC(4, kDebugObject, "moveObjects");

	// Added to DOS version in order to handle mouse properly
	// If route mode enabled, do special route processing
	if (_vm->getGameStatus().routeIndex >= 0)
		_vm->_route->processRoute();

	// Perform any adjustments to velocity based on special path types
	// and store all (visible) object baselines into the boundary file.
	// Don't store foreground or background objects
	for (int i = 0; i < _numObj; i++) {
		object_t *obj = &_objects[i];               // Get pointer to object
		seq_t *currImage = obj->currImagePtr;       // Get ptr to current image
		if (obj->screenIndex == *_vm->_screen_p) {
			switch (obj->pathType) {
			case CHASE:
			case CHASE2: {
				int8 radius = obj->radius;          // Default to object's radius
				if (radius < 0)                     // If radius infinity, use closer value
					radius = DX;

				// Allowable motion wrt boundary
				int dx = _vm->_hero->x + _vm->_hero->currImagePtr->x1 - obj->x - currImage->x1;
				int dy = _vm->_hero->y + _vm->_hero->currImagePtr->y2 - obj->y - currImage->y2 - 1;
				if (abs(dx) <= radius)
					obj->vx = 0;
				else
					obj->vx = (dx > 0) ? MIN(dx, obj->vxPath) : MAX(dx, -obj->vxPath);
				if (abs(dy) <= radius)
					obj->vy = 0;
				else
					obj->vy = (dy > 0) ? MIN(dy, obj->vyPath) : MAX(dy, -obj->vyPath);

				// Set first image in sequence (if multi-seq object)
				switch (obj->seqNumb) {
				case 4:
					if (!obj->vx) {                 // Got 4 directions
						if (obj->vx != obj->oldvx) { // vx just stopped
							if (dy >= 0)
								obj->currImagePtr = obj->seqList[DOWN].seqPtr;
							else
								obj->currImagePtr = obj->seqList[_UP].seqPtr;
						}
					} else if (obj->vx != obj->oldvx) {
						if (dx > 0)
							obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
						else
							obj->currImagePtr = obj->seqList[LEFT].seqPtr;
					}
					break;
				case 3:
				case 2:
					if (obj->vx != obj->oldvx) {    // vx just stopped
						if (dx > 0)                 // Left & right only
							obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
						else
							obj->currImagePtr = obj->seqList[LEFT].seqPtr;
					}
					break;
				}

				if (obj->vx || obj->vy) {
					obj->cycling = CYCLE_FORWARD;
				} else {
					obj->cycling = NOT_CYCLING;
					_vm->boundaryCollision(obj);    // Must have got hero!
				}
				obj->oldvx = obj->vx;
				obj->oldvy = obj->vy;
				currImage = obj->currImagePtr;      // Get (new) ptr to current image
				break;
				}
			case WANDER2:
			case WANDER:
				if (!_vm->_rnd->getRandomNumber(3 * _vm->_normalTPS)) {       // Kick on random interval
					obj->vx = _vm->_rnd->getRandomNumber(obj->vxPath << 1) - obj->vxPath;
					obj->vy = _vm->_rnd->getRandomNumber(obj->vyPath << 1) - obj->vyPath;

					// Set first image in sequence (if multi-seq object)
					if (obj->seqNumb > 1) {
						if (!obj->vx && (obj->seqNumb >= 4)) {
							if (obj->vx != obj->oldvx)  { // vx just stopped
								if (obj->vy > 0)
									obj->currImagePtr = obj->seqList[DOWN].seqPtr;
								else
									obj->currImagePtr = obj->seqList[_UP].seqPtr;
							}
						} else if (obj->vx != obj->oldvx) {
							if (obj->vx > 0)
								obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
							else
								obj->currImagePtr = obj->seqList[LEFT].seqPtr;
						}
					}
					obj->oldvx = obj->vx;
					obj->oldvy = obj->vy;
					currImage = obj->currImagePtr;  // Get (new) ptr to current image
				}
				if (obj->vx || obj->vy)
					obj->cycling = CYCLE_FORWARD;
				break;
			default:
				; // Really, nothing
			}
			// Store boundaries
			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				_vm->storeBoundary(obj->x + currImage->x1, obj->x + currImage->x2, obj->y + currImage->y2);
		}
	}

	// Move objects, allowing for boundaries
	for (int i = 0; i < _numObj; i++) {
		object_t *obj = &_objects[i];                         // Get pointer to object
		if ((obj->screenIndex == *_vm->_screen_p) && (obj->vx || obj->vy)) {
			// Only process if it's moving

			// Do object movement.  Delta_x,y return allowed movement in x,y
			// to move as close to a boundary as possible without crossing it.
			seq_t *currImage = obj->currImagePtr;   // Get ptr to current image
			// object coordinates
			int x1 = obj->x + currImage->x1;        // Left edge of object
			int x2 = obj->x + currImage->x2;        // Right edge
			int y1 = obj->y + currImage->y1;        // Top edge
			int y2 = obj->y + currImage->y2;        // Bottom edge

			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				_vm->clearBoundary(x1, x2, y2);     // Clear our own boundary

			// Allowable motion wrt boundary
			int dx = _vm->deltaX(x1, x2, obj->vx, y2);
			if (dx != obj->vx) {
				// An object boundary collision!
				_vm->boundaryCollision(obj);
				obj->vx = 0;
			}

			int dy = _vm->deltaY(x1, x2, obj->vy, y2);
			if (dy != obj->vy) {
				// An object boundary collision!
				_vm->boundaryCollision(obj);
				obj->vy = 0;
			}

			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				_vm->storeBoundary(x1, x2, y2);     // Re-store our own boundary

			obj->x += dx;                           // Update object position
			obj->y += dy;

			// Don't let object go outside screen
			if (x1 < EDGE)
				obj->x = EDGE2;
			if (x2 > (XPIX - EDGE))
				obj->x = XPIX - EDGE2 - (x2 - x1);
			if (y1 < EDGE)
				obj->y = EDGE2;
			if (y2 > (YPIX - EDGE))
				obj->y = YPIX - EDGE2 - (y2 - y1);

			if ((obj->vx == 0) && (obj->vy == 0) && (obj->pathType != WANDER2) && (obj->pathType != CHASE2))
				obj->cycling = NOT_CYCLING;
		}
	}

	// Clear all object baselines from the boundary file.
	for (int i = 0; i < _numObj; i++) {
		object_t *obj = &_objects[i];               // Get pointer to object
		seq_t *currImage = obj->currImagePtr;       // Get ptr to current image
		if ((obj->screenIndex == *_vm->_screen_p) && (obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
			_vm->clearBoundary(obj->oldx + currImage->x1, obj->oldx + currImage->x2, obj->oldy + currImage->y2);
	}

	// If maze mode is enabled, do special maze processing
	if (_maze.enabledFl) {
		seq_t *currImage = _vm->_hero->currImagePtr;// Get ptr to current image
		// hero coordinates
		int x1 = _vm->_hero->x + currImage->x1;     // Left edge of object
		int x2 = _vm->_hero->x + currImage->x2;     // Right edge
		int y1 = _vm->_hero->y + currImage->y1;     // Top edge
		int y2 = _vm->_hero->y + currImage->y2;     // Bottom edge

		_vm->_scheduler->processMaze(x1, x2, y1, y2);
	}
}

/**
* Swap all the images of one object with another.  Set hero_image (we make
* the assumption for now that the first obj is always the HERO) to the object
* number of the swapped image
*/
void ObjectHandler_v3d::swapImages(int objNumb1, int objNumb2) {
	debugC(1, kDebugObject, "swapImages(%d, %d)", objNumb1, objNumb2);

	saveSeq(&_objects[objNumb1]);

	seqList_t tmpSeqList[MAX_SEQUENCES];
	int seqListSize = sizeof(seqList_t) * MAX_SEQUENCES;

	memmove(tmpSeqList, _objects[objNumb1].seqList, seqListSize);
	memmove(_objects[objNumb1].seqList, _objects[objNumb2].seqList, seqListSize);
	memmove(_objects[objNumb2].seqList, tmpSeqList, seqListSize);
	restoreSeq(&_objects[objNumb1]);
	_objects[objNumb2].currImagePtr = _objects[objNumb2].seqList[0].seqPtr;
	_vm->_heroImage = (_vm->_heroImage == HERO) ? objNumb2 : HERO;
}

} // End of namespace Hugo
