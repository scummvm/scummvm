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

#ifndef RIVEN_STACKS_TSPIT_H
#define RIVEN_STACKS_TSPIT_H

#include "mohawk/riven_stacks/domespit.h"

#include "common/rect.h"

namespace Mohawk {
namespace RivenStacks {

/**
 * Temple Island
 */
class TSpit : public DomeSpit {
public:
	TSpit(MohawkEngine_Riven *vm);

	// External commands - Telescope
	void xtexterior300_telescopedown(uint16 argc, uint16 *argv);
	void xtexterior300_telescopeup(uint16 argc, uint16 *argv);
	void xtopenfissure();

	// External commands - Telescope cover buttons. Button is the button number (1...5).
	void xtisland390_covercombo(uint16 argc, uint16 *argv);	// Param1: button

	// External commands - Atrus' Journal and Trap Book are added to inventory
	void xtatrusgivesbooks(uint16 argc, uint16 *argv);

	// External commands - Trap Book is removed from inventory
	void xtchotakesbook(uint16 argc, uint16 *argv);
	void xthideinventory(uint16 argc, uint16 *argv);

	// External commands - Marble Puzzle
	void xt7500_checkmarbles(uint16 argc, uint16 *argv);
	void xt7600_setupmarbles(uint16 argc, uint16 *argv);
	void xt7800_setup(uint16 argc, uint16 *argv);
	void xdrawmarbles(uint16 argc, uint16 *argv);
	void xtakeit(uint16 argc, uint16 *argv);

	// External commands - Dome
	void xtscpbtn(uint16 argc, uint16 *argv);
	void xtisland4990_domecheck(uint16 argc, uint16 *argv);
	void xtisland5056_opencard(uint16 argc, uint16 *argv);
	void xtisland5056_resetsliders(uint16 argc, uint16 *argv);
	void xtisland5056_slidermd(uint16 argc, uint16 *argv);
	void xtisland5056_slidermw(uint16 argc, uint16 *argv);

	// External commands - Demo-specific
	void xtatboundary(uint16 argc, uint16 *argv);

private:
	void drawMarbles();
	void setMarbleHotspots();

	Common::Array<Common::Rect> _marbleBaseHotspots;
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
