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

#include "m4/gui/gui_mouse.h"
#include "m4/gui/gui_vmng.h"
#include "m4/core/imath.h"
#include "m4/core/mouse.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/mem/res.h"
#include "m4/vars.h"

namespace M4 {

static void transShow(void *s, void *r, void *b, int32 destX, int32 destY);

bool gui_mouse_init() {
	_G(mouseBuffer).data = nullptr;
	if ((_G(mouseBuffer).data = (uint8 *)mem_alloc(32 * 32, "mouse graphic")) == nullptr) {
		return false;
	}
	_G(mouseBuffer).w = 32;
	_G(mouseBuffer).stride = 32;
	_G(mouseBuffer).h = 32;

	auto &mouseSprite = _G(mouseSprite);
	if (!mouseSprite) {
		if ((mouseSprite = (M4sprite *)mem_alloc(sizeof(M4sprite), "mouse sprite")) == nullptr) {
			return false;
		}

		mouseSprite->x = 0;
		mouseSprite->y = 0;
		mouseSprite->w = 32;
		mouseSprite->h = 32;
		mouseSprite->xOffset = 0;
		mouseSprite->yOffset = 0;
		mouseSprite->encoding = 0;
		mouseSprite->data = GetMousePicture();
		mouseSprite->sourceHandle = nullptr;
		mouseSprite->sourceOffset = 0;

		_G(mouseX1offset) = mouseSprite->xOffset;
		_G(mouseY1offset) = mouseSprite->yOffset;
		_G(mouseX2offset) = mouseSprite->w - _G(mouseX1offset) - 1;
		_G(mouseY2offset) = mouseSprite->h - _G(mouseY1offset) - 1;
	}

	if ((_G(mouseScreenSource) = (transSprite *)mem_alloc(sizeof(transSprite), "mouse transSprite")) == nullptr) {
		return false;
	}

	_G(mouseScreenSource)->srcSprite = mouseSprite;
	_G(mouseScreenSource)->scrnBuffer = &_G(mouseBuffer);

	if ((_G(mouseScreen) = vmng_screen_create(0, 0, mouseSprite->w - 1, mouseSprite->h - 1, SCRN_TRANS, SF_MOUSE | SF_OFFSCRN | SF_TRANSPARENT,
		(void *)_G(mouseScreenSource), transShow, nullptr)) == nullptr) {
		return false;
	}

	_G(mouseSeriesHandle) = nullptr;
	_G(mouseSeriesOffset) = 0;
	_G(mouseSeriesPalOffset) = 0;

	return true;
}

void gui_mouse_shutdown() {
	mem_free(_G(mouseSprite));
	gr_buffer_free(&_G(mouseBuffer));
	mem_free((void *)_G(mouseScreenSource));

	_G(mouseSeriesHandle) = nullptr;
	_G(mouseSeriesOffset) = 0;
	_G(mouseSeriesPalOffset) = 0;
}

void transShow(void *s, void *r, void *b, int32 destX, int32 destY) {
	ScreenContext *myScreen = (ScreenContext *)s;
	matte *myRectList = (matte *)r;
	Buffer *destBuffer = (Buffer *)b;
	ScreenContext *tempScreen;
	transSprite *mySource;
	M4sprite *mySprite;
	Buffer *myBuff;
	Buffer drawSpriteBuff;
	DrawRequest spriteDrawReq;
	matte *myMatte, tempMatte;
	RectList *updateList, *updateRect;
	RectList *newUpdateList;
	uint8 *rowPtr, *destPtr;
	int32 i, j;

	// Parameter verification
	if (!myScreen)
		return;

	mySource = (transSprite *)(myScreen->scrnContent);
	if (!mySource)
		return;

	myBuff = (Buffer *)(mySource->scrnBuffer);
	if (!myBuff)
		return;

	mySprite = mySource->srcSprite;
	if (!mySprite)
		return;

	// If no destBuffer, then draw directly to video
	if (!destBuffer) {
		tempMatte.nextMatte = nullptr;

		// Loop through the dirty matte list
		myMatte = myRectList;
		while (myMatte) {

			// Create an updateRectList to catch the black areas afterwards
			updateList = vmng_CreateNewRect(myMatte->x1, myMatte->y1, myMatte->x2, myMatte->y2);
			updateList->prev = nullptr;
			updateList->next = nullptr;

			// Now loop through all the screens behind myScreen
			tempScreen = myScreen->behind;
			while (tempScreen && updateList) {
				// Duplicate the updateList
				newUpdateList = vmng_DuplicateRectList(updateList);

				// Loop through the updateList
				updateRect = updateList;
				while (updateRect) {
					// See if it intersects
					tempMatte.x1 = imath_max(updateRect->x1, tempScreen->x1);
					tempMatte.y1 = imath_max(updateRect->y1, tempScreen->y1);
					tempMatte.x2 = imath_min(updateRect->x2, tempScreen->x2);
					tempMatte.y2 = imath_min(updateRect->y2, tempScreen->y2);

					if (tempScreen->redraw && (tempMatte.x1 <= tempMatte.x2) && (tempMatte.y1 <= tempMatte.y2)) {
						// Draw the intersected part of tempScreen onto myBuffer
						(tempScreen->redraw)(tempScreen, (void *)&tempMatte, myBuff, tempMatte.x1 - myScreen->x1, tempMatte.y1 - myScreen->y1);

						// Remove that rectangle from the update list
						vmng_RemoveRectFromRectList(&newUpdateList, tempMatte.x1, tempMatte.y1, tempMatte.x2, tempMatte.y2);
					}

					// Get the next updateRect
					updateRect = updateRect->next;
				}

				// The newUpdateList now contains all the pieces not covered by tempScreen;
				// turf the update list, and replace it with the newupdateList
				vmng_DisposeRectList(&updateList);
				updateList = newUpdateList;

				// Now get the next screen
				tempScreen = tempScreen->behind;
			}

			// Now we've gone through all the screens, whatever is left in the updateList should be filled in with black
			gr_color_set(__BLACK);
			updateRect = updateList;
			while (updateRect) {
				gr_buffer_rect_fill(myBuff, updateRect->x1 - myScreen->x1, updateRect->y1 - myScreen->y1,
					updateRect->x2 - updateRect->x1 + 1, updateRect->y2 - updateRect->y1 + 1);
				updateRect = updateRect->next;
			}

			// Now dispose of the updateList
			vmng_DisposeRectList(&updateList);

			// And finally, get the next matte
			myMatte = myMatte->nextMatte;
		}

		// Now myBuff should contain a copy of everything on the screen, except the actual contents of this transparent screen
		// Now would be the time to draw the contents
		if (mySprite->sourceHandle) {
			HLock(mySprite->sourceHandle);
			mySprite->data = (uint8 *)((intptr)*(mySprite->sourceHandle) + mySprite->sourceOffset);

			drawSpriteBuff.w = mySprite->w;
			drawSpriteBuff.stride = mySprite->w;
			drawSpriteBuff.h = mySprite->h;
			drawSpriteBuff.encoding = (mySprite->encoding) & (uint8)0x7f;
			drawSpriteBuff.data = mySprite->data;

			spriteDrawReq.Src = &drawSpriteBuff;
			spriteDrawReq.Dest = myBuff;
			spriteDrawReq.x = 0;
			spriteDrawReq.y = 0;
			spriteDrawReq.scaleX = 100;
			spriteDrawReq.scaleY = 100;
			spriteDrawReq.srcDepth = 0;
			spriteDrawReq.depthCode = nullptr;
			spriteDrawReq.Pal = nullptr;
			spriteDrawReq.ICT = nullptr;

			gr_sprite_draw(&spriteDrawReq);

			// Unlock the handle
			HUnLock(mySprite->sourceHandle);
		} else if (mySprite->data) {
			// Else the data for the transparent sprite is stored directly in mySprite->data

			// Loop through the rows
			for (j = 0; (j < mySprite->h) && (j < myBuff->h); j++) {
				// Set the rowPtr and the destPtr
				rowPtr = mySprite->data + (j * mySprite->w);
				destPtr = myBuff->data + (j * myBuff->stride);

				// Loop through the columns
				for (i = 0; (i < mySprite->w) && (i < myBuff->w); i++) {
					if (*rowPtr) {
						*destPtr = *rowPtr;
					}

					destPtr++;
					rowPtr++;
				}
			}
		}

		// Now dump the matte list out to video
		myMatte = myRectList;
		while (myMatte) {
			vmng_refresh_video(myMatte->x1, myMatte->y1, myMatte->x1 - myScreen->x1, myMatte->y1 - myScreen->y1,
				myMatte->x2 - myScreen->x1, myMatte->y2 - myScreen->y1, myBuff);
			myMatte = myMatte->nextMatte;
		}
	} else {
		// Else draw to the dest buffer
		myMatte = myRectList;
		while (myMatte) {
			gr_buffer_rect_copy_2(myBuff, destBuffer, myMatte->x1 - myScreen->x1, myMatte->y1 - myScreen->y1,
				destX, destY, myMatte->x2 - myMatte->x1 + 1, myMatte->y2 - myMatte->y1 + 1);
			myMatte = myMatte->nextMatte;
		}
	}
}

bool mouse_set_sprite(int32 spriteNum) {
	M4sprite *tempSprite;
	int32 minX, minY, maxX, maxY;

	if (_G(mouseIsLocked)) {
		_G(newMouseNum) = spriteNum;
		return true;
	}

	if (spriteNum == _G(currMouseNum)) {
		return true;
	}

	if (!_G(mouseSeriesHandle) || !*_G(mouseSeriesHandle))
		return false;

	minX = _G(oldX) - _G(mouseX1offset);
	minY = _G(oldY) - _G(mouseY1offset);
	maxX = _G(oldX) + _G(mouseX2offset);
	maxY = _G(oldY) + _G(mouseY2offset);

	if ((tempSprite = CreateSprite(_G(mouseSeriesHandle), _G(mouseSeriesOffset), spriteNum,
			_G(mouseSprite), nullptr)) == nullptr)
		return false;

	_G(mouseSprite) = tempSprite;
	_G(mouseX1offset) = _G(mouseSprite)->xOffset;
	_G(mouseY1offset) = _G(mouseSprite)->yOffset;
	_G(mouseX2offset) = _G(mouseSprite)->w - _G(mouseX1offset) - 1;
	_G(mouseY2offset) = _G(mouseSprite)->h - _G(mouseY1offset) - 1;
	if (_G(mouseX) - _G(mouseX1offset) < minX)
		minX = _G(mouseX) - _G(mouseX1offset);
	if (_G(mouseY) - _G(mouseY1offset) < minY)
		minY = _G(mouseY) - _G(mouseY1offset);
	if (_G(mouseX) + _G(mouseX2offset) > maxX)
		maxX = _G(mouseX) + _G(mouseX2offset);
	if (_G(mouseY) + _G(mouseY2offset) > maxY)
		maxY = _G(mouseY) + _G(mouseY2offset);

	gui_mouse_refresh();
	_G(currMouseNum) = spriteNum;

	return true;
}

void gui_mouse_refresh() {
	MoveScreenAbs(_G(mouseScreen), _G(mouseX) - _G(mouseX1offset),
		_G(mouseY) - _G(mouseY1offset));
}

void mouse_hide() {
	vmng_screen_hide(_G(mouseScreenSource));
}

void mouse_show() {
	vmng_screen_show(_G(mouseScreenSource));
}

void mouse_lock_sprite(int32 mouseNum) {
	_G(mouseIsLocked) = false;
	mouse_set_sprite(mouseNum);
	_G(mouseIsLocked) = true;
}

void mouse_unlock_sprite() {
	_G(mouseIsLocked) = false;
	mouse_set_sprite(_G(newMouseNum));
}

} // End of namespace M4
