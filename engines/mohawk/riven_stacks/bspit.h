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

#ifndef RIVEN_STACKS_BSPIT_H
#define RIVEN_STACKS_BSPIT_H

#include "mohawk/riven_stacks/domespit.h"

namespace Mohawk {
namespace RivenStacks {

/**
 * Boiler Island
 */
class BSpit : public DomeSpit {
public:
	BSpit(MohawkEngine_Riven *vm);

	// External commands - Gehn's Lab Journal
	void xblabopenbook(uint16 argc, uint16 *argv);
	void xblabbooknextpage(uint16 argc, uint16 *argv);
	void xblabbookprevpage(uint16 argc, uint16 *argv);

	// External commands - Boiler Puzzle
	void xsoundplug(uint16 argc, uint16 *argv);
	void xbchangeboiler(uint16 argc, uint16 *argv);
	void xbupdateboiler(uint16 argc, uint16 *argv);

	// External commands - Frog Trap
	void xbsettrap(uint16 argc, uint16 *argv);
	void xbcheckcatch(uint16 argc, uint16 *argv);
	void xbait(uint16 argc, uint16 *argv);
	void xbfreeytram(uint16 argc, uint16 *argv);
	void xbaitplate(uint16 argc, uint16 *argv);

	// External commands - Dome
	void xbisland190_opencard(uint16 argc, uint16 *argv);
	void xbisland190_resetsliders(uint16 argc, uint16 *argv);
	void xbisland190_slidermd(uint16 argc, uint16 *argv);
	void xbisland190_slidermw(uint16 argc, uint16 *argv);
	void xbscpbtn(uint16 argc, uint16 *argv);
	void xbisland_domecheck(uint16 argc, uint16 *argv);

	// External commands - Water Control
	void xvalvecontrol(uint16 argc, uint16 *argv);

	// External commands - Run the Wood Chipper
	void xbchipper(uint16 argc, uint16 *argv);

	// Time callback
	void checkYtramCatch(bool playSound);
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
