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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/text.h"

namespace Agi {

SpritesMgr::SpritesMgr(AgiEngine *agi, GfxMgr *gfx) {
	_vm = agi;
	_gfx = gfx;
}

SpritesMgr::~SpritesMgr() {
	_spriteRegularList.clear();
	_spriteStaticList.clear();
}

static bool sortSpriteHelper(const Sprite &entry1, const Sprite &entry2) {
	if (entry1.sortOrder == entry2.sortOrder) {
		// If sort-order is the same, we sort according to given order
		// which makes this sort stable.
		return entry1.givenOrderNr < entry2.givenOrderNr;
	}
	return entry1.sortOrder < entry2.sortOrder;
}

void SpritesMgr::buildRegularSpriteList() {
	ScreenObjEntry *screenObj = NULL;
	uint16 givenOrderNr = 0;

	freeList(_spriteRegularList);
	for (screenObj = _vm->_game.screenObjTable; screenObj < &_vm->_game.screenObjTable[SCREENOBJECTS_MAX]; screenObj++) {
		if ((screenObj->flags & (fAnimated | fUpdate | fDrawn)) == (fAnimated | fUpdate | fDrawn)) {
			buildSpriteListAdd(givenOrderNr, screenObj, _spriteRegularList);
			givenOrderNr++;
		}
	}

	// Now sort this list
	Common::sort(_spriteRegularList.begin(), _spriteRegularList.end(), sortSpriteHelper);
//	warning("buildRegular: %d", _spriteRegularList.size());
}

void SpritesMgr::buildStaticSpriteList() {
	ScreenObjEntry *screenObj = NULL;
	uint16 givenOrderNr = 0;

	freeList(_spriteStaticList);
	for (screenObj = _vm->_game.screenObjTable; screenObj < &_vm->_game.screenObjTable[SCREENOBJECTS_MAX]; screenObj++) {
		if ((screenObj->flags & (fAnimated | fUpdate | fDrawn)) == (fAnimated | fDrawn)) { // DIFFERENCE IN HERE!
			buildSpriteListAdd(givenOrderNr, screenObj, _spriteStaticList);
			givenOrderNr++;
		}
	}

	// Now sort this list
	Common::sort(_spriteStaticList.begin(), _spriteStaticList.end(), sortSpriteHelper);
}

void SpritesMgr::buildAllSpriteLists() {
	buildStaticSpriteList();
	buildRegularSpriteList();
}

void SpritesMgr::buildSpriteListAdd(uint16 givenOrderNr, ScreenObjEntry *screenObj, SpriteList &spriteList) {
	Sprite spriteEntry;

	// Check, if screen object points to currently loaded view, if not don't add it
	if (!(_vm->_game.dirView[screenObj->currentViewNr].flags & RES_LOADED))
		return;

	spriteEntry.givenOrderNr = givenOrderNr;
//	warning("sprite add objNr %d", screenObjPtr->objectNr);
	if (screenObj->flags & fFixedPriority) {
		spriteEntry.sortOrder = _gfx->priorityToY(screenObj->priority);
//		warning(" - priorityToY (fixed) %d -> %d", screenObj->priority, spriteEntry.sortOrder);
	} else {
		spriteEntry.sortOrder = screenObj->yPos;
//		warning(" - Ypos %d -> %d", screenObjPtr->yPos, spriteEntry.sortOrder);
	}

	spriteEntry.screenObjPtr = screenObj;
	spriteEntry.xPos = screenObj->xPos;
	spriteEntry.yPos = (screenObj->yPos) - (screenObj->ySize) + 1;
	spriteEntry.xSize = screenObj->xSize;
	spriteEntry.ySize = screenObj->ySize;

	// Checking, if xPos/yPos/right/bottom are valid and do not go outside of playscreen (visual screen)
	// Original AGI did not do this (but it then resulted in memory corruption)
	if (spriteEntry.xPos < 0) {
		warning("buildSpriteListAdd(): ignoring screen obj %d, b/c xPos (%d) < 0", screenObj->objectNr, spriteEntry.xPos);
		return;
	}
	if (spriteEntry.yPos < 0) {
		warning("buildSpriteListAdd(): ignoring screen obj %d, b/c yPos (%d) < 0", screenObj->objectNr, spriteEntry.yPos);
		return;
	}
	int16 xRight = spriteEntry.xPos + spriteEntry.xSize;
	if (xRight > SCRIPT_HEIGHT) {
		warning("buildSpriteListAdd(): ignoring screen obj %d, b/c rightPos (%d) > %d", screenObj->objectNr, xRight, SCRIPT_WIDTH);
		return;
	}
	int16 yBottom = spriteEntry.yPos + spriteEntry.ySize;
	if (yBottom > SCRIPT_HEIGHT) {
		warning("buildSpriteListAdd(): ignoring screen obj %d, b/c bottomPos (%d) > %d", screenObj->objectNr, yBottom, SCRIPT_HEIGHT);
		return;
	}

//	warning("list-add: %d, %d, original yPos: %d, ySize: %d", spriteEntry.xPos, spriteEntry.yPos, screenObj->yPos, screenObj->ySize);
	spriteEntry.backgroundBuffer = (uint8 *)malloc(spriteEntry.xSize * spriteEntry.ySize * 2); // for visual + priority data
	assert(spriteEntry.backgroundBuffer);
	spriteList.push_back(spriteEntry);
}

void SpritesMgr::freeList(SpriteList &spriteList) {
	SpriteList::iterator iter;
	for (iter = spriteList.reverse_begin(); iter != spriteList.end(); iter--) {
		Sprite &sprite = *iter;

		free(sprite.backgroundBuffer);
	}
	spriteList.clear();
}

void SpritesMgr::freeRegularSprites() {
	freeList(_spriteRegularList);
}

void SpritesMgr::freeStaticSprites() {
	freeList(_spriteStaticList);
}

void SpritesMgr::freeAllSprites() {
	freeList(_spriteRegularList);
	freeList(_spriteStaticList);
}

void SpritesMgr::eraseSprites(SpriteList &spriteList) {
	SpriteList::iterator iter;
//	warning("eraseSprites - count %d", spriteList.size());
	for (iter = spriteList.reverse_begin(); iter != spriteList.end(); iter--) {
		Sprite &sprite = *iter;
		_gfx->block_restore(sprite.xPos, sprite.yPos, sprite.xSize, sprite.ySize, sprite.backgroundBuffer);
	}

	freeList(spriteList);
}

/**
 * Erase updating sprites.
 * This function follows the list of all updating sprites and restores
 * the visible and priority data of their background buffers back to
 * the AGI screen.
 *
 * @see erase_nonupd_sprites()
 * @see erase_both()
 */
void SpritesMgr::eraseRegularSprites() {
	eraseSprites(_spriteRegularList);
}

void SpritesMgr::eraseStaticSprites() {
	eraseSprites(_spriteStaticList);
}

void SpritesMgr::eraseSprites() {
	eraseSprites(_spriteRegularList);
	eraseSprites(_spriteStaticList);
}

/**
 * Draw all sprites in the given list.
 */
void SpritesMgr::drawSprites(SpriteList &spriteList) {
	SpriteList::iterator iter;
//	warning("drawSprites");

	for (iter = spriteList.begin(); iter != spriteList.end(); ++iter) {
		Sprite &sprite = *iter;
		ScreenObjEntry *screenObj = sprite.screenObjPtr;

		_gfx->block_save(sprite.xPos, sprite.yPos, sprite.xSize, sprite.ySize, sprite.backgroundBuffer);
		//debugC(8, kDebugLevelSprites, "drawSprites(): s->v->entry = %d (prio %d)", s->viewPtr->entry, s->viewPtr->priority);
//		warning("sprite %d (view %d), priority %d, sort %d, givenOrder %d", screenObj->objectNr, screenObj->currentView, screenObj->priority, sprite.sortOrder, sprite.givenOrderNr);
		drawCel(screenObj);
	}
}

/**
 * Blit updating sprites.
 * This function follows the list of all updating sprites and blits
 * them on the AGI screen.
 *
 * @see blit_nonupd_sprites()
 * @see blit_both()
 */
void SpritesMgr::drawRegularSpriteList() {
	debugC(7, kDebugLevelSprites, "drawRegularSpriteList()");
	drawSprites(_spriteRegularList);
}

void SpritesMgr::drawStaticSpriteList() {
	//debugC(7, kDebugLevelSprites, "drawRegularSpriteList()");
	drawSprites(_spriteStaticList);
}

void SpritesMgr::drawAllSpriteLists() {
	drawSprites(_spriteStaticList);
	drawSprites(_spriteRegularList);
}

void SpritesMgr::drawCel(ScreenObjEntry *screenObj) {
	int16 curX = screenObj->xPos;
	int16 baseX = screenObj->xPos;
	int16 curY = screenObj->yPos;
	AgiViewCel *celPtr = screenObj->celData;
	byte *celDataPtr = celPtr->rawBitmap;
	uint8 remainingCelHeight = celPtr->height;
	uint8 celWidth = celPtr->width;
	byte celClearKey = celPtr->clearKey;
	byte viewPriority = screenObj->priority;
	byte screenPriority = 0;
	byte curColor = 0;
	byte isViewHidden = true;

	// Adjust vertical position, given yPos is lower left, but we need upper left
	curY = curY - celPtr->height + 1;

	while (remainingCelHeight) {
		for (int16 loopX = 0; loopX < celWidth; loopX++) {
			curColor = *celDataPtr++;

			if (curColor != celClearKey) {
				screenPriority = _gfx->getPriority(curX, curY);
				if (screenPriority <= 2) {
					// control data found
					if (_gfx->checkControlPixel(curX, curY, viewPriority)) {
						_gfx->putPixel(curX, curY, GFX_SCREEN_MASK_VISUAL, curColor, 0);
						isViewHidden = false;
					}
				} else if (screenPriority <= viewPriority) {
					_gfx->putPixel(curX, curY, GFX_SCREEN_MASK_ALL, curColor, viewPriority);
					isViewHidden = false;
				}

			}
			curX++;
		}

		// go to next vertical position
		remainingCelHeight--;
		curX = baseX;
		curY++;
	}

	if (screenObj->objectNr == 0) { // if ego, update if ego is visible at the moment
		_vm->setFlag(VM_FLAG_EGO_INVISIBLE, isViewHidden);
	}
}


void SpritesMgr::showSprite(ScreenObjEntry *screenObj) {
	int16 x = 0;
	int16 y = 0;
	int16 width = 0;
	int16 height = 0;

	int16 view_height_prev = 0;
	int16 view_width_prev = 0;

	int16 y2 = 0;
	int16 height1 = 0;
	int16 height2 = 0;

	int16 x2 = 0;
	int16 width1 = 0;
	int16 width2 = 0;

	if (!_vm->_game.pictureShown)
		return;

	view_height_prev = screenObj->ySize_prev;
	view_width_prev  = screenObj->xSize_prev;

	screenObj->ySize_prev = screenObj->ySize;
	screenObj->xSize_prev = screenObj->xSize;

	if (screenObj->yPos < screenObj->yPos_prev) {
		y = screenObj->yPos_prev;
		y2 = screenObj->yPos;

		height1 = view_height_prev;
		height2 = screenObj->ySize;
	} else {
		y = screenObj->yPos;
		y2 = screenObj->yPos_prev;

		height1 = screenObj->ySize;
		height2 = view_height_prev;
	}

	if ((y2 - height2) > (y - height1)) {
		height = height1;
	} else {
		height = y - y2 + height2;
	}

	if (screenObj->xPos > screenObj->xPos_prev) {
		x = screenObj->xPos_prev;
		x2 = screenObj->xPos;
		width1 = view_width_prev;
		width2 = screenObj->xSize;
	} else {
		x = screenObj->xPos;
		x2 = screenObj->xPos_prev;
		width1 = screenObj->xSize;
		width2 = view_width_prev;
	}

	if ((x2 + width2) < (x + width1)) {
		width = width1;
	} else {
		width = width2 + x2 - x;
	}

	if ((x + width) > 161) {
		width = 161 - x;
	}

	if (1 < (height - y)) {
		height = y + 1;
	}

	// render this block
	int16 upperY = y - height + 1;
	_gfx->render_Block(x, upperY, width, height);
}

void SpritesMgr::showSprites(SpriteList &spriteList) {
	SpriteList::iterator iter;
	ScreenObjEntry *screenObjPtr = NULL;

	for (iter = spriteList.begin(); iter != spriteList.end(); ++iter) {
		Sprite &sprite = *iter;
		screenObjPtr = sprite.screenObjPtr;

		showSprite(screenObjPtr);

		if (screenObjPtr->stepTimeCount == screenObjPtr->stepTime) {
			if ((screenObjPtr->xPos == screenObjPtr->xPos_prev) && (screenObjPtr->yPos == screenObjPtr->yPos_prev)) {
				screenObjPtr->flags |= fDidntMove;
			} else {
				screenObjPtr->xPos_prev = screenObjPtr->xPos;
				screenObjPtr->yPos_prev = screenObjPtr->yPos;
				screenObjPtr->flags &= ~fDidntMove;
			}
		}
	}
	g_system->updateScreen();
	//g_system->delayMillis(20);
}

void SpritesMgr::showRegularSpriteList() {
	debugC(7, kDebugLevelSprites, "showRegularSpriteList()");
	showSprites(_spriteRegularList);
}

void SpritesMgr::showStaticSpriteList() {
	debugC(7, kDebugLevelSprites, "showStaticSpriteList()");
	showSprites(_spriteStaticList);
}

void SpritesMgr::showAllSpriteLists() {
	showSprites(_spriteStaticList);
	showSprites(_spriteRegularList);
}

/**
 * Show object and description
 * This function shows an object from the player's inventory, displaying
 * a message box with the object description.
 * @param n  Number of the object to show
 */
void SpritesMgr::showObject(int16 viewNr) {
	ScreenObjEntry screenObj;
	uint8 *backgroundBuffer = NULL;

	_vm->agiLoadResource(RESOURCETYPE_VIEW, viewNr);
	_vm->setView(&screenObj, viewNr);

	screenObj.ySize_prev = screenObj.celData->height;
	screenObj.xSize_prev = screenObj.celData->width;
	screenObj.xPos_prev = ((SCRIPT_WIDTH - 1) - screenObj.xSize) / 2;
	screenObj.xPos = screenObj.xPos_prev;
	screenObj.yPos_prev = SCRIPT_HEIGHT - 1;
	screenObj.yPos = screenObj.yPos_prev;
	screenObj.priority = 15;
	screenObj.flags = fFixedPriority; // Original AGI did "| fFixedPriority" on uninitialized memory
	screenObj.objectNr = 255; // ???

	backgroundBuffer = (uint8 *)malloc(screenObj.xSize * screenObj.ySize * 2); // for visual + priority data

	_gfx->block_save(screenObj.xPos, (screenObj.yPos - screenObj.ySize + 1), screenObj.xSize, screenObj.ySize, backgroundBuffer);
	drawCel(&screenObj);
	showSprite(&screenObj);

	_vm->_text->messageBox((char *)_vm->_game.views[viewNr].description);

	_gfx->block_restore(screenObj.xPos, (screenObj.yPos - screenObj.ySize + 1), screenObj.xSize, screenObj.ySize, backgroundBuffer);
	showSprite(&screenObj);

	free(backgroundBuffer);
}

/**
 * Add view to picture.
 * This function is used to implement the add.to.pic AGI command. It
 * copies the specified cel from a view resource on the current picture.
 * This cel is not a sprite, it can't be moved or removed.
 * @param view  number of view resource
 * @param loop  number of loop in the specified view resource
 * @param cel   number of cel in the specified loop
 * @param x     x coordinate to place the view
 * @param y     y coordinate to place the view
 * @param pri   priority to use
 * @param mar   if < 4, create a margin around the the base of the cel
 */
void SpritesMgr::addToPic(int16 viewNr, int16 loopNr, int16 celNr, int16 xPos, int16 yPos, int16 priority, int16 border) {
	debugC(3, kDebugLevelSprites, "addToPic(view=%d, loop=%d, cel=%d, x=%d, y=%d, pri=%d, border=%d)", viewNr, loopNr, celNr, xPos, yPos, priority, border);

	_vm->recordImageStackCall(ADD_VIEW, viewNr, loopNr, celNr, xPos, yPos, priority, border);

	ScreenObjEntry *screenObj = &_vm->_game.addToPicView;
	screenObj->objectNr = -1; // addToPic-view

	_vm->setView(screenObj, viewNr);
	_vm->setLoop(screenObj, loopNr);
	_vm->setCel(screenObj, celNr);

	screenObj->xSize_prev = screenObj->xSize;
	screenObj->ySize_prev = screenObj->ySize;
	screenObj->xPos_prev = xPos;
	screenObj->xPos = xPos;
	screenObj->yPos_prev = yPos;
	screenObj->yPos = yPos;
	screenObj->flags = fIgnoreObjects | fIgnoreHorizon | fFixedPriority;
	screenObj->priority = 15;
	_vm->fixPosition(screenObj);
	if (priority == 0) {
		screenObj->flags = fIgnoreHorizon;
	}
	screenObj->priority = priority;

	eraseSprites();

	// bugs related to this code: required by Gold Rush (see Sarien bug #587558)
	if (screenObj->priority == 0) {
		screenObj->priority = _gfx->priorityFromY(screenObj->yPos);
	}
	drawCel(screenObj);

	if (border <= 3) {
		// Create priority-box
		addToPicDrawPriorityBox(screenObj, border);
	}
	buildAllSpriteLists();
	drawAllSpriteLists();
	showSprite(screenObj);
}

// bugs previously related to this:
// Sarien bug #247)
void SpritesMgr::addToPicDrawPriorityBox(ScreenObjEntry *screenObj, int16 border) {
	int16 priorityFromY = _gfx->priorityFromY(screenObj->yPos);
	int16 priorityHeight = 0;
	int16 curY = 0;
	int16 curX = 0;
	int16 height = 0;
	int16 width = 0;
	int16 offsetX = 0;

	// Figure out the height of the box
	curY = screenObj->yPos;
	do {
		priorityHeight++;
		if (curY <= 0)
			break;
		curY--;
	} while (_gfx->priorityFromY(curY) == priorityFromY);

	// box height may not be larger than the actual view
	if (screenObj->ySize < priorityHeight)
		priorityHeight = screenObj->ySize;

	// now actually draw lower horizontal line
	curY = screenObj->yPos;
	curX = screenObj->xPos;

	width = screenObj->xSize;
	while (width) {
		_gfx->putPixel(curX, curY, GFX_SCREEN_MASK_PRIORITY, 0, border);
		curX++;
		width--;
	}

	if (priorityHeight > 1) {
		// Actual rectangle is needed
		curY = screenObj->yPos;
		curX = screenObj->xPos;
		offsetX = screenObj->xSize - 1;

		height = priorityHeight - 1;
		while (height) {
			curY--;
			height--;
			_gfx->putPixel(curX, curY, GFX_SCREEN_MASK_PRIORITY, 0, border); // left line
			_gfx->putPixel(curX + offsetX, curY, GFX_SCREEN_MASK_PRIORITY, 0, border); // right line
		}

		// and finally the upper horizontal line
		width = screenObj->xSize - 2;
		curX++;
		while (width > 0) {
			_gfx->putPixel(curX, curY, GFX_SCREEN_MASK_PRIORITY, 0, border);
			curX++;
			width--;
		}
	}
}

} // End of namespace Agi
