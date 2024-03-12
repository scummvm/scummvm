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

#include "m4/gui/gui_vmng_rectangles.h"
#include "m4/gui/gui_vmng.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/mem/mem.h"
#include "m4/vars.h"

namespace M4 {


#define LEFT_EDGE 1
#define RIGHT_EDGE 2

RectList *vmng_CreateNewRect(int32 x1, int32 y1, int32 x2, int32 y2) {
	RectList *newRect;
	if ((newRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectList")) == nullptr) {
		error_show(FL, 'OOS!', "vmng_CreateNewRect");
	}

	newRect->x1 = x1;
	newRect->y1 = y1;
	newRect->x2 = x2;
	newRect->y2 = y2;
	newRect->next = nullptr;
	newRect->prev = nullptr;

	return newRect;
}

void vmng_AddRectToRectList(RectList **theRectList, int32 rectX1, int32 rectY1, int32 rectX2, int32 rectY2) {
	RectList *dirtyRect, *dirtyRectList;
	RectList *endCleanRectList, *cleanRectList;
	RectList *myRect, *myRectList;
	RectList *newRect;
	bool intersected;

	// First make sure we have a valid rectangle
	if ((rectX1 > rectX2) || (rectY1 > rectY2)) {
		return;
	}

	// Intialize the dirty rect list
	dirtyRectList = vmng_CreateNewRect(rectX1, rectY1, rectX2, rectY2);

	// Intialize the clean rectList
	cleanRectList = nullptr;
	endCleanRectList = nullptr;

	// Use a local var for theRectlist
	myRectList = *theRectList;

	// Loop through all the dirtyRects
	dirtyRect = dirtyRectList;
	while (dirtyRect) {
		// Remove dirtyRect from the head of the dirtyRectList
		dirtyRectList = dirtyRectList->next;

		// Set the intersected flag
		intersected = false;

		// Loop on through
		myRect = myRectList;
		while (myRect) {

			// If the two rectangles intersect
			if ((dirtyRect->x1 <= myRect->x2) && (dirtyRect->x2 >= myRect->x1) && (dirtyRect->y1 <= myRect->y2) && (dirtyRect->y2 >= myRect->y1)) {
				// Set the intersected flag
				intersected = true;

				// If dirtyRect is not completely contained within myRect
				if ((dirtyRect->x1 < myRect->x1) || (dirtyRect->y1 < myRect->y1) || (dirtyRect->x2 > myRect->x2) || (dirtyRect->y2 > myRect->y2)) {

					// First remove it from the list
					if (myRect->prev) {
						myRect->prev->next = myRect->next;
					} else {
						myRectList = myRect->next;
					}
					if (myRect->next) {
						myRect->next->prev = myRect->prev;
					}

					// So now there is an intersection.
					// If myRect sticks out above dirtyRect, chop it off and put it in the main rect list, to be recheck by other dirty rects
					if (myRect->y1 < dirtyRect->y1) {
						newRect = vmng_CreateNewRect(myRect->x1, myRect->y1, myRect->x2, dirtyRect->y1 - 1);
						newRect->prev = nullptr;
						newRect->next = myRectList;
						if (myRectList) {
							myRectList->prev = newRect;
						}
						myRectList = newRect;

						// And set the top of myRect to be the same as dirtyRect
						myRect->y1 = dirtyRect->y1;
					} else if (dirtyRect->y1 < myRect->y1) {
						// else if dirtyRect sticks out above chop it off and put it on the dirty list
						newRect = vmng_CreateNewRect(dirtyRect->x1, dirtyRect->y1, dirtyRect->x2, myRect->y1 - 1);
						newRect->next = dirtyRectList;
						dirtyRectList = newRect;

						// and set the top of dirtyRect to be the same as myRect
						dirtyRect->y1 = myRect->y1;
					}

					// If myRect sticks out below dirtyRect, chop it off and put it in the main rect list, to be recheck by other dirty rects
					if (myRect->y2 > dirtyRect->y2) {
						newRect = vmng_CreateNewRect(myRect->x1, dirtyRect->y2 + 1, myRect->x2, myRect->y2);
						newRect->prev = nullptr;
						newRect->next = myRectList;
						if (myRectList) {
							myRectList->prev = newRect;
						}
						myRectList = newRect;

						// and set the bottom of myRect to be the same as dirtyRect
						myRect->y2 = dirtyRect->y2;
					} else if (dirtyRect->y2 > myRect->y2) {
						//else if dirtyRect sticks out below myRect...
						newRect = vmng_CreateNewRect(dirtyRect->x1, myRect->y2 + 1, dirtyRect->x2, dirtyRect->y2);
						newRect->next = dirtyRectList;
						dirtyRectList = newRect;

						// and set the bottom of dirtyRect to be the same as myRect
						dirtyRect->y2 = myRect->y2;
					}

					// Now we've got overlapping rectangles which are the same height. create one max width one
					// If the dirtyRect sticks out on either side, the resulting rect is still dirty, otherwise clean
					if ((dirtyRect->x1 < myRect->x1) || (dirtyRect->x2 > myRect->x2)) {

						// Use dirtyRect to become the max width rect
						dirtyRect->x1 = imath_min(dirtyRect->x1, myRect->x1);
						dirtyRect->x2 = imath_max(dirtyRect->x2, myRect->x2);
						dirtyRect->next = dirtyRectList;
						dirtyRectList = dirtyRect;

						// And turf myRect
						mem_free_to_stash(myRect, _G(memtypeRECT));
					} else {
						// Else we can put what's left of myRect onto the clean list and turf dirtyRect
						// Note: it is impossible to split the dirtyRect list vertically, they always stretch horizontally,
						// therefore if this dirty rect does not stick out the sides, what's left of myRect is clean
						myRect->prev = nullptr;
						myRect->next = cleanRectList;
						if (cleanRectList) {
							cleanRectList->prev = myRect;
						} else {
							endCleanRectList = myRect;
						}
						cleanRectList = myRect;

						mem_free_to_stash((void *)dirtyRect, _G(memtypeRECT));
					}

					// Exit the loop
					myRect = nullptr;
				} else {
					// else through away dirtyRect, and get the next dirtyRect
					mem_free_to_stash(dirtyRect, _G(memtypeRECT));
					myRect = nullptr;
				}
			} else {
				// else get the next rect
				myRect = myRect->next;
			}
		}

		//if we didn't intersect, put the dirtyRect on the clean list
		if (!intersected) {
			dirtyRect->prev = nullptr;
			dirtyRect->next = cleanRectList;
			if (cleanRectList) {
				cleanRectList->prev = dirtyRect;
			} else {
				endCleanRectList = dirtyRect;
			}
			cleanRectList = dirtyRect;
		}

		// get the next dirty rect
		dirtyRect = dirtyRectList;
	}

	// now, just add the clean list onto the dirty list
	if (cleanRectList) {

		//now add the entire cleanRectList to the front of myRectList
		endCleanRectList->next = myRectList;
		if (myRectList) {
			myRectList->prev = endCleanRectList;
		}
		myRectList = cleanRectList;
	}

	// Return the rect list
	*theRectList = myRectList;
}

RectList *vmng_DuplicateRectList(RectList *myRectList) {
	RectList *newRectList, *tempRect, *myRect, *prevRect;

	newRectList = nullptr;
	prevRect = nullptr;
	myRect = myRectList;
	while (myRect) {

		// Duplicate myRect and stick it on the newRectList
		if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectList")) == nullptr) {
			error_show(FL, 'OOS!', "vmng_DuplicateRectList()");
		}
		tempRect->x1 = myRect->x1;
		tempRect->y1 = myRect->y1;
		tempRect->x2 = myRect->x2;
		tempRect->y2 = myRect->y2;
		tempRect->prev = prevRect;
		tempRect->next = nullptr;
		if (prevRect) {
			prevRect->next = tempRect;
		} else {
			newRectList = tempRect;
		}
		prevRect = tempRect;

		//get the next rectangle
		myRect = myRect->next;
	}

	return newRectList;
}

bool vmng_RectIntersectsRectList(RectList *myRectList, int32 x1, int32 y1, int32 x2, int32 y2) {
	RectList *myRect;
	int32 intrX1, intrY1, intrX2, intrY2;

	// Parameter verification
	if ((!myRectList) || (x1 > x2) || (y1 > y2)) {
		return false;
	}

	// Loop through the list, and break as soon as there is an intersection
	myRect = myRectList;
	while (myRect) {
		// Calculate the intersection
		intrX1 = imath_max(myRect->x1, x1);
		intrY1 = imath_max(myRect->y1, y1);
		intrX2 = imath_min(myRect->x2, x2);
		intrY2 = imath_min(myRect->y2, y2);

		// If we intersected, return true
		if ((intrX1 <= intrX2) && (intrY1 <= intrY2)) {
			return true;
		}

		// else get the next rect in the list
		myRect = myRect->next;
	}

	// We made it through the entire list with no intersections - return false
	return false;
}

bool vmng_ClipRectList(RectList **myRectList, int32 clipX1, int32 clipY1, int32 clipX2, int32 clipY2) {
	RectList *nextRect, *myRect;
	int32 x1, y1, x2, y2;

	// Loop through myRect list
	myRect = *myRectList;
	while (myRect) {

		// Set the next rect
		nextRect = myRect->next;

		// Clip myRect
		x1 = imath_max(myRect->x1, clipX1);
		y1 = imath_max(myRect->y1, clipY1);
		x2 = imath_min(myRect->x2, clipX2);
		y2 = imath_min(myRect->y2, clipY2);

		// If we have a valid rectangle
		if ((x1 <= x2) && (y1 <= y2)) {
			// Clip the rectangle
			myRect->x1 = x1;
			myRect->y1 = y1;
			myRect->x2 = x2;
			myRect->y2 = y2;
		} else {
			// Else remove it from the rectList and turf it
			if (myRect->prev) {
				myRect->prev->next = myRect->next;
			} else {
				*myRectList = myRect->next;
			}
			if (myRect->next) {
				myRect->next->prev = myRect->prev;
			}

			mem_free_to_stash((void *)myRect, _G(memtypeRECT));
		}

		// Check the next rect
		myRect = nextRect;
	}

	return true;
}

bool vmng_RectListValid(RectList *myRectList) {
	RectList *myRect, *tempRectList;

	myRect = myRectList;
	while (myRect) {
		tempRectList = myRect->next;
		if (vmng_RectIntersectsRectList(tempRectList, myRect->x1, myRect->y1, myRect->x2, myRect->y2)) {
			return false;
		}
		myRect = myRect->next;
	}

	return true;
}

void vmng_DisposeRectList(RectList **rectList) {
	RectList *myRect;

	// Loop through the rect list
	myRect = *rectList;
	while (myRect) {
		// Remove myRect from the head of the list
		*rectList = myRect->next;

		// Dispose of myRect;
		mem_free_to_stash((void *)myRect, _G(memtypeRECT));

		// Get the next rectangle
		myRect = *rectList;
	}
}

void vmng_RemoveRectFromRectList(RectList **scrnRectList, int32 x1, int32 y1, int32 x2, int32 y2) {
	RectList *myRect, *prevRect, *nextRect, *tempRect;
	RectList *rectList, *unsortedRectList;
	int32 tempX1, tempY1, tempX2, tempY2;
	bool finished;

	rectList = *scrnRectList;

	// Go through the rectList list breaking down any rects which intersect the given coords
	unsortedRectList = nullptr;
	myRect = rectList;

	while (myRect) {
		// Set the nextRect pointer
		nextRect = myRect->next;

		// Check for an intersection
		tempX1 = imath_max(x1, myRect->x1);
		tempY1 = imath_max(y1, myRect->y1);
		tempX2 = imath_min(x2, myRect->x2);
		tempY2 = imath_min(y2, myRect->y2);

		// If we have an intersection
		if ((tempX1 <= tempX2) && (tempY1 <= tempY2)) {
			// Break myRect apart into any pieces not covered by x1, y1, x2, y2
			// Top edge
			if (myRect->y1 < y1) {
				// Create a new rect of just the part that extends beyond the top of myRect
				if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectangle")) == nullptr) {
					error_show(FL, 'OOS!', "vmng_AddRectToRectList");
				}
				tempRect->x1 = myRect->x1;
				tempRect->y1 = myRect->y1;
				tempRect->x2 = myRect->x2;
				tempRect->y2 = y1 - 1;

				// Add tempRect to the unsortedRectList
				tempRect->next = unsortedRectList;
				unsortedRectList = tempRect;

				// Update myRect
				myRect->y1 = y1;
			}

			// Bottom edge
			if (myRect->y2 > y2) {
				// Create a new rect of just the part that extends beyond the top of myRect
				if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectangle")) == nullptr) {
					error_show(FL, 'OOS!', "vmng_AddRectToRectList");
				}
				tempRect->x1 = myRect->x1;
				tempRect->y1 = y2 + 1;
				tempRect->x2 = myRect->x2;
				tempRect->y2 = myRect->y2;

				// Add tempRect to the unsortedRectList
				tempRect->next = unsortedRectList;
				unsortedRectList = tempRect;

				// Update myRect
				myRect->y2 = y2;
			}

			// Left edge
			if (myRect->x1 < x1) {
				// Create a new rect of just the part that extends beyond the top of myRect
				if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectangle")) == nullptr) {
					error_show(FL, 'OOS!', "vmng_AddRectToRectList");
				}
				tempRect->x1 = myRect->x1;
				tempRect->y1 = myRect->y1;
				tempRect->x2 = x1 - 1;
				tempRect->y2 = myRect->y2;

				// Add tempRect to the unsortedRectList
				tempRect->next = unsortedRectList;
				unsortedRectList = tempRect;
			}

			// Right edge
			if (myRect->x2 > x2) {
				// Create a new rect of just the part that extends beyond the top of myRect
				if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectangle")) == nullptr) {
					error_show(FL, 'OOS!', "vmng_AddRectToRectList");
				}

				tempRect->x1 = x2 + 1;
				tempRect->y1 = myRect->y1;
				tempRect->x2 = myRect->x2;
				tempRect->y2 = myRect->y2;

				// Add tempRect to the unsortedRectList
				tempRect->next = unsortedRectList;
				unsortedRectList = tempRect;
			}

			// Remove myRect from the list and turf it
			if (myRect->next) {
				myRect->next->prev = myRect->prev;
			}
			if (myRect->prev) {
				myRect->prev->next = myRect->next;
			} else {
				rectList = myRect->next;
			}

			mem_free_to_stash((void *)myRect, _G(memtypeRECT));
		}

		// Get the next rect
		myRect = nextRect;
	}

	// Now go through the unsorted list and insert them into the main list
	tempRect = unsortedRectList;
	while (tempRect) {
		unsortedRectList = unsortedRectList->next;
		// For each unsorted rect, loop through the rect list until its place is found
		finished = false;
		prevRect = nullptr;
		myRect = rectList;

		while (myRect && (!finished)) {
			// If it goes before myRect
			if (tempRect->y2 <= myRect->y2) {
				finished = true;
			} else {
				prevRect = myRect;
				myRect = myRect->next;
			}
		}

		// tempRect belongs after prevRect
		if (prevRect) {
			tempRect->prev = prevRect;
			tempRect->next = prevRect->next;
			if (prevRect->next) {
				prevRect->next->prev = tempRect;
			}
			prevRect->next = tempRect;
		} else {
			// else it belongs at the front of rectList
			tempRect->prev = nullptr;
			tempRect->next = rectList;
			if (rectList) {
				rectList->prev = tempRect;
			}
			rectList = tempRect;
		}

		// Get the next unsorted rect
		tempRect = unsortedRectList;
	}

	// Set the screen rect list to the resulting rect list
	*scrnRectList = rectList;
}

} // End of namespace M4
