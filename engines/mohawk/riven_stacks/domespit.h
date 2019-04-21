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

#ifndef RIVEN_STACKS_DOMESPIT_H
#define RIVEN_STACKS_DOMESPIT_H

#include "mohawk/riven_stack.h"

namespace Mohawk {
namespace RivenStacks {

class DomeSpit : public RivenStack {
public:
	DomeSpit(MohawkEngine_Riven *vm, uint16 id, const char *sliderBmpName, const char *sliderBgBmpName);

	uint32 getDomeSliderState() const;
	void setDomeSliderState(uint32 sliderState);

protected:
	void runDomeCheck();
	void runDomeButtonMovie();
	void resetDomeSliders(uint16 startHotspot);
	void checkDomeSliders();
	void checkSliderCursorChange(uint16 startHotspot);
	void dragDomeSlider(uint16 startHotspot);
	void drawDomeSliders(uint16 startHotspot);
	int16 getSliderSlotClosestToPos(uint16 startHotspot, const Common::Point &pos) const;
	bool isSliderAtSlot(int16 slot) const;
	Common::String buildCardResourceName(const Common::String &name) const;

	uint32 _sliderState;
	Common::String _sliderBmpName;
	Common::String _sliderBgBmpName;
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
