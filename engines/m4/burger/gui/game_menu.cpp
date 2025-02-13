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

#include "graphics/thumbnail.h"
#include "m4/burger/gui/game_menu.h"
#include "m4/burger/gui/interface.h"
#include "m4/adv_r/other.h"
#include "m4/adv_r/adv_background.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_player.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/hotkeys.h"
#include "m4/graphics/gr_line.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/graphics/krn_pal.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/mem/mem.h"
#include "m4/platform/keys.h"
#include "m4/burger/hotkeys.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace GUI {

static bool buttonClosesDialog;

void UpdateThumbNails(int32 firstSlot, guiMenu *myMenu);

Sprite *menu_CreateThumbnail(int32 *spriteSize) {
	Sprite *thumbNailSprite;
	GrBuff *thumbNail;
	Buffer *scrnBuff, *intrBuff, *destBuff, RLE8Buff;
	uint8 *srcPtr, *srcPtr2, *srcPtr3, *srcRowPtr, *destPtr;
	ScreenContext *gameScreen;
	int32 i, status;
	int32 currRow, beginRow, endRow;

	// Create a Sprite for the thumbNail
	if ((thumbNailSprite = (Sprite *)mem_alloc(sizeof(Sprite), "sprite")) == nullptr) {
		return nullptr;
	}

	thumbNail = new GrBuff((MAX_VIDEO_X + 1) / 3, (MAX_VIDEO_Y + 1) / 3);
	if (!thumbNail) {
		return nullptr;
	}

	destBuff = thumbNail->get_buffer();
	if (!destBuff) {
		return nullptr;
	}

	gameScreen = vmng_screen_find(_G(gameDrawBuff), &status);
	if ((!gameScreen) || (status != SCRN_ACTIVE)) {
		return nullptr;
	}

	scrnBuff = _G(gameDrawBuff)->get_buffer();
	if (!scrnBuff) {
		return nullptr;
	}

	// Grab the interface buffer
	intrBuff = _G(gameInterfaceBuff)->get_buffer();

	if (gameScreen->y1 > 0) {
		// Paint the top of the thumbnail black
		beginRow = gameScreen->y1;
		memset(destBuff->data, 21, (beginRow / 3) * destBuff->stride);
		srcRowPtr = (uint8 *)(scrnBuff->data + (-gameScreen->x1));
		destPtr = (uint8 *)(destBuff->data + ((beginRow / 3) * destBuff->stride));
	} else {
		srcRowPtr = (uint8 *)(scrnBuff->data + ((-gameScreen->y1) * scrnBuff->stride) + (-gameScreen->x1));
		beginRow = 0;
		destPtr = destBuff->data;
	}
	endRow = imath_min(MAX_VIDEO_Y, gameScreen->y2);

	for (currRow = beginRow; currRow <= endRow; currRow += 3) {

		// Set the src pointers
		srcPtr = srcRowPtr;
		srcPtr2 = srcRowPtr + scrnBuff->stride;
		srcPtr3 = srcRowPtr + (scrnBuff->stride << 1);

		for (i = 0; i < (MAX_VIDEO_X + 1) / 3; i++) {

			// Calculate the average - make sure not to extend past the end of the buffer
			if (endRow - currRow < 1) {
				*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2)) / 3));
			} else if (endRow - currRow < 2) {
				*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2) +
					*srcPtr2 + *(srcPtr2 + 1) + *(srcPtr2 + 2)) / 6));
			} else {
				*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2) +
					*srcPtr2 + *(srcPtr2 + 1) + *(srcPtr2 + 2) +
					*srcPtr3 + *(srcPtr3 + 1) + *(srcPtr3 + 2)) / 9));
			}

			if (*destPtr == 0) {
				*destPtr = 21;
			}

			// Increment the pointers
			srcPtr += 3;
			srcPtr2 += 3;
			srcPtr3 += 3;
			destPtr++;
		}

		// Update the row pointer
		srcRowPtr += scrnBuff->stride * 3;
	}

	// Reset the currRow
	beginRow = currRow;

	// Paint the interface section of the thumbnail
	if (currRow < MAX_VIDEO_Y) {
		// If the interface is visible, grab it
		if (intrBuff) {
			srcRowPtr = intrBuff->data;
			endRow = imath_min(MAX_VIDEO_Y, beginRow + intrBuff->h - 1);
			for (currRow = beginRow; currRow <= endRow; currRow += 3) {
				// Set the src pointers
				srcPtr = srcRowPtr;
				srcPtr2 = srcRowPtr + intrBuff->stride;
				srcPtr3 = srcRowPtr + (intrBuff->stride << 1);

				for (i = 0; i < (MAX_VIDEO_X + 1) / 3; i++) {
					// If the pix is outside of the inventory objects in the interface, set to black
					// If ((srcPtr - srcRowPtr < 180) || (srcPtr - srcRowPtr > 575)) {
					if (true) {			// for now make everything in the interface black
						*destPtr = 21;
					}

					// Else calculate the average - make sure not to extend past the end of the buffer
					else {
						if (endRow - currRow < 1) {
							*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2)) / 3));
						} else if (endRow - currRow < 2) {
							*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2) +
								*srcPtr2 + *(srcPtr2 + 1) + *(srcPtr2 + 2)) / 6));
						} else {
							*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2) +
								*srcPtr2 + *(srcPtr2 + 1) + *(srcPtr2 + 2) +
								*srcPtr3 + *(srcPtr3 + 1) + *(srcPtr3 + 2)) / 9));
						}
						if (*destPtr == 0) {
							*destPtr = 21;
						}
					}

					// Increment the pointers
					srcPtr += 3;
					srcPtr2 += 3;
					srcPtr3 += 3;
					destPtr++;
				}

				// Update the row pointer
				srcRowPtr += intrBuff->stride * 3;
			}
		} else {
			// Else paint the bottom of the thumbnail black
			destPtr = (uint8 *)(destBuff->data + ((currRow / 3) * destBuff->stride));
			memset(destPtr, 21, (destBuff->h - (currRow / 3)) * destBuff->stride);
		}
	}

	// Reset the currRow
	beginRow = currRow;

	if (currRow < MAX_VIDEO_Y) {
		// Paint the bottom of the thumbnail black
		destPtr = (uint8 *)(destBuff->data + ((currRow / 3) * destBuff->stride));
		memset(destPtr, 21, (destBuff->h - (currRow / 3)) * destBuff->stride);
	}

	// Compress the thumbNail data into the RLE8Buff
	if ((*spriteSize = (int32)gr_sprite_RLE8_encode(destBuff, &RLE8Buff)) <= 0) {
		return nullptr;
	}

	// Fill in the Sprite structure
	thumbNailSprite->w = destBuff->w;
	thumbNailSprite->h = destBuff->h;
	thumbNailSprite->encoding = RLE8;
	thumbNailSprite->data = nullptr;
	if ((thumbNailSprite->sourceHandle = NewHandle(*spriteSize, "thumbNail source")) == nullptr) {
		return nullptr;
	}
	thumbNailSprite->sourceOffset = 0;

	// Now copy the RLE8Buff into the thumbNail source handle
	HLock(thumbNailSprite->sourceHandle);
	thumbNailSprite->data = (uint8 *)(*(thumbNailSprite->sourceHandle));
	memcpy(thumbNailSprite->data, RLE8Buff.data, *spriteSize);
	HUnLock(thumbNailSprite->sourceHandle);

	// Release all buffers
	_G(gameDrawBuff)->release();
	if (intrBuff) {
		_G(gameInterfaceBuff)->release();
	}
	thumbNail->release();

	// Free up both the thumbNail and the RLE8Buff
	delete thumbNail;
	mem_free((void *)RLE8Buff.data);

	return thumbNailSprite;
}

//-------------------------------    HSLIDER MENU ITEM    ---------------------------------//

enum {
	H_THUMB_NORM = 0,
	H_THUMB_OVER = 1,
	H_THUMB_PRESS = 2
};


void menu_DrawHSlider(menuItemHSlider *myItem, guiMenu *myMenu, int32 x, int32 y, int32, int32) {
	Buffer *myBuff = nullptr;
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite = nullptr;

	// Verify params
	if (!myItem || !myMenu)
		return;

	// If the item is marked transparent, get the background buffer
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}
	}

	// Get the menu buffer and draw the sprite to it
	myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Get the slider info and select the thumb sprite
	switch (myItem->itemFlags) {
	case H_THUMB_OVER:
		mySprite = _GM(menuSprites)[OM_SLIDER_BTN_OVER];
		break;
	case H_THUMB_PRESS:
		mySprite = _GM(menuSprites)[OM_SLIDER_BTN_PRESS];
		break;
	default:
	case H_THUMB_NORM:
		mySprite = _GM(menuSprites)[OM_SLIDER_BTN_NORM];
		break;
	}

	// Fill in everything left of the thumb with a hilite color
	if (myItem->thumbX > 2) {
		gr_color_set(menuItem::SLIDER_BAR_COLOR);
		gr_buffer_rect_fill(myBuff, myItem->x1 + 3, myItem->y1 + 9, myItem->thumbX, myItem->thumbH - 18);
	}

	// Draw in the thumb
	gui_DrawSprite(mySprite, myBuff, myItem->x1 + myItem->thumbX, myItem->y1);

	// Release the menu buffer
	myMenu->menuBuffer->release();
}


bool hslider_Handler(menuItemHSlider *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	bool redrawItem, execCallback, handled;
	ScreenContext *myScreen;
	int32 status;
	int32 deltaSlide;
	static bool movingFlag;
	static int32 movingX;

	// Verify params
	if (!myItem)
		return false;

	if (!(eventType == EVENT_MOUSE)) {
		return false;
	}

	redrawItem = false;
	handled = true;
	execCallback = false;

	switch (event) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (menuItem::cursorInsideItem(myItem, x, y) && (x - myItem->x1 >= myItem->thumbX) &&
				(x - myItem->x1 <= myItem->thumbX + myItem->thumbW - 1)) {
			myItem->itemFlags = H_THUMB_PRESS;
			movingFlag = true;
			movingX = x;
			*currItem = myItem;
			redrawItem = true;
		} else {
			*currItem = nullptr;
			myItem->itemFlags = 0;
			redrawItem = true;
		}
		break;

	case _ME_L_drag:
	case _ME_doubleclick_drag:
		if (!*currItem) {
			return true;
		}
		if (movingFlag) {
			if (x < movingX) {
				deltaSlide = imath_min(myItem->thumbX, movingX - x);
				if (deltaSlide > 0) {
					myItem->thumbX -= deltaSlide;
					redrawItem = true;
					myItem->percent = myItem->thumbX * 100 / myItem->maxThumbX;
					execCallback = true;
				}
			} else if (x > movingX) {
				deltaSlide = imath_min(myItem->maxThumbX - myItem->thumbX, x - movingX);
				if (deltaSlide > 0) {
					myItem->thumbX += deltaSlide;
					redrawItem = true;
					myItem->percent = myItem->thumbX * 100 / myItem->maxThumbX;
					execCallback = true;
				}
			}
			movingX = x;
			if (movingX < (myItem->thumbX + myItem->x1)) {
				movingX = myItem->thumbX + myItem->x1;
			} else if (movingX > (myItem->thumbX + myItem->thumbW - 1 + myItem->x1)) {
				movingX = myItem->thumbX + myItem->thumbW - 1 + myItem->x1;
			}
		} else {
			*currItem = nullptr;
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		if (!*currItem) {
			return true;
		}
		movingFlag = false;
		if (menuItem::cursorInsideItem(myItem, x, y) && (x - myItem->x1 >= myItem->thumbX) &&
			(x - myItem->x1 <= myItem->thumbX + myItem->thumbW - 1)) {
			myItem->itemFlags = H_THUMB_OVER;
			*currItem = myItem;
		} else {
			myItem->itemFlags = H_THUMB_NORM;
			*currItem = nullptr;
		}
		redrawItem = true;
		execCallback = true;
		break;

	case _ME_move:
		if (menuItem::cursorInsideItem(myItem, x, y) && (x - myItem->x1 >= myItem->thumbX) &&
			(x - myItem->x1 <= myItem->thumbX + myItem->thumbW - 1)) {
			if (myItem->itemFlags != H_THUMB_OVER) {
				myItem->itemFlags = H_THUMB_OVER;
				*currItem = myItem;
				redrawItem = true;
			}
		} else {
			if (myItem->itemFlags != H_THUMB_NORM) {
				myItem->itemFlags = H_THUMB_NORM;
				*currItem = nullptr;
				redrawItem = true;
				handled = false;
			}
		}
		break;

	case _ME_L_hold:
	case _ME_doubleclick_hold:
		break;
	}

	// See if we need to redraw the hslider
	if (redrawItem) {
		(myItem->redraw)(myItem, myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		(myItem->callback)((void *)myItem, myItem->myMenu);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}


menuItemHSlider *menu_HSliderAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h,
	int32 initPercent, CALLBACK callback, bool transparent) {
	menuItemHSlider *newItem;
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	newItem = new menuItemHSlider();

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = myMenu;
	newItem->tag = tag;
	newItem->x1 = x;
	newItem->y1 = y;
	newItem->x2 = x + w - 1;
	newItem->y2 = y + h - 1;
	newItem->callback = callback;

	if (!transparent) {
		newItem->transparent = false;
		newItem->background = nullptr;
	} else {
		newItem->transparent = true;
		newItem->background = guiMenu::copyBackground(myMenu, x, y, w, h);
	}

	// Intialize the new slider
	newItem->itemFlags = H_THUMB_NORM;
	newItem->thumbW = _GM(menuSprites)[OM_SLIDER_BTN_NORM]->w;
	newItem->thumbH = _GM(menuSprites)[OM_SLIDER_BTN_NORM]->h;
	newItem->maxThumbX = w - _GM(menuSprites)[OM_SLIDER_BTN_NORM]->w;

	if (initPercent < 0) {
		initPercent = 0;
	} else if (initPercent > 100) {
		initPercent = 100;
	}

	// Calculate the initial thumbX
	newItem->percent = initPercent;
	newItem->thumbX = initPercent * newItem->maxThumbX / 100;

	newItem->redraw = (DrawFunction)menu_DrawHSlider;
	newItem->destroy = (DestroyFunction)menuItem::destroyItem;
	newItem->itemEventHandler = (ItemHandlerFunction)hslider_Handler;

	// Draw the slider in now
	(newItem->redraw)(newItem, myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	myScreen = vmng_screen_find(myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}


//-------------------------------    VSLIDER MENU ITEM    ---------------------------------//

enum {
	VS_NORM = 0x0000,
	VS_OVER = 0x0001,
	VS_PRESS = 0x0002,
	VS_GREY = 0x0003,
	VS_STATUS = 0x000f,
	VS_UP = 0x0010,
	VS_PAGE_UP = 0x0020,
	VS_THUMB = 0x0030,
	VS_PAGE_DOWN = 0x0040,
	VS_DOWN = 0x0050,
	VS_COMPONENT = 0x00f0
};


void menu_DrawVSlider(menuItemVSlider *myItem, guiMenu *myMenu, int32 x, int32 y, int32, int32) {
	Buffer *myBuff = nullptr;
	Buffer *backgroundBuff = nullptr;
	Sprite *upSprite = nullptr;
	Sprite *thumbSprite = nullptr;
	Sprite *downSprite = nullptr;
	Sprite *vbarSprite = nullptr;

	// Verify params
	if (!myItem || !myMenu)
		return;

	// If the item is marked transparent, get the background buffer
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}
	}

	// Get the menu buffer
	myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Set the different sprite components
	vbarSprite = _GM(menuSprites)[Burger::GUI::SL_SCROLL_BAR];
	upSprite = _GM(menuSprites)[Burger::GUI::SL_UP_BTN_NORM];
	thumbSprite = _GM(menuSprites)[Burger::GUI::SL_SLIDER_BTN_NORM];
	downSprite = _GM(menuSprites)[Burger::GUI::SL_DOWN_BTN_NORM];

	if ((myItem->itemFlags & VS_STATUS) == VS_GREY) {
		upSprite = _GM(menuSprites)[Burger::GUI::SL_UP_BTN_GREY];
		thumbSprite = nullptr;
		downSprite = _GM(menuSprites)[Burger::GUI::SL_DOWN_BTN_GREY];
	} else if ((myItem->itemFlags & VS_STATUS) == VS_OVER) {
		if ((myItem->itemFlags & VS_COMPONENT) == VS_UP) {
			upSprite = _GM(menuSprites)[Burger::GUI::SL_UP_BTN_OVER];
		} else if ((myItem->itemFlags & VS_COMPONENT) == VS_THUMB) {
			thumbSprite = _GM(menuSprites)[Burger::GUI::SL_SLIDER_BTN_OVER];
		} else if ((myItem->itemFlags & VS_COMPONENT) == VS_DOWN) {
			downSprite = _GM(menuSprites)[Burger::GUI::SL_DOWN_BTN_OVER];
		}
	} else if ((myItem->itemFlags & VS_STATUS) == VS_PRESS) {
		if ((myItem->itemFlags & VS_COMPONENT) == VS_UP) {
			upSprite = _GM(menuSprites)[Burger::GUI::SL_UP_BTN_PRESS];
		} else if ((myItem->itemFlags & VS_COMPONENT) == VS_THUMB) {
			thumbSprite = _GM(menuSprites)[Burger::GUI::SL_SLIDER_BTN_PRESS];
		} else if ((myItem->itemFlags & VS_COMPONENT) == VS_DOWN) {
			downSprite = _GM(menuSprites)[Burger::GUI::SL_DOWN_BTN_PRESS];
		}
	}

	// Draw the sprite comonents
	gui_DrawSprite(vbarSprite, myBuff, x, y + upSprite->h);
	gui_DrawSprite(upSprite, myBuff, x, y);
	gui_DrawSprite(thumbSprite, myBuff, x, y + myItem->thumbY);
	gui_DrawSprite(downSprite, myBuff, x, y + upSprite->h + vbarSprite->h);

	// Release the menu buffer
	myMenu->menuBuffer->release();
}

int32 vslider_WhereIsCursor(menuItemVSlider *myVSlider, int32 y) {
	if (y < myVSlider->minThumbY) {
		return VS_UP;
	} else if (y < myVSlider->thumbY) {
		return VS_PAGE_UP;
	} else if (y < myVSlider->thumbY + myVSlider->thumbH) {
		return VS_THUMB;
	} else if (y < myVSlider->maxThumbY + myVSlider->thumbH) {
		return VS_PAGE_DOWN;
	} else {
		return VS_DOWN;
	}
}

bool vslider_Handler(menuItemVSlider *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	bool redrawItem, execCallback, handled;
	int32 tempFlags;
	ScreenContext *myScreen;
	int32 status;
	int32 deltaSlide;
	int32 currTime;
	static bool movingFlag;
	static int32 movingY;
	static int32 callbackTime;

	// Verify params
	if (!myItem)
		return false;

	if (!(eventType == EVENT_MOUSE))
		return false;

	if ((myItem->itemFlags & VS_STATUS) == VS_GREY) {
		*currItem = nullptr;
		return false;
	}

	currTime = timer_read_60();
	redrawItem = false;
	handled = true;
	execCallback = false;

	switch (event) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			//				  digi_play(inv_click_snd, 2, 255, -1, inv_click_snd_room_lock);
			*currItem = myItem;
			tempFlags = vslider_WhereIsCursor(myItem, y - myItem->y1);
			if (tempFlags == VS_THUMB) {
				movingFlag = true;
				movingY = y;
			}
			if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
				myItem->itemFlags = tempFlags + VS_NORM;
			} else {
				myItem->itemFlags = tempFlags + VS_PRESS;
				redrawItem = true;
			}
			execCallback = true;
		} else {
			*currItem = nullptr;
			myItem->itemFlags = VS_NORM;
			redrawItem = true;
		}
		break;

	case _ME_L_drag:
	case _ME_doubleclick_drag:
		if (!*currItem) {
			return true;
		}
		if (movingFlag) {
			if (y < movingY) {
				deltaSlide = imath_min(myItem->thumbY - myItem->minThumbY, movingY - y);
				if (deltaSlide > 0) {
					myItem->thumbY -= deltaSlide;
					myItem->percent = ((myItem->thumbY - myItem->minThumbY) * 100) /
						(myItem->maxThumbY - myItem->minThumbY);
					redrawItem = true;
					execCallback = true;
				}
			} else if (y > movingY) {
				deltaSlide = imath_min(myItem->maxThumbY - myItem->thumbY, y - movingY);
				if (deltaSlide > 0) {
					myItem->thumbY += deltaSlide;
					myItem->percent = ((myItem->thumbY - myItem->minThumbY) * 100) /
						(myItem->maxThumbY - myItem->minThumbY);
					redrawItem = true;
					execCallback = true;
				}
			}
			movingY = y;
			if (movingY < (myItem->thumbY + myItem->y1)) {
				movingY = myItem->thumbY + myItem->y1;
			} else if (movingY > (myItem->thumbY + myItem->thumbH - 1 + myItem->y1)) {
				movingY = myItem->thumbY + myItem->thumbH - 1 + myItem->y1;
			}
		} else {
			if (menuItem::cursorInsideItem(myItem, x, y)) {
				tempFlags = vslider_WhereIsCursor(myItem, y - myItem->y1);
				if ((myItem->itemFlags & VS_COMPONENT) == tempFlags) {
					if ((tempFlags != VS_PAGE_UP) && (tempFlags != VS_PAGE_DOWN) &&
						((myItem->itemFlags & VS_STATUS) != VS_PRESS)) {
						myItem->itemFlags = tempFlags + VS_PRESS;
						redrawItem = true;
					}
					if (currTime - callbackTime > 6) {
						execCallback = true;
					}
				} else {
					if ((myItem->itemFlags & VS_STATUS) != VS_OVER) {
						myItem->itemFlags = (myItem->itemFlags & VS_COMPONENT) + VS_OVER;
						redrawItem = true;
					}
				}
				execCallback = true;
			} else {
				if ((myItem->itemFlags & VS_STATUS) != VS_OVER) {
					myItem->itemFlags = (myItem->itemFlags & VS_COMPONENT) + VS_OVER;
					redrawItem = true;
				}
			}
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		movingFlag = false;
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			tempFlags = vslider_WhereIsCursor(myItem, y - myItem->y1);
			if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
				myItem->itemFlags = VS_NORM;
			} else {
				myItem->itemFlags = tempFlags + VS_OVER;
				*currItem = myItem;
			}
		} else {
			myItem->itemFlags = VS_NORM;
			*currItem = nullptr;
		}
		redrawItem = true;
		if (!_GM(currMenuIsSave)) {
			UpdateThumbNails(_GM(firstSlotIndex), (guiMenu *)myItem->myMenu);
		}
		break;

	case _ME_move:
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			*currItem = myItem;
			tempFlags = vslider_WhereIsCursor(myItem, y - myItem->y1);
			if ((myItem->itemFlags & VS_COMPONENT) != tempFlags) {
				if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
					myItem->itemFlags = VS_NORM;
				} else {
					myItem->itemFlags = tempFlags + VS_OVER;
				}
				redrawItem = true;
			}
		} else {
			*currItem = nullptr;
			if (myItem->itemFlags != VS_NORM) {
				myItem->itemFlags = VS_NORM;
				redrawItem = true;
				handled = false;
			}
		}
		break;

	case _ME_L_hold:
	case _ME_doubleclick_hold:
		if (!*currItem) {
			return true;
		}
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			tempFlags = vslider_WhereIsCursor(myItem, y - myItem->y1);
			if ((myItem->itemFlags & VS_COMPONENT) == tempFlags) {
				if (currTime - callbackTime > 6) {
					execCallback = true;
				}
			}
		}
		break;
	}

	// See if we need to redraw the vslider
	if (redrawItem) {
		(myItem->redraw)(myItem, myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		callbackTime = currTime;
		(myItem->callback)((void *)myItem, myItem->myMenu);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}


menuItemVSlider *menu_VSliderAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h,
		int32 initPercent, CALLBACK callback, bool transparent) {
	menuItemVSlider *newItem;
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu)
		return nullptr;

	// Allocate a new one
	newItem = new menuItemVSlider();

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = myMenu;
	newItem->tag = tag;
	newItem->x1 = x;
	newItem->y1 = y;
	newItem->x2 = x + w - 1;
	newItem->y2 = y + h - 1;
	newItem->callback = callback;

	if (!transparent) {
		newItem->transparent = false;
		newItem->background = nullptr;
	} else {
		newItem->transparent = true;
		newItem->background = guiMenu::copyBackground(myMenu, x, y, w, h);
	}

	newItem->itemFlags = VS_NORM;

	newItem->thumbW = _GM(menuSprites)[Burger::GUI::SL_SLIDER_BTN_NORM]->w;
	newItem->thumbH = _GM(menuSprites)[Burger::GUI::SL_SLIDER_BTN_NORM]->h;

	newItem->minThumbY = _GM(menuSprites)[Burger::GUI::SL_UP_BTN_NORM]->h + 1;
	newItem->maxThumbY = _GM(menuSprites)[Burger::GUI::SL_UP_BTN_NORM]->h + _GM(menuSprites)[Burger::GUI::SL_SCROLL_BAR]->h
		- _GM(menuSprites)[Burger::GUI::SL_SLIDER_BTN_NORM]->h - 1;

	// Calculate the initial thumbY
	newItem->percent = imath_max(imath_min(initPercent, 100), 0);
	newItem->thumbY = newItem->minThumbY +
		((newItem->percent * (newItem->maxThumbY - newItem->minThumbY)) / 100);

	newItem->redraw = (DrawFunction)menu_DrawVSlider;
	newItem->destroy = (DestroyFunction)menuItem::destroyItem;
	newItem->itemEventHandler = (ItemHandlerFunction)vslider_Handler;

	// Draw the vslider in now
	(newItem->redraw)(newItem, myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	myScreen = vmng_screen_find(myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}


void menu_DisableVSlider(menuItemVSlider *myItem, int32 tag, guiMenu *myMenu) {
	// Verify params
	if (!myMenu)
		return;

	if (!myItem)
		myItem = (menuItemVSlider *)guiMenu::getItem(tag, myMenu);
	if (!myItem)
		return;

	myItem->itemFlags = VS_GREY;
}


void menu_EnableVSlider(menuItemVSlider *myItem, int32 tag, guiMenu *myMenu) {
	// Verify params
	if (!myMenu)
		return;

	if (!myItem)
		myItem = (menuItemVSlider *)guiMenu::getItem(tag, myMenu);
	if (!myItem)
		return;

	myItem->itemFlags = VS_NORM;
}


//-----------------------------    TEXTFIELD MENU ITEM    ---------------------------------//

enum {
	TF_NORM = 0,
	TF_OVER = 1,
	TF_GREY = 2
};

void menu_DrawTextField(menuItemTextField *myItem, guiMenu *myMenu, int32 x, int32 y, int32, int32) {
	menuItemTextField *myText = nullptr;
	Buffer *myBuff = nullptr;
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite = nullptr;
	char tempStr[64], tempChar;
	int32 cursorX;

	// Verify params
	if (!myItem || !myMenu)
		return;

	// If the item is marked transparent, get the background buffer
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}
	}

	// Select the sprite
	switch (myText->itemFlags) {
	case TF_GREY:
		mySprite = _GM(menuSprites)[Burger::GUI::SL_LINE_NORM];
		break;

	case TF_OVER:
		mySprite = _GM(menuSprites)[Burger::GUI::SL_LINE_OVER];
		break;

	case TF_NORM:
	default:
		mySprite = _GM(menuSprites)[Burger::GUI::SL_LINE_OVER];
		break;
	}

	// Get the menu buffer and draw the sprite to it
	myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Draw the item sprite in
	gui_DrawSprite(mySprite, myBuff, x, y);

	//write in the special tag
	gr_font_set_color(menuItem::TEXT_COLOR_NORM_FOREGROUND);
	Common::sprintf_s(tempStr, 64, "%02d", myText->specialTag);
	gr_font_set(_GM(menuFont));
	gr_font_write(myBuff, tempStr, x + 4, y + 1, 0, -1);

	//write in the text
	gr_font_write(myBuff, &myText->prompt[0], x + 26, y + 1, 0, -1);

	if (myText->itemFlags == TF_OVER) {
		// Draw in the cursor
		if (myText->cursor) {
			tempChar = *myText->cursor;
			*myText->cursor = '\0';
			cursorX = gr_font_string_width(&myText->prompt[0], -1);
			*myText->cursor = tempChar;

			gr_color_set(menuItem::TEXT_COLOR_OVER_FOREGROUND);
			gr_vline(myBuff, x + cursorX + 26, y + 1, y + 12);
		}
	}

	// Release the menu buffer
	myMenu->menuBuffer->release();
}


bool textfield_Handler(menuItemTextField *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	bool redrawItem, execCallback, handled;
	ScreenContext *myScreen;
	int32 status, temp;
	char tempStr[80], *tempPtr;

	// Verify params
	if (!myItem)
		return false;

	if (myItem->itemFlags == TF_GREY) {
		return false;
	}

	redrawItem = false;
	execCallback = false;
	handled = true;

	if (eventType == EVENT_MOUSE) {
		switch (event) {
		case _ME_L_click:
		case _ME_doubleclick:
			_GM(deleteSaveDesc) = false;
			if (menuItem::cursorInsideItem(myItem, x, y)) {
				*currItem = myItem;
			}
			break;

		case _ME_L_drag:
		case _ME_doubleclick_drag:
			break;

		case _ME_L_release:
		case _ME_doubleclick_release:
			if (!*currItem) {
				return true;
			}
			*currItem = nullptr;
			if (menuItem::cursorInsideItem(myItem, x, y)) {
				if (myItem->itemFlags == TF_OVER) {
					temp = strlen(myItem->prompt);
					if (temp > 0) {
						Common::strcpy_s(tempStr, myItem->prompt);
						tempPtr = &tempStr[temp];
						gr_font_set(_GM(menuFont));
						temp = gr_font_string_width(tempStr, -1);
						while ((tempPtr != &tempStr[0]) && (temp > x - myItem->x1 - 26)) {
							*--tempPtr = '\0';
							temp = gr_font_string_width(tempStr, -1);
						}
						myItem->cursor = &myItem->prompt[tempPtr - &tempStr[0]];
						redrawItem = true;
					}
				} else if (event == _ME_doubleclick_release) {
					execCallback = true;
				}
			}
			break;

		case _ME_move:
		case _ME_L_hold:
		case _ME_doubleclick_hold:
			break;
		}
	} else if ((eventType == EVENT_KEY) && (myItem->itemFlags == TF_OVER)) {
		switch (event) {
		case KEY_RETURN:
			_GM(deleteSaveDesc) = false;
			execCallback = true;
			break;

		case KEY_HOME:
			_GM(deleteSaveDesc) = false;
			myItem->cursor = &myItem->prompt[0];
			redrawItem = true;
			break;

		case KEY_END:
			_GM(deleteSaveDesc) = false;
			myItem->cursor = myItem->promptEnd;
			redrawItem = true;
			break;

		case KEY_LEFT:
			_GM(deleteSaveDesc) = false;
			if (myItem->cursor > &myItem->prompt[0]) {
				myItem->cursor--;
				redrawItem = true;
			}
			break;

		case KEY_RIGHT:
			_GM(deleteSaveDesc) = false;
			if (myItem->cursor < myItem->promptEnd) {
				myItem->cursor++;
				redrawItem = true;
			}
			break;

		case KEY_DELETE:
			if (_GM(deleteSaveDesc)) {
				myItem->prompt[0] = '\0';
				myItem->promptEnd = &myItem->prompt[0];
				myItem->cursor = myItem->promptEnd;
				redrawItem = true;
			} else if (myItem->cursor < myItem->promptEnd) {
				Common::strcpy_s(tempStr, (char *)(myItem->cursor + 1));
				Common::strcpy_s(myItem->cursor, 80, tempStr);
				myItem->promptEnd--;
				redrawItem = true;
			}
			break;

		case KEY_BACKSP:
			_GM(deleteSaveDesc) = false;
			if (myItem->cursor > &myItem->prompt[0]) {
				Common::strcpy_s(tempStr, myItem->cursor);
				myItem->promptEnd--;
				myItem->cursor--;
				Common::strcpy_s(myItem->cursor, 80, tempStr);
				redrawItem = true;
			}
			break;

		default:
			_GM(deleteSaveDesc) = false;
			gr_font_set(_GM(menuFont));
			temp = gr_font_string_width(&myItem->prompt[0], -1);
			if ((strlen(&myItem->prompt[0]) < 79) && (temp < myItem->pixWidth - 12) && (event >= 32) && (event <= 127)) {
				if (myItem->cursor < myItem->promptEnd) {
					Common::strcpy_s(tempStr, (char *)myItem->cursor);
					Common::sprintf_s(myItem->cursor, 80, "%c%s", (char)event, tempStr);
				} else {
					*myItem->cursor = (char)event;
					*(myItem->cursor + 1) = '\0';
				}
				myItem->cursor++;
				myItem->promptEnd++;

				redrawItem = true;
			}
			break;
		}
	} else if ((eventType == EVENT_KEY) && (event == KEY_RETURN)) {
		// The only events a NORM textfield can respond to are doubleclick_release and <return> keypress
		execCallback = true;
	} else {
		// Otherwise the event will not be handled
		return false;
	}

	// See if we need to redraw the button
	if (redrawItem) {
		(myItem->redraw)(myItem, myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		(myItem->callback)((void *)myItem, myItem->myMenu);
		myScreen = vmng_screen_find(myItem->myMenu, &status);

		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}


menuItemTextField *menu_TextFieldAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, int32 initFlags,
		const char *prompt, int32 specialTag, CALLBACK callback, bool transparent) {
	menuItemTextField *newItem;
	menuItemTextField *textInfo;
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu)
		return nullptr;

	// Allocate a new one
	newItem = new menuItemTextField();

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = myMenu;
	newItem->tag = tag;
	newItem->x1 = x;
	newItem->y1 = y;
	newItem->x2 = x + w - 1;
	newItem->y2 = y + h - 1;
	newItem->callback = callback;

	if (!transparent) {
		newItem->transparent = false;
		newItem->background = nullptr;
	} else {
		newItem->transparent = true;
		newItem->background = guiMenu::copyBackground(myMenu, x, y, w, h);
	}

	if ((textInfo = (menuItemTextField *)mem_alloc(sizeof(menuItemTextField), "menu item textfield")) == nullptr) {
		return nullptr;
	}
	textInfo->itemFlags = initFlags;

	textInfo->specialTag = specialTag;
	textInfo->pixWidth = w - 27;
	if (prompt) {
		Common::strcpy_s(&textInfo->prompt[0], 80, prompt);
		textInfo->promptEnd = &textInfo->prompt[strlen(prompt)];
	} else {
		textInfo->prompt[0] = '\0';
		textInfo->promptEnd = &textInfo->prompt[0];
	}
	textInfo->cursor = textInfo->promptEnd;

	newItem->redraw = (DrawFunction)menu_DrawTextField;
	newItem->destroy = (DestroyFunction)menuItem::destroyItem;
	newItem->itemEventHandler = (ItemHandlerFunction)textfield_Handler;

	// Draw the vslider in now
	(newItem->redraw)(newItem, myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	myScreen = vmng_screen_find(myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}


//-------------------------------------   GAME MENU   -------------------------------------//


#define GAME_MENU_X		190
#define GAME_MENU_Y		100
#define GAME_MENU_W		260
#define GAME_MENU_H		198

#define GM_MAIN_X			 45
#define GM_MAIN_Y			 53
#define GM_MAIN_W			 24
#define GM_MAIN_H			 24

#define GM_OPTIONS_X		 45
#define GM_OPTIONS_Y		 94
#define GM_OPTIONS_W		 24
#define GM_OPTIONS_H		 24

#define GM_RESUME_X		 45
#define GM_RESUME_Y		135
#define GM_RESUME_W		 24
#define GM_RESUME_H		 24

#define GM_QUIT_X			141
#define GM_QUIT_Y			135
#define GM_QUIT_W			 24
#define GM_QUIT_H		    24

#define GM_SAVE_X			141
#define GM_SAVE_Y			 53
#define GM_SAVE_W			 24
#define GM_SAVE_H		    24

#define GM_LOAD_X			141
#define GM_LOAD_Y			 94
#define GM_LOAD_W			 24
#define GM_LOAD_H			 24

static void DestroyGameMenu();

void cb_Game_Quit(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();

	// Shutdown the menu system
	guiMenu::shutdown(false);

	// Set the global that will cause the entire game to exit to dos
	_G(kernel).going = false;
}

void cb_Game_Resume(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();

	// Shutdown the menu system
	guiMenu::shutdown(true);
}

void cb_Game_Save(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();
	guiMenu::shutdown(true);
	buttonClosesDialog = true;

	// Create the save game menu
	g_engine->showSaveScreen();
}

void cb_Game_Load(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();
	guiMenu::shutdown(true);
	buttonClosesDialog = true;

	// Create the save game menu
	g_engine->showLoadScreen(M4Engine::kLoadFromGameDialog);
}

void cb_Game_Main(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();

	if (!_GM(gameMenuFromMain)) {
		// Save the game so we can resume from here if possible
		if (_GM(interfaceWasVisible) && player_commands_allowed()) {
			other_save_game_for_resurrection();
		}

		// Make sure the interface does not reappear
		_GM(interfaceWasVisible) = false;

		// Shutdown the menu system
		guiMenu::shutdown(false);
	} else {
		guiMenu::shutdown(true);
	}

	// Go to the main menu
	_G(game).setRoom(_G(executing) == WHOLE_GAME ? 903 : 901);
}

void cb_Game_Options(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();
	buttonClosesDialog = true;

	// Create the options menu
	CreateOptionsMenu(nullptr);
}

void DestroyGameMenu(void) {
	if (!_GM(gameMenu)) {
		return;
	}

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(gameMenu));

	// Destroy the menu resources
	guiMenu::destroy(_GM(gameMenu));

	// Unload the menu sprites
	guiMenu::unloadSprites();
}

void CreateGameMenuMain(RGB8 *myPalette) {
	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("gamemenu", GM_TOTAL_SPRITES)) {
		return;
	}

	_GM(gameMenu) = guiMenu::create(_GM(menuSprites)[GM_DIALOG_BOX], GAME_MENU_X, GAME_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(gameMenu)) {
		return;
	}

	menuItemButton::buttonAdd(_GM(gameMenu), GM_TAG_MAIN, GM_MAIN_X, GM_MAIN_Y, GM_MAIN_W, GM_MAIN_H, cb_Game_Main);
	menuItemButton::buttonAdd(_GM(gameMenu), GM_TAG_OPTIONS, GM_OPTIONS_X, GM_OPTIONS_Y, GM_OPTIONS_W, GM_OPTIONS_H, cb_Game_Options);
	menuItemButton::buttonAdd(_GM(gameMenu), GM_TAG_RESUME, GM_RESUME_X, GM_RESUME_Y, GM_RESUME_W, GM_RESUME_H, cb_Game_Resume);
	menuItemButton::buttonAdd(_GM(gameMenu), GM_TAG_QUIT, GM_QUIT_X, GM_QUIT_Y, GM_QUIT_W, GM_QUIT_H, cb_Game_Quit);

	if (!_GM(gameMenuFromMain)) {
		menuItemButton::buttonAdd(_GM(gameMenu), GM_TAG_SAVE, GM_SAVE_X, GM_SAVE_Y, GM_SAVE_W, GM_SAVE_H, cb_Game_Save);
	} else {
		menuItemButton::buttonAdd(_GM(gameMenu), GM_TAG_SAVE, GM_SAVE_X, GM_SAVE_Y, GM_SAVE_W, GM_SAVE_H, cb_Game_Save, menuItemButton::BTN_TYPE_GM_GENERIC, true);
	}

	// See if there are any games to load
	if (g_engine->savesExist()) {
		menuItemButton::buttonAdd(_GM(gameMenu), GM_TAG_LOAD, GM_LOAD_X, GM_LOAD_Y, GM_LOAD_W, GM_LOAD_H, cb_Game_Load);
	} else {
		menuItemButton::buttonAdd(_GM(gameMenu), GM_TAG_LOAD, GM_LOAD_X, GM_LOAD_Y, GM_LOAD_W, GM_LOAD_H, cb_Game_Load, menuItemButton::BTN_TYPE_GM_GENERIC, true);
	}

	// Configure the game so pressing <esc> will cause the menu to disappear and the game to resume
	guiMenu::configure(_GM(gameMenu), cb_Game_Resume, cb_Game_Resume);

	vmng_screen_show((void *)_GM(gameMenu));
	LockMouseSprite(0);
}


//----------------------------------   OPTIONS MENU   -------------------------------------//

void DestroyOptionsMenu();


void cb_Options_Game_Cancel(void *, void *) {
	// Reset values of items to what they were when options menu came up
	digi_set_overall_volume(_GM(remember_digi_volume));
	_G(flags)[digestability] = _GM(remember_digestability);

	// Destroy the options menu
	DestroyOptionsMenu();
	buttonClosesDialog = true;

	// Create the options menu
	CreateGameMenuMain(nullptr);
}

void cb_Options_Game_Done(void *, void *) {
	// Destroy the options menu
	DestroyOptionsMenu();
	buttonClosesDialog = true;

	// Create the options menu
	CreateGameMenuMain(nullptr);
}

void cb_Options_Digi(menuItemHSlider *myItem, guiMenu *myMenu) {
	// Set the digi volume
	digi_set_overall_volume(myItem->percent);
	term_message("digi volume: %d", myItem->percent);

	// This scroller control has been moved, so make sure that the DONE button is not greyed out
	menuItemButton::enableButton(nullptr, OM_TAG_DONE, myMenu);
	guiMenu::itemRefresh(nullptr, OM_TAG_DONE, myMenu);

}

void cb_Options_Digestability(menuItemHSlider *myItem, guiMenu *myMenu) {
	term_message("digestability: %d", myItem->percent);
	_G(flags)[digestability] = myItem->percent;

	// This scroller control has been moved, so make sure that the DONE button is not greyed out
	menuItemButton::enableButton(nullptr, OM_TAG_DONE, myMenu);
	guiMenu::itemRefresh(nullptr, OM_TAG_DONE, myMenu);
}

void DestroyOptionsMenu(void) {
	if (!_GM(opMenu))
		return;

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(opMenu));

	// Destroy the menu resources
	guiMenu::destroy(_GM(opMenu));

	// Unload the menu sprites
	guiMenu::unloadSprites();
}


void CreateOptionsMenu(RGB8 *myPalette) {
	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("opmenu", OM_TOTAL_SPRITES)) {
		return;
	}

	_GM(opMenu) = guiMenu::create(_GM(menuSprites)[OM_DIALOG_BOX], OPTIONS_MENU_X, OPTIONS_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(opMenu)) {
		return;
	}

	menuItemButton::buttonAdd(_GM(opMenu), OM_TAG_CANCEL, OM_CANCEL_X, OM_CANCEL_Y, OM_CANCEL_W, OM_CANCEL_H, cb_Options_Game_Cancel, menuItemButton::BTN_TYPE_OM_CANCEL);
	menuItemButton::buttonAdd(_GM(opMenu), OM_TAG_DONE, OM_DONE_X, OM_DONE_Y, OM_DONE_W, OM_DONE_H, cb_Options_Game_Done, menuItemButton::BTN_TYPE_OM_DONE, true);
	menu_HSliderAdd(_GM(opMenu), OM_TAG_DIGI, OM_DIGI_X, OM_DIGI_Y, OM_DIGI_W, OM_DIGI_H, digi_get_overall_volume(),
		(CALLBACK)cb_Options_Digi, true);
	menu_HSliderAdd(_GM(opMenu), OM_TAG_DIGESTABILITY, OM_DIGESTABILITY_X, OM_DIGESTABILITY_Y,
		OM_DIGESTABILITY_W, OM_DIGESTABILITY_H, _G(flags)[digestability],
		(CALLBACK)cb_Options_Digestability, true);

	// Remember the values of the items in case the user cancels
	_GM(remember_digi_volume) = digi_get_overall_volume();
	_GM(remember_digestability) = _G(flags)[digestability];

	// Configure the game so pressing <esc> will cause the menu to disappear and the gamemenu to reappear
	guiMenu::configure(_GM(opMenu), cb_Options_Game_Done, cb_Options_Game_Cancel);

	vmng_screen_show((void *)_GM(opMenu));
	LockMouseSprite(0);
}

//------------------------------------------------------------------------------------//
//--------------------------------   ERR MENU   --------------------------------------//
//------------------------------------------------------------------------------------//

void DestroyErrMenu();

void cb_Err_Done(void *, void *) {
	// Destroy the game menu
	DestroyErrMenu();

	// Shutdown the menu system
	guiMenu::shutdown(true);
}


void DestroyErrMenu(void) {
	if (!_GM(errMenu)) {
		return;
	}

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(errMenu));

	// Destroy the menu resources
	guiMenu::destroy(_GM(errMenu));

	// Unload the menu sprites
	guiMenu::unloadSprites();
}


void CreateErrMenu(RGB8 *myPalette) {
	Buffer *myBuff;

	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("errmenu", 5)) {
		return;
	}

	_GM(errMenu) = guiMenu::create(_GM(menuSprites)[EM_DIALOG_BOX], ERROR_MENU_X, ERROR_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(errMenu)) {
		return;
	}

	// Get the menu buffer
	myBuff = _GM(errMenu)->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	//write the err message
	gr_font_set_color(menuItem::TEXT_COLOR_NORM_FOREGROUND);
	gr_font_write(myBuff, "Save game failed!", 48, 8, 0, -1);

	gr_font_write(myBuff, "A disk error has", 48, 23, 0, -1);
	gr_font_write(myBuff, "occurred.", 48, 33, 0, -1);

	gr_font_write(myBuff, "Please ensure you", 48, 48, 0, -1);
	gr_font_write(myBuff, "have write access", 48, 58, 0, -1);
	gr_font_write(myBuff, "and sufficient", 48, 68, 0, -1);
	gr_font_write(myBuff, "disk space (40k).", 48, 78, 0, -1);

	_GM(errMenu)->menuBuffer->release();

	// Add the done button
	menuItemButton::buttonAdd(_GM(errMenu), EM_TAG_RETURN, EM_RETURN_X, EM_RETURN_Y, EM_RETURN_W, EM_RETURN_H, cb_Err_Done);

	// Configure the game so pressing <esc> will cause the menu to disappear and the gamemenu to reappear
	guiMenu::configure(_GM(errMenu), cb_Err_Done, cb_Err_Done);

	vmng_screen_show((void *)_GM(errMenu));
	LockMouseSprite(0);
}


//--------------------------------   SAVE / LOAD MENU   -----------------------------------//

void DestroySaveLoadMenu(bool saveMenu);
void cb_SaveLoad_Slot(menuItemButton *theItem, guiMenu *myMenu);
bool load_Handler(menuItemButton *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem);


bool LoadThumbNail(int32 slotNum) {
	Sprite *&thumbNailSprite = _GM(thumbNails)[slotNum];
	return g_engine->loadSaveThumbnail(slotNum + 1, thumbNailSprite);
}


void UnloadThumbNail(int32 slotNum) {
	if (_GM(thumbNails)[slotNum]->sourceHandle) {
		HUnLock(_GM(thumbNails)[slotNum]->sourceHandle);
		DisposeHandle(_GM(thumbNails)[slotNum]->sourceHandle);
		_GM(thumbNails)[slotNum]->sourceHandle = nullptr;
	}
}


void UpdateThumbNails(int32 firstSlot, guiMenu *myMenu) {
	int32 i, startIndex, endIndex;

	// Make sure there is something to update
	if (firstSlot == _GM(thumbIndex)) {
		return;
	}

	// Ensure firstSlot is in a valid range
	firstSlot = imath_max(imath_min(firstSlot, 89), 0);

	if (firstSlot > _GM(thumbIndex)) {
		// Dump Out all thumbnails in slots which don't overlap
		startIndex = _GM(thumbIndex);
		endIndex = imath_min(_GM(thumbIndex) + 9, firstSlot - 1);
		for (i = startIndex; i <= endIndex; i++) {
			UnloadThumbNail(i);
		}

		// Load in all thumbnails missing thumbnails
		startIndex = imath_max(_GM(thumbIndex) + 10, firstSlot);
		endIndex = imath_min(firstSlot + 9, 98);
		for (i = startIndex; i <= endIndex; i++) {
			if (_GM(slotInUse)[i]) {
				if (!LoadThumbNail(i)) {
					_GM(slotInUse)[i] = false;
					menuItemButton::disableButton(nullptr, 1001 + i - firstSlot, myMenu);
					guiMenu::itemRefresh(nullptr, 1001 + i - firstSlot, myMenu);
				}
			}
		}
	} else {
		// Else firstSlot < _GM(thumbIndex)
		// Dump Out all thumbnails in slots which don't overlap
		startIndex = imath_max(firstSlot + 10, _GM(thumbIndex));
		endIndex = imath_min(_GM(thumbIndex) + 9, 98);
		for (i = startIndex; i <= endIndex; i++) {
			UnloadThumbNail(i);
		}

		// Load in all thumbnails missing thumbnails
		startIndex = firstSlot;
		endIndex = imath_min(firstSlot + 9, _GM(thumbIndex) - 1);
		for (i = startIndex; i <= endIndex; i++) {
			if (_GM(slotInUse)[i]) {
				if (!LoadThumbNail(i)) {
					_GM(slotInUse)[i] = false;
					menuItemButton::disableButton(nullptr, 1001 + i - firstSlot, myMenu);
					guiMenu::itemRefresh(nullptr, 1001 + i - firstSlot, myMenu);
				}
			}
		}
	}

	// Set the var
	_GM(thumbIndex) = firstSlot;
}


void SetFirstSlot(int32 firstSlot, guiMenu *myMenu) {
	menuItemButton *myButton;
	int32 i;

	if (!myMenu) {
		return;
	}

	// Ensure firstSlot is in a valid range
	firstSlot = imath_max(imath_min(firstSlot, 89), 0);

	// Change the prompt and special tag of each of the slot buttons
	for (i = 0; i < MAX_SLOTS_SHOWN; i++) {
		myButton = (menuItemButton *)guiMenu::getItem(i + 1001, myMenu);

		myButton->prompt = _GM(slotTitles)[firstSlot + i];
		if (_GM(currMenuIsSave) || _GM(slotInUse)[firstSlot + i]) {
			myButton->itemFlags = menuItemButton::BTN_STATE_NORM;
		} else {
			myButton->itemFlags = menuItemButton::BTN_STATE_GREY;
		}

		myButton->specialTag = firstSlot + i + 1;
		guiMenu::itemRefresh(myButton, i + 1001, myMenu);
	}
}


void cb_SaveLoad_VSlider(menuItemVSlider *myItem, guiMenu *myMenu) {
	bool redraw;

	if (!myMenu || !myItem)
		return;

	if ((myItem->itemFlags & VS_COMPONENT) != VS_THUMB) {

		redraw = (DrawFunction)false;
		switch (myItem->itemFlags & VS_COMPONENT) {

		case VS_UP:
			if (_GM(firstSlotIndex) > 0) {
				_GM(firstSlotIndex)--;
				redraw = (DrawFunction)true;
			}
			break;

		case VS_PAGE_UP:
			if (_GM(firstSlotIndex) > 0) {
				_GM(firstSlotIndex) = imath_max(_GM(firstSlotIndex) - 10, 0);
				redraw = (DrawFunction)true;
			}
			break;

		case VS_PAGE_DOWN:
			if (_GM(firstSlotIndex) < 89) {
				_GM(firstSlotIndex) = imath_min(_GM(firstSlotIndex) + 10, 89);
				redraw = (DrawFunction)true;
			}
			break;

		case VS_DOWN:
			if (_GM(firstSlotIndex) < 89) {
				_GM(firstSlotIndex)++;
				redraw = (DrawFunction)true;
			}
			break;
		}

		// See if we were able to set a new first slot index
		if (redraw) {
			SetFirstSlot(_GM(firstSlotIndex), myMenu);

			// Calculate the new percent
			myItem->percent = (_GM(firstSlotIndex) * 100) / 89;

			// Calculate the new thumbY
			myItem->thumbY = myItem->minThumbY +
				((myItem->percent * (myItem->maxThumbY - myItem->minThumbY)) / 100);

			// Redraw the slider
			guiMenu::itemRefresh(myItem, -1, myMenu);
		}
	}

	// Else the callback came from the thumb - set the _GM(firstSlotIndex) based on the slider percent
	else {
		_GM(firstSlotIndex) = (myItem->percent * 89) / 100;
		SetFirstSlot(_GM(firstSlotIndex), myMenu);
	}
}


void cb_SaveLoad_Save(void *, guiMenu *myMenu) {
	menuItemTextField *myText;
	bool saveGameFailed;

	// If (slotSelected < 0) this callback is being executed by pressing return prematurely
	if (_GM(slotSelected) < 0) {
		return;
	}

	// First make the textfield NORM
	myText = (menuItemTextField *)guiMenu::getItem(2000, myMenu);
	if (myText)
		return;

	myText->itemFlags = TF_NORM;

	// Set the vars
	_GM(slotInUse)[_GM(slotSelected) - 1] = true;
	Common::strcpy_s(_GM(slotTitles)[_GM(slotSelected) - 1], 80, myText->prompt);

	// Save the game
	saveGameFailed = !g_engine->saveGameFromMenu(_GM(slotSelected),
		myText->prompt, _GM(_thumbnail));

	// If the save game failed, bring up the err menu
	if (saveGameFailed) {
		// Kill the save menu
		DestroySaveLoadMenu(true);

		// Create the err menu
		CreateErrMenu(nullptr);

		// Abort this procedure
		return;
	}

	// Kill the save menu
	DestroySaveLoadMenu(true);

	// Shutdown the menu system
	guiMenu::shutdown(true);
}


void cb_SaveLoad_Load(menuItemButton *, guiMenu *) {
	KernelTriggerType oldMode;

	// If (slotSelected < 0) this callback is being executed by pressing return prematurely
	if (_GM(slotSelected) < 0) {
		return;
	}

	// Kill the menu
	DestroySaveLoadMenu(false);

	// Shutdown the menu system
	guiMenu::shutdown(false);

	// See if we need to reset the ESC, F2, and F3 hotkeys
	if (_GM(gameMenuFromMain)) {
		AddSystemHotkey(KEY_ESCAPE, Burger::Hotkeys::escape_key_pressed);
		AddSystemHotkey(KEY_F2, M4::Hotkeys::saveGame);
		AddSystemHotkey(KEY_F3, M4::Hotkeys::loadGame);
	}

	// Start the restore process
	_G(kernel).restore_slot = _GM(slotSelected);
	oldMode = _G(kernel).trigger_mode;

	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_trigger_dispatch_now(TRIG_RESTORE_GAME);
	_G(kernel).trigger_mode = oldMode;
}


void cb_SaveLoad_Cancel(menuItemButton *, guiMenu *myMenu) {
	menuItem *myItem;
	int32 i, x, y, w, h;

	// If a slot has been selected, cancel will re-enable all slots
	if (_GM(slotSelected) >= 0) {
		// Enable the prev buttons
		for (i = 1001; i <= 1010; i++) {
			if (_GM(currMenuIsSave) || _GM(slotInUse)[i - 1001 + _GM(firstSlotIndex)]) {
				menuItemButton::enableButton(nullptr, i, myMenu);
				guiMenu::itemRefresh(nullptr, i, myMenu);
			}
		}

		// Find the textfield and use it's coords to place the button
		myItem = guiMenu::getItem(2000, myMenu);
		x = myItem->x1;
		y = myItem->y1;
		w = myItem->x2 - myItem->x1 + 1;
		h = myItem->y2 - myItem->y1 + 1;

		// Delete the textfield
		guiMenu::itemDelete(myItem, 2000, myMenu);

		// Add the button back in
		if (_GM(currMenuIsSave)) {
			menuItemButton::buttonAdd(myMenu, 1000 + _GM(slotSelected) - _GM(firstSlotIndex), x, y, w, h,
				(CALLBACK)cb_SaveLoad_Slot, menuItemButton::BTN_TYPE_SL_TEXT, false, true, _GM(slotTitles)[_GM(slotSelected) - 1]);
		} else {
			menuItemButton::buttonAdd(myMenu, 1000 + _GM(slotSelected) - _GM(firstSlotIndex), x, y, w, h,
				(CALLBACK)cb_SaveLoad_Slot, menuItemButton::BTN_TYPE_SL_TEXT, false, true, _GM(slotTitles)[_GM(slotSelected) - 1],
				(ItemHandlerFunction)load_Handler);

			// Remove the thumbnail
			if (_GM(saveLoadThumbNail)) {
				_GM(saveLoadThumbNail) = _GM(menuSprites)[Burger::GUI::SL_EMPTY_THUMB];
				guiMenu::itemRefresh(nullptr, SL_TAG_THUMBNAIL, myMenu);
			}
		}
		SetFirstSlot(_GM(firstSlotIndex), myMenu);

		// Enable the slider
		menu_EnableVSlider(nullptr, SL_TAG_VSLIDER, myMenu);
		guiMenu::itemRefresh(nullptr, SL_TAG_VSLIDER, myMenu);

		// Disable the save/load button
		if (_GM(currMenuIsSave)) {
			menuItemButton::disableButton(nullptr, SL_TAG_SAVE, myMenu);
			guiMenu::itemRefresh(nullptr, SL_TAG_SAVE, myMenu);
		} else {
			menuItemButton::disableButton(nullptr, SL_TAG_LOAD, myMenu);
			guiMenu::itemRefresh(nullptr, SL_TAG_LOAD, myMenu);
		}

		// Reset the slot selected var
		_GM(slotSelected) = -1;

	} else {
		// Otherwise, back to the game menu

		// Destroy the menu
		DestroySaveLoadMenu(_GM(currMenuIsSave));

		if (_GM(saveLoadFromHotkey)) {
			// Shutdown the menu system
			guiMenu::shutdown(true);
		} else {
			// Create the game menu
			CreateGameMenuMain(nullptr);
		}
	}

	buttonClosesDialog = true;
}


void cb_SaveLoad_Slot(menuItemButton *myButton, guiMenu *myMenu) {
	int32 i, x, y, w, h;
	char prompt[80];
	int32 specialTag;

	// Verify params
	if (!myMenu || !myButton)
		return;

	// Get the button
	Common::strcpy_s(prompt, 80, myButton->prompt);
	specialTag = myButton->specialTag;

	// Set the globals
	_GM(slotSelected) = myButton->specialTag;
	_GM(deleteSaveDesc) = true;

	// Disable all other buttons
	for (i = 1001; i <= 1010; i++) {
		if (i != myButton->tag) {
			menuItemButton::disableButton(nullptr, i, myMenu);
			guiMenu::itemRefresh(nullptr, i, myMenu);
		}
	}

	// Get the slot coords, and delete it
	x = myButton->x1;
	y = myButton->y1;
	w = myButton->x2 - myButton->x1 + 1;
	h = myButton->y2 - myButton->y1 + 1;
	guiMenu::itemDelete(myButton, -1, myMenu);

	if (_GM(currMenuIsSave)) {
		// Replace the current button with a textfield
		if (!strcmp(prompt, "<empty>")) {
			menu_TextFieldAdd(myMenu, 2000, x, y, w, h, TF_OVER,
				nullptr, specialTag, (CALLBACK)cb_SaveLoad_Save, true);
		} else {
			menu_TextFieldAdd(myMenu, 2000, x, y, w, h, TF_OVER,
				prompt, specialTag, (CALLBACK)cb_SaveLoad_Save, true);
		}
	} else {
		menu_TextFieldAdd(myMenu, 2000, x, y, w, h, TF_NORM,
			prompt, specialTag, (CALLBACK)cb_SaveLoad_Load, true);
	}

	// Disable the slider
	menu_DisableVSlider(nullptr, SL_TAG_VSLIDER, myMenu);
	guiMenu::itemRefresh(nullptr, SL_TAG_VSLIDER, myMenu);

	// Enable the save/load button
	if (_GM(currMenuIsSave)) {
		menuItemButton::enableButton(nullptr, SL_TAG_SAVE, myMenu);
		guiMenu::itemRefresh(nullptr, SL_TAG_SAVE, myMenu);
	} else {
		menuItemButton::enableButton(nullptr, SL_TAG_LOAD, myMenu);
		guiMenu::itemRefresh(nullptr, SL_TAG_LOAD, myMenu);
	}
}

void InitializeSlotTables(void) {
	const SaveStateList saves = g_engine->listSaves();

	// First reset all the slots to empty
	for (int i = 0; i < MAX_SLOTS; ++i) {
		Common::strcpy_s(_GM(slotTitles)[i], 80, "<empty>");
		_GM(slotInUse)[i] = false;
	}

	for (const auto &save : saves) {
		if (save.getSaveSlot() != 0) {
			Common::String desc = save.getDescription();
			Common::strcpy_s(_GM(slotTitles)[save.getSaveSlot() - 1], 80, desc.c_str());
			_GM(slotInUse)[save.getSaveSlot() - 1] = true;
		}
	}
}

bool load_Handler(menuItemButton *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	bool handled;

	// Handle the event just like any other button
	handled = menuItemButton::handler(myItem, eventType, event, x, y, currItem);

	// If we've selected a slot, we want the thumbNail to remain on the menu permanently
	if (_GM(slotSelected) >= 0) {
		return handled;
	}

	// But if the event moved the mouse, we want to display the correct thumbNail;
	if ((eventType == EVENT_MOUSE) && ((event == _ME_move) || (event == _ME_L_drag) || (event == _ME_L_release) ||
		(event == _ME_doubleclick_drag) || (event == _ME_doubleclick_release))) {

		// Get the button
		if (!myItem)
			return handled;

		// This determines that we are over the button
		if ((myItem->itemFlags == menuItemButton::BTN_STATE_OVER) || (myItem->itemFlags == menuItemButton::BTN_STATE_PRESS)) {
			// See if the current _GM(saveLoadThumbNail) is pointing to the correct sprite
			if (_GM(saveLoadThumbNail) != _GM(thumbNails)[myItem->specialTag - 1]) {
				_GM(saveLoadThumbNail) = _GM(thumbNails)[myItem->specialTag - 1];
				guiMenu::itemRefresh(nullptr, SL_TAG_THUMBNAIL, (guiMenu *)myItem->myMenu);
			}
		}

		// Else we must determine whether the thumbnail needs to be replaced with the empty thumbnail.
		else {

			// If the mouse has moved outside of the entire range of all 10 buttons,
			//or it is over a button which is not hilited it is to be removed.
			if (menuItem::cursorInsideItem(myItem, x, y)
				|| (x < SL_SCROLL_FIELD_X)
				|| (x > SL_SCROLL_FIELD_X + SL_SCROLL_FIELD_W)
				|| (y < SL_SCROLL_FIELD_Y)
				|| (y > SL_SCROLL_FIELD_Y + SL_SCROLL_FIELD_H)) {

				// Remove the thumbnail
				if (_GM(saveLoadThumbNail)) {
					_GM(saveLoadThumbNail) = _GM(menuSprites)[Burger::GUI::SL_EMPTY_THUMB];
					guiMenu::itemRefresh(nullptr, SL_TAG_THUMBNAIL, (guiMenu *)myItem->myMenu);
				}
			}
		}
	}

	return handled;
}


void DestroySaveLoadMenu(bool saveMenu) {
	int32 i;

	if (!_GM(slMenu)) {
		return;
	}

	// Determine whether the screen was the SAVE or the LOAD menu
	if (saveMenu) {

		// If SAVE, there should be a thumbnail to unload
		if (_GM(saveLoadThumbNail)) {
			DisposeHandle(_GM(saveLoadThumbNail)->sourceHandle);
			mem_free(_GM(saveLoadThumbNail));
			_GM(saveLoadThumbNail) = nullptr;
		}
	} else {
		// Else there may be up to 10 somewhere in the list to be unloaded
		for (i = 0; i < MAX_SLOTS; i++) {
			UnloadThumbNail(i);
		}
		_GM(saveLoadThumbNail) = nullptr;
	}

	// Destroy the screen
	vmng_screen_dispose(_GM(slMenu));
	guiMenu::destroy(_GM(slMenu));

	// Unload the save/load menu sprites
	guiMenu::unloadSprites();
}


void CreateSaveLoadMenu(RGB8 *myPalette, bool saveMenu) {
	ItemHandlerFunction	i_handler;
	bool buttonGreyed;

	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("slmenu", Burger::GUI::SL_TOTAL_SPRITES)) {
		return;
	}

	// Initialize some global vars
	_GM(firstSlotIndex) = 0;
	_GM(slotSelected) = -1;
	_GM(saveLoadThumbNail) = nullptr;
	_GM(thumbIndex) = 100;
	_GM(currMenuIsSave) = saveMenu;

	_GM(slMenu) = guiMenu::create(_GM(menuSprites)[Burger::GUI::SL_DIALOG_BOX], SAVE_LOAD_MENU_X, SAVE_LOAD_MENU_Y,
		MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(slMenu)) {
		return;
	}

	if (_GM(currMenuIsSave)) {
		menuItemMsg::msgAdd(_GM(slMenu), SL_TAG_SAVE_LABEL, SL_SAVE_LABEL_X, SL_SAVE_LABEL_Y, SL_SAVE_LABEL_W, SL_SAVE_LABEL_H);
		menuItemButton::buttonAdd(_GM(slMenu), SL_TAG_SAVE, SL_SAVE_X, SL_SAVE_Y, SL_SAVE_W, SL_SAVE_H,
			(CALLBACK)cb_SaveLoad_Save, menuItemButton::BTN_TYPE_SL_SAVE, true);
	} else {
		menuItemMsg::msgAdd(_GM(slMenu), SL_TAG_LOAD_LABEL, SL_LOAD_LABEL_X, SL_LOAD_LABEL_Y, SL_LOAD_LABEL_W, SL_LOAD_LABEL_H);
		menuItemButton::buttonAdd(_GM(slMenu), SL_TAG_LOAD, SL_LOAD_X, SL_LOAD_Y, SL_LOAD_W, SL_LOAD_H,
			(CALLBACK)cb_SaveLoad_Load, menuItemButton::BTN_TYPE_SL_LOAD, true);
	}

	menuItemButton::buttonAdd(_GM(slMenu), SL_TAG_CANCEL, SL_CANCEL_X, SL_CANCEL_Y, SL_CANCEL_W, SL_CANCEL_H,
		(CALLBACK)cb_SaveLoad_Cancel, menuItemButton::BTN_TYPE_SL_CANCEL);

	menu_VSliderAdd(_GM(slMenu), SL_TAG_VSLIDER, SL_SLIDER_X, SL_SLIDER_Y, SL_SLIDER_W, SL_SLIDER_H,
		0, (CALLBACK)cb_SaveLoad_VSlider);

	InitializeSlotTables();

	if (_GM(currMenuIsSave)) {
		buttonGreyed = false;
		i_handler = (ItemHandlerFunction)menuItemButton::handler;
	} else {
		buttonGreyed = true;
		i_handler = (ItemHandlerFunction)load_Handler;
	}

	for (int32 i = 0; i < MAX_SLOTS_SHOWN; i++) {
		menuItemButton::buttonAdd(_GM(slMenu), 1001 + i,
			SL_SCROLL_FIELD_X, SL_SCROLL_FIELD_Y + i * SL_SCROLL_LINE_H,
			SL_SCROLL_LINE_W, SL_SCROLL_LINE_H,
			(CALLBACK)cb_SaveLoad_Slot, menuItemButton::BTN_TYPE_SL_TEXT,
			buttonGreyed && (!_GM(slotInUse)[i]), true, _GM(slotTitles)[i], i_handler);
	}

	if (_GM(currMenuIsSave)) {
		// Create thumbnails. One in the original game format for displaying,
		// and the other in the ScummVM format for actually using in the save files
		_GM(saveLoadThumbNail) = menu_CreateThumbnail(&_GM(sizeofThumbData));
		_GM(_thumbnail).free();
		Graphics::createThumbnail(_GM(_thumbnail));

	} else {
		UpdateThumbNails(0, _GM(slMenu));
		_GM(saveLoadThumbNail) = _GM(menuSprites)[Burger::GUI::SL_EMPTY_THUMB];
	}

	menuItemMsg::msgAdd(_GM(slMenu), SL_TAG_THUMBNAIL, SL_THUMBNAIL_X, SL_THUMBNAIL_Y, SL_THUMBNAIL_W, SL_THUMBNAIL_H, false);

	if (_GM(currMenuIsSave)) {
		//<return> - if a slot has been selected, saves the game
		//<esc> - cancels and returns to the game menu
		guiMenu::configure(_GM(slMenu), (CALLBACK)cb_SaveLoad_Save, (CALLBACK)cb_SaveLoad_Cancel);
	} else {
		//<return> - if a slot has been selected, loads the selected game
		//<esc> - cancels and returns to the game menu
		guiMenu::configure(_GM(slMenu), (CALLBACK)cb_SaveLoad_Load, (CALLBACK)cb_SaveLoad_Cancel);
	}

	vmng_screen_show((void *)_GM(slMenu));
	LockMouseSprite(0);
}

void CreateGameMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(gameMenuFromMain) = false;
	CreateGameMenuMain(myPalette);
}

void CreateGameMenuFromMain(RGB8 *myPalette) {
	if (_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(gameMenuFromMain) = true;
	CreateGameMenuMain(myPalette);
}

void CreateSaveMenu(RGB8 *myPalette) {
	_GM(saveLoadFromHotkey) = false;
	CreateSaveLoadMenu(myPalette, true);
}

void CreateF2SaveMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = false;
	CreateSaveLoadMenu(myPalette, true);
}

void CreateLoadMenu(RGB8 *myPalette) {
	_GM(saveLoadFromHotkey) = false;
	CreateSaveLoadMenu(myPalette, false);
}

void CreateF3LoadMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = false;
	CreateSaveLoadMenu(myPalette, false);
}

void CreateLoadMenuFromMain(RGB8 *myPalette) {
	if (_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = true;
	CreateSaveLoadMenu(myPalette, false);
}

} // namespace GUI
} // namespace Burger
} // namespace M4
