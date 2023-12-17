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

#include "common/system.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/gui/gui_vmng.h"
#include "m4/vars.h"

namespace M4 {

static void vmng_black_out_video(int32 x1, int32 y1, int32 x2, int32 y2);
static bool MoveScreen(ScreenContext *myScreen, int32 parmX, int32 parmY, bool deltaMove);

bool GetScreenCoords(void *scrnContent, int32 *x1, int32 *y1, int32 *x2, int32 *y2) {
	ScreenContext *myScreen;
	if ((myScreen = vmng_screen_find(scrnContent, nullptr)) == nullptr)
		return false;

	if (x1)
		*x1 = myScreen->x1;
	if (y1)
		*y1 = myScreen->y1;
	if (x2)
		*x2 = myScreen->x2;
	if (y2)
		*y2 = myScreen->y2;

	return true;
}

void RestoreScreens(int32 updateX1, int32 updateY1, int32 updateX2, int32 updateY2) {
	ScreenContext *myScreen, *tempScreen;
	RectList *updateRectList, *scrnUpdateList, *tempRect;

	if (!_G(vmng_Initted) || _G(system_shutting_down)) {
		return;
	}

	// Clip to screen
	if (updateX1 < MIN_VIDEO_X)
		updateX1 = MIN_VIDEO_X;
	if (updateY1 < MIN_VIDEO_Y)
		updateY1 = MIN_VIDEO_Y;
	if (updateX2 > MAX_VIDEO_X)
		updateX2 = MAX_VIDEO_X;
	if (updateY2 > MAX_VIDEO_Y)
		updateY2 = MAX_VIDEO_Y;

	// Throw away inside out rectangles
	if ((updateX2 < updateX1) || (updateY2 < updateY1)) {
		return;
	}

	// Create a master updateRectList - when this list is empty, the screen has been redrawn
	updateRectList = vmng_CreateNewRect(updateX1, updateY1, updateX2, updateY2);

	// First loop through the screens back to front, drawing only the transparent screens
	myScreen = _G(backScreen);

	while (myScreen && updateRectList) {
		// We only draw transparent screens on the first pass
		if ((myScreen->scrnFlags & SF_TRANSPARENT) && myScreen->redraw) {
			// Duplicate the updateRectList
			scrnUpdateList = vmng_DuplicateRectList(updateRectList);

			// Now clip the scrnUpdateList with the coordinates of myScreen, see if there is anything visible
			vmng_ClipRectList(&scrnUpdateList, myScreen->x1, myScreen->y1, myScreen->x2, myScreen->y2);

			// Now remove the rects of all screens blocking the scrnUpdateList
			tempScreen = myScreen->infront;

			while (scrnUpdateList && tempScreen) {
				vmng_RemoveRectFromRectList(&scrnUpdateList, tempScreen->x1, tempScreen->y1, tempScreen->x2, tempScreen->y2);
				tempScreen = tempScreen->infront;
			}

			// If there is anything left of the scrnUpdateList, redraw it
			if (scrnUpdateList) {
				(myScreen->redraw)((void *)myScreen, (void *)scrnUpdateList, nullptr, 0, 0);

				// Now remove all the updated rects from the updateRectList
				tempRect = scrnUpdateList;
				while (tempRect && updateRectList) {
					vmng_RemoveRectFromRectList(&updateRectList, tempRect->x1, tempRect->y1, tempRect->x2, tempRect->y2);
					tempRect = tempRect->next;
				}

				// Now turf the scrnUpdateList
				vmng_DisposeRectList(&scrnUpdateList);
			}
		}

		// Get the next screen
		myScreen = myScreen->infront;
	}

	// Now, draw from front to back the non-transparent screens.  This way, we can detect when the entire update region has been
	// filled, however, if one transparent screen is infront of another, the back transparent screen must be drawn first.
	myScreen = _G(frontScreen);
	while (myScreen && updateRectList) {
		// Make sure it isn't a transparent screen
		if (!(myScreen->scrnFlags & SF_TRANSPARENT) && myScreen->redraw) {
			// Duplicate the updateRectList
			scrnUpdateList = vmng_DuplicateRectList(updateRectList);
			// Now clip the scrnUpdateList with the coordinates of myScreen, see what has to be redrawn
			vmng_ClipRectList(&scrnUpdateList, myScreen->x1, myScreen->y1, myScreen->x2, myScreen->y2);

			// If there is anything visible, redraw it
			if (scrnUpdateList) {
				myScreen->redraw((void *)myScreen, (void *)scrnUpdateList, nullptr, 0, 0);

				// Now remove all the updated rects from the updateRectList
				tempRect = scrnUpdateList;
				while (tempRect && updateRectList) {
					vmng_RemoveRectFromRectList(&updateRectList, tempRect->x1, tempRect->y1, tempRect->x2, tempRect->y2);
					tempRect = tempRect->next;
				}

				// Now turf the scrnUpdateList
				vmng_DisposeRectList(&scrnUpdateList);
			}
		}

		// Get the next screen
		myScreen = myScreen->behind;
	}

	// Now, if there is anything left, it should be filled in with black
	if (updateRectList) {
		tempRect = updateRectList;
		while (tempRect) {
			vmng_black_out_video(tempRect->x1, tempRect->y1, tempRect->x2, tempRect->y2);
			tempRect = tempRect->next;
		}

		// Now turf the updateRectList
		vmng_DisposeRectList(&updateRectList);
	}
}

void RestoreScreensInContext(int32 x1, int32 y1, int32 x2, int32 y2, ScreenContext *myScreen) {
	ScreenContext *tempScreen;

	//verify the gui has been initted
	if (!_G(vmng_Initted)) {
		return;
	}

	//verify parameters
	tempScreen = _G(frontScreen);
	while (tempScreen && (tempScreen != myScreen)) {
		tempScreen = tempScreen->behind;
	}
	if (!tempScreen) {
		return;
	}

	//now restore screens
	x1 += myScreen->x1;
	y1 += myScreen->y1;
	x2 += myScreen->x1;
	y2 += myScreen->y1;

	//restore video
	RestoreScreens(x1, y1, x2, y2);
}

bool ResizeScreen(void *scrnContent, int32 newW, int32 newH) {
	ScreenContext *myScreen;
	int32 status, oldX2, oldY2;

	if ((newW <= 0) || (newH <= 0))
		return false;
	if ((myScreen = vmng_screen_find(scrnContent, &status)) == nullptr)
		return false;

	oldX2 = myScreen->x2;
	myScreen->x2 = myScreen->x1 + newW - 1;

	if (myScreen->x2 < oldX2) {
		RestoreScreens(myScreen->x2 + 1, myScreen->y1, oldX2, myScreen->y2);
	}

	oldY2 = myScreen->y2;
	myScreen->y2 = myScreen->y1 + newH - 1;

	if (myScreen->y2 < oldY2) {
		RestoreScreens(myScreen->x1, myScreen->y2 + 1, myScreen->x2, oldY2);
	}

	return true;
}

static void vmng_black_out_video(int32 x1, int32 y1, int32 x2, int32 y2) {
	Graphics::Surface *screen = g_system->lockScreen();
	Common::Rect r(x1, y1, x2 + 1, y2 + 1);
	screen->fillRect(r, 0);
	g_system->unlockScreen();
}

bool AddScreenHotkey(void *scrnContent, int32 myKey, HotkeyCB callback) {
	ScreenContext *myScreen;
	Hotkey *myHotkey;

	if ((myScreen = vmng_screen_find(scrnContent, nullptr)) == nullptr)
		return false;

	if ((myHotkey = (Hotkey *)mem_alloc(sizeof(Hotkey), "hotkey")) == nullptr)
		return false;

	myHotkey->myKey = myKey;
	myHotkey->callback = callback;
	myHotkey->next = myScreen->scrnHotkeys;
	myScreen->scrnHotkeys = myHotkey;
	return true;
}

bool RemoveScreenHotkey(void *scrnContent, int32 myKey) {
	ScreenContext *myScreen;
	Hotkey *myHotkey, *tempHotkey;
	if ((myScreen = vmng_screen_find(scrnContent, nullptr)) == nullptr)
		return false;

	myHotkey = myScreen->scrnHotkeys;

	if (myHotkey->myKey == myKey) {
		myScreen->scrnHotkeys = myHotkey->next;
		mem_free(myHotkey);

	} else {
		while (myHotkey->next && (myHotkey->next->myKey != myKey)) {
			myHotkey = myHotkey->next;
		}
		if (myHotkey->next) {
			tempHotkey = myHotkey->next;
			myHotkey->next = tempHotkey->next;
			mem_free(tempHotkey);
		} else {
			return false;
		}
	}

	return true;
}

bool MoveScreenAbs(ScreenContext *myScreen, int32 parmX, int32 parmY) {
	return MoveScreen(myScreen, parmX, parmY, false);
}

bool MoveScreenDelta(ScreenContext *myScreen, int32 parmX, int32 parmY) {
	return MoveScreen(myScreen, parmX, parmY, true);
}

void vmng_screen_to_back(void *scrnContent) {
	ScreenContext *myScreen, *tempScreen;
	if ((myScreen = ExtractScreen(scrnContent, SCRN_ANY)) == nullptr) return;
	if (!_G(backScreen)) {
		myScreen->infront = nullptr;
		myScreen->behind = nullptr;
		_G(frontScreen) = myScreen;
		_G(backScreen) = myScreen;
	} else {
		tempScreen = _G(backScreen);
		while (tempScreen &&
			((tempScreen->scrnFlags & SF_LAYER) < (myScreen->scrnFlags & SF_LAYER))) {
			tempScreen = tempScreen->infront;
		}
		if (!tempScreen) {
			myScreen->infront = nullptr;
			myScreen->behind = _G(frontScreen);
			_G(frontScreen)->infront = myScreen;
			_G(frontScreen) = myScreen;
		} else if (tempScreen == _G(backScreen)) {
			myScreen->infront = _G(backScreen);
			myScreen->behind = nullptr;
			_G(backScreen)->behind = myScreen;
			_G(backScreen) = myScreen;
		} else {
			myScreen->infront = tempScreen;
			myScreen->behind = tempScreen->behind;
			tempScreen->behind = myScreen;
			myScreen->behind->infront = myScreen;
		}
	}

	RestoreScreens(myScreen->x1, myScreen->y1, myScreen->x2, myScreen->y2);
}

static bool MoveScreen(ScreenContext *myScreen, int32 parmX, int32 parmY, bool deltaMove) {
	int32 origX1, origY1, origX2, origY2;
	if (!_G(vmng_Initted))
		return false;

	origX1 = myScreen->x1;
	origY1 = myScreen->y1;
	origX2 = myScreen->x2;
	origY2 = myScreen->y2;

	if (!deltaMove) {
		parmX -= origX1;
		parmY -= origY1;
	}
	if (!(myScreen->scrnFlags & SF_OFFSCRN)) {
		if ((myScreen->x2 + parmX) > MAX_VIDEO_X)
			parmX = MAX_VIDEO_X - myScreen->x2;
		else if ((myScreen->x1 + parmX) < 0)
			parmX = -myScreen->x1;
		if ((myScreen->y2 + parmY) > MAX_VIDEO_Y)
			parmY = MAX_VIDEO_Y - myScreen->y2;
		else if ((myScreen->y1 + parmY) < 0)
			parmY = -myScreen->y1;
	}

	if (!(parmX || parmY))
		return false;

	myScreen->x1 += parmX;
	myScreen->y1 += parmY;
	myScreen->x2 += parmX;
	myScreen->y2 += parmY;

	if (parmY > 0) {
		RestoreScreens(origX1, origY1, origX2, myScreen->y1 - 1);
		if (parmX > 0) {
			RestoreScreens(origX1, myScreen->y1, myScreen->x1 - 1, origY2);
		} else if (parmX < 0) {
			RestoreScreens(myScreen->x2 + 1, myScreen->y1, origX2, origY2);
		}
	} else if (parmY < 0) {
		RestoreScreens(origX1, myScreen->y2 + 1, origX2, origY2);
		if (parmX > 0) {
			RestoreScreens(origX1, origY1, myScreen->x1 - 1, myScreen->y2);
		} else if (parmX < 0) {
			RestoreScreens(myScreen->x2 + 1, origY1, origX2, myScreen->y2);
		}
	} else if (parmX > 0) {
		RestoreScreens(origX1, origY1, myScreen->x1 - 1, origY2);
	} else {
		RestoreScreens(myScreen->x2 + 1, origY1, origX2, origY2);
	}

	RestoreScreens(myScreen->x1, myScreen->y1, myScreen->x2, myScreen->y2);
	return true;
}

} // End of namespace M4
