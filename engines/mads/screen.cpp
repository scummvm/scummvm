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
	_bounds2.left = _bounds.width() / 2;
	_bounds2.right = _bounds.left + (_bounds.width() + 1) / 2 - 1;

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
	_bounds2.top = _bounds.height() / 2;
	_bounds2.bottom = _bounds.top + (_bounds.height() + 1) / 2 - 1;

	_active = true;
}


void DirtyArea::setSpriteSlot(const SpriteSlot *spriteSlot) {
	int width, height;
	Scene &scene = _vm->_game->_scene;

	if (spriteSlot->_SlotType == ST_FULL_SCREEN_REFRESH) {
		// Special entry to refresh the entire screen
		_bounds.left = 0;
		_bounds.top = 0;
		width = MADS_SCREEN_WIDTH;
		height = MADS_SCENE_HEIGHT;
	}
	else {
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
	int type = slot->_slotType;
	if (type <= -20)
		type += 20;
	if (type >= 0x40)
		type &= ~0x40;

	MSurface &intSurface = _vm->_game->_scene._userInterface;
	switch (type) {
	case ST_FULL_SCREEN_REFRESH:
		_bounds.left = 0;
		_bounds.top = 0;
		setArea(intSurface.w, intSurface.h, intSurface.w, intSurface.h);
		break;
	case ST_MINUS3:
		_bounds.left = slot->_position.x;
		_bounds.top = slot->_position.y;
		// TODO: spritesIndex & frameNumber used as w & h??!
		error("TODO: Figure out ST_MINUS3. Maybe need a union?");
		break;

	default: {
		SpriteAsset *asset = _vm->_game->_scene._sprites[slot->_spritesIndex];
		MSprite *frame = asset->getFrame(slot->_frameNumber - 1);
		int w = frame->w;
		int h = frame->h;

		if (slot->_field2 == 200) {
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

DirtyAreas::DirtyAreas(MADSEngine *vm) : _vm(vm) {
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
	return (*this)[idx1]._bounds2.intersects((*this)[idx2]._bounds2);
}

void DirtyAreas::mergeAreas(int idx1, int idx2) {
	DirtyArea &da1 = (*this)[idx1];
	DirtyArea &da2 = (*this)[idx2];

	da1._bounds.extend(da2._bounds);

	da1._bounds2.left = da1._bounds.width() / 2;
	da1._bounds2.right = da1._bounds.left + (da1._bounds.width() + 1) / 2 - 1;
	da1._bounds2.top = da1._bounds.height() / 2;
	da1._bounds2.bottom = da1._bounds.top + (da1._bounds.height() + 1) / 2 - 1;

	da2._active = false;
	da1._textActive = true;
}

void DirtyAreas::copy(MSurface *srcSurface, MSurface *destSurface, const Common::Point &posAdjust) {
	for (uint i = 0; i < size(); ++i) {
		const Common::Rect &srcBounds = (*this)[i]._bounds;

		Common::Rect bounds(srcBounds.left + posAdjust.x, srcBounds.top + posAdjust.y,
			srcBounds.right + posAdjust.x, srcBounds.bottom + posAdjust.y);

		if ((*this)[i]._active && bounds.isValidRect()) {
			srcSurface->copyTo(destSurface, bounds, Common::Point(bounds.left, bounds.top));
		}
	}
}

void DirtyAreas::copyToScreen(const Common::Point &posAdjust) {
	for (uint i = 0; i < size(); ++i) {
		const Common::Rect &srcBounds = (*this)[i]._bounds;

		Common::Rect bounds(srcBounds.left + posAdjust.x, srcBounds.top + posAdjust.y,
			srcBounds.right + posAdjust.x, srcBounds.bottom + posAdjust.y);

		if ((*this)[i]._active && (*this)[i]._bounds.isValidRect()) {
			_vm->_screen.copyRectToScreen(bounds);
		}
	}
}

void DirtyAreas::reset() {
	for (uint i = 0; i < size(); ++i)
		(*this)[i]._active = false;
}

/*------------------------------------------------------------------------*/

ScreenObject::ScreenObject() {
	_category = CAT_NONE;
	_descId = 0;
	_layer = 0;
}

/*------------------------------------------------------------------------*/

ScreenObjects::ScreenObjects(MADSEngine *vm) : _vm(vm) {
	_objectY = -1;
	_v8333C = false;
	_v832EC = 0;
	_v7FECA = true;
	_v7FED6 = 0;
	_v8332A = 0;
	_category = CAT_NONE;
	_newDescId = 0;
	_newDescId = 0;
	_released = false;
	_uiCount = 0;
	_selectedObject = -1;
	_scrollerY = -1;
	_milliTime = 0;
	_eventFlag = false;
	_baseTime = 0;
}

void ScreenObjects::add(const Common::Rect &bounds, Layer layer, ScrCategory category, int descId) {
	assert(size() < 100);

	ScreenObject so;
	so._bounds = bounds;
	so._category = category;
	so._descId = descId;
	so._layer = layer;
	so._active = true;

	push_back(so);
}

void ScreenObjects::check(bool scanFlag) {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;

	if (!_vm->_events->_mouseButtons || _v832EC)
		_v7FECA = false;

	if ((_vm->_events->_mouseMoved || _vm->_game->_scene._userInterface._scrollerY 
			|| _v8332A || _v8333C) && scanFlag) {
		_category = CAT_NONE;
		_selectedObject = scanBackwards(_vm->_events->currentPos(), LAYER_GUI);
		if (_selectedObject > 0) {
			ScreenObject &scrObject = (*this)[_selectedObject];
			_category = (ScrCategory)(scrObject._category & 7);
			_newDescId = scrObject._descId;
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
		if (_vm->_events->_vD2 || (_vm->_easyMouse && !_vm->_events->_vD4))
			scene._userInterface._category = _category;

		if (!_vm->_events->_mouseButtons || _vm->_easyMouse) {
			if (userInterface._category >= CAT_COMMAND && userInterface._category <= CAT_TALK_ENTRY) {
				elementHighlighted();
			}
		}

		if (_vm->_events->_mouseButtons || (_vm->_easyMouse && scene._action._interAwaiting > 1
			&& scene._userInterface._category == CAT_INV_LIST) ||
			(_vm->_easyMouse && scene._userInterface._category == CAT_HOTSPOT)) {
			scene._action.checkActionAtMousePos();
		}

		if (_vm->_events->_mouseReleased) {
			scene._action.leftClick();
			scene._userInterface._category = CAT_NONE;
		}

		if (_vm->_events->_mouseButtons || _vm->_easyMouse || scene._userInterface._scrollerY)
			checkScroller();

		if (_vm->_events->_mouseButtons || _vm->_easyMouse)
			scene._action.set();

		_v8333C = 0;
	}

	scene._action.refresh();

	uint32 currentTicks = _vm->_events->getFrameCounter();
	if (currentTicks >= _baseTime) {
		// Check the user interface slots to see if there's any slots that need to be expired
		UISlots &uiSlots = userInterface._uiSlots;
		for (uint idx = 0; idx <  uiSlots.size(); ++idx) {
			UISlot &slot = uiSlots[idx];

			if (slot._slotType != ST_FULL_SCREEN_REFRESH && slot._slotType > -20
					&& slot._field2 != 200)
				slot._slotType = ST_EXPIRED;
		}

		// TODO: The stuff here could probably be moved to Scene::doFrame
		// Any background animation
		scene.backgroundAnimation();

		// Handle animating the selected inventory animation
		userInterface.inventoryAnim();

		// Set the base time
		_baseTime = currentTicks + 6;
	}
}

int ScreenObjects::scanBackwards(const Common::Point &pt, int layer) {
	for (int i = (int)size(); i >= 1; --i) {
		ScreenObject &sObj = (*this)[i];
		if (sObj._active && sObj._bounds.contains(pt) && sObj._layer == layer)
			return i;
	}

	// Entry not found
	return 0;
}

void ScreenObjects::checkScroller() {
	UserInterface &userInterface = _vm->_game->_scene._userInterface;
	Common::Array<int> &inventoryList = _vm->_game->_objects._inventoryList;

	if (_v832EC)
		return;

	userInterface._scrollerY = 0;
	
	if ((_category == CAT_INV_SCROLLER || (_scrollerY == 3 && _vm->_events->_vD4))
			&& (_vm->_events->_vD4 || _vm->_easyMouse)) {
		if ((_vm->_events->_vD2 || (_vm->_easyMouse && !_vm->_events->_vD4))
				&& _category == CAT_INV_SCROLLER) {
			_currentDescId = _newDescId;
		}
	}

	if (_newDescId == _currentDescId || _scrollerY == 3) {
		_vm->_game->_scene._userInterface._scrollerY = _currentDescId;
		uint32 currentMilli = g_system->getMillis();
		uint32 timeInc = _eventFlag ? 100 : 380;
		
		if (_vm->_events->_vD2 || (_milliTime + timeInc) <= currentMilli) {
			_eventFlag = _vm->_events->_vD2 < 1;
			_milliTime = currentMilli;

			switch (_currentDescId) {
			case 1:
				// Scroll up
				if (userInterface._inventoryTopIndex > 0 && inventoryList.size() > 0) {
					--userInterface._inventoryTopIndex;
					userInterface._inventoryChanged = true;
				}
				break;

			case 2:
				// Scroll down
				if (userInterface._inventoryTopIndex < (int)inventoryList.size() &&
						inventoryList.size() > 0) {
					++userInterface._inventoryTopIndex;
					userInterface._inventoryChanged = true;
				}
				break;

			case 3: {
				// Inventory slider
				int newIndex = CLIP((int)_vm->_events->currentPos().y - 170, 0, 17)
					* inventoryList.size() / 10;
				if (newIndex >= (int)inventoryList.size())
					newIndex = inventoryList.size() - 1;

				if (inventoryList.size() > 0) {
					userInterface._inventoryChanged = newIndex != userInterface._inventoryTopIndex;
					userInterface._inventoryTopIndex = newIndex;
				}
				break;
			}

			default:
				break;
			}

			if (userInterface._inventoryChanged) {
				int dummy;
				userInterface.updateSelection(CAT_INV_LIST, 0, &dummy);
			}
		}
	}

	if (userInterface._scrollerY != _scrollerY ||
			userInterface._objectY != _objectY)
		userInterface.scrollerChanged();

	_scrollerY = userInterface._scrollerY;
	_objectY = userInterface._objectY;
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
		idxP = !_v7FECA ? &userInterface._highlightedCommandIndex : 
			&userInterface._selectedActionIndex;

		if (_v7FECA && userInterface._selectedItemVocabIdx >= 0)
			userInterface.updateSelection(CAT_INV_VOCAB, -1, &userInterface._selectedItemVocabIdx);

		var4 = _released && !_v7FECA ? 1 : 0;
		break;

	case CAT_INV_LIST:
		userInterface.scrollInventory();

		index = MIN((int)invList.size() - userInterface._inventoryTopIndex, 5);
		indexEnd = invList.size() - 1;
		varA = 0;
		topIndex = userInterface._inventoryTopIndex;
		idxP = &userInterface._highlightedItemIndex;
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
		idxP = _v7FECA ? &userInterface._selectedItemVocabIdx : &userInterface._highlightedActionIndex;

		if (_v7FECA && userInterface._selectedActionIndex >= 0)
			userInterface.updateSelection(CAT_COMMAND, -1, &userInterface._selectedActionIndex);

		var4 = _released && !_v7FECA ? 1 : 0;
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
		index = 0;
		for (int idx = 0; idx < 5; ++idx) {
			if (!userInterface._talkStrings[idx].empty())
				++index;
		}

		indexEnd = index - 1;
		varA = 0;
		topIndex = 0;
		idxP = &userInterface._highlightedCommandIndex;
		var4 = -1;
		break;

	default:
		uiCount = size() - _uiCount;
		index = scene._hotspots.size();
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

	_vm->_game->_scene._highlightedHotspot = newIndex;

	if (_category == CAT_INV_LIST || _category == CAT_INV_ANIM) {
		if (action._interAwaiting == 1 && newIndex >= 0 && _released &&
				(!_vm->_events->_mouseReleased || !_vm->_easyMouse))
			newIndex = -1;
	}

	if (_released && !_vm->_game->_screenObjects._v7FECA &&
			(_vm->_events->_mouseReleased || !_vm->_easyMouse))
		newIndex = -1;

	if (_category != CAT_HOTSPOT && _category != CAT_INV_ANIM)
		userInterface.updateSelection(_category, newIndex, idxP);
}

void ScreenObjects::setActive(ScrCategory category, int descId, bool active) {
	for (uint idx = 1; idx < size(); ++idx) {
		ScreenObject &sObj = (*this)[idx];
		if (sObj._category == category && sObj._descId == descId)
			sObj._active = active;
	}
}

/*------------------------------------------------------------------------*/

ScreenSurface::ScreenSurface() {
	_dataP = nullptr;
}

void ScreenSurface::init() {
	setSize(g_system->getWidth(), g_system->getHeight());
}

void ScreenSurface::copyRectToScreen(const Common::Point &destPos,
		const Common::Rect &bounds) {
	byte *buf = getBasePtr(destPos.x, destPos.y);

	if (bounds.width() != 0 && bounds.height() != 0)
		g_system->copyRectToScreen(buf, this->pitch, bounds.left, bounds.top,
			bounds.width(), bounds.height());
}

void ScreenSurface::copyRectToScreen(const Common::Rect &bounds) {
	copyRectToScreen(Common::Point(bounds.left, bounds.top), bounds);
}


void ScreenSurface::updateScreen() {
	g_system->updateScreen();
}

void ScreenSurface::transition(ScreenTransition transitionType, bool surfaceFlag) {
	switch (transitionType) {
	case kTransitionFadeOutIn:
		fadeOut();
		fadeIn();
		break;

	case kTransitionFadeIn:
		fadeIn();
		break;

	case kTransitionBoxInBottomLeft:
	case kTransitionBoxInBottomRight:
	case kTransitionBoxInTopLeft:
	case kTransitionBoxInTopRight:
		error("TODO: transition");
		break;

	case kTransitionPanLeftToRight:
	case kTransitionPanRightToLeft:
		error("TODO: transition");

	case kTransitionCircleIn1:
	case kTransitionCircleIn2:
	case kTransitionCircleIn3:
	case kTransitionCircleIn4:
		error("TODO circle transition");

	case kCenterVertTransition:
		error("TODO: center vert transition");

	default:
		// Quick transitions
		break;
	}
}

void ScreenSurface::setPointer(MSurface *s) {
	_dataP = s->getData();
}

void ScreenSurface::fadeOut() {
	warning("TODO: Proper fade out");
}

void ScreenSurface::fadeIn() {
	warning("TODO: Proper fade in");
	_vm->_palette->setFullPalette(_vm->_palette->_mainPalette);
	_vm->_screen.copyRectToScreen(Common::Rect(0, 0, 320, 200));
}

} // End of namespace MADS
