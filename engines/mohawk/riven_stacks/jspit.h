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

#ifndef RIVEN_STACKS_JSPIT_H
#define RIVEN_STACKS_JSPIT_H

#include "mohawk/riven_stacks/domespit.h"
#include "mohawk/riven_video.h"

namespace Mohawk {
namespace RivenStacks {

/**
 * Jungle Island
 */
class JSpit : public DomeSpit {
public:
	JSpit(MohawkEngine_Riven *vm);

	// RivenStack API
	void installCardTimer() override;

	// External commands - Rebel Tunnel Puzzle
	void xreseticons(const ArgumentArray &args);
	void xicon(const ArgumentArray &args);
	void xcheckicons(const ArgumentArray &args);
	void xtoggleicon(const ArgumentArray &args);
	void xjtunnel103_pictfix(const ArgumentArray &args);
	void xjtunnel104_pictfix(const ArgumentArray &args);
	void xjtunnel105_pictfix(const ArgumentArray &args);
	void xjtunnel106_pictfix(const ArgumentArray &args);

	// External commands - Lower the gallows carriage
	void xvga1300_carriage(const ArgumentArray &args);

	// External commands - Dome
	void xjdome25_resetsliders(const ArgumentArray &args);
	void xjdome25_slidermd(const ArgumentArray &args);
	void xjdome25_slidermw(const ArgumentArray &args);
	void xjscpbtn(const ArgumentArray &args);
	void xjisland3500_domecheck(const ArgumentArray &args);

	// External commands - Whark Elevator
	void xhandlecontroldown(const ArgumentArray &args);
	void xhandlecontrolmid(const ArgumentArray &args);
	void xhandlecontrolup(const ArgumentArray &args);

	// External commands - Beetle
	void xjplaybeetle_550(const ArgumentArray &args);
	void xjplaybeetle_600(const ArgumentArray &args);
	void xjplaybeetle_950(const ArgumentArray &args);
	void xjplaybeetle_1050(const ArgumentArray &args);
	void xjplaybeetle_1450(const ArgumentArray &args);

	// External commands - Creatures in the Lagoon
	void xjlagoon700_alert(const ArgumentArray &args);
	void xjlagoon800_alert(const ArgumentArray &args);
	void xjlagoon1500_alert(const ArgumentArray &args);

	// External commands - Play the Whark Game
	void xschool280_playwhark(const ArgumentArray &args);
	void xjschool280_resetleft(const ArgumentArray &args); // DVD only
	void xjschool280_resetright(const ArgumentArray &args); // DVD only

	// External commands - Demo-specific
	void xjatboundary(const ArgumentArray &args);

	// Timer callbacks
	void sunnersTopStairsTimer();
	void sunnersMidStairsTimer();
	void sunnersLowerStairsTimer();
	void sunnersBeachTimer();

private:
	int jspitElevatorLoop();
	void redrawWharkNumberPuzzle(uint16 overlay, uint16 number);

	void sunnersPlayVideo(RivenVideo *video, uint32 destCardGlobalId, bool sunnersShouldFlee);
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
