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
		MSprite *frame = spriteSet.getFrame(((spriteSlot->_frameNumber & 0x7fff) - 1) & 0x7f);

		if (spriteSlot->_scale == -1) {
			width = frame->w;
			height = frame->h;
		}
		else {
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
	if (type >= 64)
		type &= 0xBF;

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
			_bounds.top = slot->_position.y + h / 2;
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
				mergeAreas(outerCtr, innerCtr);
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

ScreenSurface::ScreenSurface() {
	_dataP = nullptr;
}

void ScreenSurface::init() {
	setSize(g_system->getWidth(), g_system->getHeight());
}

void ScreenSurface::copyRectToScreen(const Common::Point &destPos,
		const Common::Rect &bounds) {
	byte *buf = getBasePtr(destPos.x, destPos.y);
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
