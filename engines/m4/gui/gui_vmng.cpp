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

/**
 * THE GUI:
 * gui_vmng.cpp controls the windowing system. The gui is comprised of layered independent
 * windows.  The view manager controls which pieces of each window are visible, and which
 * window receives events.  The contents of of the windows, whether they be dialog boxes,
 * buffers, or some new addition has no bearing on the performance of the view manager.
 * Therefore, each window is created with a layer, an event handler, and a redraw function.
 * When the view manager determines that an area of a window needs to be redrawn, it
 * simply calls that window's redraw function.  It is up to the redraw function to ensure
 * that the rectangle is properly redrawn.  If an event occurs, the view manager will
 * determine which window should handle the event.
 *
 * To recap then, it manages the visual display of each window's current position
 * and relative layer, and when either a keyboard event, or mouse event is registered, which
 * window's evtHandler will be given the event to process. In addition to requesting a
 * window to redraw a portion of itself, or handle an event which has occurred, vmng.cpp
 * also displays the mouse in its current location.  Through the use of an off screen bitmap
 * which is an exact duplicate of what is visible on the monitor, the view manager creates
 * a flicker-free graphical display of the mouse and all visible windows.
 *
 * NOTE: FOR MANY OF THE FOLLOWING PROCEDURES, A "void *scrnContent" IS LISTED AMONG THE
 * PARAMETERS.  THIS PARAMETER REFERS TO THE STRUCTURE FOR WHICH THE WINDOW WAS CREATED, BE
 * IT A (Buffer*), (Dialog*), (TextScrn*), OR WHATEVER.  SINCE THE VIEW MANAGER ONLY
 * REQUESTS WINDOW REFRESHES AND PASSES EVENTS, THE CONTENTS OF THE WINDOW ARE UNKNOWN,
 * AND THEREFORE, ALL ARE STORED AS (void*). FROM NOW ON, THIS WILL BE KNOWN AS THE "WINDOW
 * IDENTIFIER".
 *
 * NOTE: THE TERM "WINDOW" AND THE TERM "SCREEN" ARE COMPLETELY INTERCHANGEABLE DURING
 * THE DOCUMENTATION OF ANY GUI SOURCE CODE.
 *
 * NOTE: ANY PROCEDURE IN THIS FILE WHICH, WHEN EXECUTED, RESULTS IN A VISUAL CHANGE TO 
 *          THE MONITOR (SUCH AS vmng_screen_show(), or MoveScreen())
 *			WILL ALSO RESTORE THE MONITOR'S IMAGE, TAKING CARE OF A VIDEO REFRESH REQUIREMENTS.
 */

#include "m4/gui/gui_vmng.h"
#include "m4/gui/gui_dialog.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/mem/memman.h"
#include "m4/mem/mem.h"
#include "m4/globals.h"

namespace M4 {

bool vmng_init() {
	if (_G(vmng_Initted))
		return false;
	_G(vmng_Initted) = true;

	_G(frontScreen) = nullptr;
	_G(backScreen) = nullptr;
	_G(inactiveScreens) = nullptr;

	if (!mem_register_stash_type(&_G(memtypeSCRN), sizeof(ScreenContext), 32, "+SCRN")) {
		return false;
	}
	if (!mem_register_stash_type(&_G(memtypeMATTE), sizeof(matte), 32, "+guiMATTE")) {
		return false;
	}
	if (!mem_register_stash_type(&_G(memtypeRECT), sizeof(RectList), 256, "+guiRecList")) {
		return false;
	}

	return true;
}

void vmng_shutdown() {
	ScreenContext *myScreen;
	Hotkey *myHotkeys, *tempHotkey;

	if (!_G(vmng_Initted))
		return;
	_G(vmng_Initted) = false;

	// First, destroy all active windows
	myScreen = _G(frontScreen);
	while (myScreen) {
		_G(frontScreen) = _G(frontScreen)->behind;
		if (myScreen->scrnType == SCRN_DLG) {
			vmng_Dialog_Destroy((Dialog *)myScreen->scrnContent);
		} else if (myScreen->scrnType == SCRN_TEXT) {
			vmng_TextScrn_Destroy((TextScrn *)myScreen->scrnContent);
		}

		myHotkeys = myScreen->scrnHotkeys;
		tempHotkey = myHotkeys;
		while (tempHotkey) {
			myHotkeys = myHotkeys->next;
			mem_free(tempHotkey);
			tempHotkey = myHotkeys;
		}

		mem_free_to_stash((void *)myScreen, _G(memtypeSCRN));
		myScreen = _G(frontScreen);
	}

	// Duplicate the above loop exactly for the list of inactive windows
	myScreen = _G(inactiveScreens);
	while (myScreen) {
		_G(inactiveScreens) = _G(inactiveScreens)->behind;
		if (myScreen->scrnType == SCRN_DLG) {
			vmng_Dialog_Destroy((Dialog *)myScreen->scrnContent);
		} else if (myScreen->scrnType == SCRN_TEXT) {
			vmng_TextScrn_Destroy((TextScrn *)myScreen->scrnContent);
		}
		myHotkeys = myScreen->scrnHotkeys;
		tempHotkey = myHotkeys;
		while (tempHotkey) {
			myHotkeys = myHotkeys->next;
			mem_free(tempHotkey);
			//mem_free_to_stash((void*)tempHotkey, memtypeHOTKEY);
			tempHotkey = myHotkeys;
		}

		mem_free_to_stash((void *)myScreen, _G(memtypeSCRN));
		myScreen = _G(inactiveScreens);
	}
}

ScreenContext *vmng_screen_create(int32 x1, int32 y1, int32 x2, int32 y2, int32 scrnType, uint32 scrnFlags,
	void *scrnContent, RefreshFunc redraw, EventHandler evtHandler) {
	ScreenContext *myScreen;

	if (!_G(vmng_Initted))
		return nullptr;

	if ((myScreen = (ScreenContext *)mem_get_from_stash(_G(memtypeSCRN), "+SCRN")) == nullptr)
		return nullptr;

	myScreen->x1 = x1;
	myScreen->y1 = y1;
	myScreen->x2 = x2;
	myScreen->y2 = y2;
	myScreen->scrnType = scrnType;
	myScreen->scrnFlags = scrnFlags;
	myScreen->scrnContent = scrnContent;
	myScreen->redraw = redraw;
	myScreen->evtHandler = evtHandler;
	myScreen->scrnHotkeys = nullptr;

	if (_G(inactiveScreens))
		_G(inactiveScreens)->infront = myScreen;

	myScreen->behind = _G(inactiveScreens);
	myScreen->infront = nullptr;
	_G(inactiveScreens) = myScreen;
	return myScreen;
}

#define LEFT_EDGE 1
#define RIGHT_EDGE 2

//--------------------------------------------------------------------------
RectList *vmng_CreateNewRect(int32 x1, int32 y1, int32 x2, int32 y2) {
	RectList *newRect;
	if ((newRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectList")) == NULL) {
		error_show(FL, 'OOS!', "vmng_CreateNewRect");
	}
	newRect->x1 = x1;
	newRect->y1 = y1;
	newRect->x2 = x2;
	newRect->y2 = y2;
	newRect->next = NULL;
	newRect->prev = NULL;

	return newRect;
}

//--------------------------------------------------------------------------
void vmng_AddRectToRectList(RectList **theRectList, int32 rectX1, int32 rectY1, int32 rectX2, int32 rectY2) {
	RectList *dirtyRect, *dirtyRectList;
	RectList *endCleanRectList, *cleanRectList;
	RectList *myRect, *myRectList;
	RectList *newRect;
	bool		intersected;

	//first make sure we have a valid rectangle
	if ((rectX1 > rectX2) || (rectY1 > rectY2)) {
		return;
	}

	//intialize the dirty rect list
	dirtyRectList = vmng_CreateNewRect(rectX1, rectY1, rectX2, rectY2);

	//intialize the clean rectList
	cleanRectList = NULL;
	endCleanRectList = NULL;

	//use a local var for theRectlist
	myRectList = *theRectList;

	//loop through all the dirtyRects
	dirtyRect = dirtyRectList;
	while (dirtyRect) {

		//remove dirtyRect from the head of the dirtyRectList
		dirtyRectList = dirtyRectList->next;

		//set the intersected flag
		intersected = false;

		//loop on through
		myRect = myRectList;
		while (myRect) {

			//if the two rectangles intersect
			if ((dirtyRect->x1 <= myRect->x2) && (dirtyRect->x2 >= myRect->x1) && (dirtyRect->y1 <= myRect->y2) && (dirtyRect->y2 >= myRect->y1)) {

				//set the intersected flag
				intersected = true;

				//if dirtyRect is not completely contained within myRect
				if ((dirtyRect->x1 < myRect->x1) || (dirtyRect->y1 < myRect->y1) || (dirtyRect->x2 > myRect->x2) || (dirtyRect->y2 > myRect->y2)) {

					//first remove it from the list
					if (myRect->prev) {
						myRect->prev->next = myRect->next;
					} else {
						myRectList = myRect->next;
					}
					if (myRect->next) {
						myRect->next->prev = myRect->prev;
					}

					//so now there is an intersection.
					//if myRect sticks out above dirtyRect, chop it off and put it in the main rect list, to be recheck by other dirty rects
					if (myRect->y1 < dirtyRect->y1) {
						newRect = vmng_CreateNewRect(myRect->x1, myRect->y1, myRect->x2, dirtyRect->y1 - 1);
						newRect->prev = NULL;
						newRect->next = myRectList;
						if (myRectList) {
							myRectList->prev = newRect;
						}
						myRectList = newRect;

						//and set the top of myRect to be the same as dirtyRect
						myRect->y1 = dirtyRect->y1;
					}

					//else if dirtyRect sticks out above chop it off and put it on the dirty list
					else if (dirtyRect->y1 < myRect->y1) {
						newRect = vmng_CreateNewRect(dirtyRect->x1, dirtyRect->y1, dirtyRect->x2, myRect->y1 - 1);
						newRect->next = dirtyRectList;
						dirtyRectList = newRect;

						//and set the top of dirtyRect to be the same as myRect
						dirtyRect->y1 = myRect->y1;
					}

					//if myRect sticks out below dirtyRect, chop it off and put it in the main rect list, to be recheck by other dirty rects
					if (myRect->y2 > dirtyRect->y2) {
						newRect = vmng_CreateNewRect(myRect->x1, dirtyRect->y2 + 1, myRect->x2, myRect->y2);
						newRect->prev = NULL;
						newRect->next = myRectList;
						if (myRectList) {
							myRectList->prev = newRect;
						}
						myRectList = newRect;

						//and set the bottom of myRect to be the same as dirtyRect
						myRect->y2 = dirtyRect->y2;
					}

					//else if dirtyRect sticks out below myRect...
					else if (dirtyRect->y2 > myRect->y2) {
						newRect = vmng_CreateNewRect(dirtyRect->x1, myRect->y2 + 1, dirtyRect->x2, dirtyRect->y2);
						newRect->next = dirtyRectList;
						dirtyRectList = newRect;

						//and set the bottom of dirtyRect to be the same as myRect
						dirtyRect->y2 = myRect->y2;
					}

					//now we've got overlapping rectangles which are the same height. create one max width one
					//if the dirtyRect sticks out on either side, the resulting rect is still dirty, otherwise clean
					if ((dirtyRect->x1 < myRect->x1) || (dirtyRect->x2 > myRect->x2)) {

						//use dirtyRect to become the max width rect
						dirtyRect->x1 = imath_min(dirtyRect->x1, myRect->x1);
						dirtyRect->x2 = imath_max(dirtyRect->x2, myRect->x2);
						dirtyRect->next = dirtyRectList;
						dirtyRectList = dirtyRect;

						//and turf myRect
						mem_free_to_stash((void *)myRect, _G(memtypeRECT));
					}

					//else we can put what's left of myRect onto the clean list and turf dirtyRect
					//note, it is impossible to split the dirtyRect list vertically, they always stretch horizontally,
					//therefore if this dirty rect does not stick out the sides, what's left of myRect is clean
					else {
						myRect->prev = NULL;
						myRect->next = cleanRectList;
						if (cleanRectList) {
							cleanRectList->prev = myRect;
						} else {
							endCleanRectList = myRect;
						}
						cleanRectList = myRect;

						mem_free_to_stash((void *)dirtyRect, _G(memtypeRECT));
					}

					//exit the loop
					myRect = NULL;
				}

				//else through away dirtyRect, and get the next dirtyRect
				else {
					mem_free_to_stash((void *)dirtyRect, _G(memtypeRECT));
					myRect = NULL;
				}
			}

			//else get the next rect
			else {
				myRect = myRect->next;
			}
		}

		//if we didn't intersect, put the dirtyRect on the clean list
		if (!intersected) {
			dirtyRect->prev = NULL;
			dirtyRect->next = cleanRectList;
			if (cleanRectList) {
				cleanRectList->prev = dirtyRect;
			} else {
				endCleanRectList = dirtyRect;
			}
			cleanRectList = dirtyRect;
		}

		//get the next dirty rect
		dirtyRect = dirtyRectList;
	}

	//now, just add the clean list onto the dirty list
	if (cleanRectList) {

		//now add the entire cleanRectList to the front of myRectList
		endCleanRectList->next = myRectList;
		if (myRectList) {
			myRectList->prev = endCleanRectList;
		}
		myRectList = cleanRectList;
	}

	//return the rect list
	*theRectList = myRectList;
}

//----------------------------------------------------------------------------------------
RectList *vmng_DuplicateRectList(RectList *myRectList) {
	RectList *newRectList, *tempRect, *myRect, *prevRect;

	newRectList = NULL;
	prevRect = NULL;
	myRect = myRectList;
	while (myRect) {

		//duplicate myRect and stick it on the newRectList
		if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectList")) == NULL) {
			error_show(FL, 'OOS!', "vmng_DuplicateRectList()");
		}
		tempRect->x1 = myRect->x1;
		tempRect->y1 = myRect->y1;
		tempRect->x2 = myRect->x2;
		tempRect->y2 = myRect->y2;
		tempRect->prev = prevRect;
		tempRect->next = NULL;
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

//----------------------------------------------------------------------------------------
bool vmng_RectIntersectsRectList(RectList *myRectList, int32 x1, int32 y1, int32 x2, int32 y2) {
	RectList *myRect;
	int32			intrX1, intrY1, intrX2, intrY2;

	//parameter verification
	if ((!myRectList) || (x1 > x2) || (y1 > y2)) {
		return false;
	}

	//loop through the list, and break as soon as there is an intersection
	myRect = myRectList;
	while (myRect) {

		//calculate the intersection
		intrX1 = imath_max(myRect->x1, x1);
		intrY1 = imath_max(myRect->y1, y1);
		intrX2 = imath_min(myRect->x2, x2);
		intrY2 = imath_min(myRect->y2, y2);

		//if we intersected, return true
		if ((intrX1 <= intrX2) && (intrY1 <= intrY2)) {
			return true;
		}

		//else get the next rect in the list
		myRect = myRect->next;
	}

	//we made it through the entire list with no intersections - return false
	return false;
}

//----------------------------------------------------------------------------------------
bool vmng_ClipRectList(RectList **myRectList, int32 clipX1, int32 clipY1, int32 clipX2, int32 clipY2) {
	RectList *nextRect, *myRect;

	int32			x1, y1, x2, y2;

	//loop through myRect list
	myRect = *myRectList;
	while (myRect) {

		//set the next rect
		nextRect = myRect->next;

		//clip myRect
		x1 = imath_max(myRect->x1, clipX1);
		y1 = imath_max(myRect->y1, clipY1);
		x2 = imath_min(myRect->x2, clipX2);
		y2 = imath_min(myRect->y2, clipY2);

		//if we have a valid rectangle
		if ((x1 <= x2) && (y1 <= y2)) {

			//clip the rectangle
			myRect->x1 = x1;
			myRect->y1 = y1;
			myRect->x2 = x2;
			myRect->y2 = y2;
		}

		//else remove it from the rectList and turf it
		else {
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

		//check the next rect
		myRect = nextRect;
	}
	return true;
}

//----------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------
void vmng_DisposeRectList(RectList **rectList) {
	RectList *myRect;

	//loop through the rect list
	myRect = *rectList;
	while (myRect) {

		//remove myRect from the head of the list
		*rectList = myRect->next;

		//dispose of myRect;
		mem_free_to_stash((void *)myRect, _G(memtypeRECT));

		//get the next rectangle
		myRect = *rectList;
	}
}

//----------------------------------------------------------------------------------------
void vmng_RemoveRectFromRectList(RectList **scrnRectList, int32 x1, int32 y1, int32 x2, int32 y2) {
	RectList *myRect, *prevRect, *nextRect, *tempRect;
	RectList *rectList, *unsortedRectList;
	int32		tempX1, tempY1, tempX2, tempY2;
	bool		finished;

	rectList = *scrnRectList;

	//go through the rectList list breaking down any rects which intersect the given coords
	unsortedRectList = NULL;
	myRect = rectList;
	while (myRect) {

		//set the nextRect pointer
		nextRect = myRect->next;

		//check for an intersection
		tempX1 = imath_max(x1, myRect->x1);
		tempY1 = imath_max(y1, myRect->y1);
		tempX2 = imath_min(x2, myRect->x2);
		tempY2 = imath_min(y2, myRect->y2);

		//if we have an intersection
		if ((tempX1 <= tempX2) && (tempY1 <= tempY2)) {

			//break myRect apart into any pieces not covered by x1, y1, x2, y2
			//top edge
			if (myRect->y1 < y1) {

				//create a new rect of just the part that extends beyond the top of myRect
				if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectangle")) == NULL) {
					error_show(FL, 'OOS!', "vmng_AddRectToRectList");
				}
				tempRect->x1 = myRect->x1;
				tempRect->y1 = myRect->y1;
				tempRect->x2 = myRect->x2;
				tempRect->y2 = y1 - 1;

				//add tempRect to the unsortedRectList
				tempRect->next = unsortedRectList;
				unsortedRectList = tempRect;

				//update myRect
				myRect->y1 = y1;
			}

			//bottom edge
			if (myRect->y2 > y2) {

				//create a new rect of just the part that extends beyond the top of myRect
				if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectangle")) == NULL) {
					error_show(FL, 'OOS!', "vmng_AddRectToRectList");
				}
				tempRect->x1 = myRect->x1;
				tempRect->y1 = y2 + 1;
				tempRect->x2 = myRect->x2;
				tempRect->y2 = myRect->y2;

				//add tempRect to the unsortedRectList
				tempRect->next = unsortedRectList;
				unsortedRectList = tempRect;

				//update myRect
				myRect->y2 = y2;
			}

			//left edge
			if (myRect->x1 < x1) {

				//create a new rect of just the part that extends beyond the top of myRect
				if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectangle")) == NULL) {
					error_show(FL, 'OOS!', "vmng_AddRectToRectList");
				}
				tempRect->x1 = myRect->x1;
				tempRect->y1 = myRect->y1;
				tempRect->x2 = x1 - 1;
				tempRect->y2 = myRect->y2;

				//add tempRect to the unsortedRectList
				tempRect->next = unsortedRectList;
				unsortedRectList = tempRect;
			}

			//right edge
			if (myRect->x2 > x2) {

				//create a new rect of just the part that extends beyond the top of myRect
				if ((tempRect = (RectList *)mem_get_from_stash(_G(memtypeRECT), "+guiRectangle")) == NULL) {
					error_show(FL, 'OOS!', "vmng_AddRectToRectList");
				}
				tempRect->x1 = x2 + 1;
				tempRect->y1 = myRect->y1;
				tempRect->x2 = myRect->x2;
				tempRect->y2 = myRect->y2;

				//add tempRect to the unsortedRectList
				tempRect->next = unsortedRectList;
				unsortedRectList = tempRect;
			}

			//remove myRect from the list and turf it
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

		//get the next rect
		myRect = nextRect;
	}

	//now go through the unsorted list and insert them into the main list
	tempRect = unsortedRectList;
	while (tempRect) {

		unsortedRectList = unsortedRectList->next;

		//for each unsorted rect, loop through the rect list until its place is found
		finished = false;
		prevRect = NULL;
		myRect = rectList;
		while (myRect && (!finished)) {

			//if it goes before myRect
			if (tempRect->y2 <= myRect->y2) {
				finished = true;
			} else {
				prevRect = myRect;
				myRect = myRect->next;
			}
		}

		//tempRect belongs after prevRect
		if (prevRect) {
			tempRect->prev = prevRect;
			tempRect->next = prevRect->next;
			if (prevRect->next) {
				prevRect->next->prev = tempRect;
			}
			prevRect->next = tempRect;
		}

		//else it belongs at the front of rectList
		else {
			tempRect->prev = NULL;
			tempRect->next = rectList;
			if (rectList) {
				rectList->prev = tempRect;
			}
			rectList = tempRect;
		}

		//get the next unsorted rect
		tempRect = unsortedRectList;
	}

	//set the screen rect list to the resulting rect list
	*scrnRectList = rectList;
}

} // End of namespace M4
