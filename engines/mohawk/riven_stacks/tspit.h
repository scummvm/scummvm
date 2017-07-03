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
	void xtexterior300_telescopedown(const ArgumentArray &args);
	void xtexterior300_telescopeup(const ArgumentArray &args);
	void xtopenfissure();

	// External commands - Telescope cover buttons. Button is the button number (1...5).
	void xtisland390_covercombo(const ArgumentArray &args);	// Param1: button

	// External commands - Atrus' Journal and Trap Book are added to inventory
	void xtatrusgivesbooks(const ArgumentArray &args);

	// External commands - Trap Book is removed from inventory
	void xtchotakesbook(const ArgumentArray &args);
	void xthideinventory(const ArgumentArray &args);

	// External commands - Marble Puzzle
	void xt7500_checkmarbles(const ArgumentArray &args);
	void xt7600_setupmarbles(const ArgumentArray &args);
	void xt7800_setup(const ArgumentArray &args);
	void xdrawmarbles(const ArgumentArray &args);
	void xtakeit(const ArgumentArray &args);

	// External commands - Dome
	void xtscpbtn(const ArgumentArray &args);
	void xtisland4990_domecheck(const ArgumentArray &args);
	void xtisland5056_opencard(const ArgumentArray &args);
	void xtisland5056_resetsliders(const ArgumentArray &args);
	void xtisland5056_slidermd(const ArgumentArray &args);
	void xtisland5056_slidermw(const ArgumentArray &args);

	// External commands - Demo-specific
	void xtatboundary(const ArgumentArray &args);

private:
	void drawMarbles();
	void setMarbleHotspots();

	Common::Array<Common::Rect> _marbleBaseHotspots;
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
