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
	void xblabopenbook(const ArgumentArray &args);
	void xblabbooknextpage(const ArgumentArray &args);
	void xblabbookprevpage(const ArgumentArray &args);

	// External commands - Boiler Puzzle
	void xsoundplug(const ArgumentArray &args);
	void xbchangeboiler(const ArgumentArray &args);
	void xbupdateboiler(const ArgumentArray &args);

	// External commands - Frog Trap
	void xbsettrap(const ArgumentArray &args);
	void xbcheckcatch(const ArgumentArray &args);
	void xbait(const ArgumentArray &args);
	void xbfreeytram(const ArgumentArray &args);
	void xbaitplate(const ArgumentArray &args);

	// External commands - Dome
	void xbisland190_opencard(const ArgumentArray &args);
	void xbisland190_resetsliders(const ArgumentArray &args);
	void xbisland190_slidermd(const ArgumentArray &args);
	void xbisland190_slidermw(const ArgumentArray &args);
	void xbscpbtn(const ArgumentArray &args);
	void xbisland_domecheck(const ArgumentArray &args);

	// External commands - Water Control
	void xvalvecontrol(const ArgumentArray &args);

	// External commands - Run the Wood Chipper
	void xbchipper(const ArgumentArray &args);

	// Time callback
	void ytramTrapTimer();
	void checkYtramCatch(bool playSound);

	void valveChangePosition(uint32 valvePosition, uint16 videoId, uint16 pictureId);

	void labBookDrawDomeCombination() const;
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
