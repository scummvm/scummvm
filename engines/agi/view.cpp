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
#include "agi/graphics.h"
#include "agi/sprite.h"

namespace Agi {

void AgiEngine::updateView(ScreenObjEntry *screenObj) {
	int16 celNr, lastCelNr;

	if (screenObj->flags & fDontupdate) {
		screenObj->flags &= ~fDontupdate;
		return;
	}

	celNr = screenObj->currentCelNr;
	lastCelNr = screenObj->celCount - 1;

	switch (screenObj->cycle) {
	case kCycleNormal:
		celNr++;
		if (celNr > lastCelNr)
			celNr = 0;
		break;
	case kCycleEndOfLoop:
		if (celNr < lastCelNr) {
			debugC(5, kDebugLevelResources, "cel %d (last = %d)", celNr + 1, lastCelNr);
			if (++celNr != lastCelNr)
				break;
		}
		setFlag(screenObj->loop_flag, true);
		screenObj->flags &= ~fCycling;
		screenObj->direction = 0;
		screenObj->cycle = kCycleNormal;
		break;
	case kCycleRevLoop:
		if (celNr) {
			celNr--;
			if (celNr)
				break;
		}
		setFlag(screenObj->loop_flag, true);
		screenObj->flags &= ~fCycling;
		screenObj->direction = 0;
		screenObj->cycle = kCycleNormal;
		break;
	case kCycleReverse:
		if (celNr == 0) {
			celNr = lastCelNr;
		} else {
			celNr--;
		}
		break;
	default:
		break;
	}

	setCel(screenObj, celNr);
}

/*
 * Public functions
 */

/**
 * Decode an AGI view resource.
 * This function decodes the raw data of the specified AGI view resource
 * and fills the corresponding views array element.
 * @param n number of view resource to decode
 */
int AgiEngine::decodeView(byte *resourceData, uint16 resourceSize, int16 viewNr) {
	AgiView *viewData = &_game.views[viewNr];
	uint16 headerId = 0;
	byte   headerStepSize = 0;
	byte   headerCycleTime = 0;
	byte   headerLoopCount = 0;
	uint16 headerDescriptionOffset = 0;
	bool   isAGI256Data = false;

	AgiViewLoop *loopData = nullptr;
	uint16 loopOffset = 0;
	byte   loopHeaderCelCount = 0;

	AgiViewCel *celData = nullptr;
	uint16 celOffset = 0;
	byte   celHeaderWidth = 0;
	byte   celHeaderHeight = 0;
	byte   celHeaderTransparencyMirror = 0;
	byte   celHeaderClearKey = 0;
	bool   celHeaderMirrored = false;
	byte   celHeaderMirrorLoop = 0;

	byte  *celCompressedData = nullptr;
	uint16 celCompressedSize = 0;

	debugC(5, kDebugLevelResources, "decode_view(%d)", viewNr);

	if (resourceSize < 5)
		error("unexpected end of view data for view %d", viewNr);

	headerId = READ_LE_UINT16(resourceData);
	if (getVersion() < 0x2000) {
		headerStepSize = resourceData[0];
		headerCycleTime = resourceData[1];
	}
	headerLoopCount = resourceData[2];
	headerDescriptionOffset = READ_LE_UINT16(resourceData + 3);

	if (headerId == 0xF00F)
		isAGI256Data = true; // AGI 256-2 view detected, 256 color view

	viewData->headerStepSize = headerStepSize;
	viewData->headerCycleTime = headerCycleTime;
	viewData->loopCount = headerLoopCount;
	viewData->description = nullptr;
	viewData->loop = nullptr;

	if (headerDescriptionOffset) {
		// Figure out length of description
		uint16 descriptionPos = headerDescriptionOffset;
		uint16 descriptionLen = 0;
		while (descriptionPos < resourceSize) {
			if (resourceData[descriptionPos] == 0)
				break;
			descriptionPos++;
			descriptionLen++;
		}
		// Allocate memory for description
		viewData->description = new byte[descriptionLen + 1];
		// Copy description over
		memcpy(viewData->description, resourceData + headerDescriptionOffset, descriptionLen);
		viewData->description[descriptionLen] = 0; // set terminator
	}

	if (!viewData->loopCount) // no loops, exit now
		return errOK;

	// Check, if at least the loop-offsets are available
	if (resourceSize < 5 + (headerLoopCount * 2))
		error("unexpected end of view data for view %d", viewNr);

	// Allocate space for loop-information
	loopData = new AgiViewLoop[headerLoopCount];
	viewData->loop = loopData;

	for (int16 loopNr = 0; loopNr < headerLoopCount; loopNr++) {
		loopOffset = READ_LE_UINT16(resourceData + 5 + (loopNr * 2));

		// Check, if at least the loop-header is available
		if (resourceSize < (loopOffset + 1))
			error("unexpected end of view data for view %d", viewNr);

		// loop-header:
		//  celCount:BYTE
		//  relativeCelOffset[0]:WORD
		//  relativeCelOffset[1]:WORD
		//  etc.
		loopHeaderCelCount = resourceData[loopOffset];

		loopData->celCount = loopHeaderCelCount;
		loopData->cel = nullptr;

		// Check, if at least the cel-offsets for current loop are available
		if (resourceSize < (loopOffset + 1 + (loopHeaderCelCount * 2)))
			error("unexpected end of view data for view %d", viewNr);

		if (loopHeaderCelCount) {
			// Allocate space for cel-information of current loop
			celData = new AgiViewCel[loopHeaderCelCount];
			loopData->cel = celData;

			for (int16 celNr = 0; celNr < loopHeaderCelCount; celNr++) {
				celOffset = READ_LE_UINT16(resourceData + loopOffset + 1 + (celNr * 2));
				celOffset += loopOffset; // cel offset is relative to loop offset, so adjust accordingly

				// Check, if at least the cel-header is available
				if (resourceSize < (celOffset + 3))
					error("unexpected end of view data for view %d", viewNr);

				// cel-header:
				//  width:BYTE
				//  height:BYTE
				//  Transparency + Mirroring:BYTE
				//  celData follows
				celHeaderWidth = resourceData[celOffset + 0];
				celHeaderHeight = resourceData[celOffset + 1];
				celHeaderTransparencyMirror = resourceData[celOffset + 2];

				if (!isAGI256Data) {
					// regular AGI view data
					// Transparency + Mirroring byte is as follows:
					//  Bit 0-3 - clear key
					//  Bit 4-6 - original loop, that is not supposed to be mirrored in any case
					//  Bit 7   - apply mirroring
					celHeaderClearKey = celHeaderTransparencyMirror & 0x0F; // bit 0-3 is the clear key
					celHeaderMirrored = false;
					if (celHeaderTransparencyMirror & 0x80) {
						// mirror bit is set
						celHeaderMirrorLoop = (celHeaderTransparencyMirror >> 4) & 0x07;
						if (celHeaderMirrorLoop != loopNr) {
							// only set to mirror'd in case we are not the original loop
							celHeaderMirrored = true;
						}
					}
				} else {
					// AGI256-2 view data
					celHeaderClearKey = celHeaderTransparencyMirror; // full 8 bits for clear key
					celHeaderMirrored = false;
				}

				celData->width = celHeaderWidth;
				celData->height = celHeaderHeight;
				celData->clearKey = celHeaderClearKey;
				celData->mirrored = celHeaderMirrored;

				// Now decompress cel-data
				if ((celHeaderWidth == 0) && (celHeaderHeight == 0))
					error("view cel is 0x0");

				celCompressedData = resourceData + celOffset + 3;
				celCompressedSize = resourceSize - (celOffset + 3);

				if (celCompressedSize == 0)
					error("compressed size of loop within view %d is 0 bytes", viewNr);

				if (!isAGI256Data) {
					unpackViewCelData(celData, celCompressedData, celCompressedSize);
				} else {
					unpackViewCelDataAGI256(celData, celCompressedData, celCompressedSize);
				}
				celData++;
			}
		}

		loopData++;
	}

	return errOK;
}

void AgiEngine::unpackViewCelData(AgiViewCel *celData, byte *compressedData, uint16 compressedSize) {
	byte *rawBitmap = new byte[celData->width * celData->height];
	int16 remainingHeight = celData->height;
	int16 remainingWidth = celData->width;
	bool  isMirrored = celData->mirrored;
	byte curByte;
	byte curColor;
	byte curChunkLen;
	int16 adjustPreChangeSingle = 0;
	int16 adjustAfterChangeSingle = +1;

	celData->rawBitmap = rawBitmap;

	if (isMirrored) {
		adjustPreChangeSingle = -1;
		adjustAfterChangeSingle = 0;
		rawBitmap += celData->width;
	}

	while (remainingHeight) {
		if (!compressedSize)
			error("unexpected end of data, while unpacking AGI256 data");

		curByte = *compressedData++;
		compressedSize--;

		if (curByte == 0) {
			curColor = celData->clearKey;
			curChunkLen = remainingWidth;
		} else {
			curColor = curByte >> 4;
			curChunkLen = curByte & 0x0F;
			if (curChunkLen > remainingWidth)
				error("invalid chunk in view data");
		}

		switch (curChunkLen) {
		case 0:
			break;
		case 1:
			rawBitmap += adjustPreChangeSingle;
			*rawBitmap = curColor;
			rawBitmap += adjustAfterChangeSingle;
			break;
		default:
			if (isMirrored)
				rawBitmap -= curChunkLen;
			memset(rawBitmap, curColor, curChunkLen);
			if (!isMirrored)
				rawBitmap += curChunkLen;
			break;
		}

		remainingWidth -= curChunkLen;

		if (curByte == 0) {
			remainingWidth = celData->width;
			remainingHeight--;

			if (isMirrored)
				rawBitmap += celData->width * 2;
		}
	}

	// for CGA rendering, apply dithering
	switch (_renderMode) {
	case Common::kRenderCGA: {
		uint16 totalPixels = celData->width * celData->height;

		// dither clear key
		celData->clearKey = _gfx->getCGAMixtureColor(celData->clearKey);

		rawBitmap = celData->rawBitmap;
		for (uint16 pixelNr = 0; pixelNr < totalPixels; pixelNr++) {
			curColor = *rawBitmap;
			*rawBitmap = _gfx->getCGAMixtureColor(curColor);
			rawBitmap++;
		}
		break;
	}
	default:
		break;
	}
}

void AgiEngine::unpackViewCelDataAGI256(AgiViewCel *celData, byte *compressedData, uint16 compressedSize) {
	byte *rawBitmap = new byte[celData->width * celData->height];
	int16 remainingHeight = celData->height;
	int16 remainingWidth = celData->width;
	byte curByte;

	celData->rawBitmap = rawBitmap;

	while (remainingHeight) {
		if (!compressedSize)
			error("unexpected end of data, while unpacking AGI256 view");

		curByte = *compressedData++;
		compressedSize--;

		if (curByte == 0) {
			// Go to next vertical position
			if (remainingWidth) {
				// fill remaining bytes with clear key
				memset(rawBitmap, celData->clearKey, remainingWidth);
				rawBitmap += remainingWidth;
				remainingWidth = 0;
			}
		} else {
			if (!remainingWidth) {
				error("broken view data, while unpacking AGI256 view");
				break;
			}
			*rawBitmap = curByte;
			rawBitmap++;
			remainingWidth--;
		}

		if (curByte == 0) {
			remainingWidth = celData->width;
			remainingHeight--;
		}
	}
}

/**
 * Unloads all data in a view resource
 * @param n number of view resource
 */
void AgiEngine::unloadView(int16 viewNr) {
	AgiView *viewData = &_game.views[viewNr];

	debugC(5, kDebugLevelResources, "discard view %d", viewNr);
	if (!(_game.dirView[viewNr].flags & RES_LOADED))
		return;

	// Rebuild sprite list, see Sarien bug #779302
	_sprites->eraseSprites();

	// free data
	for (int16 loopNr = 0; loopNr < viewData->loopCount; loopNr++) {
		AgiViewLoop *loopData = &viewData->loop[loopNr];
		for (int16 celNr = 0; celNr < loopData->celCount; celNr++) {
			AgiViewCel *celData = &loopData->cel[celNr];

			delete[] celData->rawBitmap;
		}
		delete[] loopData->cel;
	}
	delete[] viewData->loop;

	if (viewData->description)
		delete[] viewData->description;

	viewData->headerCycleTime = 0;
	viewData->headerStepSize = 0;
	viewData->description = nullptr;
	viewData->loop = nullptr;
	viewData->loopCount = 0;

	// Mark this view as not loaded anymore
	_game.dirView[viewNr].flags &= ~RES_LOADED;

	_sprites->buildAllSpriteLists();
	_sprites->drawAllSpriteLists();
}

/**
 * Set a view table entry to use the specified view resource.
 * @param screenObj pointer to screen object
 * @param viewNr number of AGI view resource
 */
void AgiEngine::setView(ScreenObjEntry *screenObj, int16 viewNr) {
	if (!(_game.dirView[viewNr].flags & RES_LOADED)) {
		// View resource currently not loaded, this is probably a game bug
		// Load the resource now to fix the issue, and give out a warning
		// This happens in at least Larry 1 for Apple IIgs right after getting beaten up by taxi driver
		// Original interpreter bombs out in this situation saying "view not loaded, Press ESC to quit"
		warning("setView() called on screen object %d to use view %d, but view not loaded", screenObj->objectNr, viewNr);
		warning("probably game script bug, trying to load view into memory");
		if (agiLoadResource(RESOURCETYPE_VIEW, viewNr) != errOK) {
			// loading failed, we better error() out now
			error("setView() called to set view %d for screen object %d, which is not loaded atm and loading failed", viewNr, screenObj->objectNr);
			return;
		};
	}

	screenObj->viewResource = &_game.views[viewNr];
	screenObj->currentViewNr = viewNr;
	screenObj->loopCount = screenObj->viewResource->loopCount;
	screenObj->viewReplaced = true;

	if (getVersion() < 0x2000) {
		screenObj->stepSize = screenObj->viewResource->headerStepSize;
		screenObj->cycleTime = screenObj->viewResource->headerCycleTime;
		screenObj->cycleTimeCount = 0;
	}
	if (screenObj->currentLoopNr >= screenObj->loopCount) {
		setLoop(screenObj, 0);
	} else {
		setLoop(screenObj, screenObj->currentLoopNr);
	}
}

/**
 * Set a view table entry to use the specified loop of the current view.
 * @param screenObj pointer to screen object
 * @param loopNr number of loop
 */
void AgiEngine::setLoop(ScreenObjEntry *screenObj, int16 loopNr) {
	if (!(_game.dirView[screenObj->currentViewNr].flags & RES_LOADED)) {
		error("setLoop() called on screen object %d, which has no loaded view resource assigned to it", screenObj->objectNr);
		return;
	}
	assert(screenObj->viewResource);

	if (screenObj->loopCount == 0) {
		warning("setLoop() called on screen object %d, which has no loops (view %d)", screenObj->objectNr, screenObj->currentViewNr);
		return;
	}

	if (loopNr >= screenObj->loopCount) {
		// requested loop not existant
		// instead of error()ing out, we instead clip it
		// At least required for possibly Manhunter 1 according to previous comment when leaving the arcade machine
		// TODO: Check MH1
		// TODO: This causes an issue in KQ1, when bowing to the king in room 53
		//       Ego will face away from the king, because the scripts set the loop first and then the view
		//       Loop is corrected by us, because at that time it's invalid. Was already present in 1.7.0
		//       We should probably script-patch it out.
		int16 requestedLoopNr = loopNr;

		loopNr = screenObj->loopCount - 1;

		warning("Non-existant loop requested for screen object %d", screenObj->objectNr);
		warning("view %d, requested loop %d -> clipped to loop %d", screenObj->currentViewNr, requestedLoopNr, loopNr);
	}

	AgiViewLoop *curViewLoop = &_game.views[screenObj->currentViewNr].loop[loopNr];

	screenObj->currentLoopNr = loopNr;
	screenObj->loopData = curViewLoop;
	screenObj->celCount = curViewLoop->celCount;

	if (screenObj->currentCelNr >= screenObj->celCount) {
		setCel(screenObj, 0);
	} else {
		setCel(screenObj, screenObj->currentCelNr);
	}
}

/**
 * Set a view table entry to use the specified cel of the current loop.
 * @param screenObj pointer to screen object
 * @param celNr number of cel
 */
void AgiEngine::setCel(ScreenObjEntry *screenObj, int16 celNr) {
	if (!(_game.dirView[screenObj->currentViewNr].flags & RES_LOADED)) {
		error("setCel() called on screen object %d, which has no loaded view resource assigned to it", screenObj->objectNr);
		return;
	}
	assert(screenObj->viewResource);

	if (screenObj->loopCount == 0) {
		warning("setLoop() called on screen object %d, which has no loops (view %d)", screenObj->objectNr, screenObj->currentViewNr);
		return;
	}

	AgiViewLoop *curViewLoop = &_game.views[screenObj->currentViewNr].loop[screenObj->currentLoopNr];

	// Added by Amit Vainsencher <amitv@subdimension.com> to prevent
	// crash in KQ1 -- not in the Sierra interpreter
	if (curViewLoop->celCount == 0) {
		warning("setCel() called on screen object %d, which has no cels (view %d)", screenObj->objectNr, screenObj->currentViewNr);
		return;
	}

	if (celNr >= screenObj->celCount) {
		// requested cel not existant
		// instead of error()ing out, we instead clip it
		// At least required for King's Quest 3 on Apple IIgs - walking the planks death cutscene
		// see bug #5832, which is a game bug!
		int16 requestedCelNr = celNr;

		celNr = screenObj->celCount - 1;

		warning("Non-existant cel requested for screen object %d", screenObj->objectNr);
		warning("view %d, loop %d, requested cel %d -> clipped to cel %d", screenObj->currentViewNr, screenObj->currentLoopNr, requestedCelNr, celNr);
	}

	screenObj->currentCelNr = celNr;

	AgiViewCel *curViewCel;
	curViewCel         = &curViewLoop->cel[celNr];
	screenObj->celData = curViewCel;
	screenObj->xSize   = curViewCel->width;
	screenObj->ySize   = curViewCel->height;

	// If position isn't appropriate, update it accordingly
	clipViewCoordinates(screenObj);
}

/**
 * Restrict view table entry's position so it stays wholly inside the screen.
 * Also take horizon into account when clipping if not set to ignore it.
 * @param v pointer to view table entry
 */
void AgiEngine::clipViewCoordinates(ScreenObjEntry *screenObj) {
	if (screenObj->xPos + screenObj->xSize > SCRIPT_WIDTH) {
		screenObj->flags |= fUpdatePos;
		screenObj->xPos = SCRIPT_WIDTH - screenObj->xSize;
	}
	if (screenObj->yPos - screenObj->ySize + 1 < 0) {
		screenObj->flags |= fUpdatePos;
		screenObj->yPos = screenObj->ySize - 1;
	}
	if (screenObj->yPos <= _game.horizon && (~screenObj->flags & fIgnoreHorizon)) {
		screenObj->flags |= fUpdatePos;
		screenObj->yPos = _game.horizon + 1;
	}

	if (getVersion() < 0x2000) {
		screenObj->flags |= fDontupdate;
	}

}

/**
 * Set the view table entry as updating.
 * @param v pointer to view table entry
 */
void AgiEngine::startUpdate(ScreenObjEntry *v) {
	if (~v->flags & fUpdate) {
		_sprites->eraseSprites();
		v->flags |= fUpdate;
		_sprites->buildAllSpriteLists();
		_sprites->drawAllSpriteLists();
	}
}

/**
 * Set the view table entry as non-updating.
 * @param v pointer to view table entry
 */
void AgiEngine::stopUpdate(ScreenObjEntry *viewPtr) {
	if (viewPtr->flags & fUpdate) {
		_sprites->eraseSprites();
		viewPtr->flags &= ~fUpdate;
		_sprites->buildAllSpriteLists();
		_sprites->drawAllSpriteLists();
	}
}

// loops to use according to direction and number of loops in
// the view resource
static int loopTable2[] = {
	0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x01
};

static int loopTable4[] = {
	0x04, 0x03, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x01
};

/**
 * Update view table entries.
 * This function is called at the end of each interpreter cycle
 * to update the view table entries and blit the sprites.
 */
void AgiEngine::updateScreenObjTable() {
	ScreenObjEntry *screenObj;
	int16 changeCount, loopNr;

	changeCount = 0;
	for (screenObj = _game.screenObjTable; screenObj < &_game.screenObjTable[SCREENOBJECTS_MAX]; screenObj++) {
		if ((screenObj->flags & (fAnimated | fUpdate | fDrawn)) != (fAnimated | fUpdate | fDrawn)) {
			continue;
		}

		changeCount++;

		loopNr = 4;
		if (!(screenObj->flags & fFixLoop)) {
			switch (screenObj->loopCount) {
			case 2:
			case 3:
				loopNr = loopTable2[screenObj->direction];
				break;
			case 4:
				loopNr = loopTable4[screenObj->direction];
				break;
			default:
				// for KQ4
				if (getVersion() == 0x3086 || getGameID() == GID_KQ4)
					loopNr = loopTable4[screenObj->direction];
				break;
			}
		}

		// AGI 2.272 (ddp, xmas) doesn't test step_time_count!
		if (loopNr != 4 && loopNr != screenObj->currentLoopNr) {
			if (getVersion() <= 0x2272 || screenObj->stepTimeCount == 1) {
				setLoop(screenObj, loopNr);
			}
		}

		if (screenObj->flags & fCycling) {
			if (screenObj->cycleTimeCount) {
				screenObj->cycleTimeCount--;
				if (screenObj->cycleTimeCount == 0) {
					updateView(screenObj);
					screenObj->cycleTimeCount = screenObj->cycleTime;
				}
			}
		}
	}

	if (changeCount) {
		_sprites->eraseRegularSprites();
		updatePosition();
		_sprites->buildRegularSpriteList();
		_sprites->drawRegularSpriteList();
		_sprites->showRegularSpriteList();

		_game.screenObjTable[SCREENOBJECTS_EGO_ENTRY].flags &= ~(fOnWater | fOnLand);
	}
}

bool AgiEngine::isEgoView(const ScreenObjEntry *screenObj) {
	return screenObj == _game.screenObjTable;
}

} // End of namespace Agi
