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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/screen.h"
#include "mads/palette.h"
#include "mads/user_interface.h"

namespace MADS {

MADSEngine *DirtyArea::_vm = nullptr;

DirtyArea::DirtyArea() {
	_active = false;
	_textActive = false;
	_mergedArea = nullptr;
}

void DirtyArea::setArea(int width, int height, int maxWidth, int maxHeight) {
	if (_bounds.left % 2) {
		--_bounds.left;
		++width;
	}

	if (_bounds.left < 0)
		_bounds.left = 0;
	else if (_bounds.left > maxWidth)
		_bounds.left = maxWidth;
	int right = _bounds.left + width;
	if (right < 0)
		right = 0;
	if (right > maxWidth)
		right = maxWidth;

	_bounds.right = right;

	if (_bounds.top < 0)
		_bounds.top = 0;
	else if (_bounds.top > maxHeight)
		_bounds.top = maxHeight;
	int bottom = _bounds.top + height;
	if (bottom < 0)
		bottom = 0;
	if (bottom > maxHeight)
		bottom = maxHeight;

	_bounds.bottom = bottom;
	_active = true;
}

void DirtyArea::setSpriteSlot(const SpriteSlot *spriteSlot) {
	int width, height;
	Scene &scene = _vm->_game->_scene;

	if (spriteSlot->_flags == IMG_REFRESH) {
		// Special entry to refresh the entire screen
		_bounds.left = 0;
		_bounds.top = 0;
		width = MADS_SCREEN_WIDTH;
		height = MADS_SCENE_HEIGHT;
	} else {
		// Standard sprite slots
		_bounds.left = spriteSlot->_position.x - scene._posAdjust.x;
		_bounds.top = spriteSlot->_position.y - scene._posAdjust.y;

		SpriteAsset &spriteSet = *scene._sprites[spriteSlot->_spritesIndex];
		MSprite *frame = spriteSet.getFrame(ABS(spriteSlot->_frameNumber) - 1);

		if (spriteSlot->_scale == -1) {
			width = frame->w;
			height = frame->h;
		} else {
			width = frame->w * spriteSlot->_scale / 100;
			height = frame->h * spriteSlot->_scale / 100;

			_bounds.left -= width / 2;
			_bounds.top += -(height - 1);
		}
	}

	setArea(width, height, MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
}

void DirtyArea::setTextDisplay(const TextDisplay *textDisplay) {
	_bounds.left = textDisplay->_bounds.left;
	_bounds.top = textDisplay->_bounds.top;

	setArea(textDisplay->_bounds.width(), textDisplay->_bounds.height(),
		MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
}

void DirtyArea::setUISlot(const UISlot *slot) {
	int type = slot->_flags;
	if (type <= IMG_UPDATE_ONLY)
		type += -IMG_UPDATE_ONLY;
	if (type >= 0x40)
		type &= ~0x40;

	MSurface &intSurface = _vm->_game->_scene._userInterface;
	switch (type) {
	case IMG_REFRESH:
		_bounds.left = 0;
		_bounds.top = 0;
		setArea(intSurface.w, intSurface.h, intSurface.w, intSurface.h);
		break;

	case IMG_OVERPRINT:
		_bounds.left = slot->_position.x;
		_bounds.top = slot->_position.y;
		_bounds.setWidth(slot->_width);
		_bounds.setHeight(slot->_height);
		setArea(slot->_width, slot->_height, intSurface.w, intSurface.h);
		break;

	default: {
		SpriteAsset *asset = _vm->_game->_scene._sprites[slot->_spritesIndex];
		MSprite *frame = asset->getFrame(slot->_frameNumber - 1);
		int w = frame->w;
		int h = frame->h;

		if (slot->_segmentId == IMG_SPINNING_OBJECT) {
			_bounds.left = slot->_position.x;
			_bounds.top = slot->_position.y;
		} else {
			_bounds.left = slot->_position.x + w / 2;
			_bounds.top = slot->_position.y - h + 1;
		}

		setArea(w, h, intSurface.w, intSurface.h);
		break;
	}
	}
}

/*------------------------------------------------------------------------*/

DirtyAreas::DirtyAreas(MADSEngine *vm) /* : _vm(vm) */ {
	DirtyArea::_vm = vm;

	for (int i = 0; i < DIRTY_AREAS_SIZE; ++i) {
		DirtyArea rec;
		rec._active = false;
		push_back(rec);
	}
}

void DirtyAreas::merge(int startIndex, int count) {
	if (startIndex >= count)
		return;

	for (int outerCtr = startIndex - 1, idx = 0; idx < count; ++outerCtr, ++idx) {
		if (!(*this)[outerCtr]._active)
			continue;

		for (int innerCtr = outerCtr + 1; innerCtr < count; ++innerCtr) {
			if (!(*this)[innerCtr]._active || !intersects(outerCtr, innerCtr))
				continue;

			if ((*this)[outerCtr]._textActive && (*this)[innerCtr]._textActive)
				mergeAreas(innerCtr, outerCtr);
		}
	}
}

/**
* Returns true if two dirty areas intersect
*/
bool DirtyAreas::intersects(int idx1, int idx2) {
	return (*this)[idx1]._bounds.intersects((*this)[idx2]._bounds);
}

void DirtyAreas::mergeAreas(int idx1, int idx2) {
	DirtyArea &da1 = (*this)[idx1];
	DirtyArea &da2 = (*this)[idx2];

	da1._bounds.extend(da2._bounds);

	da2._active = false;
	da2._mergedArea = &da1;
	da1._textActive = true;
}

void DirtyAreas::copy(BaseSurface *srcSurface, BaseSurface *destSurface, const Common::Point &posAdjust) {
	for (uint i = 0; i < size(); ++i) {
		const Common::Rect &srcBounds = (*this)[i]._bounds;

		// Check if this is a sane rectangle before attempting to create it
		if (srcBounds.left >= srcBounds.right || srcBounds.top >= srcBounds.bottom)
			continue;

		Common::Rect bounds(srcBounds.left + posAdjust.x, srcBounds.top + posAdjust.y,
			srcBounds.right + posAdjust.x, srcBounds.bottom + posAdjust.y);
		Common::Point destPos(srcBounds.left, srcBounds.top);

		if ((*this)[i]._active && bounds.isValidRect()) {
			destSurface->blitFrom(*srcSurface, bounds, destPos);
		}
	}
}

void DirtyAreas::copyToScreen() {
/*
	for (uint i = 0; i < size(); ++i) {
		const Common::Rect &bounds = (*this)[i]._bounds;

		// Check if this is a sane rectangle before attempting to create it
		if (bounds.left >= bounds.right || bounds.top >= bounds.bottom)
			continue;

		if ((*this)[i]._active && (*this)[i]._bounds.isValidRect()) {
			_vm->_screen->copyRectToScreen(bounds);
		}
	}
	*/
}

void DirtyAreas::reset() {
	for (uint i = 0; i < size(); ++i)
		(*this)[i]._active = false;
}

/*------------------------------------------------------------------------*/

ScreenObject::ScreenObject() {
	_category = CAT_NONE;
	_descId = 0;
	_mode = 0;
	_active = false;
}

/*------------------------------------------------------------------------*/

ScreenObjects::ScreenObjects(MADSEngine *vm) : _vm(vm) {
	_objectY = -1;
	_forceRescan = false;
	_inputMode = kInputBuildingSentences;
	_v7FED6 = 0;
	_v8332A = 0;
	_category = CAT_NONE;
	_spotId = 0;
	_released = false;
	_uiCount = 0;
	_selectedObject = -1;
	_eventFlag = false;
	_baseTime = 0;
}

ScreenObject *ScreenObjects::add(const Common::Rect &bounds, ScreenMode mode, ScrCategory category, int descId) {
	ScreenObject so;
	so._bounds = bounds;
	so._category = category;
	so._descId = descId;
	so._mode = mode;
	so._active = true;

	push_back(so);

	return &(*this)[size()];
}

void ScreenObjects::check(bool scanFlag) {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;

	if (!_vm->_events->_mouseButtons || _inputMode != kInputBuildingSentences)
		_vm->_events->_rightMousePressed = false;

	if ((_vm->_events->_mouseMoved || userInterface._scrollbarActive
			|| _v8332A || _forceRescan) && scanFlag) {
		_category = CAT_NONE;
		_selectedObject = scanBackwards(_vm->_events->currentPos(), SCREENMODE_VGA);
		if (_selectedObject > 0) {
			ScreenObject &scrObject = (*this)[_selectedObject];
			_category = (ScrCategory)(scrObject._category & 7);
			_spotId = scrObject._descId;
		}

		// Handling for easy mouse
		ScrCategory category = scene._userInterface._category;
		if (_vm->_easyMouse && _vm->_events->_mouseButtons && category != _category
			&& scene._userInterface._category != CAT_NONE) {
			_released = true;
			if (category >= CAT_COMMAND && category <= CAT_TALK_ENTRY) {
				elementHighlighted();
			}

			scene._action.checkActionAtMousePos();
		}

		//_released = _vm->_events->_mouseReleased;
		if (_vm->_events->_mouseButtons || (_vm->_easyMouse && !_vm->_events->_mouseStatusCopy))
			scene._userInterface._category = _category;

		if (_vm->_events->_mouseButtons || _vm->_easyMouse) {
			if (userInterface._category >= CAT_COMMAND && userInterface._category <= CAT_TALK_ENTRY) {
				elementHighlighted();
			}
		}

		if (_vm->_events->_mouseButtons || (_vm->_easyMouse && scene._action._interAwaiting > AWAITING_COMMAND
			&& scene._userInterface._category == CAT_INV_LIST) ||
			(_vm->_easyMouse && scene._userInterface._category == CAT_HOTSPOT)) {
			scene._action.checkActionAtMousePos();
		}

		if (_vm->_events->_mouseReleased) {
			scene._action.leftClick();
			scene._userInterface._category = CAT_NONE;
		}

		if (_vm->_events->_mouseButtons || _vm->_easyMouse || userInterface._scrollbarActive)
			scene._userInterface.updateInventoryScroller();

		if (_vm->_events->_mouseButtons || _vm->_easyMouse)
			scene._action.set();

		_forceRescan = false;
	}

	scene._action.refresh();

	uint32 currentTicks = _vm->_events->getFrameCounter();
	if (currentTicks >= _baseTime) {
		// Check the user interface slots to see if there's any slots that need to be expired
		UISlots &uiSlots = userInterface._uiSlots;
		for (uint idx = 0; idx <  uiSlots.size(); ++idx) {
			UISlot &slot = uiSlots[idx];

			if (slot._flags != IMG_REFRESH && slot._flags > IMG_UPDATE_ONLY
				&& slot._segmentId != IMG_SPINNING_OBJECT)
					slot._flags = IMG_ERASE;
		}

		// Any background animation in the user interface
		userInterface.doBackgroundAnimation();

		// Handle animating the selected inventory item
		userInterface.inventoryAnim();

		// Set the base time
		_baseTime = currentTicks + 6;
	}
}

int ScreenObjects::scan(const Common::Point &pt, int layer) {
	for (uint i = 1; i <= size(); ++i) {
		ScreenObject &sObj = (*this)[i];
		if (sObj._active && sObj._bounds.contains(pt) && sObj._mode == layer)
			return i;
	}

	// Entry not found
	return 0;
}

int ScreenObjects::scanBackwards(const Common::Point &pt, int layer) {
	for (int i = (int)size(); i >= 1; --i) {
		ScreenObject &sObj = (*this)[i];
		if (sObj._active && sObj._bounds.contains(pt) && sObj._mode == layer)
			return i;
	}

	// Entry not found
	return 0;
}

void ScreenObjects::elementHighlighted() {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;
	Common::Array<int> &invList = _vm->_game->_objects._inventoryList;
	MADSAction &action = scene._action;
	int varA;
	int topIndex;
	int *idxP;
	int var4;
	int index;
	int indexEnd = -1;
	int var8 = 0;
	int uiCount;

	switch (userInterface._category) {
	case CAT_COMMAND:
		index = 10;
		indexEnd = 9;
		varA = 5;
		topIndex = 0;
		idxP = !_vm->_events->_rightMousePressed ? &userInterface._highlightedCommandIndex :
			&userInterface._selectedActionIndex;

		if (_vm->_events->_rightMousePressed && userInterface._selectedItemVocabIdx >= 0)
			userInterface.updateSelection(CAT_INV_VOCAB, -1, &userInterface._selectedItemVocabIdx);

		var4 = _released && !_vm->_events->_rightMousePressed ? 1 : 0;
		break;

	case CAT_INV_LIST:
		userInterface.scrollInventory();

		index = MIN((int)invList.size() - userInterface._inventoryTopIndex, 5);
		indexEnd = invList.size() - 1;
		varA = 0;
		topIndex = userInterface._inventoryTopIndex;
		idxP = &userInterface._highlightedInvIndex;
		var4 = (!_released || (_vm->_events->_mouseButtons && action._interAwaiting == 1)) ? 0 : 1;
		break;

	case CAT_INV_VOCAB:
		if (userInterface._selectedInvIndex >= 0) {
			InventoryObject &invObject = _vm->_game->_objects.getItem(
				userInterface._selectedInvIndex);
			index = invObject._vocabCount;
			indexEnd = index - 1;
		} else {
			index = 0;
		}

		varA = 0;
		topIndex = 0;
		idxP = _vm->_events->_rightMousePressed ? &userInterface._selectedItemVocabIdx : &userInterface._highlightedItemVocabIndex;

		if (_vm->_events->_rightMousePressed && userInterface._selectedActionIndex >= 0)
			userInterface.updateSelection(CAT_COMMAND, -1, &userInterface._selectedActionIndex);

		var4 = _released && !_vm->_events->_rightMousePressed ? 1 : 0;
		break;

	case CAT_INV_ANIM:
		index = 1;
		indexEnd = invList.size() - 1;
		varA = 0;
		topIndex = userInterface._selectedInvIndex;
		idxP = &var8;
		var4 = -1;
		break;

	case CAT_TALK_ENTRY:
		index = userInterface._talkStrings.size();
		indexEnd = index - 1;
		varA = 0;
		topIndex = 0;
		idxP = &userInterface._highlightedCommandIndex;
		var4 = -1;
		break;

	default:
		uiCount = size() - _uiCount;
		index = uiCount + scene._hotspots.size();
		indexEnd = index - 1;
		varA = 0;
		topIndex = 0;
		idxP = &var8;
		var4 = -1;
		break;
	}

	int newIndex = -1;
	int catIndex = userInterface._categoryIndexes[userInterface._category - 1];
	int newX = 0, newY = 0;
	Common::Point currentPos = _vm->_events->currentPos();

	for (int idx = 0; idx < index && newIndex < 0; ++idx) {
		int scrObjIndex = (_category == CAT_HOTSPOT) ? catIndex - idx + index - 1 :
			catIndex + idx;

		ScreenObject &scrObject = (*this)[scrObjIndex];
		if (!scrObject._active)
			continue;

		const Common::Rect &bounds = scrObject._bounds;
		newY = MAX((int)bounds.bottom, newY);
		newX = MAX((int)bounds.left, newX);

		if (currentPos.y >= bounds.top && currentPos.y < bounds.bottom) {
			if (var4) {
				if (currentPos.x >= bounds.left && currentPos.x < bounds.right) {
					// Cursor is inside hotspot bounds
					newIndex = scrObjIndex - catIndex;
					if (_category == CAT_HOTSPOT && newIndex < (int)scene._hotspots.size())
						newIndex = scene._hotspots.size() - newIndex - 1;
				}
			} else if (!varA) {
				newIndex = idx;
			} else if (varA <= idx) {
				if (currentPos.x > bounds.left)
					newIndex = idx;
			} else {
				if (currentPos.x < bounds.right)
					newIndex = idx;
			}
		}
	}

	if (newIndex == -1 && index > 0 && !var4) {
		if (_vm->_events->currentPos().y <= newY) {
			newIndex = 0;
			if (varA && _vm->_events->currentPos().x >= newX)
				newIndex = varA;
		} else {
			newIndex = index - 1;
		}
	}

	if (newIndex >= 0)
		newIndex = MIN(newIndex + topIndex, indexEnd);

	action._pickedWord = newIndex;

	if (_category == CAT_INV_LIST || _category == CAT_INV_ANIM) {
		if (action._interAwaiting == AWAITING_COMMAND && newIndex >= 0 && _released &&
				(!_vm->_events->_mouseReleased || !_vm->_easyMouse))
			newIndex = -1;
	}

	if (_released && !_vm->_events->_rightMousePressed &&
			(_vm->_events->_mouseReleased || !_vm->_easyMouse))
		newIndex = -1;

	if (_category != CAT_HOTSPOT && _category != CAT_INV_ANIM)
		userInterface.updateSelection(_category, newIndex, idxP);
}

void ScreenObjects::setActive(ScrCategory category, int descId, bool active) {
	for (uint idx = 1; idx <= size(); ++idx) {
		ScreenObject &sObj = (*this)[idx];
		if (sObj._category == category && sObj._descId == descId)
			sObj._active = active;
	}
}

void ScreenObjects::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_selectedObject);
	s.syncAsSint16LE(_category);
}

/*------------------------------------------------------------------------*/

Screen::Screen(): BaseSurface() {
	// Create the screen surface separately on another surface, since the screen
	// surface will be subject to change as the clipping area is altered
	_rawSurface.create(MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT);
	resetClipBounds();

	_shakeCountdown = -1;
	_random = 0x4D2;
}

void Screen::update() {
	if (_shakeCountdown >= 0) {
		_random = _random * 5 + 1;
		int offset = (_random >> 8) & 3;
		if (_shakeCountdown-- <= 0)
			offset = 0;

		// Copy the screen with the left hand hide side of the screen of a given
		// offset width shown at the very right. The offset changes to give
		// an effect of shaking the screen
		offset *= 4;
		const byte *buf = (const byte *)getBasePtr(offset, 0);
		g_system->copyRectToScreen(buf, this->pitch, 0, 0, this->pitch - offset, this->h);
		if (offset > 0)
			g_system->copyRectToScreen(getPixels(), this->pitch,
				this->pitch - offset, 0, offset, this->h);
		return;
	}

	// Reset any clip bounds if active whilst the screen is updated
	Common::Rect clipBounds = getClipBounds();
	resetClipBounds();

	// Update the screen
	Graphics::Screen::update();

	// Revert back to whatever clipping is active
	setClipBounds(clipBounds);
}

void Screen::transition(ScreenTransition transitionType, bool surfaceFlag) {
	Palette &pal = *_vm->_palette;
	Scene &scene = _vm->_game->_scene;
	byte palData[PALETTE_SIZE];

	// The original loads the new scene to the screen surface for some of the
	// transition types like fade out/in, so we need to clear the dirty rects so
	// it doesn't prematurely get blitted to the physical screen before fade out
	Common::Rect clipBounds = getClipBounds();
	clearDirtyRects();

 	switch (transitionType) {
	case kTransitionFadeIn:
	case kTransitionFadeOutIn: {
		Common::fill(&pal._colorValues[0], &pal._colorValues[3], 0);
		Common::fill(&pal._colorFlags[0], &pal._colorFlags[3], false);
		resetClipBounds();

		if (transitionType == kTransitionFadeOutIn) {
			// Fade out
			pal.getFullPalette(palData);
			pal.fadeOut(palData, nullptr, 0, PALETTE_COUNT, 0, 0, 1, 16);
		}

		// Reset palette to black
		Common::fill(&palData[0], &palData[PALETTE_SIZE], 0);
		pal.setFullPalette(palData);

		markAllDirty();
		update();
		pal.fadeIn(palData, pal._mainPalette, 0, 256, 0, 1, 1, 16);
		break;
	}

	case kTransitionBoxInBottomLeft:
	case kTransitionBoxInBottomRight:
	case kTransitionBoxInTopLeft:
	case kTransitionBoxInTopRight:
		warning("TODO: box transition");
		transition(kTransitionFadeIn, surfaceFlag);
		break;

	case kTransitionPanLeftToRight:
	case kTransitionPanRightToLeft:
		panTransition(scene._backgroundSurface, pal._mainPalette,
			transitionType - kTransitionPanLeftToRight,
			Common::Point(0, 0), scene._posAdjust, THROUGH_BLACK2, true, 1);
		break;

	case kTransitionCircleIn1:
	case kTransitionCircleIn2:
	case kTransitionCircleIn3:
	case kTransitionCircleIn4:
		warning("TODO circle transition");
		transition(kTransitionFadeIn, surfaceFlag);
		break;

	case kNullPaletteCopy:
		// Original temporarily set the palette to black, copied the scene to the
		// screen, and then restored the palette. We can give a similiar effect
		// by doing a standard quick palette fade in
		transition(kTransitionFadeIn, surfaceFlag);
		break;

	default:
		// Quick transitions
		break;
	}

	// Reset clipping
	markAllDirty();
	setClipBounds(clipBounds);
}

void Screen::panTransition(MSurface &newScreen, byte *palData, int entrySide,
		const Common::Point &srcPos, const Common::Point &destPos,
		ThroughBlack throughBlack, bool setPalette_, int numTicks) {
	EventsManager &events = *_vm->_events;
	Palette &palette = *_vm->_palette;
	Common::Point size;
	int y1, y2;
	int startX = 0;
	int deltaX;
	int loopStart;
//	uint32 baseTicks, currentTicks;
	byte paletteMap[256];

	size.x = MIN(newScreen.w, (uint16)MADS_SCREEN_WIDTH);
	size.y = newScreen.h;
	if (newScreen.h >= MADS_SCREEN_HEIGHT)
		size.y = MADS_SCENE_HEIGHT;

	// Set starting position and direction delta for the transition
	if (entrySide == 1)
		// Right to left
		startX = size.x - 1;
	deltaX = startX ? -1 : 1;

	if (setPalette_ & !throughBlack)
		palette.setFullPalette(palData);

	// TODO: Original uses a different frequency ticks counter. Need to
	// confirm frequency and see whether we need to implement it, or
	// if the current frame ticks can substitute for it
//	baseTicks = events.getFrameCounter();

	y1 = 0;
	y2 = size.y - 1;
//	sizeY = y2 - y1 + 1;

	if (throughBlack == THROUGH_BLACK2)
		swapForeground(palData, &paletteMap[0]);

	loopStart = throughBlack == THROUGH_BLACK1 ? 0 : 1;
	for (int loop = loopStart; loop < 2; ++loop) {
		int xAt = startX;
		for (int xCtr = 0; xCtr < size.x; ++xCtr, xAt += deltaX) {
			if (!loop) {
				fillRect(Common::Rect(xAt + destPos.x, y1 + destPos.y,
					xAt + destPos.x + 1, y2 + destPos.y), 0);
			} else if (throughBlack == THROUGH_BLACK2) {
				copyRectTranslate(newScreen, paletteMap,
					Common::Point(xAt, destPos.y),
					Common::Rect(srcPos.x + xAt, srcPos.y,
					srcPos.x + xAt + 1, srcPos.y + size.y));
			} else {
				newScreen.copyRectToSurface(*this, xAt, destPos.y,
					Common::Rect(srcPos.x + xAt, srcPos.y,
					srcPos.x + xAt + 1, srcPos.y + size.y));
			}

			// Slight delay
			events.pollEvents();
			g_system->delayMillis(1);
		}

		if ((setPalette_ && !loop) || throughBlack == THROUGH_BLACK2)
			palette.setFullPalette(palData);
	}

	if (throughBlack == THROUGH_BLACK2) {
		/*
		Common::Rect r(srcPos.x, srcPos.y, srcPos.x + size.x, srcPos.y + size.y);
		copyRectToSurface(newScreen, destPos.x, destPos.y, r);
		copyRectToScreen(r);
		*/
	}
}

/**
 * Translates the current screen from the old palette to the new palette
 */
void Screen::swapForeground(byte newPalette[PALETTE_SIZE], byte *paletteMap) {
	Palette &palette = *_vm->_palette;
	byte oldPalette[PALETTE_SIZE];
	byte oldMap[PALETTE_COUNT];

	palette.getFullPalette(oldPalette);
	swapPalette(oldPalette, oldMap, true);
	swapPalette(newPalette, paletteMap, false);

	// Transfer translated foreground colors. Since foregrounds are interleaved
	// with background, we only copy over each alternate RGB tuplet
	const byte *srcP = &newPalette[RGB_SIZE];
	byte *destP = &oldPalette[RGB_SIZE];
	while (destP < &oldPalette[PALETTE_SIZE]) {
		Common::copy(srcP, srcP + RGB_SIZE, destP);
		srcP += 2 * RGB_SIZE;
		destP += 2 * RGB_SIZE;
	}

	Common::Rect oldClip = getClipBounds();
	resetClipBounds();

	copyRectTranslate(*this, oldMap, Common::Point(0, 0),
		Common::Rect(0, 0, MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT));
	palette.setFullPalette(oldPalette);

	setClipBounds(oldClip);
}

/**
 * Translates a given palette into a mapping table.
 * Palettes consist of 128 RGB entries for the foreground and background
 * respectively, with the two interleaved together. So the start
 */
void Screen::swapPalette(const byte *palData, byte swapTable[PALETTE_COUNT],
		bool foreground) {
	int start = foreground ? 1 : 0;
	const byte *dynamicList = &palData[start * RGB_SIZE];
	int staticStart = 1 - start;
	const byte *staticList = &palData[staticStart * RGB_SIZE];
	const int PALETTE_START = 1;
	const int PALETTE_END = 252;

	// Set initial index values
	for (int idx = 0; idx < PALETTE_COUNT; ++idx)
		swapTable[idx] = idx;

	// Handle the 128 palette entries for the foreground or background
	for (int idx = 0; idx < (PALETTE_COUNT / 2); ++idx) {
		if (start >= PALETTE_START && start <= PALETTE_END) {
			swapTable[start] = Palette::closestColor(dynamicList, staticList,
				2 * RGB_SIZE, PALETTE_COUNT / 2) * 2 + staticStart;
		}

		dynamicList += 2 * RGB_SIZE;
		start += 2;
	}
}

void Screen::setClipBounds(const Common::Rect &r) {
	create(_rawSurface, r);
}

void Screen::resetClipBounds() {
	setClipBounds(Common::Rect(0, 0, MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT));
}

} // End of namespace MADS
