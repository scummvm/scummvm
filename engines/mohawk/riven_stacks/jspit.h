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
	virtual void installCardTimer() override;

	// External commands - Rebel Tunnel Puzzle
	void xreseticons(uint16 argc, uint16 *argv);
	void xicon(uint16 argc, uint16 *argv);
	void xcheckicons(uint16 argc, uint16 *argv);
	void xtoggleicon(uint16 argc, uint16 *argv);
	void xjtunnel103_pictfix(uint16 argc, uint16 *argv);
	void xjtunnel104_pictfix(uint16 argc, uint16 *argv);
	void xjtunnel105_pictfix(uint16 argc, uint16 *argv);
	void xjtunnel106_pictfix(uint16 argc, uint16 *argv);

	// External commands - Lower the gallows carriage
	void xvga1300_carriage(uint16 argc, uint16 *argv);

	// External commands - Dome
	void xjdome25_resetsliders(uint16 argc, uint16 *argv);
	void xjdome25_slidermd(uint16 argc, uint16 *argv);
	void xjdome25_slidermw(uint16 argc, uint16 *argv);
	void xjscpbtn(uint16 argc, uint16 *argv);
	void xjisland3500_domecheck(uint16 argc, uint16 *argv);

	// External commands - Whark Elevator
	void xhandlecontroldown(uint16 argc, uint16 *argv);
	void xhandlecontrolmid(uint16 argc, uint16 *argv);
	void xhandlecontrolup(uint16 argc, uint16 *argv);

	// External commands - Beetle
	void xjplaybeetle_550(uint16 argc, uint16 *argv);
	void xjplaybeetle_600(uint16 argc, uint16 *argv);
	void xjplaybeetle_950(uint16 argc, uint16 *argv);
	void xjplaybeetle_1050(uint16 argc, uint16 *argv);
	void xjplaybeetle_1450(uint16 argc, uint16 *argv);

	// External commands - Creatures in the Lagoon
	void xjlagoon700_alert(uint16 argc, uint16 *argv);
	void xjlagoon800_alert(uint16 argc, uint16 *argv);
	void xjlagoon1500_alert(uint16 argc, uint16 *argv);

	// External commands - Play the Whark Game
	void xschool280_playwhark(uint16 argc, uint16 *argv);
	void xjschool280_resetleft(uint16 argc, uint16 *argv); // DVD only
	void xjschool280_resetright(uint16 argc, uint16 *argv); // DVD only

	// External commands - Demo-specific
	void xjatboundary(uint16 argc, uint16 *argv);

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
