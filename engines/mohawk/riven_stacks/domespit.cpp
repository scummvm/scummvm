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

#include "mohawk/riven_stacks/domespit.h"

#include "mohawk/cursors.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_video.h"

namespace Mohawk {
namespace RivenStacks {

static const uint32 kDomeSliderDefaultState = 0x01F00000;
static const uint32 kDomeSliderSlotCount = 25;

DomeSpit::DomeSpit(MohawkEngine_Riven *vm, uint16 id, const char *sliderBmpName, const char *sliderBgBmpName) :
		RivenStack(vm, id),
		_sliderBmpName(sliderBmpName),
		_sliderBgBmpName(sliderBgBmpName) {
	_sliderState = kDomeSliderDefaultState;
}

void DomeSpit::runDomeButtonMovie() {
	// This command just plays the video of the button moving down and up.
	// The original displayed images of the button going down
	RivenVideo *video = _vm->_video->openSlot(2);
	video->playBlocking();
}

void DomeSpit::runDomeCheck() {
	// Check if we clicked while the golden frame was showing

	const RivenVideo *video = _vm->_video->getSlot(1);
	assert(video);

	int32 curFrame = video->getCurFrame();
	int32 frameCount = video->getFrameCount();

	// The final frame of the video is the 'golden' frame (double meaning: the
	// frame that is the magic one is the one with the golden symbol) but we
	// give a 3 frame leeway in either direction.
	if (frameCount - curFrame < 3 || curFrame < 3)
		_vm->_vars["domecheck"] = 1;
}

void DomeSpit::resetDomeSliders(uint16 startHotspot) {
	// The rightmost slider should move left until it finds the next slider,
	// then those two continue until they find the third slider. This continues
	// until all five sliders have returned their starting slots.
	byte slidersFound = 0;
	for (uint32 i = 0; i < kDomeSliderSlotCount; i++) {
		if (_sliderState & (1 << i)) {
			// A slider occupies this spot. Increase the number of sliders we
			// have found, but we're not doing any moving this iteration.
			slidersFound++;
		} else {
			// Move all the sliders we have found over one slot
			for (byte j = 0; j < slidersFound; j++) {
				_sliderState &= ~(1 << (i - j - 1));
				_sliderState |= 1 << (i - j);
			}

			// If we have at least one found slider, it has now moved
			// so we should redraw and play a tick sound
			if (slidersFound) {
				_vm->_sound->playCardSound("aBigTic");
				drawDomeSliders(startHotspot);
				_vm->delay(20);
			}
		}
	}

	// Sanity checks - the slider count should always be 5 and we should end up at
	// the default state after moving them all over.
	assert(slidersFound == 5);
	assert(_sliderState == kDomeSliderDefaultState);
}

void DomeSpit::checkDomeSliders() {
	RivenHotspot *resetSlidersHotspot = _vm->getCard()->getHotspotByName("ResetSliders");
	RivenHotspot *openDomeHotspot = _vm->getCard()->getHotspotByName("OpenDome");

	// Let's see if we're all matched up...
	if (_vm->_vars["adomecombo"] == _sliderState) {
		// Set the button hotspot to the open dome hotspot
		resetSlidersHotspot->enable(false);
		openDomeHotspot->enable(true);
	} else {
		// Set the button hotspot to the reset sliders hotspot
		resetSlidersHotspot->enable(true);
		openDomeHotspot->enable(false);
	}
}

void DomeSpit::checkSliderCursorChange(uint16 startHotspot) {
	// Set the cursor based on _sliderState and what hotspot we're over
	int16 sliderSlot = getSliderSlotClosestToPos(startHotspot, getMousePosition());

	if (sliderSlot >= 0 && isSliderAtSlot(sliderSlot)) {
		_vm->_cursor->setCursor(kRivenOpenHandCursor);
	} else {
		_vm->_cursor->setCursor(kRivenMainCursor);
	}
}

int16 DomeSpit::getSliderSlotClosestToPos(uint16 startHotspot, const Common::Point &pos) const {
	// Emperically found min x and max x hotspot are used to bound mouse position into
	// the slider area vertically. This and the y mouse position being directly put into
	// the slider area allows the user to move the cursor out of the slider area and still
	// be able to move the slider.
	int16 minXHotspot = 211; // suitable min x value hotspot for all domes
	int16 maxXHotspot = 407; // suitable max x value hotspot for all domes

	// Find the slider slot closest to pos. This is not necessarily the slider being moved.
	for (uint16 i = 0; i < kDomeSliderSlotCount; i++) {
		RivenHotspot *hotspot = _vm->getCard()->getHotspotByBlstId(startHotspot + i);
		Common::Rect srcRect = hotspot->getRect();
		// Only the x value of mouse position being in the hotspot matters
		// the y value of srcRect.top is chosen because it is in the rect.
		Common::Point posBounded(pos.x, srcRect.top); 
		// Now clip the x value so it lies in the x extremes of the slider hotspots.
		// If this is not done then the user can move the x position past the
		// slider area and the slider won't go all the way to that end.
		posBounded.x = CLIP<int16>(posBounded.x, minXHotspot, maxXHotspot - 1);
		if (hotspot->containsPoint(posBounded)) {
			return i;
		}
	}

	return -1;
}

bool DomeSpit::isSliderAtSlot(int16 slot) const {
	return _sliderState & (1 << (24 - slot));
}

void DomeSpit::dragDomeSlider(uint16 startHotspot) {
	int16 draggedSliderSlot = getSliderSlotClosestToPos(startHotspot, getMousePosition());

	// We're not over any slider
	if (draggedSliderSlot < 0 || !isSliderAtSlot(draggedSliderSlot)) {
		return;
	}

	// We've clicked down, so show the closed hand cursor
	_vm->_cursor->setCursor(kRivenClosedHandCursor);

	while (mouseIsDown() && !_vm->hasGameEnded()) {
		int16 hoveredHotspot = getSliderSlotClosestToPos(startHotspot, getMousePosition());
		if (hoveredHotspot >= 0) {
			if (hoveredHotspot > draggedSliderSlot && draggedSliderSlot < 24 && !isSliderAtSlot(draggedSliderSlot + 1)) {
				// We've moved the slider right one space
				_sliderState &= ~(_sliderState & (1 << (24 - draggedSliderSlot)));
				draggedSliderSlot++;
				_sliderState |= 1 << (24 - draggedSliderSlot);

				// Now play a click sound and redraw
				_vm->_sound->playCardSound("aBigTic");
				drawDomeSliders(startHotspot);
			}
			if (hoveredHotspot < draggedSliderSlot && draggedSliderSlot > 0 && !isSliderAtSlot(draggedSliderSlot - 1)) {
				// We've moved the slider left one space
				_sliderState &= ~(_sliderState & (1 << (24 - draggedSliderSlot)));
				draggedSliderSlot--;
				_sliderState |= 1 << (24 - draggedSliderSlot);

				// Now play a click sound and redraw
				_vm->_sound->playCardSound("aBigTic");
				drawDomeSliders(startHotspot);
			}
		}

		_vm->doFrame();
	}

	// Check to see if we have the right combination
	checkDomeSliders();
}

void DomeSpit::drawDomeSliders(uint16 startHotspot) {
	Common::Rect dstAreaRect = Common::Rect(200, 250, 420, 319);

	// On pspit, the rect is different by two pixels
	// (alternatively, we could just use hotspot 3 here, but only on pspit is there a hotspot for this)
	if (_vm->getStack()->getId() == kStackPspit)
		dstAreaRect.translate(-2, 0);

	// Find out bitmap id
	uint16 bitmapId = _vm->findResourceID(ID_TBMP, buildCardResourceName(_sliderBmpName));
	uint16 bgBitmapId = _vm->findResourceID(ID_TBMP, buildCardResourceName(_sliderBgBmpName));

	_vm->_gfx->beginScreenUpdate();

	for (uint16 i = 0; i < kDomeSliderSlotCount; i++) {
		RivenHotspot *hotspot = _vm->getCard()->getHotspotByBlstId(startHotspot + i);

		Common::Rect srcRect = hotspot->getRect();
		srcRect.translate(-dstAreaRect.left, -dstAreaRect.top); // Adjust the rect so it's in the destination area

		Common::Rect dstRect = hotspot->getRect();

		if (_sliderState & (1 << (24 - i)))
			_vm->_gfx->drawImageRect(bitmapId, srcRect, dstRect);
		else
			_vm->_gfx->drawImageRect(bgBitmapId, srcRect, dstRect);
	}

	_vm->_gfx->applyScreenUpdate();
}

Common::String DomeSpit::buildCardResourceName(const Common::String &name) const {
	return Common::String::format("%d_%s", _vm->getCard()->getId(), name.c_str());
}

void DomeSpit::setDomeSliderState(uint32 sliderState) {
	_sliderState = sliderState;
}

uint32 DomeSpit::getDomeSliderState() const {
	return _sliderState;
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
